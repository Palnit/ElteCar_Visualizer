#ifndef ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H
#define ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H

#include <SDL_surface.h>
#include <vector>
#include "cartesians.h"
#include "general/OpenGL_SDL/basic_window.h"
#include "general/OpenGL_SDL/shader_program.h"
#include "general/OpenGL_SDL/vertex_array_object.h"
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/threaded_multi_reader_handler.h"
#include "lidar_data.h"

class MainWindow : public BasicWindow {

public:
    /*!
     * Constructor for the class same as the basic windows constructor
     * \param title The title of the window
     * \param x The horizontal position of the window
     * \param y The vertical position of the window
     * \param width The width of the window
     * \param height The height of the window
     * \param flags Flags for the sdl window creation function SDL_WINDOW_OPENGL
     * is always appended
     */
    MainWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
        : BasicWindow(title, x, y, w, h, flags),
          m_image(nullptr) {}

    ~MainWindow() { delete m_threaded; }

    /*!
     * Implementation of the Init function of the base class
     * \return Status
     */
    int Init() override;

    /*!
     * Implementation of the Render function of the base class
     */
    void Render() override;

private:
    SharedMemory::BufferedReader<cartesians>* m_csvReader;
    SharedMemory::BufferedReader<std::vector<LidarData>>* m_lidarReader;
    SharedMemory::ThreadedMultiReaderHandler<SDL_Surface*>* m_threaded;
    SDL_Surface* m_image;
    SDL_Surface* m_image2 = nullptr;
    SDL_Surface* m_image3 = nullptr;
    SDL_Surface* m_image4 = nullptr;
    GLuint tex;
    GLuint tex2;
    GLuint tex3;
    GLuint tex4;
    VertexArrayObject VAO;
    VertexArrayObject VAO2;
    VertexArrayObject VAO3;
    VertexArrayObject VAO4;
    VertexBufferObject<float> VBO;
    VertexBufferObject<float> VBO2;
    VertexBufferObject<float> VBO3;
    VertexBufferObject<float> VBO4;
    ElementBufferObject EBO;
    GLuint vertexShader;
    GLuint fragmentShader;
    ShaderProgram shaderProgram;
};

#endif// ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H
