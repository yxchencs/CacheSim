from collections import Counter
from generate_trace import *
import os


def read_trace_from_file(file_path):
    keys = []
    types = []
    with open(file_path, 'r') as file:
        content = file.read()
        lines = content.split('\n')
        # print(lines)
        for line in lines:
            if len(line) != 0:
                # type
                op = line.split('|')[0]  # op belongs ['READ','INSERT','UPDATE','SCAN']
                # print(op)
                if op == 'GET':
                    types.append(0)
                else:
                    types.append(1)
                # key
                user = line.split(':')[1]
                if len(user) > 1:
                    key = user.split('user')[1]
                    keys.append(key)
                    # print(key)
    print('keys size:', len(keys))
    return keys, types


def convert_to_indexes(numbers, number_to_index):
    indexes = [number_to_index.get(number, -1) for number in numbers]
    return indexes


def frequency_counter(numbers):
    # 使用Counter统计数字频次,返回numbers列表中每个元素及其对应出现次数的字典
    frequency_counter = Counter(numbers)
    # print("frequency_counter",frequency_counter)
    # 创建数字到编号的映射
    number_to_index = {number: index for index, (number, _) in enumerate(frequency_counter.items())}

    # print("Number to Index Mapping:")
    # for number, index in number_to_index.items():
    # print(f"{number}: {index}")

    indexes = convert_to_indexes(numbers, number_to_index)
    # print("\nConverted Indexes:")
    # print(indexes)

    # 输出频次和编号映射
    # print("Number Frequency:")
    # for number, frequency in frequency_counter.items():
    #     print(f"{number}: {frequency}")

    # print("\nNumber to Index Mapping:")
    # for number, index in number_to_index.items():
    #     print(f"{number}: {index}")
    return indexes, len(frequency_counter)


def output_file(file_path, indexes, types, disk_size, trace_size):
    print('disk_size:', disk_size, ',trace_size:',trace_size)
    with open(file_path, "w") as file:
        file.write("offset,size,type " + str(disk_size) + " " + str(disk_size) + ' ' + str(trace_size) + "\n")
        for i in range(len(indexes)):
            file.write(str(indexes[i]) + "," + str(types[i]) + "\n")


def find_directories_with_file(root_dir, filename):
    directories = []
    for dirpath, dirnames, files in os.walk(root_dir):
        if filename in files:
            directories.append(dirpath)
    return directories


def process_trace(input_path, output_path):
    # process trace
    os.makedirs(output_path, exist_ok=True)
    output_name = "trace.txt"

    keys, types = read_trace_from_file(os.path.join(input_path,file_name))

    indexes, disk_size = frequency_counter(keys)
    trace_size = len(keys)
    output_file(os.path.join(output_path,output_name), indexes, types, disk_size, trace_size)
    print("done process trace")

    # generate storage
    storage_path = os.path.join(output_path,"storage")
    os.makedirs(storage_path, exist_ok=True)
    disk_size = disk_size * 4 * 1024
    chunk_num = disk_size
    create_file(os.path.join(storage_path,"disk.bin"), disk_size)
    create_file(os.path.join(storage_path,"cache_0.02.bin"), chunk_num * 0.02)
    create_file(os.path.join(storage_path,"cache_0.04.bin"), chunk_num * 0.04)
    create_file(os.path.join(storage_path,"cache_0.06.bin"), chunk_num * 0.06)
    create_file(os.path.join(storage_path,"cache_0.08.bin"), chunk_num * 0.08)
    create_file(os.path.join(storage_path,"cache_0.1.bin"), chunk_num * 0.1)
    print("done generate storage")

# 测试memory使用，选择uniform分布，数据集设置大一点，比如20gb，然后cache size设置8% 16% 32%
def process_trace2(input_path, output_path):
    # process trace
    os.makedirs(output_path, exist_ok=True)
    output_name = "trace.txt"

    keys, types = read_trace_from_file(os.path.join(input_path,file_name))

    indexes, disk_size = frequency_counter(keys)
    trace_size = len(keys)
    output_file(os.path.join(output_path,output_name), indexes, types, disk_size, trace_size)
    print("done process trace")

    # generate storage
    storage_path = os.path.join(output_path,"storage")
    os.makedirs(storage_path, exist_ok=True)
    disk_size = disk_size * 4 * 1024
    chunk_num = disk_size
    create_file(os.path.join(storage_path,"disk.bin"), disk_size)
    # create_file(os.path.join(storage_path,"cache_0.08.bin"), chunk_num * 0.08)
    # create_file(os.path.join(storage_path,"cache_0.16.bin"), chunk_num * 0.16)
    # create_file(os.path.join(storage_path,"cache_0.32.bin"), chunk_num * 0.32)
    print("done generate storage")

file_name = "trace_run.txt" # 用于转换成格式化trace的文件名

# 为root_directory目录下的trace_run.txt文件生成storage和trace.txt
if __name__ == '__main__':
    root_directory = "E:/projects/Caching-Policy/trace_backup/trace_5GB/"
    matching_directories = find_directories_with_file(root_directory, file_name)
    print(matching_directories)
    for directory in matching_directories:
        print("process", directory)
        process_trace2(directory,directory)
