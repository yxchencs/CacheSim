#### YCSB编译KVTracer模块的基本步骤

环境：Ubuntu 22.04.3 LTS
本教程同样适用于**已配置maven的Windows系统**，修改文件时将文件用记事本打开修改即可

##### 1. 克隆`YCSB`

```shell
git clone https://github.com/brianfrankcooper/YCSB.git
```

##### 2. 进入`YCSB`根目录

```shell
cd YCSB/
```

##### 3. 克隆`kvtracer`

```shell
git clone https://github.com/seekstar/kvtracer.git
```

##### 4. 将 `YCSB` 根目录下的 `pom.xml` 文件修改为包含 `kvtracer`

```shell
sudo vim pom.xml
```

如`+`所在行所示

```xml
  <properties>
    ...
    <redis.version>2.0.0</redis.version>
+   <kvtracer.version>0.1.0</kvtracer.version>
    ...
  </properties>
  <modules>
    ...
    <module>redis</module>
+   <module>kvtracer</module>
    ...
  </modules>
```

##### 5. 修改 `YCSB` 中的 `bin/ycsb` 以包括 `kvtracer`

```shell
sudo vim bin/ycsb
```

如`+`所在行所示

```shell
DATABASES = {
    ...
    "redis"        : "site.ycsb.db.RedisClient",
+   "kvtracer"     : "site.ycsb.db.KVTracerClient",
    ...
}
```

##### 6. 修改 `YCSB` 中的 `bin/bindings.properties` 以包括 `kvtracer`

```shell
sudo vim bin/bindings.properties
```

如`+`所在行所示

```shell
    redis:site.ycsb.db.RedisClient
+   kvtracer:site.ycsb.db.KVTracerClient
```

##### 7. 编译

```shell
mvn -pl kvtracer -am clean package
```

##### 8. 运行YCSB命令生成工作负载跟踪

###### On Linux

```shell
bin/ycsb.sh load kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_load.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```

and

```shell
bin/ycsb.sh run kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_run.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```

###### On Windows

```shell
bin/ycsb.bat load kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_load.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```

and

```shell
bin/ycsb.bat run kvtracer -P workloads/workloada -p "kvtracer.tracefile=tracea_run.txt" -p "kvtracer.keymapfile=tracea_keys.txt"
```

bin/ycsb.bat load kvtracer -P workloads/workload_zipfian_95 -p "kvtracer.tracefile=trace_zipfian_load.txt" -p "kvtracer.keymapfile=trace_zipfian_keys.txt"

bin/ycsb.bat run kvtracer -P workloads/workload_zipfian_95 -p "kvtracer.tracefile=trace_zipfian_run.txt" -p "kvtracer.keymapfile=trace_zipfian_keys.txt"