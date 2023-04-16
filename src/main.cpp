#include "argparser.h"
#include "meshdata.h"


// =========================================================
// OS specific rendering for OpenGL and Apple Metal
// =========================================================
#if __APPLE__
extern "C" {
int NSApplicationMain(int argc, const char * argv[]);
}
#else
#include "OpenGLRenderer.h"
#endif


// The one significant global variable
MeshData *mesh_data;



int main(int argc, const char * argv[]) {

  
  // parse the command line arguments and initialize the MeshData
  MeshData mymesh_data;
  mesh_data = &mymesh_data;
  ArgParser args(argc, argv, mesh_data);


  // launch the OS specific renderer
#if __APPLE__
  return NSApplicationMain(argc, argv);
#else
  OpenGLRenderer opengl_renderer(mesh_data,&args);
#endif
}

/*
#include <GLFW/glfw3.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <list>
#include "point.h"
#include "utils.h"

void drawCircle(float x, float y, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++)
    {
        float angle = 2 * PI * i / 360;
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

int main(int argc, const char* argv[])
{
    //1.732 is max dist
    float threshold = atof(argv[1]);
    std::cout << threshold << std::endl;
    bool combine = true;

    if (threshold == 0) {
        combine = false;
    }

    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    srand(time(NULL)); // seed random number generator

    window = glfwCreateWindow(640, 480, "Circles", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        for (int i = 0; i < circles.size(); i++)
        {
            Vec3f color = circles[i].getColor();
            Vec3f position = circles[i].getPosition();
            glColor3f(color[0], color[1], color[2]);
            drawCircle(position[0], position[1], position[2]);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
*/