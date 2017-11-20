/** 
                   _ooOoo_ 
                  o8888888o 
                  88" . "88 
                  (| -_- |) 
                  O\  =  /O 
               ____/`---'\____ 
             .'  \\|     |//  `. 
            /  \\|||  :  |||//  \ 
           /  _||||| -:- |||||-  \ 
           |   | \\\  -  /// |   | 
           | \_|  ''\---/''  |   | 
           \  .-\__  `-`  ___/-. / 
         ___`. .'  /--.--\  `. . __ 
      ."" '<  `.___\_<|>_/___.'  >'"". 
     | | :  `- \`.;`\ _ /`;.`/ - ` : | | 
     \  \ `-.   \_ __\ /__ _/   .-` /  / 
======`-.____`-.___\_____/___.-`____.-'====== 
                   `=---=' 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
         佛祖保佑       永无BUG 
*/
//
// Created by steve on 17-11-20.
//


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

int main(int argc, char *argv[])
{
    std::string dev_str("/dev/ttyUSB0");
    std::string save_file("./testATEUWB.txt");

    if(argc ==3)
    {
        dev_str = std::string(argv[1]);
        save_file = std::string(argv[2]);
    }


    int fd = open(dev_str.c_str(), O_RDWR | O_NOCTTY);
    if (-1 == fd) {
        std::cout << " can't open device" << std::endl;
        return 0;
    }

    std::ofstream out_file(save_file);
    out_file.precision(13);


    char chrBuffer[4000];
    unsigned short usLength = 0, usCnt = 0;
    set_opt(fd, 115200, 8, 'N', 1);




    while(true){

        memset(chrBuffer,0,4000);
        int len = read(fd,chrBuffer,1000);
        if(len>0)
        {

        }

    }

}
