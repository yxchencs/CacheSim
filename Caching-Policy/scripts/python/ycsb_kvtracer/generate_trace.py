import csv
import random
import numpy as np


def generate_trace():
    s = []
    trace_size = 100
    key_size = 100
    # value = '0123456789'
    block_size = 512
    trace = []
    for i in range(1, trace_size):
        key = i
        type = random.randint(0, 1)
        trace.append((key, type))

    print(trace)

    # header = ['key', 'data', 'rdwr']
    header = ['key', 'type']

    with open('trace.txt', 'w', encoding='utf-8', newline='') as file_obj:
        # 创建对象
        writer = csv.writer(file_obj)
        # 写表头
        writer.writerow(header)
        # 3.写入数据(一次性写入多行)
        writer.writerows(trace)

def generate_trace(key_maximun,trace_size):
    trace=[]
    for i in range(trace_size):
        if i<= key_maximun:
            key = i
        else:
            key = random.randint(0,key_maximun)
        readOrWrite = random.randint(0, 1)
        trace.append((key, readOrWrite))
    print(trace)
    random.shuffle(trace)
    print(trace)
    header = ['key', 'type']
    with open('trace.txt', 'w', encoding='utf-8', newline='') as file_obj:
        # 创建对象
        writer = csv.writer(file_obj)
        # 写表头
        writer.writerow(header)
        # 3.写入数据(一次性写入多行)
        writer.writerows(trace)

def generate_data(n):
    s = ''
    for _ in range(n):
        # print(_)
        s += str(random.randint(0, 1))
    return s

def generate_file(chunk_size,expected_size,filename):
    s=''
    with open(filename, "a", encoding='utf-8') as f:
        for i in range(expected_size):
            s=s+generate_data(chunk_size)
            print(i,'/',expected_size)
            f.writelines(s)

def create_file(file_path, size):
    """
    # 快速生成大文件
    :param file_path: 文件路径
    :param size: 文件大小，本函数以GB为单位，也可以根据需求设置为KB或MB等
    :return:
    """
    # 首先以路径path新建一个文件，并设置模式为写
    lfile = open(file_path, 'w')
    # 根据文件大小，偏移文件写入位置；位置要减掉一个字节，因为后面要写入一个字节的数据
    lfile.seek(size - 1)
    # 然后在当前位置写入任何内容，必须要写入，不然文件不会那么大哦
    lfile.write('\x00')     # lfile.write('')不会写入任何内容
    lfile.close()


if __name__ == '__main__':
    disk_size=770   *4*1024
    chunk_num=582  *4*1024
    path="D:/Projects/Caching-Policy/Caching-Policy/storage/"
    create_file(path+"disk.bin",disk_size)
    create_file(path+"cache_0.02.bin",chunk_num*0.02)
    create_file(path+"cache_0.04.bin",chunk_num*0.04)
    create_file(path+"cache_0.06.bin",chunk_num*0.06)
    create_file(path+"cache_0.08.bin",chunk_num*0.08)
    create_file(path+"cache_0.1.bin",chunk_num*0.1)