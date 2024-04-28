#ifndef _NO_CACHE_SIMULATOR_HPP_INCLUDED_
#define _NO_CACHE_SIMULATOR_HPP_INCLUDED_

#include "../utils/statistic.h"
#include "../utils/globals.h"
#include "../utils/run.h"
#include "../utils/progress_bar.h"

using namespace std;

void checkFile(fstream &file);
string getSubstringAfter(const string &original, const string &to_find);

class NoCacheSl
{
public:
    void test();
    void statistic();

    NoCacheSl(ll chunk_size_KB, string device_id, string device_path)
        : chunk_size_KB(chunk_size_KB), device_id(device_id), chunk_size(chunk_size_KB * 1024)
    {
        fd = open(device_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd >= 0);

        int res = posix_memalign((void **)&buffer_read, chunk_size, chunk_size);
        assert(res == 0);

        res = posix_memalign((void **)&buffer_write, chunk_size, chunk_size);
        assert(res == 0);
        memset(buffer_write, 0, chunk_size);
    };
    ~NoCacheSl()
    {
        close(fd);
        free(buffer_read);
        free(buffer_write);
    };

private:
    int fd;

    char *buffer_read = nullptr;
    char *buffer_write = nullptr;

    Statistic st;
    string device_id;
    ll chunk_size_KB;
    ll chunk_size;

    void readItem(vector<ll> &keys);
    void writeItem(vector<ll> &keys);
    void readChunk(const long long &offset, const long long &size);
    void writeChunk(const long long &offset, const long long &size);
};

void NoCacheSl::test()
{
    // cout << "-----------------------------------------------------------------" << endl;
    printf("test start\n");
    st.getStartTime();

    fstream fin_trace(trace_path);
    checkFile(fin_trace);

    ll curKey, curSize;
    int type;
    char c;
    string s;
    getline(fin_trace, s);

    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0, NULL);
    while (fin_trace >> curKey >> c >> type)
    {
        st.total_trace_nums++;
        show_progress_bar(st.total_trace_nums, 6000000);

        ll begin = curKey;
        ll end = (curKey - 1);
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_size += end - begin + 1;

        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * chunk_size);
        }

        gettimeofday(&t1, NULL);

        switch (type)
        {
        case 0:
            readItem(keys);
            break;
        case 1:
            writeItem(keys);
            break;
        }

        gettimeofday(&t2, NULL);
        st.total_latency.addDeltaT(st.computeDeltaT(t1,t2));
        // printf("trace: %llu, time: %lldus, total_time: %lld\n", st.total_trace_nums, deltaT, st.total_latency);
        // printChunkMap();
        // cout<<"isTraceHit: "<<isTraceHit<<' '<<"st.hit_trace_nums: "<<st.hit_trace_nums<<endl;
    }
    gettimeofday(&t3, NULL);
    st.total_time = st.computeDeltaT(t0,t3);
    st.getEndTime();
    printf("test end\n");
}

void NoCacheSl::readItem(vector<ll> &keys)
{
    st.read_nums += keys.size();
    for (int i = 0; i < keys.size(); i++)
    {
        readChunk(keys[i], chunk_size);
    }
}

void NoCacheSl::writeItem(vector<ll> &keys)
{
    st.write_nums += keys.size();
    for (int i = 0; i < keys.size(); i++)
    {
        writeChunk(keys[i], chunk_size);
    }
}

void NoCacheSl::readChunk(const long long &offset, const long long &size)
{
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    assert(offset != -1);
    assert(fd >= 0);
    int res = pread64(fd, buffer_read, size, offset);
    assert(res == size);
    gettimeofday(&end, NULL);
    if (device_id == "emmc")
        st.cache_read_latency.addDeltaT(st.computeDeltaT(begin, end));
    else if (device_id == "sd")
        st.disk_read_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void NoCacheSl::writeChunk(const long long &offset, const long long &size)
{
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    assert(offset != -1);
    assert(fd >= 0);
    int res = pwrite64(fd, buffer_write, size, offset);
    assert(res == size);
    gettimeofday(&end, NULL);
    if (device_id == "emmc")
        st.cache_write_latency.addDeltaT(st.computeDeltaT(begin, end));
    else if (device_id == "sd")
        st.disk_write_latency.addDeltaT(st.computeDeltaT(begin, end));
}

void NoCacheSl::statistic()
{
    string dir = save_root + getSubstringAfter(trace_dir, "trace/") + '/' + device_id + '/' + std::to_string(chunk_size_KB) + "KB/";
    st.resetSaveDir(dir);
    st.record();
}

#endif /*_NO_CACHE_SIMULATOR_HPP_INCLUDED_*/
