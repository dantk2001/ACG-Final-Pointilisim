HOMEWORK 3: RAY TRACING, RADIOSITY, & PHOTON MAPPING

NAME:  Kelly Wang



ESTIMATE OF # OF HOURS SPENT ON THIS ASSIGNMENT:  20+



COLLABORATORS AND OTHER RESOURCES: List the names of everyone you
talked to about this assignment and all of the resources (books,
online reference material, etc.) you consulted in completing this
assignment.

Provided paper "Rendering Caustics on Non-Lambertian Surfaces"

Remember: Your implementation for this assignment must be done on your
own, as described in "Academic Integrity for Homework" handout.



OPERATING SYSTEM & VERSION & GRAPHICS CARD:  Windows 11 x64, AMD Radeon(TM) Graphics



SELF GRADING TOTAL:  [ 20.25 / 20 ]


< Please insert notes on the implementation, known bugs, extra credit
in each section. >



2 PROGRESS POSTS [ 5 / 5 ] Posted on Submitty Discussion Forum on the
dates specified on the calendar.  Includes short description of
status, and at least one image.  Reasonable progress has been made.


SPHERE INTERSECTIONS, SHADOWS, & REFLECTION [ 2 / 2 ]
  ./render -size 200 200 -input reflective_spheres.obj 
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 1
  ./render -size 200 200 -input reflective_spheres.obj -num_bounces 3 -num_shadow_samples 1 


RAY TREE VISUALIZATION OF SHADOW & REFLECTIVE RAYS [ 1 / 1 ]


DISTRIBUTION RAY TRACING: SOFT SHADOWS & ANTIALIASING [ 2 / 2 ]
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 1
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 4
  ./render -size 200 200 -input textured_plane_reflective_sphere.obj -num_bounces 1 -num_shadow_samples 9 -num_antialias_samples 9

For distributed ray tracing I simply used a random sampling method, and the random sample was taken from the "randomPoint()" function provided in the Face class.
My antialiasing was accidentally implemented in the same function as shadows and reflections, but it looks like
it works perfectly fine regardless.

Screenshot (I got impatient midway through, but all the important parts are rendered): https://i.imgur.com/VVQo46F.png

EXTRA CREDIT: SAMPLING [ 0 ]
1 point for stratified sampling of pixel in image plane
1 point for stratified sampling of soft shadows
includes discussion of performance/quality


OTHER DISTRIBUTION RAY TRACING EXTRA CREDIT [ 0 ]
glossy surfaces, motion blur, or depth of field, etc.


BASIC FORM FACTOR COMPUTATION [ 2 / 2 ]
Description of method in README.txt.  
  ./render -size 200 200 -input cornell_box.obj

As more patches are added, the performance is exponentially impacted
as every patch must go through every other patch (n^2).

Factoring in visibility, the performance becomes n^3 as each patch
must go through each other patch, and for each other patch also go
through each object in the scene to determine visibility.

RADIOSITY SOLVER [ 2 / 3 ]
May be iterative (solution fades in) or done by inverting the form
factor matrix.


FORM FACTORS WITH VISIBILITY / OCCLUSION RAY CASTING [ .75 / 1 ]
  ./render -size 300 150 -input l.obj 
  ./render -size 300 150 -input l.obj -num_form_factor_samples 100
  ./render -size 300 150 -input l.obj -num_shadow_samples 1 
  ./render -size 300 150 -input l.obj -num_form_factor_samples 10 -num_shadow_samples 1 
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -sphere_rasterization 16 12 -num_shadow_samples 1


My calculation of the absorbed light is definitely wrong, and the images are brighter than the samples. There are
shadows and color bleeding, though.
Similarly with the L-hallway scene, it's brighter than the sample image but the darkness of the hallway seems
otherwise intact.

Front view of cornell-with-ball: https://i.imgur.com/9PcL5xn.png
Different angle to show shadow: https://i.imgur.com/o03fo0a.png

RADIOSITY EXTRA CREDIT [ 0 ]
1 point for ambient term in radiosity
1-2 points for new test scene or visualization
1 point for writing the ray traced image to a file
1-3 points extra credit for performance improvements
1-3 points for other ray tracing effects
1-3 points for gradient or discontinuity meshing in radiosity 


PHOTON DISTRIBUTION [ 2 / 2 ]
Shoots photons into the scene and the visualization looks reasonable
(the heart shape can be seen in the ring).
  ./render -size 200 200 -input reflective_ring.obj -num_photons_to_shoot 10000 -num_bounces 2 -num_shadow_samples 10
  ./render -size 200 200 -input reflective_ring.obj -num_photons_to_shoot 500000 -num_bounces 2 -num_shadow_samples 10 -num_antialias_samples 4


RAY TRACING WITH PHOTONS [ 1.5 / 2 ]
Searching for photons in the kdtree to use in ray tracing.  The
caustic ring is visible, and there are no significant artifacts in
illumination.

Something is definitely buggy with the way I calculated the surface area, so I have to apply a
flat scaling value to the sum in order for a ring to appear; even then it's a little fainter than
the sample image. It's a bandaid fix that also makes the 10000 photon image a little brighter than
it should be (except for apparently 1 cell in the tree, or at least that's what I assume).

500000 visulization, which looks about right: https://i.imgur.com/w81xEnB.png
500000 render, with ring visible: https://i.imgur.com/NAjZ9Vm.png


PHOTON MAPPING MATCHES RADIOSITY [ 1 ]
The intensity and color bleeding of photon mapping for indirect
illumination are correctly weighted and closely matches the results
from radiosity.  2 points extra credit.
  ./render -size 200 200 -input cornell_box_diffuse_sphere.obj -num_photons_to_shoot 500000 -num_shadow_samples 500 -num_photons_to_collect 500 

This scene did not cooperate for rendering this scene, so I apologize for the low quality. But looking at what
can be seen from the screenshot, it looks almost there?

Low quality render: https://i.imgur.com/ChX40QF.png


OTHER EXTRA CREDIT [ ?? ]
1-2 points for new test scene or visualization
1 point for writing the ray traced image to a file
1-3 points extra credit for performance improvements
2-5 points for irradiance caching

<Insert instructions for use and test cases and sample output as appropriate.>



KNOWN BUGS IN YOUR CODE
Please be concise!
