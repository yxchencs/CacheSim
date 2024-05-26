#ifndef _TQ_SIMULATOR_HPP_INCLUDED_
#define _TQ_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/2q.hpp"
#include "sl.h"
using namespace std;

class TqSl : public Sl
{
public:
    TqSl();

private:
    cache_2q<ll> cache_map{cache_size};

    bool isCached(const ll &key) override;
    void accessKey(const ll &key, const bool &isGet) override;;
    ll getVictim() override;;
    // special for 2q: may replace when cache is not full because of A1out
    void writeCacheWhenReadItem(const ll &key, char* buffer) override;;
    void writeCacheWhenWriteItem(const ll &key, char* buffer) override;;
};

TqSl::TqSl() : Sl()
{
    st.caching_policy = "2q";
}

bool TqSl::isCached(const ll &key)
{
    return cache_map.cached(key);
}

void TqSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.touch(key);
}

ll TqSl::getVictim()
{
    return cache_map.getVictim();
}

// TODO::Delete
void TqSl::writeCacheWhenReadItem(const ll &key, char* buffer)
{
    ll victim = getVictim(); 
    // victim==-1 ==> no need for replace ==> free_cache give a free one 
    if (victim == -1)
    {
        // cout<<"no replace"<<endl;
        ll offset_cache = free_cache.back();
        block item = {key, offset_cache};
        block_map[key] = item;
        free_cache.pop_back();
        writeBlock(true, offset_cache, block_size, buffer);
    }
    // victim!=-1 ==> replace ==> victim provide a free one[no matter cache is full/not full]
    else
    {
        // cout<<"replace"<<endl;
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

// TODO::Delete
void TqSl::writeCacheWhenWriteItem(const ll &key, char* buffer)
{
    ll victim = getVictim(); 
    // victim==-1 ==> no need for replace ==> free_cache give a free one 
    if (victim == -1)
    {
        // cout<<"no replace"<<endl;
        ll offset_cache = free_cache.back();
        block item = {key, offset_cache, 1};
        block_map[key] = item;
        free_cache.pop_back();
        writeBlock(true, offset_cache, block_size, buffer);
    }
    // victim!=-1 ==> replace ==> victim provide a free one[no matter cache is full/not full]
    else
    {
        // cout<<"replace"<<endl;
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

#endif /*_TQ_SIMULATOR_HPP_INCLUDED_*/
