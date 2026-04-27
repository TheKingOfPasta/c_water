#include <float.h>
#include <glad/glad.h>
// glad
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config_reloader.h"
#include "opengl/headers.h"
#include "opengl/shader_simulation.h"
#include "opengl/utils.h"
#include "opengl/shader_particle.h"

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
            glUniform1ui(loc_passStep, step);
            glUniform1ui(loc_passStage, stage);

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
    shader_simulation* s;
} AppState;

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);
    state->mouse_x = xpos;
    state->mouse_y = ypos;
}

static void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    AppState* state = ((AppState*)glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_W)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){0, 0, 5});
    if (key == GLFW_KEY_S)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){0, 0, -5});
    if (key == GLFW_KEY_D)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){5, 0, 0});
    if (key == GLFW_KEY_A)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){-5, 0, 0});
    if (key == GLFW_KEY_SPACE)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){0, 5, 0});
    if (key == GLFW_KEY_LEFT_SHIFT)
        vec3_add_inplace(&state->s->cam_pos, (Vec3){0, -5, 0});

    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_N)
        state->step = true;

    if (key == GLFW_KEY_P || key == GLFW_KEY_ENTER)
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
    shader_particle* particles = (shader_particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

    int pts_x = (int)ceil(sqrt(NB_PARTICLES));
    int pts_y = (NB_PARTICLES + pts_x - 1) / pts_x;

    float padding = 2.0f * c->radius + 1.0f;

    srand(time(NULL));

    for (int i = 0; i < NB_PARTICLES; i++)
    {
        particles[i].velo.x = 0;
        particles[i].velo.y = 0;
        particles[i].velo.z = 0;

        particles[i].pos = (Vec3)
        {
            .x = c->sx * 0.5f,
            .y = c->sy * 0.5f,
            .z = c->sz * 0.5f,
        };

        /*particles[i].pos = (Vec3){
            .x = c->sx * (float)rand() / RAND_MAX,
            .y = c->sy * (float)rand() / RAND_MAX,
            .z = c->sz * (float)rand() / RAND_MAX,
        };*/
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

#define PRINT_SSBO(ssbo, type, nb_elts, print_func)\
    do\
    {\
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);\
        type* arr = (type*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);\
        printf("%s\n", #ssbo);\
        for (int i = 0; i < nb_elts; i++)\
        {\
            print_func(arr, i);\
        }\
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);\
    }\
    while (0)

void print_particle(shader_particle* p, int index)
{
    p = p + index;
    printf("                                 %f %f %f += %f %f %f\n", p->pos.x, p->pos.y, p->pos.z, p->velo.x, p->velo.y, p->velo.z);
    if (!isnormal(p->pos.x) || !isnormal(p->pos.y) || !isnormal(p->pos.z) || !isnormal(p->velo.x) || !isnormal(p->velo.y) || !isnormal(p->velo.z))
    if (p->pos.x != 0 && p->pos.y != 0 && p->pos.z != 0 && p->velo.x != 0 && p->velo.y != 0 && p->velo.z != 0)
    {
    }
}

void print_pair(chunk_particle_idx_pair* arr, int index)
{
    chunk_particle_idx_pair* p = arr + index;
    printf("%i : chunk_idx =  %u - particle_idx = %u\n", index, p->chunk_idx, p->particle_idx);
}

void print_chunk(uint32_t* c, int index)
{
    if (c[index] != -1u)
        printf("%i : %i\n", index, c[index]);
}

void print_float(float *arr, int index)
{
    printf("%i : %f\n", index, arr[index]);
}

int main()
{
    srand(time(NULL));

    reload_config();
    s = calloc(1, sizeof(shader_simulation));

    AppState state = {
        .step = false,
        .step_mode = true,
        .s = s,
    };

    s->cam_pos = (Vec3){
        .x = c->sx / 2,
        .y = c->sy / 2,
        .z = -30,
    };

    GLFWwindow* win = init_window();
    glfwSwapInterval(0);

    glfwSetWindowUserPointer(win, &state);
    glfwSetKeyCallback(win, key_callback);
    glfwSetCursorPosCallback(win, cursor_callback);

    GLuint init_particles_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_particles.comp");
    GLuint predicted_positions_prog =
        compile_shader(GL_COMPUTE_SHADER, "shaders/predicted_positions.comp");
    GLuint chunks_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/chunks.comp");
    GLuint density_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/density.comp");
    GLuint pressure_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/pressure.comp");
    GLuint init_pairs_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_pairs.comp");
    GLuint sort_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/sort.comp");
    GLuint init_chunks_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_chunks.comp");
    GLuint viscosity_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/viscosity.comp");
    GLuint update_pos_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/update_pos.comp");

    GLuint vs = compile_shader(GL_VERTEX_SHADER, "shaders/shader.vert");
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, "shaders/shader.frag");
    GLuint render_prog = create_program(vs, fs);

    c->chunk_size = c->particle_influence_radius;
    c->nb_chunk_x = c->sx / c->chunk_size + 1;
    c->nb_chunk_y = c->sy / c->chunk_size + 1;
    c->nb_chunk_z = c->sz / c->chunk_size + 1;

    unsigned int n2 = next_p2(NB_PARTICLES);
    chunk_particle_idx_pair* pairs = malloc(n2 * sizeof(chunk_particle_idx_pair));
    for (unsigned int i = 0; i < n2; i++)
        pairs[i].chunk_idx = INT32_MAX;

    /*GLuint particles_buffer;
    glGenBuffers(1, &particles_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, particles_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shader_particle) * NB_PARTICLES, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_PARTICLES, particles_buffer);*/

    GLuint particles_ssbo = opengl_add_array(NULL, sizeof(shader_particle) * NB_PARTICLES, BINDING_PARTICLES);
    GLuint pred_pos_ssbo =
        opengl_add_array(NULL, sizeof(Vec3) * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    GLuint densities_ssbo = opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);
    GLuint start_chunks_ssbo = opengl_add_array(
        NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z, BINDING_START_CHUNKS);
    GLuint pairs_ssbo =
        opengl_add_array(pairs, sizeof(chunk_particle_idx_pair) * n2, BINDING_PAIRS);

    //init_particles(particles_ssbo);

    float vertices[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f
    };

    unsigned int quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(LOCATION_POS);
    glVertexAttribPointer(LOCATION_POS, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    bind_uniform_buffer(&s->config_ubo, BINDING_CONFIG, c, sizeof(config));
    bind_uniform_buffer(&s->simulation_ubo, BINDING_SIMULATION, s, sizeof(shader_simulation));

    GLuint loc_passStep = glGetUniformLocation(sort_prog, "passStep");
    GLuint loc_passStage = glGetUniformLocation(sort_prog, "passStage");
    GLuint loc_next_p2 = glGetUniformLocation(sort_prog, "next_p2");
    GLuint loc_init_pairs_n2 = glGetUniformLocation(init_pairs_prog, "N2");

    GLuint loc_NB_PARTICLES_render = glGetUniformLocation(render_prog, "NB_PARTICLES");
    GLuint loc_cam_pos = glGetUniformLocation(render_prog, "cam_pos");

#define FPS_COUNT 100

    double total = 0.0;

    double FPS[FPS_COUNT] = { 0 };
    size_t fps_i = 0;

    opengl_launch_program(init_particles_prog, s, NB_PARTICLES);

    opengl_launch_program(predicted_positions_prog, s, NB_PARTICLES);

    double last_t = glfwGetTime();

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();

        s->dt = (float)(t0 - last_t);
        last_t = t0;

        if (state.reset)
        {
            reload_config();
            opengl_launch_program(init_particles_prog, s, NB_PARTICLES);
            state.reset = false;
        }

        START_TIME(init_chunks);
        opengl_launch_program(init_chunks_prog, s, c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z);
        END_TIME(init_chunks);

        if (!state.step_mode || state.step)
        {
            START_TIME(predicted_positions);
            opengl_launch_program(predicted_positions_prog, s, NB_PARTICLES);
            END_TIME(predicted_positions);
        }

        START_TIME(init_pairs);
        opengl_prepare_program(init_pairs_prog, s);
        glUniform1ui(loc_init_pairs_n2, n2);
        opengl_launch_last_prepared_program(NB_PARTICLES);
        END_TIME(init_pairs);

        //PRINT_SSBO(pairs_ssbo, chunk_particle_idx_pair, NB_PARTICLES, print_pair);

        START_TIME(sort);
        opengl_prepare_program(sort_prog, s);
        glUniform1ui(loc_next_p2, n2);
        sort_pairs(loc_passStep, loc_passStage);
        END_TIME(sort);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, pairs_ssbo);
        chunk_particle_idx_pair* arr = (chunk_particle_idx_pair*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        for (int i = 0; i < NB_PARTICLES; i++)
        for (int j = i + 1; j < NB_PARTICLES; j++)
        {
            if (arr[i].chunk_idx > arr[j].chunk_idx)
            {
                chunk_particle_idx_pair tmp = arr[i];
                arr[i] = arr[j];
                arr[j] = tmp;
            }
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        //PRINT_SSBO(pairs_ssbo, chunk_particle_idx_pair, NB_PARTICLES, print_pair);

        START_TIME(chunks);
        opengl_launch_program(chunks_prog, s, NB_PARTICLES);
        END_TIME(chunks);

        if (!state.step_mode || state.step)
        {
            //PRINT_SSBO(start_chunks_ssbo, uint32_t, c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z, print_chunk);

            // printf("\npairs chunkidx - particleidx : \n");
            // print_pairs(pairs_ssbo);
            // print_start_chunks(start_chunks_ssbo);

            // print_particles(particles_ssbo);

            START_TIME(density);
            opengl_launch_program(density_prog, s, NB_PARTICLES);
            END_TIME(density);
            // print_densities(densities_ssbo);
            // print_predicted_positions(pred_pos_ssbo);

            START_TIME(pressure);
            opengl_launch_program(pressure_prog, s, NB_PARTICLES);
            END_TIME(pressure);

            START_TIME(viscosity);
            opengl_launch_program(viscosity_prog, s, NB_PARTICLES);
            END_TIME(viscosity);

            START_TIME(update_pos);
            opengl_launch_program(update_pos_prog, s, NB_PARTICLES);
            END_TIME(update_pos);

            //PRINT_SSBO(particles_ssbo, shader_particle, NB_PARTICLES, print_particle);

            state.step = false;
            PRINT_SSBO(densities_ssbo, float, NB_PARTICLES, print_float);

            PRINT_SSBO(start_chunks_ssbo, uint32_t, (int)(c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z), print_chunk);
            PRINT_SSBO(pairs_ssbo, chunk_particle_idx_pair, NB_PARTICLES, print_pair);
            PRINT_SSBO(particles_ssbo, shader_particle, NB_PARTICLES, print_particle);
        }

        double t1 = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(render_prog);
        glUniform1ui(loc_NB_PARTICLES_render, NB_PARTICLES);
        glBindBuffer(GL_UNIFORM_BUFFER, s->config_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(config), c);
        glBindBuffer(GL_UNIFORM_BUFFER, s->simulation_ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(shader_simulation), s);
        glUniform3f(loc_cam_pos, s->cam_pos.x, s->cam_pos.y, s->cam_pos.z);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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
