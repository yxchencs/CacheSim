##### 项目概述

本项目用于测试部署在多级混合存储机制的缓存策略的系统性能数据，项目结构如下：

```shell
Caching-Policy/
├── doc
│   ├── git.md
│   └── ycsb-kvtracer.md
├── scripts
│   ├── cpu_mem_disk.sh
│   ├── join_files.py
│   └── ycsb_kvtracer_process.py
├── src
│   ├── cache
│   │   ├── 2q.hpp
│   │   ├── arc.h
│   │   ├── cache.hpp
│   │   ├── cache_policy.hpp
│   │   ├── clockpro.hpp
│   │   ├── fifo.hpp
│   │   ├── lfu.hpp
│   │   ├── lirs.h
│   │   ├── lru.hpp
│   │   ├── random.h
│   │   └── tinylfu
│   │       ├── bloom_filter.hpp
│   │       ├── detail.hpp
│   │       ├── frequency_sketch.hpp
│   │       └── tinylfu.hpp
│   ├── main.cpp
│   ├── simulator
│   │   ├── 2qSl.h
│   │   ├── arcSl.h
│   │   ├── clockproSl.h
│   │   ├── fifoSl.h
│   │   ├── lfuSl.h
│   │   ├── lirsSl.h
│   │   ├── lruSl.h
│   │   ├── randomSl.h
│   │   ├── sl.h
│   │   └── tinylfuSl.h
│   └── utils
│       ├── bitmap.h
│       ├── cache_conf.h
│       ├── chunk.h
│       ├── globals.h
│       ├── policy.h
│       └── statistic.h
└── trace
```

##### 项目配置

###### 运行环境

设备：Odroid-c4
操作系统：Ubuntu 20.04.4 LTS
处理器：Cortex-A55
内存：3.6 GiB
主存：SD、eMMC

###### 对eMMC进行分区和格式化

1. 列出系统上所有可用的磁盘分区

```shell
fdisk -l
```

2. 创建磁盘分区

```shell
fdisk /dev/mmcblk0
```

依次输入如下命令

```shell
n
p
1
2048
w
```

3. 将新创建的分区 `/dev/mmcblk0p1` 格式化为ext4文件系统

```shell
mkfs -t ext4 /dev/mmcblk0p1
```

###### 挂载eMMC

1. 查看系统所有识别到的磁盘

```shell
fdisk -l
```

2. 将eMMC临时挂载到/mnt/eMMC

```shell
sudo mount /dev/mmcblk0p1 /mnt/eMMC
```

3. 检查磁盘挂载情况

```shell
lsblk
```

###### 配置 `globals.h`

1. 打开 `src/utils/globals.h`，修改 `cache_path`和 `DISK_PATH`

```c++
const char *cache_path = "/mnt/eMMC/cache_0.1.bin";
const char *DISK_PATH = "../trace/zipfian/r100w_o15w_0.99/read_0/storage/disk.bin";
```

2. 修改 `TRACE_PATH`

```C++
const char *TRACE_PATH = "../trace/zipfian/r100w_o15w_0.99/read_0/trace.txt";
```

3. 修改 `disk_size`和 `chunk_num`

```C++
const long long disk_size = 33668;
const long long chunk_num = 33668;
```

其中，`disk_size`表示disk的总存储容量，由trace访问的offset跨度决定；

`chunk_num`表示trace访问的不重复的chunk总size。

> 在生成的trace.txt文件第一行按顺序记录了统计的 `disk_size`、`chunk_num`和 `TRACE_SIZE`信息，我使用YCSB生成的trace控制了单次访问一个chunk，因此前两者数值一致，只记录了一个数字。

###### 配置cache和disk文件

1. 进入项目根目录

```shell
cd Caching-Policy/Caching-Policy
```

2. 将用作缓存地址空间的文件复制进 `/mnt/eMMC`

```shell
sudo cp trace/zipfian/r100w_o15w_0.99/read_0/storage/* /mnt/eMMC/
```

3. 为cache和disk文件赋予可写可读可执行权限

```shell
sudo chmod 777 /mnt/eMMC/cache*
sudo chmod 777 trace/zipfian/r100w_o15w_0.99/read_0/storage/disk.bin
```

##### 项目运行

生成 `results`目录

```shell
mkdir results
```

运行结果均保存在 `results`中

###### Terminal 1

```shell
cd scripts
sh cpu_mem_disk.sh
```

`Terminal 2`中代码运行结束后 `Ctrl+C`终止

###### Terminal 2

1. 进入源代码目录

```shell
cd src
```

2. 编译main.cpp

```shell
g++ -std=c++17 -o main main.cpp # -std=c++17  for clock-pro
```

3. 执行

```shell
./main 0 3
```

其中，第一个参数 `0`用于指定cache容量占disk容量的比例为0.02，其余对应关系如下表：

| Number | Cache size |
| :----: | :--------: |
|   0    |    0.02    |
|   1    |    0.04    |
|   2    |    0.06    |
|   3    |    0.08    |
|   4    |    0.1     |

除此之外，第二个参数 `3`用于指定缓存策略为LRU，其余对应关系如下表：

| Number | Caching policy |
| :----: | :------------: |
|   0    |     RANDOM     |
|   1    |      FIFO      |
|   2    |      LFU       |
|   3    |      LRU       |
|   4    |      LIRS      |
|   5    |      ARC       |
|   6    |    CLOCKPRO    |
|   7    |       2Q       |
|   8    |    TINYLFU     |

##### 项目内容

###### Caching policy

下面列出缓存策略实现参考的代码仓库和博客

- FIFO-LFU-LRU
  https://github.com/vpetrigo/caches
- LIRS
  https://github.com/374576114/cache
  https://blog.csdn.net/z69183787/article/details/105534151
- ARC
  https://github.com/anuj-rai-23/Adaptive-Replacement-Cache-ARC-Algorithm
- Clock-Pro
  https://github.com/maximecaron/ClockProCPP
- 2Q
  https://github.com/Mirageinvo/2Q-cache
  https://blog.csdn.net/Sableye/article/details/118703319
- TinyLFU
  https://github.com/vimpunk/tinylfu

###### Trace

1. [Nexus5_Kernel_BIOTracer_traces - Nexus 5 Smartphone Traces](http://iotta.snia.org/traces/block-io)
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

ref: Revisiting Temporal Storage I/O Behaviors of Smartphone Applications: Analysis and Synthesis
https://ieeexplore.ieee.org/document/9975405
ref: I/O Characteristics of Smartphone Applications and Their Implications for eMMC Design
https://ieeexplore.ieee.org/abstract/document/7314143

2. [Smartphone Traces](http://visa.lab.asu.edu/web/resources/traces/)

http://visa.lab.asu.edu/web/resources/traces/traces-smartphone/

sample: mobi.trace.1

ref: Q. Yang, R. Jin, and M. Zhao, “SmartDedup: Optimizing Deduplication for Resource-constrained Devices,” Proceedings of USENIX Annual Technical Conference (USENIX ATC ’19), July 2019.

3. [MobileAppTraces](https://astl.xmu.edu.cn/appdedupe.html)

The trace file-names are indicating different mobile applications and some file-names also include the running hours. Each record in the trace files is as follows:
[Time in s] [R or W] [LBA] [size in 512 Bytes blocks] [pid] [process] [MD5 per 4096 Bytes]
sample:
7.430000743 	R	     1548752	        24	       744	       Binder_5	fb0ece031db8f58df6849c2211df8c5a-35598db6787678b2acf4d0fc056f3b1d-9fc4e51c5541ecaeefc9da9e7cc55587

ref: Improving Flash Memory Performance and Reliability for Smartphones With I/O Deduplication
cite: Bo Mao, Suzhen Wu, Hong Jiang, Xiao Chen, and Weijian Yang. Content-aware Trace Collection and I/O Deduplication for Smartphones. In Proceedings of the 33rd International Conference on Massive Storage Systems and Technology (MSST'17), Santa Clara, CA, USA, May 15-19, 2017.

4. [uniform/latest/zipfian trace](ycsb-kvtracer.md)
5. random trace

###### Statistic

本项目主要测试的数据如下

1. `main.cpp`
   - hit ratio
     - block hit ratio
     - trace hit ratio
     - read/write hit ratio
   - latency
     - total time
     - average time
     - p95/p99 latency
   - band width = total size / total time
   - average size = total size / request number
2. `cpu_mem_disk.sh`
   - cpu usage
   - memory used
   - disk read/write
3. IoT Power - CC
   - power
   - energy
