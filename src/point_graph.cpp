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
#include <filesystem>
#include <cmath>


void PointGraph::WriteGraph() {
    //Change this to match your own path to Circles
    std::ofstream out_file("../circles/Circles/" + GLOBAL_args->out_file + ".txt");

    out_file << GLOBAL_args->mesh_data->width << " " << GLOBAL_args->mesh_data->height << "\n";

    for (std::map<int, Point*>::iterator itr = graph.begin(); itr != graph.end(); itr++) {
        Point* p = PostProcessPoint(itr->second); //Point* p = itr->second;
        Vec3f color = p->getColor();
        Vec3f position = p->getPosition();
        out_file << color[0] << " " << color[1] << " " << color[2] << " " << position[0] << " " << position[1] << " " << p->getTimesCombined() << "\n";
    }

    out_file.close();

    std::cout << "Output to " << GLOBAL_args->out_file << ".txt" << std::endl;
}

// populate graph
void PointGraph::GraphPoints() {
    // throw away everything beforehand
    Clear();

    // GLOBAL_args->mesh_data->height and width are 500x500
    // let's just do a nice 100x100 for now.
    int div = 5;
    int rows = GLOBAL_args->mesh_data->height / div;
    int cols = GLOBAL_args->mesh_data->width  / div;
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
    //printGraph();
    WriteGraph();
}

void PointGraph::Clear() {
    if (graph.size() == 0) return;
    for (std::map<int,Point*>::iterator it = graph.begin(); it != graph.end(); it++) {
        if (it->second != NULL) delete it->second;
    }
    graph.clear();
    next_point_id = 0;
}

void PointGraph::packMesh(float* &current, float* &current_points) {
    for (std::map<int,Point*>::iterator it = graph.begin(); it != graph.end(); it++) {
        Vec3f v = raytracer->PublicPixelGetPos(it->second->getPosition().x(),it->second->getPosition().y());
        Vec3f color = it->second->getColor();
        float12 t = { float(v.x()),float(v.y()),float(v.z()),1,   0,0,0,0,   float(color.r()),float(color.g()),float(color.b()),1 };
        memcpy(current_points, &t, sizeof(float)*12); current_points += 12;
        //drawCircle(v.x(), v.y(), v.z(), it->second->getTimesCombined() * 0.01);
    }
}

//iterate through this set of neighbors and update the point in the graphs neighbor sets to have new_index instead of index or index_two
void PointGraph::cleanUpNeighbors(std::set<int> neighbors, int id1, int id2) {
    for (std::set<int>::iterator itr = neighbors.begin(); itr != neighbors.end(); itr++) {
        Point* p;
        //have to check if a neighbor is in the add list and update from there, not from the graph
        std::map<int, Point*>::iterator found = graph.find(*itr);
        if (found == graph.end()) {
            p = addList.find(*itr)->second;
        }
        else {
            p = graph.find(*itr)->second;
        }
        std::set<int> n = p->getNeighbors();
        n.erase(id1);
        n.erase(id2);
        n.insert(next_point_id);
        p->setNeighbors(n);
    }
}

void PointGraph::printGraph() {
    for (std::map<int, Point*>::iterator itr = graph.begin(); itr != graph.end(); itr++) {
        std::set<int> n = itr->second->getNeighbors();
        for (std::set<int>::iterator it = n.begin(); it != n.end(); it++) {
            bool isIn = graph.find(*it) != graph.end();
            if (!isIn) {
                std::cout << itr->first << " : " << *it << std::endl;
            }
        }
    }
}

void PointGraph::CombinePoints() {
    //combination algorithm
    //1.732 is max dist
    int num_combined = 1;
    while(num_combined != 0) {
        std::vector<int> deleteList;
        for (std::map<int, Point*>::iterator itr = graph.begin(); itr != graph.end(); itr++) {
            int index = itr->first;
            Point* point = itr->second;
            //can change times combined to match with a global variable later
            if (point->getTimesCombined() < 2 && std::count(deleteList.begin(), deleteList.end(), index) == 0) {
                std::set<int> n = point->getNeighbors();
                std::pair<int, float> closest = { index, 2.0f };
                for (std::set<int>::iterator it = n.begin(); it != n.end(); it++) {
                    //should only do combinations with points who have never combined, have not yet been added to graph, or in delete list
                    std::map<int, Point*>::iterator found = graph.find(*it);
                    if (found == graph.end() || std::count(deleteList.begin(), deleteList.end(), *it) != 0) {
                        continue;
                    }
                    if (found->second->getTimesCombined() > 1) {
                        continue;
                    }
                    float dist = abs(DistanceBetweenTwoPoints(point->getColor(), found->second->getColor()));
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
                    Point* point_two = graph.find(index_two)->second;

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
                    Point* new_point = new Point(avgPosition, avgColor, next_point_id, point->getTimesCombined() + point_two->getTimesCombined() + 1, n1);
                    //add new one to addList
                    addList.insert(std::pair<int, Point*>(next_point_id, new_point));
                    next_point_id++;
                    //remove old two from map by adding to deletelist
                    deleteList.push_back(index);
                    deleteList.push_back(index_two);
                    //std::cout << "done combining : " << index << " " << closest.first << std::endl;
                }
            }
        }

        //std::cout << "updating graph" << std::endl;

        //remove all points in delete list
        for (int i = 0; i < deleteList.size(); i++) {
            //delete point
            //std::cout << deleteList[i] << std::endl;
            delete graph.find(deleteList[i])->second;
            graph.erase(deleteList[i]);
        }

        num_combined = addList.size();

        std::cout << "number of points combined: " << num_combined << std::endl;

        //add all points in add list
        for (std::map<int, Point*>::iterator itr = addList.begin(); itr != addList.end(); itr++) {
            //add point
            graph.insert(*itr);
        }

        addList.clear();
    }

    WriteGraph();
}

Point* PointGraph::PostProcessPoint(Point* p) {
    // random jitter position (ref'd off of utils.h's RandomUnitVector() function)
    Vec3f posJitter = Vec3f(2*GLOBAL_args->rand()-1, 2*GLOBAL_args->rand()-1, 0);
    posJitter.Normalize(); posJitter = 0.25 * posJitter; // scale it a bit
    Vec3f newPos = posJitter + p->getPosition();

    // rgb already [0,1]
    // rgb to hsl
    Vec3f color = p->getColor();
    float max = color.r(); char maxRGB = 'r';
    if (color.g() > max) { max = color.g(); maxRGB = 'g'; }
    if (color.b() > max) { max = color.b(); maxRGB = 'b'; }
    float min = color.r();
    if (color.g() < min) { min = color.g(); }
    if (color.b() < min) { min = color.b(); }
    float delta = max - min;

    float l = (max + min) / 2.0;
    float s = delta/(1.0 - fabs((2 * l) - 1.0));
    float h = 0; if (fabs(delta) > EPSILON) {
        if (maxRGB == 'r') h = 60.0 * (fmod((color.g() - color.b())/delta, 6.0));
        if (maxRGB == 'g') h = 60.0 * (((color.b() - color.r())/delta) + 2.0);
        if (maxRGB == 'b') h = 60.0 * (((color.r() - color.g())/delta) + 4.0);
    }

    // determine hue shift;
    float hueShift = 0.0; float base = 45;
    float shade_prob = 1 - color.Length();
    //the darker it is the more complementary colors appear
    if (2*GLOBAL_args->rand()-1 < shade_prob) {
        float dir = 2*GLOBAL_args->rand()-1; // it's then split evenly 1/2 and 1/2 for which direction
        if (dir > 0.5) hueShift = base;
        else hueShift = 360.0 - base;
        h += hueShift; h = fmod(h,360.0);
    }

    float sat = 2*GLOBAL_args->rand()-1;
    if (sat > 0.5) {
        float sat2 = 1.0 + (2*GLOBAL_args->rand()-1);
        s *= sat; if (s > 1.0) s = 1.0;
    }

    // hsl back to rgb
    float c = (1.0 - fabs((2 * l) - 1.0)) * s;
    float x = c * (1.0 - fabs(fmod(h/60.0,2) - 1.0));
    float m = l - (c/2.0);
    Vec3f preRGB = Vec3f();
    if      (h >=EPSILON&& h <  60.0) preRGB = Vec3f(c,x,0);
    else if (h >=  60.0 && h < 120.0) preRGB = Vec3f(x,c,0);
    else if (h >= 120.0 && h < 180.0) preRGB = Vec3f(0,c,x);
    else if (h >= 180.0 && h < 240.0) preRGB = Vec3f(0,x,c);
    else if (h >= 240.0 && h < 300.0) preRGB = Vec3f(x,0,c);
    else if (h >= 300.0 && h < 360.0) preRGB = Vec3f(c,0,x);

    Vec3f newColor = Vec3f(preRGB.r() + m,preRGB.g() + m, preRGB.b() + m);
    if (color.Length() <= EPSILON) newColor = Vec3f();

    Point newPoint = Point(newPos,newColor,p->getID(),p->getTimesCombined(),p->getNeighbors());
    return &newPoint;
}

// =========================================================================
