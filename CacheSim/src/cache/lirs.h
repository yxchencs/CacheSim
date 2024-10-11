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

// cache_size_ >= 10

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
        assert(cache_size_ >= 10);
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

    // ɾ��Q��βԪ��
    void FreeOne()
    {
        assert(!q_.empty()); // ���Q�ǿ�

        // ɾ��Q��βԪ��
        auto pnode = q_.back();
        // ll key=pnode->key;//for key_erased
        q_.pop_back();
        pnode->q = q_.end();

        victim = pnode->key;

        // ʹp INVALID
        if (IS_VALID(pnode->value))
        {
            pnode->value = INVALID;
            --used_size_;
        }

        // ��p��S��->������ΪNHIR
        if (pnode->s != s_.end())
        {
            pnode->type = NHIR;
            // ��p����S��->map_ɾ��p
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
        // ���ʴ���LIR��HIR�����ݿ�
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
        // ���ʲ�����LIR��HIR�����ݿ�
        if (used_size_ >= cache_size_ || q_.size() >= q_size_)
        { // ����
            FreeOne();
        }

        // if (used_size_ >= cache_size_) { //���cache���˻�Q��������
        //     erased_key=FreeOne();//ɾ��һ�� ��������
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
        //[LIR]������->�ƶ���S����
        if (p->type == LIR)
        {
            assert(p->s != s_.end()); // ��֤p��S��
            MoveTop(p);               // ��p�ƶ���S����
            //[HIR]������->����ת����p�ƶ���S���ף�s��β�ƶ���Q����
        }
        else if (p->type == HIR)
        {
            assert(p->q != q_.end()); // ��֤p��Q��
            if (p->s != s_.end())
            {                  // ��p��S��
                p->type = LIR; // ת��ΪLIR����

                MoveTop(p);    // p�ƶ���S����
                Pop(p, false); // ��Q������ɾ��p
                Bottom();      // ��S���ж�βԪ���ƶ���Q���ж��ף�ͬʱ��LIRת��ΪHIR
            }
            else
            {                      // ��p����S��
                Push(p, true);     // put p into S
                MoveTop(p, false); // move p top of Q
            }
            //[NHIR]����cache�е�������->
        }
        else
        { // p->type==NHIR
            assert(p->type == NHIR);
            FreeOne(); // ɾ��Q��βԪ��
            p->value = value;

            // ��p��S��->����ת��
            if (p->s != s_.end())
            { // p is in S
                p->type = LIR;
                MoveTop(p); // move p top of S
                Bottom();
                // ��p����S��Q��->p����ΪHIR,��pͬʱ����S��Q����
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

    // ջ��֦: ��Sջ��(back)���з�LIRɾ��->��֤Sջ��һ����LIR
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
    // ��S���ж�βԪ���ƶ���Q���ж��ף�ͬʱ��LIRת��ΪHIR
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
        { // ��p����S����
            s_.push_front(p);
            p->s = s_.begin();
        }
        else
        { // ��p����Q����
            q_.push_front(p);
            p->q = q_.begin();
        }
        // ��p����map_��,��p����map_
        if (map_.find(p->key) == map_.end())
        {
            map_[p->key] = p;
        }
    }
    // true from S, false from Q
    void Pop(std::shared_ptr<lirs_node> p, bool fromS)
    {
        if (fromS)
        { // ��p��S��ɾ��
            assert(p->s != s_.end());
            s_.erase(p->s);
            p->s = s_.end();
        }
        else
        { // ��p��Q��ɾ��
            assert(p->q != q_.end());
            q_.erase(p->q);
            p->q = q_.end();
        }
    }
    // ��p�ƶ���S/Q�Ķ���, trueΪS,falseΪQ
    void MoveTop(std::shared_ptr<lirs_node> p, bool toS = true)
    {
        Pop(p, toS);
        Push(p, toS);
    }

    // front -- top  back  -- bottom
    // Stack S: �������ݿ����ʷ���?(LIR,HIR,NHIR),Խ����ջ��recencyԽС;����LRU,ֻ�Ǵ�С���?
    // Queue Q������resident-HIR����,��С�̶�ΪLhirs
    std::list<std::shared_ptr<lirs_node>> s_, q_;
    std::map<ll, std::shared_ptr<lirs_node>> map_;

    ll cache_size_, used_size_; // used_size_: ĿǰVALID������
    ll s_size_;
    ll q_size_;

    ll victim;

    // statistic sta_;
};

#endif // __LIRS_H__
