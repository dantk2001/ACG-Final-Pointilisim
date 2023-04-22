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

  // ACCESSORS
  Point* getPoint(int i) { return graph[i]; }
  int pointCount() { return graph.size(); }
  void VisualizeGraph(); // visualizer

  void PointGraph::printGraph();

  // MODIFIERS
  void setRayTracer(RayTracer* r) { raytracer = r; }
  void GraphPoints(); // populate graph
  void CombinePoints();
  Point* PostProcessPoint(Point* p);
  void Clear();
  void cleanUpNeighbors(std::set<int> neighbors, int id1, int id2);

  // RENDERING ??
  void packMesh(float* &current, float* &current_points);

  // WRITING
  void WriteGraph();
  
 private:
  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  RayTracer* raytracer;
  std::map<int,Point*> graph; // map of nodes, each node contains neighbors (edges)
  std::map<int, Point*> addList;
  int next_point_id;
};

// =========================================================================

#endif
