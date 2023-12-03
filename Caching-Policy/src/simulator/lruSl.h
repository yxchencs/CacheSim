#ifndef _LRU_SIMULATOR_HPP_INCLUDED_
#define _LRU_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/lru.hpp"
#include "sl.h"
using namespace std;

class LruSl : public Sl
{
public:
    LruSl();
private:
    template <typename Key, typename Value>
    using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRU>;
    lru_cache_t<long long, bool> cache_map{CACHE_SIZE};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
};

LruSl::LruSl():Sl(){
    st.caching_policy = "lru";
}

bool LruSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void LruSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.Put(key, 0);
}

ll LruSl::getVictim(){
    return cache_map.getVictim();
}
#endif /*_LRU_SIMULATOR_HPP_INCLUDED_*/
