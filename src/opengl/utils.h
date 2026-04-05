#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

void opengl_add_config(GLuint program, GLuint ubo);
GLuint opengl_add_array(void *array, int size, int index);
void opengl_prepare_program(GLuint program);
void opengl_launch_last_prepared_program();
void opengl_launch_program(GLuint program, GLuint ubo, GLuint particles_ssbo);
GLuint create_compute_program(GLuint s, const char *src);
GLuint compile_shader(GLenum type, const char* src);
GLuint create_program(GLuint s1, GLuint s2);
char *read_shader_includes(char *file);
char *read_shader(char *file);
GLFWwindow* init_window();
