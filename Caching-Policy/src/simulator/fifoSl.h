#ifndef _FIFO_SIMULATOR_HPP_INCLUDED_
#define _FIFO_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/fifo.hpp"
#include "sl.h"
using namespace std;

class FifoSl : public Sl
{
public:
    FifoSl();

private:
    template <typename Key, typename Value>
    using fifo_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::FIFO>;
    fifo_cache_t<long long, bool> cache_map{CACHE_SIZE};

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
};

FifoSl::FifoSl() : Sl()
{
    st.caching_policy = "fifo";
}

bool FifoSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void FifoSl::accessKey(const ll &key, const bool &isGet)
{
    return cache_map.Put(key, 0);
}

ll FifoSl::getVictim()
{
    return cache_map.getVictim();
}

#endif /*_FIFO_SIMULATOR_HPP_INCLUDED_*/
