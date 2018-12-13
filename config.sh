#########################################################################
# File Name: config.sh
# Author: Sues
# mail: sumory.kaka@foxmail.com
# Created Time: Thu 13 Dec 2018 06:26:21 AM PST
# Version : 1.0
#########################################################################
#!/bin/bash
export ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-;make vexpress_ca9x4_defconfig;
