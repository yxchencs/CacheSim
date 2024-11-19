import pandas as pd
import numpy as np
from scipy import stats
import matplotlib.pyplot as plt
import seaborn as sns
from scipy.stats import shapiro, probplot

# Set the font to Times New Roman
plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['font.weight'] = 'bold'

def analyze_excel_column(file_path, column_name, sheet_name=0):
    print('//',column_name,'//')

    # Read the Excel file
    data = pd.read_excel(file_path, sheet_name=sheet_name)

    # Check if the column name exists
    if column_name not in data.columns:
        raise ValueError(f"'{column_name}' does not exist in the Excel file.")

    # Extract the target column
    column_data = data[column_name].dropna()  # Remove missing values

    # Calculate statistics
    mean_value = column_data.mean()
    median_value = column_data.median()
    variance_value = column_data.var()
    Q1 = column_data.quantile(0.25)
    Q3 = column_data.quantile(0.75)

    # Print basic statistics
    print(f"Mean: {mean_value:.2f}")
    print(f"Median: {median_value:.2f}")
    print(f"Variance: {variance_value:.2f}")
    print(f"Q1: {Q1:.2f}, Q3: {Q3:.2f}")

    # Normality test (Shapiro-Wilk)
    shapiro_test = shapiro(column_data)
    print(f"Shapiro-Wilk Test: Statistic={shapiro_test.statistic:.4f}, p-value={shapiro_test.pvalue:.4f}")

    # Visualization analysis
    plt.figure(figsize=(10, 6))  # Set width to 8, height to 6

    # Histogram with KDE
    sns.histplot(column_data, bins=30, kde=True, color='skyblue', edgecolor='black')

    # Set xlabel and ylabel with bold font weight
    plt.xlabel(column_name, fontsize=16, fontweight='bold')
    plt.ylabel("Frequency", fontsize=16, fontweight='bold')

    # Add median and quartile lines
    plt.axvline(median_value, color='blue', linestyle='--', label='Median')
    plt.axvline(Q1, color='green', linestyle='--', label='Q1 (25th Percentile)')
    plt.axvline(Q3, color='orange', linestyle='--', label='Q3 (75th Percentile)')

    # Update legend to include the prop parameter for font properties and set frameon=False
    plt.legend(prop={'size': 16, 'weight': 'bold'}, frameon=False)

    # Create a title for saving the file (as an example, using the column name)
    title = f'HistogramAndKDEOf{column_name.replace(" ", "")}'


    # Save the figure as PDF
    # plt.title(title, fontsize=16, fontweight='bold')
    plt.savefig(f"{title}.eps")  # 保存为 PDF 文件
    plt.close()

    # Q-Q Plot (commented out, but not deleted)
    # plt.subplot(1, 2, 2)
    # probplot(column_data, dist="norm", plot=plt)
    # plt.title('Q-Q Plot', fontsize=16, fontweight='bold')
    #
    # plt.xlabel('Theoretical Quantiles', fontsize=16, fontweight='bold')  # 加粗的 x 轴标签
    # plt.ylabel('Sample Quantiles', fontsize=16, fontweight='bold')      # 加粗的 y 轴标签
    #
    # plt.tick_params(axis='x', labelsize=14, width=2)
    # plt.tick_params(axis='y', labelsize=14, width=2)

# Example call
analyze_excel_column('D:\\document\\Paper\\CP\\review\\results\\statistic.xlsx', 'Average CPU Usage(%)')
analyze_excel_column('D:\\document\\Paper\\CP\\review\\results\\statistic.xlsx', 'Average Power(W)')
