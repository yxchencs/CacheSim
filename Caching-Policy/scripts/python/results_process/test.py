line = "emmc_write: nums: 34859; average latency: 0.0183321 ms; tail latency: P95 = 0.219 ms, P99 = 0.264 ms"

emmc_write_nums = int(line.split(';')[0].strip().split()[2])
emmc_write_avg_latency = float(line.split(';')[1].strip().split()[2])
emmc_write_p99 = float(line.split(';')[2].strip().split()[8])

print(emmc_write_nums,emmc_write_avg_latency,emmc_write_p99)