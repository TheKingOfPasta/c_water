#include <float.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <assert.h>
#include <math.h>
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

void print_start_chunks(GLuint ssbo)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    uint32_t* densities = (uint32_t *)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if (!densities)
    {
        printf("Failed to map densities buffer\n");
        return;
    }

    for (int i = 0; i < c->nb_chunk_x * c->nb_chunk_y; i++)
    {
        printf("%i : %u\n", i, densities[i]);
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

    int pts_x = (int)ceil(sqrt(NB_PARTICLES));
    int pts_y = (NB_PARTICLES + pts_x - 1) / pts_x;

    float padding = 2.0f * c->radius + 1.0f;

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        float rx = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * c->radius * 1.3f;
        float ry = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * c->radius * 1.3f;

        particles[i].velo.x = 0;
        particles[i].velo.y = 0;

        particles[i].pos = (Vec2){
            .x = c->sx / 2.0f + ((i % pts_x) - pts_x / 2.0f) * padding + rx,
            .y = c->sy / 2.0f + ((int)(i / pts_y) - pts_y / 2.0f) * padding + ry
        };
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

    GLFWwindow *win = init_window();
    //glfwSwapInterval(0);

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    GLuint predicted_positions_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/predicted_positions.comp");
    GLuint chunks_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/chunks.comp");
    GLuint density_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/density.comp");
    GLuint pressure_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/pressure.comp");
    GLuint init_pairs_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_pairs.comp");
    GLuint sort_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/sort.comp");
    GLuint init_chunks_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_chunks.comp");
    GLuint viscosity_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/viscosity.comp");

    GLuint vs = compile_shader(GL_VERTEX_SHADER, "shaders/shader.vert");
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, "shaders/shader.frag");
    GLuint render_prog = create_program(vs, fs);

    c->chunk_size = c->particle_influence_radius;
    c->nb_chunk_x = c->sx / c->chunk_size + 1;
    c->nb_chunk_y = c->sy / c->chunk_size + 1;

    int n2 = next_p2(NB_PARTICLES);
    chunk_particle_idx_pair* pairs = malloc(n2 * sizeof(chunk_particle_idx_pair));
    for (int i = 0; i < n2; i++)
        pairs[i].chunk_idx = INT32_MAX;

    GLuint particles_buffer;
    glGenBuffers(1, &particles_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, particles_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * NB_PARTICLES, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_PARTICLES, particles_buffer);

    //GLuint particles_ssbo = opengl_add_array(NULL, sizeof(Particle) * NB_PARTICLES, BINDING_PARTICLES);
    GLuint pred_pos_ssbo = opengl_add_array(NULL, sizeof(Vec2) * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    GLuint densities_ssbo = opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);
    GLuint start_chunks_ssbo = opengl_add_array(NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y, BINDING_START_CHUNKS);
    GLuint pairs_ssbo = opengl_add_array(pairs, sizeof(chunk_particle_idx_pair) * n2, BINDING_PAIRS);

    init_particles(particles_buffer);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, particles_buffer);

    glBindVertexArray(vao);
    glVertexAttribPointer(
        LOCATION_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0
    );
    glEnableVertexAttribArray(LOCATION_POS);

    glVertexAttribPointer(
        LOCATION_VELO, 2, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(sizeof(Vec2))
    );
    glEnableVertexAttribArray(LOCATION_VELO);

    GLuint ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(config), c, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, BINDING_CONFIG, ubo);

    GLuint loc_passStep = glGetUniformLocation(sort_prog, "passStep");
    GLuint loc_passStage = glGetUniformLocation(sort_prog, "passStage");
    GLuint loc_next_p2 = glGetUniformLocation(sort_prog, "next_p2");

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
            init_particles(particles_buffer);
            state.reset = false;
        }

        if (!state.step_mode || state.step)
        {
            opengl_launch_program(init_chunks_prog, ubo, c->nb_chunk_x * c->nb_chunk_y);

            printf("\n");
            START_TIME(predicted_positions);
            opengl_launch_program(predicted_positions_prog, ubo, NB_PARTICLES);
            END_TIME(predicted_positions);

            START_TIME(init_pairs);
            opengl_launch_program(init_pairs_prog, ubo, NB_PARTICLES);
            END_TIME(init_pairs);

            START_TIME(sort);
            opengl_prepare_program(sort_prog, ubo);
            glUniform1i(loc_next_p2, n2);
            sort_pairs(loc_passStep, loc_passStage);
            END_TIME(sort);

            START_TIME(chunks);
            opengl_launch_program(chunks_prog, ubo, NB_PARTICLES);
            END_TIME(chunks);

            //print_pairs(pairs_ssbo);
            //printf("\n\n\n\n-----------------------------------------------------------------------------------------------------------------\n");

            //print_particles(particles_ssbo);

            START_TIME(density);
            opengl_launch_program(density_prog, ubo, NB_PARTICLES);
            END_TIME(density);
            // print_densities(densities_ssbo);
            // print_predicted_positions(pred_pos_ssbo);

            START_TIME(viscosity);
            opengl_launch_program(viscosity_prog, ubo, NB_PARTICLES);
            END_TIME(viscosity);

            START_TIME(pressure);
            opengl_launch_program(pressure_prog, ubo, NB_PARTICLES);
            END_TIME(pressure);

            state.step = false;
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        }


        double t1 = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render_prog);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, NB_PARTICLES);

        glfwSwapBuffers(win);
        glfwPollEvents();
        double t2 = glfwGetTime();

        total -= FPS[fps_i] / FPS_COUNT;

        FPS[fps_i++] = 1.0 / (t1 - t0);
        total += FPS[fps_i - 1] / FPS_COUNT;

        if (fps_i == FPS_COUNT)
            fps_i = 0;

        printf("\r%f %f                         ", total, 1.0 / (t2 - t0));

        fflush(stdout);
    }

    glfwTerminate();
}
