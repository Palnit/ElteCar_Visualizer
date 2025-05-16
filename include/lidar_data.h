#ifndef ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H
#define ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H

#include <omp.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

/// \class LidarData
/// contains one point of data from the lidar point cloud
template<typename T>
class LidarData {
public:
    glm::vec<3, T> data;
    int reflect;
    [[nodiscard]] T X() const { return data.x; }
    [[nodiscard]] T Y() const { return data.y; }
    [[nodiscard]] T Z() const { return data.z; }
    LidarData& operator+=(const glm::vec<3, T>& rhs) {
        data += rhs;
        return *this;
    }
    LidarData& operator-=(const LidarData& rhs) {
        data -= rhs.data;
        return *this;
    }

    friend LidarData operator-(const LidarData& lhs, const LidarData& rhs) {
        return LidarData{lhs.data - rhs.data, lhs.reflect};
    }

    friend LidarData operator+(LidarData lhs, const glm::vec<3, T>& rhs) {
        lhs += rhs;
        return lhs;
    }

    T Length() { return glm::length(data); }
};

template<typename T>
using LidarPointCloud = std::vector<LidarData<T>>;

using LidarPointCloudD = LidarPointCloud<double>;

template<typename T>
static void TranslatePointCloud(LidarPointCloud<T>& PointCloud,
                                const glm::vec<3, T>& Translation) {
#pragma omp parallel for
    for (auto& Point : PointCloud) { Point += Translation; }
}

template<typename T>
static std::vector<std::pair<size_t, size_t>> FindClosestPoints(
    const LidarPointCloud<T>& SrcPointCloud,
    const LidarPointCloud<T>& TargetPointCloud) {
    std::vector<std::pair<size_t, size_t>> ClosestPoints;

    for (size_t i = 0; i < SrcPointCloud.size(); i++) {
        size_t IndexOfClosestPoint = 0;
        T Distance = std::numeric_limits<T>::max();
        for (size_t j = 0; j < TargetPointCloud.size(); j++) {
            auto Diff = TargetPointCloud[i] - SrcPointCloud[j];
            auto DistanceTmp = Diff.Length();
            if (DistanceTmp < Distance) {
                Distance = DistanceTmp;
                IndexOfClosestPoint = j;
            }
        }
        ClosestPoints.emplace_back(i, IndexOfClosestPoint);
    }
    return ClosestPoints;
}

template<typename T>
static LidarPointCloud<T> DownSamplePointCloud(
    LidarPointCloud<T> InputPointCloud) {}

#endif// ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H
