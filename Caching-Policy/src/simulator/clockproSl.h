#ifndef _CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#define	_CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#include "../cache/clockpro.hpp"
#include "sl.h"
#include "../utils/config.h"

using namespace std;

class ClockproSl : public Sl{
public:
    void test(); // 1,4097 ==> 0,4096 + 4096,4096  
private:
    clockpro::Cache<long long,bool> cache_map{CACHE_SIZE};

    bool readItem(vector<ll>& keys);
    bool writeItem(vector<ll>& keys);
    void writeCache(const ll &key);
};

bool ClockproSl::readItem(vector<ll>& keys){
    bool isTraceHit = true;
    st.read_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if(keys[i]==81920) cout<<"cache_map.Cached(keys[i]): "<<cache_map.Cached(keys[i])<<endl;

        if (cache_map.Cached(keys[i])){
            cout<<'['<<keys[i]<<']'<<endl;
            st.read_hit_nums += 1;
            // cache_map.Get(keys[i]);
            cache_map.Set(keys[i],0);
            readCache(chunk_map[keys[i]].offset_cache);
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            cout<<'['<<keys[i]<<']'<<endl;
            isTraceHit = false;
            cache_map.Set(keys[i],0);
            readDisk(keys[i]);
            writeCache(keys[i]);  
        }
    }
    return isTraceHit;
}

bool ClockproSl::writeItem(vector<ll>& keys){
    bool isTraceHit = true;
    st.write_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (cache_map.Cached(keys[i]))
        {
            st.write_hit_nums += 1;
            cache_map.Set(keys[i],0);
            coverageCache(&chunk_map[keys[i]]);
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            isTraceHit = false;
            cache_map.Set(keys[i],0);
            writeCache(keys[i]);
        }
    }
    return isTraceHit;
}

void ClockproSl::writeCache(const ll &key){
    printf("writeCache\n");
    if (!isWriteCache()) return;  

    // cache not full
    if (!free_cache.empty())
    {
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, CHUNK_SIZE);
    }
    // cache full
    else { 
        ll victim = cache_map.getCurVictim();
        assert(victim!=-1);
        ll offset_cache = chunk_map[victim].offset_cache;
        chunk_map[victim].offset_cache = -1;
        if(chunk_map.count(key) == 0){
            chunk item = {key, offset_cache};
            chunk_map[key] = item;
        } else {
            chunk_map[key].offset_cache=offset_cache;
        }
        writeChunk(true, offset_cache, CHUNK_SIZE);
        writeBack(&chunk_map[victim]);
    }
}

void ClockproSl::test() {
    st.caching_policy="lirs";
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
#endif /*_CLOCKPRO_SIMULATOR_HPP_INCLUDED_*/
