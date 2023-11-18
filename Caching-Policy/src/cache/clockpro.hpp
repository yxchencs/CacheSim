#ifndef __CLOCK_PRO_H__
#define __CLOCK_PRO_H__
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <map>
// #include "cuckoohash_map.hh"
// #include <libcuckoo/bucket_container.hh>
// #include <libcuckoo/cuckoohash_config.hh>
// #include <libcuckoo/cuckoohash_util.hh>
// #include <libcuckoo/bucket_container.hh>
// #include <libcuckoo/cuckoohash_map.hh>

namespace clockpro {

enum PageType
{
	ptEmpty,
	ptTest,
	ptCold,
	ptHot
};

template <typename k, typename v>
struct Entry  {
	// member for instrusive circular list
	Entry<k, v>* next;
	Entry<k, v>* prev;
	PageType ptype;
	bool ref;
	k key;
	std::optional<v> val;

	Entry() {
		next = this;
		prev = this;
	}

	Entry(bool ref, k pkey, v pval, PageType type) {
		next = this;
		prev = this;
		key = pkey;
		val = pval;
		ptype = type;
	}

	// Link connects ring r with ring s such that r.Next()
	// becomes s and returns the original value for r.Next().
	// r must not be empty.
	Entry<k, v>* Link(Entry* s)
	{
		auto n = this->Next();
		if (s != nullptr)
		{
			auto p = s->Prev();
			this->next = s;
			s->prev = this;
			n->prev = p;
			p->next = n;
		}
		return n;
	}

	Entry<k, v>* init()
	{
		this->next = this;
		this->prev = this;
		return this;
	}

	// Next returns the next ring element. r must not be empty.
	Entry<k, v>* Next()
	{

		return this->next;
	}

	// Prev returns the previous ring element. r must not be empty.
	Entry<k, v>*  Prev()
	{
		return this->prev;
	}

	Entry<k, v>* Unlink(int n )
	{
		if (n <= 0)
		{
			return nullptr;
		}
		return this->Link(this->Move(n + 1));
	}

	Entry<k, v>*  Move(int n )
	{
		Entry<k, v>* r = this;
		if (n < 0) 
		{
			for( ; n < 0; n++)
			{
				r = r->prev;
			}
		}
		else if (n > 0) 
		{
			for (; n > 0; n--) 
			{
				r = r->next;
			}
		}
		return r;
	}
};



template <typename k, typename v>
struct Cache {
	typedef Entry<k, v>* Entryref;
	size_t _capacity;
	size_t _test_capacity;
	size_t	_cold_capacity;
	mutable std::mutex cacheMutex;
	// cuckoohash_map<k, Entryref> map;
	std::map<k, Entryref> cache_map;
	Entryref hand_hot;
	Entryref hand_cold;
	Entryref hand_test;
	size_t count_hot;
	size_t count_cold;
	size_t count_test;

	k curVictim;

	Cache(size_t size)
	{
		if (size < 3) {
			throw "Cache size cannot be less than 3 entries";
		}
		_capacity = size;
		_cold_capacity = size;
		hand_hot = nullptr;
		hand_cold = nullptr;
		hand_test = nullptr;
		count_hot = 0;
		count_cold = 0;
		count_test = 0;
		curVictim = (ll)-1;
	}

    k getCurVictim(){
        // isReplaced = false;
        k victim = curVictim;
        curVictim = (ll)-1;
        return victim;
    }

	std::optional<v> Get(const k& key)
	{
		
		int cnt = cache_map.count(key); // Entryref mentry; bool found = map.find(key, mentry);
		if (!cnt)
		{
			return {}; // not found
		}
		Entryref mentry = cache_map.at(key);
		if (!mentry->val.has_value())
		{
			return {}; // meta_del or convert from cold node to test node (eviction)
		}
		mentry->ref = true;
		return mentry->val;
	}

	bool Set(k key, v value)
	{
		int cnt = cache_map.count(key); // Entryref mentry; bool found = map.find(key, mentry);
		if (!cnt) // miss
		{
			// Allocate memory outside of holding cache the lock
			auto e = new Entry<k, v>(false, key, value, ptCold);
			// no cache entry?  add it
			cache_map[key]=e; // map.insert_or_assign(key, e);
			std::unique_lock<std::mutex> lockx(cacheMutex);
			meta_add(e);
			count_cold++;
			return true;
		}
		Entryref mentry = cache_map.at(key);
		if (mentry->ptype == ptTest) // miss (NonResidentHit)
		{
			std::unique_lock<std::mutex> lockx(cacheMutex);
			// cache entry was a test page
			if (_cold_capacity < _capacity)
			{
				_cold_capacity++;
			}
			meta_del(mentry, false);
			count_test--;
			mentry->ptype = ptHot;
			meta_add(mentry);
			count_hot++;
			return true;
		} else { // Hit
			// cache entry was a hot or cold page
			mentry->val = value;
			mentry->ref = true;
			return false;
		}
	}

	bool Cached(k key){
		int cnt = cache_map.count(key); // Entryref mentry; bool found = map.find(key, mentry);
		if (!cnt) return false; // miss
		Entryref mentry = cache_map.at(key);
		if(key==81920) cout<<"data exict: ptype = "<<mentry->ptype<<endl;
		if (mentry->ptype == ptTest) return false;
		return true;
	}

	void meta_add(Entryref r)
	{
		printf("meta_add\n");
		evict();
		if (hand_hot == nullptr)
		{
			// first element
			hand_hot = r;
			hand_cold = r;
			hand_test = r;
		} else {
			//Add meta data after hand hot
			hand_hot->Link(r);
		}
		
		if (hand_cold == hand_hot) {
			hand_cold = r->Next();
		}
		if (hand_test == hand_hot) {
			hand_test = r->Next();
		}
		hand_hot = r->Next();
	}

	void meta_del(Entryref e, bool deleteNode = true)
	{
		printf("meta_del\n");
		e->ptype = ptEmpty;
		e->ref = false;
		e->val = {};

		auto it = cache_map.find(e->key);
    	if (it != cache_map.end()) cache_map.erase(it);// map.erase(e->key);

		auto next = e->Next();
		if (e == hand_hot) {
			hand_hot = next;
		}

		if (e == hand_cold){
			hand_cold = next;
		}

		if (e == hand_test){
			hand_test = next;
		}

		e->Prev()->Unlink(1);
		if (deleteNode)
		    delete e;
	}

	void evict()
	{
		printf("evict\n");
		while (_capacity <= count_hot + count_cold)
		{
			run_hand_cold();
		}
	}

	void run_hand_cold()
	{
		printf("run_hand_cold: ");
		cout<<hand_cold->key<<endl;
		auto mentry = hand_cold;
		if (mentry->ptype == ptCold)
		{
			if (mentry->ref)
			{
				mentry->ptype = ptHot;
				mentry->ref = false;
				count_cold--;
				count_hot++;
			} else {
				// convert from cold node to test node (eviction)
				mentry->ptype = ptTest;
				mentry->val = {};
				count_cold--;
				count_test++;

				curVictim = mentry->key;
				cout<<"curVictim: "<<curVictim<<endl;

				while (_capacity < count_test)
				{
					run_hand_test();
				}
			}
		}
		// Move the hand forward
		hand_cold = hand_cold->Next();
		while (_capacity - _cold_capacity < count_hot)
		{
			run_hand_hot();
		}
	}

	void run_hand_hot() 
	{
		printf("run_hand_hot: ");
		cout<<hand_hot->key<<endl;
		if (hand_hot == hand_test)
		{
			run_hand_test();
		}

		auto mentry = hand_hot;
		if (mentry->ptype == ptHot)
        {
			if (mentry->ref) 
			{
				mentry->ref = false;
			}
			else
			{
				mentry->ptype = ptCold;
				count_hot--;
				count_cold++;
			}
		}
		hand_hot = hand_hot->Next();
	}

	void run_hand_test()
	{
		printf("run_hand_test: ");
		cout<<hand_test->key<<endl;
		if (hand_test == hand_cold)
		{
			run_hand_cold();
		}

		auto mentry = hand_test;

		if (mentry->ptype == ptTest)
		{
			auto prev = hand_test->Prev();
			meta_del(hand_test);
			hand_test = prev;

			count_test--;
			if (_cold_capacity > 1)
			{
				_cold_capacity--;
			}
		}
		// Move the hand forward
		hand_test = hand_test->Next();
	}

};

}
#endif // __CLOCK_PRO_H__
