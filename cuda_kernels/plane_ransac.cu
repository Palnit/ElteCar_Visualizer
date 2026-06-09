#include <HUH/Math/vector.h>

struct LidarVertex {
    HUH::Vector3f pos;
    HUH::Vector3f color;
};

extern "C" {

__global__ void PlaneRansac(LidarVertex* vertices, HUH::Vector3f color, HUH::Uint32 count) {
    int workIndex = threadIdx.x + blockDim.x * blockIdx.x;
    if (workIndex >= count) {
        return;
    }

    vertices[workIndex].color = color;
}
}