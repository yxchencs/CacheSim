#include "utils/run.h"
using namespace std;


int main(){
    if (!mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC"))
    {
        return 1;
    }

    // run2();
    run();
    // run_no_cache();
    return 0;
}

/*
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
sudo date -s "YYYY-YY-DD HH:mm:ss"
g++ -std=c++17 -o main main.cpp
sudo ./main
*/
