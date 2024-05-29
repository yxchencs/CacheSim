import os
import pandas as pd
import util
import tqdm


# 1GB/uniform/read_1/4KB/emmc
def get_folder_list():
    global device_id_list

    for folder1 in os.listdir(util.path_head):
        sub_dir1 = os.path.join(util.path_head, folder1)
        if os.path.isdir(sub_dir1) and folder1 != 'log':
            for folder2 in os.listdir(sub_dir1):
                sub_dir2 = os.path.join(sub_dir1, folder2)
                if os.path.isdir(sub_dir2):
                    for folder3 in os.listdir(sub_dir2):
                        sub_dir3 = os.path.join(sub_dir2, folder3)
                        if os.path.isdir(sub_dir3):
                            device_id_list.append(folder2)
                            util.operation_read_ratio_list.append(folder1)
                            util.block_size_list.append(folder3)

    # print(f"util.operation_read_ratio_list({len(util.operation_read_ratio_list)}):{util.operation_read_ratio_list}")
    # print(f"device_id_list({len(device_id_list)}):{device_id_list}")
    # print(f"util.block_size_list({len(util.block_size_list)}):{util.block_size_list}")
    print('done get folders')


def process_results_for_device_test():
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


    for i in tqdm.trange(len(util.operation_read_ratio_list)):
        operation_read_ratio = util.operation_read_ratio_list[i]
        device_id = device_id_list[i]
        block_size = util.block_size_list[i]

        file_path_begin = os.path.join(util.path_head, operation_read_ratio, device_id, block_size)
        print('data process:', file_path_begin)

        rdwr_only = False
        if operation_read_ratio == 'read_1' or operation_read_ratio == 'read_0':
            rdwr_only = True
        file_statistic_path = os.path.join(file_path_begin, util.file_statistic_name)

        hit_ratio, total, p99, avg_latency, \
        time_begin, time_end, bandwidth, \
        emmc_read_nums, emmc_read_avg_latency, emmc_read_p99, \
        emmc_write_nums, emmc_write_avg_latency, emmc_write_p99, \
        sd_read_nums, sd_read_avg_latency, sd_read_p99, \
        sd_write_nums, sd_write_avg_latency, sd_write_p99 = \
            util.extract_statistic(
                file_statistic_path,
                rdwr_only)

        list_cpu_usage.append(
            util.calculate_avg_cpu_usage(util.file_cpu_usage_path, time_begin, time_end))
        list_mem_used.append(util.calculate_avg_mem_used(util.file_mem_used_path, time_begin, time_end))
        emmc_kb_read, emmc_kb_wrtn, sd_kb_read, sd_kb_wrtn \
            = util.calculate_disk_read_wrtn(util.file_disk_path, time_begin, time_end)

        list_avg_power.append(util.calculate_avg_power(util.file_power_path, time_begin, time_end))

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

    list_energy = [a * b if a is not None and b is not None else None for a, b in zip(list_avg_power, list_total)]
    list_emmc_kb_read = [x / 1024 for x in list_emmc_kb_read]
    list_emmc_kb_wrtn = [x / 1024 for x in list_emmc_kb_wrtn]
    list_sd_kb_read = [x / 1024 for x in list_sd_kb_read]
    list_sd_kb_wrtn = [x / 1024 for x in list_sd_kb_wrtn]
    dict_device_id = {'sd': 'SD', 'emmc': "eMMC"}
    device_id_list = [dict_device_id[x] for x in device_id_list]

    data = {'Operation Read Ratio': util.operation_read_ratio_list, 'Device ID': device_id_list,
            'Block Size(KB)': util.block_size_list,
            'Average Latency(ms)': list_avg_latency, 'P99 Latency(ms)': list_p99,
            'Average CPU Usage(%)': list_cpu_usage, 'Average Memory Used(MB)': list_mem_used,
            'Total Time(s)': list_total, "Bandwidth(MB/s)": list_bandwidth,
            'Average Power(W)': list_avg_power, 'Energy(J)': list_energy,
            'eMMC Read Size(KB)': list_emmc_kb_read, 'eMMC Write Size(KB)': list_emmc_kb_wrtn,
            'SD Read Size(KB)': list_sd_kb_read, 'SD Write Size(KB)': list_sd_kb_wrtn,
            'eMMC Read Numbers': list_emmc_read_nums, 'eMMC Read Average Latency(ms)': list_emmc_read_avg_latency,
            'eMMC Read P99 Latency(ms)': list_emmc_read_p99,
            'eMMC Write Numbers': list_emmc_write_nums, 'eMMC Write Average Latency(ms)': list_emmc_write_avg_latency,
            'eMMC Write P99 Latency(ms)': list_emmc_write_p99,
            'SD Read Numbers': list_sd_read_nums, 'SD Read Average Latency(ms)': list_sd_read_avg_latency,
            'SD Read P99 Latency(ms)': list_sd_read_p99,
            'SD Write Numbers': list_sd_write_nums, 'SD Write Average Latency(ms)': list_sd_write_avg_latency,
            'SD Write P99 Latency(ms)': list_sd_write_p99}

    df = pd.DataFrame(data)
    excel_file = os.path.join(util.path_head, 'statistic.xlsx')
    df.to_excel(excel_file, index=False)
    print(f"data save in {excel_file}")


def process_device_test():
    util.reset_path()
    get_folder_list()
    process_results_for_device_test()


device_id_list = []

if __name__ == '__main__':
    util.path_head = 'E:/records/2024-05-07_01-23-13_device_test/'
    process_device_test()
