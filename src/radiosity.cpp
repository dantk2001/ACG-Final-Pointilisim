#include "vectors.h"
#include "radiosity.h"
#include "mesh.h"
#include "face.h"
#include "sphere.h"
#include "raytree.h"
#include "raytracer.h"
#include "utils.h"

// ================================================================
// CONSTRUCTOR & DESTRUCTOR
// ================================================================
Radiosity::Radiosity(Mesh *m, ArgParser *a) {
  mesh = m;
  args = a;
  num_faces = -1;  
  formfactors = NULL;
  area = NULL;
  undistributed = NULL;
  absorbed = NULL;
  radiance = NULL;
  max_undistributed_patch = -1;
  total_area = -1;
  Reset();
}

Radiosity::~Radiosity() {
  Cleanup();
}

void Radiosity::Cleanup() {
  delete [] formfactors;
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;
  num_faces = -1;
  formfactors = NULL;
  area = NULL;
  undistributed = NULL;
  absorbed = NULL;
  radiance = NULL;
  max_undistributed_patch = -1;
  total_area = -1;
}

void Radiosity::Reset() {
  delete [] area;
  delete [] undistributed;
  delete [] absorbed;
  delete [] radiance;

  // create and fill the data structures
  num_faces = mesh->numFaces();
  area = new float[num_faces];
  undistributed = new Vec3f[num_faces];
  absorbed = new Vec3f[num_faces];
  radiance = new Vec3f[num_faces];
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    f->setRadiosityPatchIndex(i);
    setArea(i,f->getArea());
    Vec3f emit = f->getMaterial()->getEmittedColor();
    setUndistributed(i,emit);
    setAbsorbed(i,Vec3f(0,0,0));
    setRadiance(i,emit);
  }

  // find the patch with the most undistributed energy
  findMaxUndistributed();
}


// =======================================================================================
// =======================================================================================

void Radiosity::findMaxUndistributed() {
  // find the patch with the most undistributed energy 
  // don't forget that the patches may have different sizes!
  max_undistributed_patch = -1;
  total_undistributed = 0;
  total_area = 0;
  float max = -1;
  for (int i = 0; i < num_faces; i++) {
    float m = getUndistributed(i).Length() * getArea(i);
    total_undistributed += m;
    total_area += getArea(i);
    if (max < m) {
      max = m;
      max_undistributed_patch = i;
    }
  }
  assert (max_undistributed_patch >= 0 && max_undistributed_patch < num_faces);
}


void Radiosity::ComputeFormFactors() {
  assert (formfactors == NULL);
  assert (num_faces > 0);
  formfactors = new float[num_faces*num_faces];

  // =====================================
  // ASSIGNMENT:  COMPUTE THE FORM FACTORS
  // =====================================
  // go through every face and compare to every other face
  for (int i = 0; i < num_faces; i++) {
    Face* i_face = getMesh()->getFace(i);
    for (int j = 0; j < num_faces; j++) {
      if (i == j) { formfactors[i*num_faces + j] = 0.0; continue; }

      Face* j_face = getMesh()->getFace(j);
      // calculate visibility between patches
      int n = args->mesh_data->num_form_factor_samples; // how many rays to cast between patches?
      float sum = 0.0;
      for (int k = 0; k < n; k++) {
        // random point to random point, unless it's the first ray
        Vec3f i_point = i_face->RandomPoint(); if (k == 0) i_point = i_face->computeCentroid();
        Vec3f j_point = j_face->RandomPoint(); if (k == 0) j_point = j_face->computeCentroid();

        Vec3f dir_ij = j_point - i_point; dir_ij.Normalize();
        Ray r = Ray(i_point, dir_ij);
        Hit h = Hit();

        Vec3f dir_ji = i_point - j_point; dir_ji.Normalize();
        Ray r2 = Ray(j_point, dir_ji);
        Hit h2 = Hit();

        // if there's a hit, compute point-to-point form factor
        if (j_face->intersect(r, h, false) && i_face->intersect(r2, h2, false)) {
          // first check to make sure nothing is in the way
          Hit h_temp = Hit();
          bool skip = false;
          // CastRay looks for whole primitives rather than their faces so I'll just take the code
          // and replace it with all faces in mesh rather than quads and primitives
          for (int i = 0; i < getMesh()->numFaces(); i++) {
            Face *f = getMesh()->getFace(i);
            if (f->intersect(r,h_temp,args->mesh_data->intersect_backfacing))
              if (h_temp.getT() < h.getT()) skip = true;
          }
          if (!skip) {
            // h will have j's normal
            // h2 will have i's normal
            Vec3f v_i = h.getT() * dir_ij;
            Vec3f v_j = h2.getT() * -dir_ij;
            float r = (j_point - i_point).Length();
            float pi = 3.14159265; // if this was in utils I couldn't find it
            float cos_theta_i = v_i.Dot3(h2.getNormal()) / r; // normal should be length 1
            float cos_theta_j = v_j.Dot3(h.getNormal()) / r;

            float v = 4/3 * pi; // unit sphere ??
            sum += (cos_theta_i * cos_theta_j) * v; // simplifies to this if we take A's as unit circle
          }
        }
      }
      formfactors[i*num_faces + j] = sum/float(n);
    }
  }

  // each row of form factor 2D array should sum to 0
  for (int i = 0; i < num_faces; i++) {
    normalizeFormFactors(i);
  }
}


// ================================================================
// ================================================================

float Radiosity::Iterate() {
  if (formfactors == NULL) 
    ComputeFormFactors();
  assert (formfactors != NULL);

  // ==========================================
  // ASSIGNMENT:  IMPLEMENT RADIOSITY ALGORITHM
  // ==========================================
  findMaxUndistributed();
  double pi = 3.14159265;
  int j = max_undistributed_patch;
  for (int i = 0; i < num_faces; i++) {
    float ff = getFormFactor(i,j);
    if (ff > 0) {
      Vec3f matR = getMesh()->getFace(i)->getMaterial()->getDiffuseColor();
      Vec3f FB = ff * getUndistributed(j);
      Vec3f pFB = Vec3f(matR.x() * FB.x(),matR.y() * FB.y(),matR.z() * FB.z());
      total_undistributed += pFB.Length();
      setUndistributed(i, getUndistributed(i) + pFB);
      setRadiance(i, getRadiance(i) + pFB);
      Vec3f absorbed = (ff / pi) * getUndistributed(i); // highly unsure
      setAbsorbed(i, getAbsorbed(i) + absorbed);
    }
  }
  total_undistributed -= getUndistributed(j).Length();
  //std::cout << total_undistributed << std::endl;
  setUndistributed(j,Vec3f());
  // return the total light yet undistributed
  // (so we can decide when the solution has sufficiently converged)
  return total_undistributed;


}



// =======================================================================================
// HELPER FUNCTIONS FOR RENDERING
// =======================================================================================

// for interpolation
void CollectFacesWithVertex(Vertex *have, Face *f, std::vector<Face*> &faces) {
  for (unsigned int i = 0; i < faces.size(); i++) {
    if (faces[i] == f) return;
  }
  if (have != (*f)[0] && have != (*f)[1] && have != (*f)[2] && have != (*f)[3]) return;
  faces.push_back(f);
  for (int i = 0; i < 4; i++) {
    Edge *ea = f->getEdge()->getOpposite();
    Edge *eb = f->getEdge()->getNext()->getOpposite();
    Edge *ec = f->getEdge()->getNext()->getNext()->getOpposite();
    Edge *ed = f->getEdge()->getNext()->getNext()->getNext()->getOpposite();
    if (ea != NULL) CollectFacesWithVertex(have,ea->getFace(),faces);
    if (eb != NULL) CollectFacesWithVertex(have,eb->getFace(),faces);
    if (ec != NULL) CollectFacesWithVertex(have,ec->getFace(),faces);
    if (ed != NULL) CollectFacesWithVertex(have,ed->getFace(),faces);
  }
}

// different visualization modes
Vec3f Radiosity::setupHelperForColor(Face *f, int i, int j) {
  assert (mesh->getFace(i) == f);
  assert (j >= 0 && j < 4);
  if (args->mesh_data->render_mode == RENDER_MATERIALS) {
    return f->getMaterial()->getDiffuseColor();
  } else if (args->mesh_data->render_mode == RENDER_RADIANCE && args->mesh_data->interpolate == true) {
    std::vector<Face*> faces;
    CollectFacesWithVertex((*f)[j],f,faces);
    float total = 0;
    Vec3f color = Vec3f(0,0,0);
    Vec3f normal = f->computeNormal();
    for (unsigned int i = 0; i < faces.size(); i++) {
      Vec3f normal2 = faces[i]->computeNormal();
      float area = faces[i]->getArea();
      if (normal.Dot3(normal2) < 0.5) continue;
      assert (area > 0);
      total += area;
      color += float(area) * getRadiance(faces[i]->getRadiosityPatchIndex());
    }
    assert (total > 0);
    color /= total;
    return color;
  } else if (args->mesh_data->render_mode == RENDER_LIGHTS) {
    return f->getMaterial()->getEmittedColor();
  } else if (args->mesh_data->render_mode == RENDER_UNDISTRIBUTED) { 
    return getUndistributed(i);
  } else if (args->mesh_data->render_mode == RENDER_ABSORBED) {
    return getAbsorbed(i);
  } else if (args->mesh_data->render_mode == RENDER_RADIANCE) {
    return getRadiance(i);
  } else if (args->mesh_data->render_mode == RENDER_FORM_FACTORS) {
    if (formfactors == NULL) ComputeFormFactors();
    float scale = 0.2 * total_area/getArea(i);
    float factor = scale * getFormFactor(max_undistributed_patch,i);
    return Vec3f(factor,factor,factor);
  } else {
    assert(0);
  }
  exit(0);
}

// =======================================================================================

int Radiosity::triCount() {
  return 12*num_faces;
}

void Radiosity::packMesh(float* &current) {
  
  for (int i = 0; i < num_faces; i++) {
    Face *f = mesh->getFace(i);
    Vec3f normal = f->computeNormal();

    //double avg_s = 0;
    //double avg_t = 0;

    // wireframe is normally black, except when it's the special
    // patch, then the wireframe is red
    Vec3f wireframe_color(0,0,0);
    if (args->mesh_data->render_mode == RENDER_FORM_FACTORS && i == max_undistributed_patch) {
      wireframe_color = Vec3f(1,0,0);
    }

    // 4 corner vertices
    Vec3f a_pos = ((*f)[0])->get();
    Vec3f a_color = setupHelperForColor(f,i,0);
    a_color = Vec3f(linear_to_srgb(a_color.r()),linear_to_srgb(a_color.g()),linear_to_srgb(a_color.b()));
    Vec3f b_pos = ((*f)[1])->get();
    Vec3f b_color = setupHelperForColor(f,i,1);
    b_color = Vec3f(linear_to_srgb(b_color.r()),linear_to_srgb(b_color.g()),linear_to_srgb(b_color.b()));
    Vec3f c_pos = ((*f)[2])->get();
    Vec3f c_color = setupHelperForColor(f,i,2);
    c_color = Vec3f(linear_to_srgb(c_color.r()),linear_to_srgb(c_color.g()),linear_to_srgb(c_color.b()));
    Vec3f d_pos = ((*f)[3])->get();
    Vec3f d_color = setupHelperForColor(f,i,3);
    d_color = Vec3f(linear_to_srgb(d_color.r()),linear_to_srgb(d_color.g()),linear_to_srgb(d_color.b()));

    Vec3f avg_color = 0.25f * (a_color+b_color+c_color+d_color);
    
    // the centroid (for wireframe rendering)
    Vec3f centroid = f->computeCentroid();
    
    AddWireFrameTriangle(current,
                         a_pos,b_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         a_color,b_color,avg_color);
    AddWireFrameTriangle(current,
                         b_pos,c_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         b_color,c_color,avg_color);
    AddWireFrameTriangle(current,
                         c_pos,d_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         c_color,d_color,avg_color);
    AddWireFrameTriangle(current,
                         d_pos,a_pos,centroid,
                         normal,normal,normal,
                         wireframe_color,
                         d_color,a_color,avg_color);

  }
}

// =======================================================================================
