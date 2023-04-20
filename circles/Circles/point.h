#pragma once
#ifndef _POINT_H_
#define _POINT_H_

#include <vector>
#include <set>
#include <map>

#include "vectors.h"

// ===========================================================
// Class to store the information when a point hits a surface
// Should be very similar to photon class
// THERE SHOULDN'T BE ANY NEED TO DYNAMICALLY ALLOCATE HERE?

class Point {
public:

    // CONSTRUCTOR
    Point(const float x_, const float y_, const Vec3f& c, const float r) :
        x(x_), y(y_), color(c), radius(r) { }

    // ACCESSORS
    const float getX() const { return x; }
    const float getY() const { return y; }
    const Vec3f& getColor() const { return color; }
    const float getRadius() const { return radius; }

private:
    // REPRESENTATION
    float x;
    float y;
    Vec3f color;
    float radius;
};

#endif