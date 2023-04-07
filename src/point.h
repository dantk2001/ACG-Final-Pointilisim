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
    position(p),color(c),id(id_),times_combined(t),neighbors(n) { lock = false; }

  // ACCESSORS
  const Vec3f& getPosition() const { return position; }
  const Vec3f& getColor() const { return color; }
  const int getID() const { return id; }
  int getTimesCombined() const { return times_combined; }
  std::set<int> getNeighbors() { return neighbors; }
  void printNeighbors() const {
    std::cout << "Printing neighbors of " << id << std::endl;
    for (std::set<int>::iterator it = neighbors.begin(); it != neighbors.end(); it++) {
      std::cout << *it << " ";
    } std::cout << std::endl;
  }

  // ACCESSORS
  bool isLocked() { return lock; }

  // MODIFIERS
  void setNeighbors(std::set<int> n) { neighbors = n; }
<<<<<<< HEAD
  void Lock() { lock = true; }
=======
  //for combination algorithm
  void removeNeighbor(int id) { neighbors.erase(id); }
  void updateId() { id--; }
>>>>>>> main

 private:
  // REPRESENTATION
  Vec3f position;
  Vec3f color;
  int id;
  int times_combined; // at start of init is 0
  std::set<int> neighbors;
  bool lock;
};

#endif
