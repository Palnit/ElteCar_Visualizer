#include <iostream>
#include <string>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "main_window.h"

int main() {
    MainWindow win("Edge Detector",
                   SDL_WINDOWPOS_CENTERED,
                   SDL_WINDOWPOS_CENTERED,
                   1024,
                   720,
                   SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
                       | SDL_WINDOW_RESIZABLE);

    return win.run();
}
