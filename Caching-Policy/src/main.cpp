#include "utils/run.h"
using namespace std;


int main(){
    // mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC");

    run();
    // run_tmp();
    return 0;
}

/*Terminal 1
cd src
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
sudo date -s "YYYY-YY-DD HH:mm:ss"
sudo g++ -g -std=c++17 -o main main.cpp
sudo valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./main
sudo ./main
*/
/*Terminal 2
cd scripts/bash
sudo bash cpu_mem_disk.sh
*/
