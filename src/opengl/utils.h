#pragma once

#include <glad/glad.h>
// glad
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "opengl/shader_simulation.h"

void opengl_add_config(GLuint program, shader_simulation* s);
GLuint opengl_add_array(void* array, int size, int index);
void opengl_prepare_program(GLuint program, shader_simulation* s);
void opengl_launch_last_prepared_program(size_t elt_count);
void opengl_launch_program(GLuint program, shader_simulation* s, size_t elt_count);
void bind_uniform_buffer(GLuint* ubo, GLuint binding, void* ptr, size_t elt_size);

GLuint create_compute_program(GLuint s, const char* src);
GLuint compile_shader(GLenum type, char* file_name);
GLuint create_program(GLuint s1, GLuint s2);
char* read_shader_includes(char* file);
char* read_shader(char* file);
GLFWwindow* init_window();

#define ENABLE_PRINTS false

#if ENABLE_PRINTS == true

#    define START_TIME(t)                                                                          \
        GLuint query##t;                                                                           \
        glGenQueries(1, &query##t);                                                                \
        glBeginQuery(GL_TIME_ELAPSED, query##t);

#    define END_TIME(t)                                                                            \
        glEndQuery(GL_TIME_ELAPSED);                                                               \
        GLuint64 elapsed##t;                                                                       \
        glGetQueryObjectui64v(query##t, GL_QUERY_RESULT, &elapsed##t);                             \
        printf("GPU time %s : %.2f ms\n", #t, elapsed##t / 1e6);

#else
#    define START_TIME(t) ;
#    define END_TIME(t) ;
#endif
