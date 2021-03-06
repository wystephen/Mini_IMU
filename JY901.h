#ifndef JY901_h
#define JY901_h

#include "string.h"
#include <thread>
#include <mutex>
#include <atomic>

#define SAVE            0x00
#define CALSW        0x01
#define RSW            0x02
#define RRATE            0x03
#define BAUD            0x04
#define AXOFFSET    0x05
#define AYOFFSET    0x06
#define AZOFFSET    0x07
#define GXOFFSET    0x08
#define GYOFFSET    0x09
#define GZOFFSET    0x0a
#define HXOFFSET    0x0b
#define HYOFFSET    0x0c
#define HZOFFSET    0x0d
#define D0MODE        0x0e
#define D1MODE        0x0f
#define D2MODE        0x10
#define D3MODE        0x11
#define D0PWMH        0x12
#define D1PWMH        0x13
#define D2PWMH        0x14
#define D3PWMH        0x15
#define D0PWMT        0x16
#define D1PWMT        0x17
#define D2PWMT        0x18
#define D3PWMT        0x19
#define IICADDR        0x1a
#define LEDOFF        0x1b
#define GPSBAUD        0x1c

#define YYMM                0x30
#define DDHH                0x31
#define MMSS                0x32
#define MS                    0x33
#define AX                    0x34
#define AY                    0x35
#define AZ                    0x36
#define GX                    0x37
#define GY                    0x38
#define GZ                    0x39
#define HX                    0x3a
#define HY                    0x3b
#define HZ                    0x3c
#define Roll                0x3d
#define Pitch                0x3e
#define Yaw                    0x3f
#define TEMP                0x40
#define D0Status        0x41
#define D1Status        0x42
#define D2Status        0x43
#define D3Status        0x44
#define PressureL        0x45
#define PressureH        0x46
#define HeightL            0x47
#define HeightH            0x48
#define LonL                0x49
#define LonH                0x4a
#define LatL                0x4b
#define LatH                0x4c
#define GPSHeight   0x4d
#define GPSYAW      0x4e
#define GPSVL                0x4f
#define GPSVH                0x50

#define DIO_MODE_AIN 0
#define DIO_MODE_DIN 1
#define DIO_MODE_DOH 2
#define DIO_MODE_DOL 3
#define DIO_MODE_DOPWM 4
#define DIO_MODE_GPS 5

struct STime {
    unsigned char ucYear;
    unsigned char ucMonth;
    unsigned char ucDay;
    unsigned char ucHour;
    unsigned char ucMinute;
    unsigned char ucSecond;
    unsigned short usMiliSecond;
};
struct SAcc {
    short a[3];
    short T;
};
struct SGyro {
    short w[3];
    short T;
};
struct SAngle {
    short Angle[3];
    short T;
};
struct SMag {
    short h[3];
    short T;
};

struct SDStatus {
    short sDStatus[4];
};

struct SPress {
    long lPressure;
    long lAltitude;
};

struct SLonLat {
    long lLon;
    long lLat;
};

struct SGPSV {
    short sGPSHeight;
    short sGPSYaw;
    long lGPSVelocity;
};

class CJY901 {
public:
    struct STime stcTime;
    struct SAcc stcAcc;
    struct SGyro stcGyro;
    struct SAngle stcAngle;
    struct SMag stcMag;
    struct SDStatus stcDStatus;
    struct SPress stcPress;
    struct SLonLat stcLonLat;
    struct SGPSV stcGPSV;
    std::atomic<bool> isend ;
    unsigned char global_buff[2000];
    int last_len = -1;

    std::mutex preprocess_mutex_;

    unsigned char chrTemp[4000];

    CJY901() {
        std::cout << "after initial jy901" << std::endl;
        isend=false;
    }

    void CopeSerialData(char ucData[], unsigned short usLength);

    bool getisend() {
        if (isend) {
            isend = false;
            return true;
        } else {
            return false;
        }

    }
};


void CJY901::CopeSerialData(char ucData[], unsigned short usLength) {
    static unsigned char ucRxCnt = 0;
    static unsigned short usRxLength = 0;


    if (last_len > 0) {
        memcpy(chrTemp, global_buff, last_len);
        memcpy(&chrTemp[last_len], ucData, usLength);

    } else {
        memcpy(chrTemp, ucData, usLength);
    }
    usRxLength += usLength;
    while (usRxLength >= 11) {
        if (chrTemp[0] != 0x55) {
            usRxLength--;
            memcpy(&chrTemp[0], &chrTemp[1], usRxLength);
            continue;
        }
        switch (chrTemp[1]) {
            case 0x50:
                memcpy(&stcTime, &chrTemp[2], 8);
                isend = true;
//                std::cout << "time ";// << std::endl;
                break;
            case 0x51:
                memcpy(&stcAcc, &chrTemp[2], 8);
//                std::cout << "acc" ;//<< std::endl;
                break;
            case 0x52:
                memcpy(&stcGyro, &chrTemp[2], 8);
//                std::cout << "gyro" ;//<< std::endl;
                break;
            case 0x53:
                memcpy(&stcAngle, &chrTemp[2], 8);
//                std::cout << "angular" ;//<< std::endl;
                break;
            case 0x54:
                memcpy(&stcMag, &chrTemp[2], 8);
//                std::cout << "mag" ;//<< std::endl;
//                isend=true;
                break;
            case 0x55:
                memcpy(&stcDStatus, &chrTemp[2], 8);

                break;
            case 0x56:
                memcpy(&stcPress, &chrTemp[2], 8);
//                std::cout << " press "<< std::endl;
//                isend = true;
                break;
            case 0x57:
                memcpy(&stcLonLat, &chrTemp[2], 8);
                break;
            case 0x58:
                memcpy(&stcGPSV, &chrTemp[2], 8);
                break;
            default:
                std::cout << "SOME ERROR IN DECODER" << std::endl;
                break;
        }
        usRxLength -= 11;
        memcpy(&chrTemp[0], &chrTemp[11], usRxLength);
    }
    memcpy(global_buff, chrTemp, usRxLength);
    last_len = usRxLength;

}
//extern CJY901 JY901;
#endif