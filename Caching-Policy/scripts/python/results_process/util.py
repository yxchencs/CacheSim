import pandas as pd
import os
from datetime import datetime, timedelta
import re

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


def convert_cache_policy(x):
    dict = {'random': 'Random',
            'fifo': 'FIFO', 'lru': 'LRU', 'lfu': 'LFU',
            'lirs': 'LIRS', 'arc': 'ARC', 'clockpro': 'CLOCK-Pro',
            '2q': '2Q', 'tinylfu': 'TinyLFU'}
    return dict[x]


# 合并所有statistic.xlsx，存放到path_head目录下
# 输入文件头 path_head 和 存放各个statistic.xlsx的文件list folder_list
def merge_excel_files(path_dir, folder_list):
    # 初始化空 DataFrame 用于合并数据
    combined_df = pd.DataFrame()

    # 遍历文件夹并读取每个文件
    for folder in folder_list:
        file_path = os.path.join(path_dir, folder, file_result_name)
        print(file_path)
        if os.path.exists(file_path):
            df = pd.read_excel(file_path)
            combined_df = combined_df.append(df, ignore_index=True)

    # 保存合并后的 DataFrame 到新文件
    save_dir = os.path.join(path_dir, file_result_name)
    combined_df.to_excel(save_dir, index=False)
    print("Files merged successfully into", save_dir)


def str_to_datetime(raw_str, format):
    return datetime.strptime(raw_str, format)


def calculate_avg_cpu_usage(file_path, time_begin, time_end):
    cpu_cnt, cpu_sum = 0, 0
    time_format = "%Y-%m-%d %H:%M:%S"

    with open(file_path, 'r') as file:
        for line in file:
            # print(line)
            parts = line.strip().split()
            time = str_to_datetime((parts[1] + ' ' + parts[2]), time_format)

            if time_end >= time >= time_begin:
                cpu_cnt += 1
                cpu_sum += float(parts[0].split('%')[0].strip())

    if cpu_cnt != 0:
        return cpu_sum / cpu_cnt
    else:
        return None


def calculate_avg_power(file_path, time_begin, time_end):
    power_cnt, power_sum = 0, 0
    # Assuming the date is not important for the time in the Excel file, and only time is relevant
    time_format = "%H:%M:%S"
    # Convert string parameters to time objects
    # print('calculate_avg_power->', str(time_begin).split()[-1])
    time_begin_obj = datetime.strptime(str(time_begin).split()[-1], time_format)
    time_end_obj = datetime.strptime(str(time_end).split()[-1], time_format)
    # print(time_begin_obj, time_end_obj)
    # Read the Excel file
    df = pd.read_excel(file_path)
    # print(df.columns)

    # Calculate the average power between time_begin and time_end
    for index, row in df.iterrows():
        # Extract the time from the Excel file
        if str(row['Time(HH:MM:SS)']) != 'Time(HH:MM:SS)':
            current_time_obj = datetime.strptime(str(row['Time(HH:MM:SS)']), time_format)
            # Check if the current time is within the time range specified
            if time_end_obj >= current_time_obj >= time_begin_obj:
                power_cnt += 1
                power_sum += float(row['Power(W)'])
    # print("power_cnt:",power_cnt)
    if power_cnt == 0:
        return None
    return power_sum / power_cnt


# [time_begin, time_end]时间段所有mem_used的平均值
def calculate_avg_mem_used(file_path, time_begin, time_end):
    mem_begin, mem_cnt, mem_sum = 0, 0, 0
    time_format = "%Y-%m-%d %H:%M:%S"
    # with open(file_path, 'r') as file:
    #     for line in file:
    #         parts = line.strip().split()
    #         time = str_to_datetime((parts[1] + ' ' + parts[2]), time_format)
    #
    #         if time >= time_begin:
    #             mem_begin = int(parts[0].split('MB')[0])
    #             break

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split()
            time = str_to_datetime((parts[1] + ' ' + parts[2]), time_format)

            if time_end >= time >= time_begin:  # 不取到等号以避免数据突变带来的数据异常
                mem_used = int(parts[0].split('MB')[0].strip())
                if mem_begin == 0:
                    mem_begin = mem_used
                mem_cnt += 1
                # mem_sum += int(parts[0].split('MB')[0].strip()) - mem_begin # [time_begin, time_end]时间段内所有mem_used数值和mem_begin的差之和 / mem_cnt
                mem_sum += mem_used

    if mem_cnt != 0:
        return mem_sum / mem_cnt - mem_begin
    else:
        return None


# IO Read/Write Mount
def calculate_disk_read_wrtn(file_path, time_begin, time_end):
    emmc_kb_read_begin, emmc_kb_wrtn_begin, sd_kb_read_begin, sd_kb_wrtn_begin = 0, 0, 0, 0
    emmc_kb_read_end, emmc_kb_wrtn_end, sd_kb_read_end, sd_kb_wrtn_end = 0, 0, 0, 0
    time_format = "%Y-%m-%d %H:%M:%S"

    with open(file_path, 'r') as file:
        for line in file:
            # print(line)
            parts = line.strip().split(';')
            time = str_to_datetime((parts[2].strip()), time_format)

            if time == time_begin:
                emmc = parts[0].split()
                sd = parts[1].split()
                emmc_kb_read_begin = int(emmc[3].strip(','))
                emmc_kb_wrtn_begin = int(emmc[6].strip())
                sd_kb_read_begin = int(sd[3].strip(','))
                sd_kb_wrtn_begin = int(sd[6].strip())
                break
            if time == time_begin + timedelta(seconds=1):
                emmc = parts[0].split()
                sd = parts[1].split()
                emmc_kb_read_begin = int(emmc[3].strip(','))
                emmc_kb_wrtn_begin = int(emmc[6].strip())
                sd_kb_read_begin = int(sd[3].strip(','))
                sd_kb_wrtn_begin = int(sd[6].strip())
                break

    with open(file_path, 'r') as file:
        for line in file:
            # print(line)
            parts = line.strip().split(';')
            time = str_to_datetime((parts[2].strip()), time_format)

            if time == time_end:
                emmc = parts[0].split()
                sd = parts[1].split()
                emmc_kb_read_end = int(emmc[3].strip(','))
                emmc_kb_wrtn_end = int(emmc[6].strip())
                sd_kb_read_end = int(sd[3].strip(','))
                sd_kb_wrtn_end = int(sd[6].strip())
                break
            if time == time_end + timedelta(seconds=1):
                emmc = parts[0].split()
                sd = parts[1].split()
                emmc_kb_read_end = int(emmc[3].strip(','))
                emmc_kb_wrtn_end = int(emmc[6].strip())
                sd_kb_read_end = int(sd[3].strip(','))
                sd_kb_wrtn_end = int(sd[6].strip())
                break

    emmc_kb_read = emmc_kb_read_end - emmc_kb_read_begin
    emmc_kb_wrtn = emmc_kb_wrtn_end - emmc_kb_wrtn_begin
    sd_kb_read = sd_kb_read_end - sd_kb_read_begin
    sd_kb_wrtn = sd_kb_wrtn_end - sd_kb_wrtn_begin
    return emmc_kb_read, emmc_kb_wrtn, sd_kb_read, sd_kb_wrtn


def convert_power_from_txt_to_xlsx():
    global log_dir
    global file_power_path
    df = pd.read_csv(os.path.join(log_dir, "Umeter.txt"), sep="\s+", header=None,
                     names=["Time(HH:MM:SS)", "Voltage(V)", "Current(A)", "D+(V)", "D-(V)"], skiprows=1, encoding='GBK')
    df.drop(columns=["D+(V)", "D-(V)"], inplace=True)
    df["Power(W)"] = df["Voltage(V)"] * df["Current(A)"]

    df.to_excel(file_power_path, index=False)
    print(f'done convert {file_power_path}')


def reset_path():
    global path_head
    global log_dir
    global file_cpu_usage_path
    global file_mem_used_path
    global file_disk_path
    global file_power_path

    log_dir = os.path.join(path_head, dir_log_name)

    file_cpu_usage_path = os.path.join(log_dir, file_cpu_usage_name)
    os.chmod(file_cpu_usage_path, 0o666)
    file_mem_used_path = os.path.join(log_dir, file_mem_used_name)
    os.chmod(file_mem_used_path, 0o666)
    file_disk_path = os.path.join(log_dir, file_disk_name)
    os.chmod(file_disk_path, 0o666)

    if PROCESS_POWER:
        file_power_path = os.path.join(log_dir, file_power_name)
        convert_power_from_txt_to_xlsx()
        os.chmod(file_power_path, 0o666)

    print('done reset path')


def extract_statistic(filename, rdwr_only):
    with open(filename, 'r') as file:
        data = file.read()
    bias = 0
    if not rdwr_only:
        bias = 2
    lines = data.strip().split('\n')
    total_time = float(lines[15 + bias].split()[2])
    avg_latency = float(lines[16 + bias].split()[2])
    p99 = float(lines[17 + bias].split('=')[2].split()[0])
    bandwidth = float(lines[20 + bias].split()[1])

    emmc_read_nums = int(lines[22 + bias].split(';')[0].strip().split()[2])
    emmc_read_avg_latency = float(lines[22 + bias].split(';')[1].strip().split()[2])
    emmc_read_p99 = float(lines[22 + bias].split(';')[2].strip().split()[8])

    emmc_write_nums = int(lines[23 + bias].split(';')[0].strip().split()[2])
    emmc_write_avg_latency = float(lines[23 + bias].split(';')[1].strip().split()[2])
    emmc_write_p99 = float(lines[23 + bias].split(';')[2].strip().split()[8])

    sd_read_nums = int(lines[24 + bias].split(';')[0].strip().split()[2])
    sd_read_avg_latency = float(lines[24 + bias].split(';')[1].strip().split()[2])
    sd_read_p99 = float(lines[24 + bias].split(';')[2].strip().split()[8])

    sd_write_nums = int(lines[25 + bias].split(';')[0].strip().split()[2])
    sd_write_avg_latency = float(lines[25 + bias].split(';')[1].strip().split()[2])
    sd_write_p99 = float(lines[25 + bias].split(';')[2].strip().split()[8])

    time_format = "%Y/%m/%d %H:%M:%S"

    time_begin = lines[14 + bias].split('to')[0].split('From')[1].strip()
    time_begin = str_to_datetime(time_begin, time_format)

    time_end = lines[14 + bias].split('to')[1].strip()
    time_end = str_to_datetime(time_end, time_format)

    # print(time_begin, time_end)

    return total_time, \
           p99, avg_latency, time_begin, time_end, bandwidth, \
           emmc_read_nums, emmc_read_avg_latency, emmc_read_p99, \
           emmc_write_nums, emmc_write_avg_latency, emmc_write_p99, \
           sd_read_nums, sd_read_avg_latency, sd_read_p99, \
           sd_write_nums, sd_write_avg_latency, sd_write_p99


def reset_folder_list():
    global workload_type_list
    global operation_read_ratio_list
    global block_size_list
    workload_type_list = []
    operation_read_ratio_list = []
    block_size_list = []


path_head = ''
log_dir = ''
file_cpu_usage_path = ''
file_mem_used_path = ''
file_disk_path = ''
file_power_path = ''

dir_log_name = 'log'
file_cpu_usage_name = 'cpu_usage.log'
file_mem_used_name = 'mem_used.log'
file_disk_name = 'disk_read_wrtn.log'
file_power_name = 'power.xlsx'
file_statistic_name = 'statistic.txt'
file_result_name = 'statistic.xlsx'

workload_type_list = []
operation_read_ratio_list = []
block_size_list = []

PRINT_INFO = False
PROCESS_POWER = True # TODO::NotCompleted
