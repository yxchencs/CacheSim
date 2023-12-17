#!/bin/sh

save_directory="../results/"
cpu_usage_addr="${save_directory}cpu_usage.log"
mem_used_addr="${save_directory}mem_used.log"
disk_read_wrtn_addr="${save_directory}disk_read_wrtn.log"
top_output_addr="top_output.txt"
rm "$cpu_usage_addr"
rm "$mem_used_addr"
rm "$disk_read_wrtn_addr"
rm "$top_output_addr"

eMMC_name="mmcblk0"
sd_name="mmcblk1"

while true; do
    # 记录当前时刻
    time=$(date "+%Y-%m-%d %H:%M:%S")

    # 记录memory used
    mem_used=$(free -m | grep "Mem:" | awk '{print $3}')

    # 运行top命令以批处理模式，输出到临时文件
    top -b -n 1 > "$top_output_addr"

    # 运行 iostat 命令并保存输出
    iostat_output=$(iostat -d)

    # 使用grep命令提取包含"us"和"sy"的行，并将结果保存到变量中
    us_sy_lines=$(grep -E "^\s*%Cpu\(s\):.*us|sy" top_output.txt)

    # 使用awk命令提取用户时间和系统时间，并保存到变量中
    user_usage=$(echo "$us_sy_lines" | awk '{print $2}' | tr -d 'us,')
    sys_usage=$(echo "$us_sy_lines" | awk '{print $4}' | tr -d 'sy,')

    # cpu使用时间 = 用户时间 + 系统时间
    cpu_usage=`echo ${user_usage} ${sys_usage} | awk '{print $1+$2}'`

    # 从输出中提取 kB_read 和 kB_wrtn 数据
    read_data1=$(echo "$iostat_output" | grep -E "$eMMC_name" | awk 'NR==1 {print $6}')
    write_data1=$(echo "$iostat_output" | grep -E "$eMMC_name" | awk 'NR==1 {print $7}')
    read_data2=$(echo "$iostat_output" | grep -E "$sd_name" | awk 'NR==1 {print $6}')
    write_data2=$(echo "$iostat_output" | grep -E "$sd_name" | awk 'NR==1 {print $7}')

    # 打印统计结果
    # echo "User Usage: $user_usage%; System Usage: $sys_usage%"
    # echo "Cpu Usage: $cpu_usage%"
    # echo "${eMMC_name}: kB_read = $read_data1, kB_wrtn = $write_data1"
    # echo "${sd_name}: kB_read = $read_data2, kB_wrtn = $write_data2"

    # 保存结果
    echo "${cpu_usage}% $time" >> "$cpu_usage_addr"
    echo "${mem_used} $time" >> "$mem_used_addr"
    echo "${eMMC_name}: kB_read = $read_data1, kB_wrtn = $write_data1; ${sd_name}: kB_read = ${read_data2}, kB_wrtn = ${write_data2}; $time" >> "$disk_read_wrtn_addr"

    # 清理临时文件
    rm top_output.txt

    sleep 1
done
