#ifndef ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
#define ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H

/// \class Cartesians
/// Contains the values from one row of the csv file
struct Cartesians {
    int ID;
    float Lat;
    float Lon;
    float Alt;
    float Vel;
    float Ax, Ay, Az;
    float Mx, My, Mz;
};

#endif// ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
