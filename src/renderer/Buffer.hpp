#pragma once

#include <glad/glad.h>
#include <vector>

namespace renderer {

class Buffer {

public:
    Buffer();
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    void upload(const std::vector<float>& data, int vertex_size);

    void draw(GLenum mode, int vertex_count) const;

private:
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};

} // namespace renderer
