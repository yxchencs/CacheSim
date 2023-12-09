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
    cache_2q<ll> cache_map{CACHE_SIZE};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
    void writeCache(const ll &key); // special for 2q: may replace when cache is not full because of A1out

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

void TqSl::writeCache(const ll &key)
{
    cout<<"writeCache: ";
    if (!isWriteCache())
        return;


    if(!free_cache.empty()){
        cout<<"cache not full, ";
    } else {
        cout<<"cache full, ";
    }

    ll victim = getVictim(); 
    // victim==-1 ==> no need for replace ==> free_cache give a free one 
    if (victim == -1)
    {
        cout<<"no replace"<<endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, CHUNK_SIZE);
    }
    // victim!=-1 ==> replace ==> victim provide a free one[no matter cache is full/not full]
    else
    {
        cout<<"replace"<<endl;
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

#endif /*_TQ_SIMULATOR_HPP_INCLUDED_*/
