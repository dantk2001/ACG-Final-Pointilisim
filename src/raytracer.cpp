#include "raytracer.h"
#include "material.h"
#include "argparser.h"
#include "raytree.h"
#include "utils.h"
#include "mesh.h"
#include "face.h"
#include "primitive.h"
#include "photon_mapping.h"
#include "point_graph.h"
#include "boundingbox.h"
#include "camera.h"
#include <random>

// ===========================================================================
// casts a single ray through the scene geometry and finds the closest hit
bool RayTracer::CastRay(const Ray &ray, Hit &h, bool use_rasterized_patches) const {
  bool answer = false;

  // intersect each of the quads
  for (int i = 0; i < mesh->numOriginalQuads(); i++) {
    Face *f = mesh->getOriginalQuad(i);
    if (f->intersect(ray,h,args->mesh_data->intersect_backfacing)) answer = true;
  }

  // intersect each of the primitives (either the patches, or the original primitives)
  if (use_rasterized_patches) {
    for (int i = 0; i < mesh->numRasterizedPrimitiveFaces(); i++) {
      Face *f = mesh->getRasterizedPrimitiveFace(i);
      if (f->intersect(ray,h,args->mesh_data->intersect_backfacing)) answer = true;
    }
  } else {
    int num_primitives = mesh->numPrimitives();
    for (int i = 0; i < num_primitives; i++) {
      if (mesh->getPrimitive(i)->intersect(ray,h)) answer = true;
    }
  }
  return answer;
}

// ===========================================================================
// does the recursive (shadow rays & recursive rays) work
Vec3f RayTracer::TraceRay(Ray &ray, Hit &hit, int bounce_count) const {
  Ray origRay = Ray(ray.getOrigin(),ray.getDirection());
  Vec3f answer_ = Vec3f();

  for (int a = 0; a < args->mesh_data->num_antialias_samples; a++) {
    hit = Hit();
    Vec3f answer = Vec3f();
    // First cast a ray and see if we hit anything.
    Vec3f aliasNoise = Vec3f();

    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<float> distr(0,1);
    
    if (a != 0) aliasNoise = Vec3f(distr(eng)/1000.0,distr(eng)/1000.0,distr(eng)/1000.0);
    Vec3f aliasDir = origRay.getDirection() + aliasNoise; aliasDir.Normalize();
    ray = Ray(origRay.getOrigin(), aliasDir);
    bool intersect = CastRay(ray,hit,false);
      
    // if there is no intersection, simply return the background color
    if (intersect == false) {
      answer_ += Vec3f(srgb_to_linear(mesh->background_color.r()),
                  srgb_to_linear(mesh->background_color.g()),
                  srgb_to_linear(mesh->background_color.b()));
      continue;
    }

    // otherwise decide what to do based on the material
    Material *m = hit.getMaterial();
    assert (m != NULL);

    // rays coming from the light source are set to white, don't bother to ray trace further.
    if (m->getEmittedColor().Length() > 0.001) {
      answer_ += Vec3f(1,1,1);
      continue;
    } 
  
    
    Vec3f normal = hit.getNormal();
    Vec3f point = ray.pointAtParameter(hit.getT());

    Vec3f ambient_light = Vec3f(args->mesh_data->ambient_light.data[0],
                                args->mesh_data->ambient_light.data[1],
                                args->mesh_data->ambient_light.data[2]);
    
    // ----------------------------------------------
    //  start with the indirect light (ambient light)
    Vec3f diffuse_color = m->getDiffuseColor(hit.get_s(),hit.get_t());
    if (args->mesh_data->gather_indirect) {
      // photon mapping for more accurate indirect light
      answer = diffuse_color * (photon_mapping->GatherIndirect(point, normal, ray.getDirection()) + ambient_light);
    } else {
      // the usual ray tracing hack for indirect light
      answer = diffuse_color * ambient_light;
    }      

    // ----------------------------------------------
    // add contributions from each light that is not in shadow
    int num_lights = mesh->getLights().size();
    for (int i = 0; i < num_lights; i++) {

      Face *f = mesh->getLights()[i];
      Vec3f lightColor = f->getMaterial()->getEmittedColor() * f->getArea();
      Vec3f myLightColor;
      Vec3f lightCentroid = f->computeCentroid();
      Vec3f dirToLightCentroid = lightCentroid-point;
      dirToLightCentroid.Normalize();
      


      // ===========================================
      // ASSIGNMENT:  ADD SHADOW & SOFT SHADOW LOGIC
      // ===========================================

      float distToLightCentroid = (lightCentroid-point).Length();
      myLightColor = 1 / float (M_PI*distToLightCentroid*distToLightCentroid) * lightColor;

      int shadowSamples = args->mesh_data->num_shadow_samples;
      for (int k = 0; k < shadowSamples; k++) {
          //construct ray from selected point to light
          Vec3f randDirToLight = f->RandomPoint() - point;
          if (shadowSamples == 1) randDirToLight = dirToLightCentroid;
          randDirToLight.Normalize();
          Ray shadowRay = Ray(point, randDirToLight);
          RayTree::AddShadowSegment(shadowRay, 0, distToLightCentroid);
          // check for intersections with objects in scene; if just one blocks, then don't include light
          bool isBlocked = false;
          for (int j = 0; j < mesh->numPrimitives(); j++) {
            isBlocked = isBlocked || mesh->getPrimitive(j)->intersect(shadowRay, hit);
          }
          if (!isBlocked) {
            answer += (1.0 / shadowSamples) * (m->Shade(ray,hit,dirToLightCentroid,myLightColor));
          }
        }
      if (shadowSamples == 0) answer += (m->Shade(ray,hit,dirToLightCentroid,myLightColor));
      
      // add the lighting contribution from this particular light at this point
      // (fix this to check for blockers between the light & this surface)
      //answer += m->Shade(ray,hit,dirToLightCentroid,myLightColor);
    }
        
    // ----------------------------------------------
    // add contribution from reflection, if the surface is shiny


    Vec3f reflectiveColor = m->getReflectiveColor();

    
    // =================================
    // ASSIGNMENT:  ADD REFLECTIVE LOGIC
    // =================================
    if (bounce_count > 0) {
      Vec3f V = ray.getDirection(); Vec3f N = hit.getNormal();
      Vec3f reflectDir = V - 2 * V.Dot3(N) * N; reflectDir.Normalize();
      Hit reflectHit = Hit();
      Ray reflectRay = Ray(point,reflectDir);

      Vec3f recursedColor = TraceRay(reflectRay,reflectHit,bounce_count-1);
      RayTree::AddReflectedSegment(reflectRay, 0, reflectHit.getT());
      answer += Vec3f(reflectiveColor.x() * recursedColor.x(),reflectiveColor.y() * recursedColor.y(),reflectiveColor.z() * recursedColor.z());

    }

    answer_ += answer;
  }
  answer_ = (1.0/double(args->mesh_data->num_antialias_samples)) * answer_;
  return answer_; 

}








// trace a ray through pixel (i,j) of the image an return the color
Vec3f VisualizeTraceRay(double i, double j) {
  

  // compute and set the pixel color
  int max_d = mymax(GLOBAL_args->mesh_data->width,GLOBAL_args->mesh_data->height);
  Vec3f color;
  


  // ==================================
  // ASSIGNMENT: IMPLEMENT ANTIALIASING
  // ==================================
  // oh well I do this in the TraceRay function whoops

  // Here's what we do with a single sample per pixel:
  // construct & trace a ray through the center of the pixle
  double x = (i-GLOBAL_args->mesh_data->width/2.0)/double(max_d)+0.5;
  double y = (j-GLOBAL_args->mesh_data->height/2.0)/double(max_d)+0.5;
  Ray r = GLOBAL_args->mesh->camera->generateRay(x,y); 
  Hit hit;
  color = GLOBAL_args->raytracer->TraceRay(r,hit,GLOBAL_args->mesh_data->num_bounces);
  // add that ray for visualization
  RayTree::AddMainSegment(r,0,hit.getT());



  // return the color
  return color;
}




// for visualization: find the "corners" of a pixel on an image plane
// 1/2 way between the camera & point of interest
Vec3f PixelGetPos(double i, double j) {
  int max_d = mymax(GLOBAL_args->mesh_data->width,GLOBAL_args->mesh_data->height);
  double x = (i-GLOBAL_args->mesh_data->width/2.0)/double(max_d)+0.5;
  double y = (j-GLOBAL_args->mesh_data->height/2.0)/double(max_d)+0.5;
  Camera *camera = GLOBAL_args->mesh->camera;
  Ray r = camera->generateRay(x,y); 
  Vec3f cp = camera->camera_position;
  Vec3f poi = camera->point_of_interest;
  float distance = (cp-poi).Length()/2.0f;
  return r.getOrigin()+distance*r.getDirection();
}





// Scan through the image from the lower left corner across each row
// and then up to the top right.  Initially the image is sampled very
// coarsely.  Increment the static variables that track the progress
// through the scans
int RayTraceDrawPixel() {
  if (GLOBAL_args->mesh_data->raytracing_x >= GLOBAL_args->mesh_data->raytracing_divs_x) {
    // end of row
    GLOBAL_args->mesh_data->raytracing_x = 0; 
    GLOBAL_args->mesh_data->raytracing_y += 1;
  }
  if (GLOBAL_args->mesh_data->raytracing_y >= GLOBAL_args->mesh_data->raytracing_divs_y) {
    // last row
    if (GLOBAL_args->mesh_data->raytracing_divs_x >= GLOBAL_args->mesh_data->width ||
        GLOBAL_args->mesh_data->raytracing_divs_y >= GLOBAL_args->mesh_data->height) {
      // stop rendering, matches resolution of current camera
      return 0; 
    }
    // else decrease pixel size & start over again in the bottom left corner
    GLOBAL_args->mesh_data->raytracing_divs_x *= 3;
    GLOBAL_args->mesh_data->raytracing_divs_y *= 3;
    if (GLOBAL_args->mesh_data->raytracing_divs_x > GLOBAL_args->mesh_data->width * 0.51 ||
        GLOBAL_args->mesh_data->raytracing_divs_x > GLOBAL_args->mesh_data->height * 0.51) {
      GLOBAL_args->mesh_data->raytracing_divs_x = GLOBAL_args->mesh_data->width;
      GLOBAL_args->mesh_data->raytracing_divs_y = GLOBAL_args->mesh_data->height;
    }
    GLOBAL_args->mesh_data->raytracing_x = 0;
    GLOBAL_args->mesh_data->raytracing_y = 0;

    if (GLOBAL_args->raytracer->render_to_a) {
      GLOBAL_args->raytracer->pixels_b.clear();
      GLOBAL_args->raytracer->render_to_a = false;
    } else {
      GLOBAL_args->raytracer->pixels_a.clear();
      GLOBAL_args->raytracer->render_to_a = true;
    }
  }

  double x_spacing = GLOBAL_args->mesh_data->width / double (GLOBAL_args->mesh_data->raytracing_divs_x);
  double y_spacing = GLOBAL_args->mesh_data->height / double (GLOBAL_args->mesh_data->raytracing_divs_y);

  // compute the color and position of intersection
  Vec3f pos1 =  PixelGetPos((GLOBAL_args->mesh_data->raytracing_x  )*x_spacing, (GLOBAL_args->mesh_data->raytracing_y  )*y_spacing);
  Vec3f pos2 =  PixelGetPos((GLOBAL_args->mesh_data->raytracing_x+1)*x_spacing, (GLOBAL_args->mesh_data->raytracing_y  )*y_spacing);
  Vec3f pos3 =  PixelGetPos((GLOBAL_args->mesh_data->raytracing_x+1)*x_spacing, (GLOBAL_args->mesh_data->raytracing_y+1)*y_spacing);
  Vec3f pos4 =  PixelGetPos((GLOBAL_args->mesh_data->raytracing_x  )*x_spacing, (GLOBAL_args->mesh_data->raytracing_y+1)*y_spacing);

  Vec3f color = VisualizeTraceRay((GLOBAL_args->mesh_data->raytracing_x+0.5)*x_spacing, (GLOBAL_args->mesh_data->raytracing_y+0.5)*y_spacing);

  double r = linear_to_srgb(color.r());
  double g = linear_to_srgb(color.g());
  double b = linear_to_srgb(color.b());

  Pixel p;
  p.v1 = pos1;
  p.v2 = pos2;
  p.v3 = pos3;
  p.v4 = pos4;
  p.color = Vec3f(r,g,b);
    
  if (GLOBAL_args->raytracer->render_to_a) {
    GLOBAL_args->raytracer->pixels_a.push_back(p);
  } else {
    GLOBAL_args->raytracer->pixels_b.push_back(p);
  }  

  GLOBAL_args->mesh_data->raytracing_x += 1;
  return 1;
}

// ===========================================================================

int RayTracer::triCount() {
  int count = (pixels_a.size() + pixels_b.size()) * 2;
  return count;
}

void RayTracer::packMesh(float* &current) {
  for (unsigned int i = 0; i < pixels_a.size(); i++) {
    Pixel &p = pixels_a[i];
    Vec3f v1 = p.v1;
    Vec3f v2 = p.v2;
    Vec3f v3 = p.v3;
    Vec3f v4 = p.v4;
    Vec3f normal = ComputeNormal(v1,v2,v3) + ComputeNormal(v1,v3,v4);
    normal.Normalize();
    if (render_to_a) {
      v1 += 0.02*normal;
      v2 += 0.02*normal;
      v3 += 0.02*normal;
      v4 += 0.02*normal;
    }
    normal = Vec3f(0,0,0);
    AddQuad(current,v1,v2,v3,v4,normal,p.color);
  }

  for (unsigned int i = 0; i < pixels_b.size(); i++) {
    Pixel &p = pixels_b[i];
    Vec3f v1 = p.v1;
    Vec3f v2 = p.v2;
    Vec3f v3 = p.v3;
    Vec3f v4 = p.v4;
    Vec3f normal = ComputeNormal(v1,v2,v3) + ComputeNormal(v1,v3,v4);
    normal.Normalize();
    if (!render_to_a) {
      v1 += 0.02*normal;
      v2 += 0.02*normal;
      v3 += 0.02*normal;
      v4 += 0.02*normal;
    }
    normal = Vec3f(0,0,0);
    AddQuad(current,v1,v2,v3,v4,normal,p.color);
  }
}

// ===========================================================================
