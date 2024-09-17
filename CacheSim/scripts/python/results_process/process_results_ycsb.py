import os
import pandas as pd
import util2
import tqdm


# 1GB/uniform/read_1/4KB/io_on/0.1/2q
def get_folder_list():
    global disk_size_list
    global io_list
    global cache_size_list
    global cache_policy_list

    # disk_size
    for folder1 in os.listdir(util2.path_head):
        sub_dir1 = os.path.join(util2.path_head, folder1)
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
                                                            util2.workload_type_list.append(folder2)
                                                            util2.operation_read_ratio_list.append(folder3)
                                                            util2.block_size_list.append(folder4)
                                                            io_list.append(folder5)
                                                            cache_size_list.append(folder6)
                                                            cache_policy_list.append(folder7)

    if util2.PRINT_INFO:
        print(f"disk_size_list({len(disk_size_list)}):{disk_size_list}")
        print(f"workload_type_list({len(util2.workload_type_list)}):{util2.workload_type_list}")
        print(f"operation_read_ratio_list({len(util2.operation_read_ratio_list)}):{util2.operation_read_ratio_list}")
        print(f"block_size_list({len(util2.block_size_list)}):{util2.block_size_list}")
        print(f"io_list({len(io_list)}):{io_list}")
        print(f"cache_size_list({len(cache_size_list)}):{cache_size_list}")
        print(f"cache_policy_list({len(cache_policy_list)}):{cache_policy_list}")

    print('done get folders')


def process_results():
    global disk_size_list
    global io_list
    global cache_size_list
    global cache_policy_list

    list_hit_ratio, list_total, list_p99, list_avg_latency = [], [], [], []
    list_cpu_usage, list_mem_used, list_emmc_kb_read, list_emmc_kb_wrtn, list_sd_kb_read, list_sd_kb_wrtn = [], [], [], [], [], []
    list_time_begin, list_time_end = [], []
    list_bandwidth = []
    list_avg_power, list_energy = [], []
    list_emmc_read_nums, list_emmc_read_avg_latency, list_emmc_read_p99 = [], [], []
    list_emmc_write_nums, list_emmc_write_avg_latency, list_emmc_write_p99 = [], [], []
    list_sd_read_nums, list_sd_read_avg_latency, list_sd_read_p99 = [], [], []
    list_sd_write_nums, list_sd_write_avg_latency, list_sd_write_p99 = [], [], []
    list_total_time_32gb, list_energy_32gb = [], []

    for i in tqdm.trange(len(util2.operation_read_ratio_list)):
        disk_size = disk_size_list[i]
        disk_size_unit = disk_size[-2:]
        workload_type = util2.workload_type_list[i]
        operation_read_ratio = util2.operation_read_ratio_list[i]
        block_size = util2.block_size_list[i]
        io = io_list[i]
        cache_size = cache_size_list[i]
        cache_policy = cache_policy_list[i]

        file_path_begin = os.path.join(util2.path_head, disk_size, workload_type, operation_read_ratio,
                                       block_size, io, cache_size, cache_policy)

        # print(file_path_begin)
        if util2.PRINT_INFO:
            print('data process:', file_path_begin)

        rdwr_only = False
        if operation_read_ratio == 'read_1' or operation_read_ratio == 'read_0':
            rdwr_only = True
        file_statistic_path = os.path.join(file_path_begin, util2.file_statistic_name)

        hit_ratio, total, p99, avg_latency, \
        time_begin, time_end, bandwidth, \
        emmc_read_nums, emmc_read_avg_latency, emmc_read_p99, \
        emmc_write_nums, emmc_write_avg_latency, emmc_write_p99, \
        sd_read_nums, sd_read_avg_latency, sd_read_p99, \
        sd_write_nums, sd_write_avg_latency, sd_write_p99 = \
            util2.extract_statistic(
                file_statistic_path,
                rdwr_only)

        total_time_32gb = 32 * 1024 / bandwidth
        avg_power = util2.calculate_avg_power(util2.file_power_path, time_begin, time_end)
        if avg_power is not None:
            energy_32gb = total_time_32gb * avg_power
        else:
            energy_32gb = None

        list_total_time_32gb.append(total_time_32gb)
        list_energy_32gb.append(energy_32gb)

        list_cpu_usage.append(
            util2.calculate_avg_cpu_usage(util2.file_cpu_usage_path, time_begin, time_end))
        list_mem_used.append(util2.calculate_avg_mem_used(util2.file_mem_used_path, time_begin, time_end))
        emmc_kb_read, emmc_kb_wrtn, sd_kb_read, sd_kb_wrtn \
            = util2.calculate_disk_read_wrtn(util2.file_disk_path, time_begin, time_end)

        list_avg_power.append(avg_power)

        list_emmc_kb_read.append(emmc_kb_read)
        list_emmc_kb_wrtn.append(emmc_kb_wrtn)
        list_sd_kb_read.append(sd_kb_read)
        list_sd_kb_wrtn.append(sd_kb_wrtn)

        list_hit_ratio.append(hit_ratio)
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

    list_cache_policy = [util2.convert_cache_policy(x) for x in cache_policy_list]
    list_disk_size = [util2.convert_to_numeric(x) for x in disk_size_list]
    list_block_size = [util2.convert_to_numeric(x) for x in util2.block_size_list]
    list_operation_read_ratio = [util2.convert_to_numeric(x) for x in util2.operation_read_ratio_list]
    # list_energy = [a * b if a is not None and b is not None else None for a, b in zip(list_avg_power, list_total)]
    list_emmc_kb_read = [x / 1024 for x in list_emmc_kb_read]
    list_emmc_kb_wrtn = [x / 1024 for x in list_emmc_kb_wrtn]
    list_sd_kb_read = [x / 1024 for x in list_sd_kb_read]
    list_sd_kb_wrtn = [x / 1024 for x in list_sd_kb_wrtn]

    data = {f'Disk Size({disk_size_unit})': list_disk_size,
            'Workload Type': util2.workload_type_list,
            'Operation Read Ratio': list_operation_read_ratio,
            'Block Size(KB)': list_block_size,
            'IO(on/off)': io_list,
            'Cache Size': cache_size_list,
            'Caching Policy': list_cache_policy,
            'Hit Ratio': list_hit_ratio,
            'P99 Latency(ms)': list_p99, 'Average Latency(ms)': list_avg_latency,
            # 'Total Time(s)': list_total, # Origin Total Time
            "Bandwidth(MB/s)": list_bandwidth,
            'Average CPU Usage(%)': list_cpu_usage, 'Average Memory Used(MB)': list_mem_used,
            'Average Power(W)': list_avg_power,  # Origin Energy
            # 'Energy(J)': list_energy,
            # 'Total Time(s)[32GB]': list_total_time_32gb, # Total Time For 32GB
            'Energy(J)[32GB]': list_energy_32gb,  # Energy For 32GB
            'eMMC Read Size(KB)': list_emmc_kb_read, 'eMMC Write Size(KB)': list_emmc_kb_wrtn,
            'SD Read Size(KB)': list_sd_kb_read, 'SD Write Size(KB)': list_sd_kb_wrtn,
            'eMMC Read Numbers': list_emmc_read_nums, 'eMMC Read Average Latency(ms)': list_emmc_read_avg_latency,
            'eMMC Read P99 Latency(ms)': list_emmc_read_p99,
            'eMMC Write Numbers': list_emmc_write_nums, 'eMMC Write Average Latency(ms)': list_emmc_write_avg_latency,
            'eMMC Write P99 Latency(ms)': list_emmc_write_p99,
            'SD Read Numbers': list_sd_read_nums, 'SD Read Average Latency(ms)': list_sd_read_avg_latency,
            'SD Read P99 Latency(ms)': list_sd_read_p99,
            'SD Write Numbers': list_sd_write_nums, 'SD Write Average Latency(ms)': list_sd_write_avg_latency,
            'SD Write P99 Latency(ms)': list_sd_write_p99,
            'Time Begin': list_time_begin, 'Time End': list_time_end}

    df = pd.DataFrame(data)
    excel_file = os.path.join(util2.path_head, 'statistic.xlsx')
    df.to_excel(excel_file, index=False)
    print(f"data save in {excel_file}")


def process_cache_test():
    util2.reset_path()
    get_folder_list()
    process_results()


disk_size_list = []
io_list = []
cache_size_list = []
cache_policy_list = []

if __name__ == '__main__':
    path_root = 'D:/Projects/Caching-Policy/records'
    folder_list = ['2024-08-07_11-32-09_cache_ycsb_latest_read_0.6_1KB_io_on_0.1_random','2024-08-06_00-26-32_cache_ycsb_latest_read_0.6_4-1024KB',
                   '2024-08-06_23-02-27_cache_ycsb_latest_read_0.6_1KB_io_on_0.1']
    # for folder in folder_list:
    #     util2.path_head = os.path.join(path_root, folder)
    #     print(util2.path_head)
    #     process_cache_test()
    util2.merge_excel_files(path_root, folder_list)
