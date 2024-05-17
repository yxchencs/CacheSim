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
    // vector<ll> getVictimList(); 
    vector<ll> getDeletedList();
    void deleteBlockInfo();
    // special for clockpro: may multi-victims at one access because of test period
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

// vector<ll> ClockproSl::getVictimList()
// {
//     return cache_map.getVictimList();
// }

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


#endif /*_CLOCKPRO_SIMULATOR_HPP_INCLUDED_*/
