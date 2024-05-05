#include "utils/run.h"
using namespace std;


int main(){
    // mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC");

    // run2();
    run();
    // run_no_cache();
    // run_no_cache_fixed_disk_size();
    // run_no_cache_example();
    return 0;
}

/*Terminal 1
cd src
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
sudo date -s "YYYY-YY-DD HH:mm:ss"
g++ -std=c++17 -o main main.cpp
sudo ./main
*/
/*Terminal 2
cd scripts/bash
sudo bash cpu_mem_disk.sh
*/
