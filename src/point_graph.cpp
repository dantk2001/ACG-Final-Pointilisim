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
    // TODO CLEAR DYNAMIC MAP
    for (std::map<int,Point*>::iterator it = graph.begin(); it != graph.end(); it++) {
        delete it->second;
    }
    graph.clear();
    next_point_id = 0;

    // GLOBAL_args->mesh_data->height and width are 500x500
    // let's just do a nice 100x100 for now.
    int div = 5;
    int rows = GLOBAL_args->mesh_data->height / div;
    int cols = GLOBAL_args->mesh_data->width  / div;
    double y_spacing = GLOBAL_args->mesh_data->height / div;
    double x_spacing = GLOBAL_args->mesh_data->width  / div;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float myX = (i+0.5)*x_spacing; // not sure if some of these
            float myY = (j+0.5)*y_spacing; // should be switched?
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

// =========================================================================
