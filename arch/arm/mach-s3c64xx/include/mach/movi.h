#ifndef __MOVI_H__
#define __MOVI_H__

/****************************************************************
 *©°©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©´
 *©¦                    SD/MMC Device                           ©¦
 *©À©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ð©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©È
 *©¦ FileSystem ©¦ Kernel ©¦ BL2 ©¦ ENV ©¦ BL1 ©¦Signature©¦ Reserved ©¦
 *©À©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©¤©¤©¤©¤©à©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©È
 *©¦            ©¦        ©¦     ©¦32 Bs©¦16 Bs©¦1 Blocks ©¦1 Blocks  ©¦
 *©¸©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©Ø©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¼
 */

#define TCM_BASE	0x0C004000
/**
 * This Function copies SD/MMC Card Data to memory.
 * Always use EPLL source clock.
 * @param channel : HSMMC Controller channel number ( Not support. Depend on GPN15, GPN14 and GPN13 ) 
 * @param StartBlkAddress : Source card(SD/MMC) Address.(It must block address.)
 * @param blockSize : Number of blocks to copy.
 * @param memoryPtr : Buffer to copy from.
 * @param with_init : reinitialize or not 
 * @return bool(unsigend char) - Success or failure.
 */
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))


#define HSMMC_CHANNEL		0
#define MOVI_INIT_REQUIRED	0
#define BL2_BASE		CONFIG_SYS_UBOOT_BASE

/* size information */
#define SS_SIZE			(8 * 1024)      //bl1 size
#define eFUSE_SIZE		(1 * 1024)	// 0.5k eFuse, 0.5k reserved`

/* movinand definitions */
#define MOVI_BLKSIZE		512

#if 1
/**
 * Total block count of the MMC device
 */
#define MOVI_TOTAL_BLKCNT	*((volatile unsigned int*)(TCM_BASE - 0x4))

/**
 * globalSDHCInfoBit[31:16]  RCA address
 * globalSDHCInfoBit[2]      If SD card detected, this value will be set
 * globalSDHCInfoBit[1]      If MMC card detected, this value will be set
 * globalSDHCInfoBit[0]      If the SD/MMC device is operating in sector mode, this value will be set
 */
#define MOVI_HIGH_CAPACITY	*((volatile unsigned int*)(TCM_BASE - 0x8))
#define MOVI_SD_DETECT_MASK	(1<<2)
#define MOVI_MMC_DETECT_MASK	(1<<1)
#define movi_sdmmc_is_detected	(0x6 & MOVI_HIGH_CAPACITY) 
#else
#define MOVI_TOTAL_BLKCNT	7864320 // 7864320 // 3995648 // 1003520 /* static movinand total block count: for writing to movinand when nand boot */
#define MOVI_HIGH_CAPACITY	0
#endif

/* partition information */
#define PART_SIZE_BL		(512 * 1024)
#define PART_SIZE_KERNEL	(4 * 1024 * 1024)
#define PART_SIZE_ROOTFS	(8 * 1024 * 1024)

#define MOVI_LAST_BLKPOS	(MOVI_TOTAL_BLKCNT - (eFUSE_SIZE / MOVI_BLKSIZE))
#define MOVI_BL1_BLKCNT		(SS_SIZE / MOVI_BLKSIZE)
#define MOVI_ENV_BLKCNT		(CONFIG_ENV_SIZE / MOVI_BLKSIZE)
#define MOVI_BL2_BLKCNT		(PART_SIZE_BL / MOVI_BLKSIZE)
#define MOVI_ZIMAGE_BLKCNT	(PART_SIZE_KERNEL / MOVI_BLKSIZE)
#define MOVI_BL2_POS		(MOVI_LAST_BLKPOS - MOVI_BL1_BLKCNT - MOVI_BL2_BLKCNT - MOVI_ENV_BLKCNT)
#define MOVI_ROOTFS_BLKCNT	(PART_SIZE_ROOTFS / MOVI_BLKSIZE)

#define MMC_ENV_OFFSET  (0 - eFUSE_SIZE - SS_SIZE - CONFIG_ENV_SIZE)

#endif /*__MOVI_H__*/
