import os
import re
import csv
import random
from collections import Counter

# size(B)
def create_file(file_path, size):
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


def output_file(file_path, indexes, types, block_num, trace_size, block_size_KB):
    print('block_num:', block_num, 'trace_size:',trace_size, 'block_size_KB:', block_size_KB)
    block_size = block_size_KB * 1024
    with open(file_path, "w") as file:
        # offset = index of the block
        file.write("offset,size,type " + str(block_num) + " " + str(block_num) + ' ' + str(trace_size) + ' ' + str(block_size_KB) +"\n")
        for i in range(len(indexes)):
            file.write(str(indexes[i]*block_size) + "," + str(block_size) + "," + str(types[i]) + "\n")


def find_directories_with_file(root_dir, filename):
    directories = []
    for dirpath, dirnames, files in os.walk(root_dir):
        if filename in files:
            directories.append(dirpath)
    return directories


def generate_trace_device_test(input_path, output_path, block_size_KB):
    global max_disk_size

    trace_file_path = os.path.join(output_path, trace_file_name)
    print("trace_file_path:",trace_file_path)
    if not os.path.exists(trace_file_path):
        keys, types = read_trace_from_file(os.path.join(input_path, ycsb_trace_run_name))
        indexes, block_num = frequency_counter(keys)
        trace_size = len(keys)
        output_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
        disk_size = block_num * block_size_KB * 1024
        max_disk_size = max(max_disk_size, disk_size)
        print("done process trace")
    else:
        print("trace exists")   


def generate_trace(input_path, output_path, block_size_KB):
    global max_disk_size

    trace_file_path = os.path.join(output_path, trace_file_name)
    print("trace_file_path:",trace_file_path)
    # if not os.path.exists(trace_file_path):
    #     keys, types = read_trace_from_file(os.path.join(input_path, ycsb_trace_run_name))
    #     indexes, block_num = frequency_counter(keys)
    #     trace_size = len(keys)
    #     output_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
    #     disk_size = block_num * block_size_KB * 1024
    #     print("disk_size:", disk_size/1024/1024/1024, "GB")
    #     print("done process trace")
    #     generate_storage(output_path, disk_size)
    # else:
    #     print("trace exists")  
    keys, types = read_trace_from_file(os.path.join(input_path, ycsb_trace_run_name))
    indexes, block_num = frequency_counter(keys)
    if block_num < 10:
        print("Exiting program due to block number being less than 10.")
        exit()
    trace_size = len(keys)
    output_file(trace_file_path, indexes, types, block_num, trace_size, block_size_KB)
    disk_size = block_num * block_size_KB * 1024
    print("disk_size:", disk_size/1024/1024/1024, "GB")
    print("done process trace")
    generate_storage(output_path, disk_size)

def generate_storage_device_test(output_path, disk_size):
    storage_path = os.path.join(output_path,storage_dir)
    print("storage_path:",storage_path)
    os.makedirs(storage_path, exist_ok=True)
    disk_file_path = os.path.join(storage_path, disk_name)
    if not os.path.exists(disk_file_path):
        create_file(disk_file_path, disk_size)
        print("done generate storage")
    else:
        print("storage exists")

def generate_storage(output_path, disk_size):
    storage_path = os.path.join(output_path,storage_dir)
    print("storage_path:",storage_path)
    os.makedirs(storage_path, exist_ok=True)
    disk_file_path = os.path.join(storage_path, disk_name)
    cache_size_list = [0.02, 0.04, 0.06, 0.08, 0.1]
    # if not os.path.exists(disk_file_path):
    #     print(f"disk_size: {disk_size}B")
    #     create_file(disk_file_path, disk_size*(1+file_size_save_threshold))
    #     for cache_size in cache_size_list:
    #         cache_file_path = os.path.join(storage_path, f"cache_{cache_size}.bin")
    #         print(f"cache_size({cache_size}): {disk_size*cache_size*(1+file_size_save_threshold)}B")
    #         create_file(cache_file_path, disk_size*cache_size*(1+file_size_save_threshold))
    #     print("done generate storage")
    # else:
    #     print("storage exists")
    print(f"disk_size: {disk_size}B")
    create_file(disk_file_path, disk_size*(1+file_size_save_threshold))
    for cache_size in cache_size_list:
        cache_file_path = os.path.join(storage_path, f"cache_{cache_size}.bin")
        print(f"cache_size({cache_size}): {disk_size*cache_size*(1+file_size_save_threshold)}B")
        create_file(cache_file_path, disk_size*cache_size*(1+file_size_save_threshold))
    print("done generate storage")

def process_workload_device_test(ycsb_dir, workload_name, save_dir):
    workload_root = os.path.join(ycsb_dir, workload_name)
    workload_dirs = find_directories_with_file(workload_root, ycsb_trace_run_name)

    trace_save_root = os.path.join(save_dir, workload_name)
    if workload_root in workload_dirs:
        workload_dirs.remove(workload_root) # pop root dir
    for dir in workload_dirs: print(dir)
    list_block_size_KB = [int(re.search(r'(\d+)KB', dir).group(1)) for dir in workload_dirs]
    # print(list_block_size_KB)
    for i in range(len(workload_dirs)):
        trace_save_dir = os.path.join(trace_save_root,f"{list_block_size_KB[i]}KB")
        os.makedirs(trace_save_dir, exist_ok=True)
        print("process:", workload_dirs[i])
        generate_trace_device_test(workload_dirs[i], trace_save_dir, list_block_size_KB[i])
        print("save_dir:", trace_save_dir)

def process_workload(ycsb_dir, workload_name, save_dir):
    workload_root = os.path.join(ycsb_dir, workload_name)
    workload_dirs = find_directories_with_file(workload_root, ycsb_trace_run_name)

    trace_save_root = os.path.join(save_dir, workload_name)
    if workload_root in workload_dirs:
        workload_dirs.remove(workload_root) # pop root dir
    # for dir in workload_dirs: print(dir)
    list_block_size_KB = [int(re.search(r'(\d+)KB', dir).group(1)) for dir in workload_dirs]
    # print(list_block_size_KB)
    for i in range(len(workload_dirs)):
        block_size_KB = list_block_size_KB[i]
        trace_save_dir = os.path.join(trace_save_root,f"{block_size_KB}KB")
        os.makedirs(trace_save_dir, exist_ok=True)
        print("process:", workload_dirs[i])
        generate_trace(workload_dirs[i], trace_save_dir, block_size_KB)
        print("save_dir:", trace_save_dir)
    
def device_test():
    ycsb_root = "D:/Projects/YCSB/workloads"
    trace_name = "5GB/uniform"
    workload_name_list = ["read_0","read_0.2","read_0.4","read_0.6","read_0.8","read_1"]
    save_root = "E:/projects/Caching-Policy/trace_backup"
    
    ycsb_dir = os.path.join(ycsb_root, trace_name)
    save_dir = os.path.join(save_root, trace_name)
    os.mkdir(save_dir)
    for workload_name in workload_name_list:
        process_workload_device_test(ycsb_dir, workload_name, save_dir)
    generate_storage_device_test(save_dir, max_disk_size)

def caching_policy_test():
    # ycsb_root = "../../../../workloads"
    ycsb_root = "D:/Projects/YCSB/workloads"
    # trace_name = "1GB/latest"
    trace_name = "1GB/uniform"
    # workload_name_list = ["read_0","read_0.2","read_0.4","read_0.6","read_0.8","read_1"]
    workload_name_list = ["read_1"]
    save_root = "E:/projects/Caching-Policy/trace_wait"

    ycsb_dir = os.path.join(ycsb_root, trace_name)
    save_dir = os.path.join(save_root, trace_name)
    for workload_name in workload_name_list:
        # print(workload_name)
        process_workload(ycsb_dir, workload_name, save_dir)


ycsb_trace_run_name = "trace_run.txt"
trace_file_name = "trace.txt"
storage_dir = "storage"
disk_name = "disk.bin"
max_disk_size = 0
file_size_save_threshold = 0 # assert real file size > need size

if __name__ == '__main__':
    caching_policy_test()
