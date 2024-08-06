import os
import pandas as pd
import util2
import tqdm


# 1GB/uniform/read_1/4KB/emmc
def get_folder_list():
    global disk_size_list
    global device_id_list

    for folder1 in os.listdir(util2.path_head):
        sub_dir1 = os.path.join(util2.path_head, folder1)
        if os.path.isdir(sub_dir1) and folder1 != 'log':
            for folder2 in os.listdir(sub_dir1):
                sub_dir2 = os.path.join(sub_dir1, folder2)
                if os.path.isdir(sub_dir2):
                    for folder3 in os.listdir(sub_dir2):
                        sub_dir3 = os.path.join(sub_dir2, folder3)
                        if os.path.isdir(sub_dir3):
                            for folder4 in os.listdir(sub_dir3):
                                sub_dir4 = os.path.join(sub_dir3, folder4)
                                if os.path.isdir(sub_dir4):
                                    for folder5 in os.listdir(sub_dir4):
                                        sub_dir5 = os.path.join(sub_dir4, folder5)
                                        if os.path.isdir(sub_dir5):
                                            disk_size_list.append(folder1)
                                            util2.workload_type_list.append(folder2)
                                            util2.operation_read_ratio_list.append(folder3)
                                            util2.block_size_list.append(folder4)
                                            device_id_list.append(folder5)
    if util2.PRINT_INFO:
        print(f"disk_size_list({len(disk_size_list)}):{disk_size_list}")
        print(f"workload_type_list({len(util2.workload_type_list)}):{util2.workload_type_list}")
        print(
            f"util2.operation_read_ratio_list({len(util2.operation_read_ratio_list)}):{util2.operation_read_ratio_list}")
        print(f"util2.block_size_list({len(util2.block_size_list)}):{util2.block_size_list}")
        print(f"device_id_list({len(device_id_list)}):{device_id_list}")
    print('done get folders')


def process_results_for_device_test():
    global disk_size_list
    global device_id_list

    list_total, list_p99, list_avg_latency = [], [], []
    list_cpu_usage, list_mem_used, list_emmc_kb_read, list_emmc_kb_wrtn, list_sd_kb_read, list_sd_kb_wrtn = [], [], [], [], [], []
    list_time_begin, list_time_end = [], []
    list_bandwidth = []
    list_avg_power, list_energy = [], []
    list_emmc_read_nums, list_emmc_read_avg_latency, list_emmc_read_p99 = [], [], []
    list_emmc_write_nums, list_emmc_write_avg_latency, list_emmc_write_p99 = [], [], []
    list_sd_read_nums, list_sd_read_avg_latency, list_sd_read_p99 = [], [], []
    list_sd_write_nums, list_sd_write_avg_latency, list_sd_write_p99 = [], [], []

    for i in tqdm.trange(len(disk_size_list)):
        disk_size = disk_size_list[i]
        disk_size_unit = disk_size[-2:]
        workload_type = util2.workload_type_list[i]
        operation_read_ratio = util2.operation_read_ratio_list[i]
        block_size = util2.block_size_list[i]
        device_id = device_id_list[i]

        file_path_begin = os.path.join(util2.path_head, disk_size, workload_type, operation_read_ratio, block_size,
                                       device_id)
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

        list_cpu_usage.append(
            util2.calculate_avg_cpu_usage(util2.file_cpu_usage_path, time_begin, time_end))
        list_mem_used.append(util2.calculate_avg_mem_used(util2.file_mem_used_path, time_begin, time_end))
        emmc_kb_read, emmc_kb_wrtn, sd_kb_read, sd_kb_wrtn \
            = util2.calculate_disk_read_wrtn(util2.file_disk_path, time_begin, time_end)

        list_avg_power.append(util2.calculate_avg_power(util2.file_power_path, time_begin, time_end))

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

    list_disk_size = [util2.convert_to_numeric(x) for x in disk_size_list]
    list_block_size = [util2.convert_to_numeric(x) for x in util2.block_size_list]
    list_operation_read_ratio = [util2.convert_to_numeric(x) for x in util2.operation_read_ratio_list]
    list_energy = [a * b if a is not None and b is not None else None for a, b in zip(list_avg_power, list_total)]
    list_emmc_kb_read = [x / 1024 for x in list_emmc_kb_read]
    list_emmc_kb_wrtn = [x / 1024 for x in list_emmc_kb_wrtn]
    list_sd_kb_read = [x / 1024 for x in list_sd_kb_read]
    list_sd_kb_wrtn = [x / 1024 for x in list_sd_kb_wrtn]
    dict_device_id = {'sd': 'SD', 'emmc': "eMMC"}
    device_id_list = [dict_device_id[x] for x in device_id_list]

    data = {f'Disk Size({disk_size_unit})': list_disk_size,
            'Workload Type': util2.workload_type_list,
            'Operation Read Ratio': list_operation_read_ratio,
            'Block Size(KB)': list_block_size,
            'Device ID': device_id_list,
            'P99 Latency(ms)': list_p99, 'Average Latency(ms)': list_avg_latency,
            'Total Time(s)': list_total, "Bandwidth(MB/s)": list_bandwidth,
            'Average CPU Usage(%)': list_cpu_usage, 'Average Memory Used(MB)': list_mem_used,
            'eMMC Read Size(KB)': list_emmc_kb_read, 'eMMC Write Size(KB)': list_emmc_kb_wrtn,
            'SD Read Size(KB)': list_sd_kb_read, 'SD Write Size(KB)': list_sd_kb_wrtn,
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
    excel_file = os.path.join(util2.path_head, 'statistic.xlsx')
    df.to_excel(excel_file, index=False)
    print(f"data save in {excel_file}")


def process_device_test():
    util2.reset_path()
    get_folder_list()
    process_results_for_device_test()


disk_size_list = []
device_id_list = []

if __name__ == '__main__':
    util2.path_head = 'D:/projects/Caching-Policy/records/2024-07-11_00-15-30_device_2GB_ycsb_all'
    process_device_test()
