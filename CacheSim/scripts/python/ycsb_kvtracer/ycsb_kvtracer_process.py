import os
import re
import sys
import util
from collections import Counter


# size(B)
def create_storage_file(file_path, size):
    if not do_generate_storage: return
    f = open(file_path, 'w')
    f.seek(size - 1)
    f.write('\x00')
    f.close()


def read_trace_from_file(file_path):
    keys = []
    types = []
    with open(file_path, 'r') as file:
        content = file.read()
        lines = content.split('\n')
        for line in lines:
            if len(line) != 0:
                # type
                op = line.split('|')[0]  # op belongs ['READ','INSERT','UPDATE','SCAN']
                if op == 'GET':
                    types.append(0)
                else:
                    types.append(1)
                # key
                user = line.split(':')[1]
                if len(user) > 1:
                    key = user.split('user')[1]
                    keys.append(key)
    # print('keys size:', len(keys))
    return keys, types


def convert_to_indexes(numbers, number_to_index):
    indexes = [number_to_index.get(number, -1) for number in numbers]
    return indexes


# Replace concrete key with index in frequency_counter
# To make sure keys are consecutive
def frequency_counter(numbers):
    frequency_counter = Counter(numbers)
    number_to_index = {number: index for index, (number, _) in enumerate(frequency_counter.items())}
    indexes = convert_to_indexes(numbers, number_to_index)
    return indexes, len(frequency_counter)


def create_trace_file(file_path, indexes, types, block_num, trace_size, block_size_KB):
    print('block_num:', block_num, 'trace_size:', trace_size, 'block_size_KB:', block_size_KB)
    if not do_generate_trace: return
    block_size = block_size_KB * 1024
    with open(file_path, "w") as file:
        # offset = index of the block
        file.write("offset,size,type " + str(block_num) + " " + str(block_num) + ' ' + str(trace_size) + ' ' + str(
            block_size_KB) + "\n")
        for i in range(len(indexes)):
            file.write(str(indexes[i] * block_size) + "," + str(block_size) + "," + str(types[i]) + "\n")


def find_directories_with_file(root_dir, filename):
    directories = []
    for dirpath, dirnames, files in os.walk(root_dir):
        if filename in files:
            directories.append(dirpath)
    return directories


def generate_trace_device_test(input_path, output_path, block_size_KB):
    global max_disk_size

    trace_file_path = os.path.join(output_path, trace_file_name)
    # print("trace_file_path:", trace_file_path)
    if not os.path.exists(trace_file_path):
        keys, types = read_trace_from_file(os.path.join(input_path, ycsb_trace_run_name))
        indexes, block_num = frequency_counter(keys)
        trace_size = len(keys)
        create_trace_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
        disk_size = block_num * block_size_KB * 1024
        max_disk_size = max(max_disk_size, disk_size)
        print("done process trace")
    else:
        print("trace exists")


def generate_trace(input_path, output_path, block_size_KB):
    global max_disk_size
    global min_disk_size
    trace_run_file_path = os.path.join(input_path, ycsb_trace_run_name)
    trace_file_path = os.path.join(output_path, trace_file_name)
    if not os.path.exists(trace_run_file_path):
        return
    # print("trace_file_path:", trace_file_path)
    if check_trace_exist:
        if not os.path.exists(trace_file_path):
            keys, types = read_trace_from_file(trace_run_file_path)
            indexes, block_num = frequency_counter(keys)
            # block_num >= 500
            # min_cache_size = block_num * 0.02
            # if min_cache_size < 10:  # Demand of caching policy
            #     print("Exiting program due to min_cache_size being less than 10.")
            #     print(f"block_num: {block_num}, min_cache_size: {min_cache_size}")
            #     exit()
            trace_size = len(keys)
            create_trace_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
            disk_size = block_num * block_size_KB * 1024
            max_disk_size = max(max_disk_size, disk_size)
            min_disk_size = min(min_disk_size, disk_size)
            print("disk_size:", disk_size / 1024 / 1024, "MB")
            print("done process trace")
            # generate_storage(output_path, disk_size)
        else:
            print("trace exists")
    else:
        keys, types = read_trace_from_file(trace_run_file_path)
        indexes, block_num = frequency_counter(keys)
        # block_num >= 500
        # min_cache_size = block_num * 0.02
        # if min_cache_size < 10:  # Demand of caching policy
        #     print("Exiting program due to min_cache_size being less than 10.")
        #     print(f"block_num: {block_num}, min_cache_size: {min_cache_size}")
        #     exit()
        trace_size = len(keys)
        create_trace_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
        disk_size = block_num * block_size_KB * 1024
        max_disk_size = max(max_disk_size, disk_size)
        min_disk_size = min(min_disk_size, disk_size)
        print("disk_size:", disk_size / 1024 / 1024, "MB")
        print("done process trace")
        # generate_storage(output_path, disk_size)


def generate_storage_device_test(output_path, disk_size_B):
    storage_path = os.path.join(output_path, storage_dir)
    print("storage_path:", storage_path)
    os.makedirs(storage_path, exist_ok=True)
    disk_file_path = os.path.join(storage_path, disk_name)
    if not os.path.exists(disk_file_path):
        create_storage_file(disk_file_path, disk_size_B)
        print("done generate storage")
    else:
        print("storage exists")


def generate_storage(output_path, disk_size_B):
    storage_path = os.path.join(output_path, storage_dir)
    print("storage_path:", storage_path)
    os.makedirs(storage_path, exist_ok=True)
    disk_file_path = os.path.join(storage_path, disk_name)
    if check_storage_exist:
        if not os.path.exists(disk_file_path):
            print(f"disk_size: {disk_size_B}B")
            create_storage_file(disk_file_path, disk_size_B * (1 + file_size_save_threshold))
            if cache_or_device:
                for cache_size in cache_size_list:
                    cache_file_path = os.path.join(storage_path, f"cache_{cache_size}.bin")
                    print(f"cache_size({cache_size}): {disk_size_B * cache_size * (1 + file_size_save_threshold)}B")
                    create_storage_file(cache_file_path, disk_size_B * cache_size * (1 + file_size_save_threshold))
            print("done generate storage")
        else:
            print("storage exists")
    else:
        print(f"disk_size: {disk_size_B}B")
        create_storage_file(disk_file_path, disk_size_B * (1 + file_size_save_threshold))
        if cache_or_device:
            for cache_size in cache_size_list:
                cache_file_path = os.path.join(storage_path, f"cache_{cache_size}.bin")
                print(f"cache_size({cache_size}): {disk_size_B * cache_size * (1 + file_size_save_threshold)}B")
                create_storage_file(cache_file_path, disk_size_B * cache_size * (1 + file_size_save_threshold))
        print("done generate storage")


def process_workload(trace_dir, trace_save_root, block_size_KB):
    os.makedirs(trace_save_root, exist_ok=True)
    print("process:", trace_dir)
    generate_trace(trace_dir, trace_save_root, block_size_KB)
    print("save_dir:", trace_save_root)


def test():

    for disk_size in disk_size_list:
        total_workload = 0
        valid_workload = 0
        for workload_type in workload_type_list:
            for operation_read_ratio in operation_read_ratio_list:
                for block_size_KB in block_size_KB_list:
                    total_workload = total_workload + 1
                    path = os.path.join(disk_size, workload_type, operation_read_ratio, str(block_size_KB) + 'KB')
                    trace_root = os.path.join(ycsb_root, path)
                    if not os.path.exists(trace_root):
                        continue
                    valid_workload = valid_workload + 1
                    print('------------------------------------------------------------------------------')
                    print(f"path: {path}")
                    trace_save_root = os.path.join(save_root, path)
                    process_workload(trace_root, trace_save_root, block_size_KB)
        if not valid_workload:
            continue
        print('------------------------------------------------------------------------------')
        storage_path = os.path.join(save_root, disk_size)
        disk_size_B = util.extract_disk_size(disk_size)
        generate_storage(storage_path, disk_size_B)
        print('------------------------------------------------------------------------------')
        print(f'Done generate trace({valid_workload}/{total_workload})')


ycsb_trace_run_name = "trace_run.txt"
trace_file_name = "trace.txt"
storage_dir = "storage"
disk_name = "disk.bin"

max_disk_size = 0
min_disk_size = sys.maxsize

file_size_save_threshold = 0  # assert real file size > need size
do_generate_trace = True
do_generate_storage = True
check_trace_exist = False
check_storage_exist = False

save_root = "D:/Projects/Caching-Policy/Caching-Policy/trace"

# device test
# ycsb_root = 'D:/Projects/Caching-Policy/Caching-Policy/workloads'
ycsb_root = 'D:/Users/Ram/Desktop'
disk_size_list = ["2GB"]
# workload_type_list = ['uniform']
# operation_read_ratio_list = ["read_0", "read_1"]
workload_type_list = ["latest", "uniform", "zipfian"]
operation_read_ratio_list = ["read_0", "read_0.2", "read_0.4", "read_0.6", "read_0.8", "read_1"]
block_size_KB_list = [1, 2, 4, 8, 16, 64, 256, 1024, 4096]

# cache test
cache_size_list = [0.02, 0.04, 0.06, 0.08, 0.1]
cache_or_device = True
if __name__ == '__main__':
    test()
    max_disk_size = max_disk_size / 1024 / 1024
    min_disk_size = min_disk_size / 1024 / 1024
    print(f'max_disk_size: {max_disk_size}MB, min_disk_size: {min_disk_size}MB')
