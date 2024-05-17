#ifndef __LIRS_H__
#define __LIRS_H__

#include <list>
#include <iostream>
#include <unordered_map>
#include <assert.h>
#include <stdio.h>
#include <map>
#include "../utils/globals.h"
#include <fstream>

/*  定义宏函数NEED_PRUNING
    输入：一个指向具有type成员的结构体的指针n
    处理：根据该对象的type属性是否等于LIR来判断是否需要进行修剪操作
    输出：如果需要修剪，则宏函数返回true（非零值），否则返回false（零值）*/
#define NEED_PRUNING(n) ((n)->type != LIR)

// typedef unsigned long long ll;
// typedef long long ll;

#define NONE ((ll)-11)
#define INVALID (NONE)
#define IS_VALID(value) ((value) != NONE && (value) != INVALID)

struct lirs_node;
typedef std::list<std::shared_ptr<lirs_node>>::iterator lirs_iterator;

enum lirs_type
{
    LIR = 101,
    HIR,  // 102 -> resident-HIR
    NHIR, // 103 -> nonresident-HIR
};

struct lirs_node
{
    ll key;
    ll value;

    lirs_type type;
    lirs_iterator s;
    lirs_iterator q;

    lirs_node(ll _key, ll _value, lirs_iterator ends, lirs_iterator endq)
        : key(_key), value(_value), s(ends), q(endq), type(LIR) {}
    lirs_node(ll _key, ll _value, lirs_iterator ends, lirs_iterator endq, lirs_type _type)
        : key(_key), value(_value), s(ends), q(endq), type(_type) {}

    void Set(lirs_type _type) { type = _type; }
};

class LIRS
{
public:
    LIRS(ll size = cache_size)
        : cache_size_(size), used_size_(0), s_size_(0.99 * size), q_size_(0.1 * size) // Lhirs = 1%
    {
        assert(s_size_);
        assert(q_size_);
        // isReplaced = false;
        victim = (ll)-1;
        // PrintTxt();
    }

    // LIRS(ll size, ll s, ll q)
    //     : cache_size_(size), used_size_(0), s_size_(s), q_size_(q) {}

    ~LIRS()
    {
        // for (auto it = map_.begin(); it != map_.end(); ++it)
        // {
        //     // std::cout << "key: " << it->second->key << std::endl;
        //     delete (it->second);
        // }
        map_.clear();
    }

    ll getVictim()
    {
        ll curVictim = victim;
        victim = -1;
        return curVictim;
    }

    // 删除Q队尾元素
    void FreeOne()
    {
        assert(!q_.empty()); // 检查Q非空

        // 删除Q队尾元素
        auto pnode = q_.back();
        // ll key=pnode->key;//for key_erased
        q_.pop_back();
        pnode->q = q_.end();

        victim = pnode->key;

        // 使p INVALID
        if (IS_VALID(pnode->value))
        {
            pnode->value = INVALID;
            --used_size_;
        }

        // 若p在S中->设置其为NHIR
        if (pnode->s != s_.end())
        {
            pnode->type = NHIR;
            // 若p不在S中->map_删除p
        }
        else
        {
            // std::cout << "Free Pnode" << std::endl;
            map_.erase(pnode->key);
            // delete pnode;
        }
        // return key;//for key_erased

        // isReplaced = true;
    }

    bool Add(ll key, ll value)
    {
        // Print();
        // 访问存在LIR和HIR的数据块
        if (map_.find(key) != map_.end())
        { // find it
            auto pnode = map_[key];
            if (!IS_VALID(pnode->value))
            {
                ++used_size_;
            }
            Get(key, value);
            // PrintTxt();
            return true;
        }
        // 访问不存在LIR和HIR的数据块
        if (used_size_ >= cache_size_ || q_.size() >= q_size_)
        { // 清理
            FreeOne();
        }

        // if (used_size_ >= cache_size_) { //如果cache满了或Q队列满了
        //     erased_key=FreeOne();//删除一个 操作见上
        // } else if(q_.size() >= q_size_) FreeOne();

        // S is not FULL, so just input it as LIR
        auto p = std::make_shared<lirs_node>(key, value, s_.end(), q_.end());
        assert(p);
        Push(p, true);
        ++used_size_;

        // S is FULL, so just input it as HIR
        if (used_size_ > s_size_)
        {
            // if (s_.size() > s_size_) {
            p->type = HIR;
            Push(p, false);
        }
        // PrintTxt();
        return true;
    }

    ll Get(ll key, ll value = INVALID)
    {
        if (map_.find(key) == map_.end())
        {
            return NONE;
        }

        auto p = map_[key]; // lirs_node*
        //[LIR]热数据->移动到S队首
        if (p->type == LIR)
        {
            assert(p->s != s_.end()); // 保证p在S中
            MoveTop(p);               // 将p移动到S队首
            //[HIR]冷数据->冷热转换：p移动到S队首，s队尾移动到Q队首
        }
        else if (p->type == HIR)
        {
            assert(p->q != q_.end()); // 保证p在Q中
            if (p->s != s_.end())
            {                  // 若p在S中
                p->type = LIR; // 转化为LIR类型

                MoveTop(p);    // p移动到S队首
                Pop(p, false); // 在Q队列中删除p
                Bottom();      // 将S队列队尾元素移动到Q队列队首，同时将LIR转变为HIR
            }
            else
            {                      // 若p不在S中
                Push(p, true);     // put p into S
                MoveTop(p, false); // move p top of Q
            }
            //[NHIR]不在cache中的冷数据->
        }
        else
        { // p->type==NHIR
            assert(p->type == NHIR);
            FreeOne(); // 删除Q队尾元素
            p->value = value;

            // 若p在S中->冷热转换
            if (p->s != s_.end())
            { // p is in S
                p->type = LIR;
                MoveTop(p); // move p top of S
                Bottom();
                // 若p不在S和Q中->p设置为HIR,将p同时放入S和Q队首
            }
            else
            {
                assert(p->q == q_.end());
                p->type = HIR;
                Push(p, true);  // put p into S
                Push(p, false); // put p into Q
            }
        }

        Pruning();

        return p->value;
    }

    ll Peek(ll key)
    {
        ll value = NONE;
        if (map_.find(key) != map_.end())
        { // find it
            value = map_[key]->value;
        }

        // sta_.Hit(value);
        return value;
    }

    // bool Cached(ll key){
    //     ll value = NONE;
    //     if (map_.find(key) != map_.end()) {//find it
    //         value = map_[key]->value;
    //         if((value != NONE) && (value != INVALID)){
    //             return true;
    //         }
    //     }

    //     return false;
    // }

    // 栈剪枝: 将S栈底(back)所有非LIR删除->保证S栈底一定是LIR
    void Pruning()
    {
        while (!s_.empty() && NEED_PRUNING(s_.back()))
        {
            s_.back()->s = s_.end();
            s_.pop_back();
        }
    }

    void PrintTxt(bool flag = false)
    {
        std::ofstream outputFile("output.txt", std::ios::app);
        if (!flag)
        {
            for (auto pit = s_.begin(); pit != s_.end(); ++pit)
            {
                auto it = *pit;
                outputFile << "[" << it->key << ":" << it->value << " " << it->type << "]";
            }
            outputFile << std::endl;

            for (auto pit = q_.begin(); pit != q_.end(); ++pit)
            {
                auto it = *pit;
                outputFile << "[" << it->key << ":" << it->value << " " << it->type << "]";
            }
            outputFile << std::endl;
        }

        // sta_.Print();
        outputFile << "{" << s_.size() << ":" << q_.size() << "}"
                   << "{" << used_size_ << ":" << s_size_ << ":" << q_size_ << "}" << std::endl
                   << std::endl;
        outputFile.close();
    }

    void Print(bool flag = false)
    {
        if (!flag)
        {
            for (auto pit = s_.begin(); pit != s_.end(); ++pit)
            {
                auto it = *pit;
                std::cout << "[" << it->key << ":" << it->value << " " << it->type << "]";
            }
            std::cout << std::endl;

            for (auto pit = q_.begin(); pit != q_.end(); ++pit)
            {
                auto it = *pit;
                std::cout << "[" << it->key << ":" << it->value << " " << it->type << "]";
            }
            std::cout << std::endl;
        }

        // sta_.Print();
        std::cout << "{" << s_.size() << ":" << q_.size() << "}"
                  << "{" << used_size_ << ":" << s_size_ << ":" << q_size_ << "}" << std::endl;
    }

private:
    // 将S队列队尾元素移动到Q队列队首，同时将LIR转变为HIR
    void Bottom()
    {
        auto bottom = s_.back();
        if (bottom->type == LIR)
        {
            bottom->type = HIR;
            if (bottom->q != q_.end())
            {
                Pop(bottom, false);
            }
            Push(bottom, false);
        }
    }
    // true to S, false to Q
    void Push(std::shared_ptr<lirs_node> p, bool toS)
    {
        if (toS)
        { // 将p插入S队首
            s_.push_front(p);
            p->s = s_.begin();
        }
        else
        { // 将p插入Q队首
            q_.push_front(p);
            p->q = q_.begin();
        }
        // 若p不在map_中,则将p加入map_
        if (map_.find(p->key) == map_.end())
        {
            map_[p->key] = p;
        }
    }
    // true from S, false from Q
    void Pop(std::shared_ptr<lirs_node> p, bool fromS)
    {
        if (fromS)
        { // 将p从S中删除
            assert(p->s != s_.end());
            s_.erase(p->s);
            p->s = s_.end();
        }
        else
        { // 将p从Q中删除
            assert(p->q != q_.end());
            q_.erase(p->q);
            p->q = q_.end();
        }
    }
    // 将p移动到S/Q的队首, true为S,false为Q
    void MoveTop(std::shared_ptr<lirs_node> p, bool toS = true)
    {
        Pop(p, toS);
        Push(p, toS);
    }

    // front -- top  back  -- bottom
    // Stack S: 保存数据块的历史记录(LIR,HIR,NHIR),越靠近栈顶recency越小;类似LRU,只是大小会变
    // Queue Q：保存resident-HIR集合,大小固定为Lhirs
    std::list<std::shared_ptr<lirs_node>> s_, q_;
    std::map<ll, std::shared_ptr<lirs_node>> map_;

    ll cache_size_, used_size_; // used_size_: 目前VALID的数据
    ll s_size_;
    ll q_size_;

    ll victim;

    // statistic sta_;
};

#endif // __LIRS_H__
