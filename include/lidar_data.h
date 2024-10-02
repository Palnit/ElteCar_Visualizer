#ifndef ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H
#define ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H

/// \class LidarData
/// contains one point of data from the lidar point cloud
struct LidarData {
    double x;
    double y;
    double z;
    int reflect;
};

#endif// ELTECAR_DATASERVER_INCLUDE_LIDAR_DATA_H
