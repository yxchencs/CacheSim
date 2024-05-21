import os
import pandas as pd
from datetime import datetime, timedelta
from openpyxl.styles import Font, Alignment
from openpyxl.utils.dataframe import dataframe_to_rows
from util import *

def str_to_datetime(raw_str, format):
    return datetime.strptime(raw_str, format)


def extract_statistic_device_test(filename, rdwr_only):
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
           sd_write_nums, sd_write_avg_latency, sd_write_p99 \

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

            if time_end > time > time_begin:  # 不取到等号以避免数据突变带来的数据异常
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


def resetPath(power_on):
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

    if power_on:
        file_power_path = os.path.join(log_dir, file_power_name)
        convertPowerFromTxt2Xlsx()
        os.chmod(file_power_path, 0o666)

    print('done reset path')


# trace_name/1GB/uniform/read_1/4KB/io_on/0.1/2q
def get_folder_list():
    global disk_size_list
    global workload_type_list
    global operation_read_ratio_list
    global block_size_list
    global io_list
    global cache_size_list
    global cache_policy_list

    # disk_size
    for folder1 in os.listdir(path_head):
        sub_dir1 = os.path.join(path_head, folder1)
        if os.path.isdir(sub_dir1) and folder1 != 'log':
            # workload_type
            for folder2 in os.listdir(sub_dir1):
                sub_dir2 = os.path.join(sub_dir1, folder2)
                if os.path.isdir(sub_dir2):
                    # operation_read_ratio
                    for folder3 in os.listdir(sub_dir2):
                        sub_dir3 = os.path.join(sub_dir2, folder3)
                        if os.path.isdir(sub_dir3):
                            # block_size
                            for folder4 in os.listdir(sub_dir3):
                                sub_dir4 = os.path.join(sub_dir3, folder4)
                                if os.path.isdir(sub_dir4):
                                    # io
                                    for folder5 in os.listdir(sub_dir4):
                                        sub_dir5 = os.path.join(sub_dir4, folder5)
                                        if os.path.isdir(sub_dir5):
                                            # cache_size
                                            for folder6 in os.listdir(sub_dir5):
                                                sub_dir6 = os.path.join(sub_dir5, folder6)
                                                if os.path.isdir(sub_dir6):
                                                    # cache_policy
                                                    for folder7 in os.listdir(sub_dir6):
                                                        sub_dir7 = os.path.join(sub_dir6, folder7)
                                                        if os.path.isdir(sub_dir7):
                                                            disk_size_list.append(folder1)
                                                            workload_type_list.append(folder2)
                                                            operation_read_ratio_list.append(folder3)
                                                            block_size_list.append(folder4)
                                                            io_list.append(folder5)
                                                            cache_size_list.append(folder6)
                                                            cache_policy_list.append(folder7)

    if PRINT_INFO:
        print(f"disk_size_list({len(disk_size_list)}):{disk_size_list}")
        print(f"workload_type_list({len(workload_type_list)}):{workload_type_list}")
        print(f"operation_read_ratio_list({len(operation_read_ratio_list)}):{operation_read_ratio_list}")
        print(f"block_size_list({len(block_size_list)}):{block_size_list}")
        print(f"io_list({len(io_list)}):{io_list}")
        print(f"cache_size_list({len(cache_size_list)}):{cache_size_list}")
        print(f"cache_policy_list({len(cache_policy_list)}):{cache_policy_list}")

    print('done get folders')


def process_results():
    global disk_size_list
    global workload_type_list
    global operation_read_ratio_list
    global block_size_list
    global io_list
    global cache_size_list
    global cache_policy_list

    list_total, list_p99, list_avg_latency = [], [], []
    list_cpu_usage, list_mem_used, list_emmc_kb_read, list_emmc_kb_wrtn, list_sd_kb_read, list_sd_kb_wrtn = [], [], [], [], [], []
    list_time_begin, list_time_end = [], []
    list_bandwidth = []
    list_avg_power, list_energy = [], []
    list_emmc_read_nums, list_emmc_read_avg_latency, list_emmc_read_p99 = [], [], []
    list_emmc_write_nums, list_emmc_write_avg_latency, list_emmc_write_p99 = [], [], []
    list_sd_read_nums, list_sd_read_avg_latency, list_sd_read_p99 = [], [], []
    list_sd_write_nums, list_sd_write_avg_latency, list_sd_write_p99 = [], [], []

    for i in range(len(operation_read_ratio_list)):
        disk_size = disk_size_list[i]
        workload_type = workload_type_list[i]
        operation_read_ratio = operation_read_ratio_list[i]
        block_size = block_size_list[i]
        io = io_list[i]
        cache_size = cache_size_list[i]
        cache_policy = cache_policy_list[i]

        file_path_begin = os.path.join(path_head, disk_size, workload_type, operation_read_ratio,
                                       block_size, io, cache_size, cache_policy)
        print('data process:', file_path_begin)

        rdwr_only = False
        if operation_read_ratio == 'read_1' or operation_read_ratio == 'read_0':
            rdwr_only = True
        file_statistic_path = os.path.join(file_path_begin, file_statistic_name)

        total, p99, avg_latency, \
        time_begin, time_end, bandwidth, \
        emmc_read_nums, emmc_read_avg_latency, emmc_read_p99, \
        emmc_write_nums, emmc_write_avg_latency, emmc_write_p99, \
        sd_read_nums, sd_read_avg_latency, sd_read_p99, \
        sd_write_nums, sd_write_avg_latency, sd_write_p99 = \
            extract_statistic_device_test(
                file_statistic_path,
                rdwr_only)

        list_cpu_usage.append(
            calculate_avg_cpu_usage(file_cpu_usage_path, time_begin,
                                    time_end - timedelta(seconds=1)))
        list_mem_used.append(calculate_avg_mem_used(file_mem_used_path, time_begin, time_end))
        emmc_kb_read, emmc_kb_wrtn, sd_kb_read, sd_kb_wrtn \
            = calculate_disk_read_wrtn(file_disk_path, time_begin, time_end)

        list_avg_power.append(calculate_avg_power(file_power_path, time_begin, time_end))

        list_emmc_kb_read.append(emmc_kb_read)
        list_emmc_kb_wrtn.append(emmc_kb_wrtn)
        list_sd_kb_read.append(sd_kb_read)
        list_sd_kb_wrtn.append(sd_kb_wrtn)

        list_total.append(total)
        list_p99.append(p99)
        list_avg_latency.append(avg_latency)

        list_time_begin.append(time_begin)
        list_time_end.append(time_end)

        list_bandwidth.append(bandwidth)

        list_emmc_read_nums.append(emmc_read_nums)
        list_emmc_read_avg_latency.append(emmc_read_avg_latency)
        list_emmc_read_p99.append(emmc_read_p99)
        list_emmc_write_nums.append(emmc_write_nums)
        list_emmc_write_avg_latency.append(emmc_write_avg_latency)
        list_emmc_write_p99.append(emmc_write_p99)
        list_sd_read_nums.append(sd_read_nums)
        list_sd_read_avg_latency.append(sd_read_avg_latency)
        list_sd_read_p99.append(sd_read_p99)
        list_sd_write_nums.append(sd_write_nums)
        list_sd_write_avg_latency.append(sd_write_avg_latency)
        list_sd_write_p99.append(sd_write_p99)

    list_disk_size = [convert_to_numeric(x) for x in disk_size_list]
    list_block_size = [convert_to_numeric(x) for x in block_size_list]
    list_operation_read_ratio = [convert_to_numeric(x) for x in operation_read_ratio_list]
    list_energy = [a * b if a is not None and b is not None else None for a, b in zip(list_avg_power, list_total)]
    list_emmc_kb_read = [x / 1024 for x in list_emmc_kb_read]
    list_emmc_kb_wrtn = [x / 1024 for x in list_emmc_kb_wrtn]
    list_sd_kb_read = [x / 1024 for x in list_sd_kb_read]
    list_sd_kb_wrtn = [x / 1024 for x in list_sd_kb_wrtn]

    data = {'Disk Size(GB)':list_disk_size,
            'Workload Type':workload_type_list,
            'Operation Read Ratio':list_operation_read_ratio,
            'Block Size(KB)':list_block_size,
            'IO(on/off)':io_list,
            'Cache Size':cache_size_list,
            'Caching Policy':cache_policy_list,
            'Average Latency(ms)': list_avg_latency, 'P99 Latency(ms)': list_p99,
            'Average CPU Usage(%)': list_cpu_usage, 'Average Memory Used(MB)': list_mem_used,
            'Total Time(s)': list_total, "Bandwidth(MB/s)": list_bandwidth,
            'eMMC Read Size(KB)':list_emmc_kb_read, 'eMMC Write Size(KB)':list_emmc_kb_wrtn,
            'SD Read Size(KB)':list_sd_kb_read, 'SD Write Size(KB)':list_sd_kb_wrtn,
            'Average Power(W)': list_avg_power, 'Energy(J)': list_energy,
            'eMMC Read Numbers': list_emmc_read_nums, 'eMMC Read Average Latency(ms)': list_emmc_read_avg_latency,
            'eMMC Read P99 Latency(ms)': list_emmc_read_p99,
            'eMMC Write Numbers': list_emmc_write_nums, 'eMMC Write Average Latency(ms)': list_emmc_write_avg_latency,
            'eMMC Write P99 Latency(ms)': list_emmc_write_p99,
            'SD Read Numbers': list_sd_read_nums, 'SD Read Average Latency(ms)': list_sd_read_avg_latency,
            'SD Read P99 Latency(ms)': list_sd_read_p99,
            'SD Write Numbers': list_sd_write_nums, 'SD Write Average Latency(ms)': list_sd_write_avg_latency,
            'SD Write P99 Latency(ms)': list_sd_write_p99}



    df = pd.DataFrame(data)
    excel_file = path_head + 'statistic.xlsx'
    df.to_excel(excel_file, index=False)
    print(f"data save in {excel_file}")


def convertPowerFromTxt2Xlsx():
    global log_dir
    global file_power_path
    df = pd.read_csv(os.path.join(log_dir, "Umeter.txt"), sep="\s+", header=None,
                     names=["Time(HH:MM:SS)", "Voltage(V)", "Current(A)", "D+(V)", "D-(V)"], skiprows=1, encoding='GBK')
    df.drop(columns=["D+(V)", "D-(V)"], inplace=True)
    df["Power(W)"] = df["Voltage(V)"] * df["Current(A)"]

    df.to_excel(file_power_path, index=False)
    print(f'done convert {file_power_path}')


def process_cache_test():
    resetPath(True)
    get_folder_list()
    process_results()


dir_log_name = 'log'
file_cpu_usage_name = 'cpu_usage.log'
file_mem_used_name = 'mem_used.log'
file_disk_name = 'disk_read_wrtn.log'
file_power_name = 'power.xlsx'
file_statistic_name = 'statistic.txt'

disk_size_list = []
workload_type_list = []
operation_read_ratio_list = []
block_size_list = []
io_list = []
cache_size_list = []
cache_policy_list = []

path_head = ''
log_dir = ''
file_cpu_usage_path = ''
file_mem_used_path = ''
file_disk_path = ''
file_power_path = ''

PRINT_INFO = False

if __name__ == '__main__':
    path_head = 'E:/projects/records/2024-05-19_17-45-38_read_0.6/'
    # path_head = 'E:/projects/records/test/'
    process_cache_test()

