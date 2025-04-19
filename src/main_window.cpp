#include "main_window.h"
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <vector>
#include "HUH/Graphics/file_handling.h"
#include "cartesians.h"
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/threaded_multi_reader_handler.h"
#include "lidar_data.h"

/// temporary function to read sdl_surface from shm
/// @param pointer pointer to shared memory
/// @param size size of shared memory
/// @return the returned sdl_surface pointer
SDL_Surface* tmp(void* pointer, int size) {
    SDL_IOStream* rwops = SDL_IOFromConstMem(pointer, size);

    SDL_Surface* LoadedImg = IMG_Load_IO(rwops, 0);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    SDL_PixelFormat format = SDL_PIXELFORMAT_ABGR8888;
#else
    SDL_PixelFormat format = SDL_PIXELFORMAT_RGBA8888;
#endif

    SDL_Surface* image;
    image = SDL_ConvertSurface(LoadedImg, format);

    SDL_DestroySurface(LoadedImg);
    return image;
}
/// temporary function to get lidar data array
/// @param pointer pointer to shared memory
/// @param size size of shared memory
/// @return the returned lidar array
std::vector<LidarData> tmp2(void* pointer, int size) {
    std::vector<LidarData> output;
    LidarData* lidarPoiter = (LidarData*) pointer;
    for (int i = 0; i < size / sizeof(LidarData); i++) {
        output.push_back(*lidarPoiter);
        lidarPoiter++;
    }
    return output;
}

int MainWindow::Init() {

    m_threaded = new SharedMemory::ThreadedMultiReaderHandler<SDL_Surface*>(
        "Images", tmp);

    m_lidarReader =
        new SharedMemory::BufferedReader<std::vector<LidarData>>("Lidar", tmp2);

    m_csvReader = new SharedMemory::BufferedReader<Cartesians>(
        "Csv", [](void* pointer, int size) { return *(Cartesians*) pointer; });

    //for demonstration only
    glGenTextures(1, &tex);
    glGenTextures(1, &tex2);
    glGenTextures(1, &tex3);
    glGenTextures(1, &tex4);
    float verts[] = {-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f,
                     0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  1.0f,
                     0.0f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

    float verts2[] = {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                      0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                      0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};

    float verts3[] = {-1.0f, -1.0f, 0.0f,  0.0f, 1.0f, -1.0f, 0.0f,
                      0.0f,  0.0f,  0.0f,  0.0f, 0.0f, 0.0f,  1.0f,
                      0.0f,  0.0f,  -1.0f, 0.0f, 1.0f, 1.0f};

    float verts4[] = {0.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f,  0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                      0.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 1.0f};

    VBO.AddElement(verts);
    VBO2.AddElement(verts2);
    VBO3.AddElement(verts3);
    VBO4.AddElement(verts4);
    EBO.AddElement({2, 3, 1, 3, 0, 1});

    vertexShader = HUH::FileHandling::LoadShader(GL_VERTEX_SHADER,
                                            "shaders/default_vertex.vert");

    fragmentShader = HUH::FileHandling::LoadShader(GL_FRAGMENT_SHADER,
                                              "shaders/default_fragment.frag");

    shaderProgram.AttachShader(vertexShader);
    shaderProgram.AttachShader(fragmentShader);

    VBO.AddAttribute({{3, 5 * sizeof(float), (void*) 0},
                      {2, 5 * sizeof(float), (void*) (3 * sizeof(float))}});
    VBO2.AddAttribute({{3, 5 * sizeof(float), (void*) 0},
                       {2, 5 * sizeof(float), (void*) (3 * sizeof(float))}});
    VBO3.AddAttribute({{3, 5 * sizeof(float), (void*) 0},
                       {2, 5 * sizeof(float), (void*) (3 * sizeof(float))}});
    VBO4.AddAttribute({{3, 5 * sizeof(float), (void*) 0},
                       {2, 5 * sizeof(float), (void*) (3 * sizeof(float))}});
    VAO.AddVertexBuffer(VBO);
    VAO.AddElementBuffer(EBO);
    VAO2.AddVertexBuffer(VBO2);
    VAO2.AddElementBuffer(EBO);
    VAO3.AddVertexBuffer(VBO3);
    VAO3.AddElementBuffer(EBO);
    VAO4.AddVertexBuffer(VBO4);
    VAO4.AddElementBuffer(EBO);
    return 0;
}

void MainWindow::Render() {
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glViewport(0, 0, m_width, m_height);
    glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderProgram.Bind();
    VAO.Bind();
    auto data = m_threaded->ReadMultiMemory();
    if (data.empty()) {
        VAO.UnBind();
        shaderProgram.UnBind();
        return;
    }
    bool fail;
    auto Lidar = m_lidarReader->readData(fail);
    if (Lidar.size() > 0) {
        std::cout << "Lidar:" << Lidar.size() << std::endl;
    }
    auto Cart = m_csvReader->readData(fail);
    std::cout << "Cartesians ID: " << Cart.ID << " Alt: " << Cart.Alt
              << std::endl;

    //demosntration code only
    if (m_image != nullptr) { SDL_DestroySurface(m_image); }
    m_image = data[0];

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image->w, m_image->h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, m_image->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO.UnBind();

    VAO2.Bind();
    if (m_image2 != nullptr) { SDL_DestroySurface(m_image2); }
    m_image2 = data[1];
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image2->w, m_image2->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image2->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO2.UnBind();

    VAO3.Bind();
    if (m_image3 != nullptr) { SDL_DestroySurface(m_image3); }
    m_image3 = data[2];
    glBindTexture(GL_TEXTURE_2D, tex3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image3->w, m_image3->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image3->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO3.UnBind();

    VAO4.Bind();
    if (m_image4 != nullptr) { SDL_DestroySurface(m_image4); }
    m_image4 = data[3];
    glBindTexture(GL_TEXTURE_2D, tex4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image4->w, m_image4->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image4->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO4.UnBind();

    shaderProgram.UnBind();
}
