#include "utils/run.h"
using namespace std;

int main(int argc, char* argv[]) {
    // mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC");
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <run_mode> [arguments]" << endl;
        return 1;
    }

    string runMode = argv[1];

    if (runMode == "device" ) {
        runNoCache();
    } else if (runMode == "ycsb") {
        runYcsb();
    } else if (runMode == "real") {
        runReal();
    }
    // else if (runMode == "ycsb_test") {
    //     runYcsbIoOnCacheSize10ppRandom();
    // }
    else {
        cerr << "Unknown run mode: " << runMode << endl;
        return 1;
    }

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
