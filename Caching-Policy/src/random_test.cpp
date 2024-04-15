#include "utils/run.h"
using namespace std;

int main(){
    io_on = false;
    cache_size_index = 4; // 0.1
    caching_policy_index = 0; // random
    // trace_dir = "../trace_20240124_2/latest/read_0/";
    trace_dir = "/mnt/e/projects/Caching-Policy/Caching-Policy/trace_20240124_2/latest/read_0/";
    trace_path = trace_dir + "/trace.txt";
    storage_dir = trace_dir + "/storage/";
    cache_dir = trace_dir + "/storage/";
    save_root = "../../records/" + getCurrentDateTime() + '/';
    run_once();
}

// g++ -std=c++17 -o random_test random_test.cpp