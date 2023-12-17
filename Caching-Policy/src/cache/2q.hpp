#ifndef __2Q_HPP__
#define __2Q_HPP__

#include <list>
#include <unordered_map>
#include <cassert>

template <typename Key>
class cache_2q final
{
public:
    cache_2q(size_t cache_size);
    bool cached(Key key);
    void touch(Key key);
    Key getVictim();

private:
    void print();
    void printSize();

    void add_to_A_in(Key key);
    void move_to_A_m_head(Key key);
    void add_to_A_m(Key key);
    void eject_from_A_in();
    void eject_from_A_m();
    void eject_to_A_out();
    void reclaimfor(Key key);

    Key victim_;
    size_t cache_size_;
    size_t A_in_size_;
    size_t A_out_size_;
    size_t A_m_size_;
    list<Key> A_in_;
    list<Key> A_out_;
    list<Key> A_m_; // head - hot, tail - cold
    unordered_map<Key, typename list<Key>::iterator> A_in_hash_;
    unordered_map<Key, typename list<Key>::iterator> A_out_hash_;
    unordered_map<Key, typename list<Key>::iterator> A_m_hash_;
};

template <typename Key>
cache_2q<Key>::cache_2q(size_t cache_size)
    : cache_size_(cache_size),
      A_out_size_(static_cast<size_t>(cache_size / 2)),                         // 50% of cache_size
      A_in_size_(static_cast<size_t>(static_cast<size_t>(cache_size / 2)) / 2), // 25% of cache_size
      A_m_size_(cache_size - A_in_size_),                                                    // cache_size
      victim_(-1)
{
    assert(cache_size >= 3);
    // printSize();
}

// change key with A_m.head()
template <typename Key>
void cache_2q<Key>::move_to_A_m_head(Key key)
{
    Key head_el = *A_m_.begin();
    swap(*A_m_.begin(), *A_m_hash_[key]);
    A_m_hash_[head_el] = A_m_hash_[key];
    A_m_hash_[key] = A_m_.begin();
}

template <typename Key>
void cache_2q<Key>::add_to_A_m(Key key)
{
    // cout<<"add_to_A_m: "<<key<<endl;
    A_m_.push_front(key);
    A_m_hash_[key] = A_m_.begin();
}

template <typename Key>
void cache_2q<Key>::add_to_A_in(Key key)
{
    // cout<<"add_to_A_in: "<<key<<endl;
    A_in_.push_front(key);
    A_in_hash_[key] = A_in_.begin();
}

template <typename Key>
void cache_2q<Key>::eject_from_A_in()
{
    auto it = A_in_.end();
    A_in_hash_.erase(*(--it));
    victim_ = *it;
    A_in_.pop_back(); 
    // cout<<"eject_from_A_in: "<<victim_<<endl;
}

template <typename Key>
void cache_2q<Key>::eject_from_A_m()
{
    auto it = A_m_.end();
    A_m_hash_.erase(*(--it));
    victim_ = *it;
    A_m_.pop_back(); 
    // cout<<"eject_from_A_m: "<<victim_<<endl;
}

template <typename Key>
void cache_2q<Key>::eject_to_A_out()
{
    // cout<<"eject_to_A_out: "<<victim_<<endl;
    assert(A_out_.size() <= A_out_size_);
    if (A_out_.size() >= A_out_size_)
    {
        auto it = A_out_.end();
        it--;
        A_out_hash_.erase(*it);
        A_out_.pop_back();
        A_out_.push_front(victim_);
        A_out_hash_[victim_] = A_out_.begin();
    }
    else
    {
        A_out_.push_front(victim_);
        A_out_hash_[victim_] = A_out_.begin();
    }
}

// apply for data reclaim
template <typename Key>
void cache_2q<Key>::reclaimfor(Key key)
{
    // cout<<"reclaimfor: "<<key<<endl;
    if (A_in_.size() >= A_in_size_)
    {
        eject_from_A_in();
        eject_to_A_out();
    }
    // A_m full
    else if(A_m_.size() >= A_m_size_)
    {
        eject_from_A_m();
    }
}

template <typename Key>
void cache_2q<Key>::touch(Key key)
{
    victim_  = -1; /// reset victim;
    // cout << "===access " << key << "===" << endl;
    // [hit] key in A_m
    if (A_m_hash_.find(key) != A_m_hash_.end())
    {
        // cout<<"[hit] key in A_m"<<endl;
        move_to_A_m_head(key);
    }
    // [miss] key in A_out
    else if (A_out_hash_.find(key) != A_out_hash_.end())
    {
        // cout<<"[miss] key in A_out"<<endl;
        reclaimfor(key);
        // eject_from_A_out(key); // useless for now
        add_to_A_m(key);
    }
    // [hit] key in A_in
    else if (A_in_hash_.find(key) != A_in_hash_.end())
    {
        // cout<<"[hit] key in A_in"<<endl;
        // do nothing
    }
    // [miss] key in no queue
    else
    {
        // cout<<"[miss] key in no queue"<<endl;
        reclaimfor(key);
        add_to_A_in(key);
    }
    // print();
}

template <typename Key>
bool cache_2q<Key>::cached(Key key)
{
    if (A_in_hash_.find(key) != A_in_hash_.end())
    {
        return true;
    }
    if (A_m_hash_.find(key) != A_m_hash_.end())
    {
        return true;
    }
    return false;
}

template <typename Key>
Key cache_2q<Key>::getVictim()
{
    // cout << "2q->victim: " << victim_ << endl;
    return victim_;
}

template <typename Key>
void cache_2q<Key>::print()
{
    cout << "A_in: ";
    for (const auto &key : A_in_)
    {
        cout << key << " ";
    }
    cout << endl;

    cout << "A_out: ";
    for (const auto &key : A_out_)
    {
        cout << key << " ";
    }
    cout << endl;

    cout << "A_m: ";
    for (const auto &key : A_m_)
    {
        cout << key << " ";
    }
    cout << endl;
}

template <typename Key>
void cache_2q<Key>::printSize()
{
    cout << "cache_size_: " << cache_size_
         << ", A_in_size_: " << A_in_size_
         << ", A_out_size_: " << A_out_size_
         << ", A_m_size_: " << A_m_size_;
}

#endif // __2Q_HPP__