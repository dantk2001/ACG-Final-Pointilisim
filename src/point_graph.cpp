#include "point.h"
#include "raytracer.h"
#include "material.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "primitive.h"
#include "photon_mapping.h"
#include "boundingbox.h"
#include "camera.h"
#include <random>
#include <vector>
#include <set>
#include <map>

// populate graph
void PointGraph::GraphPoints() {
    // throw away everything beforehand
    Clear();
    graph.clear();
    next_point_id = 0;

    // GLOBAL_args->mesh_data->height and width are 500x500
    // let's just do a nice 100x100 for now.
    int div = 5;
    int rows = GLOBAL_args->mesh_data->height / div;
    int cols = GLOBAL_args->mesh_data->width  / div;
    std::cout << rows << " x " << cols << std::endl;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float myX = (j+0.5) * div;//*x_spacing; // not sure if some of these
            float myY = (i+0.5) * div;//*y_spacing; // should be switched?
            Vec3f color = VisualizeTraceRay(myX, myY);
            color = Vec3f(linear_to_srgb(color.r()),
                          linear_to_srgb(color.g()),
                          linear_to_srgb(color.b()));

            std::set<int> tmp;
            if (next_point_id - cols >= 0) {
                tmp.insert(next_point_id - cols); // up
                if (j != 0) tmp.insert(next_point_id - cols - 1); // up left
                if (cols - j != 1) tmp.insert(next_point_id - cols + 1); // up right
            }

            if (j != 0) tmp.insert(next_point_id - 1); // left
            if (cols - j != 1) tmp.insert(next_point_id + 1); // right

            if (next_point_id + cols < rows*cols) {
                tmp.insert(next_point_id + cols); // down
                if (j != 0) tmp.insert(next_point_id + cols - 1); // down left
                if (cols - j != 1) tmp.insert(next_point_id + cols + 1); // down right
            }
            
            graph[next_point_id] = new Point(Vec3f(myX, myY, 0), color, next_point_id, 0, tmp);
            next_point_id++;
        }
    }
}

void PointGraph::Clear() {
    for (std::map<int,Point*>::iterator it = graph.begin(); it != graph.end(); it++) {
        delete it->second;
    }
}

void PointGraph::packMesh(float* &current, float* &current_points) {
    for (std::map<int,Point*>::iterator it = graph.begin(); it != graph.end(); it++) {
        Vec3f v = raytracer->PublicPixelGetPos(it->second->getPosition().x(),it->second->getPosition().y());
        Vec3f color = it->second->getColor();
        float12 t = { float(v.x()),float(v.y()),float(v.z()),1,   0,0,0,0,   float(color.r()),float(color.g()),float(color.b()),1 };
        memcpy(current_points, &t, sizeof(float)*12); current_points += 12;
    }
}

//iterate through this set of neighbors and update the point in the graphs neighbor sets to have new_index instead of index or index_two
void PointGraph::cleanUpNeighbors(std::set<int> neighbors, int id1, int id2) {
    for (std::set<int>::iterator itr = neighbors.begin(); itr != neighbors.end(); itr++) {
        Point* p = graph.find(*itr)->second;
        std::set<int> n = p->getNeighbors();
        n.erase(id1);
        n.erase(id2);
        n.insert(next_point_id);
        p->setNeighbors(n);
    }
}

void PointGraph::CombinePoints() {
    //combination algorithm
    std::vector<int> deleteList;
    std::vector<std::pair<int, Point*>> addList;
    for (std::map<int, Point*>::iterator itr = graph.begin(); itr != graph.end(); itr++) {
        int index = itr->first;
        Point* point = itr->second;
        //can change times combined to match with a global variable later
        if (point->getTimesCombined() == 0 && std::count(deleteList.begin(), deleteList.end(), index) == 0) {
            std::set<int> n = point->getNeighbors();
            std::set<int>::iterator it;
            std::pair<int, float> closest = { index, 2.0f };
            for (it = n.begin(); it != n.end(); it++) {
                //should only do combinations with points who have never combined?
                if (graph.find(*it)->second->getTimesCombined() != 0) {
                    continue;
                }
                float dist = abs(DistanceBetweenTwoPoints(point->getColor(), graph.find(*it)->second->getColor()));
                //if (itr == n.begin()) {
                //    std::cout << dist << std::endl;
                //}
                if (dist < closest.second && dist <= GLOBAL_args->threshold) {
                    //std::cout << "new closest" << std::endl;
                    closest = std::make_pair(*it, dist);
                }
            }
            int index_two = closest.first;
            //found one within threshold
            if (index_two != index) {
                Point* point_two = graph.find(closest.second)->second;

                float p1_weight = (float)(point->getTimesCombined() + 1.0);
                float p2_weight = (float)(point_two->getTimesCombined() + 1.0);

                //std::cout << "combining : " << index << " " << closest.first << std::endl;
                Vec3f avgColor = ((point->getColor() * p1_weight) + (point_two->getColor() * p2_weight));
                avgColor /= p1_weight + p2_weight; //2.0f;
                Vec3f avgPosition = ((point->getPosition() * p1_weight) + (point_two->getPosition() * p2_weight));
                avgPosition /= p1_weight + p2_weight; //2.0f;
                //combine neighbors
                std::set<int> n1 = point->getNeighbors();
                std::set<int> n2 = point_two->getNeighbors();
                n1.insert(n2.begin(), n2.end());
                //make sure index and index two aren't in the neighbors set
                n1.erase(index);
                n1.erase(index_two);
                //clean up neighbors
                cleanUpNeighbors(n1, index, index_two);
                //make new point with graph next_index
                Point* new_point = new Point(avgPosition, avgColor, next_point_id, point->getTimesCombined() + 1, n1);
                //add new one to addList
                addList.push_back(std::pair<int, Point*>(index, new_point));
                next_point_id++;
                //delete old points
                delete point;
                delete point_two;
                //remove old two from map by adding to deletelist
                deleteList.push_back(index);
                deleteList.push_back(index_two);
            }
        }
    }

    //remove all points in delete list
    for (int i = 0; i < deleteList.size(); i++) {
        //delete point
        graph.erase(deleteList[i]);
    }

    //add all points in add list
    for (int i = 0; i < addList.size(); i++) {
        //delete point
        graph.insert(addList[i]);
    }
}

// =========================================================================
