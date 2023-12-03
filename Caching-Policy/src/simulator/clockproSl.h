#ifndef _CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#define	_CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#include "../cache/clockpro.hpp"
#include "sl.h"
#include "../utils/config.h"

using namespace std;

class ClockproSl : public Sl{
private:
    clockpro::Cache<long long,bool> cache_map{CACHE_SIZE};

    void init();
    bool readItem(vector<ll>& keys);
    bool writeItem(vector<ll>& keys);
    void writeCache(const ll &key);
};

void ClockproSl::init(){
    Sl::init();
    st.caching_policy = "clockpro";
}

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
#endif /*_CLOCKPRO_SIMULATOR_HPP_INCLUDED_*/
