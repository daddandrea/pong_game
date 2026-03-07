#include "Buffer.hpp"

namespace renderer {

Buffer::Buffer() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
}

Buffer::~Buffer() {
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
}

void Buffer::upload(const std::vector<float>& data, int vertex_size) {
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(data.size() * sizeof(float)),
                 data.data(),
                 GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);

    if (vertex_size == 2) {
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    } else if (vertex_size == 4) {
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    }

    glBindVertexArray(0);
}

void Buffer::draw(GLenum mode, int vertex_count) const {
    glBindVertexArray(m_vao);
    glDrawArrays(mode, 0, vertex_count);
    glBindVertexArray(0);
}
} // namespace renderer
