#ifndef _ARC_SIMULATOR_H_INCLUDED_
#define _ARC_SIMULATOR_H_INCLUDED_
#include "../cache/arc.h"
#include "sl.h"
using namespace std;

class ArcSl : public Sl
{
private:
    ARC cache_map;

    void init();
    bool readItem(vector<ll> &keys);
    bool writeItem(vector<ll> &keys);
    void writeCache(const ll &key);
};

void ArcSl::init()
{
    Sl::init();
    st.caching_policy = "arc";
}

bool ArcSl::readItem(vector<ll> &keys)
{
    bool isTraceHit = true;
    st.read_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (cache_map.Cached(keys[i]))
        {
            st.read_hit_nums += 1;
            cache_map.arc_lookup(keys[i]);
            readCache(chunk_map[keys[i]].offset_cache);
            keys[i] = -1;
        }
    }
    // cache miss
    for (int i = 0; i < keys.size(); i++)
    {
        if (keys[i] != -1)
        {
            isTraceHit = false;
            readDisk(keys[i]);
            writeCache(keys[i]);
        }
    }
    return isTraceHit;
}

bool ArcSl::writeItem(vector<ll> &keys)
{
    bool isTraceHit = true;
    st.write_nums += keys.size();
    // cache hit
    for (int i = 0; i < keys.size(); i++)
    {
        if (cache_map.Cached(keys[i]))
        {
            st.write_hit_nums += 1;
            cache_map.arc_lookup(keys[i]);
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
            writeCache(keys[i]);
        }
    }
    return isTraceHit;
}

void ArcSl::writeCache(const ll &key)
{
    if (!isWriteCache())
        return;

    // cache not full
    if (!free_cache.empty())
    {
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        cache_map.arc_lookup(key);
        writeChunk(true, offset_cache, CHUNK_SIZE);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        ll victim = cache_map.arc_lookup(key);
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
        writeChunk(true, offset_cache, CHUNK_SIZE);
        writeBack(&chunk_map[victim]);
    }
}
#endif /*_ARC_SIMULATOR_H_INCLUDED_*/
