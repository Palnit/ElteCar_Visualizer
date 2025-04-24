#include "main_window.h"
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vector>

#include "HUH/Graphics/file_handling.h"
#include "HUH/Graphics/generic_structs.h"
#include "HUH/Graphics/texture.h"
#include "cartesians.h"
#include "general/SharedMemory/bufferd_reader.h"
#include "general/SharedMemory/threaded_multi_reader_handler.h"
#include "lidar_data.h"

/// temporary function to get lidar data array
/// @param pointer pointer to shared memory
/// @param size size of shared memory
/// @return the returned lidar array
std::vector<LidarData> LidarReader(void* pointer, int size) {
    std::vector<LidarData> output;
    const auto* LidarPointer = static_cast<LidarData*>(pointer);
    for (int i = 0; i < size / sizeof(LidarData); i++) {
        output.push_back(*LidarPointer);
        LidarPointer++;
    }
    return output;
}

int MainWindow::Init() {
    glEnable(GL_PROGRAM_POINT_SIZE);
    m_textureParams.push_back(new HUH::TextureParameters(
        glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    m_textureParams.emplace_back(new HUH::TextureParameters(
        glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    m_threaded = new SharedMemory::ThreadedMultiReaderHandler<SDL_Surface*>(
        "Images", HUH::FileHandling::LoadImageFromMemory);

    m_lidarReader = new SharedMemory::BufferedReader<std::vector<LidarData>>(
        "Lidar", LidarReader);

    m_csvReader = new SharedMemory::BufferedReader<Cartesians>(
        "Csv", [](void* pointer, int size) {
            return *static_cast<Cartesians*>(pointer);
        });

    //for demonstration only
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

    fragmentShader = HUH::FileHandling::LoadShader(
        GL_FRAGMENT_SHADER, "shaders/default_fragment.frag");

    vertexShaderLidar =
        HUH::FileHandling::LoadShader(GL_VERTEX_SHADER, "shaders/lidar.vert");

    fragmentShaderLidar =
        HUH::FileHandling::LoadShader(GL_FRAGMENT_SHADER, "shaders/lidar.frag");

    shaderProgram.AttachShader(vertexShader);
    shaderProgram.AttachShader(fragmentShader);
    shaderProgramLidar.AttachShader(vertexShaderLidar);
    shaderProgramLidar.AttachShader(fragmentShaderLidar);

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

    VBOLidar.AddAttribute(
        {{3, 6 * sizeof(float), (void*) 0},
         {3, 6 * sizeof(float), (void*) (3 * sizeof(float))}});
    VAOLidar.AddVertexBuffer(VBOLidar);
    return 0;
}

void MainWindow::Render() {
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glViewport(0, 0, m_width, m_height);
    // glCullFace(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT);

    bool fail;
    auto Lidar = m_lidarReader->readData(fail);
    if (!Lidar.empty()) {
        VBOLidar.Clear();
        size_t i = 0;
        for (auto lidar : Lidar) {
            if (first) {
                std::cout << i << ":" << lidar.x << " " << lidar.y << " "
                          << lidar.z << std::endl;
            }
            // if (i >= 100) { break; }
            VBOLidar.AddElement(static_cast<float>(lidar.x));
            VBOLidar.AddElement(static_cast<float>(lidar.y));
            VBOLidar.AddElement(static_cast<float>(lidar.z));
            VBOLidar.AddElement(1.0f);
            VBOLidar.AddElement(0.0f);
            VBOLidar.AddElement(0.0f);
            i++;
        }
        if (first) { first = false; }
        shaderProgramLidar.Bind();

        auto model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
        shaderProgramLidar.SetUniform("model", glUniformMatrix4fv, 1, GL_FALSE,
                                      &model[0][0]);
        glm::mat4 view = m_camera.GetViewMatrix();
        shaderProgramLidar.SetUniform("view", glUniformMatrix4fv, 1, GL_FALSE,
                                      &view[0][0]);
        glm::mat4 projection = m_camera.GetProjectionMatrix();
        shaderProgramLidar.SetUniform("projection", glUniformMatrix4fv, 1,
                                      GL_FALSE, &projection[0][0]);
        VAOLidar.Bind();
        VBOLidar.Bind();
        glDrawArrays(GL_POINTS, 0, Lidar.size());
        VBO.UnBind();
        VAOLidar.UnBind();
        shaderProgramLidar.UnBind();
    }

    shaderProgram.Bind();
    glm::mat4 model = glm::mat4(1.0f);
    shaderProgram.SetUniform("model", glUniformMatrix4fv, 1, GL_FALSE,
                             &model[0][0]);
    glm::mat4 view = m_camera.GetViewMatrix();
    shaderProgram.SetUniform("view", glUniformMatrix4fv, 1, GL_FALSE,
                             &view[0][0]);
    glm::mat4 projection = m_camera.GetProjectionMatrix();
    shaderProgram.SetUniform("projection", glUniformMatrix4fv, 1, GL_FALSE,
                             &projection[0][0]);
    VAO.Bind();
    auto data = m_threaded->ReadMultiMemory();
    if (data.empty()) {
        VAO.UnBind();
        shaderProgram.UnBind();
        return;
    }
    // auto Cart = m_csvReader->readData(fail);
    // std::cout << "Cartesians ID: " << Cart.ID << " Alt: " << Cart.Alt
    //           << std::endl;

    //demosntration code only
    if (m_image != nullptr) { SDL_DestroySurface(m_image); }
    m_image = data[0];

    HUH::Texture2D TestTexture(m_image);
    TestTexture.SetParameters(m_textureParams);
    TestTexture.Bind(0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    TestTexture.UnBind();
    VAO.UnBind();

    VAO2.Bind();
    if (m_image2 != nullptr) { SDL_DestroySurface(m_image2); }
    m_image2 = data[1];

    HUH::Texture2D TestTexture2(m_image2);
    TestTexture2.SetParameters(m_textureParams);
    TestTexture2.Bind(0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    TestTexture2.UnBind();
    VAO2.UnBind();

    VAO3.Bind();
    if (m_image3 != nullptr) { SDL_DestroySurface(m_image3); }
    m_image3 = data[2];

    HUH::Texture2D TestTexture3(m_image3);
    TestTexture3.SetParameters(m_textureParams);
    TestTexture3.Bind(0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    TestTexture3.UnBind();
    VAO3.UnBind();

    VAO4.Bind();
    if (m_image4 != nullptr) { SDL_DestroySurface(m_image4); }
    m_image4 = data[3];

    HUH::Texture2D TestTexture4(m_image4);
    TestTexture4.SetParameters(m_textureParams);
    TestTexture4.Bind(0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    TestTexture4.UnBind();

    VAO4.UnBind();

    shaderProgram.UnBind();
}
void MainWindow::KeyboardDown(const SDL_KeyboardEvent& ev) {
    m_camera.ProcessKeyboardDown(ev);
}
void MainWindow::KeyboardUp(const SDL_KeyboardEvent& ev) {
    m_camera.ProcessKeyboardUp(ev);
}
void MainWindow::MouseMove(const SDL_MouseMotionEvent& ev) {
    m_camera.ProcessMouseMovement(ev);
}
void MainWindow::Update() {
    BasicWindow::Update();
    m_camera.Update();
}
