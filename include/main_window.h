#ifndef ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H
#define ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H

#include <HUH/Graphics/basic_window.h>
#include <HUH/Graphics/shader_program.h>
#include <HUH/Graphics/vertex_array_object.h>
#include <SDL3/SDL_surface.h>
#include <vector>
#include "HUH/Graphics/camera.h"
#include "HUH/Graphics/texture.h"
#include "cartesians.h"
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/threaded_multi_reader_handler.h"
#include "lidar_data.h"

class MainWindow : public HUH::BasicWindow {

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
          m_image(nullptr),
          m_camera(glm::vec3(0.0f, 0.0f, -3.0f)) {
        m_camera.SetAspect(static_cast<float>(m_width)
                           / static_cast<float>(m_height));
    }

    ~MainWindow() {
        delete m_threaded;
        delete m_csvReader;
        delete m_lidarReader;
        for (const auto Params : m_textureParams) { delete Params; }
    }

    /*!
     * Implementation of the Init function of the base class
     * \return Status
     */
    int Init() override;

    /*!
     * Implementation of the Render function of the base class
     */
    void Render() override;
    void KeyboardDown(const SDL_KeyboardEvent& ev) override;
    void KeyboardUp(const SDL_KeyboardEvent& ev) override;
    void MouseMove(const SDL_MouseMotionEvent& ev) override;
    void Update() override;

private:
    SharedMemory::BufferedReader<Cartesians>* m_csvReader;
    SharedMemory::BufferedReader<std::vector<LidarData>>* m_lidarReader;
    SharedMemory::ThreadedMultiReaderHandler<SDL_Surface*>* m_threaded;
    SDL_Surface* m_image;
    SDL_Surface* m_image2 = nullptr;
    SDL_Surface* m_image3 = nullptr;
    SDL_Surface* m_image4 = nullptr;
    HUH::VertexArrayObject VAO;
    HUH::VertexArrayObject VAO2;
    HUH::VertexArrayObject VAO3;
    HUH::VertexArrayObject VAO4;
    HUH::VertexBufferObject<float> VBO;
    HUH::VertexBufferObject<float> VBO2;
    HUH::VertexBufferObject<float> VBO3;
    HUH::VertexBufferObject<float> VBO4;
    HUH::ElementBufferObject EBO;
    std::vector<HUH::ITextureParameters*> m_textureParams;
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint vertexShaderLidar;
    GLuint fragmentShaderLidar;
    HUH::ShaderProgram shaderProgram;
    HUH::ShaderProgram shaderProgramLidar;
    HUH::Camera m_camera;
    bool first = true;
    HUH::VertexArrayObject VAOLidar;
    HUH::VertexBufferObject<float> VBOLidar;
};

#endif// ELTECAR_VISUALIZER_INCLUDE_MAIN_WINDOW_H
