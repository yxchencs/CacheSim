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

#include "../simulator/randomSl.h"
#include "../simulator/fifoSl.h"
#include "../simulator/lruSl.h"
#include "../simulator/lfuSl.h"
#include "../simulator/lirsSl.h"
#include "../simulator/arcSl.h"
#include "../simulator/clockproSl.h"
#include "../simulator/2qSl.h"
#include "../simulator/tinylfuSl.h"

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

bool copy_file_to_directory(const fs::path& source_file, const fs::path& target_directory) {
    fs::path target_file = target_directory / source_file.filename();

    try {
        // 复制文件
        fs::copy(source_file, target_file, fs::copy_options::overwrite_existing);

        // 设置目标文件的权限
        // 给所有用户读写权限
        fs::permissions(source_file, fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::group_write |
                        fs::perms::others_read | fs::perms::others_write);
        fs::permissions(target_file, fs::perms::owner_read | fs::perms::owner_write |
                        fs::perms::group_read | fs::perms::group_write |
                        fs::perms::others_read | fs::perms::others_write);

        return true; // 复制并设置权限成功
    } catch (const fs::filesystem_error& e) {
        std::cerr << "复制文件或设置权限时出错: " << e.what() << std::endl;
        return false; // 操作失败
    }
}

void copy_files_containing_cache(const fs::path& source_directory, const fs::path& target_directory) {
    if (!fs::exists(source_directory) || !fs::is_directory(source_directory)) {
        std::cerr << "源目录不存在或不是一个有效目录。" << std::endl;
        return;
    }

    for (const auto& entry : fs::directory_iterator(source_directory)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("cache") != std::string::npos) {
                // if (copy_file_to_directory(entry.path(), target_directory)) {
                    // std::cout << "已复制: " << filename << std::endl;
                // }
                assert(copy_file_to_directory(entry.path(), target_directory));
            }
        }
    }
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

// 若目录不存在，则创建目录
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

// 运行需要的函数
void initCacheSize(){
    fstream fin_trace(trace_path);
    checkFile(fin_trace);
    string s;
    fin_trace>>s>>chunk_num>>disk_size;
    cache_size = chunk_num*cache_size_factor;
    // cout<<s<<", "<<chunk_num<<", "<<disk_size<<", "<<cache_size<<endl;
}

void run_once(){
    printf("--------------------------------------------------------------------------------\n");
    Sl *sim = nullptr;
    cache_size_factor = cacheSizeTypes[cache_size_index];
    initCacheSize();
    cache_path = cache_dir+cachePath[cache_size_index];
    cout<<"cache_path: "<<cache_path<<endl;
    switch(policyTypes[caching_policy_index]){
        case PolicyType::RANDOM: 
            sim = new RandomSl();
            break;
        case PolicyType::FIFO:
            sim = new FifoSl();
            break;
        case PolicyType::LFU:
            sim = new LfuSl();
            break;
        case PolicyType::LRU:
            sim = new LruSl();
            break;
        case PolicyType::LIRS:
            sim = new LirsSl();
            break;
        case PolicyType::ARC:
            sim = new ArcSl();
            break;
        case PolicyType::CLOCKPRO:
            sim = new ClockproSl();
            break;
        case PolicyType::TQ:
            sim = new TqSl();
            break;
        case PolicyType::TINYLFU:
            sim = new TinylfuSl();
            break;
    }

    sim->test();
    sim->statistic();
}

void run(){
    save_root = "../../records/" + getCurrentDateTime() + '/';
    cout<<"save_root: "<<save_root<<endl;
    mkdir(save_root);
    cache_dir = "/mnt/eMMC/";
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
            for(int i=0;i<cache_size_types_size;i++){
                cache_size_index = i;
                for(int j=0;j<policy_types_size;j++){
                    caching_policy_index = j;
                    run_once();
                }
            }
        }
    }
}

#endif /*_RUN_HPP_INCLUDED_*/