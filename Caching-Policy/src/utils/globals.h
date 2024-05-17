#ifndef _GLOBALS_HPP_INCLUDED_
#define _GLOBALS_HPP_INCLUDED_
#include <string>
typedef long long ll;

size_t block_num;
ll disk_size; // Unit: block_size
bool io_on; // 控制是否开启读写IO
int cache_size_index;
int caching_policy_index;
double cache_size_factor; // eg: 0.02
size_t cache_size; // Unit: B
ll block_size; // Unit: B
std::string cache_dir;
std::string cache_path;
std::string trace_dir;
std::string trace_path;
ll trace_size;
std::string storage_dir;
std::string save_root;
bool isAccessSingleBlock;

const bool RANDOM_ON = false; // 控制是否随机写入cache，即在所有Caching-Policy外面再加上一层cache写入过滤器，注意与RANDOM算法无关
const int RANDOM_THRESHOLD = 0;  // 随机写入cache的概率[0-1]
/* O_DIRECT 绕过缓冲区高速缓存，直接IO，以下为使用限制：
    0.块大小(block size)指设备的物理块大小，通常为512字节
    1.用于传递数据的缓冲区，其内存边界必须对齐为块大小的整数倍
    2.数据传输的开始点，即文件和设备的偏移量，必须是块大小的整数倍
    3.待传递数据的长度必须是块大小的整数倍。
    不遵守上述任一限制均将导致EINVAL错误
*/
const bool O_DIRECT_ON = true; // 控制是否开启O_DIRECT
#endif /*_GLOBALS_INCLUDED_*/
