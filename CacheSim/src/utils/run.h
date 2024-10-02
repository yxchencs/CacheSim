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

double extractDiskSizeKB(const std::string& str) {
    std::regex pattern(R"((\d+(\.\d+)?)\s*([a-zA-Z]+))");
    std::smatch match;
    if (std::regex_match(str, match, pattern)) {
        std::string number = match[1];
        std::string unit = match[3];
        // std::cout << "number: " << number << ", unit: " << unit << std::endl;
        if (unit == "GB") {
            return std::stod(number) * 1024 * 1024;
        } else if (unit == "MB") {
            return std::stod(number) * 1024;
        }
    }
    return 0.0;
}

std::vector<std::string> findTracePathsReal(const std::string& root_dir) {
    std::vector<std::string> directories;

    fs::path root_path(root_dir); 

    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        std::cerr << "Provided path is not a valid directory." << std::endl;
        return directories;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
        if (entry.is_regular_file() && entry.path().filename() == "trace.txt") {
            fs::path parent_dir = entry.path().parent_path();
            fs::path storage_dir = parent_dir / "storage";

            if (fs::exists(storage_dir) && fs::is_directory(storage_dir)) {
                directories.push_back(parent_dir.string()); 
            }
        }
    }

    return directories;
}

std::vector<std::string> findTracePathsYcsb(const std::string& root_dir) {
    std::vector<std::string> directories;

    fs::path root_path(root_dir); 
    if (!fs::exists(root_path) || !fs::is_directory(root_path)) {
        std::cerr << "Provided path is not a valid directory." << std::endl;
        return directories;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
        if (entry.is_regular_file() && entry.path().filename() == "trace.txt") {
            fs::path parent_dir = entry.path().parent_path();
            directories.push_back(parent_dir.string()); 
        }
    }

    return directories;
}

std::vector<std::string> findTraceRootNames(const std::string& folder_path) {
    std::vector<std::string> trace_roots;

    // 遍历文件夹中的文件和子文件夹
    for (const auto& entry : fs::directory_iterator(folder_path)) {
        if (entry.is_directory()) {
            trace_roots.push_back(entry.path().filename().string());
        }
    }

    return trace_roots;
}

std::vector<std::string> findTracePathsSharedStorage(const fs::path& root_dir) {
    std::vector<std::string> directories;

    if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
        std::cerr << "Provided path is not a valid directory." << std::endl;
        return directories;
    }

    for (const auto& entry : fs::recursive_directory_iterator(root_dir)) {
        if (entry.is_regular_file() && entry.path().filename() == "trace.txt") {
            directories.push_back(entry.path().parent_path().string());
        }
    }

    return directories;
}

bool copyFileToDirectory(const fs::path& source_file, const fs::path& target_directory) {
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

void copyFilesContainingCache(const fs::path& source_directory, const fs::path& target_directory) {
    if (!fs::exists(source_directory) || !fs::is_directory(source_directory)) {
        std::cerr << "source dir does not exist or not valid: "<<source_directory.string() << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(source_directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("cache") != std::string::npos) {
                assert(copyFileToDirectory(entry.path(), target_directory));
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

string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d_%H-%M-%S");

    return ss.str();
}


void makeSaveRoot() {
    // save root
    save_root = "../records/" + getCurrentDateTime() + '/';
    mkdir(save_root);
    cout<<"save_root: "<<save_root<<endl;
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


void initParm(){
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


void runRealOnce(){
    std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("--------------------------------------------------------------------------------\n");
    std::unique_ptr<Sl> sim = nullptr;
    cache_size_factor = cacheSizeTypes[cache_size_index];
    initParm();
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

void runReal(){
    makeSaveRoot();
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    auto trace_root_dir = "../trace/";
    // auto trace_root_dir = "../../trace_wait/";
    auto trace_dirs = findTracePathsReal(trace_root_dir);

    for (const auto& dir : trace_dirs) {
        trace_dir = dir;
        trace_path = trace_dir+"/trace.txt";
        std::cout<<"trace_path: "<<trace_path<<std::endl;
        // storage dir
        storage_dir = trace_dir + "/storage/";
        copyFilesContainingCache(storage_dir, cache_dir);
        for(int k=0; k<2; k++){ // io
            io_on = k;
            for(int i=0;i<cache_size_types_size;i++){ // cache_size
                cache_size_index = i;
                for(int j=0;j<policy_types_size;j++){ // cache_policy
                    caching_policy_index = j;
                    runRealOnce();
                }
            }
        }
    }
}



// [tmp test]not copy disk.bin & cache.bin& use original bin
void runRealTest(){
    makeSaveRoot();
    // trace dir
    auto trace_root_dir = "../trace/";
    auto trace_dirs = findTracePathsReal(trace_root_dir);

    for (const auto& dir : trace_dirs) {
        trace_dir = dir;
        trace_path = trace_dir+"/trace.txt";
        std::cout<<"trace_path: "<<trace_path<<std::endl;
        
        cache_dir = storage_dir = trace_dir + "/storage/";
        
        io_on = 0;

        cache_size_index = 4;

        caching_policy_index = 6;
        runRealOnce();

        // for(int j=0;j<policy_types_size;j++){ // cache_policy
        //     caching_policy_index = j;
        //     runRealOnce();
        // }
    }
}


// ycsb test
void initParmYcsb(){
    fstream fin_trace(trace_path);
    checkFile(fin_trace);
    string s;
    ll block_size_KB;
    fin_trace>>s>>block_num>>disk_size>>trace_size>>block_size_KB;
    disk_size = disk_size_KB / block_size_KB;
    cache_size = disk_size*cache_size_factor;
    block_size = block_size_KB * 1024;
    cout<<"block_num: "<<block_num<<", disk_size: "<<disk_size<<", trace_size: "<<trace_size
        <<", block_size_KB: "<<block_size_KB<<", cache_size: "<<cache_size<<endl;
    assert(cache_size >= 10); // block_num >= 500
}

void runYcsbOnce(){
    std::this_thread::sleep_for(std::chrono::seconds(20));

    printf("--------------------------------------------------------------------------------\n");
    std::unique_ptr<Sl> sim = nullptr;
    cache_size_factor = cacheSizeTypes[cache_size_index];
    initParmYcsb();
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


void runYcsb(){
    makeSaveRoot();
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    string trace_root_dir = "../trace/";
    // auto trace_root_dir = "../../trace_wait/";
    auto trace_root_names = findTraceRootNames(trace_root_dir);
    for (const auto& root_name : trace_root_names) { // disk_size = trace_name
        cout << "trace: " << root_name << endl;
        string trace_root = trace_root_dir + '/' + root_name;
        disk_size_KB = extractDiskSizeKB(root_name);
        cout << "disk_size_KB: " << disk_size_KB << " KB" << endl;
        storage_dir = trace_root + "/storage/";
        copyFilesContainingCache(storage_dir, cache_dir);
        auto trace_dirs = findTracePathsYcsb(trace_root);
        for (const auto& dir : trace_dirs) {
            trace_dir = dir;
            trace_path = trace_dir+"/trace.txt";
            std::cout<<"single_trace_path: "<<trace_path<<std::endl;
            for(int k=0; k<2; k++){ // io
                io_on = k;
                for(int i=0;i<cache_size_types_size;i++){ // cache_size
                    cache_size_index = i;
                    for(int j=0;j<policy_types_size;j++){ // cache_policy
                        caching_policy_index = j;
                        runYcsbOnce();
                    }
                }
            }
        }
    }
}

void runYcsbTest(){
    makeSaveRoot();
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    string trace_root_dir = "../trace/";
    auto trace_root_names = findTraceRootNames(trace_root_dir);
    for (const auto& root_name : trace_root_names) { // disk_size = trace_name
        cout << "trace: " << root_name << endl;
        string trace_root = trace_root_dir + '/' + root_name;
        disk_size_KB = extractDiskSizeKB(root_name);
        storage_dir = trace_root + "/storage/";
        copyFilesContainingCache(storage_dir, cache_dir);
        auto trace_dirs = findTracePathsYcsb(trace_root);
        for (const auto& dir : trace_dirs) {
            trace_dir = dir;
            trace_path = trace_dir+"/trace.txt";
            std::cout<<"single_trace_path: "<<trace_path<<std::endl;
                io_on = 1;
                for(int i=0;i<cache_size_types_size;i++){ // cache_size
                    cache_size_index = i;
                    caching_policy_index = 3;
                    runYcsbOnce();
                }
        }
    }
}


void runYcsbIoOnCacheSize10pp(){
    makeSaveRoot();
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    string trace_root_dir = "../trace/";
    // auto trace_root_dir = "../../trace_wait/";
    auto trace_root_names = findTraceRootNames(trace_root_dir);
    for (const auto& root_name : trace_root_names) { // disk_size = trace_name
        cout << "trace: " << root_name << endl;
        string trace_root = trace_root_dir + '/' + root_name;
        disk_size_KB = extractDiskSizeKB(root_name);
        cout << "disk_size_KB: " << disk_size_KB << " KB" << endl;
        storage_dir = trace_root + "/storage/";
        copyFilesContainingCache(storage_dir, cache_dir);
        auto trace_dirs = findTracePathsYcsb(trace_root);
        for (const auto& dir : trace_dirs) {
            trace_dir = dir;
            trace_path = trace_dir+"/trace.txt";
            std::cout<<"single_trace_path: "<<trace_path<<std::endl;
            io_on = 1;  // io = on
            cache_size_index = cache_size_types_size-1; // cache_size = 0.1
            for(int j=0;j<policy_types_size;j++){ // cache_policy
                caching_policy_index = j;
                runYcsbOnce();
            }
        }
    }
}

void runYcsbIoOnCacheSize10ppRandom(){
    makeSaveRoot();
    // cache dir
    cache_dir = "/mnt/eMMC/";
    mkdir(cache_dir);
    cout<<"cache_dir: "<<cache_dir<<endl;
    // trace dir
    string trace_root_dir = "../trace/";
    // auto trace_root_dir = "../../trace_wait/";
    auto trace_root_names = findTraceRootNames(trace_root_dir);
    for (const auto& root_name : trace_root_names) { // disk_size = trace_name
        cout << "trace: " << root_name << endl;
        string trace_root = trace_root_dir + '/' + root_name;
        disk_size_KB = extractDiskSizeKB(root_name);
        cout << "disk_size_KB: " << disk_size_KB << " KB" << endl;
        storage_dir = trace_root + "/storage/";
        copyFilesContainingCache(storage_dir, cache_dir);
        auto trace_dirs = findTracePathsYcsb(trace_root);
        for (const auto& dir : trace_dirs) {
            trace_dir = dir;
            trace_path = trace_dir+"/trace.txt";
            std::cout<<"single_trace_path: "<<trace_path<<std::endl;
            io_on = 1;  // io = on
            cache_size_index = cache_size_types_size-1; // cache_size = 0.1
            caching_policy_index = 0; // cache_policy = random
            runYcsbOnce();
        }
    }
}

// No Cache Test

void initParmNoCache(){
    fstream fin_trace(trace_path);
    checkFile(fin_trace);
    string s;
    ll block_size_KB;
    fin_trace>>s>>block_num>>disk_size>>trace_size>>block_size_KB;
    block_size = block_size_KB * 1024;
    cout<<"block_num: "<<block_num<<", disk_size: "<<disk_size<<", trace_size: "<<trace_size
        <<", block_size_KB: "<<block_size_KB<<", cache_size: "<<cache_size<<endl;
}


// @brief 测试无缓存直接读写设备(sd/eMMC)的性能
// @param trace_path
// @param device_id
// @param device_path
// @param block_size
void runNoCacheOnce(std::string device_id, std::string device_path)
{
    std::this_thread::sleep_for(std::chrono::seconds(20));
    std::cout <<"device_id: " << device_id << ", device_path: " << device_path << std::endl;
    NoCacheSl sl(device_id, device_path);
    sl.test();
    sl.statistic();
}

void runNoCache(){
    makeSaveRoot();

    std::string device_id;
    std::string device_path;
    std::string disk_name = "disk.bin";

    // emmc dir
    std::string emmc_dir = "/mnt/eMMC/";
    mkdir(emmc_dir);
    cout<<"emmc_dir: "<<emmc_dir<<endl;
    // sd dir
    std::string sd_dir = "../storage/";
    mkdir(sd_dir);
    cout<<"sd_dir: "<<sd_dir<<endl;
    // trace dir
    string trace_root_dir = "../trace/";
    auto trace_root_names = findTraceRootNames(trace_root_dir);
    for (const auto& root_name : trace_root_names) { // disk_size = trace_name
        cout << "trace: " << root_name << endl;
        string trace_root = trace_root_dir + '/' + root_name;
        disk_size_KB = extractDiskSizeKB(root_name);
        storage_dir = trace_root + "/storage/";
        string disk_dir = storage_dir + '/' + disk_name;
        copyFileToDirectory(disk_dir, sd_dir);
        copyFileToDirectory(disk_dir, emmc_dir);
        auto trace_dirs = findTracePathsYcsb(trace_root);
        for (const auto& dir : trace_dirs) {
            trace_dir = dir;
            trace_path = trace_dir+"/trace.txt";
            initParmNoCache();

            std::cout<<"single_trace_path: "<<trace_path<<std::endl;

            // sd
            device_id = "sd";
            device_path = sd_dir + "/" + disk_name;
            runNoCacheOnce(device_id, device_path);

            // emmc 
            device_id = "emmc";
            device_path = emmc_dir + "/" + disk_name;
            runNoCacheOnce(device_id, device_path);

        }
    }
}

#endif /*_RUN_HPP_INCLUDED_*/
