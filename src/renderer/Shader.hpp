#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace renderer {

class Shader {
public:
    Shader(const char* vert_src, const char* frag_src);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void bind() const;

    void set_mat4(const char* name, const glm::mat4& m) const;
    void set_vec4(const char* name, const glm::vec4& v) const;
    void set_int(const char* name, int i) const;
    void set_float(const char* name, float f) const;

private:
    GLuint m_program = 0;

    static GLuint compile(GLenum type, const char* src);
};

}
