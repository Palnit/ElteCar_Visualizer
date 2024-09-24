#include "main_window.h"
#include <SDL_image.h>
#include <SDL_rwops.h>
#include <SDL_surface.h>
#include "general/OpenGL_SDL/element_buffer_object.h"
#include "general/OpenGL_SDL/file_handling.h"
#include "general/OpenGL_SDL/shader_program.h"
#include "general/OpenGL_SDL/vertex_array_object.h"
#include "general/OpenGL_SDL/vertex_buffer_object.h"
#include "general/SharedMemory/bufferd_reader.h"

SDL_Surface* tmp(void* pointer, int size) {
    SDL_RWops* rwops = SDL_RWFromConstMem(pointer, size);

    SDL_Surface* LoadedImg = IMG_Load_RW(rwops, 0);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    uint32_t format = SDL_PIXELFORMAT_ABGR8888;
#else
    uint32_t format = SDL_PIXELFORMAT_RGBA8888;
#endif

    SDL_Surface* image;
    image = SDL_ConvertSurfaceFormat(LoadedImg, format, 0);

    SDL_FreeSurface(LoadedImg);
    return image;
}

int MainWindow::Init() {

    m_reader = new SharedMemory::BufferedReader<SDL_Surface*>("Asd", tmp);
    m_reader2 = new SharedMemory::BufferedReader<SDL_Surface*>("Asd2", tmp);
    m_reader3 = new SharedMemory::BufferedReader<SDL_Surface*>("Asd3", tmp);
    m_reader4 = new SharedMemory::BufferedReader<SDL_Surface*>("Asd4", tmp);

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

    vertexShader = FileHandling::LoadShader(GL_VERTEX_SHADER,
                                            "shaders/default_vertex.vert");

    fragmentShader = FileHandling::LoadShader(GL_FRAGMENT_SHADER,
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
    if (m_image != nullptr) { SDL_FreeSurface(m_image); }
    m_image = m_reader->readData();
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image->w, m_image->h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, m_image->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO.UnBind();

    VAO2.Bind();
    if (m_image2 != nullptr) { SDL_FreeSurface(m_image2); }
    m_image2 = m_reader2->readData();
    glBindTexture(GL_TEXTURE_2D, tex2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image2->w, m_image2->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image2->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO2.UnBind();

    VAO3.Bind();
    if (m_image3 != nullptr) { SDL_FreeSurface(m_image3); }
    m_image3 = m_reader3->readData();
    glBindTexture(GL_TEXTURE_2D, tex3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image3->w, m_image3->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image3->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO3.UnBind();

    VAO4.Bind();
    if (m_image4 != nullptr) { SDL_FreeSurface(m_image4); }
    m_image4 = m_reader4->readData();
    glBindTexture(GL_TEXTURE_2D, tex4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image4->w, m_image4->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, m_image4->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    VAO4.UnBind();

    shaderProgram.UnBind();
}
