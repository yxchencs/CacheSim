#ifndef _ARC_SIMULATOR_H_INCLUDED_
#define	_ARC_SIMULATOR_H_INCLUDED_
#include "../cache/arc.h"
#include "sl.h"
using namespace std;

class ArcSl : public Sl{
public:
    void test(); // 1,4097 ==> 0,4096 + 4096,4096  
private:
    ARC cache_map;

    void readItem();
    void writeItem();
    void writeCache(const long long& key);

};

void ArcSl::readItem() {
    st.read_nums+=1;
    long long key = curKey;
    // cache hit
    if (cache_map.Cached(key)) {
        st.read_hit_nums+=1;
        // cout << key << " hit" << endl;
        cache_map.arc_lookup(key);
        //printChunk(cache_map.arc_lookup(key));       
        readCache(chunk_map[key].offset_cache); // cache->buffer  
    }
    // cache miss
    else {
        // cout << key << " miss" << endl;
        //readDisk(key); // disk->buffer
        writeCache(key); // buffer->cache 
    }
    // cout <<"[read]" << "key=" << key << ", size=" << CHUNK_SIZE << endl;
}

void ArcSl::writeItem() {   
    st.write_nums+=1;
    long long key = curKey;
    // cache hit
    if (cache_map.Cached(key)) {
        st.write_hit_nums+=1;
        // cout << key << " hit" << endl;
        cache_map.arc_lookup(key);
        coverageCache(&chunk_map[key]);
    }
    // cache miss
    else {
        // cout << key << " miss" << endl;
        writeDisk(key);
        writeCache(key);
    }
    // cout << "[write]" << "key=" << key << ", size=" << CHUNK_SIZE << endl;
}

void ArcSl::writeCache(const long long& key) {
    if(!isWriteCache()) return;
    // cout << "write cache" << endl;
    // write cache_map
    if (!free_cache.empty()) {
        // cout << "cache not full" << endl;
        long long offset_cache = free_cache.back();
        chunk item = {key,offset_cache};
        chunk_map[key]=item;
        //cout << "arg->offset_cache " << arg->offset_cache << endl;
        free_cache.pop_back();
        cache_map.arc_lookup(key);
        //chunk_map[arg->key] = *arg;
        writeChunk(true, offset_cache, CHUNK_SIZE);
    }
    else { // cache full
        // cout << "cache full" << endl;
        long long key_erased = -1;
        key_erased = cache_map.arc_lookup(key);
        // cout<<"victim: "<<key_erased<<endl;
        long long offset_cache = chunk_map[key_erased].offset_cache;
        chunk_map[key_erased].offset_cache = -1;
        chunk item = {key,offset_cache};
        chunk_map[key]=item;
        //chunk_map[arg->key] = arg;
        writeChunk(true, offset_cache,CHUNK_SIZE);
        writeBack(&chunk_map[key_erased]);
    }
    // delete arg;
}

void ArcSl::test() {
    st.caching_policy="arc";
    cout<<"-----------------------------------------------------------------"<<endl;
    cout<<"test start"<<endl;
    ifstream fin(TRACE_PATH);
    long long curSize;
    int type;
    char c;
    string s;
    if (!fin.is_open()) {
        cout << "Error: opening trace file fail" << endl;
        exit(1);
    }
    getline(fin, s);
    // getline(fin, s);
    // getline(fin, s);
    timespec t0, t3, t1, t2;

    st.getStartTime();
    clock_gettime(CLOCK_MONOTONIC, &t0);
    while (fin >> curKey >> c >> curSize >> c >> type) {
        if(st.total_trace_nums>3) break;
        st.total_trace_nums++;
        // cout << curKey << " " << curSize << " " << type << endl;
        // long long times=(curSize-1)/CHUNK_SIZE+1;
        long long begin = curKey/CHUNK_SIZE;
        long long end = (curKey+curSize-1)/CHUNK_SIZE;
        vector<long long> keys;
        for(long long i=begin; i<=end; i++){
            keys.push_back(i*CHUNK_SIZE);
        }
        st.request_size_v.push_back(end-begin+1);
        st.total_request_size+=end-begin+1;
        // cout<<"keys.size="<<keys.size()<<endl;
        bool isTraceHit=true;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        // double t1 = clock();
        switch (type) {
        case 0:
            for(int i=0; i<keys.size(); i++){
                if(cache_map.Cached(keys[i])){
                    curKey=keys[i];
                    // cout<<"curKey="<<curKey<<endl;
                    readItem();
                    keys[i]=-1;
                } 
            }
            for(int i=0; i<keys.size(); i++){
                if(keys[i]!=-1){
                    isTraceHit=false;
                    curKey=keys[i];
                    // cout<<"curKey="<<curKey<<endl;
                    readItem();
                } 
            }
            break;
        case 1:
            for(int i=0; i<keys.size(); i++){
                if(cache_map.Cached(keys[i])){
                    curKey=keys[i];
                    // cout<<"curKey="<<curKey<<endl;
                    writeItem();
                    keys[i]=-1;
                } 
            }
            for(int i=0; i<keys.size(); i++){
                if(keys[i]!=-1){
                    isTraceHit=false;
                    curKey=keys[i];
                    // cout<<"curKey="<<curKey<<endl;
                    writeItem();
                } 
            }
            break;
        }
        // double t2 = clock();
        clock_gettime(CLOCK_MONOTONIC, &t2);
        unsigned long long deltaT = ((t2.tv_sec - t1.tv_sec) * powl(10, 9) + t2.tv_nsec - t1.tv_nsec); //ns
        // double deltaT = t2 - t1;
        st.latency_v.push_back(deltaT);
        st.total_latency+=deltaT;
        // cout<<"********** total_time: "<<st.total_time<<"**********"<<endl;
        // cout<<"trace "<<st.total_trace_nums<<" time: "<<deltaT<<"ns"<<endl;
        printf("trace: %llu time: %llu ns\n",st.total_trace_nums,deltaT);
        if(isTraceHit){
            st.hit_trace_nums++;
        }
        // printChunkMap();
    }
    clock_gettime(CLOCK_MONOTONIC, &t3);
    st.getEndTime();
    st.total_time = ((t3.tv_sec - t0.tv_sec) * powl(10, 9) + t3.tv_nsec - t0.tv_nsec);
}

#endif /*_ARC_SIMULATOR_H_INCLUDED_*/
