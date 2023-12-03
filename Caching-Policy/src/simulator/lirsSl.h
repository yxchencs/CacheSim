#ifndef _LIRS_SIMULATOR_H_INCLUDED_
#define _LIRS_SIMULATOR_H_INCLUDED_
#include "../cache/lirs.h"
#include "sl.h"
using namespace std;

class LirsSl : public Sl
{
public:
    LirsSl();

private:
    LIRS cache_map;

    bool isCached(const ll &key);
    void accessKey(const ll &key, const bool &isGet);
    ll getVictim();
};

LirsSl::LirsSl : Sl()
{
    st.caching_policy = "lirs";
}

bool LirsSl::isCached(const ll &key)
{
    auto v = cache_map.Peek(key);
    return IS_VALID(v);
}

void LirsSl::accessKey(const ll &key, const bool &isGet)
{
    if (isGet)
    {
        cache_map.Get(key);
    }
    else
    {
        cache_map.Add(key, 0);
    }
}

ll LirsSl::getVictim()
{
    return cache_map.getCurVictim();
}

#endif /*_LIRS_SIMULATOR_H_INCLUDED_*/
