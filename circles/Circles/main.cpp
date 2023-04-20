#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include "point.h"

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

int main(int argc, const char* argv[])
{

    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    if (argc < 2) {
        std::cerr << "No input file given" << std::endl;
    }
    std::ifstream in_file("textured_plane_reflective_sphere.txt");

    int width;
    int height;

    in_file >> width >> height;

    std::vector<Point> points;

    float color0;
    float color1;
    float color2;
    float pos0;
    float pos1;
    int times_combined;
    while (true) {
        in_file >> color0 >> color1 >> color2 >> pos0 >> pos1 >> times_combined;
        //scale pos's
        //pos should range from -1 to 1
        pos0 = (pos0 - (width / 2)) / (width / 2);
        pos1 = (pos1 - (height / 2)) / (height / 2);
        float radius = (times_combined + 1) * (1.0f / ((width + height) / 2)) * 5;
        Point p(pos0, pos1, Vec3f(color0, color1, color2), radius);
        points.push_back(p);
        if (in_file.eof()) {
            break;
        }
    }

    window = glfwCreateWindow(width, height, "Circles", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        for (int i = 0; i < points.size(); i++) {
            Vec3f color = points[i].getColor();
            glColor3f(color.r(), color.g(), color.b());
            drawCircle(points[i].getX(), points[i].getY(), points[i].getRadius());
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}