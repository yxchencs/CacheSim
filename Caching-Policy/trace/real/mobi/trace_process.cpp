#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

// str 为输入字符串，del为分隔符，返回分割之后的字符串数组
vector<string> split(string str, char del) {
    stringstream ss(str);
    string tmp;
    vector<string> res;
    while (getline(ss, tmp, del)) {
        res.push_back(tmp);
    }
    return res;
}

void trace_search(string s_in,string s_out){
	ifstream  fin;
	ofstream fout;
	fin.open(s_in.c_str(),ios::in);
	fout.open(s_out.c_str(),ios::out);
	string str;
	vector<string> a;
	fout<<"offset,size,type";
	while(getline(fin,str)){
		cout<<str<<endl;
		a=split(str,',');
		if(a[3]!="0" && a[5]=="0"||a[5]=="1")
			fout<<endl<<a[4]<<','<<a[3]<<','<<a[5];
	}
	cout<<"end"<<endl;
	fout.close();
}


int main(){
	string ss1="before/mobi.trace.463";
	string ss2="after/mobi.trace.463.txt";
	trace_search(ss1,ss2);
}
