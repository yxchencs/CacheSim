#ifndef _BLOCK_HPP_INCLUDED_
#define	_BLOCK_HPP_INCLUDED_

#include "globals.h"

struct Block {
    ll key;
    ll offset_cache;
    int dirty;
    Block(){}
    Block(const ll &key, const ll &oc, const int &d = 0) : key(key), offset_cache(oc), dirty(d) {}
    friend ostream&  operator<<(ostream& os, const Block& arg);
};
ostream& operator<<(ostream& os, const Block& arg) {
    os << "key: " << arg.key << endl;
    os << "offset_cache: " << arg.offset_cache << endl;
    // os << "size: " << arg.size << endl;
    os << "dirty: " << arg.dirty << endl;
    return os;
}

#endif /*_BLOCK_INCLUDED_*/
