#ifndef STATISTIC_HPP_INCLUDED_
#define STATISTIC_HPP_INCLUDED_

#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <sstream> // for stringstream
#include <iomanip> // for setprecision()
#include "globals.h"
#include "run.h"

using namespace std;

void mkdir(std::string path);
std::string getSubstringAfter(const std::string &original, const std::string &to_find);

struct Latency
{
    vector<ll> latency_v;  // us
    ll total_latency;      // us
    float average_latency; // ms
    float p95_latency;     // ms
    float p99_latency;     // ms

    Latency() : total_latency(0), average_latency(0), p95_latency(0), p99_latency(0) {}
    void addDeltaT(ll deltaT)
    {
        latency_v.push_back(deltaT);
        total_latency += deltaT;
    }
    bool checkLatencySize() {
        if(!latency_v.size()) {
            return false;
        }
        return true;
    }
    void compute()
    {
        if(!checkLatencySize()){
            return;
        }

        ll latency_v_size = latency_v.size();
        sort(latency_v.begin(), latency_v.end());
        p95_latency = latency_v[ceil(0.95 * latency_v_size)] * 1.0 / 1e3;
        p99_latency = latency_v[ceil(0.99 * latency_v_size)] * 1.0 / 1e3;

        if (latency_v_size != 0)
        {
            average_latency = total_latency * 0.1 / latency_v_size / 1e3;
        }
    }
    void save(string save_path)
    {
        if(!checkLatencySize()){
            return;
        }

        ofstream fout(save_path);

        if (fout.is_open())
        {
            fout << "traceNo latency(us)" << endl;
            for (int i = 0; i < latency_v.size(); i++)
            {
                // cout<<i+1<<' '<<latency_v[i]<<endl;
                fout << i + 1 << ' ' << latency_v[i] << endl;
            }
            fout.close();
            // cout<<"save trace latency success"<<endl;
        }
        else
        {
            cerr << "error: can not open result file: " << save_path << endl;
        }
    }

    void print_latency_v()
    {
        cout << "latency(ns): " << endl;
        for (auto t : latency_v)
        {
            cout << t << ' ';
        }
        cout << endl;
    }

    int size()
    {
        return latency_v.size();
    }
};

class Statistic
{
public:
    Statistic();

    ll computeDeltaT(struct timeval begin, struct timeval end);
    void record();
    void printStatistic();
    void writeStatistic();
    void getCurrentTimeFormatted(char *formattedTime);
    void getStartTime();
    void getEndTime();
    void computeLatency();
    void saveLatency();
    void resetSaveDir(const string &dir);
    void makeDefaultSaveDir();

    string caching_policy;

    char startTime[20], endTime[20];

    ll read_hit_nums;
    ll read_nums;

    ll write_hit_nums;
    ll write_nums;

    ll hit_trace_nums;
    ll total_trace_nums;

    ll total_time;

    struct Latency total_latency;
    struct Latency cache_read_latency;
    struct Latency cache_write_latency;
    struct Latency disk_read_latency;
    struct Latency disk_write_latency;

    vector<ll> request_size_v;
    ll total_request_size;

    string save_dir;
};

Statistic::Statistic()
{
    read_hit_nums = 0;
    read_nums = 0;

    write_hit_nums = 0;
    write_nums = 0;

    hit_trace_nums = 0;
    total_trace_nums = 0;

    total_time = 0;
    total_request_size = 0;
}

ll Statistic::computeDeltaT(struct timeval begin, struct timeval end)
{
    return (end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec);
}

void Statistic::makeDefaultSaveDir()
{
    save_dir = save_root + getSubstringAfter(trace_dir, "trace/");

    stringstream ss;
    ss << setprecision(2) << cache_size_factor;
    string str_cache_size_factor = ss.str();
    if (io_on)
    {
        save_dir = save_dir + "/io_on/" + str_cache_size_factor + '/' + caching_policy + '/';
    }
    else
    {
        save_dir = save_dir + "/io_off/" + str_cache_size_factor + '/' + caching_policy + '/';
    }

    mkdir(save_dir);
    cout << "save_dir: " << save_dir << endl;
}

void Statistic::getCurrentTimeFormatted(char *formattedTime)
{
    time_t timep;
    struct tm *p;

    time(&timep);
    p = localtime(&timep);

    snprintf(formattedTime, 20, "%d/%02d/%02d %02d:%02d:%02d",
             1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
             p->tm_hour, p->tm_min, p->tm_sec);
}

void Statistic::getStartTime()
{
    getCurrentTimeFormatted(startTime);
}

void Statistic::getEndTime()
{
    getCurrentTimeFormatted(endTime);
}

void Statistic::writeStatistic()
{
    ofstream fout(save_dir + "statistic.txt");

    if (!fout.is_open())
    {
        cerr << "error: can not open result file:" << save_dir + "statistic.txt" << endl;
        return;
    }

    fout << "caching policy: " << caching_policy << endl;
    fout << "trace: " << trace_path << endl;
    fout << "cache: " << cache_path << endl;
    fout << "O_DIRECT: " << O_DIRECT_ON << endl;
    fout << "chunk size: " << CHUNK_SIZE << " B" << endl;
    fout << "disk size: " << disk_size << " x " << CHUNK_SIZE << " B" << endl;
    fout << "cache size: " << cache_size << " x " << CHUNK_SIZE << " B" << endl;
    fout << "chunk number: " << chunk_num << " x " << CHUNK_SIZE << " B" << endl;
    if (read_nums != 0)
    {
        fout << "read hit/total number: " << read_hit_nums << '/' << read_nums << endl;
        fout << "read hit ratio: " << read_hit_nums * 1.0 / read_nums << endl;
    }
    if (write_nums != 0)
    {
        fout << "write hit/total number: " << write_hit_nums << '/' << write_nums << endl;
        fout << "write hit ratio: " << write_hit_nums * 1.0 / write_nums << endl;
    }
    if ((read_nums + write_nums) != 0)
    {
        fout << "hit/total number: " << read_hit_nums + write_hit_nums << '/' << read_nums + write_nums << endl;
        fout << "hit ratio: " << (read_hit_nums + write_hit_nums) * 1.0 / (read_nums + write_nums) << endl;
    }
    if (total_trace_nums != 0)
    {
        fout << "hit/total trace: " << hit_trace_nums << '/' << total_trace_nums << endl;
        fout << "trace hit ratio: " << hit_trace_nums * 1.0 / total_trace_nums << endl;
    }

    fout << "From " << startTime << " to " << endTime << endl;
    fout << "total time: " << total_time * 1.0 / 1e6 << " s" << endl;
    if (total_trace_nums != 0)
    {
        fout << "average latency: " << total_latency.average_latency << " ms" << endl;
    }

    fout << "tail latency: P95 = " << total_latency.p95_latency << " ms, P99 = " << total_latency.p99_latency << " ms" << endl;

    // fout<<"total request size: "<<total_request_size<<" x "<<CHUNK_SIZE<<"B"<<endl;  // number of chunks
    fout << "total request size: " << total_request_size * 1.0 * CHUNK_SIZE / 1024 / 1024 << " MB" << endl;
    if (total_trace_nums != 0)
    {
        fout << "average size: " << total_request_size * 1.0 * CHUNK_SIZE / 1024 / total_trace_nums << " KB" << endl;
    }
    if (total_time != 0)
    {
        fout << "bandwidth: " << total_request_size * 1.0 * CHUNK_SIZE / 1024 / 1024 / (total_time * 1.0 / 1e6) << " MB/s" << endl; // fout<<"bandwidth: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024 / (total_time*1.0/1e9)<<" MB/s"<<endl;
    }
    fout << "io_on: " << io_on << endl;;

    fout << "emmc_read: nums: " << cache_read_latency.size() << "; average latency: " << cache_read_latency.average_latency << " ms; tail latency: P95 = " << cache_read_latency.p95_latency << " ms, P99 = " << cache_read_latency.p99_latency << " ms" << endl;
    fout << "emmc_write: nums: " << cache_write_latency.size() << "; average latency: " << cache_write_latency.average_latency << " ms; tail latency: P95 = " << cache_write_latency.p95_latency << " ms, P99 = " << cache_write_latency.p99_latency << " ms" << endl;
    fout << "sd_read: nums: " << disk_read_latency.size() << "; average latency: " << disk_read_latency.average_latency << " ms; tail latency: P95 = " << disk_read_latency.p95_latency << " ms, P99 = " << disk_read_latency.p99_latency << " ms" << endl;
    fout << "sd_write: nums: " << disk_write_latency.size() << "; average latency: " << disk_write_latency.average_latency << " ms; tail latency: P95 = " << disk_write_latency.p95_latency << " ms, P99 = " << disk_write_latency.p99_latency << " ms" << endl;

    fout.close();
    printf("statistic saved\n");
}

void Statistic::record()
{
    cout << "From " << startTime << " to " << endTime << endl;
    saveLatency();
    computeLatency();
    writeStatistic();
}

void Statistic::computeLatency()
{
    total_latency.compute();
    cache_read_latency.compute();
    cache_write_latency.compute();
    disk_read_latency.compute();
    disk_write_latency.compute();
}

void Statistic::saveLatency()
{
    total_latency.save(save_dir + "trace_latency.txt");
    cache_read_latency.save(save_dir + "cache_read_latency.txt");
    cache_write_latency.save(save_dir + "cache_write_latency.txt");
    disk_read_latency.save(save_dir + "disk_read_latency.txt");
    disk_write_latency.save(save_dir + "disk_write_latency.txt");
    printf("latency_v saved\n");
}

void Statistic::resetSaveDir(const string &dir) {
    save_dir = dir;
    mkdir(save_dir);
    cout << "save_dir: " << save_dir << endl;
}

#endif /*STATISTIC_HPP_INCLUDED_*/
