# 1 Presentation

CacheSim is a tool to evaluate the performance of various classical caching algorithms on small edge devices. The tool implements nine caching algorithms including FIFO, LRU, LFU, 2Q, ARC, LIRS, CLOCK-Pro and TinyLFU.

# 2 Configuration

## 2.1 Environment

- Device: Odroid-c4
- OS: Ubuntu 20.04.4 LTS
- Processor: Cortex-A55
- Memory: 3.6 GiB
- Storage: SD, eMMC

> Note: VMware are not recommended, code may report errors.

## 2.2 Requirements

- C++: 17 or later
- Python 3.7 or later
- Storage: Any two flash devices with performance differences
- C++ Package: uuid-dev

# 3 Quick Start with sample for Code Debugging on PC(Optional)

> Refer to this section if you want to debug your code on a PC and not for actual edge device cache testing. If you don't need it, then skip this section.

1. Check trace

Ensure that the trace data is located in the directory `CacheSim/trace` and that trace is at the same level as `src`. Create the directory `trace` if it does not exist.

```shell
cd CacheSim
sudo rm -rf trace
mkdir trace
```

Copy the `CacheSim/sample/125MB` folder to the `CacheSim/trace` directory if you want to test the `nocache` and `ycsb` modes, or copy the `CacheSim/sample/real_trace_3` folder to the trace directory if you want to test the `real` and `montecarlo` mode.

```shell
cp -r sample/125MB trace
```

or

```shell
cp -r sample/real_trace_3 trace
```

2. Compile

Make sure you're in a linux environment. Then compile.

```shell
cd CacheSim/src
sudo g++ -g -std=c++17 -o main main.cpp -luuid
```

2. Run

```shell
sudo ./main <run_mode>
```

> Note: `run_mode` includes `nocache`, `ycsb`, `real` and `montecarlo`.

# 4 Preparation

## 4.1 Partitioning and Formatting eMMC

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

# 5 Getting started

## 5.1 Configure per boot

### 5.1.1 Setting the real system time if not networked

```shell
sudo date -s "YYYY-YY-DD HH:mm:ss"
```

### 5.1.2 Mount eMMC

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

## 5.2 Do each test

### 5.2.1 Place trace

Place the `trace` file in the `CacheSim\trace` directory. Create the directory `trace` if it does not exist.

> Note: Only one set of Trace can be placed at a time。

### 5.2.2 Run

#### Power Tester

Start power tester.

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
sudo g++ -std=c++17 -o main main.cpp -luuid
```

> Note: `-std=c++17` is for CLOCK-Pro

3. Execute

```shell
sudo ./main <run_mode>
```

> Note: `run_mode` includes `nocache`, `ycsb`, `real` and `montecarlo`.

> If you want to debug a memory leak, replace the normal run command with the following:
>
> ```shell
> sudo valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./main <run_mode>
> ```

### 5.3 (Optional) Extending CacheSim

In addition, CacheSim has good scalability and supports extending new caching algorithms and designing new test flows. To add a new cache algorithm implementation, follow these steps.

1. Implement the new caching algorithm in the `src/cache/` folder.
2. Inherit the base `class Sl` in the `src/simulator/` folder, call the new caching algorithm, implement the interfaces of the base `class Sl` (`isCached()`, `accessKey()` and `getVictim()`).
3. Add the new caching algorithm to `src/utils/policy.h`.
4. Add the new caching algorithm to the `runYcsbOnce()` and `runRealOnce()` functions in `src/utils/run.h`.
5. (Optional) Write new own test run function in `src/utils/run.h` and call it in `src/main.cpp`.

# 6 Details

## 6.1 Caching algorithms

The following is a list of code repositories and blogs referenced for caching algorithm implementations.

- FIFO-LFU-LRU: https://github.com/vpetrigo/caches
- 2Q: https://github.com/Mirageinvo/2Q-cache & https://blog.csdn.net/Sableye/article/details/118703319
- ARC: https://github.com/anuj-rai-23/Adaptive-Replacement-Cache-ARC-Algorithm
- LIRS: https://github.com/374576114/cache &
  https://blog.csdn.net/z69183787/article/details/105534151
- Clock-Pro: https://github.com/maximecaron/ClockProCPP
- TinyLFU: https://github.com/vimpunk/tinylfu

## 6.2 Traces

### 6.2.1 YCSB-KVTracer Traces

For further details, refer to: https://github.com/yxchencs/YCSB-KVTracer.

### 6.2.2 Real Traces

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

## 6.3 Statistics

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
