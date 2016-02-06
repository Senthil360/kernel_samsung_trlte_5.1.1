#!/system/bin/sh

#Set default values on boot
echo "200000000" > /sys/class/kgsl/kgsl-3d0/devfreq/min_freq
echo "600000000" > /sys/class/kgsl/kgsl-3d0/max_gpuclk
echo deadline > /sys/block/mmcblk0/queue/scheduler
echo cfq > /sys/block/mmcblk0rpmb/queue/scheduler
echo deadline > /sys/block/mmcblk1/queue/scheduler
sleep 2;
echo 8 > /sys/block/mmcblk0/queue/iosched/fifo_batch
echo 1 > /sys/block/mmcblk0/queue/iosched/front_merges
echo 250 > /sys/block/mmcblk0/queue/iosched/read_expire
echo 2500 > /sys/block/mmcblk0/queue/iosched/write_expire
echo 2 > /sys/block/mmcblk0/queue/iosched/writes_starved
echo 8 > /sys/block/mmcblk1/queue/iosched/fifo_batch
echo 1 > /sys/block/mmcblk1/queue/iosched/front_merges
echo 250 > /sys/block/mmcblk1/queue/iosched/read_expire
echo 2500 > /sys/block/mmcblk1/queue/iosched/write_expire
echo 2 > /sys/block/mmcblk1/queue/iosched/writes_starved
echo "14080,21248,28416,35328,42496,49664" > /sys/module/lowmemorykiller/parameters/minfree
echo "0" > /sys/module/mmc_core/parameters/crc

sync
