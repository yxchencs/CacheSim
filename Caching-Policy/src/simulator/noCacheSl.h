#ifndef _NO_CACHE_SIMULATOR_HPP_INCLUDED_
#define _NO_CACHE_SIMULATOR_HPP_INCLUDED_

#include <sstream>
#include "../utils/statistic.h"
#include "../utils/globals.h"
#include "../utils/run.h"
#include "../utils/progressBar.h"

using namespace std;

void checkFile(fstream &file);
string getSubstringAfter(const string &original, const string &to_find);

class NoCacheSl
{
public:
    void test();
    void statistic();

    NoCacheSl(string operation_read_ratio, string device_id, string device_path)
        : operation_read_ratio(operation_read_ratio), device_id(device_id)
    {
        fd = open(device_path.c_str(), O_RDWR | O_DIRECT, 0664);
        assert(fd >= 0);

        int res = posix_memalign((void **)&buffer_read, block_size, block_size);
        assert(res == 0);

        res = posix_memalign((void **)&buffer_write, block_size, block_size);
        assert(res == 0);
        memset(buffer_write, 0, block_size);
    };
    ~NoCacheSl()
    {
        close(fd);
        free(buffer_read);
        free(buffer_write);
    };

private:
    int fd;

    char* buffer_read;
    char* buffer_write;

    Statistic st;
    string device_id;
    string operation_read_ratio;

    void readItem(vector<ll> &keys);
    void writeItem(vector<ll> &keys);
    void readBlock(const long long &offset, const long long &size);
    void writeBlock(const long long &offset, const long long &size);
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

    // std::istringstream iss(s);
    // std::string temp;
    // int number;
    // int trace_size = 0;
    // int count = 0;

    // Get trace_size.
    // while (iss >> temp) {
    //     std::stringstream converter(temp);
    //     if (converter >> number) { 
    //         count++;
    //         if (count == 3) {
    //             trace_size = number;
    //             break;
    //         }
    //     }
    // }

    struct timeval t0, t3, t1, t2;
    gettimeofday(&t0, NULL);
    while (fin_trace >> curKey >> c >> curSize >> c >> type)
    {
        // cout << "----------" << curKey << ' ' << curSize << ' ' << type << "----------" << endl;
        st.total_trace_nums++;
        showProgressBar(st.total_trace_nums, trace_size);


        ll begin = curKey / block_size;
        ll end = (curKey + curSize - 1) / block_size;
        st.request_size_v.push_back(end - begin + 1);
        st.total_request_number += end - begin + 1;
        vector<ll> keys;
        for (ll i = begin; i <= end; i++)
        {
            keys.push_back(i * block_size);
        }

        // st.request_size_v.push_back(block_size);
        // st.total_request_number += 1;
        // vector<ll> keys;
        // keys.push_back(curKey * block_size);

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
        readBlock(keys[i], block_size);
    }
}

void NoCacheSl::writeItem(vector<ll> &keys)
{
    st.write_nums += keys.size();
    for (int i = 0; i < keys.size(); i++)
    {
        writeBlock(keys[i], block_size);
    }
}

void NoCacheSl::readBlock(const long long &offset, const long long &size)
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

void NoCacheSl::writeBlock(const long long &offset, const long long &size)
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
    string dir = save_root + operation_read_ratio + '/' + device_id + '/' + std::to_string(int(block_size*1.0/1024)) + "KB/";

    st.resetSaveDir(dir);
    st.record();
}

#endif /*_NO_CACHE_SIMULATOR_HPP_INCLUDED_*/
