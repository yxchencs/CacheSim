#ifndef _LRU_SIMULATORHPP_INCLUDED_
#define	_LRU_SIMULATORHPP_INCLUDED_

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

//for file_open
#include <sys/types.h>		
#include <fcntl.h>			
#include <unistd.h>			
#include <errno.h>			
#include <sys/ioctl.h>
#include <sys/time.h>		

#include "../utils/bitmap.h"
#include "../utils/chunk.h"
#include "../utils/config.h"
#include "../utils/statistic.h"


using namespace std;

class Sl{
public:

    void test();
    void statistic();
    void closeFile();
    
protected:

    int fd_cache,fd_disk;

    long long curKey;
    vector<long long> free_cache;
    map<long long, chunk> chunk_map;
    Statistic st;
    
    Sl();
    void init();
    void initFreeCache();
    void initFile();

    virtual bool readItem(vector<ll>& keys) = 0;
    virtual bool writeItem(vector<ll>& keys) = 0;
    virtual void writeCache(const ll& key) = 0;

    void writeBack(chunk* arg);

    void readDisk(const long long& key); // disk->cache disk->buffer
    void writeDisk(const long long& key);

    void readCache(const ll& offset_cache); // cache->buffer
    void coverageCache(chunk* arg);

    void normRead(bool isCache, const long long& offset, const long long& size);
    void normWrite(bool isCache, const long long& offset, const long long& size);
    void odirectRead(bool isCache, const long long& offset, const long long& size);
    void odirectWrite(bool isCache, const long long& offset, const long long& size);

    void readChunk(bool isCache, const long long& offset, const long long& size);
    void writeChunk(bool isCache, const long long& offset, const long long& size);
    
    void printChunk(chunk* arg);
    void printChunkMap();
    void printFreeCache();

    bool isWriteCache();//使用随机策略，根据概率决定是否写入cache

    void checkFile(fstream &file);
};

void Sl::test() {
    cout << "-----------------------------------------------------------------" << endl;
    cout << "test start" << endl;
    st.getStartTime();
    
    fstream fin(TRACE_PATH);
    checkFile(fin);
    
    ll curSize; int type; char c; string s;
    getline(fin, s);
    
    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0,NULL);
    while (fin >> curKey >> c >> curSize >> c >> type)
    {
        if(type==1) continue;
        cout<<"----------"<<curKey<<' '<<curSize<<' '<<type<<"----------"<<endl;

        st.total_trace_nums++;
        bool isTraceHit;
        
        ll begin = curKey / CHUNK_SIZE;
        ll end = (curKey + curSize - 1) / CHUNK_SIZE;
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_size += end - begin + 1;
        
        vector<ll> keys;
        for (ll i = begin; i <= end; i++) {
            keys.push_back(i * CHUNK_SIZE);
        }
        
        gettimeofday(&t1,NULL);
        
        switch (type){
        case 0:
            isTraceHit = readItem(keys);
            break;
        case 1:
            isTraceHit = writeItem(keys);
            break;
        }

        gettimeofday(&t2,NULL);
        long long deltaT = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
        st.latency_v.push_back(deltaT);
        st.total_latency += deltaT;
        printf("trace: %llu time: %lld us total: %lld us\n", st.total_trace_nums, deltaT, st.total_latency);//printf("trace: %llu time: %llu ns\n", st.total_trace_nums, deltaT);
        if (isTraceHit) st.hit_trace_nums++;
        printChunkMap();
    }
    gettimeofday(&t3,NULL);
    st.total_time = (t3.tv_sec-t0.tv_sec)*1000000+(t3.tv_usec-t0.tv_usec);
    st.getEndTime();
}

void Sl::checkFile(fstream &file){
    if (!file.is_open()) {
        cout << "Error: opening trace file fail" << endl;
        exit(1);
    }
}

bool Sl::isWriteCache(){
    if(!RANDOM_ON) return true;

    int min = 1,max = 100;
    random_device seed;//硬件生成随机数种子
    ranlux48 engine(seed());//利用种子生成随机数引擎
    uniform_int_distribution<> distrib(min, max);//设置随机数范围，并为均匀分布
    int random = distrib(engine);//随机数
    // cout<<"random: "<<random<<endl;
    // printf("random: %d\n",random);
    if(random <= RANDOM_THRESHOLD)
        // printf("do write cache\n");
        return true;

    // printf("don't write cache\n");
    return false;
}

Sl::Sl(){   
    init();
    cout << "CachePolicy Construct success" << endl;
}

void Sl::init() {
    initFreeCache();
    initFile();
    cout << "init success" << endl;
}

void Sl::initFile(){
    fd_cache = open(CACHE_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_cache>=0);
    fd_disk = open(DISK_PATH, O_RDWR | O_DIRECT, 0664);
    assert(fd_disk>=0);
}

void Sl::closeFile(){
    close(fd_cache);
    close(fd_disk);
}

void Sl::printFreeCache() {
    cout << "free cache: " << endl;
    for (int i = 0; i < free_cache.size(); i++) {
        cout << free_cache[i] << ' ';
    }
    cout << endl;
    //cout << free_cache.back() << endl;
}

void Sl::writeBack(chunk* arg) {
    if (arg->dirty == 1) {
        arg->dirty = 0;
        writeDisk(arg->key);
    }
}

void Sl::normRead(bool isCache, const long long& offset, const long long& size){
   int fd=-1;
    if(isCache) fd=fd_cache;
    else fd=fd_disk;
    assert(fd >= 0);
    char buffer[CHUNK_SIZE];

    int res = pread64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::normWrite(bool isCache, const long long& offset, const long long& size){
    int fd=-1;
    if(isCache) fd=fd_cache;
    else fd=fd_disk;
    assert(fd >= 0);
    char buffer[CHUNK_SIZE] = "Ram15978";

    int res = pwrite64(fd, buffer, size, offset);
    assert(res == size);

    // close(fd);
}

void Sl::odirectRead(bool isCache, const long long& offset, const long long& size){
    int fd=-1;
    if(isCache) fd=fd_cache;
    else fd=fd_disk;
    assert(fd >= 0);
    char *buffer = nullptr;
    int res = posix_memalign((void **) &buffer, CHUNK_SIZE, size);
    assert(res == 0);

    res = pread64(fd, buffer, size, offset);
    // printf("odirectRead: %d\n",res);
    assert(res == size);

    free(buffer);
    // close(fd);
}

void Sl::odirectWrite(bool isCache, const long long& offset, const long long& size){
    int fd=-1;
    if(isCache) fd=fd_cache;
    else fd=fd_disk;
    // cout<<filePath<<endl;
    assert(fd >= 0);
    char *buffer = nullptr;
    int res = posix_memalign((void **) &buffer, CHUNK_SIZE, size);
    assert(res == 0);

    strcpy(buffer, "Ram15978");
    res = pwrite64(fd, buffer, size, offset);
    // cout<<"odirectWrite: "<<res<<' '<<fd<<' '<<buffer<<' '<<size<<' '<<offset<<endl;
    // printf("odirectWrite: %d\n",res);
    assert(res == size);

    free(buffer);
    // close(fd);
}

void Sl::readChunk(bool isCache, const long long& offset, const long long& size) {
    assert(offset!=-1);
    if(O_DIRECT_ON) odirectRead(isCache,offset,size);
    else normRead(isCache,offset,size);
}

void Sl::writeChunk(bool isCache, const long long& offset, const long long& size) {
    assert(offset!=-1);
    if(O_DIRECT_ON) odirectWrite(isCache,offset,size);
    else normWrite(isCache,offset,size);
}

void Sl::printChunk(chunk* arg) {
    cout << "key: " << arg->key << endl;
    cout << "offset_cache: " << arg->offset_cache << endl;
    cout << "dirty: " << arg->dirty << endl;
}

void Sl::initFreeCache() {
    for (long long i = 0; i < CACHE_SIZE; i++) {
        free_cache.push_back(i * CHUNK_SIZE);
        // cout<<i * CHUNK_SIZE<<" has pushed in to free cache"<<endl;
    }
}

void Sl::readCache(const ll& offset_cache) {
    // printf("readCache\n");
    assert(offset_cache!=-1);
    readChunk(true, offset_cache,CHUNK_SIZE);
}

void Sl::readDisk(const long long& key) {
    // printf("readDisk\n");
    assert(key!=-1);
    readChunk(false, key, CHUNK_SIZE);
}

void Sl::printChunkMap() {
    cout << "chunk map:" << endl;
    map<long long, chunk>::iterator it;          
    for (it = chunk_map.begin(); it != chunk_map.end(); it++)
        cout << it->second;
}

void Sl::coverageCache(chunk* arg) {
    // cout<<"coverageCache"<<endl;
    arg->dirty = 1;
    writeChunk(true, arg->offset_cache, CHUNK_SIZE);
}

void Sl::writeDisk(const long long& key) {
    // cout<<"writeDisk"<<endl;
    writeChunk(false, key, CHUNK_SIZE);
}


void Sl::statistic(){
    // st.printStatistic();
    st.writeStatistic();
}

#endif /*_LRU_SIMULATORHPP_INCLUDED_*/
