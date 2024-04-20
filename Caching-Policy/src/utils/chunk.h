#ifndef _CHUNK_HPP_INCLUDED_
#define	_CHUNK_HPP_INCLUDED_

#include "globals.h"

struct chunk {
    ll key;
    ll offset_cache;
    int  dirty;
    chunk(){}
    chunk(const ll &key, const ll &oc, const int &d = 0) : key(key), offset_cache(oc), dirty(d) {}
    friend ostream&  operator<<(ostream& os, const chunk& arg);
};
ostream& operator<<(ostream& os, const chunk& arg) {
    os << "key: " << arg.key << endl;
    os << "offset_cache: " << arg.offset_cache << endl;
    // os << "size: " << arg.size << endl;
    os << "dirty: " << arg.dirty << endl;
    return os;
}

const ll CHUNK_SIZE = 4 * 1024;
#endif /*_CHUNK_INCLUDED_*/
