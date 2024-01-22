#!/bin/bash
# 此脚本在YSCB/目录下运行

# 定义 genarate_trace 目录的路径
GENERATE_TRACE_DIR="/mnt/d/Projects/DatabaseLearn/genarate_trace"

# 遍历 workloads 文件夹下的每个子目录
for workload_dir in workloads/*/*; do
    if [ -d "$workload_dir" ]; then
        echo "处理目录: $workload_dir"

        # 使用 YCSB 生成 trace 文件
        bin/ycsb.sh load kvtracer -P "$workload_dir/workload" -p "kvtracer.tracefile=trace_load.txt" -p "kvtracer.keymapfile=trace_keys.txt"
        bin/ycsb.sh run kvtracer -P "$workload_dir/workload" -p "kvtracer.tracefile=trace_run.txt" -p "kvtracer.keymapfile=trace_keys.txt"

        # 将 trace_run.txt 文件复制到 workload 目录
        cp "trace_run.txt" "$workload_dir/"

        # 运行 ycsb_kvtracer_process.py 脚本
        python "$GENERATE_TRACE_DIR/ycsb_kvtracer_process.py"

        # 将 genarate_trace/trace 目录下的所有文件移动到 workload 目录
        mv "$GENERATE_TRACE_DIR/trace/"* "$workload_dir/"
    fi
done
