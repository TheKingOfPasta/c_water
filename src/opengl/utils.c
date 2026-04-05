#include "opengl/utils.h"

#include "config_reloader.h"
#include "simulation/simulation.h"

void opengl_add_config(GLuint program)
{
    glUniform1i(glGetUniformLocation(program,"c.sx"), c->sx);
    glUniform1i(glGetUniformLocation(program,"c.sy"), c->sy);
    glUniform1f(glGetUniformLocation(program,"c.radius"), c->radius);
    glUniform1f(glGetUniformLocation(program,"c.gravity_multiplier"), c->gravity_multiplier);
    glUniform1f(glGetUniformLocation(program,"c.pressure_force"), c->pressure_force);
    glUniform1f(glGetUniformLocation(program,"c.target_pressure"), c->target_pressure);
    glUniform1f(glGetUniformLocation(program,"c.velocity_collision_dampner"), c->velocity_collision_dampner);
    glUniform1f(glGetUniformLocation(program,"c.velocity_drag"), c->velocity_drag);
    glUniform1f(glGetUniformLocation(program,"c.particle_influence_radius"), c->particle_influence_radius);
}

void opengl_add_array(void *array, int size, int index)
{
    GLuint particleSSBO;
    glGenBuffers(1, &particleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, array, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, particleSSBO);
}

void opengl_prepare_program(GLuint program)
{
    glUseProgram(program);
}

void opengl_launch_last_prepared_program()
{
    glDispatchCompute((NB_PARTICLES+255)/256,1,1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void opengl_launch_program(GLuint program)
{
    opengl_prepare_program(program);
    opengl_launch_last_prepared_program();
}
