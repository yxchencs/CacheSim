#ifndef _ARC_SIMULATOR_H_INCLUDED_
#define _ARC_SIMULATOR_H_INCLUDED_
#include "../cache/arc.h"
#include "sl.h"
using namespace std;

class ArcSl : public Sl
{
public:
    ArcSl();

private:
    ARC cache_map;

    bool isCached(const ll &key) override;;
    void accessKey(const ll &key, const bool &isGet) override;;
    ll getVictim() override;;
};

ArcSl::ArcSl() : Sl()
{
    st.caching_policy = "arc";
}

bool ArcSl::isCached(const ll &key)
{
    return cache_map.Cached(key);
}

void ArcSl::accessKey(const ll &key, const bool &isGet)
{
    cache_map.arc_lookup(key);
}

ll ArcSl::getVictim()
{
    return cache_map.getVictim();
}

#endif /*_ARC_SIMULATOR_H_INCLUDED_*/
