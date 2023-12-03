#ifndef _RANDOM_SIMULATOR_HPP_INCLUDED_
#define _RANDOM_SIMULATOR_HPP_INCLUDED_
#include "../cache/cache.hpp"
#include "../cache/random.h"
#include "sl.h"
using namespace std;

class RandomSl : public Sl
{
private:
    RANDOM cache_map;
    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
};

// RandomSl::RandomSl(){
//     Sl::Sl();
//     st.caching_policy = "random";
// }

// void RandomSl::init(){
//     Sl::init();
//     st.caching_policy = "fifo";
// }

bool RandomSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void RandomSl::accessKey(const ll &key, const bool &isGet)
{
    return cache_map.Touch(key);
}

ll RandomSl::getVictim(){
    return cache_map.getVictim();
}

#endif /*_RANDOM_SIMULATOR_HPP_INCLUDED_*/
