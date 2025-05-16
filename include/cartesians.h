#ifndef ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
#define ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H

/// \class Cartesians
/// Contains the values from one row of the csv file
struct Cartesians {
    int ID;
    double Lat;
    double Lon;
    double Alt;
    double Vel;
    double Roll, Pitch, Yaw;
    double Ax, Ay, Az;
    double Mx, My, Mz;
};

#endif// ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
