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

#define PI 3.14159265358979323846

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

std::set<int> getNeighbors(int row, int col, int ROWS, int COLS) {
    std::set<int> n;
    //up, down, left, right
    //diagonals
    // Find the indices of the current element's neighbors
    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = col - 1; j <= col + 1; j++) {
            // Ignore the current element itself and out-of-bounds indices
            if (i >= 0 && i < ROWS && j >= 0 && j < COLS && !(i == row && j == col)) {
                n.insert((i * ROWS) + j);
            }
        }
    }
    return n;
}

void cleanUpCombination(std::vector<Point>* circles, int id) {
    //subtract 1 from all index's after circle removed
    //remove neighbor from everything
    for (int i = 0; i < circles->size(); i++) {
        if (i > id) {
            (*circles)[i].updateId();
        }
        (*circles)[i].removeNeighbor(id);
    }
    //this is broken, figure out how to repopulate neighbors correctly later
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

    //position(p),color(c),id(id_),times_combined(t),neighbors(n)
    std::vector<Point> circles;
    //set neighbors aswell
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            Vec3f color ((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
            Vec3f position (-0.80f + j * 0.20f, 0.80f - i * 0.20f, 0.1f);
            Point p(position, color, (4 * i) + j, 0, getNeighbors(i, j, 8, 8));
            circles.push_back(p);
        }
    }

    //combination algorithm
    std::vector<int> deleteList;
    for (int i = 0; i < circles.size(); i++) {
        if (circles[i].getTimesCombined() == 0 && std::count(deleteList.begin(), deleteList.end(), i) == 0) {
            std::set<int> n = circles[i].getNeighbors();
            std::set<int>::iterator itr;
            std::pair<int, float> closest = { i, 2.0f };
            for (itr = n.begin(); itr != n.end(); itr++) {
                float dist = abs(DistanceBetweenTwoPoints(circles[i].getColor(), circles[*itr].getColor()));
                //if (itr == n.begin()) {
                //    std::cout << dist << std::endl;
                //}
                if (dist < closest.second && dist <= threshold) {
                    //std::cout << "new closest" << std::endl;
                    closest = std::make_pair(*itr, dist);
                }
            }
            //found one within threshold
            if (closest.first != i) {
                std::cout << "combining : " << i << " " << closest.first << std::endl;
                Vec3f avgColor = (circles[i].getColor() + circles[closest.second].getColor());
                avgColor /= 2.0f;
                Vec3f avgPosition = (circles[i].getPosition() + circles[closest.second].getPosition());
                avgPosition /= 2.0f;
                circles[i] = Point(avgPosition, avgColor, i, 1, getNeighbors(i / 8, i % 8, 8, 8));
                //remove the one being absorbed from all neighbors lists and subtract one from all
                //after the removed index
                cleanUpCombination(&circles, closest.first);
                deleteList.push_back(closest.first);
            }
        }
    }

    //remove all circles in delete list
    for (int i = 0; i < deleteList.size(); i++) {
        //delete cirlce
        circles.erase(circles.begin() + (deleteList[i] - 1 - i));
    }

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