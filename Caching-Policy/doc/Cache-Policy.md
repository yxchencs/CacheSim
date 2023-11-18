# Cache-Policy

## Environment 
操作系统：Ubuntu 20.04.4 LTS
处理器：Cortex-A55
内存：3.6 GiB


## caching policy
password: odroid

fifo-lfu-lru
https://github.com/vpetrigo/caches

fifo-lfu
https://github.com/solangii/caches

LIRS
https://github.com/374576114/cache
https://blog.csdn.net/z69183787/article/details/105534151

ARC
https://github.com/Xiaoccer/ARC_Cache
https://github.com/anuj-rai-23/Adaptive-Replacement-Cache-ARC-Algorithm

Clock-Pro
https://github.com/maximecaron/ClockProCPP

## Trace
### [Nexus5_Kernel_BIOTracer_traces - Nexus 5 Smartphone Traces](http://iotta.snia.org/traces/block-io)
sample: log106_Messaging.txt
- 列0: access起始地址，单位为扇区
- 列1: 说明大小应该是时间为8，因为基本块大小是4kb或8扇区。但是mmc驱动程序添加了额外的扇区到一些大小，因此你需要清理它。
- 列2: 访问大小，以字节为单位。
- 列3: 访问类型&是否等待:最低位表示读或写(0表示读，1表示写)，第三位表示请求是否等待(4表示没有等待，0表示已经等待了一段时间)
  例如，5表示请求是一个写请求，请求在处理之前没有等待，这表明请求到达时队列是空的。
- 列4: 请求生成时间(请求生成并插入到请求队列)。
- 列5: 请求处理开始时间(MMC驱动程序从请求队列取出请求并开始处理)
- 列6: 向硬件提交请求的时间(驱动程序向硬件发出请求的时间)
- 列7: 请求完成时间(请求完成后调用回调函数的时间)

### [Traces – Smartphone](http://visa.lab.asu.edu/web/resources/traces/)
http://visa.lab.asu.edu/web/resources/traces/traces-smartphone/

sample: mobi.trace.1

ref: Q. Yang, R. Jin, and M. Zhao, “SmartDedup: Optimizing Deduplication for Resource-constrained Devices,” Proceedings of USENIX Annual Technical Conference (USENIX ATC ’19), July 2019.

### [MobileAppTraces](https://astl.xmu.edu.cn/appdedupe.html)
The trace file-names are indicating different mobile applications and some file-names also include the running hours. Each record in the trace files is as follows: 
[Time in s] [R or W] [LBA] [size in 512 Bytes blocks] [pid] [process] [MD5 per 4096 Bytes]
sample:
7.430000743 	R	     1548752	        24	       744	       Binder_5	fb0ece031db8f58df6849c2211df8c5a-35598db6787678b2acf4d0fc056f3b1d-9fc4e51c5541ecaeefc9da9e7cc55587

ref: Bo Mao, Suzhen Wu, Hong Jiang, Xiao Chen, and Weijian Yang. Content-aware Trace Collection and I/O Deduplication for Smartphones. In Proceedings of the 33rd International Conference on Massive Storage Systems and Technology (MSST'17), Santa Clara, CA, USA, May 15-19, 2017.
### config
> 本小节目录结构如下
> - Caching-Policy
>   - src
>   - storage
>   - trace

#### 在Windows系统运行
1. 在`trace`文件夹放入对应的trace文件，如`trace.txt`，并修改`src/utils/config.h`中的`TRACE_PATH`和`DISK_SIZE`、`CHUNK_NUM`
2. 在`storage`文件夹放入对应的storage文件，共5个：`disk.bin`, `cache_0.02.bin`, `cache_0.04.bin`, `cache_0.06.bin`, `cache_0.08.bin`, `cache_0.1.bin`

## create disk patition
fdisk -l
fdisk /dev/mmcblk0
```
n
p
1
2048
w
```
mkfs -t ext4 /dev/mmcblk0p1
sudo mount /dev/mmcblk0p1 /mnt/eMMC 
sudo chmod 777 /mnt/eMMC/*
## How to run

1. fdisk -l 查看系统所有识别到的磁盘
2. sudo mount /dev/mmcblk0p1 /mnt/eMMC 临时挂载
3. lsblk 查看磁盘挂载情况
6. sudo chmod 777 /mnt/eMMC/cache* 授权
7. cd /home/odroid/R/Caching-Policy/cache/src
8. chmod 777 ../storage  # 目录 可写可读可执行
8. g++ -std=c++17 -o test test.cpp # -std=c++17  for clock-pro
9. ./test
10. ls -lht document size
## Test

1. bash cp.sh 测试CPU使用率

## trace

## statistic

1. *+ Latency = the time process a trace
2. *+ average time
3. *+ band width = total size / total time
4. *+ average size = total size / request number
5. *+ hit ratio
6. *+ trace hit ratio
7. power/energy
9. *+ cpu usage

o-direct

## problem

1. how to cetegory different caching policy
2. relationship between hit ratio, power, latency
3. caching policy on resouce-limited edge
4. analizy data and conclude from which
5. o-direct
6. c.sh output formalated txt: cpu usage
7. test add start/end time

8. debug c.sh sleep: missing operand
9. power

## 统计代码量
```shell
find . -type f -print | xargs wc -l
```
代码量: 3309

使用代码量统计命令`cloc`
```shell
cloc .
```
