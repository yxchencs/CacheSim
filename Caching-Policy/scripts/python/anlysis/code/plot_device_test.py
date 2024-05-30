import pandas as pd
import matplotlib.pyplot as plt
import os

data_dir = '../data/statistic'
result_dir = '../result'
device_id_list = ['eMMC', 'SD']
operation_read_ratio_list = ['read_0', 'read_1']

# 设置全局字体
plt.rcParams['font.family'] = 'Times New Roman'

# 读取Excel文件
original_data = pd.read_excel(os.path.join(data_dir, 'device_test_5GB_uniform_statistic.xlsx'))

# 设置图表的大小和风格
plt.rcParams["figure.figsize"] = (12, 6)

# 定义Block Size和对应的颜色
block_sizes = ['1KB', '4KB', '16KB', '64KB', '256KB', '1024KB', '4096KB']
colors = ['#FFFFFF', 'none']
hatches = ['', '//']

# 不同的y坐标
y_labels = ['Average Latency(ms)', 'P99 Latency(ms)', 'Average CPU Usage(%)', 'Total Time(s)', 'Bandwidth(MB/s)',
            'Average Power(W)', 'Energy(J)']


def barplot_pure_read_vs_pure_write(device_id_, y_label_):
    # 初始化保存路径
    save_dir = os.path.join(result_dir, 'pure_read_vs_pure_write', device_id_)
    os.makedirs(save_dir, exist_ok=True)
    # 初始化画布和子图
    fig, ax = plt.subplots()
    device_data = original_data[original_data['Device ID'] == device_id_]

    # 遍历每种Block Size
    for idx, block_size in enumerate(block_sizes):
        # 根据Block Size筛选数据
        chunk_data = device_data[device_data['Block Size(KB)'] == block_size]

        # 提取read_0和read_1的Average Latency
        read_0_latencies = chunk_data[chunk_data['Operation Read Ratio'] == 'read_1'][y_label_]
        read_1_latencies = chunk_data[chunk_data['Operation Read Ratio'] == 'read_0'][y_label_]

        # 画柱状图
        ax.bar(idx * 2, read_0_latencies, color=colors[0], edgecolor='black', hatch=hatches[0], width=0.4,
               label=f"Block Size {block_size} read_1")
        ax.bar(idx * 2 + 0.6, read_1_latencies, color=colors[1], edgecolor='black', hatch=hatches[1], width=0.4,
               label=f"Block Size {block_size} read_0")

    # 移除横坐标的刻度线
    ax.tick_params(axis='x', which='both', bottom=False)

    # 设置横纵坐标标签和标题
    ax.set_xlabel('Block Size(KB)')
    ax.set_ylabel(y_label_)
    ax.set_xticks([idx * 2 + 0.2 for idx in range(len(block_sizes))])
    ax.set_xticklabels(block_sizes)
    # 设置图例
    ax.legend(['Pure Read', 'Pure Write'], loc='upper center', bbox_to_anchor=(0.5, 1), fancybox=True, shadow=True,
              ncol=2, frameon=False)

    if y_label_ == 'Bandwidth(MB/s)':
        y_label_ = 'Bandwidth(MBps)'

    plt.savefig(os.path.join(save_dir, f"{y_label_}.svg"))
    plt.close(fig)
    # 显示图形
    # plt.show()


def barplot_emmc_vs_sd(operation_read_ratio_, y_label_):
    # 初始化保存路径
    save_dir = os.path.join(result_dir, 'emmc_vs_sd', operation_read_ratio_)
    os.makedirs(save_dir, exist_ok=True)
    # 初始化画布和子图
    fig, ax = plt.subplots()
    read_ratio_data = original_data[original_data['Operation Read Ratio'] == operation_read_ratio_]

    # 遍历每种Block Size
    for idx, block_size in enumerate(block_sizes):
        # 根据Block Size筛选数据
        chunk_data = read_ratio_data[read_ratio_data['Block Size(KB)'] == block_size]

        # 提取eMMC和SD的Average Latency
        emmc_latencies = chunk_data[chunk_data['Device ID'] == 'eMMC'][y_label_]
        sd_latencies = chunk_data[chunk_data['Device ID'] == 'SD'][y_label_]

        # 画柱状图
        ax.bar(idx * 2, emmc_latencies, color=colors[0], edgecolor='black', hatch=hatches[0], width=0.4,
               label=f"Block Size {block_size} eMMC")
        ax.bar(idx * 2 + 0.6, sd_latencies, color=colors[1], edgecolor='black', hatch=hatches[1], width=0.4,
               label=f"Block Size {block_size} SD")

    # 移除横坐标的刻度线
    ax.tick_params(axis='x', which='both', bottom=False)

    # 设置横纵坐标标签和标题
    ax.set_xlabel('Block Size(KB)')
    ax.set_ylabel(y_label_)
    ax.set_xticks([idx * 2 + 0.2 for idx in range(len(block_sizes))])
    ax.set_xticklabels(block_sizes)
    # 设置图例
    ax.legend(['eMMC', 'SD'], loc='upper center', bbox_to_anchor=(0.5, 1), fancybox=True, shadow=True, ncol=2,
              frameon=False)

    if y_label_ == 'Bandwidth(MB/s)':
        y_label_ = 'Bandwidth(MBps)'

    plt.savefig(os.path.join(save_dir, f"{y_label_}.svg"))
    plt.close(fig)
    # 显示图形
    # plt.show()


if __name__ == '__main__':
    for y_label in y_labels:
        for device_id in device_id_list:
            barplot_pure_read_vs_pure_write(device_id, y_label)

        for operation_read_ratio in operation_read_ratio_list:
            barplot_emmc_vs_sd(operation_read_ratio, y_label)
