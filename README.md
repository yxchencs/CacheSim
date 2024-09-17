# Presentation

CacheSim is a tool to evaluate the performance of various classical caching algorithms on small edge devices. The tool implements nine caching algorithms including FIFO, LRU, LFU, 2Q, ARC, LIRS, CLOCK-Pro and TinyLFU.

# Configuration

## Environment

- Device: Odroid-c4
- OS: Ubuntu 20.04.4 LTS
- Processor: Cortex-A55
- Memory: 3.6 GiB
- Storage: SD, eMMC

## Requirements

- C++: 17 or later
- Python 3.7 or later
- Storage: Any two flash devices with performance differences

# Preparation

## Partitioning and Formatting eMMC

> Note: eMMC here can be the better performing of the two types of flash memory.

1. List all available disk partitions on the system

```shell
fdisk -l
```

2. Create disk partitions

```shell
fdisk /dev/mmcblk0
```

Enter the following commands in sequence

```shell
n
p
1
2048
w
```

3. Format the newly created partition `/dev/mmcblk0p1` as ext4 file system

```shell
mkfs -t ext4 /dev/mmcblk0p1
```

# Getting started

## Configure per boot

### Setting the real system time if not networked

```shell
sudo date -s "YYYY-YY-DD HH:mm:ss"
```

### Mount eMMC

1. View all disks recognized by the system

```shell
fdisk -l
```

2. Temporarily mount eMMC to /mnt/eMMC

```shell
sudo mount /dev/mmcblk0p1 /mnt/eMMC
```

3. Check disk mounts

```shell
lsblk
```

## Do each test

### Place trace

Place the `trace` file in the `CacheSim\trace` directory.

### Run

#### Terminal 1

```shell
cd CacheSim/scripts/bash
bash cpu_mem_disk.sh
```

> Note: manually conduct `Ctrl+C` to terminate code at the end of its run in `Terminal 2`.

#### Terminal 2

1. Go to the source code directory

```shell
cd CacheSim/src
```

2. Compile `main.cpp`

```shell
sudo g++ -std=c++17 -o main main.cpp
```

> Note: `-std=c++17` is for CLOCK-Pro

3. Execute

```shell
sudo ./main <run_mode>
```

> Note: `run_mode` includes `device`, `ycsb` and `real`.

# Details

## Caching algorithms

The following is a list of code repositories and blogs referenced for caching algorithm implementations.

- FIFO-LFU-LRU: https://github.com/vpetrigo/caches
- 2Q: https://github.com/Mirageinvo/2Q-cache & https://blog.csdn.net/Sableye/article/details/118703319
- ARC: https://github.com/anuj-rai-23/Adaptive-Replacement-Cache-ARC-Algorithm
- LIRS: https://github.com/374576114/cache &
  https://blog.csdn.net/z69183787/article/details/105534151
- Clock-Pro: https://github.com/maximecaron/ClockProCPP
- TinyLFU: https://github.com/vimpunk/tinylfu

## Traces

### YCSB-KVTracer Traces

For further details, refer to: https://github.com/yxchencs/YCSB-KVTracer

### Real Traces

1. [Nexus5_Kernel_BIOTracer_traces - Nexus 5 Smartphone Traces](http://iotta.snia.org/traces/block-io)

   - Trace sample: log106_Messaging.txt
   - Cite: I/O Characteristics of Smartphone Applications and Their Implications for eMMC Design
     https://ieeexplore.ieee.org/abstract/document/7314143
2. [Smartphone Traces](http://visa.lab.asu.edu/web/resources/traces/)

   - Trace sample: mobi.trace.0.txt
   - Cite: Q. Yang, R. Jin, and M. Zhao, “SmartDedup: Optimizing Deduplication for Resource-constrained Devices,” Proceedings of USENIX Annual Technical Conference (USENIX ATC ’19), July 2019.
3. [MobileAppTraces](https://astl.xmu.edu.cn/appdedupe.html)

   - Trace sample: baidutieba-4h.txt
   - Cite: Bo Mao, Suzhen Wu, Hong Jiang, Xiao Chen, and Weijian Yang. Content-aware Trace Collection and I/O Deduplication for Smartphones. In Proceedings of the 33rd International Conference on Massive Storage Systems and Technology (MSST'17), Santa Clara, CA, USA, May 15-19, 2017.

## Statistics

The main data tested in this project are as follows:

1. `main.cpp`
   - Hit ratio
     - Block hit ratio
     - Trace hit ratio
     - Read/Write hit ratio
   - Latency
     - Total time
     - Average time
     - P95/P99 latency
   - Bandwidth = total size / total time
   - Average size = total size / request number
2. `cpu_mem_disk.sh`
   - Average cpu usage
   - Average memory used
   - Disk read/write
3. U96P-B power meter
   - Average power
   - Total Energy
