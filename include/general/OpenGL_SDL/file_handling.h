//
// Created by Palnit on 2024. 01. 16.
//

#ifndef GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_F_H_
#define GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_F_H_

#include <SDL2/SDL_surface.h>
#include <GL/glew.h>

/*!
 * Namespace containing useful functions for file handling
 */
namespace FileHandling {
/*!
 * Loads any kinds of image in RGBA format into an SDL_Surface object
 * \param file The path to the image file to be opened
 * \return A pointer to the constructed SDL_Surface object the user is responsible
 * for the pointers lifetime
 */
SDL_Surface* LoadImage(const char* file);

/*!
 * Loads any kind of shader from a shader file and compiles it
 * \param shaderType The type of shader to be compiled into
 * \param filename The path to the shader file
 * \return The OpenGl identifier for the shader
 */
GLuint LoadShader(GLenum shaderType, const char* filename);

} // FileHandling

#endif //GPGPU_EDGE_DETECTOR_INCLUDE_GENERAL_F_H_
