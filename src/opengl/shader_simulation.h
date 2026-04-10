#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "utils/vec3.h"

typedef struct
{
    float dt;
    GLuint config_ubo;
    GLuint simulation_ubo;
    Vec3 cam_pos;
} shader_simulation;

extern shader_simulation *s;
