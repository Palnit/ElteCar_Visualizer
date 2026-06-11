#include <HUH/Math/vector.h>
#include <curanddx.hpp>

struct LidarVertex {
    HUH::Vector3f pos;
    HUH::Vector3f color;
};
using RNG750 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<750>() + curanddx::Thread());
using RNG800 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<800>() + curanddx::Thread());
using RNG860 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<860>() + curanddx::Thread());
using RNG870 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<870>() + curanddx::Thread());
using RNG890 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<890>() + curanddx::Thread());
using RNG900 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<900>() + curanddx::Thread());
using RNG1000 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<1000>() + curanddx::Thread());
using RNG1100 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<1100>() + curanddx::Thread());
using RNG1200 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<1200>() + curanddx::Thread());
using RNG1210 = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<1210>() + curanddx::Thread());

#define DEFINE_RANDOM_PLANE_KERNEL(SM) \
__global__ void RandomPlane## SM(LidarVertex* vertices, \
                                HUH::Uint32 numVertices, \
                                HUH::Vector4f* planes, \
                                HUH::Uint64 seed, \
                                HUH::Uint64 offset) { \
    RandomPlane<RNG## SM>(vertices, numVertices, planes, seed, offset); \
}

template<typename RNG>
__device__ void RandomPlane(LidarVertex* vertices,
                            HUH::Uint32 numVertices,
                            HUH::Vector4f* planes,
                            HUH::Uint64 seed,
                            HUH::Uint64 offset) {
    const auto i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i > numVertices) {
        return;
    }

    RNG rng(seed, 0, offset + i);

    curanddx::uniform<float> dist(0, static_cast<float>(numVertices));
    auto random = dist.generate4(rng);
    auto p1 = vertices[static_cast<uint>(random.x)].pos;
    auto p2 = vertices[static_cast<uint>(random.y)].pos;
    auto p3 = vertices[static_cast<uint>(random.z)].pos;

    planes[i].X() = (p2.Y() - p1.Y()) * (p3.Z() - p1.Z()) - (p2.Z() - p1.Z()) * (p3.Y() - p2.Y());
    planes[i].Y() = (p2.Z() - p1.Z()) * (p3.X() - p1.X()) - (p2.X() - p1.X()) * (p3.Z() - p2.Z());
    planes[i].Z() = (p2.X() - p1.X()) * (p3.Y() - p1.Y()) - (p2.Y() - p1.Y()) * (p3.X() - p1.X());
    planes[i].W() = -(planes[i].X() * p1.X() + planes[i].Y() * p1.Y() + planes[i].Z() * p1.Z());
}

extern "C" {

DEFINE_RANDOM_PLANE_KERNEL(750)
DEFINE_RANDOM_PLANE_KERNEL(800)
DEFINE_RANDOM_PLANE_KERNEL(860)
DEFINE_RANDOM_PLANE_KERNEL(870)
DEFINE_RANDOM_PLANE_KERNEL(890)
DEFINE_RANDOM_PLANE_KERNEL(900)
DEFINE_RANDOM_PLANE_KERNEL(1000)
DEFINE_RANDOM_PLANE_KERNEL(1100)
DEFINE_RANDOM_PLANE_KERNEL(1200)
DEFINE_RANDOM_PLANE_KERNEL(1210)

__global__ void PlaneRansac(LidarVertex* vertices,
                            HUH::Uint32 count,
                            HUH::Vector3f color,
                            HUH::Vector4f* planes,
                            HUH::Uint64 num_inliners) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    if (workIndex >= count) {
        return;
    }

    extern __shared__ HUH::Uint8 inliners[];

    vertices[workIndex].color = color;
}
}