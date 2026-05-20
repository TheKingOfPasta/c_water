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
#include <time.h>

#include "config_reloader.h"
#include "opengl/headers.h"
#include "opengl/shader_particle.h"
#include "opengl/shader_simulation.h"
#include "opengl/utils.h"

int next_p2(int n)
{
    int res = 1;
    while (res < n)
        res *= 2;

    return res;
}

void bitonic_sort_pairs(GLuint sort_prog, shader_simulation* s, int n2)
{
    int loc_step = glGetUniformLocation(sort_prog, "passStep");
    int loc_stage = glGetUniformLocation(sort_prog, "passStage");
    int loc_n2 = glGetUniformLocation(sort_prog, "next_p2");
    int groups = (n2 / 2 + 255) / 256;

    opengl_prepare_program(sort_prog, s);
    glUniform1ui(loc_n2, n2);

    for (int k = 2; k <= n2; k *= 2)
    {
        for (int j = k; j >= 2; j /= 2)
        {
            glUniform1i(loc_step, j);
            glUniform1i(loc_stage, k);
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
    bool draw_particles;
    bool reset;
    bool mouse_initialized;
    double mouse_x;
    double mouse_y;
    shader_simulation* s;
    int var_index;
} AppState;

#define CAM_PITCH_LIMIT 1.55334f

static void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* state = (AppState*)glfwGetWindowUserPointer(window);

    if (!state->mouse_initialized)
    {
        // first move is not jank
        state->mouse_x = xpos;
        state->mouse_y = ypos;
        state->mouse_initialized = true;
        return;
    }

    double dx = xpos - state->mouse_x;
    double dy = ypos - state->mouse_y;

    state->s->cam_yaw += (float)dx * c->cam_mouse_sensitivity;
    state->s->cam_pitch += (float)dy * c->cam_mouse_sensitivity;

    if (state->s->cam_pitch > CAM_PITCH_LIMIT)
        state->s->cam_pitch = CAM_PITCH_LIMIT;
    if (state->s->cam_pitch < -CAM_PITCH_LIMIT)
        state->s->cam_pitch = -CAM_PITCH_LIMIT;

    state->mouse_x = xpos;
    state->mouse_y = ypos;
}

static void key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action,
                         [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    AppState* state = ((AppState*)glfwGetWindowUserPointer(window));

    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_N)
        state->step = true;

    if (key == GLFW_KEY_P || key == GLFW_KEY_ENTER)
        state->step_mode = !state->step_mode;

    if (key == GLFW_KEY_R)
        state->reset = true;

    if (key == GLFW_KEY_G)
        state->draw_chunks = !state->draw_chunks;

    if (key == GLFW_KEY_F)
        state->draw_particles = !state->draw_particles;

    if (key == GLFW_KEY_C)
        reload_config();

    if (key == GLFW_KEY_RIGHT && state->var_index < 7)
        state->var_index += 1;
    if (key == GLFW_KEY_LEFT && state->var_index > 0)
        state->var_index -= 1;
}

static void update_camera(GLFWwindow* window, AppState* state, float dt)
{
    float cos_pitch = cosf(state->s->cam_pitch);
    float sin_pitch = sinf(state->s->cam_pitch);
    float cos_yaw = cosf(-state->s->cam_yaw);
    float sin_yaw = sinf(-state->s->cam_yaw);

    float step = c->cam_move_speed * dt;

    Vec3 fwd = {
        .x = -sin_yaw * cos_pitch * step,
        .y = -sin_pitch * step,
        .z = cos_yaw * cos_pitch * step,
    };

    Vec3 right = {
        .x = cos_yaw * step,
        .y = 0,
        .z = sin_yaw * step,
    };

    Vec3 up = {
        .x = 0,
        .y = step,
        .z = 0,
    };

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        vec3_add_inplace(&state->s->cam_pos, fwd);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        vec3_sub_inplace(&state->s->cam_pos, fwd);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        vec3_add_inplace(&state->s->cam_pos, right);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        vec3_sub_inplace(&state->s->cam_pos, right);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        vec3_add_inplace(&state->s->cam_pos, up);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        vec3_sub_inplace(&state->s->cam_pos, up);

    if (glfwGetKey(window, GLFW_KEY_UP))
    {
        float* var = &(c->pressure_force) + state->var_index;
        *var *= 1 + 0.1 * s->dt;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN))
    {
        float* var = &(c->pressure_force) + state->var_index;
        *var *= 1 - 0.1 * s->dt;
    }
}

#define PRINT_SSBO(ssbo, type, nb_elts, print_func)                                                \
    do                                                                                             \
    {                                                                                              \
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);                                              \
        type* arr = (type*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);                    \
        printf("%s\n", #ssbo);                                                                     \
        for (int i = 0; i < nb_elts; i++)                                                          \
        {                                                                                          \
            print_func(arr, i);                                                                    \
        }                                                                                          \
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);                                                   \
    } while (0)

void print_particle(shader_particle* p, int index)
{
    p = p + index;
    printf("arr[%i] = %f %f %f += %f %f %f\n", index, p->pos.x, p->pos.y, p->pos.z, p->velo.x,
           p->velo.y, p->velo.z);
    if (!isnormal(p->pos.x) || !isnormal(p->pos.y) || !isnormal(p->pos.z) || !isnormal(p->velo.x)
        || !isnormal(p->velo.y) || !isnormal(p->velo.z))
        if (p->pos.x != 0 && p->pos.y != 0 && p->pos.z != 0 && p->velo.x != 0 && p->velo.y != 0
            && p->velo.z != 0)
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

void print_float(float* arr, int index)
{
    printf("%i : %f\n", index, arr[index]);
}

void print_vec3(Vec3* arr, int index)
{
    printf("arr[%i] = ", index);
    vec3_print(arr + index);
    printf("\n");
}

int main()
{
    srand(time(NULL));

    reload_config();
    s = calloc(1, sizeof(shader_simulation));

    AppState state = {
        .step = false,
        .step_mode = true,
        .mouse_initialized = false,
        .s = s,
        .var_index = 0,
    };

    s->cam_pos = (Vec3){
        .x = c->sx / 2,
        .y = c->sy / 2,
        .z = 0,
    };

    GLFWwindow* win = init_window();
    glfwSwapInterval(0);

    uint32_t win_w = c->static_config.screen_width;
    uint32_t win_h = c->static_config.screen_height;
    uint32_t render_w = (uint32_t)((float)win_w * c->static_config.render_scale);
    uint32_t render_h = (uint32_t)((float)win_h * c->static_config.render_scale);
    if (render_w < 1)
        render_w = 1;
    if (render_h < 1)
        render_h = 1;
    c->static_config.screen_width = render_w;
    c->static_config.screen_height = render_h;

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
    GLuint bitonic_sort_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/sort.comp");
    GLuint init_chunks_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/init_chunks.comp");
    GLuint viscosity_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/viscosity.comp");
    GLuint update_pos_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/update_pos.comp");
    GLuint existence_field_prog = compile_shader(GL_COMPUTE_SHADER, "shaders/density_field.comp");

    GLuint vs = compile_shader(GL_VERTEX_SHADER, "shaders/shader.vert");
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, "shaders/shader.frag");
    GLuint render_prog = create_program(vs, fs);

    GLuint pts_vs = compile_shader(GL_VERTEX_SHADER, "shaders/particles.vert");
    GLuint pts_fs = compile_shader(GL_FRAGMENT_SHADER, "shaders/particles.frag");
    GLuint particles_prog = create_program(pts_vs, pts_fs);
    GLuint loc_NB_PARTICLES_pts = glGetUniformLocation(particles_prog, "NB_PARTICLES");

    glEnable(GL_PROGRAM_POINT_SIZE);

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

    GLuint particles_ssbo =
        opengl_add_array(NULL, sizeof(shader_particle) * NB_PARTICLES, BINDING_PARTICLES);
    GLuint pred_pos_ssbo = opengl_add_array(NULL, 16 * NB_PARTICLES, BINDING_PREDICTED_POSITIONS);
    GLuint densities_ssbo = opengl_add_array(NULL, sizeof(float) * NB_PARTICLES, BINDING_DENSITIES);
    GLuint start_chunks_ssbo =
        opengl_add_array(NULL, sizeof(uint32_t) * c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z,
                         BINDING_START_CHUNKS);
    GLuint pairs_ssbo =
        opengl_add_array(pairs, sizeof(chunk_particle_idx_pair) * n2, BINDING_PAIRS);
    GLuint existence_field_ssbo =
        opengl_add_array(NULL, sizeof(GLuint) * c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z,
                         BINDING_EXISTENCE_FIELD);

    float vertices[] = { -1.0f, -1.0f, 3.0f, -1.0f, -1.0f, 3.0f };

    unsigned int quadVAO, quadVBO;

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(LOCATION_POS);
    glVertexAttribPointer(LOCATION_POS, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

    GLuint fbo, fbo_tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &fbo_tex);
    glBindTexture(GL_TEXTURE_2D, fbo_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, render_w, render_h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    bind_uniform_buffer(&s->config_ubo, BINDING_CONFIG, c, sizeof(config));
    bind_uniform_buffer(&s->simulation_ubo, BINDING_SIMULATION, s, sizeof(shader_simulation));

    GLuint loc_init_pairs_n2 = glGetUniformLocation(init_pairs_prog, "N2");

    GLuint loc_NB_PARTICLES_render = glGetUniformLocation(render_prog, "NB_PARTICLES");

    opengl_launch_program(init_particles_prog, s, NB_PARTICLES);

    opengl_launch_program(predicted_positions_prog, s, NB_PARTICLES);

    double last_t = glfwGetTime();

    while (!glfwWindowShouldClose(win))
    {
        double t0 = glfwGetTime();
        TIMINGS_FRAME_START();

        s->dt = (float)(t0 - last_t) * c->sim_speed;
        last_t = t0;

        START_TIME(camera);
        update_camera(win, &state, s->dt);
        END_TIME(camera);

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

        // PRINT_SSBO(pairs_ssbo, chunk_particle_idx_pair, NB_PARTICLES, print_pair);

        START_TIME(sort);
        bitonic_sort_pairs(bitonic_sort_prog, s, n2);
        END_TIME(sort);

        START_TIME(chunks);
        opengl_launch_program(chunks_prog, s, NB_PARTICLES);
        END_TIME(chunks);

        if (!state.step_mode || state.step)
        {
            // PRINT_SSBO(start_chunks_ssbo, uint32_t, c->nb_chunk_x * c->nb_chunk_y *
            // c->nb_chunk_z, print_chunk);

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

            // PRINT_SSBO(particles_ssbo, shader_particle, NB_PARTICLES, print_particle);

            state.step = false;

            if (state.step_mode)
            {
                PRINT_SSBO(densities_ssbo, float, NB_PARTICLES, print_float);
                PRINT_SSBO(start_chunks_ssbo, uint32_t,
                           (int)(c->nb_chunk_x * c->nb_chunk_y * c->nb_chunk_z), print_chunk);
                PRINT_SSBO(pairs_ssbo, chunk_particle_idx_pair, NB_PARTICLES, print_pair);
                PRINT_SSBO(particles_ssbo, shader_particle, NB_PARTICLES, print_particle);
            }
        }

        START_TIME(existence_field);
        opengl_launch_program(existence_field_prog, s, NB_CHUNKS);
        END_TIME(existence_field);

        double t1 = glfwGetTime();

        START_TIME(render);
        if (state.draw_particles)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, win_w, win_h);
            glClearColor(0.04f, 0.04f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            opengl_prepare_program(particles_prog, s);
            glUniform1ui(loc_NB_PARTICLES_pts, NB_PARTICLES);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_POINTS, 0, NB_PARTICLES);

            glDisable(GL_DEPTH_TEST);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, render_w, render_h);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(render_prog);
            glUniform1ui(loc_NB_PARTICLES_render, NB_PARTICLES);
            glBindBuffer(GL_UNIFORM_BUFFER, s->config_ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(config), c);
            glBindBuffer(GL_UNIFORM_BUFFER, s->simulation_ubo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(shader_simulation), s);

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, render_w, render_h, 0, 0, win_w, win_h, GL_COLOR_BUFFER_BIT,
                              GL_LINEAR);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        END_TIME(render);

        START_TIME(swap);
        glfwSwapBuffers(win);
        glfwPollEvents();
        END_TIME(swap);

        double t2 = glfwGetTime();

        char* var_names[] = {
            "pressure_force", "target_pressure",    "particle_influence_radius",
            "radius",         "gravity_multiplier", "velocity_collision_dampner",
            "velocity_drag",  "viscosity_strength",
        };

        PRINT_TIMINGS(1.0 / (t2 - t0));
        printf("\n%s : %f                                   ", var_names[state.var_index],
               *(&(c->pressure_force) + state.var_index));
        printf("\n");
        fflush(stdout);
    }

    glfwTerminate();
}
