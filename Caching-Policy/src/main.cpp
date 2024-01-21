#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include "simulator/randomSl.h"
#include "simulator/fifoSl.h"
#include "simulator/lruSl.h"
#include "simulator/lfuSl.h"
#include "simulator/lirsSl.h"
#include "simulator/arcSl.h"
#include "simulator/clockproSl.h"
#include "simulator/2qSl.h"
#include "simulator/tinylfuSl.h"

using namespace std;

void run(int i, int j){
    Sl *sim = nullptr;
    cache_size_index = i, caching_policy_index = j;
    cache_size_factor = cacheSizeTypes[cache_size_index];
    cache_size = CHUNK_NUM*cache_size_factor;
    cache_path = CACHE_PATH_HEAD+cachePath[cache_size_index];
    switch(policyTypes[caching_policy_index]){
        case PolicyType::RANDOM: 
            sim = new RandomSl();
            break;
        case PolicyType::FIFO:
            sim = new FifoSl();
            break;
        case PolicyType::LFU:
            sim = new LfuSl();
            break;
        case PolicyType::LRU:
            sim = new LruSl();
            break;
        case PolicyType::LIRS:
            sim = new LirsSl();
            break;
        case PolicyType::ARC:
            sim = new ArcSl();
            break;
        case PolicyType::CLOCKPRO:
            sim = new ClockproSl();
            break;
        case PolicyType::TQ:
            sim = new TqSl();
            break;
        case PolicyType::TINYLFU:
            sim = new TinylfuSl();
            break;
    }

    sim->test();
    sim->statistic();
}

int main(){
    cout<<"cache_size_types_size: "<<cache_size_types_size<<", policy_types_size: "<<policy_types_size<<endl;
    io_on = false;
    for(int i=0;i<cache_size_types_size;i++){
        for(int j=0;j<policy_types_size;j++){
            run(i,j);
        }
    }
    io_on = true;
    for(int i=0;i<cache_size_types_size;i++){
        for(int j=0;j<policy_types_size;j++){
            run(i,j);
        }
    }
    return 0;
}
/*
sudo mount /dev/mmcblk0p1 /mnt/eMMC
lsblk
g++ -std=c++17 -o main main.cpp
./main
*/
