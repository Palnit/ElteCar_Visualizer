#ifndef SLAM_CREATER_H
#define SLAM_CREATER_H
#include <pcl/point_cloud.h>

#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>
#include <osmium/osm/location.hpp>
#include <pcl/impl/point_types.hpp>

#include "SharedMemory/bufferd_reader.h"
#include "cartesians.h"
#include "lidar_data.h"

class SlamCreator {
public:
    using PointCloud = pcl::PointCloud<pcl::PointXYZ>;
    SlamCreator();
    ~SlamCreator();
    PointCloud::Ptr GetLidarData();

private:
    /// temporary function to get lidar data array
    /// @param pointer pointer to shared memory
    /// @param size size of shared memory
    /// @return the returned lidar array
    static PointCloud::Ptr LidarReader(void* pointer, int size);

    [[nodiscard]] Eigen::Affine3f CartesianToTranslation(
        const Cartesians& Cart);

    SharedMemory::BufferedReader<Cartesians>* m_csvReader;
    SharedMemory::BufferedReader<PointCloud::Ptr>* m_lidarReader;
    PointCloud::Ptr m_lidarFinalCloud;
    PointCloud::Ptr m_prevLidarCloud;
    osmium::Location m_originLocation;
    Cartesians m_originalCart;
    Eigen::Vector2f m_prev_acceleration;
    float theta;
    double m_originalAltitude;
    bool m_first = true;
};

#endif//SLAM_CREATER_H
