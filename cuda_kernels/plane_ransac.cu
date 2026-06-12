#include <HUH/Math/vector.h>
#include <cuda/atomic>
#include <curanddx.hpp>

struct LidarVertex {
    alignas(16) HUH::Vector4f pos;
    alignas(16) HUH::Vector4f color;
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

HUH_FORCE_INLINE __device__ HUH::Uint32 warpReduceSum(HUH::Uint32 val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }
    return val;
}

__global__ void PlaneRansacSum(LidarVertex* vertices,
                               HUH::Uint32 count,
                               HUH::Vector4f* planes,
                               HUH::Uint32* inlinersSum,
                               HUH::Uint32* inliners,
                               float threshold) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    auto planeIndex = blockDim.y * blockIdx.y;

    HUH::Uint32 isInliner = 0;
    if (workIndex >= count) {
        return;
    }

    __shared__ HUH::Uint32 blockLocal[32];

    // printf("distance: %f\n",
    //        abs(fmaf(planes[planeIndex].X(), vertices[workIndex].pos.X(),
    //                 fmaf(planes[planeIndex].Y(), vertices[workIndex].pos.Y(),
    //                      fmaf(planes[planeIndex].Z(), vertices[workIndex].pos.Z(), planes[planeIndex].W()))))
    //            * rnorm3df(planes[planeIndex].X(), planes[planeIndex].Y(), planes[planeIndex].Z()));

    isInliner = abs(fmaf(planes[planeIndex].X(), vertices[workIndex].pos.X(),
                         fmaf(planes[planeIndex].Y(), vertices[workIndex].pos.Y(),
                              fmaf(planes[planeIndex].Z(), vertices[workIndex].pos.Z(), planes[planeIndex].W()))))
                * rnorm3df(planes[planeIndex].X(), planes[planeIndex].Y(), planes[planeIndex].Z())
            < threshold
        ? 1
        : 0;

    auto index = threadIdx.x / 32 + blockIdx.x;
    auto bit = threadIdx.x % 32;

    HUH::Uint32 bitfield = isInliner << bit;

    atomicOr(inliners + index + gridDim.x * planeIndex, bitfield);

    isInliner = warpReduceSum(isInliner);

    if (threadIdx.x % 32 == 0) {
        blockLocal[threadIdx.x / 32] = isInliner;
    }

    __syncthreads();

    if (threadIdx.x < 32) {
        HUH::Uint32 val = threadIdx.x < (blockDim.x + 31) / 32 ? blockLocal[threadIdx.x] : 0;
        val = warpReduceSum(val);

        if (threadIdx.x == 0) {
            atomicAdd(&inlinersSum[planeIndex], val);
        }
    }
}

__global__ void PlaneMax(const HUH::Uint32* inlinersSum, const HUH::Uint32 iter, HUH::Uint32* indexMax) {

    HUH::Uint32 threadMax = 0;
    HUH::Uint32 threadIndex = 0;
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    if (workIndex >= iter) {
        return;
    }

    threadIndex = workIndex;
    threadMax = inlinersSum[threadIndex];

    __shared__ HUH::Uint32 maxBlock[32];
    __shared__ HUH::Uint32 indexBlock[32];

    for (int offset = 16; offset > 0; offset /= 2) {
        auto upMax = __shfl_down_sync(0xffffffff, threadMax, offset);
        auto upIndex = __shfl_down_sync(0xffffffff, threadIndex, offset);
        if (upMax > threadMax) {
            threadMax = upMax;
            threadIndex = upIndex;
        }
    }

    if (threadIdx.x % 32 == 0) {
        maxBlock[threadIndex] = threadMax;
        indexBlock[threadIndex] = threadIndex;
    }

    __syncthreads();

    cuda::atomic_ref<HUH::Uint32, cuda::thread_scope_device> indexRef(indexMax[0]);
    cuda::atomic_ref<HUH::Uint32, cuda::thread_scope_device> maxRef(indexMax[1]);

    if (threadIdx.x < 32) {
        HUH::Uint32 warpMax = threadIdx.x < (blockDim.x + 31) / 32 ? maxBlock[threadIdx.x] : 0;
        HUH::Uint32 warpIndex = threadIdx.x < (blockDim.x + 31) / 32 ? indexBlock[threadIdx.x] : 0;
        for (int offset = 16; offset > 0; offset /= 2) {
            auto upMax = __shfl_down_sync(0xffffffff, warpMax, offset);
            auto upIndex = __shfl_down_sync(0xffffffff, warpIndex, offset);

            if (upMax > warpMax) {
                warpMax = upMax;
                warpIndex = upIndex;
            }
        }

        if (threadIdx.x == 0) {
            if (warpMax > maxRef) {
                maxRef.store(warpMax);
                indexRef.store(warpIndex);
            }
        }
    }
}

__global__ void PlaneColor(LidarVertex* vertices,
                           const HUH::Uint32 count,
                           HUH::Vector4f color,
                           const HUH::Uint32* inliners,
                           const HUH::Uint32* indexMax) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    if (workIndex >= count) {
        return;
    }

    auto index = threadIdx.x / 32 + blockIdx.x;
    auto bit = threadIdx.x % 32;

    HUH::Uint32 bitfield = 1 << bit;
    if ((*(inliners + index + gridDim.x * indexMax[0]) & bitfield) != 0) {
        vertices[workIndex].color = color;
    }
}
}