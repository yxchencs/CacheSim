#ifndef _NO_CACHE_SIMULATOR_HPP_INCLUDED_
#define _NO_CACHE_SIMULATOR_HPP_INCLUDED_

#include "../utils/statistic.h"
#include "../utils/globals.h"
#include "../utils/run.h"
using namespace std;

void checkFile(fstream &file);
std::string getSubstringAfter(const std::string& original, const std::string& to_find);

class NoCacheSl{
public:
    void test();
    void statistic();

    NoCacheSl(std::string device_id, std::string device_path):device_id(device_id){
        fd= open(device_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd >= 0);

        int res = posix_memalign((void **)&buffer_read, CHUNK_SIZE, CHUNK_SIZE);
        assert(res == 0);

        res = posix_memalign((void **)&buffer_write, CHUNK_SIZE, CHUNK_SIZE);
        assert(res == 0);
        memset(buffer_write, 0, CHUNK_SIZE);
    };
    ~NoCacheSl(){
        close(fd);
        free(buffer_read);
        free(buffer_write);
    };
private:
    int fd;

    char * buffer_read = nullptr;
    char * buffer_write = nullptr;

    Statistic st;
    std::string device_id;

    bool readItem(vector<ll> &keys);
    bool writeItem(vector<ll> &keys);
    void readChunk(const long long &offset, const long long &size);
    void writeChunk(const long long &offset, const long long &size);
};

void NoCacheSl::test(){
    // cout << "-----------------------------------------------------------------" << endl;
    printf("test start\n");
    st.getStartTime();

    fstream fin_trace(trace_path);
    checkFile(fin_trace);

    ll curKey,curSize;
    int type;
    char c;
    string s;
    getline(fin_trace,s);

    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0, NULL);
    while (fin_trace >> curKey >> c >> curSize >> c >> type)
    {
        // if(st.total_trace_nums>3) break;
        st.total_trace_nums++;
        bool isTraceHit;

        ll begin = curKey / CHUNK_SIZE;
        ll end = (curKey + curSize - 1) / CHUNK_SIZE;
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_size += end - begin + 1;

        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * CHUNK_SIZE);
        }

        gettimeofday(&t1, NULL);

        switch (type)
        {
        case 0:
            isTraceHit = readItem(keys);
            break;
        case 1:
            isTraceHit = writeItem(keys);
            break;
        }

        gettimeofday(&t2, NULL);
        long long deltaT = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);
        st.latency_v.push_back(deltaT);
        st.total_latency += deltaT;
        printf("trace: %llu, time: %lldus, total_time: %lld\n", st.total_trace_nums, deltaT, st.total_latency);
        if (isTraceHit)
            st.hit_trace_nums++;
        // printChunkMap();
        // cout<<"isTraceHit: "<<isTraceHit<<' '<<"st.hit_trace_nums: "<<st.hit_trace_nums<<endl;
    }
    gettimeofday(&t3, NULL);
    st.total_time = (t3.tv_sec - t0.tv_sec) * 1000000 + (t3.tv_usec - t0.tv_usec);
    st.getEndTime();
    printf("test end\n");
}

bool NoCacheSl::readItem(vector<ll> &keys)
{
    st.read_nums += keys.size();
    for (int i = 0; i < keys.size(); i++)
    {
        readChunk(keys[i], CHUNK_SIZE);
    }
    return false;
}

bool NoCacheSl::writeItem(vector<ll> &keys)
{
    st.write_nums += keys.size();
    for (int i = 0; i < keys.size(); i++)
    {
        writeChunk(keys[i],CHUNK_SIZE);
    }
    return false;
}

void NoCacheSl::readChunk(const long long &offset, const long long &size)
{
    assert(offset != -1);
    assert(fd >= 0);
    int res = pread64(fd, buffer_read, size, offset);
    assert(res == size);
}

void NoCacheSl::writeChunk(const long long &offset, const long long &size)
{
    assert(offset != -1);
    assert(fd >= 0);
    int res = pwrite64(fd, buffer_write, size, offset);
    assert(res == size);
}

void NoCacheSl::statistic()
{
    std::string dir = save_root + getSubstringAfter(trace_dir,"trace/") + '/' + device_id + '/';
    st.record_in_dir(dir);
}

#endif /*_NO_CACHE_SIMULATOR_HPP_INCLUDED_*/
