#ifndef _POINT_GRAPH_H_
#define _POINT_GRAPH_H_

#include <vector>
#include <set>
#include <map>

#include "point.h"

class Mesh;
class ArgParser;
class Ray;
class Hit;
class RayTracer;

// =========================================================================
// A class that shoots points through pixels on the screen, collects the color information,
// and stores adjacent pixels as a graph structure for BFS access.

class PointGraph {

 public:

  // CONSTRUCTOR & DESTRUCTOR
  PointGraph(Mesh* _mesh, ArgParser *_args) {
    mesh = _mesh;
    args = _args;
    next_point_id = 0;
    // this is just for reference lol
    //graph[0] = new Point(Vec3f(),Vec3f(),0,0,std::set<int>());
    // map uses erase for specific elements i.e. graph.erase(1);
  }
  ~PointGraph() { Clear(); }

  void setRayTracer(RayTracer* r) { raytracer = r; }
  // populate graph
  void GraphPoints();

  void Clear();
  
 private:
  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  RayTracer* raytracer;
  std::map<int,Point*> graph; // map of nodes, each node contains neighbors (edges)
  int next_point_id;
};

// =========================================================================

#endif
