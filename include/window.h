#pragma once

#include "HUH/Cuda/memory_allocator.h"
#include "HUH/Cuda/module.h"
#include "cartesians.h"
#include "lidar_data.h"

#include <HUH/Cuda/device.h>

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
    ~MainWindow();
    int Run();

private:
    void InitializeRenderPass();
    void InitializePipeline();

    void InitializeVertexBuffers();
    void InitializeUniformBuffers();

    void WindowResize(HUH::Window* win, HUH::Vector2u32 size);
    void ReadImages();
    bool ReadLidar();
    void RecordImageBufferCopy();

    std::vector<Vertex> m_imagePlane{{{0, -1.f, 1.f}, {0, 0}},
                                     {{0, 1.f, 1.f}, {1, 0}},
                                     {{0, 1.f, -1.f}, {1, 1}},
                                     {{0, -1.f, -1.f}, {0, 1}}};

    std::vector<HUH::Uint32> m_imageIndices = {0, 1, 2, 2, 3, 0};

    HUH::Window m_window;
    HUH::Graphics::Time m_time;
    HUH::Graphics::Camera m_camera;
    HUH::Vector2u32 m_viewportSize{1024, 720};
    size_t frame_index = 0;

    HUH::Cuda::Device* m_cudaGpu = nullptr;
    HUH::Cuda::Module m_cudaModule;
    HUH::Cuda::MemoryAllocator m_cudaMemoryAllocator;

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

    std::vector<std::vector<HUH::RHI::Buffer*>> m_imUniformModelBuffers;
    std::vector<std::vector<HUH::RHI::Barrier*>> m_imBarrierDst;
    std::vector<std::vector<HUH::RHI::Barrier*>> m_imBarrierOpt;

    std::vector<std::vector<HUH::RHI::Buffer*>> m_imImagesTransferBuffer;
    std::vector<std::vector<HUH::RHI::Image*>> m_imImageBuffers;

    std::vector<HUH::RHI::Buffer*> m_lidarUniformModelBuffers;

    HUH::RHI::Buffer* m_imVertexBuffer = nullptr;
    HUH::RHI::Buffer* m_imIndicesBuffer = nullptr;

    std::vector<HUH::RHI::Buffer*> m_lidarVertexBuffers;
    std::vector<size_t> m_lidarSizes;

    SharedMemory::ThreadedMultiReaderHandler<HUH::Image>* m_imageReader;
    SharedMemory::BufferedReader<Cartesians>* m_csvReader;
    SharedMemory::BufferedReader<std::vector<LidarVertex>>* m_lidarReader;
};