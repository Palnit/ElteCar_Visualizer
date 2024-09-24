#ifndef ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
#define ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H

struct cartesians {
    int ID;
    float Lat;
    float Lon;
    float Alt;
    float Vel;
    float Ax, Ay, Az;
    float Mx, My, Mz;
};

#endif// ELTECAR_DATASERVER_INCLUDE_CARTESIANS_H
