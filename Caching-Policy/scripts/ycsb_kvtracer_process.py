from collections import Counter
from generate_trace import *

def read_trace_from_file(file_path):
    with open(file_path, 'r') as file:
        keys = []
        types = []
        content = file.read()
        lines = content.split('\n')
        # print(lines)
        for line in lines:
            if len(line) != 0:
                # type
                op = line.split('|')[0]  # op belongs ['READ','INSERT','UPDATE','SCAN']
                # print(op)
                if op == 'READ':
                    types.append(0)
                else:
                    types.append(1)
                # key
                user = line.split(':')[1]
                if len(user) > 1:
                    key = user.split('user')[1]
                    keys.append(key)
                    # print(key)

        return keys, types


def convert_to_indexes(numbers, number_to_index):
    indexes = [number_to_index.get(number, -1) for number in numbers]
    return indexes


def frequency_counter(numbers):
    # 使用Counter统计数字频次
    frequency_counter = Counter(numbers)
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

    return indexes, len(number_to_index)




def output_file(file_path, indexes, types, chunk_size, disk_size, trace_size):
    with open(file_path, "w") as file:
        file.write("offset,size,type " + str(disk_size) + " " + str(trace_size) + "\n")
        for i in range(len(indexes)):
            file.write(str(indexes[i] * chunk_size) + "," + str(chunk_size) + "," + str(types[i]) + "\n")


if __name__ == '__main__':
    # process trace
    file_name = "trace_zipfian_run.txt"
    input_path = "D:/Projects/YCSB/"
    # output_path = "D:/Projects/Caching-Policy/Caching-Policy/trace/YCSB-KVTracer/"
    output_path = "trace/"
    keys, types = read_trace_from_file(input_path + file_name)
    indexes, disk_size = frequency_counter(keys)
    chunk_size = 4 * 1024
    trace_size = len(keys)
    output_file(output_path + file_name, indexes, types, chunk_size, disk_size, trace_size)
    print("done process trace")

    # generate storage
    storage_path="D:/Projects/Caching-Policy/Caching-Policy/storage/"
    disk_size=disk_size*4*1024
    chunk_num=disk_size
    print(disk_size,chunk_num)
    create_file(storage_path+"disk.bin",disk_size)
    create_file(storage_path+"cache_0.02.bin",chunk_num*0.02)
    create_file(storage_path+"cache_0.04.bin",chunk_num*0.04)
    create_file(storage_path+"cache_0.06.bin",chunk_num*0.06)
    create_file(storage_path+"cache_0.08.bin",chunk_num*0.08)
    create_file(storage_path+"cache_0.1.bin",chunk_num*0.1)
    print("done generate storage")
