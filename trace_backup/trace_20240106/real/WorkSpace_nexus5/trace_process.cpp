#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <string.h>
#include <stdlib.h>

using namespace std;


// str 为输入字符串，del为分隔符，返回分割之后的字符串数组
vector<string> split(string str) {
	istringstream ss(str);
	vector<string> w;
	string word;
	while(ss >> word) {
		w.push_back(word);
	}
    return w;
}

void trace_search(string s_in,string s_out){
	ifstream  fin;
	ofstream fout;
	fin.open(s_in.c_str(),ios::in);
	fout.open(s_out.c_str(),ios::out);
	string str;
	vector<string> a;
	while(getline(fin,str)){
		a=split(str);
		int t=atoi(a[3].c_str())%2;
		fout<<a[0]<<','<<a[2]<<','<<t<<endl;
	}
	fout.close();
}


int main(){
	string ss1="before/log260_FaceBookHandOuts.txt";
	string ss2="after/log260_FaceBookHandOuts.txt";
	trace_search(ss1,ss2);
}
