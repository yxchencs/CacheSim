#include "utils/run.h"
using namespace std;


int main(){
    // Use the function to mount /dev/mmcblk0p1 to /mnt/eMMC and check if it was successful
    bool result = executeAndCheckMount("/dev/mmcblk0p1", "/mnt/eMMC");
    
    if (result) {
        // Mount was successful
        std::cout << "Mount successful." << std::endl;
    } else {
        // Mount failed
        std::cout << "Mount failed." << std::endl;
        return 1;
    }
    
    run2();
    // run();
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
