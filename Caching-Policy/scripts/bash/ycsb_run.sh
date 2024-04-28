#!/bin/bash
# 此脚本用于批量生成对应workload的trace_run.txt
# 此脚本在 YCSB 目录下, Linux 环境运行
# 如何运行:
#   0. wsl
#   1. export JAVA_OPTS="-Xms512m -Xmx4g"
#   2. bash ycsb_run.sh

# 开启 globstar 以支持 ** 匹配
shopt -s globstar

# 定义目标 workload 目录的根路径
TARGET_WORKLOAD_DIR="workloads/5GB/**"

# 遍历 workloads 文件夹下的每个子目录
for workload_subdir in $TARGET_WORKLOAD_DIR; do
    if [ -d "$workload_subdir" ]; then
        echo "process dir: $workload_subdir"

        # 使用 YCSB 生成 trace 文件
        ./bin/ycsb.sh load kvtracer -P "$workload_subdir/workload" -p "kvtracer.tracefile=trace_load.txt" -p "kvtracer.keymapfile=trace_keys.txt"
        ./bin/ycsb.sh run kvtracer -P "$workload_subdir/workload" -p "kvtracer.tracefile=trace_run.txt" -p "kvtracer.keymapfile=trace_keys.txt"

        # 将 trace_run.txt 文件移动到 workload 目录
        sudo mv "trace_run.txt" "$workload_subdir/"
    fi
done
