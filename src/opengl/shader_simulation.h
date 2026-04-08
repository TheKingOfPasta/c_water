#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

typedef struct
{
    float dt;
    GLuint config_ubo;
    GLuint simulation_ubo;
} shader_simulation;

extern shader_simulation *s;
