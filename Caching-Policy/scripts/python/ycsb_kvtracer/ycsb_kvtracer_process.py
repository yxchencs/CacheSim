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
    with open(file_path, "w") as file:
        # offset = index of the block
        file.write("offset,size,type " + str(block_num) + " " + str(block_num) + ' ' + str(trace_size) + ' ' + str(block_size_KB) +"\n")
        for i in range(len(indexes)):
            file.write(str(indexes[i]) + "," + str(types[i]) + "\n")


def find_directories_with_file(root_dir, filename):
    directories = []
    for dirpath, dirnames, files in os.walk(root_dir):
        if filename in files:
            directories.append(dirpath)
    return directories


def generate_trace(input_path, output_path, block_size_KB):
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


def generate_storage(output_path, disk_size):
    storage_path = os.path.join(output_path,storage_dir)
    print("storage_path:",storage_path)
    os.makedirs(storage_path, exist_ok=True)
    disk_file_path = os.path.join(storage_path, disk_name)
    if not os.path.exists(disk_file_path):
        create_file(disk_file_path, disk_size)
        print("done generate storage")
    else:
        print("storage exists")

def process_workload_device_test(ycsb_dir, trace_name, save_root):
    workload_dir = os.path.join(ycsb_dir, trace_name)
    workload_dirs = find_directories_with_file(workload_dir, ycsb_trace_run_name)

    save_root = os.path.join(save_root, trace_name)
    # workload_dirs.remove(workload_dir) # pop root dir
    # for dir in workload_dirs: print(dir)
    list_block_size_KB = [int(re.search(r'(\d+)KB', dir).group(1)) for dir in workload_dirs]
    # print(list_block_size_KB)
    for i in range(len(workload_dirs)):
        save_dir = os.path.join(save_root,f"{list_block_size_KB[i]}KB")
        os.makedirs(save_dir, exist_ok=True)
        print("process:", workload_dirs[i])
        generate_trace(workload_dirs[i], save_dir, list_block_size_KB[i])
        print("save_dir:", save_dir)
    generate_storage(save_root, max_disk_size)


ycsb_trace_run_name = "trace_run.txt"
trace_file_name = "trace.txt"
storage_dir = "storage"
disk_name = "disk.bin"
max_disk_size = 0

if __name__ == '__main__':
    ycsb_dir = "D:/Projects/YCSB/workloads/device_test_5GB"
    trace_name = "5GB_uniform_read_0"
    save_root = "E:/projects/Caching-Policy/trace_backup"
    process_workload_device_test(ycsb_dir, trace_name, save_root)
        
