#include <HUH/Math/functions.h>
#include <HUH/Math/vector.h>
#include <cuda/atomic>
#include <curanddx.hpp>
#include <cusolverdx.hpp>

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

using EIG750 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<750>() + cusolverdx::Thread());

using EIG800 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<800>() + cusolverdx::Thread());

using EIG860 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<860>() + cusolverdx::Thread());

using EIG870 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<870>() + cusolverdx::Thread());

using EIG890 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<890>() + cusolverdx::Thread());

using EIG900 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<900>() + cusolverdx::Thread());

using EIG1000 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<1000>() + cusolverdx::Thread());

using EIG1100 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<1100>() + cusolverdx::Thread());

using EIG1200 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<1200>() + cusolverdx::Thread());

using EIG1210 =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<1210>() + cusolverdx::Thread());

#define RANDOM_PLANE_KERNEL(SM) \
__global__ void RandomPlane## SM(LidarVertex* vertices, \
                                HUH::Uint32 numVertices, \
                                HUH::Vector4f* planes, \
                                HUH::Uint64 seed, \
                                HUH::Uint64 offset, \
                                HUH::Uint32* indices, \
                                HUH::Uint32* indicesNumber) { \
    RandomPlane<RNG## SM>(vertices, numVertices, planes, seed, offset, indices, indicesNumber); \
}

#define EIG_KERNEL(SM) \
__global__ void EigKernel## SM(HUH::Matrix4x4f* mat, float* lambda, float* workspace, int* info) { \
    EigKernel<EIG## SM>(mat,lambda,workspace,info); \
}

template<typename RNG>
__device__ void RandomPlane(LidarVertex* vertices,
                            HUH::Uint32 numVertices,
                            HUH::Vector4f* planes,
                            HUH::Uint64 seed,
                            HUH::Uint64 offset,
                            HUH::Uint32* indices,
                            HUH::Uint32* indicesNumber) {
    const auto i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i > numVertices) {
        return;
    }

    RNG rng(seed, 0, offset + i);

    curanddx::uniform<float> dist(0, static_cast<float>(*indicesNumber));
    auto random = dist.generate4(rng);

    // TODO same index ?
    auto r1 = static_cast<uint>(random.x);
    auto r2 = static_cast<uint>(random.y);
    auto r3 = static_cast<uint>(random.z);
    // auto r4 = static_cast<uint>(random.w);

    auto i1 = indices[r1];
    auto i2 = indices[r2];
    auto i3 = indices[r3];
    // auto i4 = indices[r4];

    auto p1 = vertices[i1].pos;
    auto p2 = vertices[i2].pos;
    auto p3 = vertices[i3].pos;
    auto v1 = HUH::Vector3f(p2 - p1);
    auto v2 = HUH::Vector3f(p3 - p1);
    planes[i] = HUH::Vector4f(v1.Cross(v2));
    planes[i].Normalize();
    planes[i].W() = -(planes->X() * p1.X() + planes->Y() * p1.Y() + planes->Z() * p1.Z());
}

template<typename EIG>
__device__ void EigKernel(HUH::Matrix4x4f* mat, float* lambda, float* workspace, int* info) {

    EIG().execute((float*)mat, lambda, workspace, info);
}

template<typename T>
HUH_FORCE_INLINE __device__ T WarpReduceSum(T val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }
    return val;
}

template<typename T>
HUH_FORCE_INLINE __device__ T WarpReduceMax(T val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        val = HUH::Max(val, __shfl_down_sync(0xffffffff, val, offset));
    }
    return val;
}

template<typename T, size_t N>
HUH_FORCE_INLINE __device__ HUH::Vector<T, N> WarpReduceMax(HUH::Vector<T, N> val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        for (size_t i = 0; i < N; i++) {
            val[i] = HUH::Max(val[i], __shfl_down_sync(0xffffffff, val[i], offset));
        }
    }
    return val;
}

template<typename T>
HUH_FORCE_INLINE __device__ T WarpReduceMin(T val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        val = HUH::Min(val, __shfl_down_sync(0xffffffff, val, offset));
    }
    return val;
}

template<typename T, size_t N>
HUH_FORCE_INLINE __device__ HUH::Vector<T, N> WarpReduceMin(HUH::Vector<T, N> val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        for (size_t i = 0; i < N; i++) {
            val[i] = HUH::Min(val[i], __shfl_down_sync(0xffffffff, val[i], offset));
        }
    }
    return val;
}

template<size_t N>
    requires(N >= 3)
HUH_FORCE_INLINE __device__ float PlanePointDistance(const HUH::Vector4f& plane, const HUH::Vector<float, N>& point) {

    auto res = fmaf(plane.Z(), point.data[2], plane.W());
    res = fmaf(plane.Y(), point.data[1], res);
    res = fmaf(plane.X(), point.data[0], res);
    return abs(res) * rnorm3df(plane.X(), plane.Y(), plane.Z());
}

extern "C" {

RANDOM_PLANE_KERNEL(750)
RANDOM_PLANE_KERNEL(800)
RANDOM_PLANE_KERNEL(860)
RANDOM_PLANE_KERNEL(870)
RANDOM_PLANE_KERNEL(890)
RANDOM_PLANE_KERNEL(900)
RANDOM_PLANE_KERNEL(1000)
RANDOM_PLANE_KERNEL(1100)
RANDOM_PLANE_KERNEL(1200)
RANDOM_PLANE_KERNEL(1210)

EIG_KERNEL(750)
EIG_KERNEL(800)
EIG_KERNEL(860)
EIG_KERNEL(870)
EIG_KERNEL(890)
EIG_KERNEL(900)
EIG_KERNEL(1000)
EIG_KERNEL(1100)
EIG_KERNEL(1200)
EIG_KERNEL(1210)

__global__ void LidarMinMax(LidarVertex* vertices, HUH::Uint32 count, HUH::Vector4f* MinMax) {
    HUH::Vector4f threadMax(-INFINITY);
    HUH::Vector4f threadMin(INFINITY);

    extern __shared__ float shared[];

    auto sharedMax = reinterpret_cast<HUH::Vector4f*>(shared);
    auto sharedMin = sharedMax + 32;

    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;

    if (workIndex < count) {
        threadMax = vertices[workIndex].pos;
        threadMin = vertices[workIndex].pos;
    }

    threadMax = WarpReduceMax(threadMax);

    threadMin = WarpReduceMin(threadMin);

    if (threadIdx.x % 32 == 0) {
        sharedMax[threadIdx.x / 32] = threadMax;
        sharedMin[threadIdx.x / 32] = threadMin;
    }
    cuda::atomic_ref<float, cuda::thread_scope_device> minRef1(MinMax[0][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> minRef2(MinMax[0][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> minRef3(MinMax[0][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> minRef4(MinMax[0][3]);

    cuda::atomic_ref<float, cuda::thread_scope_device> maxRef1(MinMax[1][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> maxRef2(MinMax[1][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> maxRef3(MinMax[1][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> maxRef4(MinMax[1][3]);

    __syncthreads();

    if (threadIdx.x < 32) {

        HUH::Vector4f warpMax =
            threadIdx.x < (blockDim.x + 31) / 32 ? sharedMax[threadIdx.x] : HUH::Vector4f(-INFINITY);
        HUH::Vector4f warpMin = threadIdx.x < (blockDim.x + 31) / 32 ? sharedMin[threadIdx.x] : HUH::Vector4f(INFINITY);

        warpMax = WarpReduceMax(warpMax);

        warpMin = WarpReduceMin(warpMin);

        if (threadIdx.x == 0) {

            (void)maxRef1.fetch_max(warpMax[0]);
            (void)maxRef2.fetch_max(warpMax[1]);
            (void)maxRef3.fetch_max(warpMax[2]);
            (void)maxRef4.fetch_max(warpMax[3]);

            (void)minRef1.fetch_min(warpMin[0]);
            (void)minRef2.fetch_min(warpMin[1]);
            (void)minRef3.fetch_min(warpMin[2]);
            (void)minRef4.fetch_min(warpMin[3]);
        }
    }
}

__global__ void Lidar2DMap(LidarVertex* vertices,
                           HUH::Uint32 count,
                           HUH::Vector4f* MinMax,
                           float voxelSize,
                           HUH::Uint32* indices,
                           HUH::Uint32* indicesNumber) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    auto avg = (MinMax[0] + MinMax[1]) / 2;
    int x = static_cast<int>(MinMax[0].X()) + static_cast<int>(blockDim.y * blockIdx.y);
    int y = static_cast<int>(MinMax[0].Y()) + static_cast<int>(blockDim.z * blockIdx.z);

    float threadMinZ(INFINITY);
    HUH::Uint32 threadIndex = 0;

    __shared__ float minBlock[32];
    __shared__ HUH::Uint32 indexBlock[32];

    if (workIndex < count) {
        if (static_cast<float>(x) <= vertices[workIndex].pos.X()
            && static_cast<float>(x + 1) > vertices[workIndex].pos.X()
            && static_cast<float>(y) <= vertices[workIndex].pos.Y()
            && static_cast<float>(y + 1) > vertices[workIndex].pos.Y()) {
            threadMinZ = vertices[workIndex].pos.Z();
            threadIndex = workIndex;
        }
    }

    for (int offset = 16; offset > 0; offset /= 2) {
        auto tmpZ = __shfl_down_sync(0xffffffff, threadMinZ, offset);
        auto tmpIndex = __shfl_down_sync(0xffffffff, threadIndex, offset);
        if (tmpZ < threadMinZ) {
            threadMinZ = tmpZ;
            threadIndex = tmpIndex;
        }
    }

    if (threadIdx.x % 32 == 0) {
        minBlock[threadIdx.x / 32] = threadMinZ;
        indexBlock[threadIdx.x / 32] = threadIndex;
    }

    __syncthreads();

    cuda::atomic_ref<HUH::Uint32, cuda::thread_scope_device> index(*indicesNumber);

    if (threadIdx.x < 32) {

        auto warpMinZ = threadIdx.x < (blockDim.x + 31) / 32 ? minBlock[threadIdx.x] : INFINITY;
        auto warpIndex = threadIdx.x < (blockDim.x + 31) / 32 ? indexBlock[threadIdx.x] : 0;

        for (int offset = 16; offset > 0; offset /= 2) {
            auto tmpZ = __shfl_down_sync(0xffffffff, warpMinZ, offset);
            auto tmpIndex = __shfl_down_sync(0xffffffff, warpIndex, offset);
            if (tmpZ < warpMinZ) {
                warpMinZ = tmpZ;
                warpIndex = tmpIndex;
            }
        }

        if (threadIdx.x == 0) {
            if (warpMinZ < (MinMax[0].Z() + 1)) {
                auto i = index.fetch_add(1);
                indices[i] = warpIndex;
            }
        }
    }
}

__global__ void PlaneRansacSum(HUH::Vector4f* vertices,
                               const HUH::Uint32 count,
                               HUH::Vector4f* planes,
                               HUH::Uint32* inlinersSum,
                               HUH::Vector4f* MinMax,
                               float threshold) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    auto planeIndex = blockDim.y * blockIdx.y;

    HUH::Uint32 isInliner = 0;
    HUH::Vector4f Center{0};
    if (workIndex >= count) {
        return;
    }

    if (vertices[workIndex].Z() > MinMax[0].Z() + 2) {
        return;
    }

    if ((vertices[workIndex] - Center).Norm() < 0.5) {
        return;
    }

    __shared__ HUH::Uint32 blockLocal[32];

    isInliner = PlanePointDistance(planes[planeIndex], vertices[workIndex]) < threshold ? 1 : 0;

    isInliner = WarpReduceSum(isInliner);

    if (threadIdx.x % 32 == 0) {
        blockLocal[threadIdx.x / 32] = isInliner;
    }

    __syncthreads();

    if (threadIdx.x < 32) {
        HUH::Uint32 val = threadIdx.x < (blockDim.x + 31) / 32 ? blockLocal[threadIdx.x] : 0;
        val = WarpReduceSum(val);

        if (threadIdx.x == 0) {
            atomicAdd(&inlinersSum[planeIndex], val);
        }
    }
}

__device__ int lock = 0;

__global__ void PlaneMax(const HUH::Uint32* inlinersSum, const HUH::Uint32 iter, HUH::Uint32* indexMax) {

    HUH::Uint32 threadMax = 0;
    HUH::Uint32 threadIndex = 0;
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    if (workIndex < iter) {
        threadIndex = workIndex;
        threadMax = inlinersSum[threadIndex];
    }

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
        maxBlock[threadIdx.x / 32] = threadMax;
        indexBlock[threadIdx.x / 32] = threadIndex;
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
            while (atomicCAS(&lock, 0, 1) != 0) {
            }
            if (warpMax > maxRef) {

                maxRef.store(warpMax);
                indexRef.store(warpIndex);
            }
            __threadfence();
            atomicExch(&lock, 0);
        }
    }
}

__global__ void PlaneColor(LidarVertex* vertices,
                           HUH::Vector4f* planes,
                           const HUH::Uint32 count,
                           HUH::Vector4f color,
                           const HUH::Uint32* indexMax) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;

    if (workIndex >= count) {
        return;
    }

    if (PlanePointDistance(planes[*indexMax], vertices[workIndex].pos) < 0.5f) {
        vertices[workIndex].color = color;
    }
}
}