import pandas as pd
import numpy as np
from scipy import stats
import matplotlib.pyplot as plt
import seaborn as sns
from scipy.stats import shapiro, probplot

# 设置字体为 Times New Roman
plt.rcParams['font.family'] = 'Times New Roman'

def analyze_excel_column(file_path, column_name, sheet_name=0):
    # 读取Excel文件
    data = pd.read_excel(file_path, sheet_name=sheet_name)

    # 确认列名是否存在
    if column_name not in data.columns:
        raise ValueError(f"'{column_name}' does not exist in the Excel file.")

    # 提取目标列
    column_data = data[column_name].dropna()  # 去除缺失值

    # 计算均值
    mean_value = column_data.mean()

    # 计算中位数
    median_value = column_data.median()

    # 计算方差
    variance_value = column_data.var()

    # 计算四分位数
    Q1 = column_data.quantile(0.25)
    Q3 = column_data.quantile(0.75)

    # 计算95%置信区间
    confidence_interval = stats.norm.interval(0.95, loc=mean_value, scale=stats.sem(column_data))

    # 打印基本统计结果
    print(f"Mean: {mean_value}")
    print(f"Median: {median_value}")
    print(f"Variance: {variance_value}")
    print(f"Q1: {Q1}, Q3: {Q3}")
    print(f"95% Confidence Interval: {confidence_interval}")

    # 计算偏度和峰度
    skewness = stats.skew(column_data)
    kurtosis_value = stats.kurtosis(column_data, fisher=False)

    # 打印偏度和峰度
    print(f"Skewness: {skewness}")
    print(f"Kurtosis: {kurtosis_value}")

    # 正态性检验 (Shapiro-Wilk)
    shapiro_test = shapiro(column_data)
    print(f"Shapiro-Wilk Test: Statistic={shapiro_test.statistic}, p-value={shapiro_test.pvalue}")

    # 数据可视化分析
    plt.figure(figsize=(14, 6))

    # 直方图与KDE
    plt.subplot(1, 2, 1)
    sns.histplot(column_data, bins=30, kde=True, color='skyblue', edgecolor='black')
    plt.title(f'Histogram and KDE of {column_name}', fontsize=16)
    plt.xlabel(column_name, fontsize=14)
    plt.ylabel("Frequency", fontsize=14)

    # 添加中位数和四分位数线
    plt.axvline(median_value, color='blue', linestyle='--', label='Median')
    plt.axvline(Q1, color='green', linestyle='--', label='Q1 (25th Percentile)')
    plt.axvline(Q3, color='orange', linestyle='--', label='Q3 (75th Percentile)')
    plt.legend()

    # Q-Q Plot
    plt.subplot(1, 2, 2)
    probplot(column_data, dist="norm", plot=plt)
    plt.title('Q-Q Plot', fontsize=16)

    plt.tight_layout()
    plt.show()

# 示例调用
analyze_excel_column('D:\code\data\statistic_cache_ycsb_125MB_2000_8.1.xlsx', 'Average CPU Usage(%)')
