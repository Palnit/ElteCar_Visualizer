#include <general/slam_creater.h>
#include <pcl/common/transforms.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/registration/icp.h>

#include <osmium/osm/location.hpp>
#include <thread>

SlamCreator::SlamCreator() {
    m_lidarReader =
        new SharedMemory::BufferedReader<SlamCreator::PointCloud::Ptr>(
            "Lidar", SlamCreator::LidarReader);
    m_csvReader = new SharedMemory::BufferedReader<Cartesians>(
        "Csv", [](void* pointer, int size) {
            return *static_cast<Cartesians*>(pointer);
        });
}
SlamCreator::~SlamCreator() {
    delete m_csvReader;
    delete m_lidarReader;
}
SlamCreator::PointCloud::Ptr SlamCreator::GetLidarData() {
    bool fail, fail2;
    auto Cart = m_csvReader->readData(fail);
    auto lidar = m_lidarReader->readData(fail2);
    if (fail || fail2) { return {}; }
    if (m_first) {
        PointCloud::Ptr out2(new PointCloud());
        pcl::VoxelGrid<pcl::PointXYZ> Voxel;
        Voxel.setInputCloud(lidar);
        Voxel.setLeafSize(1.5f, 1.5f, 1.5f);
        Voxel.filter(*out2);
        m_lidarFinalCloud = out2;
        m_prevLidarCloud = lidar;
        m_first = false;
        m_originLocation = osmium::Location(Cart.Lon, Cart.Lat);
        m_originalCart = Cart;
        m_prev_acceleration = Eigen::Vector2f(Cart.Ax, Cart.Az);
        m_originalAltitude = Cart.Alt;
        return m_lidarFinalCloud;
    }
    const auto Translation = CartesianToTranslation(Cart);
    PointCloud::Ptr out(new PointCloud());
    pcl::transformPointCloud(*lidar, *out, Translation);
    PointCloud::Ptr out2(new PointCloud());
    pcl::VoxelGrid<pcl::PointXYZ> Voxel;
    Voxel.setInputCloud(out);
    Voxel.setLeafSize(1.5f, 1.5f, 1.5f);
    Voxel.filter(*out2);
    // std::cout << "filter " << out->size() << " : " << out2->size() << std::endl;

    pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
    icp.setInputSource(out2);
    icp.setInputTarget(m_prevLidarCloud);
    icp.setNumberOfThreads(8);
    icp.setMaxCorrespondenceDistance(0.5);
    icp.setMaximumIterations(10);
    icp.setTransformationEpsilon(std::numeric_limits<double>::epsilon());
    icp.setEuclideanFitnessEpsilon(1);
    // icp.setRANSACOutlierRejectionThreshold(1.5);
    PointCloud::Ptr Final(new PointCloud());
    icp.align(*Final);
    std::cout << "ICP has "
              << (icp.hasConverged() ? "converged" : "not converged")
              << ", score: " << icp.getFitnessScore()
              << " Iters: " << icp.nr_iterations_ << std::endl;
    // std::cout << icp.getFinalTransformation() << std::endl;
    m_prevLidarCloud = Final;
    *m_lidarFinalCloud += *Final;
    pcl::transformPointCloud(*out, *out, icp.getFinalTransformation());

    return Final;
}
SlamCreator::PointCloud::Ptr SlamCreator::LidarReader(void* pointer, int size) {
    PointCloud::Ptr output(new PointCloud);
    const auto* LidarPointer = static_cast<LidarData<double>*>(pointer);
    for (int i = 0; i < size / sizeof(LidarData<double>); i++) {
        output->emplace_back(LidarPointer->data.x, LidarPointer->data.y,
                             LidarPointer->data.z);
        LidarPointer++;
    }
    return output;
}

Eigen::Affine3f SlamCreator::CartesianToTranslation(const Cartesians& Cart) {
    osmium::Location Location(Cart.Lon, Cart.Lat);
    Eigen::Affine3f Transform = Eigen::Affine3f::Identity();
    Transform.translation()
        << static_cast<float>(Location.x() - m_originLocation.x()) / 100,
        static_cast<float>(Location.y() - m_originLocation.y()) / 100,
        static_cast<float>(Cart.Alt - m_originalAltitude);
    // Eigen::Vector2f current_acceleration(Cart.Ax, Cart.Az);
    // m_prev_acceleration = current_acceleration;
    // float roll = std::atan2(Cart.My, Cart.Mx);
    // float pitch =
    //     std::atan2((-Cart.Ax), std::sqrt(Cart.Ay * Cart.Ay + Cart.Az * Cart.Az))
    //     * 57.3;
    Transform.rotate(Eigen::AngleAxisf(Cart.Roll, Eigen::Vector3f::UnitZ()));
    Transform.rotate(Eigen::AngleAxisf(Cart.Pitch, Eigen::Vector3f::UnitX()));
    // Transform.rotate(Eigen::AngleAxisf(Cart.Yaw, Eigen::Vector3f::UnitY()));
    return Transform;
}