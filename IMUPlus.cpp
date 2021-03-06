//
// Created by steve on 17-5-17.
//

#include <iostream>
#include     <stdio.h>      /*标准输入输出定义*/
#include     <stdlib.h>     /*标准函数库定义*/
#include     <unistd.h>     /*Unix 标准函数定义*/
#include     <sys/types.h>
#include     <sys/stat.h>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "time.h"
#include "stdio.h"
#include "JY901.h"

//#include <boost/asio.hpp>
//#include <boost/bind.hpp>

#include <fstream>
#include <cmath>

#include <mutex>
#include <thread>


#include <chrono>

#include "CharQueue.h"

bool checkUSB(std::string dev_name) {
    FILE *stream;
    char out_res[1000];
    stream = popen(std::string("ls " + dev_name).c_str(), "r");
    memset(out_res, '\0', sizeof(out_res));
    fread(out_res, sizeof(char), sizeof(out_res), stream);
    pclose(stream);
//    std::cout << std::string(out_res).size() << std::endl;
    if (std::string(out_res).size() == 0) {
        return false;
    } else {
        return true;
    }
}

double now() {
    auto tt = std::chrono::system_clock::now();
    auto t_nanosec = std::chrono::duration_cast<std::chrono::nanoseconds>(tt.time_since_epoch());

    double time_now(double(t_nanosec.count()) * 1e-9);
    return time_now;
}

unsigned char ucComNo[2] = {0, 0};
bool StopExeFlag = false;

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop) {
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0) {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (nBits) {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch (nEvent) {
        case 'O':                     //奇校验
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E':                     //偶校验
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':                    //无校验
            newtio.c_cflag &= ~PARENB;
            break;
    }

    switch (nSpeed) {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 921600:
            cfsetispeed(&newtio, B921600);
            cfsetospeed(&newtio, B921600);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;

        default:
            cfsetispeed(&newtio, B921600);
            cfsetospeed(&newtio, B921600);
            break;
    }
    if (nStop == 1) {
        newtio.c_cflag &= ~CSTOPB;
    } else if (nStop == 2) {
        newtio.c_cflag |= CSTOPB;
    }
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0) {
        perror("com set error");
        return -1;
    }
    printf("set done!\n");
    return 0;
}

void ReadThread(int fd, CharQueue<char> &cq, std::string dev_name) {
//   std::thread::
//    usleep(1000);
    std::cout << "start read" << std::endl;
    char buff[4000];
    unsigned short length = 0;
    int first_time = 0;
    while (true) {


        length = read(fd, buff, 1000);
        if (first_time < 20) {
            length = read(fd, buff, 3100);
            first_time += 1;
            continue;
        } else {
            if (length > 0) {
                cq.AddBuf(buff, length);
//            std::cout << " buff lenght :" << length << std::endl;
            } else {
                if (!checkUSB(dev_name)) {
                    return;
                }
                usleep(10);
            }
        }

        //usleep(100);
    }
}


int main(int argc, char *argv[]) {
    std::string dev_str("/dev/ttyUSB0");
    std::string save_file("test" + std::to_string(now()) + ".txt");

    if (argc == 3) {
        dev_str = std::string(argv[1]);
        save_file = std::string(argv[2]);
    }


    CJY901 JY901;

    int fd = open(dev_str.c_str(), O_RDWR | O_NOCTTY);
    if (-1 == fd) {
        std::cout << " can't open device" << std::endl;
        return 0;
    }
    std::ofstream out_file(save_file);


    char chrBuffer[4000];
    unsigned short usLength = 0, usCnt = 0;
    set_opt(fd, 460800, 8, 'N', 1);

    float last_milisecond = (float) JY901.stcTime.usMiliSecond / 1000;
    int error_times = 0;
    double start_time = now();
    double imu_start_time = 0.0;
    out_file.precision(13);
//    out_file << now() << std::endl;

    CharQueue<char> cq(10000);

    std::thread t(ReadThread, fd, std::ref(cq), dev_str);

//    t.detach();

//    usleep(1000000);
    int counter_times(0);
    std::cout.precision(20);

//    sleep(2);
    cq.DeletBuf(cq.getSize());


    while (true) {

        int size = 1;
        if (cq.ReadBuf(chrBuffer, size)) {

            cq.DeletBuf(size);
            usLength = size;
        } else {
            if (!checkUSB(dev_str)) {
                return 0;
            }

            usLength = 0;
            usleep(10);
        }

        if (usLength > 0) {
            JY901.CopeSerialData(chrBuffer, usLength);
        }

        if (JY901.getisend())//|| last_milisecond!=(float)JY901.stcTime.usMiliSecond/1000)
        {
            if (std::fabs(std::fabs((float) JY901.stcTime.usMiliSecond / 1000 - last_milisecond) - 0.005) > 0.0001 &&
                std::fabs(std::fabs((float) JY901.stcTime.usMiliSecond / 1000 - last_milisecond) - 0.995) > 0.0001) {
                error_times++;
                if (error_times % 1 == 0) {
                    std::cout << error_times << "average time :" << (now() - start_time) / double(error_times)
                              << "cq size :" << cq.getSize() << " "
                              << std::endl;
                }
                continue;

            }

            struct tm t = {0};

            t.tm_year = (short) JY901.stcTime.ucYear;
            t.tm_mon = (short) JY901.stcTime.ucMonth;
            t.tm_mday = (short) JY901.stcTime.ucDay;
            t.tm_hour = (short) JY901.stcTime.ucHour;
            t.tm_min = (short) JY901.stcTime.ucMinute;
            t.tm_sec = (short) (float) JY901.stcTime.ucSecond;


//            std::cout << t.tm_hour<<":"<<t.tm_min<<":"<<t.tm_sec<<" ";

            double imu_time_now = timegm(&t);
            imu_time_now += (float) JY901.stcTime.usMiliSecond / 1000;


            if (counter_times < 2) {
//                imu_start_time =
                imu_start_time = imu_time_now;

                start_time = now();
                counter_times++;

                std::cout << "reset first start time " << std::endl;
                continue;
            } else {
//                out_file << imu_time_now - imu_start_time + start_time << "," <<
                out_file << now()<< "," <<
                         (float) JY901.stcAcc.a[0] / 32768 * 16 << "," << (float) JY901.stcAcc.a[1] / 32768 * 16 << ","
                         <<
                         (float) JY901.stcAcc.a[2] / 32768 * 16 << "," <<
                         (float) JY901.stcGyro.w[0] / 32768 * 2000 << "," << (float) JY901.stcGyro.w[1] / 32768 * 2000
                         << "," <<
                         (float) JY901.stcGyro.w[2] / 32768 * 2000 << "," <<
                         JY901.stcMag.h[0] << "," << JY901.stcMag.h[1] << "," << JY901.stcMag.h[2]
                         << "," << JY901.stcPress.lPressure;
                out_file << std::endl;

                last_milisecond = (float) JY901.stcTime.usMiliSecond / 1000;
            }


        }

    }

}
