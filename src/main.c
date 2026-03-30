#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "image_drawing.h"
#include "simulation.h"

#define WIDTH 500
#define HEIGHT 500

static void error_callback([[maybe_unused]] int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void framebuffer_size_callback(__attribute_maybe_unused__ GLFWwindow* window,
                               int width, int height)
{
    glViewport(0, 0, width, height);
}

typedef struct AppState
{
    bool step;
    bool step_mode;
    bool reset;
    double mouse_x;
    double mouse_y;
} AppState;

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    state->mouse_x = xpos;
    state->mouse_y = ypos;
}

static void key_callback(GLFWwindow* window, int key,
                         [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    AppState* state = ((AppState*)glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_N && action == GLFW_PRESS)
        state->step = true;

    if ((key == GLFW_KEY_P || key == GLFW_KEY_SPACE) && action == GLFW_PRESS)
        state->step_mode = !state->step_mode;

    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        state->reset = true;
}

int main(void)
{
    int w = WIDTH;
    int h = HEIGHT;
    RGB8 background = (RGB8){ .r = 30, .g = 20, .b = 50 };

    Image img = image_blank(w, h);
    image_fill(&img, background);

    Simulation s = simulation_gen(w, h);

    AppState state = {
        .step = true,
        .step_mode = false,
    };

    glfwInit();

    glfwSetErrorCallback(error_callback);

    // // to use glDrawPixels, it was deprecated, we maybe use a texture that
    // 									we put on a quad in front of the cam
    //    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(w, h, "C Water", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, &state);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);

    glPixelZoom(1, -1);
    glRasterPos2f(-1, 1);

    while (!glfwWindowShouldClose(window))
    {
        double t0 = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();

        if (state.reset)
        {
            s = simulation_gen(w, h);
            state.reset = false;
        }

        if (!state.step_mode || state.step)
        {
            image_fill(&img, background);
            simulation_step(&s);
            simulation_draw_field(&s, &img);
            simulation_draw_balls(&s, &img);
            // simulation_draw_field_arrow(&s, &img);
            state.step = false;
        }

        for (size_t i = 0; i < NB_PARTICULES; i++)
        {
            Particule p = s.particules[i];
            Vec2 v = simulation_compute_gradient(&s, p.pos.x, p.pos.y);
            v = vec2_mul_scalar(v, 500);
            vec2_add_inplace(&v, p.pos);
            image_draw_vector(&img, p.pos.x, p.pos.y, v.x, v.y, rgb8_red());
        }

        //simulation_draw_mouse_gradient(&s, &img, padding, (int)state.mouse_x, (int)state.mouse_y);
        glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, img.pixels);
        glfwSwapBuffers(window);

        // printf("\r%f", 1.0 / (glfwGetTime() - t0));
        fflush(stdout);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
