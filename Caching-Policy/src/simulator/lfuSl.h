#ifndef _LFU_SIMULATOR_HPP_INCLUDED_
#define	_LFU_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/lfu.hpp"
#include "sl.h"
using namespace std;

class LfuSl : public Sl{
public:
    LfuSl();
private:
    template <typename Key, typename Value>
    using lfu_cache_t = typename caches::fixed_sized_cache<Key, Value,caches::LFU>;
    lfu_cache_t<long long, bool> cache_map{cache_size};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
};

LfuSl::LfuSl():Sl(){
    st.caching_policy = "lfu";
}

bool LfuSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void LfuSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.Put(key, 0);
}

ll LfuSl::getVictim(){
    return cache_map.getVictim();
}

#endif /*_LFU_SIMULATOR_HPP_INCLUDED_*/
