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
#include "../utils/chunk.h"
#include "../utils/globals.h"
#include "../utils/statistic.h"
#include "../utils/cache_conf.h"
#include "../utils/policy.h"
#include "../utils/run.h"
#include "../utils/progress_bar.h"

using namespace std;

void checkFile(fstream &file);
class Sl
{
public:
    void test();
    void statistic();

protected:
    string disk_path;

    int fd_cache, fd_disk;
    fstream fin_trace;
    char * buffer_read = nullptr;
    char * buffer_write = nullptr;

    long long curKey;
    vector<long long> free_cache;
    map<long long, chunk> chunk_map;
    Statistic st;

    Sl();
    ~Sl();
    void init();
    void initFreeCache();
    void initFile();
    void closeFile();

    bool readItem(vector<ll> &keys);
    bool writeItem(vector<ll> &keys);
    virtual void writeCache(const ll &key);

    void writeBack(chunk *arg);

    void readDisk(const long long &key); // disk->cache disk->buffer
    void writeDisk(const long long &key);

    void readCache(const ll &offset_cache); // cache->buffer
    void coverageCache(chunk *arg);

    void normRead(bool isCache, const long long &offset, const long long &size);
    void normWrite(bool isCache, const long long &offset, const long long &size);
    void odirectRead(bool isCache, const long long &offset, const long long &size);
    void odirectWrite(bool isCache, const long long &offset, const long long &size);

    void readChunk(bool isCache, const long long &offset, const long long &size);
    void writeChunk(bool isCache, const long long &offset, const long long &size);

    void printChunk(chunk *arg);
    void printChunkMap();
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
            readCache(chunk_map[keys[i]].offset_cache);
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
            readDisk(keys[i]);
            writeCache(keys[i]);
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
            coverageCache(&chunk_map[keys[i]]);
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
            writeCache(keys[i]);
        }
    }
    // for (int i = 0; i < keys.size(); i++){
    //     cout<<keys[i]<<' ';
    // }
    // cout<<endl;
    return isTraceHit;
}

void Sl::writeCache(const ll &key)
{
    // cout << "writeCache: ";
    if (!isWriteCache())
        return;

    // cache not full
    if (!free_cache.empty())
    {
        // cout << "cache not full" << endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, chunk_size);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        ll victim = getVictim(); // [lirs] ll victim = cache_map.getCurVictim();
        assert(victim != -1);
        ll offset_cache = chunk_map[victim].offset_cache;
        chunk_map[victim].offset_cache = -1;
        if (chunk_map.count(key) == 0)
        {
            chunk item = {key, offset_cache};
            chunk_map[key] = item;
        }
        else
        {
            chunk_map[key].offset_cache = offset_cache;
        }
        writeChunk(true, offset_cache, chunk_size);
        writeBack(&chunk_map[victim]);
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
        show_progress_bar(st.total_trace_nums, trace_size);


        ll begin = curKey / chunk_size;
        ll end = (curKey + curSize - 1) / chunk_size;
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_number += end - begin + 1;
        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * chunk_size);
        }

        // st.request_size_v.push_back(chunk_size);
        // st.total_request_number += 1;
        // vector<ll> keys;
        // keys.push_back(curKey * chunk_size);

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
}

void Sl::init()
{
    initFile();
    initFreeCache();
    // cout << "init success" << endl;
}

void Sl::initFile()
{
    disk_path = storage_dir + "disk.bin";
    // cout<<"disk_path: "<<disk_path<<endl;
    if(O_DIRECT_ON){
        fd_cache = open(cache_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd_cache >= 0);
        fd_disk = open(disk_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd_disk >= 0);
    } else {
        fd_cache = open(cache_path.c_str(), O_RDWR, 0664);
        assert(fd_cache >= 0);
        fd_disk = open(disk_path.c_str(), O_RDWR, 0664);
        assert(fd_disk >= 0);
    }

    int res = posix_memalign((void **)&buffer_read, chunk_size, chunk_size);
    assert(res == 0);

    res = posix_memalign((void **)&buffer_write, chunk_size, chunk_size);
    assert(res == 0);
    memset(buffer_write, 0, chunk_size);
}

void Sl::closeFile()
{
    close(fd_cache);
    close(fd_disk);
    free(buffer_read);
    free(buffer_write);
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

void Sl::writeBack(chunk *arg)
{
    if (arg->dirty == 1)
    {
        arg->dirty = 0;
        writeDisk(arg->key);
    }
}

void Sl::normRead(bool isCache, const long long &offset, const long long &size)
{
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);
    char buffer[chunk_size];

    int res = pread64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::normWrite(bool isCache, const long long &offset, const long long &size)
{
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);
    char buffer[chunk_size] = "Ram15978";

    int res = pwrite64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::odirectRead(bool isCache, const long long &offset, const long long &size)
{
    // cout<<"odirectRead"<<endl;
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    assert(fd >= 0);

    int res = pread64(fd, buffer_read, size, offset);
    // printf("odirectRead: %d\n",res);
    assert(res == size);
}

void Sl::odirectWrite(bool isCache, const long long &offset, const long long &size)
{
    assert(offset != -1);
    int fd = -1;
    if (isCache)
        fd = fd_cache;
    else
        fd = fd_disk;
    // cout<<filePath<<endl;
    assert(fd >= 0);

    int res = pwrite64(fd, buffer_write, size, offset);
    // cout<<"odirectWrite: res: "<<res<<", fd: "<<fd<<", buffer: "<<buffer<<", size: "<<size<<", offset: "<<offset<<endl;
    // printf("odirectWrite: %d\n",res);
    assert(res == size);
}

void Sl::readChunk(bool isCache, const long long &offset, const long long &size)
{
    // printf("readChunk\n");
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    if(!io_on) return;
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectRead(isCache, offset, size);
    else
        normRead(isCache, offset, size);
    gettimeofday(&end, NULL);
    if (isCache)
        st.cache_read_latency.addDeltaT(st.computeDeltaT(begin, end));
    else
        st.disk_read_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void Sl::writeChunk(bool isCache, const long long &offset, const long long &size)
{
    // printf("writeChunk\n");
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    if(!io_on) return;
    assert(offset != -1);
    if (O_DIRECT_ON)
        odirectWrite(isCache, offset, size);
    else
        normWrite(isCache, offset, size);
    gettimeofday(&end, NULL);
    if (isCache)
        st.cache_write_latency.addDeltaT(st.computeDeltaT(begin, end));
    else
        st.disk_write_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void Sl::printChunk(chunk *arg)
{
    cout << "key: " << arg->key << endl;
    cout << "offset_cache: " << arg->offset_cache << endl;
    cout << "dirty: " << arg->dirty << endl;
}

void Sl::initFreeCache()
{
    for (long long i = 0; i < cache_size; i++)
    {
        free_cache.push_back(i * chunk_size);
        // cout<<i * chunk_size<<" has pushed in to free cache"<<endl;
    }
}

void Sl::readCache(const ll &offset_cache)
{
    // printf("readCache\n");
    assert(offset_cache != -1);
    readChunk(true, offset_cache, chunk_size);
}

void Sl::readDisk(const long long &key)
{
    // printf("readDisk\n");
    assert(key != -1);
    readChunk(false, key, chunk_size);
}

void Sl::printChunkMap()
{
    cout << "chunk map:" << endl;
    map<long long, chunk>::iterator it;
    for (it = chunk_map.begin(); it != chunk_map.end(); it++)
        cout << it->second;
}

void Sl::coverageCache(chunk *arg)
{
    // cout << "coverageCache" << endl;
    arg->dirty = 1;
    writeChunk(true, arg->offset_cache, chunk_size);
}

void Sl::writeDisk(const long long &key)
{
    // cout << "writeDisk" << endl;
    writeChunk(false, key, chunk_size);
}

void Sl::statistic()
{
    // st.printStatistic();
    st.makeDefaultSaveDir();
    st.record();
}

#endif /*_SIMULATOR_HPP_INCLUDED_*/
