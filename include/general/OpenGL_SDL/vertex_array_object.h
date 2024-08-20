//
// Created by Palnit on 2024. 01. 17.
//

#ifndef GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_VERTEXARRAYOBJECT_H_
#define GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_VERTEXARRAYOBJECT_H_

#include "GL/glew.h"
#include "element_buffer_object.h"
#include "vertex_buffer_object.h"

/*!
 * \class VertexArrayObject
 * \brief An abstraction class for the OpenGl vertex arrays
 *
 * It takes a VertexBufferObject class and an ElementBufferObject and takes
 * makes the OpenGl vertex attributes from the VBO and binds the EBO to it self
 */
class VertexArrayObject {
public:
    /*!
     * Constructor that generates the vertex arrays
     */
    VertexArrayObject() : m_VAO(0), m_count(0) {
    }

    ~VertexArrayObject() {
        if (m_VAO) {
            glDeleteBuffers(1, &m_VAO);
        }
    }

    /*!
     * Binds the vertex array
     */
    void Bind();

    /*!
     * UnBinds the vertex array
     */
    void UnBind();

    /*!
     * Function to add a vertex buffer to the array and gets the attribute pointers
     * \tparam T The template type of the vertex buffer
     * \param VBO The Vbo to be added
     */
    template<typename T>
    void AddVertexBuffer(VertexBufferObject<T>& VBO) {
        Bind();
        VBO.Bind();
        for (unsigned int i = 0; i < VBO.GetDescriptors().size(); i++) {
            glVertexAttribPointer(m_count,
                                  VBO.GetDescriptors()[i].size,
                                  VBO.GetDescriptors()[i].type,
                                  VBO.GetDescriptors()[i].normalized,
                                  VBO.GetDescriptors()[i].stride,
                                  VBO.GetDescriptors()[i].offset);
            glEnableVertexAttribArray(m_count);
            m_count++;
        }
        UnBind();
        VBO.UnBind();
    }

    /*!
     * Adds an element buffer to the vertex array
     * \param EBO
     */
    void AddElementBuffer(ElementBufferObject& EBO) {
        Bind();
        EBO.Bind();
        UnBind();
        EBO.UnBind();
    }

private:
    GLuint m_VAO;
    GLuint m_count;
};

#endif//GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_VERTEXARRAYOBJECT_H_
