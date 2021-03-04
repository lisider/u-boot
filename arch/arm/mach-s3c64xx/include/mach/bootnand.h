#ifndef __BOOTNAND_H__
#define __BOOTNAND_H__

#define TCM_BASE	0x0C004000
/**
 * This Function copies a block of page to destination memory.( 8-bit ECC only )
 * @param u32 blcok : Source block address number to copy.
 * @param u32 page : Source page address number to copy.
 * @param u8 *buffer : Target Buffer pointer.
 * @return int - Success or failure.
 */ 
#define NF8_ReadPage(a,b,c) (((int(*)(u32, u32, u8 *))(*((u32 *)(TCM_BASE + 0x0))))(a,b,c)

/**
 * This Function copies a block of page to destination memory.( 8-bit ECC only )
 * @param u32 blcok : Source block address number to copy.
 * @param u32 page : Source page address number to copy.
 * @param u8 *buffer : Target Buffer pointer.
 * @return int - Success or failure.
 */ 
#define NF8_ReadPage_Adv(a,b,c) (((int(*)(u32, u32, u8 *))(*((u32 *)(TCM_BASE + 0x4))))(a,b,c)

#endif //__BOOTNAND_H__
