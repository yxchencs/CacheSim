#!/bin/sh

save_directory="../../../log/"
mkdir "$save_directory"
cpu_usage_addr="${save_directory}cpu_usage.log"
mem_used_addr="${save_directory}mem_used.log"
disk_read_wrtn_addr="${save_directory}disk_read_wrtn.log"
top_output_addr="${save_directory}top_output.txt"
rm "$cpu_usage_addr"
rm "$mem_used_addr"
rm "$disk_read_wrtn_addr"
rm "$top_output_addr"

eMMC_name="mmcblk0"
sd_name="mmcblk1"

while true; do
    # Record current time
    time=$(date "+%Y-%m-%d %H:%M:%S")

    # Record memory used
    mem_used=$(free -m | grep "Mem:" | awk '{print $3}')

    # Run top command in batch mode and output to temporary file
    top -b -n 1 > "$top_output_addr"

    # Run iostat command and save output
    iostat_output=$(iostat -d -k)

    # Use grep command to extract lines containing "us" and "sy" and save to variable
    us_sy_lines=$(grep -E "^\s*%Cpu\(s\):.*us,..*sy" "$top_output_addr")

    # Use awk command to extract user time and system time and save to variables
    user_usage=$(echo "$us_sy_lines" | awk '{print $2}' | tr -d 'us,')
    sys_usage=$(echo "$us_sy_lines" | awk '{print $4}' | tr -d 'sy,')

    # CPU usage time = user time + system time
    cpu_usage=$(echo "${user_usage} ${sys_usage}" | awk '{print $1 + $2}')

    # Extract kB_read and kB_wrtn data from output
    read_data1=$(echo "$iostat_output" | grep -E "$eMMC_name" | awk 'NR==1 {print $6}')
    write_data1=$(echo "$iostat_output" | grep -E "$eMMC_name" | awk 'NR==1 {print $7}')
    read_data2=$(echo "$iostat_output" | grep -E "$sd_name" | awk 'NR==1 {print $6}')
    write_data2=$(echo "$iostat_output" | grep -E "$sd_name" | awk 'NR==1 {print $7}')

    # Print statistics
    # echo "User Usage: $user_usage%; System Usage: $sys_usage%"
    # echo "Cpu Usage: $cpu_usage%"
    # echo "${eMMC_name}: kB_read = $read_data1, kB_wrtn = $write_data1"
    # echo "${sd_name}: kB_read = $read_data2, kB_wrtn = $write_data2"

    # Save results
    echo "${cpu_usage}% $time" >> "$cpu_usage_addr"
    echo "${mem_used} $time" >> "$mem_used_addr"
    echo "${eMMC_name}: kB_read = $read_data1, kB_wrtn = $write_data1; ${sd_name}: kB_read = ${read_data2}, kB_wrtn = ${write_data2}; $time" >> "$disk_read_wrtn_addr"

    sleep 1
done
