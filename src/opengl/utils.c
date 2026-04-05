#include "opengl/utils.h"

#include "opengl/headers.h"
#include "config_reloader.h"
#include "simulation/simulation.h"

void opengl_add_config(GLuint program, GLuint ubo)
{
    glUniform1i(glGetUniformLocation(program, "NB_PARTICLES"), NB_PARTICLES);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(config), c);
}

GLuint opengl_add_array(void *array, int size, int index)
{
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, array, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, ssbo);

    return ssbo;
}

void opengl_prepare_program(GLuint program)
{
    glUseProgram(program);
}

void opengl_launch_last_prepared_program()
{
    glDispatchCompute((NB_PARTICLES+255)/256,1,1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void opengl_launch_program(GLuint program, GLuint ubo, GLuint particles_ssbo)
{
    opengl_prepare_program(program);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_PARTICLES, particles_ssbo);
    opengl_add_config(program, ubo);
    opengl_launch_last_prepared_program();
}
