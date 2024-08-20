//
// Created by Palnit on 2024. 01. 17.
//

#include "general/OpenGL_SDL/element_buffer_object.h"

void ElementBufferObject::AddElement(unsigned int element) {
    m_elements.push_back(element);
    m_set = false;
}
void ElementBufferObject::AddElement(std::initializer_list<unsigned int> list) {
    m_elements.insert(m_elements.end(), list.begin(), list.end());
    m_set = false;
}

void ElementBufferObject::Bind() {
    if (!m_EBO) {
        glGenBuffers(1, &m_EBO);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    if (!m_set) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(unsigned int) * m_elements.size(),
                     m_elements.data(),
                     m_usage);
        m_set = true;
    }
}
void ElementBufferObject::UnBind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
ElementBufferObject::~ElementBufferObject() {
    m_elements.clear();
    if (m_EBO) {
        glDeleteBuffers(1, &m_EBO);
    }
}
void ElementBufferObject::SetUsage(GLenum usage) {
    m_usage = usage;
}
