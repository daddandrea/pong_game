#include "Shader.hpp"
#include <core/Logger.hpp>

#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

namespace renderer {

GLuint Shader::compile(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        glDeleteShader(shader);
        Log::error("Shader compile error: {}", log);
        throw std::runtime_error(std::string("Shader compile error:\n") + log);
    }

    return shader;
}

Shader::Shader(const char* vert_src, const char* frag_src) {
    GLuint vert = compile(GL_VERTEX_SHADER, vert_src);
    GLuint frag = compile(GL_FRAGMENT_SHADER, frag_src);

    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);

    GLint status = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (!status) {
        char log[1024];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        glDeleteShader(vert);
        glDeleteShader(frag);
        glDeleteProgram(m_program);
        Log::error("Shader link error: {}", log);
        throw std::runtime_error(std::string("Shader link error:\n") + log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    if (m_program) glDeleteProgram(m_program);
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::set_mat4(const char* name, const glm::mat4& m) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, glm::value_ptr(m));
}

void Shader::set_vec4(const char* name, const glm::vec4& v) const {
    glUniform4fv(glGetUniformLocation(m_program, name), 1, glm::value_ptr(v));
}

void Shader::set_int(const char* name, int i) const {
    glUniform1i(glGetUniformLocation(m_program, name), i);
}

void Shader::set_float(const char* name, float f) const {
    glUniform1f(glGetUniformLocation(m_program, name), f);
}
} // namespace renderer
