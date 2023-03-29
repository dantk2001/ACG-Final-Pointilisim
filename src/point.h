#ifndef _POINT_H_
#define _POINT_H_

#include <vector>
#include <set>
#include <map>

#include "vectors.h"

class ArgParser;
class RayTracer;

// ===========================================================
// Class to store the information when a point hits a surface
// Should be very similar to photon class
// THERE SHOULDN'T BE ANY NEED TO DYNAMICALLY ALLOCATE HERE?

class Point {
 public:

  // CONSTRUCTOR
  Point(const Vec3f &p, const Vec3f &c, const int id_, int t, std::set<int> n) :
    position(p),color(c),id(id_),times_combined(t),neighbors(n) {}

  // ACCESSORS
  const Vec3f& getPosition() const { return position; }
  const Vec3f& getColor() const { return color; }
  const int getID() const { return id; }
  int getTimesCombined() const { return times_combined; }

  // MODIFIERS
  void setNeighbors(std::set<int> n) { neighbors = n; }

 private:
  // REPRESENTATION
  Vec3f position;
  Vec3f color;
  int id;
  int times_combined;
  std::set<int> neighbors;
};

#endif
