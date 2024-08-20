//
// Created by Palnit on 2024. 01. 17.
//

#ifndef GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_SHADER_PROGRAM_H_
#define GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_SHADER_PROGRAM_H_

#include <vector>
#include "GL/glew.h"

/*!
 * \class ShaderProgram
 * \brief An abstraction class for OpenGl-s shader programs
 *
 * It can take different kinds of shaders to link them and latter when needed
 * bind the program
 */
class ShaderProgram {
public:
    /*!
     * Default constructor generates the OpenGl program
     */
    ShaderProgram();

    /*!
     * Default destructor deletes the OpenGl program
     */
    ~ShaderProgram();

    /*!
     * Attaches a shader to the program
     * \param shader The id of the shader to be attached
     */
    void AttachShader(GLuint shader);

    /*!
     * Links the shader program
     */
    void LinkProgram();

    /*!
     * Binds the shader program
     */
    void Bind();

    /*!
     * UnBinds the shader program
     */
    void UnBind();

    void CreateProgram();

private:
    GLuint m_program;
    std::vector<GLuint> m_shaders;
    bool linked = false;
};

#endif//GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_OPENGL_SDL_SHADER_PROGRAM_H_
