#ifndef _GLOBALS_HPP_INCLUDED_
#define _GLOBALS_HPP_INCLUDED_
#include <string>
typedef long long ll;

double cache_size_factor;
size_t cache_size;
std::string cache_path;

const ll DISK_SIZE = 33668;
const size_t CHUNK_NUM = 33668;
const char *TRACE_PATH = "../trace/zipfian/zipfian_r100w_o15w_0.99/trace.txt";
const char *DISK_PATH = "../trace/zipfian/zipfian_r100w_o15w_0.99/storage/disk.bin";
const std::string CACHE_PATH_HEAD = "../trace/zipfian/zipfian_r100w_o15w_0.99/storage/";

const bool IO_ON = true; // 控制是否开启读写IO

const bool RANDOM_ON = false; // 控制是否随机写入cache，即在所有Caching-Policy外面再加上一层cache写入过滤器，注意与RANDOM算法无关
const int RANDOM_THRESHOLD = 0;  // 随机写入cache的概率[0-1]
// /* O_DIRECT 绕过缓冲区高速缓存，直接IO，以下为使用限制：
//     0.块大小(block size)指设备的物理块大小，通常为512字节
//     1.用于传递数据的缓冲区，其内存边界必须对齐为块大小的整数倍
//     2.数据传输的开始点，即文件和设备的偏移量，必须是块大小的整数倍
//     3.待传递数据的长度必须是块大小的整数倍。
//     不遵守上述任一限制均将导致EINVAL错误
// */
const bool O_DIRECT_ON = true; // 控制是否开启O_DIRECT
#endif /*_GLOBALS_INCLUDED_*/
