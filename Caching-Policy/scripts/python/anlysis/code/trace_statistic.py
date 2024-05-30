# 统计trace.txt的信息

import pandas as pd
import os
import glob


def trace_stat_dict(trace_txt_path_):
    data = pd.read_csv(trace_txt_path_, sep=',', names=['offset', 'size', 'type'], skiprows=1)
    READ = 0
    WRITE = 1
    stat_dict_ = {'Trace Path': trace_txt_path_,
                  'Total Request Number': data.shape[0],
                  'Read Request Number': (data['type'] == READ).sum(),
                  'Write Request Number': (data['type'] == WRITE).sum(),
                  'Max Request Size(B)': data['size'].max(), 'Min Request Size(B)': data['size'].min(),
                  'Max Read Request Size(B)': data[data['type'] == READ]['size'].max(),
                  'Min Read Request Size(B)': data[data['type'] == READ]['size'].min(),
                  'Max Write Request Size(B)': data[data['type'] == WRITE]['size'].max(),
                  'Min Write Request Size(B)': data[data['type'] == WRITE]['size'].min()}

    stat_dict_['Read Request Ratio'] = stat_dict_['Read Request Number'] / stat_dict_['Total Request Number']
    return stat_dict_


if __name__ == '__main__':
    trace_dir = '../data/trace'
    result_dir = '../result/trace'
    os.makedirs(result_dir, exist_ok=True)
    trace_txt_paths = glob.glob(os.path.join(trace_dir, '**', 'trace.txt'), recursive=True)
    stat_list = []
    for trace_txt_path in trace_txt_paths:
        stat_dict = trace_stat_dict(trace_txt_path)
        stat_list.append(stat_dict)
    df = pd.DataFrame(stat_list)
    print(df)
    output_csv_path = os.path.join(result_dir, 'trace_statistics.csv')
    df.to_csv(output_csv_path, index=False)
