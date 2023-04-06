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
    graph.clear();

    // GLOBAL_args->mesh_data->height and width are 500x500
    std::cout << "hi it works\n";
}

void PointGraph::Clear() {

}

// =========================================================================
