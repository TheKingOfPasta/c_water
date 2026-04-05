#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "config_reloader.h"
#include "utils/utils.h"
#include "simulation/simulation.h"
#include "opengl/utils.h"
#include "opengl/headers.h"

GLuint create_compute_program(GLuint s)
{
    GLuint p = glCreateProgram();
    glAttachShader(p,s);
    glLinkProgram(p);

    return p;
}

GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,NULL);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){
        char log[1024];
        glGetShaderInfoLog(s,1024,NULL,log);
        printf("shader error:\n%s\n",log);
        exit(1);
    }

    if (type == GL_COMPUTE_SHADER)
        return create_compute_program(s);

    return s;
}

GLuint create_program(GLuint s1, GLuint s2)
{
    GLuint p = glCreateProgram();
    glAttachShader(p,s1);
    glAttachShader(p,s2);
    glLinkProgram(p);

    return p;
}

char *read_shader(char *file)
{
    char *version = "#version 430 core\n\n";

    char *bindings = read_all_file("src/opengl/headers.h");
    char *f = read_all_file(file);
    char *config = read_all_file("src/opengl/headers.glsl");

    size_t cpy_i = 0;
    size_t f_i = 0;
    size_t len = strlen(f);
    size_t cpy_len = len;
    char *f_cpy = calloc(len + 1, sizeof(char));

    while (f_i < len)
    {
        if (strncmp(f + f_i, "#include \"", sizeof("#include \"") - 1) == 0)
        {
            f_i += sizeof("#include \"") - 1;

            size_t file_name_size = 0;
            while (f[f_i + file_name_size] != '"')
                file_name_size += 1;

            char *file_name = calloc(file_name_size + 1, sizeof(char));
            file_name = strncpy(file_name, f + f_i, file_name_size);
            char *replace_with = read_all_file(file_name);
            free(file_name);

            cpy_len += strlen(replace_with);
            f_cpy = realloc(f_cpy, cpy_len);

            f_cpy = strcat(f_cpy, replace_with);
            cpy_i += -sizeof("#include \"") - file_name_size + 31 + strlen(replace_with);
            f_i += file_name_size;
            free(replace_with);
        }
        else
            f_cpy[cpy_i] = f[f_i];

        f_i += 1;
        cpy_i += 1;
    }

    char* res = calloc(strlen(version) + strlen(bindings) + strlen(f_cpy) + strlen(config) + 1, sizeof(char));
    res = strcat(res, version);
    res = strcat(res, bindings);
    res = strcat(res, config);
    res = strcat(res, "\n");
    res = strcat(res, f_cpy);

    free(bindings);
    free(f);
    free(f_cpy);
    free(config);

    return res;
}

int main()
{
    srand(time(NULL));

    if(!glfwInit())
        return 1;

    reload_config();

    const char *predicted_positions_src = read_shader("shaders/predicted_positions.comp");
    const char *compute_src = read_shader("shaders/shader.comp");
    const char *density_src = read_shader("shaders/density.comp");
    const char *frag_src = read_shader("shaders/shader.frag");
    const char *vert_src = read_shader("shaders/shader.vert");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(c->sx, c->sy, "C Water", NULL, NULL);

    glfwMakeContextCurrent(win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return 1;

    GLuint predicted_positions_prog = compile_shader(GL_COMPUTE_SHADER, predicted_positions_src);
    GLuint density_prog = compile_shader(GL_COMPUTE_SHADER, density_src);

    GLuint compute_prog = compile_shader(GL_COMPUTE_SHADER, compute_src);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint render_prog = create_program(vs, fs);

    Particle particles[NB_PARTICLES];

    for(int i = 0; i < NB_PARTICLES; i++)
    {
        particles[i].pos.x = 1920.0 * (float)rand() / RAND_MAX;
        particles[i].pos.y = 1080.0 * (float)rand() / RAND_MAX;
        particles[i].velo.x = 0;
        particles[i].velo.y = 0;
    }

    opengl_add_array(particles, sizeof(Particle) * NB_PARTICLES, BINDING_PARTICLES);
    opengl_add_array(NULL, sizeof(Vec2) * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);

    GLuint vao;
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    GLuint ubo;// Uniform buffer object <=> pass struct to shaders
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(config), c, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

    glEnable(GL_PROGRAM_POINT_SIZE);

    float dt = 0.016f;

    while(!glfwWindowShouldClose(win))
    {
        opengl_launch_program(predicted_positions_prog);
        opengl_launch_program(density_prog);

        opengl_prepare_program(compute_prog);

        glUniform1f(glGetUniformLocation(compute_prog,"dt"),dt);

        opengl_launch_last_prepared_program();



        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(render_prog);
        glDrawArrays(GL_POINTS,0,NB_PARTICLES);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwTerminate();
}

/*#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include "config_reloader.h"
#include "image/image.h"
#include "image/image_drawing.h"
#include "simulation/simulation.h"
#include "utils/colorRGB8.h"

static void error_callback([[maybe_unused]] int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

typedef struct AppState
{
    bool step;
    bool step_mode;
    bool draw_densities;
    bool draw_chunks;
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

    if (key == GLFW_KEY_G)
        state->draw_chunks = !state->draw_chunks;
    // simulation_print_chunks(state->s);

    if (key == GLFW_KEY_C)
        reload_config();
}

float my_float_lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

int main(void)
{
    reload_config();

    RGB8 background = rgb8_black();

    Image img = image_blank(c->sx, c->sy);
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

    GLFWwindow* window =
        glfwCreateWindow(img.sx, img.sy, "C Water", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, &state);
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
            reload_config();
            s = simulation_gen();
            state.s = &s;
            state.reset = false;
        }

        image_fill(&img, background);
        if (!state.step_mode || state.step)
        {
            simulation_step(&s);
            state.step = false;
        }

        if (state.draw_densities)
            simulation_draw_density(&s, &img);
        if (state.draw_chunks)
            simulation_draw_chunks(&s, &img, state.mouse_x, state.mouse_y);

        simulation_draw_balls(&s, &img);

        glDrawPixels(c->sx, c->sy, GL_RGB, GL_UNSIGNED_BYTE, img.pixels);
        glfwSwapBuffers(window);

        printf("\r%s %f           ", state.step_mode ? "PAUSED  " : "UNPAUSED", 1.0 / (glfwGetTime() - t0));
        fflush(stdout);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
*/
