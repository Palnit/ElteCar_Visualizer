#include "window.h"

#include "HUH/FileHandling/Image/image_reader.h"
#include "HUH/Graphics/camera_window_connector.h"
#include "HUH/RHI/Types/barrier.h"
#include "HUH/RHI/Types/fence.h"
#include "HUH/RHI/memory_allocator.h"
#include "HUH/RHI/render_pass.h"
#include "HUH/RHI/vertex_factory.h"
#include "lidar_data.h"

#include <HUH/RHI/device.h>
#include <HUH/RHI/dynamic_rhi.h>
#include <HUH/RHI/shader.h>
#include <HUH/RHI/swapchain.h>

#include <fstream>

inline HUH::LogCategory AppLog("Application");

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

MainWindow::~MainWindow() {
    delete m_imageReader;
    delete m_lidarReader;
    delete m_csvReader;
    m_cudaGpu->Destroy();
}

MainWindow::MainWindow() : m_window("Elte Car Visualizer", {1024, 720}) {
    m_imageReader = new SharedMemory::ThreadedMultiReaderHandler<HUH::Image>("Images", [](void* pointer, int size) {
        // TODO no copy ?
        std::vector data(static_cast<HUH::Uint8*>(pointer), static_cast<HUH::Uint8*>(pointer) + size);
        return HUH::FileHandling::ReadImageFromData(data);
    });

    m_lidarReader = new SharedMemory::BufferedReader<std::vector<LidarVertex>>("Lidar", [](void* pointer, int size) {
        auto* data = static_cast<LidarData<double>*>(pointer);
        std::vector<LidarVertex> vertexData;
        vertexData.resize(size / sizeof(LidarData<double>));
        for (size_t i = 0; i < vertexData.size(); ++i) {
            vertexData[i] = {data[i].data, {1, 0, 0}};
        }
        return vertexData;
    });
    m_csvReader = new SharedMemory::BufferedReader<Cartesians>(
        "Csv", [](void* pointer, int size) { return *static_cast<Cartesians*>(pointer); });

    // Discarding handler becouse i don't need it this class wll exist only with paretn
    auto Handler = m_window.OnSizeChange.Add(this, &MainWindow::WindowResize);

    HUH::RHI::DynamicRHI::LoadRHI(HUH::RHI::RenderApi::Vulkan);
    m_rhi = HUH::RHI::DynamicRHI::Create();
    m_rhi->Init();

    auto devices = m_rhi->GetDevices();
    for (auto device : devices) {
        if (device->Information.type == HUH::RHI::Device::Type::Dedicated) {
            m_gpu = device;
            m_cudaGpu = HUH::Cuda::Device::CreateFromRHI(m_gpu);
            if (!m_cudaGpu) {
                continue;
            }
            break;
        }
    }

    if (m_gpu == nullptr) {
        HUH_WLOG(AppLog, "No Suitable Gpu found Falling back to 0 gpu")
        m_gpu = devices[0];
    }

    m_cudaGpu->ActivateDevice();

    // Init GPU
    m_graphicsQueue = m_gpu->RequestQueue(HUH::RHI::Queue::Graphics);
    // TODO separate queues for transfer and visualization 2 threaded system
    // m_transferQueue = m_gpu->RequestQueue(HUH::RHI::Queue::Transfer);
    m_gpu->Init();

    m_memoryAllocator = m_gpu->CreateMemoryAllocator();

    m_mainCommandPool = m_gpu->CreateCommandPool();
    m_mainCommandPool->Init(2, m_graphicsQueue);

    // Init Swapchain
    m_swapchain = m_gpu->CreateSwapchain(m_window);
    m_swapchain->Init(HUH::RHI::Format::R8G8B8A8_SRGB, HUH::RHI::Swapchain::PresentMode::Immediate, 2);

    InitializeRenderPass();
    InitializePipeline();

    InitializeVertexBuffers();
    InitializeUniformBuffers();
}

int MainWindow::Run() {
    m_camera.Transform.Position.X() -= 1;
    HUH::Graphics::CameraWindowConnector connector(m_camera, m_window, m_time);
    connector.CameraMove = HUH::KeyBindings::LeftAlt;
    connector.Up = HUH::KeyBindings::e;
    connector.Down = HUH::KeyBindings::q;

    auto fence = m_gpu->CreateFence(2);
    auto fenceS = m_gpu->CreateFence(2);
    auto fenceS2 = m_gpu->CreateFence(2);
    auto fenceCuda = m_gpu->CreateFence(2);

    while (m_window.Loop()) {
        ReadImages();
        bool lidarFound = ReadLidar();
        fence[frame_index]->Wait();
        m_time.Update();
        auto Image = m_swapchain->NextImage(fenceS[frame_index]);
        fence[frame_index]->Reset();
        (*m_mainCommandPool)[frame_index]->Reset();
        (*m_mainCommandPool)[frame_index]->SetViewPort(m_viewportSize);
        (*m_mainCommandPool)[frame_index]->SetScissor(m_viewportSize);
        (*m_mainCommandPool)[frame_index]->SetClearColor({0.45f, 0.55f, 0.60f, 1.00f});
        (*m_mainCommandPool)[frame_index]->Begin();
        RecordImageBufferCopy();
        (*m_mainCommandPool)[frame_index]->BeginRendering(m_imRenderPass, Image, {0, 0},
                                                          {m_viewportSize.X(), m_viewportSize.Y() / 2});
        (*m_mainCommandPool)[frame_index]->BindPipeline(m_imPipeline);
        (*m_mainCommandPool)[frame_index]->BindVertexBuffer(m_imVertexBuffer, 0);
        (*m_mainCommandPool)[frame_index]->BindIndexBuffer(m_imIndicesBuffer);
        auto scale = 1.f / static_cast<float>(m_imImageBuffers[frame_index].size());
        HUH::Matrix4x4f baseScale({1, 0, 0, 0}, {0, scale, 0, 0}, {0, 0, 0.5, 0},
                                  {0, -scale * static_cast<float>(m_imImageBuffers[frame_index].size() - 1), 0.5, 1});
        for (size_t i = 0; i < m_imImageBuffers[frame_index].size(); i++) {
            HUH::Matrix4x4f model = baseScale;
            model[3][1] += static_cast<float>(i) * scale * 2;
            model *= HUH::Matrix4x4f{{0, 0, 1, 0}, {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0, 1}};
            m_imUniformModelBuffers[frame_index][i]->UploadData(&model);
            (*m_mainCommandPool)[frame_index]->BindUniformBuffers(m_imUniformModelBuffers[frame_index][i]);
            (*m_mainCommandPool)[frame_index]->BindSampledImage(m_imImageBuffers[frame_index][i]);
            (*m_mainCommandPool)[frame_index]->DrawIndexed(m_imageIndices.size(), 1);
        }
        (*m_mainCommandPool)[frame_index]->EndRendering();
        (*m_mainCommandPool)[frame_index]->BeginRendering(m_lidarRenderPass, Image,
                                                          {0, static_cast<HUH::Int32>(m_viewportSize.Y() / 2)},
                                                          {m_viewportSize.X(), m_viewportSize.Y() / 2});
        if (lidarFound) {
            HUH::Cuda::UniquePtr ptr(
                static_cast<LidarVertex*>(m_cudaMemoryAllocator.MapRHIBuffer(m_lidarVertexBuffers[frame_index])));

            HUH::Vector3i test = {1024, 1, 1};
            HUH::Vector3i test2 = {static_cast<int>(m_lidarSizes[frame_index] / 1024) + 1, 1, 1};
            HUH_TLOG("block: {}", test);
            HUH_TLOG("GRID: {}", test2);
            if (m_cudaModule.Load("plane_ransac.ptx")) {
                for (auto& func : m_cudaModule.GetFunctions()) {
                    func.SetBlock(test);
                    func.SetGrid(test2);
                    // LidarVertex* data;
                    // cudaMallocManaged(&data, 4 * sizeof(LidarVertex));
                    // cudaMemcpy(data, ptr.Get(), 4 * sizeof(LidarVertex), cudaMemcpyDeviceToHost);
                    // for (size_t i = 0; i < 4; ++i) {
                    //     // data[i].pos = test;
                    //     // data[i].color = test;
                    //     HUH_TLOG("ORiginal COlor: {}", data[i].color)
                    // }

                    func.Execute(ptr.Get(), HUH::Vector3f(0, 0, 1),
                                 static_cast<HUH::Uint32>(m_lidarSizes[frame_index]));

                    cudaDeviceSynchronize();
                    // cudaMemcpy(data, ptr.Get(), 4 * sizeof(LidarVertex), cudaMemcpyDeviceToHost);
                    // for (size_t i = 0; i < 4; ++i) {
                    //     HUH_TLOG("Changed COlor: {}", ptr[i].color)
                    // }

                    HUH_TLOG("Functions Name: {}", func.Name)
                }
            }
            (*m_mainCommandPool)[frame_index]->BindPipeline(m_lidarPipeline);
            CameraData cameraData(m_camera.GetViewMatrix(), m_camera.GetPerspectiveProjectionMatrix());
            m_lidarUniformModelBuffers[frame_index]->UploadData(&cameraData);
            (*m_mainCommandPool)[frame_index]->BindUniformBuffers(m_lidarUniformModelBuffers[frame_index]);
            (*m_mainCommandPool)[frame_index]->BindVertexBuffer(m_lidarVertexBuffers[frame_index], 0);
            (*m_mainCommandPool)[frame_index]->Draw(m_lidarSizes[frame_index], 1);
        }
        (*m_mainCommandPool)[frame_index]->EndRendering();
        (*m_mainCommandPool)[frame_index]->End();
        m_graphicsQueue->Submit((*m_mainCommandPool)[frame_index], fenceS[frame_index], fenceS2[frame_index],
                                fence[frame_index]);
        m_swapchain->Present(m_graphicsQueue, fenceS2[frame_index]);
        frame_index = (frame_index + 1) % 2;
        // if (lidarFound) {
        //     break;
        // }
    }
    // m_graphicsQueue->WaitIdle();
    m_rhi->Destroy();
    return 0;
}

void MainWindow::InitializeRenderPass() {
    m_imRenderPass = m_gpu->CreateRenderPass();
    m_lidarRenderPass = m_gpu->CreateRenderPass();
    HUH::RHI::RenderPass::SubPass imSubPass;
    imSubPass.ColorAttachments.push_back({
        .AttachmentFormat = m_swapchain->GetFormat(),
        .ColorLoadOp = HUH::RHI::RenderPass::LoadOp::Clear,
        .ColorStoreOp = HUH::RHI::RenderPass::StoreOp::Store,
        .InitialLayout = HUH::RHI::RenderPass::Layout::Unknown,
        .FinalLayout = HUH::RHI::RenderPass::Layout::Color,
    });

    HUH::RHI::RenderPass::SubPass lidarSubPass;
    lidarSubPass.ColorAttachments.push_back({
        .AttachmentFormat = m_swapchain->GetFormat(),
        .ColorLoadOp = HUH::RHI::RenderPass::LoadOp::Clear,
        .ColorStoreOp = HUH::RHI::RenderPass::StoreOp::Store,
        .InitialLayout = HUH::RHI::RenderPass::Layout::Color,
        .FinalLayout = HUH::RHI::RenderPass::Layout::Present,
    });
    m_imRenderPass->AddSubPass(imSubPass);
    m_lidarRenderPass->AddSubPass(lidarSubPass);
    m_imRenderPass->AddDependency({.DstSubPassIndex = 0,
                                   .SrcStageMask = HUH::RHI::Pipeline::Stages::ColorAttachmentOutput,
                                   .DstStageMask = HUH::RHI::Pipeline::Stages::ColorAttachmentOutput,
                                   .SrcAccessType = HUH::RHI::AccessType::Unknown,
                                   .DstAccessType = HUH::RHI::AccessType::ColorWrite});
    m_lidarRenderPass->AddDependency({.DstSubPassIndex = 0,
                                      .SrcStageMask = HUH::RHI::Pipeline::Stages::ColorAttachmentOutput,
                                      .DstStageMask = HUH::RHI::Pipeline::Stages::ColorAttachmentOutput,
                                      .SrcAccessType = HUH::RHI::AccessType::Unknown,
                                      .DstAccessType = HUH::RHI::AccessType::ColorWrite});

    m_imRenderPass->Init();
    m_lidarRenderPass->Init();
}

void MainWindow::InitializePipeline() {
    auto imVertShaderBin = readFile("./VulkanShader/im_vert.spv");
    auto imFragShaderBin = readFile("./VulkanShader/im_frag.spv");

    auto lidarVertShaderBin = readFile("./VulkanShader/lidar_vert.spv");
    auto lidarFragShaderBin = readFile("./VulkanShader/lidar_frag.spv");

    // Init Shader
    m_imVertShader = m_gpu->CreateShader(imVertShaderBin.data(), imVertShaderBin.size());
    m_imVertShader->Init(HUH::RHI::Shader::Stage::Vertex);
    m_imFragShader = m_gpu->CreateShader(imFragShaderBin.data(), imFragShaderBin.size());
    m_imFragShader->Init(HUH::RHI::Shader::Stage::Fragment);

    m_lidarVertShader = m_gpu->CreateShader(lidarVertShaderBin.data(), lidarVertShaderBin.size());
    m_lidarVertShader->Init(HUH::RHI::Shader::Stage::Vertex);
    m_lidarFragShader = m_gpu->CreateShader(lidarFragShaderBin.data(), lidarFragShaderBin.size());
    m_lidarFragShader->Init(HUH::RHI::Shader::Stage::Fragment);

    // Init VertexFactory
    HUH::RHI::VertexFactory imVertexFactory;
    HUH::RHI::VertexFactory lidarVertexFactory;

    imVertexFactory.AddVertexStream<&Vertex::pos, &Vertex::texCoord>();

    lidarVertexFactory.AddVertexStream<&LidarVertex::pos, &LidarVertex::color>();
    lidarVertexFactory.SetPolygonMode(HUH::RHI::VertexFactory::PolygonMode::Point);

    m_imPipeline = m_gpu->CreatePipeline();
    m_lidarPipeline = m_gpu->CreatePipeline();

    m_imPipeline->AddShader(m_imVertShader);
    m_imPipeline->AddShader(m_imFragShader);

    m_lidarPipeline->AddShader(m_lidarVertShader);
    m_lidarPipeline->AddShader(m_lidarFragShader);

    m_imPipeline->Init({m_imRenderPass,
                        imVertexFactory,
                        {
                            {HUH::RHI::Pipeline::DescriptorTypes::Uniform, 1, HUH::RHI::Shader::Stage::Vertex},
                            {HUH::RHI::Pipeline::DescriptorTypes::ImageSampler, 1, HUH::RHI::Shader::Stage::Fragment},
                        },
                        false});

    m_lidarPipeline->Init({m_lidarRenderPass,
                           lidarVertexFactory,
                           {
                               {HUH::RHI::Pipeline::DescriptorTypes::Uniform, 1, HUH::RHI::Shader::Stage::Vertex},
                           },
                           false});
}

void MainWindow::InitializeVertexBuffers() {
    auto srcVertexBuffer =
        m_imPipeline->CreateBuffer(HUH::RHI::Buffer::Type::SRC, sizeof(Vertex) * m_imagePlane.size());
    m_imVertexBuffer = m_imPipeline->CreateBuffer(HUH::RHI::Buffer::Type::VERTEX | HUH::RHI::Buffer::Type::DST,
                                                  sizeof(Vertex) * m_imagePlane.size());

    m_memoryAllocator->Allocate(srcVertexBuffer, HUH::RHI::MemoryAllocator::Host | HUH::RHI::MemoryAllocator::Device);
    m_memoryAllocator->Allocate(m_imVertexBuffer, HUH::RHI::MemoryAllocator::Device);

    auto srcIndexBuffer =
        m_imPipeline->CreateBuffer(HUH::RHI::Buffer::Type::SRC, sizeof(HUH::Uint32) * m_imageIndices.size());
    m_imIndicesBuffer = m_imPipeline->CreateBuffer(HUH::RHI::Buffer::Type::INDEX | HUH::RHI::Buffer::Type::DST,
                                                   sizeof(HUH::Uint32) * m_imageIndices.size());

    m_memoryAllocator->Allocate(srcIndexBuffer, HUH::RHI::MemoryAllocator::Host | HUH::RHI::MemoryAllocator::Device);
    m_memoryAllocator->Allocate(m_imIndicesBuffer, HUH::RHI::MemoryAllocator::Device);

    srcVertexBuffer->CopyData(m_imagePlane.data());
    srcIndexBuffer->CopyData(m_imageIndices.data());

    (*m_mainCommandPool)[0]->Begin();
    (*m_mainCommandPool)[0]->CopyBuffer(srcVertexBuffer, m_imVertexBuffer);
    (*m_mainCommandPool)[0]->CopyBuffer(srcIndexBuffer, m_imIndicesBuffer);
    (*m_mainCommandPool)[0]->End();
    m_graphicsQueue->Submit((*m_mainCommandPool)[0]);
    m_graphicsQueue->WaitIdle();
    m_imPipeline->DestroyBuffer(srcVertexBuffer);
    m_imPipeline->DestroyBuffer(srcIndexBuffer);
}

void MainWindow::InitializeUniformBuffers() {
    for (size_t i = 0; i < 2; ++i) {
        auto cameraBuffer = m_lidarPipeline->CreateBuffer(HUH::RHI::Buffer::Type::UNIFORM, sizeof(CameraData), 0);
        m_memoryAllocator->Allocate(cameraBuffer, HUH::RHI::MemoryAllocator::Device | HUH::RHI::MemoryAllocator::Host);
        m_lidarUniformModelBuffers.push_back(cameraBuffer);
        m_imUniformModelBuffers.emplace_back();
        m_imImageBuffers.emplace_back();
        m_imImagesTransferBuffer.emplace_back();
        m_imBarrierDst.emplace_back();
        m_imBarrierOpt.emplace_back();
        m_lidarVertexBuffers.resize(2, nullptr);
        m_lidarSizes.resize(2, 0);
    }
}

void MainWindow::WindowResize(HUH::Window* win, HUH::Vector2u32 size) {
    m_viewportSize = size;
}

void MainWindow::ReadImages() {
    auto Images = m_imageReader->ReadMultiMemory();
    m_imBarrierDst[frame_index].resize(Images.size(), nullptr);
    m_imBarrierOpt[frame_index].resize(Images.size(), nullptr);
    for (size_t i = 0; i < Images.size(); ++i) {
        if (m_imImageBuffers[frame_index].size() < i + 1) {
            auto modelBuffer = m_imPipeline->CreateBuffer(HUH::RHI::Buffer::Type::UNIFORM, sizeof(HUH::Matrix4x4f), 0);
            m_memoryAllocator->Allocate(modelBuffer,
                                        HUH::RHI::MemoryAllocator::Device | HUH::RHI::MemoryAllocator::Host);
            m_imUniformModelBuffers[frame_index].push_back(modelBuffer);
        }
        if (m_imImageBuffers[frame_index].size() < i + 1) {
            auto srcImageBuffer = m_imPipeline->CreateBuffer(
                HUH::RHI::Buffer::Type::SRC, sizeof(HUH::Vector4u8) * Images[i].Size.Height() * Images[i].Size.Width());
            m_memoryAllocator->Allocate(srcImageBuffer,
                                        HUH::RHI::MemoryAllocator::Host | HUH::RHI::MemoryAllocator::Device);
            m_imImagesTransferBuffer[frame_index].push_back(srcImageBuffer);

            auto dstImage =
                m_imPipeline->CreateImage(HUH::RHI::Image::Type::DST | HUH::RHI::Image::Sampled, Images[i].Size, 1);
            m_memoryAllocator->Allocate(dstImage, HUH::RHI::MemoryAllocator::Device);
            dstImage->Init({HUH::RHI::Format::R8G8B8A8_SRGB, 1, Images[i].Size});
            m_imImageBuffers[frame_index].push_back(dstImage);
        } else if (m_imImageBuffers[frame_index][i]->GetSize() != Images[i].Size) {
            m_imPipeline->DestroyBuffer(m_imImagesTransferBuffer[frame_index][i]);
            m_imPipeline->DestroyImage(m_imImageBuffers[frame_index][i]);

            auto srcImageBuffer = m_imPipeline->CreateBuffer(
                HUH::RHI::Buffer::Type::SRC, sizeof(HUH::Vector4u8) * Images[i].Size.Height() * Images[i].Size.Width());
            m_memoryAllocator->Allocate(srcImageBuffer,
                                        HUH::RHI::MemoryAllocator::Host | HUH::RHI::MemoryAllocator::Device);
            m_imImagesTransferBuffer[frame_index][i] = srcImageBuffer;

            auto dstImage =
                m_imPipeline->CreateImage(HUH::RHI::Image::Type::DST | HUH::RHI::Image::Sampled, Images[i].Size, 1);
            m_memoryAllocator->Allocate(dstImage, HUH::RHI::MemoryAllocator::Device);
            dstImage->Init({HUH::RHI::Format::R8G8B8A8_SRGB, 1, Images[i].Size});
            m_imImageBuffers[frame_index][i] = dstImage;
        }
        m_imImagesTransferBuffer[frame_index][i]->UploadData(Images[i].Pixels.data());
    }
}

bool MainWindow::ReadLidar() {
    bool failed = false;
    auto lidarData = m_lidarReader->readData(failed);
    if (failed) {
        return !failed;
    }
    if (m_lidarVertexBuffers[frame_index] == nullptr || m_lidarSizes[frame_index] != lidarData.size()) {
        m_lidarVertexBuffers[frame_index] =
            m_lidarPipeline->CreateBuffer(HUH::RHI::Buffer::Type::VERTEX, sizeof(LidarVertex) * lidarData.size());
        m_memoryAllocator->Allocate(m_lidarVertexBuffers[frame_index],
                                    HUH::RHI::MemoryAllocator::Device | HUH::RHI::MemoryAllocator::Host);
        m_lidarSizes[frame_index] = lidarData.size();
    }
    m_lidarVertexBuffers[frame_index]->UploadData(lidarData.data());
    return true;
}

void MainWindow::RecordImageBufferCopy() {
    for (size_t i = 0; i < m_imImageBuffers[frame_index].size(); ++i) {
        if (m_imBarrierDst[frame_index][i]) {
            m_imBarrierDst[frame_index][i]->Destroy();
        }
        m_imBarrierDst[frame_index][i] = m_gpu->CreateBarrier();
        m_imBarrierDst[frame_index][i]->Init({
            .Image = m_imImageBuffers[frame_index][i],
            .Transitions =
                {
                    {.srcStage = HUH::RHI::Pipeline::Stages::TopOfPipe,
                     .dstStage = HUH::RHI::Pipeline::Stages::Transfer,
                     .srcAccess = HUH::RHI::AccessType::Unknown,
                     .dstAccess = HUH::RHI::AccessType::TransferWrite,
                     .srcLayout = HUH::RHI::RenderPass::Layout::Unknown,
                     .dstLayout = HUH::RHI::RenderPass::Layout::TransferDst},
                },
        });
        if (m_imBarrierOpt[frame_index][i]) {
            m_imBarrierOpt[frame_index][i]->Destroy();
        }
        m_imBarrierOpt[frame_index][i] = m_gpu->CreateBarrier();
        m_imBarrierOpt[frame_index][i]->Init({
            .Image = m_imImageBuffers[frame_index][i],
            .Transitions =
                {
                    {.srcStage = HUH::RHI::Pipeline::Stages::Transfer,
                     .dstStage = HUH::RHI::Pipeline::Stages::FragmentShader,
                     .srcAccess = HUH::RHI::AccessType::TransferWrite,
                     .dstAccess = HUH::RHI::AccessType::ShaderRead,
                     .srcLayout = HUH::RHI::RenderPass::Layout::TransferDst,
                     .dstLayout = HUH::RHI::RenderPass::Layout::ShaderReadOnly},
                },
        });

        (*m_mainCommandPool)[frame_index]->BindBarrier(m_imBarrierDst[frame_index][i]);
        (*m_mainCommandPool)[frame_index]->CopyBuffer(m_imImagesTransferBuffer[frame_index][i],
                                                      m_imImageBuffers[frame_index][i]);
        (*m_mainCommandPool)[frame_index]->BindBarrier(m_imBarrierOpt[frame_index][i]);
    }
}