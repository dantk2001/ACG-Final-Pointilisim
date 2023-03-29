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
  PointGraph(ArgParser *_args, int i_, int j_) {
    args = _args;
    i = i_; j = j_;
    // this is just for reference lol
    //graph[0] = new Point(Vec3f(),Vec3f(),0,0,std::set<int>());
    // map uses erase for specific elements i.e. graph.erase(1);
  }
  ~PointGraph() { Clear(); }

  // populate graph
  void GraphPoints();

  void Clear();
  
 private:
  // REPRESENTATION
  ArgParser *args;
  // raytracer passes these in (height and width)
  int i;
  int j;
  std::map<int,Point*> graph; // map of nodes, each node contains neighbors (edges)
};

// =========================================================================

#endif
