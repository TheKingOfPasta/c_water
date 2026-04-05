#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

void opengl_add_config(GLuint program);
void opengl_add_array(void *array, int size, int index);
void opengl_prepare_program(GLuint program);
void opengl_launch_last_prepared_program();
void opengl_launch_program(GLuint program);
