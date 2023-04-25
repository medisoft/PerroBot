#ifndef _MAIN_H_
#define _MAIN_H_
#include <PCA9685.h>

struct LegAngle
{
    int shoulder;
    int tibia;
    int femur;
};

struct Perro
{
    // int pos1;
    // int pos2;
    // int pos3;
    LegAngle frontRight;
    LegAngle frontLeft;
    LegAngle backRight;
    LegAngle backLeft;
};

struct Leg
{
    PCA9685_ServoEval shoulder;
    PCA9685_ServoEval tibia;
    PCA9685_ServoEval femur;
};
struct Servos
{
    Leg frontRight;
    Leg frontLeft;
    Leg backRight;
    Leg backLeft;
};

struct Params
{
    Perro perro;
    Servos servos;
};
#endif