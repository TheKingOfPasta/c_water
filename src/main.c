#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "image/image.h"
#include "image/image_drawing.h"
#include "simulation/simulation.h"

#define WIDTH 1920
#define HEIGHT 1200

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
    bool draw_densities;
    bool reset;
    double mouse_x;
    double mouse_y;
    Simulation* s;
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

    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_N)
        state->step = true;

    if (key == GLFW_KEY_P || key == GLFW_KEY_SPACE)
        state->step_mode = !state->step_mode;

    if (key == GLFW_KEY_R)
        state->reset = true;

    if (key == GLFW_KEY_D)
        state->draw_densities = !state->draw_densities;

    if (key == GLFW_KEY_C)
        simulation_print_chunks(state->s);

    if (key == GLFW_KEY_G)
        reload_config();
}

int main(void)
{
    reload_config();

    int w = WIDTH;
    int h = HEIGHT;
    RGB8 background = (RGB8){ .r = 30, .g = 20, .b = 50 };

    Image img = image_blank(w, h);
    image_fill(&img, background);

    Simulation s = simulation_gen();

    AppState state = {
        .step = false,
        .step_mode = true,
        .s = &s,
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
            simulation_free(&s);
            s = simulation_gen();
            state.s = &s;
            state.reset = false;
        }

        image_fill(&img, background);
        if (!state.step_mode || state.step)
        {
            simulation_step(&s);
            simulation_draw_balls(&s, &img);
            state.step = false;
        }

        if (state.draw_densities)
        {
            simulation_draw_density(&s, &img);
        }

        simulation_draw_chunks(&s, &img, state.mouse_x, state.mouse_y);

        glDrawPixels(w, h, GL_RGB, GL_UNSIGNED_BYTE, img.pixels);
        glfwSwapBuffers(window);

        printf("\r%s %f", state.step_mode ? "PAUSED  " : "UNPAUSED",
               1.0 / (glfwGetTime() - t0));
        fflush(stdout);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
