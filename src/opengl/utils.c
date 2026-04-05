#include "opengl/utils.h"
#include <stdlib.h>
#include <string.h>

#include "opengl/headers.h"
#include "config_reloader.h"
#include "simulation/simulation.h"
#include "utils/utils.h"

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

GLuint compile_shader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if(!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, 1024, NULL, log);
        printf("shader error %s:\n%s\n", src, log);
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
    char *config = read_all_file("src/opengl/headers.glsl");
    char *f_cpy = read_shader_includes(file);

    char* res = calloc(strlen(version) + strlen(bindings) + strlen(f_cpy) + strlen(config) + 1, sizeof(char));
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
