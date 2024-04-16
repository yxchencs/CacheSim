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

###### 运行参数说明

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
