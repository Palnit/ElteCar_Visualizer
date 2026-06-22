#include <HUH/Math/functions.h>
#include <HUH/Math/matrix.h>
#include <HUH/Math/vector.h>
#include <cuda/atomic>
#include <curanddx.hpp>
#include <cusolverdx.hpp>

struct LidarVertex {
    alignas(16) HUH::Vector4f pos;
    alignas(16) HUH::Vector4f color;
};

struct EigData {
    alignas(16) HUH::Matrix4x4f mat{0};
    alignas(16) HUH::Vector4f lambda{0};
    float workspace[6];
    int info;
};

template<unsigned int SM>
using RNG = decltype(curanddx::Generator<curanddx::philox4_32>() + curanddx::SM<SM>() + curanddx::Thread());

template<unsigned int SM>
using EIG =
    decltype(cusolverdx::Size<4>() + cusolverdx::Precision<float>() + cusolverdx::Type<cusolverdx::type::real>()
             + cusolverdx::Function<cusolverdx::heev>() + cusolverdx::FillMode<cusolverdx::fill_mode::upper>()
             + cusolverdx::Arrangement<cusolverdx::arrangement::row_major>()
             + cusolverdx::Job<cusolverdx::job::overwrite_vectors>() + cusolverdx::SM<SM>() + cusolverdx::Thread());

#define RNG_CASE(SM) \
case SM: { \
    RNG<SM> rng## SM(seed, 0, offset + i); \
    random = dist.generate4(rng## SM); \
    break; \
}

#define EIG_CASE(SM) \
case SM: { \
    EIG<SM>().execute((float*)&eigData[i].mat, (float*)&eigData[i].lambda, eigData[i].workspace, &eigData[i].info); \
    break; \
}

#define RANDOM_PLANE_KERNEL(SM) \
__global__ void RandomPlane## SM(LidarVertex* vertices, \
                                HUH::Uint32 numVertices, \
                                HUH::Vector4f* planes, \
                                HUH::Uint64 seed, \
                                HUH::Uint64 offset, \
                                HUH::Uint32* indices, \
                                HUH::Uint32* indicesNumber) { \
    RandomPlane<RNG<SM>>(vertices, numVertices, planes, seed, offset, indices, indicesNumber); \
}

__global__ void PlaneEquation(HUH::Vector4f* points, HUH::Matrix4x4f* res) {
    // TODO
}

template<typename T>
HUH_FORCE_INLINE __device__ T WarpReduceSum(T val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        val += __shfl_down_sync(0xffffffff, val, offset);
    }
    return val;
}

template<typename T, size_t N>
HUH_FORCE_INLINE __device__ HUH::Vector<T, N> WarpReduceSum(HUH::Vector<T, N> val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        for (size_t i = 0; i < N; i++) {
            val[i] += __shfl_down_sync(0xffffffff, val[i], offset);
        }
    }
    return val;
}

template<typename T, size_t N, size_t M>
HUH_FORCE_INLINE __device__ HUH::Matrix<T, N, M> WarpReduceSum(HUH::Matrix<T, N, M> val) {

    for (int offset = 16; offset > 0; offset /= 2) {
        for (size_t i = 0; i < N; i++) {
            val[i] += WarpReduceSum(val[i]);
        }
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

__global__ void EigKernel(EigData* eigData, HUH::Uint32 batches, unsigned int sm) {

    const auto i = threadIdx.x + blockIdx.x * blockDim.x;
    if (i >= batches) {
        return;
    }

    switch (sm) {
        EIG_CASE(750)
        EIG_CASE(800)
        EIG_CASE(860)
        EIG_CASE(870)
        EIG_CASE(890)
        EIG_CASE(900)
        EIG_CASE(1000)
        EIG_CASE(1100)
        EIG_CASE(1200)
        EIG_CASE(1210)
        default: {
            EIG<750>().execute((float*)&eigData[i].mat, (float*)&eigData[i].lambda, eigData[i].workspace,
                               &eigData[i].info);
            break;
        }
    }
}

__global__ void RandomPlane(LidarVertex* vertices,
                            HUH::Uint32 numVertices,
                            EigData* eigData,
                            HUH::Uint64 seed,
                            HUH::Uint64 offset,
                            HUH::Uint32* indices,
                            HUH::Uint32* indicesNumber,
                            unsigned int sm) {
    const auto i = threadIdx.x + blockDim.x * blockIdx.x;
    if (i > numVertices) {
        return;
    }

    curanddx::uniform<float> dist(0, static_cast<float>(*indicesNumber));

    float4 random;
    switch (sm) {
        RNG_CASE(750)
        RNG_CASE(800)
        RNG_CASE(860)
        RNG_CASE(870)
        RNG_CASE(890)
        RNG_CASE(900)
        RNG_CASE(1000)
        RNG_CASE(1100)
        RNG_CASE(1200)
        RNG_CASE(1210)
        default: {
            RNG<750> rngDef(seed, 0, offset + i);
            random = dist.generate4(rngDef);
            break;
        }
    }

    auto r1 = static_cast<uint>(random.x);
    auto r2 = static_cast<uint>(random.y);
    auto r3 = static_cast<uint>(random.z);
    auto r4 = static_cast<uint>(random.w);

    auto i1 = indices[r1];
    auto i2 = indices[r2];
    auto i3 = indices[r3];
    auto i4 = indices[r4];

    HUH::Vector4f poss[] = {vertices[i1].pos, vertices[i2].pos, vertices[i3].pos, vertices[i4].pos};
    auto& eig = eigData[i];

    for (auto& pos : poss) {
        eig.mat[0][0] += pos.X() * pos.X();
        eig.mat[0][1] += pos.X() * pos.Y();
        eig.mat[0][2] += pos.X() * pos.Z();
        eig.mat[0][3] += pos.X();

        eig.mat[1][0] += pos.X() * pos.Y();
        eig.mat[1][1] += pos.Y() * pos.Y();
        eig.mat[1][2] += pos.Y() * pos.Z();
        eig.mat[1][3] += pos.Y();

        eig.mat[2][0] += pos.X() * pos.Z();
        eig.mat[2][1] += pos.Y() * pos.Z();
        eig.mat[2][2] += pos.Z() * pos.Z();
        eig.mat[2][3] += pos.Z();

        eig.mat[3][0] += pos.X();
        eig.mat[3][1] += pos.Y();
        eig.mat[3][2] += pos.Z();
        eig.mat[3][3] += 1;
    }

    // auto plane = eig.mat.GetTransposed()[0];
    // if (i == 0) {
    //     printf("Test: %f,%f,%f,%f", plane.X(), plane.Y(), plane.Z(), plane.W());
    // }
    //
    // planes[i] = mat.Transpose()[0];

    // auto v1 = HUH::Vector3f(poss[1] - poss[0]);
    // auto v2 = HUH::Vector3f(poss[2] - poss[0]);
    // planes[i] = HUH::Vector4f(v1.Cross(v2));
    // planes[i].Normalize();
    // planes[i].W() = -(planes->X() * poss[0].X() + planes->Y() * poss[0].Y() + planes->Z() * poss[0].Z());
}

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
                               EigData* eigData,
                               HUH::Uint32* inlinersSum,
                               HUH::Vector4f* MinMax,
                               float threshold) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    auto planeIndex = blockDim.y * blockIdx.y;

    HUH::Uint32 isInliner = 0;
    HUH::Vector4f Center{0};

    if (workIndex < count && vertices[workIndex].Z() < MinMax[0].Z() + 2
        && (vertices[workIndex] - Center).Norm() > 0.5) {
        auto plane = eigData[planeIndex].mat.GetTransposed()[0];

        // if (planeIndex == 0 && workIndex < 32) {
        //     printf("Plane: %f,%f,%f,%f\n", plane.X(), plane.Y(), plane.Z(), plane.W());
        // }

        isInliner = PlanePointDistance(plane, vertices[workIndex]) < threshold ? 1 : 0;
    }

    __shared__ HUH::Uint32 blockLocal[32];

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

__global__ void RecalcPlane(LidarVertex* vertices,
                            EigData* eigData,
                            const HUH::Uint32 count,
                            EigData* eigDataOut,
                            const HUH::Uint32* indexMax,
                            float threshold) {

    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;

    HUH::Matrix4x4f mat{0};
    HUH::Vector4f pos{INFINITY};
    auto plane = eigData[*indexMax].mat.GetTransposed()[0];
    if (workIndex < count) {
        pos = vertices[workIndex].pos;
    }

    // printf("Point: [%f,%f,%f,%f], Plane:[%f,%f,%f,%f], Distance: %f, thres? %f Index: %d\n", pos[0], pos[1], pos[2],
    //        pos[3], plane[0], plane[1], plane[2], plane[3], PlanePointDistance(plane, pos), threshold, *indexMax);
    if (PlanePointDistance(plane, pos) < threshold) {
        mat[0][0] = pos.X() * pos.X();
        mat[0][1] = pos.X() * pos.Y();
        mat[0][2] = pos.X() * pos.Z();
        mat[0][3] = pos.X();

        mat[1][0] = pos.X() * pos.Y();
        mat[1][1] = pos.Y() * pos.Y();
        mat[1][2] = pos.Y() * pos.Z();
        mat[1][3] = pos.Y();

        mat[2][0] = pos.X() * pos.Z();
        mat[2][1] = pos.Y() * pos.Z();
        mat[2][2] = pos.Z() * pos.Z();
        mat[2][3] = pos.Z();

        mat[3][0] = pos.X();
        mat[3][1] = pos.Y();
        mat[3][2] = pos.Z();
        mat[3][3] = 1;
    }

    extern __shared__ float shared[];

    auto blockLocal = reinterpret_cast<HUH::Matrix4x4f*>(shared);

    mat = WarpReduceSum(mat);

    if (threadIdx.x % 32 == 0) {
        blockLocal[threadIdx.x / 32] = mat;
    }

    __syncthreads();

    cuda::atomic_ref<float, cuda::thread_scope_device> r00(eigDataOut->mat[0][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r01(eigDataOut->mat[0][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r02(eigDataOut->mat[0][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r03(eigDataOut->mat[0][3]);

    cuda::atomic_ref<float, cuda::thread_scope_device> r10(eigDataOut->mat[1][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r11(eigDataOut->mat[1][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r12(eigDataOut->mat[1][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r13(eigDataOut->mat[1][3]);

    cuda::atomic_ref<float, cuda::thread_scope_device> r20(eigDataOut->mat[2][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r21(eigDataOut->mat[2][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r22(eigDataOut->mat[2][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r23(eigDataOut->mat[2][3]);

    cuda::atomic_ref<float, cuda::thread_scope_device> r30(eigDataOut->mat[3][0]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r31(eigDataOut->mat[3][1]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r32(eigDataOut->mat[3][2]);
    cuda::atomic_ref<float, cuda::thread_scope_device> r33(eigDataOut->mat[3][3]);

    if (threadIdx.x < 32) {
        HUH::Matrix4x4f val = threadIdx.x < (blockDim.x + 31) / 32 ? blockLocal[threadIdx.x] : HUH::Matrix4x4f{0};
        val = WarpReduceSum(val);

        // printf("R00 %f,R01 %f,R02 %f,R03 %f\n", val[0][0], val[0][1], val[0][2], val[0][3]);
        if (threadIdx.x == 0) {
            r00.fetch_add(val[0][0]);
            r01.fetch_add(val[0][1]);
            r02.fetch_add(val[0][2]);
            r03.fetch_add(val[0][3]);

            r10.fetch_add(val[1][0]);
            r11.fetch_add(val[1][1]);
            r12.fetch_add(val[1][2]);
            r13.fetch_add(val[1][3]);

            r20.fetch_add(val[2][0]);
            r21.fetch_add(val[2][1]);
            r22.fetch_add(val[2][2]);
            r23.fetch_add(val[2][3]);

            r30.fetch_add(val[3][0]);
            r31.fetch_add(val[3][1]);
            r32.fetch_add(val[3][2]);
            r33.fetch_add(val[3][3]);
        }
    }
}

__global__ void PlaneColor(LidarVertex* vertices,
                           EigData* eigData,
                           const HUH::Uint32 count,
                           HUH::Vector4f color,
                           float threshold) {
    auto workIndex = threadIdx.x + blockDim.x * blockIdx.x;

    if (workIndex >= count) {
        return;
    }

    auto plane = eigData->mat.GetTransposed()[0];
    if (PlanePointDistance(plane, vertices[workIndex].pos) < 0.5f) {
        vertices[workIndex].color = color;
    }
}
}