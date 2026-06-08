#include <HUH/Math/vector.h>

struct LidarVertex {
    HUH::Vector3f pos;
    HUH::Vector3f color;
};

extern "C" {

__global__ void PlaneRansac(LidarVertex* vertices) {
    vertices[threadIdx.x].pos = {0, 0, 1};
}
}