#!/bin/bash

# TODO : 分区时 请勿将 (3. 剩余未分区(62409166-62410752),用于u-boot)部分 分区


if [ $# -eq 0 ];then
    echo usage : ./do.sh u-boot.bin u-boot-spl.bin /dev/sdcard
    echo you must have superuser permisson
    exit -2
fi

boot=$1
bootspl=$2
sdcard=$3

############################### STEP0 分区

#sd卡32G,62410752个sector
#    1. 512M 分区 (2048-1050623),用于zImage,fat16 // ${sdcard}1            2048     1050623      524288   83  Linux
#    2.      分区 (1050624-62409165) ,用于rootfs,ext3
#    3. 剩余未分区(62409166-62410752),用于u-boot

# 62409166-62410752(最后一个sector) 排布

# |_________________|________|________|______________|______________|(END)
# BL2(1024个sector) ENV(32)  BL1(16)  signature(1)   Reserved(1025)

############################### STEP0.5 制作文件系统


############################### STEP1 u-boot


[ ! -e ${sdcard} ] && echo ${sdcard} not exist && exit -1

BYTES_PRE_SECTOR=512

let ALL_SECTOR_NUMBER=`sudo fdisk -l ${sdcard} | head -1 | tail -1 | awk -F " " '{print $7}'`
let Reserved_SECTOR_NUMBER=1025
let Signature_SECTOR_NUMBER=1
let BL1_SECTOR_NUMBER=16
let BL2_SECTOR_NUMBER=1024
let ENV_SZ=32

BL1_SECTOR_START_ADDR=`echo  ${ALL_SECTOR_NUMBER} - 1025 - 1 - 16 | bc`
BL2_SECTOR_START_ADDR=`echo ${BL1_SECTOR_START_ADDR} - ${BL2_SECTOR_NUMBER} - ${ENV_SZ} | bc`

echo ALL_SECTOR_NUMBER     : ${ALL_SECTOR_NUMBER}
echo BL1_SECTOR_START_ADDR : ${BL1_SECTOR_START_ADDR}
echo BL2_SECTOR_START_ADDR : ${BL2_SECTOR_START_ADDR}


if [ -f ${boot} ];then
    rm ./bl1.bin ./bl2.bin -f
	cp ${bootspl} ./bl1.bin
    cp ${boot}    ./bl2.bin
fi


sudo dd if=./bl1.bin of=${sdcard} seek=${BL1_SECTOR_START_ADDR} bs=${BYTES_PRE_SECTOR} count=${BL1_SECTOR_NUMBER}
sudo dd if=./bl2.bin of=${sdcard} seek=${BL2_SECTOR_START_ADDR} bs=${BYTES_PRE_SECTOR} count=${BL2_SECTOR_NUMBER}

sudo rm ./bl1.bin ./bl2.bin
