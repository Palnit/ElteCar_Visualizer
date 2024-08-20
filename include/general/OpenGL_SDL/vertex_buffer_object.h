//
// Created by Palnit on 2024. 01. 17.
//

#ifndef GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_VERTEXBUFFEROBJECT_H_
#define GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_VERTEXBUFFEROBJECT_H_

#include <vector>
#include "GL/glew.h"

/*!
 * \class VertexBufferObject
 * \brief An abstraction to the OpenGl vertex buffers
 *
 * It takes any type az a template and stores it's data and generates an OpenGl
 * vertex buffer from it based on the given attributes
 * \tparam T The template type of the stored data
 */
template<typename T>
class VertexBufferObject {
public:
    class AttributeDescriptor;

    /*!
     * Default constructor generates the buffer
     */
    VertexBufferObject() : m_VBO(0), m_usage(GL_STATIC_DRAW) {
    }

    /*!
     * Constructor generates the buffer and stores the usage type
     * \param usage The usage type
     */
    VertexBufferObject(GLenum usage) : m_VBO(0), m_usage(usage){};

    /*!
     * Constructor that takes an array as it's initial data and a usage type
     * \tparam size The template to get the size of the array
     * \param elements The array of elements
     * \param usage The usage type
     */
    template<auto size>
    VertexBufferObject(T (&elements)[size],
                       GLenum usage) : m_VBO(0), m_usage(usage) {
        m_elements.insert(m_elements.end(),
                          elements,
                          elements + size);
    }

    /*!
     * Constructor that takes an initializer list as it's initial data and
     * a usage type
     * \param list The initializer list
     * \param usage The usage type
     */
    VertexBufferObject(std::initializer_list<T> list,
                       GLenum usage) : m_VBO(0), m_usage(usage) {
        m_elements.insert(m_elements.end(),
                          list.begin(),
                          list.end());
    }

    /*!
     * Destructor for the VBO it clears the arrays and deletes the buffer
     */
    ~VertexBufferObject() {
        m_elements.clear();
        m_desc.clear();
        if (m_VBO) {
            glDeleteBuffers(1, &m_VBO);
        }
    }

    /*!
     * Set the usage type of the buffer
     * \param usage The usage type
     */
    void SetUsage(GLenum usage) {
        m_usage = usage;
    }

    /*!
     * Add an element of type T to the data
     * \param element The element to be added
     */
    void AddElement(T element) {
        m_elements.push_back(element);
        m_set = false;
    }

    /*!
     * Add an array to the data
     * \tparam size Template to get the size of the array
     * \param elements The array of elements
     */
    template<auto size>
    void AddElement(T (&elements)[size]) {
        m_elements.insert(m_elements.end(),
                          elements,
                          elements + size);
        m_set = false;
    }

    /*!
     * Add a initializer list of elements to the data
     * \param list The initializer list
     */
    void AddElement(std::initializer_list<T> list) {
        m_elements.insert(m_elements.end(), list.begin(), list.end());
        m_set = false;
    };

    /*!
     * Add a initializer list of attribute descriptors to the descriptors
     * \param list The initializer list
     */
    void AddAttribute(std::initializer_list<AttributeDescriptor> list) {
        m_desc.insert(m_desc.end(), list.begin(), list.end());
    }

    /*!
     * Add an attribute descriptor to the VBO
     * \param element The descriptor to be added
     */
    void AddAttribute(AttributeDescriptor element) {
        m_desc.push_back(element);
    }

    /*!
     * Bind the VBO
     */
    void Bind() {
        if (!m_VBO) {
            glGenBuffers(1, &m_VBO);
        }
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        if (!m_set) {
            glBufferData(GL_ARRAY_BUFFER,
                         sizeof(T) * m_elements.size(),
                         m_elements.data(),
                         m_usage);
            m_set = true;
        }
    }

    /*!
     * UnBind the VBO
     */
    static void UnBind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    /*!
     * \class AttributeDescriptor
     * \brief Attribute description for a VBO
     *
     * It contains all data that describes a data type of VBO and the data OpenGL
     * needs to be abel to handel it later in the shader pipeline
     */
    struct AttributeDescriptor {
        /*!
         * Constructor
         * \param size The size of the data array
         * \param stride The stride of the data
         * \param type The type of the data
         * \param normalized Should it be normalized or not
         * \param offset The offset of the data
         */
        AttributeDescriptor(GLint size, GLsizei stride, GLenum type, GLboolean normalized, const GLvoid* offset)
            : size(size),
              stride(stride),
              type(type),
              normalized(normalized),
              offset(offset) {
        }

        /*!
         * Constructor
         * \param size The size of the data array
         * \param stride The stride of the data
         * \param type The type of the data
         * \param offset The offset of the data
         */
        AttributeDescriptor(GLint size,
                            GLsizei stride,
                            GLenum type,
                            const GLvoid* offset)
            : size(size),
              stride(stride),
              type(type),
              normalized(GL_FALSE),
              offset(offset) {
        }

        /*!
         * Constructor
         * \param size The size of the data array
         * \param stride The stride of the data
         * \param offset The offset of the data
         */
        AttributeDescriptor(GLint size, GLsizei stride, const GLvoid* offset)
            : size(size),
              stride(stride),
              type(GL_FLOAT),
              normalized(GL_FALSE),
              offset(offset) {
        }
        GLint size;
        GLenum type;
        GLboolean normalized;
        GLsizei stride;
        const GLvoid* offset;
    };

    /*!
     * Returns all the descriptors of the vbo
     * \return A vector of descriptors
     */
    const std::vector<AttributeDescriptor>& GetDescriptors() {
        return m_desc;
    }

private:
    std::vector<T> m_elements;
    GLuint m_VBO;
    GLenum m_usage;
    bool m_set = false;
    std::vector<AttributeDescriptor> m_desc;
};

#endif//GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_VERTEXBUFFEROBJECT_H_
