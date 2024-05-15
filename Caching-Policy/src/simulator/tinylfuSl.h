#ifndef _TINYLFU_SIMULATOR_HPP_INCLUDED_
#define _TINYLFU_SIMULATOR_HPP_INCLUDED_
#include "../cache/tinylfu/tinylfu.hpp"
#include "../cache/tinylfu/bloom_filter.hpp"
#include "sl.h"
using namespace std;

// 值加载器函数
std::shared_ptr<std::string> value_loader(const int& key) {
    // 基于整数键创建一个新的字符串值
    auto new_value = std::make_shared<std::string>("Value for key " + std::to_string(key));
    return new_value;
}

class TinylfuSl : public Sl
{
public:
    TinylfuSl();

private:
    wtinylfu_cache<ll,bool> cache_map{cache_size};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
    // special for 2q: may replace when cache is not full because of A1out
    void writeCacheWhenReadItem(const ll &key, char* buffer);
    void writeCacheWhenWriteItem(const ll &key, char* buffer);

};

TinylfuSl::TinylfuSl() : Sl()
{
    st.caching_policy = "tinylfu";
}

bool TinylfuSl::isCached(const ll &key)
{
    return cache_map.contains(key);
}

void TinylfuSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.get_and_insert_if_missing(key,value_loader);
}

ll TinylfuSl::getVictim()
{
    return cache_map.getVictim();
}

void TinylfuSl::writeCacheWhenReadItem(const ll &key, char* buffer)
{
    ll victim = getVictim(); 
    // victim==-1 ==> no need for replace ==> free_cache give a free one 
    if (victim == -1)
    {
        // cout<<"no replace"<<endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, chunk_size, buffer);
    }
    // victim!=-1 ==> replace ==> victim provide a free one[no matter cache is full/not full]
    else
    {
        // cout<<"replace"<<endl;
        ll offset_cache = chunk_map[victim].offset_cache;
        writeBack(&chunk_map[victim]);
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
        writeChunk(true, offset_cache, chunk_size, buffer);
    }
}

void TinylfuSl::writeCacheWhenWriteItem(const ll &key, char* buffer)
{
    ll victim = getVictim(); 
    // victim==-1 ==> no need for replace ==> free_cache give a free one 
    if (victim == -1)
    {
        // cout<<"no replace"<<endl;
        ll offset_cache = free_cache.back();
        chunk item = {key, offset_cache, 1};
        chunk_map[key] = item;
        free_cache.pop_back();
        writeChunk(true, offset_cache, chunk_size, buffer);
    }
    // victim!=-1 ==> replace ==> victim provide a free one[no matter cache is full/not full]
    else
    {
        // cout<<"replace"<<endl;
        ll offset_cache = chunk_map[victim].offset_cache;
        writeBack(&chunk_map[victim]);
        chunk_map[victim].offset_cache = -1;
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

#endif /*_TINYLFU_SIMULATOR_HPP_INCLUDED_*/
