//
// Created by Palnit on 2024. 01. 16.
//

#include "general/OpenGL_SDL/file_handling.h"
#include <SDL2/SDL_image.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

SDL_Surface* FileHandling::LoadImage(const char* file) {
    SDL_Surface* LoadedImg = IMG_Load(file);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    uint32_t format = SDL_PIXELFORMAT_ABGR8888;
#else
    uint32_t format = SDL_PIXELFORMAT_RGBA8888;
#endif

    SDL_Surface* NLoadedImg = SDL_ConvertSurfaceFormat(LoadedImg, format, 0);
    SDL_FreeSurface(LoadedImg);
    return NLoadedImg;
}
GLuint FileHandling::LoadShader(GLenum shaderType, const char* filename) {
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filename);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    } catch (std::ifstream::failure e) {
        std::cout << "Error" << std::endl;
    }
    const char* shaderCodeCStr = shaderCode.c_str();
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCodeCStr, NULL);
    glCompileShader(shader);
    return shader;
}
