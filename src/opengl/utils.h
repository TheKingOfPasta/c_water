#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

void opengl_add_config(GLuint program);
void opengl_add_array(void *array, int elt_size, int index);
