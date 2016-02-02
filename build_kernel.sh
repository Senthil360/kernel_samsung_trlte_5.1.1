export ARCH=arm
export CROSS_COMPILE=/home/crimsonthunder/arm-eabi-5.2/bin/arm-eabi-
mkdir output

make -C $(pwd) O=$(pwd)/output VARIANT_DEFCONFIG=apq8084_sec_trlte_eur_defconfig apq8084_sec_defconfig SELINUX_DEFCONFIG=selinux_defconfig SELINUX_LOG_DEFCONFIG=selinux_log_defconfig TIMA_DEFCONFIG=tima_defconfig DMVERITY_DEFCONFIG=dmverity_defconfig
make -C $(pwd) O=output -j12

mv output/arch/arm/boot/zImage $(pwd)/imgtools/Boot/kernel
./imgtools/dtbTool -o ./imgtools/Boot/dt.img -s 4096 -p ./output/scripts/dtc/ ./output/arch/arm/boot/dts/
bash ./imgtools/mkboot ./imgtools/Boot ./imgtools/Boot.img
mv ./imgtools/Boot.img ./Boot.img
rm ./imgtools/Boot/kernel
rm ./imgtools/Boot/dt.img
rm -rf output/