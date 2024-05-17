#ifndef _SIMULATOR_HPP_INCLUDED_
#define _SIMULATOR_HPP_INCLUDED_

#include <iostream>
#include <fstream>
#include <streambuf>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <cassert>
#include <set>
#include <map>
#include <random>
#include <string> // for to_string()

// for file_open
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "../utils/bitmap.h"
#include "../utils/block.h"
#include "../utils/globals.h"
#include "../utils/statistic.h"
#include "../utils/cacheConf.h"
#include "../utils/policy.h"
#include "../utils/run.h"
#include "../utils/progressBar.h"

using namespace std;

void checkFile(fstream &file);
class Sl
{
public:
    Sl();
    virtual ~Sl();
    void test();
    void statistic();

protected:
    string disk_path;

    int fd_cache, fd_disk;
    fstream fin_trace;
    unique_ptr<char[]> buffer_read;
    unique_ptr<char[]> buffer_write; 

    ll curKey;
    vector<ll> free_cache;
    map<ll, block> block_map;
    Statistic st;
    void init();
    void initFreeCache();
    void initFile();
    void closeFile();

    bool readItem(vector<ll> &keys);
    bool writeItem(vector<ll> &keys);
    // virtual void writeCache(const ll &key, char* buffer);
    virtual void writeCacheWhenReadItem(const ll &key, char* buffer);
    virtual void writeCacheWhenWriteItem(const ll &key, char* buffer);

    void writeBack(const ll &key);

    void readDisk(const ll &key, char* buffer); // disk->cache disk->buffer
    void writeDisk(const ll &key, char* buffer);

    void readCache(const ll &offset_cache, char* buffer); // cache->buffer

    void read(bool isCache, const ll &offset, const ll &size, char* buffer);
    void write(bool isCache, const ll &offset, const ll &size, char* buffer);

    void readBlock(bool isCache, const ll &offset, const ll &size, char* buffer);
    void writeBlock(bool isCache, const ll &offset, const ll &size, char* buffer);

    void printBlock(const ll &key);
    void printBlockMap();
    void printFreeCache();

    bool isWriteCache(); // 使用随机策略，根据概率决定是否写入cache

    virtual bool isCached(const ll &key) = 0;
    virtual void accessKey(const ll &key, const bool &isGet) = 0;
    virtual ll getVictim() = 0;
};

bool Sl::readItem(vector<ll> &keys)
{
    bool isTraceHit = true;
    st.read_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (isCached(keys[i]))
        {
            st.read_hit_nums += 1;
            accessKey(keys[i], true); // [lirs] cache_map.Get(keys[i]);
            readCache(block_map[keys[i]].offset_cache, buffer_read.get()); // cache -> buffer
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            // cout<<"cache miss"<<endl;
            isTraceHit = false;
            accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);
            readDisk(keys[i], buffer_read.get());
            writeCacheWhenReadItem(keys[i], buffer_read.get()); // disk -> cache
        }
    }
    // for (int i = 0; i < keys.size(); i++){
    //     cout<<keys[i]<<' ';
    // }
    // cout<<endl;
    return isTraceHit;
}

bool Sl::writeItem(vector<ll> &keys)
{
    bool isTraceHit = true;
    st.write_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (isCached(keys[i]))
        {
            st.write_hit_nums += 1;
            accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);
            block_map[keys[i]].dirty = 1;
            writeBlock(true, block_map[keys[i]].offset_cache, block_size, buffer_write.get());
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            // cout<<"miss "<<keys[i]<<endl;
            isTraceHit = false;
            accessKey(keys[i], false); // [lirs] cache_map.Add(keys[i],0);
            writeCacheWhenWriteItem(keys[i], buffer_write.get()); // buffer -> cache
        }
    }
    // for (int i = 0; i < keys.size(); i++){
    //     cout<<keys[i]<<' ';
    // }
    // cout<<endl;
    return isTraceHit;
}

void Sl::writeCacheWhenReadItem(const ll &key, char* buffer)
{
    // cout << "writeCacheWhenReadItem";
    // printf("Sl::writeCacheWhenReadItem\n");
    // cache not full
    if (!free_cache.empty())
    {
        // cout << "cache not full" << endl;
        ll offset_cache = free_cache.back();
        block item = {key, offset_cache};
        block_map[key] = item;
        free_cache.pop_back();
        writeBlock(true, offset_cache, block_size, buffer);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        ll victim = getVictim();
        assert(victim != -1);

        writeBack(victim);
        
        assert(block_map.find(victim) != block_map.end());
        ll offset_cache = block_map[victim].offset_cache;
        assert(offset_cache != -1);
        block_map[victim].offset_cache = -1;

        if (block_map.count(key) == 0)
        {
            block item = {key, offset_cache};
            block_map[key] = item;
        }
        else
        {
            block_map[key].offset_cache = offset_cache;
        }
        writeBlock(true, offset_cache, block_size, buffer);
    }
}

void Sl::writeCacheWhenWriteItem(const ll &key, char* buffer)
{
    // printf("Sl::writeCacheWhenWriteItem\n");
    // cout << "writeCacheWhenWrite";

    // cache not full
    if (!free_cache.empty())
    {
        // cout << "cache not full" << endl;
        ll offset_cache = free_cache.back();
        block item = {key, offset_cache, 1};
        block_map[key] = item;
        free_cache.pop_back();
        writeBlock(true, offset_cache, block_size, buffer);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        ll victim = getVictim();
        assert(victim != -1);       

        writeBack(victim);
        
        assert(block_map.find(victim) != block_map.end());
        ll offset_cache = block_map[victim].offset_cache;
        assert(offset_cache != -1);
        block_map[victim].offset_cache = -1;

        if (block_map.count(key) == 0)
        {
            block item = {key, offset_cache, 1};
            block_map[key] = item;
        }
        else
        {
            block_map[key].offset_cache = offset_cache;
            block_map[key].dirty = 1;
        }
        writeBlock(true, offset_cache, block_size, buffer);
    }
}

void Sl::test()
{
    // cout << "-----------------------------------------------------------------" << endl;
    // printf("test start\n");
    st.getStartTime();

    fstream fin_trace(trace_path);
    checkFile(fin_trace);

    ll curSize;
    int type;
    char c;
    string s;
    getline(fin_trace,s);

    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0, NULL);
    while (fin_trace >> curKey >> c >> curSize >> c >> type)
    {
        // cout << "----------" << curKey << ' ' << curSize << ' ' << type << "----------" << endl;
        st.total_trace_nums++;
        showProgressBar(st.total_trace_nums, trace_size);


        ll begin = curKey / block_size;
        ll end = (curKey + curSize - 1) / block_size;

        st.request_size_v.push_back(end - begin + 1);
        st.total_request_number += end - begin + 1;
        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * block_size);
        }

        // st.request_size_v.push_back(block_size);
        // st.total_request_number += 1;
        // vector<ll> keys;
        // keys.push_back(curKey * block_size);

        gettimeofday(&t1, NULL);

        bool isTraceHit;
        switch (type)
        {
        case 0:
            isTraceHit = readItem(keys);
            break;
        case 1:
            isTraceHit = writeItem(keys);
            break;
        }

        gettimeofday(&t2, NULL);
        st.total_latency.addDeltaT(st.computeDeltaT(t1,t2));
        if (isTraceHit) st.hit_trace_nums++;
    }
    gettimeofday(&t3, NULL);
    st.total_time = st.computeDeltaT(t0,t3);
    st.getEndTime();
    // printf("test end\n");
}

bool Sl::isWriteCache()
{
    if (!RANDOM_ON)
        return true;

    int min = 1, max = 100;
    random_device seed;                           // 硬件生成随机数种子
    ranlux48 engine(seed());                      // 利用种子生成随机数引擎
    uniform_int_distribution<> distrib(min, max); // 设置随机数范围，并为均匀分布
    int random = distrib(engine);                 // 随机数
    // cout<<"random: "<<random<<endl;
    // printf("random: %d\n",random);
    if (random <= RANDOM_THRESHOLD)
        // printf("do write cache\n");
        return true;

    // printf("don't write cache\n");
    return false;
}

Sl::Sl()
{
    init();
    // cout << "CachePolicy Construct success" << endl;
}

Sl::~Sl()
{
    closeFile();
    block_map.clear();
    free_cache.clear();
}

void Sl::init()
{
    initFile();
    initFreeCache();
    // cout << "init success" << endl;
}

void Sl::initFile()
{
    // printf("initFile\n");
    disk_path = storage_dir + "disk.bin";
    printf("disk_path: %s", disk_path.c_str());
    if (O_DIRECT_ON)
    {
        fd_cache = open(cache_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd_cache >= 0);
        fd_disk = open(disk_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd_disk >= 0);
    }
    else
    {
        fd_cache = open(cache_path.c_str(), O_RDWR, 0664);
        assert(fd_cache >= 0);
        fd_disk = open(disk_path.c_str(), O_RDWR, 0664);
        assert(fd_disk >= 0);
    }

    buffer_read = make_unique<char[]>(block_size);  
    buffer_write = make_unique<char[]>(block_size); 
    memset(buffer_write.get(), 0, block_size); 
}

void Sl::closeFile()
{
    close(fd_cache);
    close(fd_disk);
}

void Sl::printFreeCache()
{
    cout << "free cache: ";
    for (int i = 0; i < free_cache.size(); i++)
    {
        cout << free_cache[i] << ' ';
    }
    cout << endl;
    // cout << free_cache.back() << endl;
}

void Sl::writeBack(const ll &key)
{
    assert(block_map.find(key) != block_map.end());
    ll offset_cache = block_map[key].offset_cache;
    if (block_map[key].dirty == 1)
    {
        block_map[key].dirty = 0;
        readCache(offset_cache, buffer_read.get());
        writeDisk(key, buffer_read.get());
    }
}

void Sl::read(bool isCache, const ll &offset, const ll &size, char* buffer)
{
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);

    int res = pread64(fd, buffer, size, offset);
    // cout<<"[read]isCache: "<<isCache<<", offset: "<<offset<<", res: "<<res<<endl;
    assert(res == size);
}

void Sl::write(bool isCache, const ll &offset, const ll &size, char* buffer)
{
    assert(offset != -1);
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    // cout<<filePath<<endl;
    assert(fd >= 0);

    int res = pwrite64(fd, buffer, size, offset);
    // cout<<"write: res: "<<res<<", fd: "<<fd<<", buffer: "<<buffer<<", size: "<<size<<", offset: "<<offset<<endl;
    // printf("write: %d\n",res);
    assert(res == size);
}

void Sl::readBlock(bool isCache, const ll &offset, const ll &size, char* buffer)
{
    // printf("readBlock\n");
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    if(!io_on) return;
    assert(offset != -1);
    if (O_DIRECT_ON)
        read(isCache, offset, size, buffer);
    gettimeofday(&end, NULL);
    if (isCache)
        st.cache_read_latency.addDeltaT(st.computeDeltaT(begin, end));
    else
        st.disk_read_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void Sl::writeBlock(bool isCache, const ll &offset, const ll &size, char* buffer)
{
    // printf("writeBlock\n");
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    if(!io_on) return;
    assert(offset != -1);
    if (O_DIRECT_ON)
        write(isCache, offset, size, buffer);
    gettimeofday(&end, NULL);
    if (isCache)
        st.cache_write_latency.addDeltaT(st.computeDeltaT(begin, end));
    else
        st.disk_write_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void Sl::printBlock(const ll &key)
{
    block blockTmp = block_map[key];
    cout << "key: " << blockTmp.key << endl;
    cout << "offset_cache: " << blockTmp.offset_cache << endl;
    cout << "dirty: " << blockTmp.dirty << endl;
}

void Sl::initFreeCache()
{
    for (ll i = 0; i < cache_size; i++)
    {
        free_cache.push_back(i * block_size);
        // cout<<i * block_size<<" has pushed in to free cache"<<endl;
    }
}

void Sl::readCache(const ll &offset_cache, char* buffer)
{
    // printf("readCache\n");
    assert(offset_cache != -1);
    readBlock(true, offset_cache, block_size, buffer);
}

void Sl::readDisk(const ll &key, char* buffer)
{
    // printf("readDisk\n");
    assert(key != -1);
    readBlock(false, key, block_size, buffer);
}

void Sl::printBlockMap()
{
    cout << "block map:" << endl;
    map<ll, block>::iterator it;
    for (it = block_map.begin(); it != block_map.end(); it++)
        cout << it->second;
}


void Sl::writeDisk(const ll &key, char* buffer)
{
    // cout << "writeDisk" << endl;
    writeBlock(false, key, block_size, buffer);
}

void Sl::statistic()
{
    // st.printStatistic();
    st.makeDefaultSaveDir();
    st.record();
}

#endif /*_SIMULATOR_HPP_INCLUDED_*/
