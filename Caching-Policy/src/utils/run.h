#ifndef _RUN_HPP_INCLUDED_
#define _RUN_HPP_INCLUDED_
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <unistd.h>
#include "mount.h"
#include <regex>
#include <chrono>
#include <thread>

#include "../simulator/randomSl.h"
#include "../simulator/fifoSl.h"
#include "../simulator/lruSl.h"
#include "../simulator/lfuSl.h"
#include "../simulator/lirsSl.h"
#include "../simulator/arcSl.h"
#include "../simulator/clockproSl.h"
#include "../simulator/2qSl.h"
#include "../simulator/tinylfuSl.h"
#include "../simulator/noCacheSl.h"

#include "../utils/progressBar.h"

namespace fs = std::filesystem;

// 获取当前时间点的日期时间格式化字符串，形如 2023-03-15_15:30:45
std::string getCurrentDateTime() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    // 将时间格式化为字符串
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");

    return ss.str();
}

std::vector<fs::path> find_trace_paths(const fs::path& root_dir) {
    std::vector<fs::path> directories;

    if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
        std::cerr << "Provided path is not a valid directory." << std::endl;
        return directories;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root_dir)) {
        if (entry.is_regular_file() && entry.path().filename() == "trace.txt") {
            fs::path parent_dir = entry.path().parent_path();
            fs::path storage_dir = parent_dir / "storage";

            if (fs::exists(storage_dir) && fs::is_directory(storage_dir)) {
                directories.push_back(parent_dir);
            }
        }
    }

    return directories;
}

std::vector<fs::path> find_trace_paths_shard_storage(const fs::path& root_dir) {
    std::vector<fs::path> directories;

    if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
        std::cerr << "Provided path is not a valid directory." << std::endl;
        return directories;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root_dir)) {
        if (entry.is_regular_file() && entry.path().filename() == "trace.txt") {
            directories.push_back(entry.path().parent_path());
        }
    }

    return directories;
}

bool copy_file_to_directory(const fs::path& source_file, const fs::path& target_directory) {
    fs::path target_file = target_directory / source_file.filename();

    try {
        fs::copy(source_file, target_file, fs::copy_options::overwrite_existing);

        fs::permissions(source_file, fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::group_write |
                        fs::perms::others_read | fs::perms::others_write);
        fs::permissions(target_file, fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::group_write |
                        fs::perms::others_read | fs::perms::others_write);
        std::cout<<"copy "<<source_file<<" to "<<target_directory<<std::endl;
        return true; 
    } catch (const fs::filesystem_error& e) {
        std::cerr << "error when copy or give permissions: " << e.what() << std::endl;
        return false; 
    }
}

void copy_files_containing_cache(const fs::path& source_directory, const fs::path& target_directory) {
    if (!fs::exists(source_directory) || !fs::is_directory(source_directory)) {
        std::cerr << "source dir does not exist or not valid: "<<source_directory.string() << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(source_directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("cache") != std::string::npos) {
                assert(copy_file_to_directory(entry.path(), target_directory));
            }
        }
    }
    printf("done copy files containing cache\n");
}

std::string getSubstringAfter(const std::string& original, const std::string& to_find) {
    size_t pos = original.find(to_find);
    if (pos != std::string::npos) {
        // 返回 to_find 后面的部分
        return original.substr(pos + to_find.length());
    } else {
        // 如果 to_find 未找到，可以返回原始字符串或空字符串
        return original;  // 或者返回 ""
    }
}

void mkdir(string path){
	string cmd("mkdir -p " + path);
	int ret = system(cmd.c_str());
	assert(ret!=-1);
}

void checkFile(fstream &file)
{
    if (!file.is_open())
    {
        cout << "Error: opening trace file fail" << endl;
        exit(1);
    }
}

bool checkSpaceEnough(int fd, ll offset, ll size) {
    struct stat fileStat;
    if (fstat(fd, &fileStat) == -1) {
        cerr << "Failed to get file status." << endl;
        return false;
    }

    off_t fileSize = fileStat.st_size;
    off_t requiredSpace = offset + size;
    if (requiredSpace > fileSize) {
        cerr << "Not enough space in the file." << endl;
        return false;
    }

    return true;
}

bool checkRes(int res) {
    if (res == -1) {
        std::cerr << "write: pwrite64 failed with error: " << strerror(errno) << std::endl;
        std::cerr << "write: errno: " << errno << std::endl;

        switch(errno) {
            case EBADF:
                std::cerr << "Error: Invalid file descriptor." << std::endl;
                break;
            case EFAULT:
                std::cerr << "Error: Buffer is outside accessible address space." << std::endl;
                break;
            case EFBIG:
                std::cerr << "Error: File is too large." << std::endl;
                break;
            case EINTR:
                std::cerr << "Error: Write operation interrupted by signal." << std::endl;
                break;
            case EINVAL:
                std::cerr << "Error: Invalid offset or file not suitable for writing." << std::endl;
                break;
            case ENOSPC:
                std::cerr << "Error: No space left on device." << std::endl;
                break;
            case EROFS:
                std::cerr << "Error: File system is read-only." << std::endl;
                break;
            default:
                std::cerr << "Error: Unexpected error." << std::endl;
                break;
        }
        return false;
    }
    return true;
}


void initCacheAndDiskSize(){
    fstream fin_trace(trace_path);
    checkFile(fin_trace);
    string s;
    ll block_size_KB;
    fin_trace>>s>>block_num>>disk_size>>trace_size>>block_size_KB;
    cache_size = block_num*cache_size_factor;
    assert(cache_size >= 10); // block_num >= 500
    block_size = block_size_KB * 1024;
    cout<<"block_num: "<<block_num<<", disk_size: "<<disk_size<<", trace_size: "<<trace_size
        <<", block_size_KB: "<<block_size_KB<<", cache_size: "<<cache_size<<endl;
}

void run_once(){
    std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("--------------------------------------------------------------------------------\n");
    std::unique_ptr<Sl> sim = nullptr;
    cache_size_factor = cacheSizeTypes[cache_size_index];
    initCacheAndDiskSize();
    cache_path = cache_dir+cachePath[cache_size_index];
    cout<<"cache_path: "<<cache_path<<endl;
    switch(policyTypes[caching_policy_index]){
        case PolicyType::RANDOM: 
            sim = std::make_unique<RandomSl>();
            break;
        case PolicyType::FIFO:
            sim = std::make_unique<FifoSl>();
            break;
        case PolicyType::LFU:
            sim = std::make_unique<LfuSl>();
            break;
        case PolicyType::LRU:
            sim = std::make_unique<LruSl>();
            break;
        case PolicyType::LIRS:
            sim = std::make_unique<LirsSl>();
            break;
        case PolicyType::ARC:
            sim = std::make_unique<ArcSl>();
            break;
        case PolicyType::CLOCKPRO:
            sim = std::make_unique<ClockproSl>();
            break;
        case PolicyType::TQ:
            sim = std::make_unique<TqSl>();
            break;
        case PolicyType::TINYLFU:
            sim = std::make_unique<TinylfuSl>();
            break;
    }

    sim->test();
    sim->statistic();
}

void run(){
    // save root
    save_root = "../records/" + getCurrentDateTime() + '/';
    mkdir(save_root);
    cout<<"save_root: "<<save_root<<endl;
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    auto trace_root_dir = "../trace/";
    // auto trace_root_dir = "../../trace_wait/";
    auto trace_dirs = find_trace_paths(trace_root_dir);

    for (const auto& dir : trace_dirs) {
        trace_dir = dir;
        trace_path = trace_dir+"/trace.txt";
        std::cout<<"trace_path: "<<trace_path<<std::endl;
        // storage dir
        storage_dir = trace_dir + "/storage/";
        copy_files_containing_cache(storage_dir, cache_dir);
        for(int k=0; k<2; k++){ // io
            io_on = 1-k;
            for(int i=0;i<cache_size_types_size;i++){ // cache_size
                cache_size_index = i;
                for(int j=0;j<policy_types_size;j++){ // cache_policy
                    caching_policy_index = j;
                    run_once();
                }
            }
        }
    }
}

// [tmp test]not copy disk.bin & cache.bin& use original bin
void run_tmp(){
    // save root
    save_root = "../records/" + getCurrentDateTime() + '/';
    mkdir(save_root);
    cout<<"save_root: "<<save_root<<endl;
    // trace dir
    auto trace_root_dir = "../trace/";
    auto trace_dirs = find_trace_paths(trace_root_dir);

    for (const auto& dir : trace_dirs) {
        trace_dir = dir;
        trace_path = trace_dir+"/trace.txt";
        std::cout<<"trace_path: "<<trace_path<<std::endl;
        
        cache_dir = storage_dir = trace_dir + "/storage/";
        
        io_on = 1;

        cache_size_index = 4;
        for(int j=0;j<policy_types_size;j++){ // cache_policy
            caching_policy_index = j;
            run_once();
        }
    }
}

// for run2()
void run_once2(){
    std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("--------------------------------------------------------------------------------\n");
    std::unique_ptr<Sl> sim = nullptr;
    cache_size_factor = cacheSizeTypes2[cache_size_index];
    initCacheAndDiskSize();
    cache_path = cache_dir+cachePath2[cache_size_index];
    cout<<"cache_path: "<<cache_path<<endl;
    switch(policyTypes[caching_policy_index]){
        case PolicyType::RANDOM: 
            sim = std::make_unique<RandomSl>();
            break;
        case PolicyType::FIFO:
            sim = std::make_unique<FifoSl>();
            break;
        case PolicyType::LFU:
            sim = std::make_unique<LfuSl>();
            break;
        case PolicyType::LRU:
            sim = std::make_unique<LruSl>();
            break;
        case PolicyType::LIRS:
            sim = std::make_unique<LirsSl>();
            break;
        case PolicyType::ARC:
            sim = std::make_unique<ArcSl>();
            break;
        case PolicyType::CLOCKPRO:
            sim = std::make_unique<ClockproSl>();
            break;
        case PolicyType::TQ:
            sim = std::make_unique<TqSl>();
            break;
        case PolicyType::TINYLFU:
            sim = std::make_unique<TinylfuSl>();
            break;
    }

    sim->test();
    sim->statistic();
}

// 测试memory使用，选择uniform分布，数据集设置大一点，比如20gb，然后cache size设置8% 16% 32%
void run2(){
    save_root = "../records/" + getCurrentDateTime() + '/';
    cout<<"save_root: "<<save_root<<endl;
    mkdir(save_root);
    cache_dir = "/mnt/eMMC/";
    block_size = 4 * 1024;
    // cout<<"cache_dir: "<<cache_dir<<endl;
    mkdir(cache_dir);

    auto trace_root_dir = "../trace/";
    auto trace_dirs = find_trace_paths(trace_root_dir);
    for (const auto& dir : trace_dirs) {
        trace_dir = dir;
        trace_path = trace_dir+"/trace.txt";
        std::cout<<"trace_path: "<<trace_path<<std::endl;
        storage_dir = trace_dir + "/storage/";

        copy_files_containing_cache(storage_dir, cache_dir);
        for(int k=0; k<2; k++){
            io_on = k;
            for(int i=0;i<cache_size_types_size2;i++){
                cache_size_index = i;
                for(int j=0;j<policy_types_size;j++){
                    caching_policy_index = j;
                    run_once2();
                }
            }
        }
    }
}

// @brief 测试无缓存直接读写设备(sd/eMMC)的性能
// @param trace_path
// @param device_id
// @param device_path
// @param block_size
void run_no_cache_once(std::string operation_read_ratio, ll block_size_KB, std::string device_id, std::string device_path)
{
    std::this_thread::sleep_for(std::chrono::seconds(20));
    std::cout << "operation_read_ratio: " << operation_read_ratio << ", block_size_KB: " << block_size_KB
    <<", device_id: " << device_id << ", device_path: " << device_path << std::endl;
    block_size = block_size_KB * 1024;
    NoCacheSl sl(operation_read_ratio, device_id, device_path);
    sl.test();
    sl.statistic();
}

// void run_no_cache_example(){
//     save_root = "../records/" + getCurrentDateTime() + '/';
//     trace_dir = "../trace/uniform/r100w_o15w_0.99/read_0/";
//     trace_path = trace_dir + "trace.txt";
//     ll block_size_KB = 4;
//     std::string device_id = "disk";
//     std::string device_path = trace_dir + "storage/disk.bin";
//     run_no_cache_once(block_size_KB, device_id, device_path);
// }

// void run_no_cache(){
//     std::string device_id;
//     std::string device_path;

//     save_root = "../records/" + getCurrentDateTime() + '/';
//     mkdir(save_root);
//     std::string emmc_dir = "/mnt/eMMC/";
//     mkdir(emmc_dir);
//     std::string sd_dir = "../storage/";
//     mkdir(sd_dir);

//     ll list_block_size_KB[] = {1, 4, 16, 64, 256, 1024};

//     auto trace_dirs = find_trace_paths("../trace/");

//     for (const auto &dir : trace_dirs)
//     {
//         trace_dir = dir;
//         trace_path = trace_dir + "/trace.txt";
//         std::cout << "trace_path: " << trace_path << std::endl;
//         std::string disk_dir = trace_dir + "/storage/disk.bin";

//         device_id = "sd";
//         device_path = sd_dir + "disk.bin";
//         // copy_files_containing_cache(storage_dir, sd_dir);
//         for (auto block_size_KB : list_block_size_KB)
//             run_no_cache_once(block_size_KB, device_id, device_path);

//         device_id = "emmc";
//         device_path = emmc_dir + "disk.bin";
//         // copy_files_containing_cache(storage_dir, emmc_dir);
//         for (auto block_size_KB : list_block_size_KB)
//             run_no_cache_once(block_size_KB, device_id, device_path);
//     }
// }

// Need copy disk.bin manually
void run_no_cache_fixed_disk_size(){
    std::string device_id;
    std::string device_path;
    initCacheAndDiskSize();
    save_root = "../records/" + getCurrentDateTime() + '/';
    mkdir(save_root);
    std::string emmc_dir = "/mnt/eMMC/";
    mkdir(emmc_dir);
    std::string sd_dir = "../storage/";
    mkdir(sd_dir);
    std::string disk_name = "disk.bin";

    std::regex re("(\\d+)KB");
    std::smatch matches;
    int block_size_KB;

    std::string trace_root = "../trace/";
    vector<std::string> operation_read_ratio_list = {"read_1", "read_0"};
    for(auto operation_read_ratio: operation_read_ratio_list) {
        auto trace_dirs = find_trace_paths_shard_storage(trace_root+"/"+operation_read_ratio+"/");
        for (const auto &dir : trace_dirs)
        {
            trace_dir = dir.string();
            std::cout << "trace_dir: " << trace_dir << std::endl;
            if (std::regex_search(trace_dir, matches, re)) {
                // std::cout << "Number extracted: " << matches[1] << std::endl;
                block_size_KB = std::stoi(matches[1]);
            } else {
                std::cout << "No number found." << std::endl;
                continue;
            }

            trace_path = trace_dir + "/trace.txt";
            std::cout << "trace_path: " << trace_path << std::endl;
            // std::string disk_dir = trace_dir + "/storage/" + disk_name; // Copying manually

            device_id = "sd";
            device_path = sd_dir + "/" + disk_name;
            // copy_file_to_directory(disk_dir, sd_dir);
            run_no_cache_once(operation_read_ratio, block_size_KB, device_id, device_path);

            device_id = "emmc";
            device_path = emmc_dir + "/" + disk_name;
            // copy_file_to_directory(disk_dir, emmc_dir);
            run_no_cache_once(operation_read_ratio, block_size_KB, device_id, device_path);
        }
    }
}
#endif /*_RUN_HPP_INCLUDED_*/
