//
// Created by Palnit on 2024. 01. 17.
//

#include "general/OpenGL_SDL/generic_structs.h"

#include <SDL2/SDL.h>

void ErrorHandling::HandelSDLError(const char* type) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                 "[%s] Error during the SDL initialization: %s",
                 type,
                 SDL_GetError());
}
