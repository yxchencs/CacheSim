#include<iostream>
#include<fstream>
#include<set>
#include<climits>
#include"../../src/utils/config.h"
using namespace std;

int main(){
    // string trace_path="WorkSpace_nexus5/nexus5_31.txt";
    // string trace_path="YCSB-KVTracer/trace_zipfian_run.txt";
    // string trace_path="mobi/mobi.trace.88.txt";
    // string trace_path="MobileAPPTraces/58-life-4h.trace/58-life-4h.trace.txt";
    // string trace_path="MobileAPPTraces/baidutieba-4h/baidutieba-4h.txt";
    // string trace_path="MobileAPPTraces/meitu-4h/meitu-4h.txt";
    // string trace_path="MobileAPPTraces/mojitianqi/mojitianqi.txt";
    string trace_path="mobi/after/mobi.trace.463.txt";
    ifstream fin(trace_path);
    if (!fin.is_open()) {
        cout << "Error: opening trace file fail" << endl;
        fin.close();
        exit(1);
    }
    string s; char c; int type,cnt;
    long long curKey, curSize, begin,end, minBegin=9223372036854775807ll, maxEnd=0;
    getline(fin, s);
    set<long long> keys;
    while (fin >> curKey >> c >> curSize >> c >> type) {
        // cout<<curKey<<" "<<curSize<<" "<<type<<endl;
        begin = curKey/CHUNK_SIZE;
        end =(curKey+curSize-1)/CHUNK_SIZE;
        cout<<"trace: "<<++cnt<<" begin: "<<begin<<" "<<"end: "<<end<<endl;
        for(long long i=begin; i<=end; i++){
            keys.insert(i);
        }
        minBegin=min(minBegin,begin);
        maxEnd=max(maxEnd,end);
    }
    fin.close();
    cout<<"begin: "<<minBegin<<endl;
    cout<<"end: "<<maxEnd<<endl;
    cout<<"interval: "<<maxEnd-minBegin+1<<" x "<<CHUNK_SIZE<<" B"<<endl;
    cout<<"disk size: "<<(maxEnd-minBegin+1)*1.0/1024*CHUNK_SIZE/1024/1024<<" GB"<<endl;
    cout<<"workload: "<<keys.size()<<" x "<<CHUNK_SIZE<<" B"<<endl;
}
