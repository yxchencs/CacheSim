#ifndef _GLOBALS_HPP_INCLUDED_
#define _GLOBALS_HPP_INCLUDED_
#include <string>
typedef long long ll;

size_t block_num;
ll disk_size; // Unit: block_size
ll disk_size_KB; // Unit: KB
bool io_on; // Controls whether read/write IO is enabled
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

const bool RANDOM_ON = false; // Controls whether or not randomized writes are written to cache, i.e., adds a layer of cache write filters on top of all Caching-Policy, noting that it has nothing to do with the RANDOM algorithm
const int RANDOM_THRESHOLD = 0;  // Probability of random write to cache [0-1]
/*  O_DIRECT bypasses the buffer cache and directs IO. The following are the usage restrictions:
        0. block size (block size) refers to the physical block size of the device, usually 512 bytes
        1. The buffer used to transfer data, its memory boundaries must be aligned to an integer multiple of the block size
        2. The start point of data transfer, i.e., the offset between the file and the device, must be an integer multiple of the block size.
        3. The length of the data to be transferred must be an integer multiple of the block size.
    Failure to comply with any of the above restrictions will result in an EINVAL error.
*/
const bool O_DIRECT_ON = true; // Controls whether O_DIRECT is turned on
const ll N = 100; // run N epochs in runMonteCarloSimulation()
#endif /*_GLOBALS_INCLUDED_*/
