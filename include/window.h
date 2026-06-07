#pragma once

#include "cartesians.h"
#include "lidar_data.h"

#include <HUH/Math/matrix.h>
#include "HUH/FileHandling/Image/image.h"
#include "HUH/Graphics/camera.h"
#include "HUH/Graphics/time.h"
#include "general/SharedMemory/threaded_multi_reader_handler.h"

#include <HUH/RHI/fwd.h>
#include <HUH/Window/window.h>

class MainWindow {
public:
    struct Vertex {
        HUH::Vector3f pos;
        HUH::Vector2f texCoord;
    };
    struct LidarVertex {
        HUH::Vector3f pos;
        HUH::Vector3f color;
    };

    struct CameraData {
        HUH::Matrix4x4f view = HUH::Matrix4x4f::Identity();
        HUH::Matrix4x4f proj = HUH::Matrix4x4f::Identity();
    };
    MainWindow();
    int Run();

private:
    void InitializeRenderPass();
    void InitializePipeline();

    void InitializeVertexBuffers();
    void InitializeUniformBuffers();

    std::vector<Vertex> m_imagePlane{{{0, -0.25f, 0.25f}, {0, 1}},
                                     {{0, 0.25f, 0.25f}, {0, 0}},
                                     {{0, 0.25f, -0.25f}, {1, 0}},
                                     {{0, -0.25f, -0.25f}, {1, 1}}};

    std::vector<HUH::Uint32> m_imageIndices = {0, 1, 2, 2, 3, 0};

    HUH::Window m_window;
    HUH::Graphics::Time m_time;
    HUH::Graphics::Camera m_camera;
    size_t frame_index = 0;

    HUH::RHI::DynamicRHI* m_rhi = nullptr;
    HUH::RHI::Device* m_gpu = nullptr;
    HUH::RHI::Queue* m_graphicsQueue = nullptr;
    HUH::RHI::Swapchain* m_swapchain = nullptr;
    HUH::RHI::CommandPool* m_mainCommandPool = nullptr;
    HUH::RHI::MemoryAllocator* m_memoryAllocator = nullptr;

    HUH::RHI::Shader* m_imVertShader = nullptr;
    HUH::RHI::Shader* m_imFragShader = nullptr;

    HUH::RHI::Shader* m_lidarVertShader = nullptr;
    HUH::RHI::Shader* m_lidarFragShader = nullptr;

    HUH::RHI::RenderPass* m_imRenderPass = nullptr;
    HUH::RHI::RenderPass* m_lidarRenderPass = nullptr;

    HUH::RHI::Pipeline* m_imPipeline = nullptr;
    HUH::RHI::Pipeline* m_lidarPipeline = nullptr;

    std::vector<std::vector<HUH::RHI::Buffer*>> m_imagesTransferBuffer;
    std::vector<HUH::RHI::Image*> m_imageBuffers;
    std::vector<HUH::RHI::Buffer*> m_imUniformModelBuffers;

    std::vector<HUH::RHI::Buffer*> m_lidarUniformModelBuffers;

    HUH::RHI::Buffer* m_imVertexBuffer = nullptr;
    HUH::RHI::Buffer* m_imIndicesBuffer = nullptr;

    std::vector<HUH::RHI::Buffer*> m_lidarVertexBuffers;

    SharedMemory::ThreadedMultiReaderHandler<HUH::Image>* m_threaded;
    SharedMemory::BufferedReader<Cartesians>* m_csvReader;
    SharedMemory::BufferedReader<std::vector<LidarVertex>>* m_lidarReader;
};