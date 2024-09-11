#ifndef _BITMAP_HPP_INCLUDED_
#define _BITMAP_HPP_INCLUDED_
#include<iostream>
#include<vector>
using namespace std;

class BitMap{
public:
	//位图的内存大小和数据范围有关
	BitMap(size_t range)
		:_bit(range / 32 + 1)
	{}

	void set(const size_t num)
	{
		//计算数组中的下标
		int idx = num / 32;
		//计算num在对应下标整数中的下标位置
		int bitIdx = num % 32;
		//将对应的比特位置1
		_bit[idx] |= 1 << bitIdx;
	}

	bool find(const size_t num)
	{
		int idx = num / 32;
		int bitIdx = num % 32;
		return (_bit[idx] >> bitIdx) & 1;
	}

	void reset(const size_t num)
	{
		int idx = num / 32;
		int bitIdx = num % 32;
		_bit[idx] &= ~(1 << bitIdx);
	}
private:
	vector<int> _bit;
};

#endif	/* _BITMAP_HPP_INCLUDED_ */
