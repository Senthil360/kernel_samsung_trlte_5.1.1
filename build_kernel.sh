export ARCH=arm
export CROSS_COMPILE=/home/crimsonthunder/arm-eabi-5.2/arm-linux-gnueabi-5.2/bin/arm-eabi-
mkdir output

make -C $(pwd) O=$(pwd)/output VARIANT_DEFCONFIG=apq8084_sec_trlte_eur_defconfig apq8084_sec_defconfig SELINUX_DEFCONFIG=selinux_defconfig SELINUX_LOG_DEFCONFIG=selinux_log_defconfig TIMA_DEFCONFIG=tima_defconfig DMVERITY_DEFCONFIG=dmverity_defconfig
make -C $(pwd) O=output -j12

mv output/arch/arm/boot/zImage $(pwd)/AIK-Linux/split_img/boot.img-zImage
cd AIK-Linux/
bash repackimg.sh
cd .. 
mv AIK-Linux/image-new.img $(pwd)/Kernel.img
rm  -r output/
