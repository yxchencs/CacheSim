#include "utils/run.h"
using namespace std;

int main(){
    chunk_num = 4*1024;
    disk_size = 1024;
    io_on = false;
    cache_size_index = 4;
    caching_policy_index = 0;
    cache_size_factor = 0.1;
    cache_size = 64;
    cache_dir = "../storage/cache_0.1.cpp";
    std::string trace_dir;
    std::string trace_path;
    std::string storage_dir;
    std::string save_root;
    run_once();
}