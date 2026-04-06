#include <float.h>
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

void print_densities(GLuint densities_ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, densities_ssbo);
    float *densities = (float *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if (!densities)
    {
        printf("Failed to map densities buffer\n");
        return;
    }

    for (int i = 0; i < NB_PARTICLES; i++)
        printf("density[%d] = %f\n", i, densities[i]);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void print_predicted_positions(GLuint ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    Vec2* densities = (Vec2 *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if (!densities)
    {
        printf("Failed to map densities buffer\n");
        return;
    }

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        printf("pred_pos[%i] = ", i);
        vec2_print(densities + i);
        printf("\n");
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void print_particles(GLuint ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    Particle* densities = (Particle *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if (!densities)
    {
        printf("Failed to map densities buffer\n");
        return;
    }

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        printf("pred_pos[%i] = ", i);
        vec2_print(&densities[i].pos);
        printf("\n");
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void print_pairs(GLuint ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    chunk_particle_idx_pair* densities = (chunk_particle_idx_pair*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if (!densities)
    {
        printf("Failed to map densities buffer\n");
        return;
    }

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        printf("%i %i\n", densities[i].chunk_idx, densities[i].particle_idx);
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

int next_p2(int n)
{
    int res = 1;
    while (res < n)
        res *= 2;

    return res;
}

void sort_pairs(GLuint loc_passStep, GLuint loc_passStage)
{
    int n = next_p2(NB_PARTICLES);
    int numThreads = n / 2;

    for (int stage = 2; stage <= n; stage <<= 1)
    {
        for (int step = stage; step >= 2; step >>= 1)
        {
            glUniform1i(loc_passStep,  step);
            glUniform1i(loc_passStage, stage);

            int groups = (numThreads + 255) / 256;
            glDispatchCompute(groups, 1, 1);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }
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

void init_particles(GLuint particles_ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles_ssbo);
    Particle* particles = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

    for(int i = 0; i < NB_PARTICLES; i++)
    {
        particles[i].pos.x = 1920.0 * (float)rand() / RAND_MAX;
        particles[i].pos.y = 1080.0 * (float)rand() / RAND_MAX;
        particles[i].velo.x = 0;
        particles[i].velo.y = 0;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

static inline int pair_sort(const void* p1, const void* p2)
{
    uint16_t a = ((chunk_particle_idx_pair*)p1)->chunk_idx;
    uint16_t b = ((chunk_particle_idx_pair*)p2)->chunk_idx;
    return a - b;
}

int main()
{
    AppState state = {
        .step = false,
        .step_mode = true,
    };

    srand(time(NULL));

    reload_config();

    const char *predicted_positions_src = read_shader("shaders/predicted_positions.comp");
    const char *chunks_src = read_shader("shaders/chunks.comp");
    const char *density_src = read_shader("shaders/density.comp");
    const char *compute_src = read_shader("shaders/shader.comp");
    const char *init_pairs_src = read_shader("shaders/init_pairs.comp");
    const char *sort_src = read_shader("shaders/sort.comp");

    const char *frag_src = read_shader("shaders/shader.frag");
    const char *vert_src = read_shader("shaders/shader.vert");

    GLFWwindow *win = init_window();

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    GLuint predicted_positions_prog = compile_shader(GL_COMPUTE_SHADER, predicted_positions_src);
    GLuint chunks_prog = compile_shader(GL_COMPUTE_SHADER, chunks_src);
    GLuint density_prog = compile_shader(GL_COMPUTE_SHADER, density_src);
    GLuint compute_prog = compile_shader(GL_COMPUTE_SHADER, compute_src);
    GLuint init_pairs_prog = compile_shader(GL_COMPUTE_SHADER, init_pairs_src);
    GLuint sort_prog = compile_shader(GL_COMPUTE_SHADER, sort_src);

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint render_prog = create_program(vs, fs);

    c->chunk_size = c->radius * CHUNK_SIZE_SCALE_COMPARED_TO_PARTICLE_RADIUS;
    c->nb_chunk_x = c->sx / c->chunk_size + 1;
    c->nb_chunk_y = c->sy / c->chunk_size + 1;

    int n2 = next_p2(NB_PARTICLES);
    chunk_particle_idx_pair* pairs = malloc(n2 * sizeof(chunk_particle_idx_pair));
    for (int i = 0; i < n2; i++)
        pairs[i].chunk_idx = INT32_MAX;

    GLuint particles_ssbo = opengl_add_array(NULL, sizeof(Particle) * NB_PARTICLES, BINDING_PARTICLES);
    GLuint pred_pos_ssbo = opengl_add_array(NULL, sizeof(Vec2) * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    GLuint densities_ssbo = opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);
    GLuint start_chunks_ssbo = opengl_add_array(NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y, BINDING_START_CHUNKS);
    GLuint end_chunks_ssbo = opengl_add_array(NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y, BINDING_END_CHUNKS);
    GLuint pairs_ssbo = opengl_add_array(pairs, sizeof(chunk_particle_idx_pair) * n2, BINDING_PAIRS);

    init_particles(particles_ssbo);

    GLuint vao;
    glGenVertexArrays(1,&vao);
    glBindVertexArray(vao);

    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(config), c, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_CONFIG, ubo);

    GLuint loc_passStep = glGetUniformLocation(sort_prog, "passStep");
    GLuint loc_passStage = glGetUniformLocation(sort_prog, "passStage");

    glEnable(GL_PROGRAM_POINT_SIZE);

    #define FPS_COUNT 100

    double total = 0.0;

    double FPS[FPS_COUNT] = { 0 };
    size_t fps_i = 0;

    while(!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();

        if (state.reset)
        {
            reload_config();
            init_particles(particles_ssbo);
            state.reset = false;
        }

        if (!state.step_mode || state.step)
        {
            opengl_launch_program(predicted_positions_prog, ubo, NB_PARTICLES);

            opengl_launch_program(init_pairs_prog, ubo, NB_PARTICLES);

            opengl_prepare_program(sort_prog, ubo);
            sort_pairs(loc_passStep, loc_passStage);

            opengl_launch_program(chunks_prog, ubo, 1);
            //print_pairs(pairs_ssbo);
            //printf("\n\n\n\n-----------------------------------------------------------------------------------------------------------------\n");

            //print_particles(particles_ssbo);

            opengl_launch_program(density_prog, ubo, NB_PARTICLES);
            // print_densities(densities_ssbo);
            // print_predicted_positions(pred_pos_ssbo);

            opengl_launch_program(compute_prog, ubo, NB_PARTICLES);

            state.step = false;
        }



        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(render_prog);
        glDrawArrays(GL_POINTS,0,NB_PARTICLES);

        glfwSwapBuffers(win);
        glfwPollEvents();

        double t1 = glfwGetTime();

        total -= FPS[fps_i] / FPS_COUNT;

        FPS[fps_i++] = 1.0 / (t1 - t0);
        total += FPS[fps_i - 1] / FPS_COUNT;

        if (fps_i == FPS_COUNT)
            fps_i = 0;

        printf("\r%f           ", total);

        fflush(stdout);
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
