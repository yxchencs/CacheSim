#include "utils/run.h"
using namespace std;


int main(){
    // mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC");

    run_tmp2();
    return 0;
}

/*Terminal 1
cd src
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
sudo date -s "YYYY-YY-DD HH:mm:ss"
sudo g++ -std=c++17 -o main main.cpp
sudo ./main
*/
/*Terminal 2
cd scripts/bash
sudo bash cpu_mem_disk.sh
*/
