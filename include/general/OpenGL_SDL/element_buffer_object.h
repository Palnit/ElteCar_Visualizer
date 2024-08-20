//
// Created by Palnit on 2024. 01. 17.
//

#ifndef GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_ELEMENTBUFFEROBJECT_H_
#define GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_ELEMENTBUFFEROBJECT_H_

#include <vector>
#include "GL/glew.h"
/*!
 * \class ElementBufferObject
 * \brief A simple abstraction for OpenGl Element buffers
 *
 * It contains a vector of unsigned int that it binds as its data when you simply
 * call bind and only replaces the data when any new data has been loaded
 */
class ElementBufferObject {
public:
    /*!
     * Default constructor
     */
    ElementBufferObject() : m_EBO(0), m_usage(GL_STATIC_DRAW) {
    }

    /*!
     * Constructor
     * \param usage The way the data will be used
     */
    ElementBufferObject(GLenum usage) : m_EBO(0), m_usage(usage) {
    }

    /*!
     * A templated constructor to take any sized array as input
     * \tparam size The template to get the size of the array
     * \param elements The array it self
     * \param usage The way the data will be used
     */
    template<auto size>
    ElementBufferObject(unsigned int (&elements)[size],
                        GLenum usage) : m_EBO(0), m_usage(usage) {
        m_elements.insert(m_elements.end(),
                          elements,
                          elements + size);
    }

    /*!
     * Constructor with an initializer list
     * \param list the initializer list
     * \param usage The way the data will be used
     */
    ElementBufferObject(std::initializer_list<unsigned int> list,
                        GLenum usage) : m_EBO(0), m_usage(usage) {
        m_elements.insert(m_elements.end(),
                          list.begin(),
                          list.end());
    }

    /*!
     * Destructor deletes the buffer
     */
    ~ElementBufferObject();

    /*!
     * Set the usage data
     * \param usage The way the data will be used
     */
    void SetUsage(GLenum usage);

    /*!
     * Add a single element to the buffer
     * \param element The element to be added
     */
    void AddElement(unsigned int element);

    /*!
     * Add an array to the element buffer
     * \tparam size The templated parameter to take any sized array
     * \param elements The array of elements
     */
    template<auto size>
    void AddElement(unsigned int (&elements)[size]) {
        m_elements.insert(m_elements.end(),
                          elements,
                          elements + size);
        m_set = false;
    }

    /*!
     * Add elements taken by an initializer list
     * \param list The initializer list of elements
     */
    void AddElement(std::initializer_list<unsigned int> list);

    /*!
     * Binds the buffer and makes the data available
     */
    void Bind();

    /*!
     * UnBinds the buffer
     */
    static void UnBind();

private:
    std::vector<unsigned int> m_elements;
    GLuint m_EBO;
    GLenum m_usage;
    bool m_set = false;
};

#endif//GPGPU_EDGE_DETECTOR_SRC_GENERAL_OPENGL_SDL_ELEMENTBUFFEROBJECT_H_
