#ifndef _BLOCK_HPP_INCLUDED_
#define	_BLOCK_HPP_INCLUDED_

#include "globals.h"

struct block {
    ll key;
    ll offset_cache;
    int  dirty;
    block(){}
    block(const ll &key, const ll &oc, const int &d = 0) : key(key), offset_cache(oc), dirty(d) {}
    friend ostream&  operator<<(ostream& os, const block& arg);
};
ostream& operator<<(ostream& os, const block& arg) {
    os << "key: " << arg.key << endl;
    os << "offset_cache: " << arg.offset_cache << endl;
    // os << "size: " << arg.size << endl;
    os << "dirty: " << arg.dirty << endl;
    return os;
}

#endif /*_BLOCK_INCLUDED_*/
