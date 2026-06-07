#pragma once

#include <HUH/Math/vector.h>

/// \class LidarData
/// contains one point of data from the lidar point cloud
template<typename T>
class LidarData {
public:
    HUH::Vector3<T> data;
    int reflect;
    [[nodiscard]] T X() const { return data.X(); }
    [[nodiscard]] T Y() const { return data.Y(); }
    [[nodiscard]] T Z() const { return data.Z(); }
    LidarData& operator+=(const HUH::Vector3<T>& rhs) {
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

    friend LidarData operator+(LidarData lhs, HUH::Vector3<T>& rhs) {
        lhs += rhs;
        return lhs;
    }
};
