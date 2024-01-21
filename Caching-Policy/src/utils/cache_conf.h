#ifndef _CACHE_CONF_HPP_INCLUDED_
#define _CACHE_CONF_HPP_INCLUDED_

double cacheSizeTypes[] = {
    0.02,
    0.04,
    0.06,
    0.08,
    0.1
};

int cache_size_types_size = sizeof(cacheSizeTypes) / sizeof(cacheSizeTypes[0]);

const char *cachePath[] = {
    "cache_0.02.bin",
    "cache_0.04.bin",
    "cache_0.06.bin",
    "cache_0.08.bin",
    "cache_0.1.bin",
};
#endif /*_CACHE_CONF_INCLUDED_*/
