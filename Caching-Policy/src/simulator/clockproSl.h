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
    // ~ClockproSl();

private:
    clockpro::Cache<ll, bool> cache_map{cache_size};

    bool isCached(const ll &key) override;
    void accessKey(const ll &key, const bool &isGet) override;
    ll getVictim() override;
    vector<ll> getVictimList(); 
    vector<ll> getDeletedList();
    void deleteBlockInfo();
    // special for clockpro: may multi-victims at one access because of test period
    void writeCacheWhenReadItem(const ll &key, char *buffer) override;
    void writeCacheWhenWriteItem(const ll &key, char* buffer) override;

};

ClockproSl::ClockproSl() : Sl()
{
    st.caching_policy = "clockpro";
}

// ClockproSl::~ClockproSl() {}

bool ClockproSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void ClockproSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.Set(key, 0);
    // deleteBlockInfo();
}

ll ClockproSl::getVictim()
{
    return cache_map.getVictim();
}

vector<ll> ClockproSl::getVictimList()
{
    return cache_map.getVictimList();
}

vector<ll> ClockproSl::getDeletedList()
{
    return cache_map.getDeletedList();
}

void ClockproSl::deleteBlockInfo() 
{
    vector<ll> deletedList = getDeletedList();
    for(auto key: deletedList)
    {
        assert(block_map.find(key) != block_map.end());
        // assert(block_map[key].offset_cache == -1);
        block_map.erase(key);
    }
}


void ClockproSl::writeCacheWhenReadItem(const ll &key, char* buffer)
{
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
        vector<ll> victimList = getVictimList(); // [lirs] ll victim = cache_map.getCurVictim();
        for (int i = 0; i < victimList.size(); i++)
        {
            ll victim = victimList[i];
            assert(victim != -1);
            ll offset_cache = block_map[victim].offset_cache;
            writeBack(victim);
            block_map[victim].offset_cache = -1;
            if (i != victimList.size() - 1)
            {
                free_cache.push_back(offset_cache);
            }
            else // last victim
            {
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
    }
}

void ClockproSl::writeCacheWhenWriteItem(const ll &key, char* buffer)
{
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
        vector<ll> victimList = getVictimList(); // [lirs] ll victim = cache_map.getCurVictim();
        for (int i = 0; i < victimList.size(); i++)
        {
            ll victim = victimList[i];
            assert(victim != -1);
            ll offset_cache = block_map[victim].offset_cache;
            writeBack(victim);
            block_map[victim].offset_cache = -1;
            if (i != victimList.size() - 1)
            {
                free_cache.push_back(offset_cache);
            }
            else
            {
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
    }
}

#endif /*_CLOCKPRO_SIMULATOR_HPP_INCLUDED_*/
