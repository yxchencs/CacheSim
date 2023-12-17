#ifndef __ARC_H__
#define __ARC_H__
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include "../utils/config.h"
#define HASHSIZE 100000
using namespace std;

class ARC
{
public:
    ARC(long long cache_size = CACHE_SIZE) : c(cache_size), cacheSize(cache_size) {}

    // A function to check whether Page x is available in 'v' queue
    int check(vector<long long> v, long long x);

    // A function to insert page 'i' in 'v' queue.
    void queue_insert(vector<long long> &v, long long i);

    // function to pop LRU element from queue 'v'
    void queue_delete(vector<long long> &v);

    // function to move a particular page from one queue to another, 'x' from queue 'v' to queue 'w'
    void movefrom(vector<long long> &v, vector<long long> &w, long long x);

    /*
    Replace subroutine as specified in the reference paper
    This function is called when a page with given 'pageNumber' is to be moved from
    T2 to B2 or T1 to B1. Basically this function is used to move the elements out from
    one list and add it to the another list beginning.
    Finally return victim key.
    */
    void Replace(const long long i, const float p);

    // function to look object through given key.
    // if have to replace, return victim key.
    void arc_lookup(long long i);

    bool Cached(long long i);

    void printV();

    ll getVictim();

private:
    // creating a hash file through array
    long long Hash[HASHSIZE];
    // map<long long, long long> chunk_map;

    // we use vector(dynamic array) data structures to represent queues.
    /*
    A ARC Cache consisting of 4 Queues
        mrug (B1)- Most Recently Used Ghost
        mru (T1) - Most Recently Used
        mfu (T2) - Most Frequently Used
        mfug (B2) - Most Frequently Used Ghost
    */
    vector<long long> mrug, mru, mfu, mfug;
    float p = 0.0;
    long long cacheSize;
    long long c;
    ll victim;
};

// A function to check whether Page x is available in 'v' queue
int ARC::check(vector<long long> v, long long x)
{
    unsigned int l = v.size(), i;
    for (i = 0; i < l; i++)
    {
        if (v[i] == x)
            return 1;
    }
    return 0;
}

// A function to insert page 'i' in 'v' queue.
void ARC::queue_insert(vector<long long> &v, long long i)
{
    if (v.size() == cacheSize)
        v.erase(v.begin());
    v.push_back(i);
}

// function to pop LRU element from queue 'v'
void ARC::queue_delete(vector<long long> &v)
{
    if (v.size() > 0)
        v.erase(v.begin());
}

// function to move a particular page from one queue to another, 'x' from queue 'v' to queue 'w'
void ARC::movefrom(vector<long long> &v, vector<long long> &w, long long x)
{
    int i, j, l = v.size();
    for (i = 0; i < l; i++)
        if (v[i] == x)
        {
            v.erase(v.begin() + i);
            break;
        }

    if (w.size() == cacheSize)
        w.erase(w.begin());
    w.push_back(x);
}

/*
Replace subroutine as specified in the reference paper
This function is called when a page with given 'pageNumber' is to be moved from
T2 to B2 or T1 to B1. Basically this function is used to move the elements out from
one list and add it to the another list beginning.
Finally return victim key.
*/
void ARC::Replace(const long long i, const float p)
{
    // cout << "Replace" << endl;
    long long curVictim = -1;
    if ((mru.size() >= 1) && ((mru.size() > p) || (check(mfug, i)) && (p == mru.size())))
    {
        // mru full
        if (mru.size() > 0)
        {
            curVictim = mru[0];
            movefrom(mru, mrug, mru[0]);
        }
    }
    else
    {
        if (mfu.size() > 0)
        {
            curVictim = mfu[0];
            movefrom(mfu, mfug, mfu[0]);
        }
    }
    victim = curVictim;
}

// function to look object through given key.
// if have to replace, return victim key.
void ARC::arc_lookup(long long i)
{
    // cout << "===access " << i << "===" << endl;
    // if (chunk_map.count(i)!=0)
    if(Hash[i % HASHSIZE])
    {
        // Case 1: Part A: Page found in MRU
        if (check(mru, i))
        {
            // cout<<"Case 1: Part A"<<endl;
            // HitCount++;
            movefrom(mru, mfu, i);
        }
        // Case 1: Part B: Page found in MFU
        else if (check(mfu, i))
        {
            // cout<<"Case 1: Part B"<<endl;
            // HitCount++;
            movefrom(mfu, mfu, i);
        }
        // Case 2: Page found in MRUG
        else if (check(mrug, i))
        {
            // cout<<"Case 2"<<endl;
            // MissCount++;
            p = (float)min((float)c, (float)(p + max((mfug.size() * 1.0) / mrug.size(), 1.0)));
            Replace(i, p);
            movefrom(mrug, mfu, i);
        }
        // Case 3: Page found in MFUG
        else if (check(mfug, i))
        {
            // cout<<"Case 3"<<endl;
            // MissCount++;
            p = (float)max((float)0.0, (float)(p - max((mrug.size() * 1.0) / mfug.size(), 1.0)));
            Replace(i, p);
            movefrom(mfug, mfu, i);
        }
        // Case 4: Page not found in any of the queues.
        else
        {
            // MissCount++;
            // Case 4: Part A: When L1 has c pages / L1 full
            if ((mru.size() + mrug.size()) == c)
            {
                // Case 4: Part A: Part a: mrug not empty ==> delete mrug
                if (mru.size() < c)
                {
                    // cout<<"Case 4: Part A: Part a"<<endl;
                    Hash[ mrug[0]% HASHSIZE]--;
                    // chunk_map[mrug[0]]--;

                    queue_delete(mrug);
                    Replace(i, p);
                }
                // Case 4: Part A: Part b: mrug empty     ===> delete mru
                else
                {
                    // cout<<"Case 4: Part A: Part b"<<endl;
                    Hash[ mru[0]% HASHSIZE]--;
                    // chunk_map[mru[0]]--;
                    victim = mru[0];
                    queue_delete(mru);
                }
            }
            // Case 4: Part B: L1 has less than c pages
            else if ((mru.size() + mrug.size()) < c)
            {
                // cout<<"Case 4: Part B"<<endl;
                if ((mru.size() + mfu.size() + mrug.size() + mfug.size()) >= c)
                {
                    if ((mru.size() + mfu.size() + mrug.size() + mfug.size()) == (2 * c))
                    {
                        Hash[ mfug[0] % HASHSIZE]--;
                        // chunk_map[mrug[0]]--;

                        queue_delete(mfug);
                    }
                    Replace(i, p);
                }
            }
            // Move the page to the most recently used position
            queue_insert(mru, i);
            Hash[i % HASHSIZE]++;
            // chunk_map[i]++;
        }
    }

    else
    {
        // Page not found, increase miss count
        //  MissCount++;

        // Case 4: Part A: L1 has c pages / L1 full
        if ((mru.size() + mrug.size()) == c)
        {
            // cout<<"Case 5: Part A"<<endl;
            if (mru.size() < c)
            {
                Hash[mrug[0]%HASHSIZE]--;
                // chunk_map[mrug[0]]--;

                queue_delete(mrug);
                Replace(i, p);
            }

            else
            {
                Hash[mru[0]%HASHSIZE]--;
                // chunk_map[mru[0]]--;
                victim = mru[0];
                queue_delete(mru);
            }
        }

        // Case 4: Part B: L1 less than c pages
        else if ((mru.size() + mrug.size()) < c)
        {
            // cout<<"Case 5: Part B"<<endl;
            if ((mru.size() + mfu.size() + mrug.size() + mfug.size()) >= c)
            {
                if ((mru.size() + mfu.size() + mrug.size() + mfug.size()) == 2 * c)
                {
                    Hash[mfug[0]%HASHSIZE]--;
                    // chunk_map[mfug[0]]--;

                    queue_delete(mfug);
                }
                Replace(i, p);
            }
        }

        // Move the page to the most recently used position
        queue_insert(mru, i);
        Hash[i % HASHSIZE]++;
        // chunk_map[i]++;
    }

    // cout << "victim=" << victim << endl;
    // printV();

}

// A function to check whether key i is cached
bool ARC::Cached(long long i)
{
    // if (chunk_map.count(i)!=0)
    if(Hash[i % HASHSIZE])
    {
        // Page found in MRU & MFU
        if (check(mru, i) || check(mfu, i))
        {
            return true;
        }
    }

    return false;
}

void ARC::printV()
{
    unsigned int l, i;

    cout << "mrug: ";
    l = mrug.size();
    for (i = 0; i < l; i++)
    {
        cout << mrug[i] << ' ';
    }
    cout << endl;

    cout << "mru: ";
    l = mru.size();
    for (i = 0; i < l; i++)
    {
        cout << mru[i] << ' ';
    }
    cout << endl;

    cout << "mfu: ";
    l = mfu.size();
    for (i = 0; i < l; i++)
    {
        cout << mfu[i] << ' ';
    }
    cout << endl;

    cout << "mfug: ";
    l = mfug.size();
    for (i = 0; i < l; i++)
    {
        cout << mfug[i] << ' ';
    }
    cout << endl;
}

ll ARC::getVictim(){
    return victim;
}


#endif // __ARC_H__