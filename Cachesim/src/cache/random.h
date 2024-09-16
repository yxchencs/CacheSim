#ifndef RANDOM_H
#define RANDOM_H


#include <list>
#include <map>
#include <random>
#include "../utils/globals.h"

using namespace std;

class RANDOM {
public:
    
    RANDOM(ll size=cache_size):cache_size_(size){}
    ~RANDOM() {
        cache_map.clear();
    }

    bool Cached(const ll &key)
    {
        return cache_map.count(key)!=0;
    }

    bool Get(const ll &key){
        return cache_map.at(key);
    }

    void Put(const ll &key)
    {
        cache_map[key]=0;
    }

    ll getVictim(){
        return victim;
    }

    void Touch(const ll &key)
    {
        ll curVictim = -1;
        if(cache_map.size() >= cache_size_){
            curVictim = ReplCandidate();
            Erase(curVictim);
            victim = curVictim;
        }

        Put(key);
        // return victim; 
    }

    void Erase(const ll &key) 
    {
        auto it = cache_map.find(key);
        if (it != cache_map.end())
            cache_map.erase(it); // map.erase(e->key);
    }


    const ll &ReplCandidate()
    {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, cache_map.size() - 1);

        auto it = cache_map.begin();
        advance(it, dis(gen));
        return it->first;
    }

private:
    ll cache_size_; 
    map<ll, bool> cache_map;
    ll victim;
};

#endif // RANDOM_H
