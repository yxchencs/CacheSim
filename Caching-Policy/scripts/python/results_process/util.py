def convert_to_numeric(value):
    if value.endswith("KB"):
        return int(value[:-2])
    elif value.endswith("MB"):
        return int(value[:-2])
    elif value.endswith("GB"):
        return int(value[:-2])
    elif value.startswith("read_"):
        return float(value.split("_")[1])
    else:
        return int(value)


def convert_cache_policy(list):
    dict = {'random': 'Random',
            'fifo': 'FIFO',   'lru': 'LRU', 'lfu': 'LFU',
            'lirs': 'LIRS', 'arc': 'ARC', 'clockpro': 'CLOCK-Pro',
            '2q': '2Q', 'tinylfu': 'TinyLFU'}

    list_ = [dict[x] for x in list]
    return list_
