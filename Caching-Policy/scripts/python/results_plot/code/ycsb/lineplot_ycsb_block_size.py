import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import os

data_dir = '../../data/statistic'
result_dir = '../../result/ycsb/lineplot/block_size_no_log(x)'
result_dir = '../../result/ycsb/lineplot/4KB/block_size'


# 读入数据
df = pd.read_excel(os.path.join(data_dir, 'statistic_125MB_uniform_latest_zipfian.xlsx'))

# 设置全局字体
plt.rcParams['font.family'] = 'Times New Roman'

# 设置图表的大小和风格
plt.rcParams["figure.figsize"] = (12, 6)
sns.set(style="white")  # 设置背景为全白色

# 定义不同的线条和标记样式
markers = ['o', 's', 'D']
LineStyles = ['-', '--', '-.']

# 获取 Workload Type 和 Operation Read Ratio 的组合
trace_operation_combinations = df[
    ['Workload Type', 'Operation Read Ratio', 'Cache Size', 'IO(on/off)']].drop_duplicates()
combination_list = trace_operation_combinations.values.tolist()  # 转换为列表


y_labels = ['Hit Ratio', 'Average Latency(ms)', 'P99 Latency(ms)', 'Total Time(s)',
            'Bandwidth(MB/s)',
            'Average CPU Usage(%)', 'Average Memory Used(MB)',
            'Average Power(W)', 'Energy(J)',
            'eMMC Read Size(KB)', 'eMMC Write Size(KB)',
            'SD Read Size(KB)', 'SD Write Size(KB)',
            'Average Power(W)', 'Energy(J)',
            'eMMC Read Numbers', 'eMMC Read Average Latency(ms)', 'eMMC Read P99 Latency(ms)',
            'eMMC Write Numbers', 'eMMC Write Average Latency(ms)', 'eMMC Write P99 Latency(ms)',
            'SD Read Numbers', 'SD Read Average Latency(ms)', 'SD Read P99 Latency(ms)',
            'SD Write Numbers', 'SD Write Average Latency(ms)', 'SD Write P99 Latency(ms)']

caching_policy_list = ['Random', 'FIFO', 'LFU', 'LRU', 'LIRS', 'ARC', 'CLOCK-Pro', '2Q', 'TinyLFU']
# caching_policy_list = ['CLOCK-Pro']


# 要测的y轴变量
def lineplot_ycsb_caching_policy(combination_, y_label_):
    # 创建图表
    fig, ax = plt.subplots()

    workload_type, operation_read_ratio, cache_size, io_status = combination_
    save_dir = os.path.join(result_dir, workload_type, str(operation_read_ratio), str(cache_size), io_status)
    os.makedirs(save_dir, exist_ok=True)

    trace_data = df[(df['Workload Type'] == workload_type) & (df['Operation Read Ratio'] == operation_read_ratio) & (
            df['Cache Size'] == cache_size) & (df['IO(on/off)'] == io_status)]

    if trace_data.empty:
        print(f"组合 {combination_} 没有数据")
        return

    # 设置 x 轴的离散值
    discrete_ticks = df['Block Size(KB)'].unique()

    # 遍历每种 Caching Policy 的组合
    for idx, caching_policy in enumerate(caching_policy_list):  # 筛选特定 Caching Policy 的数据
        policy_data = trace_data[trace_data['Caching Policy'] == caching_policy]

        if policy_data.empty:
            continue

        # 过滤掉不在离散值范围内的行
        policy_data = policy_data[policy_data['Block Size(KB)'].isin(discrete_ticks)]

        # 获取标记样式和线型
        mk = markers[idx % len(markers)]
        ls = LineStyles[idx % len(LineStyles)]

        # 绘制折线图
        sns.lineplot(ax=ax, data=policy_data, x='Block Size(KB)', y=f"{y_label_}", marker=mk, linestyle=ls,
                     label=f"{caching_policy}")

    # 设置坐标轴标签
    ax.set_xlabel("Block Size(KB)")
    ax.set_ylabel(f"{y_label_}")

    # 设置 x 轴为对数刻度
    # ax.set_xscale('log')

    ax.set_xticks(discrete_ticks)
    ax.get_xaxis().set_major_formatter(plt.ScalarFormatter())
    ax.set_xticklabels(discrete_ticks)

    ax.legend(loc='best', ncol=3, frameon=False, handlelength=3.5)

    # plt.show()

    if y_label_ == 'Bandwidth(MB/s)':
        y_label_ = 'Bandwidth(MBps)'

    # 保存图表到文件夹中
    save_path = os.path.join(save_dir, f"{y_label_}.svg")
    plt.savefig(save_path, bbox_inches='tight', pad_inches=0)
    plt.close(fig)

    print(save_path)


if __name__ == '__main__':
    for y_label in y_labels:
        for combination in combination_list:
            lineplot_ycsb_caching_policy(combination, y_label)
