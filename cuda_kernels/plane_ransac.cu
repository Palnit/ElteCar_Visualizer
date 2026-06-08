#include <HUH/Math/vector.h>

struct LidarVertex {
    HUH::Vector3f pos;
    HUH::Vector3f color;
};

extern "C" {

__global__ void PlaneRansac(LidarVertex* vertices, HUH::Vector3f color) {
    vertices[threadIdx.x].color = color;
}
}