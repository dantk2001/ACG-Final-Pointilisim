#include "utils.h"
#include "material.h"
#include "argparser.h"
#include "sphere.h"
#include "vertex.h"
#include "mesh.h"
#include "ray.h"
#include "hit.h"

// ====================================================================
// ====================================================================

bool Sphere::intersect(const Ray &r, Hit &h) const {

  // ==========================================
  // ASSIGNMENT:  IMPLEMENT SPHERE INTERSECTION
  // ==========================================

  // plug the explicit ray equation into the implict sphere equation and solve
  // convert ray to sphere's coordinates
  Vec3f Ro = r.getOrigin() - center;
  Vec3f Rd = r.getDirection();
  // check if ray is pointing towards or away from sphere
  // breakpoint
  if ((center - (Ro + Rd)).Length() > (center - (Ro - Rd)).Length()) return false;
  // check if intersection occurs
  // quadratic...
  double a = 1.0; double b = 2.0 * Rd.Dot3(Ro); double c = Ro.Dot3(Ro) - square(radius);
  double d = (square(b) - (4*a*c));
  if (d < 0) return false; // 0 solutions
  // return true if the sphere was intersected, and update the hit
  // data structure to contain the value of t for the ray at the
  // intersection point, the material, and the normal
  // will definitely have at least one solution, might have 2
  double quadPlus = (-b + sqrt(d)) / (2.0 * a);
  double quadMinus = (-b - sqrt(d)) / (2.0 * a);
  Vec3f p1 = r.pointAtParameter(quadPlus);
  Vec3f p2 = r.pointAtParameter(quadMinus);
  Vec3f point; double t;
  // set closer intersection point as hit
  if ((p1 - Ro).Length() < (p1 - Ro).Length()) { point = p1; t = quadPlus; }
  else { point = p2; t = quadMinus; }
  if (t < 0) return false;
  // update hit data structure
  Vec3f normal = (point - center); normal.Normalize();
  h.set(t, material, normal);
  return true;

} 

// ====================================================================
// ====================================================================

// helper function to place a grid of points on the sphere
Vec3f ComputeSpherePoint(float s, float t, const Vec3f center, float radius) {
  float angle = 2*M_PI*s;
  float y = -cos(M_PI*t);
  float factor = sqrt(1-y*y);
  float x = factor*cos(angle);
  float z = factor*-sin(angle);
  Vec3f answer = Vec3f(x,y,z);
  answer *= radius;
  answer += center;
  return answer;
}

void Sphere::addRasterizedFaces(Mesh *m, ArgParser *args) {
  
  // and convert it into quad patches for radiosity
  int h = args->mesh_data->sphere_horiz;
  int v = args->mesh_data->sphere_vert;
  assert (h % 2 == 0);
  int i,j;
  int va,vb,vc,vd;
  Vertex *a,*b,*c,*d;
  int offset = m->numVertices(); //vertices.size();

  // place vertices
  m->addVertex(center+radius*Vec3f(0,-1,0));  // bottom
  for (j = 1; j < v; j++) {  // middle
    for (i = 0; i < h; i++) {
      float s = i / float(h);
      float t = j / float(v);
      m->addVertex(ComputeSpherePoint(s,t,center,radius));
    }
  }
  m->addVertex(center+radius*Vec3f(0,1,0));  // top

  // the middle patches
  for (j = 1; j < v-1; j++) {
    for (i = 0; i < h; i++) {
      va = 1 +  i      + h*(j-1);
      vb = 1 + (i+1)%h + h*(j-1);
      vc = 1 +  i      + h*(j);
      vd = 1 + (i+1)%h + h*(j);
      a = m->getVertex(offset + va);
      b = m->getVertex(offset + vb);
      c = m->getVertex(offset + vc);
      d = m->getVertex(offset + vd);
      m->addRasterizedPrimitiveFace(a,b,d,c,material);
    }
  }

  for (i = 0; i < h; i+=2) {
    // the bottom patches
    va = 0;
    vb = 1 +  i;
    vc = 1 + (i+1)%h;
    vd = 1 + (i+2)%h;
    a = m->getVertex(offset + va);
    b = m->getVertex(offset + vb);
    c = m->getVertex(offset + vc);
    d = m->getVertex(offset + vd);
    m->addRasterizedPrimitiveFace(d,c,b,a,material);
    // the top patches
    va = 1 + h*(v-1);
    vb = 1 +  i      + h*(v-2);
    vc = 1 + (i+1)%h + h*(v-2);
    vd = 1 + (i+2)%h + h*(v-2);
    a = m->getVertex(offset + va);
    b = m->getVertex(offset + vb);
    c = m->getVertex(offset + vc);
    d = m->getVertex(offset + vd);
    m->addRasterizedPrimitiveFace(b,c,d,a,material);
  }
}
