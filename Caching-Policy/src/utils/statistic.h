#ifndef STATISTIC_HPP_INCLUDED_
#define	STATISTIC_HPP_INCLUDED_

#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>
#include "config.h"

using namespace std;

class Statistic{
public:

    Statistic(){
        caching_policy="null";

        read_hit_nums=0;
        read_nums=0;

        write_hit_nums=0;
        write_nums=0;

        hit_trace_nums=0;
        total_trace_nums=0;

        total_time=0;
        average_latency=0;
        total_request_size=0;
        
        total_latency = 0;
    }

    //unsigned long long choose_nth(vector<unsigned long long> &a,int startIndex, int endIndex, int n);
    //unsigned long long percentile(const double& p);
    void printStatistic();
    void writeStatistic();
    void getCurrentTimeFormatted(char *formattedTime);
    void getStartTime();
    void getEndTime();
    void saveLatency();

    string caching_policy;

    char startTime[20],endTime[20];

    long long read_hit_nums;
    long long read_nums;

    long long write_hit_nums;
    long long write_nums;

    long long hit_trace_nums;
    long long total_trace_nums;

    //记录处理每个trace使用的时间(ns)
    // vector<double> latency_v;
    // double total_time;
    // double average_latency;


    long long total_time;//float total_time;//unsigned long long total_time;//整体的运行时间

    vector<long long> latency_v;
    long long total_latency;//float total_latency;//unsigned long long total_latency;//所有trace的时间之和
    float average_latency;//unsigned long long average_latency;//所有trace的平均时间

    //记录每个trace实际访问的块的个数
    vector<long long> request_size_v;
    long long total_request_size;

};


void Statistic::getCurrentTimeFormatted(char *formattedTime) {
    time_t timep;
    struct tm *p;

    // 获取当前时间
    time(&timep);
    p = localtime(&timep);

    // 格式化时间为字符串
    snprintf(formattedTime, 20, "%d/%02d/%02d %02d:%02d:%02d",
            1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
}

void Statistic::getStartTime(){
    getCurrentTimeFormatted(startTime);
}

void Statistic::getEndTime(){
    getCurrentTimeFormatted(endTime);
}
/*
unsigned long long Statistic::choose_nth(vector<unsigned long long> &a,int startIndex, int endIndex, int n){
    unsigned long long midOne = a[startIndex];
    int i = startIndex, j = endIndex;
    if(i == j) //递归出口之一
        return a[i];

    if(i < j)
    {
        while(i < j)
        {
            for(; i < j; j--)
            if(a[j] < midOne)
            {
                a[i++] = a[j];
                break;
            }
            for(; i < j; i++)
            if(a[i] > midOne)
            {
                a[j--] = a[i];
                break;
            }
        }
        a[i] = midOne;//支点归位

        int th = endIndex - i + 1;//计算下标为i的数第几大

        if(th == n)//正好找到
        {
            return a[i];
        }
        else
        {
            if(th > n )//在支点右边找
                return choose_nth(a, i + 1, endIndex, n);
            else//在支点左边找第(n-th)大,因为右边th个数都比支点大
                return choose_nth(a, startIndex, i - 1, n - th);
        }
    }
    return 0; //debug: warning: control reaches end of non-void function [-Wreturn-type]
}

unsigned long long Statistic::percentile(const double& p){
    double position = 1+(latency_v.size()-1)*p;
    if(ceil(position) == floor(position)){
        return choose_nth(latency_v,0,latency_v.size()-1,position);
    }else{
        unsigned long long pre=choose_nth(latency_v,0,latency_v.size()-1,floor(position));
        unsigned long long cur=choose_nth(latency_v,0,latency_v.size()-1,ceil(position));
        return pre+(cur-pre)*p;
    }
}
*/
void Statistic::printStatistic(){
    cout<<"-----------------------------------------------------------------"<<endl;
    cout<<"statistic:"<<endl;
    cout<<"caching policy: "<<caching_policy<<endl;
    cout<<"trace: "<<TRACE_PATH<<endl;
    cout<<"cache: "<<CACHE_PATH<<endl;
    cout<<"O_DIRECT: "<<O_DIRECT_ON<<endl;
    cout<<"chunk size: "<<CHUNK_SIZE<<" B"<<endl;
    cout<<"disk size: "<<DISK_SIZE<<" x "<< CHUNK_SIZE <<" B"<<endl;
    cout<<"cache size: "<<CACHE_SIZE<<" x "<< CHUNK_SIZE <<" B"<<endl;
    cout<<"chunk number: "<<CHUNK_NUM<<" x "<< CHUNK_SIZE <<" B"<<endl;
    if(read_nums!=0){
        cout<<"read hit/total number: "<<read_hit_nums<<'/'<<read_nums<<endl;
        cout<<"read hit ratio: "<<read_hit_nums*1.0/read_nums<<endl;
    }
    if(write_nums!=0){
        cout<<"write hit/total number: "<<write_hit_nums<<'/'<<write_nums<<endl;
        cout<<"write hit ratio: "<<write_hit_nums*1.0/write_nums<<endl;
    }
    if((read_nums+write_nums)!=0){
        cout<<"hit/total number: "<<read_hit_nums+write_hit_nums<<'/'<<read_nums+write_nums<<endl;
        cout<<"hit ratio: "<<(read_hit_nums+write_hit_nums)*1.0/(read_nums+write_nums)<<endl;
    }
    if(total_trace_nums!=0){
        cout<<"hit/total trace: "<<hit_trace_nums<<'/'<<total_trace_nums<<endl;
        cout<<"trace hit ratio: "<<hit_trace_nums*1.0/total_trace_nums<<endl;
    }

    
    // cout<<"time(ns): "<<endl;
    // for(auto t:latency_v){
    //      cout<<t<<' ';
    // }
    // cout<<endl; 

    cout<<"From "<<startTime<<" to "<<endTime<<endl;
    cout<<"total time: "<<total_time*1.0/1e6<<" s"<<endl;//cout<<"total time: "<<total_time*1.0/1e9<<" s"<<endl;
    if(total_trace_nums != 0){
        average_latency=total_latency/total_trace_nums;
        cout<<"average latency: "<<average_latency*1.0/1e3<<" ms"<<endl;//cout<<"average latency: "<<average_latency*1.0/1e6<<" ms"<<endl;
        
    }
    sort(latency_v.begin(),latency_v.end());
    cout<<"tail latency: P95="<<latency_v[ceil(0.95*total_trace_nums)]*1.0/1e6<<" ms, P99="<<latency_v[ceil(0.99*total_trace_nums)]*1.0/1e6<<" ms"<<endl;

    // cout<<"total request size: "<<total_request_size<<" x "<<CHUNK_SIZE<<"B"<<endl;  // number of chunks
    cout<<"total request size: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024<<" MB"<<endl;
    if(total_trace_nums != 0){
        cout<<"average size: "<<total_request_size*1.0*CHUNK_SIZE/1024 / total_trace_nums<<" KB"<<endl;
    }
    if(total_time!=0){
        cout<<"bandwidth: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024 / (total_time*1.0/1e6)<<" MB/s"<<endl;//cout<<"bandwidth: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024 / (total_time*1.0/1e9)<<" MB/s"<<endl;
    // puts("power: ");
    // puts("energy: ");
        
        // cout<<"long latency: P95="<<choose_nth(latency_v,0,latency_v.size()-1,ceil(0.95*total_trace_nums))/1e6<<" ms, P99="<<choose_nth(latency_v,0,latency_v.size()-1,ceil(0.99*total_trace_nums))/1e6<<" ms"<<endl;
        // cout<<"tail latency: P95="<<percentile(0.95)/1e6<<" ms, P99="<<percentile(0.99)/1e6<<" ms"<<endl;
    }

    // p分位数位置的值 = 位于p分位数取整后位置的值 + （位于p分位数取整下一位位置的值 - 位于p分位数取整后位置的值）*（p分位数位置 - p分位数位置取整）
    

}

void Statistic::writeStatistic(){
    saveLatency();

    ofstream fout("../results/statistic.txt");
    
    if(!fout.is_open()){
        cerr<<"error: can not open result file"<<endl;
        return;
    }

    fout<<"caching policy: "<<caching_policy<<endl;
    fout<<"trace: "<<TRACE_PATH<<endl;
    fout<<"cache: "<<CACHE_PATH<<endl;
    fout<<"O_DIRECT: "<<O_DIRECT_ON<<endl;
    fout<<"chunk size: "<<CHUNK_SIZE<<" B"<<endl;
    fout<<"disk size: "<<DISK_SIZE<<" x "<< CHUNK_SIZE <<" B"<<endl;
    fout<<"cache size: "<<CACHE_SIZE<<" x "<< CHUNK_SIZE <<" B"<<endl;
    fout<<"chunk number: "<<CHUNK_NUM<<" x "<< CHUNK_SIZE <<" B"<<endl;
    if(read_nums!=0){
        fout<<"read hit/total number: "<<read_hit_nums<<'/'<<read_nums<<endl;
        fout<<"read hit ratio: "<<read_hit_nums*1.0/read_nums<<endl;
    }
    if(write_nums!=0){
        fout<<"write hit/total number: "<<write_hit_nums<<'/'<<write_nums<<endl;
        fout<<"write hit ratio: "<<write_hit_nums*1.0/write_nums<<endl;
    }
    if((read_nums+write_nums)!=0){
        fout<<"hit/total number: "<<read_hit_nums+write_hit_nums<<'/'<<read_nums+write_nums<<endl;
        fout<<"hit ratio: "<<(read_hit_nums+write_hit_nums)*1.0/(read_nums+write_nums)<<endl;
    }
    if(total_trace_nums!=0){
        fout<<"hit/total trace: "<<hit_trace_nums<<'/'<<total_trace_nums<<endl;
        fout<<"trace hit ratio: "<<hit_trace_nums*1.0/total_trace_nums<<endl;
    }

    
    // fout<<"time(ns): "<<endl;
    // for(auto t:latency_v){
    //      fout<<t<<' ';
    // }
    // fout<<endl; 

    fout<<"From "<<startTime<<" to "<<endTime<<endl;
    fout<<"total time: "<<total_time*1.0/1e6<<" s"<<endl;//fout<<"total time: "<<total_time*1.0/1e9<<" s"<<endl;
    if(total_trace_nums != 0){
        average_latency=total_latency*1.0/total_trace_nums;        
        fout<<"average latency: "<<average_latency*1.0/1e3<<" ms"<<endl;//fout<<"average latency: "<<average_latency*1.0/1e6<<" ms"<<endl;
        
    }
    sort(latency_v.begin(),latency_v.end());
    fout<<"tail latency: P95 = "<<latency_v[ceil(0.95*total_trace_nums)]*1.0/1e3<<" ms, P99 = "<<latency_v[ceil(0.99*total_trace_nums)]*1.0/1e3<<" ms"<<endl;//fout<<"tail latency: P95 ="<<latency_v[ceil(0.95*total_trace_nums)]*1.0/1e6<<" ms, P99 ="<<latency_v[ceil(0.99*total_trace_nums)]*1.0/1e6<<" ms"<<endl;

    // fout<<"total request size: "<<total_request_size<<" x "<<CHUNK_SIZE<<"B"<<endl;  // number of chunks
    fout<<"total request size: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024<<" MB"<<endl;
    if(total_trace_nums != 0){
        fout<<"average size: "<<total_request_size*1.0*CHUNK_SIZE/1024 / total_trace_nums<<" KB"<<endl;
    }
    if(total_time!=0){
        fout<<"bandwidth: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024 / (total_time*1.0/1e6)<<" MB/s"<<endl;//fout<<"bandwidth: "<<total_request_size*1.0*CHUNK_SIZE/1024/1024 / (total_time*1.0/1e9)<<" MB/s"<<endl;
    }
    fout<<"power: ";
    fout.close();
}


void Statistic::saveLatency(){
    ofstream fout("../results/trace_latency.txt");
    
    if(fout.is_open()){
        fout<<"traceNo latency(us)"<<endl;
        for(int i=0;i<latency_v.size();i++){
            // cout<<i+1<<' '<<latency_v[i]<<endl;
            fout<<i+1<<' '<<latency_v[i]<<endl;
        }
        fout.close();
        cout<<"save trace latency success"<<endl;
    } else {
        cerr<<"error: can not open result file"<<endl;
    }
}

#endif /*STATISTIC_HPP_INCLUDED_*/
