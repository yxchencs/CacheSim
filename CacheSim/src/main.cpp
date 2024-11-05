#include "utils/run.h"
using namespace std;

int main(int argc, char* argv[]) {
    // mountAndCheck("/dev/mmcblk0p1", "/mnt/eMMC");
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <run_mode> [arguments]" << endl;
        return 1;
    }

    string runMode = argv[1];

    if (runMode == "nocache" ) {
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
