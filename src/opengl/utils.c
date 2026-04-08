#include "opengl/utils.h"
#include <stdlib.h>
#include <string.h>

#include "opengl/headers.h"
#include "config_reloader.h"
#include "simulation/simulation.h"
#include "utils/utils.h"

void opengl_add_config(GLuint program, shader_simulation *s)
{
    glUniform1i(glGetUniformLocation(program, "NB_PARTICLES"), NB_PARTICLES);

    glBindBuffer(GL_UNIFORM_BUFFER, s->config_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(config), c);
    glBindBuffer(GL_UNIFORM_BUFFER, s->simulation_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(shader_simulation), s);
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

void bind_uniform_buffer(GLuint *ubo, GLuint binding, void *ptr, size_t elt_size)
{
    glGenBuffers(1, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, *ubo);
    glBufferData(GL_UNIFORM_BUFFER, elt_size, ptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, *ubo);
}

void opengl_prepare_program(GLuint program, shader_simulation *s)
{
    glUseProgram(program);
    opengl_add_config(program, s);
}

void opengl_launch_last_prepared_program(size_t elt_count)
{
    glDispatchCompute((elt_count+255)/256,1,1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void opengl_launch_program(GLuint program, shader_simulation *s, size_t elt_count)
{
    opengl_prepare_program(program, s);
    opengl_launch_last_prepared_program(elt_count);
}

GLuint create_compute_program(GLuint s, const char *src)
{
    GLuint p = glCreateProgram();
    glAttachShader(p, s);
    glLinkProgram(p);

    int ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(p, 1024, NULL, log);
        printf("program link error:\n%s\n%s\n", log, src);
        exit(1);
    }

    return p;
}

GLuint compile_shader(GLenum type, char* file_name)
{
    const char *src = read_shader(file_name);

    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, 1024, NULL, log);
        printf("shader error\n%s\n:\n%s\n", src, log);
        exit(1);
    }

    if (type == GL_COMPUTE_SHADER)
        return create_compute_program(s, src);

    return s;
}

GLuint create_program(GLuint s1, GLuint s2)
{
    GLuint p = glCreateProgram();
    glAttachShader(p,s1);
    glAttachShader(p,s2);
    glLinkProgram(p);

    return p;
}

char *read_shader_includes(char *file)
{
    char *f = read_all_file(file);
    size_t result_len = 1;
    char *result = calloc(result_len, sizeof(char));

    const char *include_text = "#include \"";
    size_t include_len = strlen(include_text);
    size_t f_i = 0;
    size_t len = strlen(f);

    while (f_i < len)
    {
        if (strncmp(f + f_i, include_text, include_len) == 0)
        {
            f_i += include_len;

            char file_name[150] = { 0 };
            size_t file_name_size = 0;
            while (f[f_i] != '"')
                file_name[file_name_size++] = f[f_i++];
            f_i++;

            char *included = read_shader_includes(file_name);
            size_t included_len = strlen(included);

            result = realloc(result, result_len + included_len + 1 + len);
            strcat(result, included);
            result_len += included_len;

            free(included);
        }
        else
        {
            result = realloc(result, result_len + len);
            result[result_len - 1] = f[f_i++];
            result[result_len] = '\0';
            result_len++;
        }
    }

    free(f);
    return result;
}

char *read_shader(char *file)
{
    char *version = "#version 430 core\n#line 1\n\n";

    char *bindings = read_all_file("src/opengl/headers.h");
    char *f = read_all_file(file);
    char *config = read_all_file("shaders/headers.glsl");
    char *f_cpy = read_shader_includes(file);

    char* res = calloc(strlen(version) + strlen(bindings) + strlen(config) + strlen(f_cpy) + 2, sizeof(char));
    res = strcat(res, version);
    res = strcat(res, bindings);
    res = strcat(res, config);
    res = strcat(res, "\n");
    res = strcat(res, f_cpy);

    free(bindings);
    free(f);
    free(f_cpy);
    free(config);

    /*printf("------------------------------------\n");
    printf("%s\n", res);
    printf("------------------------------------\n");*/
    return res;
}

GLFWwindow* init_window()
{
    if(!glfwInit())
    {
        printf("glfwInit() failed\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(c->sx, c->sy, "C Water", NULL, NULL);

    glfwMakeContextCurrent(win);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("glad failed to load\n");
        exit(1);
    }

    return win;
}
