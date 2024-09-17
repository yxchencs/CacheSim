import pandas as pd
import matplotlib.pyplot as plt
import os

data_dir = '../../data/statistic'
result_dir = '../../result'
statistic_name = 'statistic_real_3_new.xlsx'

# 设置全局字体
plt.rcParams['font.family'] = 'Times New Roman'
# 设置图表的大小和风格
plt.rcParams["figure.figsize"] = (12, 6)

# 定义颜色和样式
colors = ['#FFFFFF', '#CCCCCC', '#666666', '#FFFFFF', '#CCCCCC', '#666666', '#FFFFFF', '#CCCCCC', '#666666']
hatches = ['', '', '', '/', '/', '/', '\\\\', '\\\\', '\\\\']

# 读取Excel文件
original_data = pd.read_excel(os.path.join(data_dir, statistic_name))

# 获取 Workload Type 和 Operation Read Ratio 的组合
trace_operation_combinations = original_data[
    ['Workload Type', 'Real Trace Name', 'IO(on/off)', 'Cache Size']].drop_duplicates()
combination_list = trace_operation_combinations.values.tolist()  # 转换为列表

# 不同的y坐标
y_labels = ['Hit Ratio', 'Average Latency(ms)', 'P99 Latency(ms)', 'Average CPU Usage(%)', 'Total Time(s)',
            'Bandwidth(MB/s)',
            'Average Power(W)', 'Energy(J)']


def barplots_real_single_cache_policy(combination_, y_label_):
    workload_type, real_trace_name, io_status, cache_size = combination_
    if y_label_ == 'Bandwidth(MB/s)':
        file_name = 'Bandwidth(MBps)'
    else:
        file_name = y_label_.replace(' ', '_')
    save_dir = os.path.join(result_dir, workload_type, real_trace_name, io_status, str(cache_size))
    os.makedirs(save_dir, exist_ok=True)  # 先创建目录
    save_path = os.path.join(save_dir, f"{file_name}.svg")

    # 初始化画布和子图
    fig, ax = plt.subplots()
    trace_data = original_data[(original_data['Workload Type'] == workload_type) &
                               (original_data['Real Trace Name'] == real_trace_name) &
                               (original_data['IO(on/off)'] == io_status) &
                               (original_data['Cache Size'] == cache_size)]

    caching_policy_list = ['Random', 'FIFO', 'LFU',  'LRU', 'LIRS', 'ARC', 'CLOCK-Pro','2Q',  'TinyLFU']

    # 遍历每种Block Size
    for idx, caching_policy in enumerate(caching_policy_list):
        # 根据Block Size筛选数据
        y_axis = trace_data[trace_data['Caching Policy'] == caching_policy][y_label_]

        # 画柱状图
        if not y_axis.empty:
            ax.bar(idx, y_axis, color=colors[idx], edgecolor='black', hatch=hatches[idx], width=0.4,
                   label=f"{caching_policy}")

    # 移除横坐标的刻度线
    ax.tick_params(axis='x', which='both', bottom=False)

    # 设置横纵坐标标签和标题
    ax.set_xlabel('Caching Policy')
    ax.set_ylabel(y_label_)
    ax.set_xticks([idx for idx in range(len(caching_policy_list))])
    ax.set_xticklabels(caching_policy_list)
    # 设置图例
    ax.legend(caching_policy_list, loc='upper left', bbox_to_anchor=(0, 1), fancybox=True, shadow=False, ncol=5,
              frameon=False)
    # 显示图形
    # plt.show()

    # 保存图形
    plt.savefig(save_path, bbox_inches='tight', pad_inches=0)

    plt.close(fig)


if __name__ == '__main__':
    for y_label in y_labels:
        for combination in combination_list:
            barplots_real_single_cache_policy(combination, y_label)
