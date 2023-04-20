#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>

#define M_PI 3.14159265358979323846

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
        float color0;
        float color1;
        float color2;
        float pos0;
        float pos1;
        int times_combined;
        while (true) {
            in_file >> color0 >> color1 >> color2 >> pos0 >> pos1 >> times_combined;
            glColor3f(color0, color1, color2);
            drawCircle(pos0, pos1, times_combined * 0.05f);
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