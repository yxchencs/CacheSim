import pandas as pd
import os
from datetime import datetime, timedelta
from concurrent.futures import ThreadPoolExecutor


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

    total_cnt = len(folder_list)
    combine_cnt = 0
    # 遍历文件夹并读取每个文件
    for folder in folder_list:
        file_path = os.path.join(path_dir, folder, file_result_name)
        if os.path.exists(file_path):
            df = pd.read_excel(file_path)
            combined_df = combined_df.append(df, ignore_index=True)
            combine_cnt = combine_cnt + 1
            print(f'Combine {file_path}')
        else:
            print(f'Not exist {file_path}')

    # 保存合并后的 DataFrame 到新文件
    save_dir = os.path.join(path_dir, file_result_name)
    combined_df.to_excel(save_dir, index=False)
    print(f"Combine {combine_cnt}/{total_cnt} files successfully into", save_dir)


def str_to_datetime(raw_str, format):
    return datetime.strptime(raw_str, format)


def calculate_avg_cpu_usage(file_path, time_begin, time_end):
    time_format = "%Y-%m-%d %H:%M:%S"
    cpu_data = pd.read_csv(file_path, sep=' ', header=None, names=['cpu', 'date', 'time'])
    cpu_data['datetime'] = pd.to_datetime(cpu_data['date'] + ' ' + cpu_data['time'], format=time_format)
    filter_data = cpu_data[(cpu_data['datetime'] >= time_begin) & (cpu_data['datetime'] <= time_end)]
    if filter_data.empty:
        filter_data = cpu_data[(cpu_data['datetime'] >= time_begin - delta_time) & (cpu_data['datetime'] <= time_end)]
        if filter_data.empty:
            filter_data = cpu_data[
                (cpu_data['datetime'] >= time_begin) & (cpu_data['datetime'] <= time_end + delta_time)]
            if filter_data.empty:
                filter_data = cpu_data[
                    (cpu_data['datetime'] >= time_begin - delta_time) & (cpu_data['datetime'] <= time_end + delta_time)]
    if not filter_data.empty:
        filter_data.loc[:, 'cpu'] = filter_data['cpu'].str.replace('%', '').astype(float)
        return filter_data['cpu'].mean()
    else:
        return None


def calculate_avg_power(file_path, time_begin, time_end):
    time_format = "%H:%M:%S"
    # 读取 Excel 文件
    df = pd.read_excel(file_path)

    # 将时间列转换为 datetime.time 对象
    df['Time(HH:MM:SS)'] = pd.to_datetime(df['Time(HH:MM:SS)'], format=time_format).dt.time

    # 将 time_begin 和 time_end 转换为 datetime.time 对象
    time_begin_obj = time_begin.time()
    time_end_obj = time_end.time()

    # 筛选在时间范围内的行
    mask = (df['Time(HH:MM:SS)'] >= time_begin_obj) & (df['Time(HH:MM:SS)'] <= time_end_obj)
    filtered_df = df.loc[mask]

    # 如果初始范围内没有数据，尝试扩展时间范围
    time_begin_extended = (time_begin - delta_time).time()
    time_end_extended = (time_end + delta_time).time()
    if filtered_df.empty:
        mask_extended = (df['Time(HH:MM:SS)'] >= time_begin_extended) & (df['Time(HH:MM:SS)'] <= time_end_obj)
        filtered_df = df.loc[mask_extended]

        if filtered_df.empty:
            mask_extended = (df['Time(HH:MM:SS)'] >= time_begin_obj) & (df['Time(HH:MM:SS)'] <= time_end_extended)
            filtered_df = df.loc[mask_extended]

            if filtered_df.empty:
                mask_extended = (df['Time(HH:MM:SS)'] >= time_begin_extended) & (
                            df['Time(HH:MM:SS)'] <= time_end_extended)
                filtered_df = df.loc[mask_extended]

    # 计算平均功率
    if not filtered_df.empty:
        avg_power = filtered_df['Power(W)'].mean()
        return avg_power
    else:
        return None


# [time_begin, time_end]时间段所有mem_used的平均值
def calculate_avg_mem_used(file_path, time_begin, time_end):
    time_format = "%Y-%m-%d %H:%M:%S"
    # 读取文件并解析日期时间
    mem_data = pd.read_csv(file_path, sep=' ', header=None, names=['mem', 'date', 'time'])
    mem_data['datetime'] = pd.to_datetime(mem_data['date'] + ' ' + mem_data['time'], format=time_format)

    # 尝试在初始时间范围内筛选数据
    filter_data = mem_data[(mem_data['datetime'] >= time_begin) & (mem_data['datetime'] <= time_end)]

    # 如果初始范围内没有数据，尝试扩展时间范围
    if filter_data.empty:
        filter_data = mem_data[
            (mem_data['datetime'] >= time_begin - delta_time) & (mem_data['datetime'] <= time_end)]
        if filter_data.empty:
            filter_data = mem_data[
                (mem_data['datetime'] >= time_begin) & (mem_data['datetime'] <= time_end + delta_time)]
            if filter_data.empty:
                filter_data = mem_data[
                    (mem_data['datetime'] >= time_begin - delta_time) & (mem_data['datetime'] <= time_end + delta_time)]

    if not filter_data.empty:
        filter_data.loc[:, 'mem'] = filter_data['mem'].astype(int)
        return filter_data['mem'].mean()
    else:
        return None

def calculate_disk_read_wrtn(file_path, time_begin, time_end, delta_time=timedelta(minutes=5)):
    # 定义时间格式
    time_format = "%Y-%m-%d %H:%M:%S"

    # 读取文件，解析数据
    data = pd.read_csv(file_path, sep=';', header=None, names=['mmcblk0', 'mmcblk1', 'timestamp'])
    data['timestamp'] = pd.to_datetime(data['timestamp'].str.strip(), format=time_format)

    # 提取 eMMC 和 SD 的读写数据
    data['emmc_kb_read'] = pd.to_numeric(data['mmcblk0'].str.extract(r'kB_read\s*=\s*(\d+)')[0], errors='coerce')
    data['emmc_kb_wrtn'] = pd.to_numeric(data['mmcblk0'].str.extract(r'kB_wrtn\s*=\s*(\d+)')[0], errors='coerce')
    data['sd_kb_read'] = pd.to_numeric(data['mmcblk1'].str.extract(r'kB_read\s*=\s*(\d+)')[0], errors='coerce')
    data['sd_kb_wrtn'] = pd.to_numeric(data['mmcblk1'].str.extract(r'kB_wrtn\s*=\s*(\d+)')[0], errors='coerce')

    # 转换为 float 再转换为 int，以处理大数值
    data['emmc_kb_read'] = data['emmc_kb_read'].astype(float).astype('Int64')
    data['emmc_kb_wrtn'] = data['emmc_kb_wrtn'].astype(float).astype('Int64')
    data['sd_kb_read'] = data['sd_kb_read'].astype(float).astype('Int64')
    data['sd_kb_wrtn'] = data['sd_kb_wrtn'].astype(float).astype('Int64')

    # 筛选出时间范围内的数据
    filter_data = data[(data['timestamp'] >= time_begin) & (data['timestamp'] <= time_end)]

    if filter_data.empty:
        filter_data = data[(data['timestamp'] >= time_begin - delta_time) & (data['timestamp'] <= time_end)]
        if filter_data.empty:
            filter_data = data[(data['timestamp'] >= time_begin) & (data['timestamp'] <= time_end + delta_time)]
            if filter_data.empty:
                filter_data = data[
                    (data['timestamp'] >= time_begin - delta_time) & (data['timestamp'] <= time_end + delta_time)]

    if not filter_data.empty:
        # 计算读写数据的差异
        emmc_kb_read_diff = filter_data['emmc_kb_read'].iloc[-1] - filter_data['emmc_kb_read'].iloc[0]
        emmc_kb_wrtn_diff = filter_data['emmc_kb_wrtn'].iloc[-1] - filter_data['emmc_kb_wrtn'].iloc[0]
        sd_kb_read_diff = filter_data['sd_kb_read'].iloc[-1] - filter_data['sd_kb_read'].iloc[0]
        sd_kb_wrtn_diff = filter_data['sd_kb_wrtn'].iloc[-1] - filter_data['sd_kb_wrtn'].iloc[0]
        return emmc_kb_read_diff, emmc_kb_wrtn_diff, sd_kb_read_diff, sd_kb_wrtn_diff
    else:
        return None, None, None, None

# IO Read/Write Mount
# def calculate_disk_read_wrtn(file_path, time_begin, time_end):
#     # 定义时间格式
#     time_format = "%Y-%m-%d %H:%M:%S"
#
#     # 读取文件，解析数据
#     data = pd.read_csv(file_path, sep=';', header=None, names=['mmcblk0', 'mmcblk1', 'timestamp'])
#     data['timestamp'] = pd.to_datetime(data['timestamp'].str.strip(), format=time_format)
#
#     # 提取 eMMC 和 SD 的读写数据
#     data['emmc_kb_read'] = data['mmcblk0'].str.extract(r'kB_read\s*=\s*(\d+)').astype(int)
#     data['emmc_kb_wrtn'] = data['mmcblk0'].str.extract(r'kB_wrtn\s*=\s*(\d+)').astype(int)
#     data['sd_kb_read'] = data['mmcblk1'].str.extract(r'kB_read\s*=\s*(\d+)').astype(int)
#     data['sd_kb_wrtn'] = data['mmcblk1'].str.extract(r'kB_wrtn\s*=\s*(\d+)').astype(int)
#
#     # 筛选出时间范围内的数据
#     filter_data = data[(data['timestamp'] >= time_begin) & (data['timestamp'] <= time_end)]
#
#     if filter_data.empty:
#         filter_data = data[(data['timestamp'] >= time_begin - delta_time) & (data['timestamp'] <= time_end)]
#         if filter_data.empty:
#             filter_data = data[(data['timestamp'] >= time_begin) & (data['timestamp'] <= time_end + delta_time)]
#             if filter_data.empty:
#                 filter_data = data[
#                     (data['timestamp'] >= time_begin - delta_time) & (data['timestamp'] <= time_end + delta_time)]
#
#     if not filter_data.empty:
#         # 计算读写数据的差异
#         emmc_kb_read_diff = filter_data['emmc_kb_read'].iloc[-1] - filter_data['emmc_kb_read'].iloc[0]
#         emmc_kb_wrtn_diff = filter_data['emmc_kb_wrtn'].iloc[-1] - filter_data['emmc_kb_wrtn'].iloc[0]
#         sd_kb_read_diff = filter_data['sd_kb_read'].iloc[-1] - filter_data['sd_kb_read'].iloc[0]
#         sd_kb_wrtn_diff = filter_data['sd_kb_wrtn'].iloc[-1] - filter_data['sd_kb_wrtn'].iloc[0]
#         return emmc_kb_read_diff, emmc_kb_wrtn_diff, sd_kb_read_diff, sd_kb_wrtn_diff
#     else:
#         return None, None, None, None


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
    hit_ratio = 0
    if len(lines[13 + bias].split()) > 3:
        hit_ratio = float(lines[13 + bias].split()[3])
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

    return hit_ratio, total_time, \
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
delta_time = timedelta(seconds=1)

workload_type_list = []
operation_read_ratio_list = []
block_size_list = []

PRINT_INFO = False
PROCESS_POWER = True  # TODO::NotCompleted
