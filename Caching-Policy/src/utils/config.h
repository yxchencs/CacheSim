#ifndef _CONFIG_HPP_INCLUDED_
#define _CONFIG_HPP_INCLUDED_
#include "policy.h"

// const PolicyType POLICY = policyTypes[0]; // 0~8
// const double CACHE_SIZE_LIST[5]={0.02,0.04,0.06,0.08,0.1};

const long long DISK_SIZE = 33668; // disk_capacity=DISK_SIZE*CHUNK_SIZE
const long long CHUNK_NUM = 33668;

const char *TRACE_PATH = "../trace/zipfian/zipfian_r20w_o15w_0.99/trace.txt";
const char *CACHE_PATH = "../trace/zipfian/zipfian_r20w_o15w_0.99/storage/cache_0.1.bin";
const char *DISK_PATH = "../trace/zipfian/zipfian_r20w_o15w_0.99/storage/disk.bin";

typedef long long ll;
const long long CHUNK_SIZE = 4 * 1024; // 4kB
const long long CACHE_SIZE=CHUNK_NUM*0.1;

// const char* TRACE_PATH="/home/odroid/R/Caching-Policy/Caching-Policy/trace/zipfian/1.15_1000000_150000_1_false/trace.txt";
// const char* TRACE_PATH = "/mnt/d/Projects/Caching-Policy/Caching-Policy/trace/latest/1000000_150000_1_false/trace.txt";
// const char* TRACE_PATH = "/home/odroid/R/Caching-Policy/Caching-Policy/trace/real/MobileAPPTraces/mojitianqi/mojitianqi.txt";
// const char *TRACE_PATH = "../trace/real/MobileAPPTraces/mojitianqi/mojitianqi.txt";
// const char* TRACE_PATH = "/home/odroid/R/Caching-Policy/Caching-Policy/trace/real/MobileAPPTraces/qq/qq.txt";
//  const char* TRACE_PATH = "/home/odroid/R/Caching-Policy/Caching-Policy/trace/test.txt";
//  const char* TRACE_PATH = "../trace/test.txt";

// const char* TRACE_PATH = "/home/odroid/R/Caching-Policy/Caching-Policy/trace/test_trace.txt";
// const char* CACHE_PATH="../storage/cache_0.02.bin";
// const char* CACHE_PATH="/mnt/eMMC/cache_0.02.bin";
// const char* CACHE_PATH="/dev/mmcblk0";
// const char* DISK_PATH="/dev/mmcblk0";
//  const char* CACHE_PATH="/mnt/d/Projects/Caching-Policy/Caching-Policy/storage/cache.bin";
//  const char* DISK_PATH="/mnt/d/Projects/Caching-Policy/Caching-Policy/storage/disk.bin";


/* O_DIRECT 绕过缓冲区高速缓存，直接IO，以下为使用限制：
    0.块大小(block size)指设备的物理块大小，通常为512字节
    1.用于传递数据的缓冲区，其内存边界必须对齐为块大小的整数倍
    2.数据传输的开始点，即文件和设备的偏移量，必须是块大小的整数倍
    3.待传递数据的长度必须是块大小的整数倍。
    不遵守上述任一限制均将导致EINVAL错误
*/
const bool O_DIRECT_ON = true;
const bool RANDOM_ON = false;
const int RANDOM_THRESHOLD = 0; // 10 -> 0.1的概率写入缓存

#endif /*_CONFIG_INCLUDED_*/
