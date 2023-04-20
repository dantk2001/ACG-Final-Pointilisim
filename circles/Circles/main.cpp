#include <GLFW\glfw3.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include "vectors.h"

void drawCircle(float x, float y, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);
    for (int i = 0; i <= 360; i++)
    {
        float angle = 2 * M_PI * i / 360;
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();
}

int circles_main(int argc, const char* argv[])
{

    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    if (argc < 2) {
        std::cerr << "No input file given" << std::endl;
    }
    std::ifstream in_file(argv[1] + '.txt');
    std::string delimiter = ":";

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
        Vec3f color;
        Vec3f position;
        int times_combined;
        while (true) {
            in_file >> color >> position >> times_combined;
            glColor3f(color[0], color[1], color[2]);
            drawCircle(position[0], position[1], times_combined * 0.05f);
            if (in_file.eof()) {
                break;
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}