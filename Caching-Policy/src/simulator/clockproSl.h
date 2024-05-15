#ifndef _CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#define _CLOCKPRO_SIMULATOR_HPP_INCLUDED_
#include "../cache/clockpro.hpp"
#include "sl.h"
#include "../utils/globals.h"

using namespace std;

class ClockproSl : public Sl
{
public:
    ClockproSl();

private:
    clockpro::Cache<long long, bool> cache_map{cache_size};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
    vector<ll> getVictimList(); // special for clockpro: may multi-victims at one access because of test period
    void writeCacheWhenReadItem(const ll &key, char* buffer);
    void writeCacheWhenWriteItem(const ll &key, char* buffer);
};

ClockproSl::ClockproSl() : Sl()
{
    st.caching_policy = "clockpro";
}

bool ClockproSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void ClockproSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.Set(key, 0);
}

ll ClockproSl::getVictim()
{
    return cache_map.getVictim();
}

vector<ll> ClockproSl::getVictimList()
{
    return cache_map.getVictimList();
}

void ClockproSl::writeCacheWhenReadItem(const ll &key, char* buffer)
{
    // cache not full
    if (!free_cache.empty())
    {
        // cout << "cache not full" << endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, chunk_size, buffer);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        vector<ll> victimList = getVictimList(); // [lirs] ll victim = cache_map.getCurVictim();
        for (int i = 0; i < victimList.size(); i++)
        {
            ll victim = victimList[i];
            assert(victim != -1);
            ll offset_cache = chunk_map[victim].offset_cache;
            writeBack(&chunk_map[victim]);
            chunk_map[victim].offset_cache = -1;
            if (i != victimList.size() - 1)
            {
                free_cache.push_back(offset_cache);
            }
            else // last victim
            {
                if (chunk_map.count(key) == 0)
                {
                    chunk item = {key, offset_cache};
                    chunk_map[key] = item;
                }
                else
                {
                    chunk_map[key].offset_cache = offset_cache;
                }
                writeChunk(true, offset_cache, chunk_size, buffer);
            }
        }
    }
}

void ClockproSl::writeCacheWhenWriteItem(const ll &key, char* buffer)
{
    // cache not full
    if (!free_cache.empty())
    {
        // cout << "cache not full" << endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache, 1};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, chunk_size, buffer);
    }
    // cache full
    else
    {
        // cout << "cache full" << endl;
        vector<ll> victimList = getVictimList(); // [lirs] ll victim = cache_map.getCurVictim();
        for (int i = 0; i < victimList.size(); i++)
        {
            ll victim = victimList[i];
            assert(victim != -1);
            ll offset_cache = chunk_map[victim].offset_cache;
            writeBack(&chunk_map[victim]);
            chunk_map[victim].offset_cache = -1;
            if (i != victimList.size() - 1)
            {
                free_cache.push_back(offset_cache);
            }
            else
            {
                if (chunk_map.count(key) == 0)
                {
                    chunk item = {key, offset_cache, 1};
                    chunk_map[key] = item;
                }
                else
                {
                    chunk_map[key].offset_cache = offset_cache;
                    chunk_map[key].dirty = 1;
                }
                writeChunk(true, offset_cache, chunk_size, buffer);
            }
        }
    }
}


#endif /*_CLOCKPRO_SIMULATOR_HPP_INCLUDED_*/
