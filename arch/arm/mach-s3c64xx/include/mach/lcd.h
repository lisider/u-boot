#ifndef __S3CFB_LCD_H__
#define __S3CFB_LCD_H__

#ifndef LCD_COLOR24
#error "LCD_COLOR24 is still undef ,please define it first"
#endif
#define LCD_BPP		LCD_COLOR24

/**********************************************************************
 * LCD config
 **********************************************************************
 */
//timing
#define  VBP	(1+1)	//Vertical back porch
#define  VFP	(1+1)	//Vertical front porch
#define  VSPW	(9+1)		//Vertical sync pulse width
#define  HBP	(1+1)	//Horizontal back porch
#define  HFP	(1+1)	//Horizontal front porch
#define  HSPW	(40+1)	//Horizontal sync pulse width

#define REFRESH_FREQ	(70)
/**********************************************************************/
#if LCD_BPP == LCD_COLOR24
#define  BITS_PER_PIXEL		24
#define  BYTES_PER_PIXEL       4
#else //#elif LCD_BPP == LCD_COLOR16
#define  BITS_PER_PIXEL		16
#define  BYTES_PER_PIXEL       2
#endif

#define  LCD_WIDTH				(480)
#define  LCD_HEIGHT				(272)

#define  VIRT_FB_WIDTH_MAX		LCD_WIDTH
#define  VIRT_FB_HEIGHT_MAX	LCD_HEIGHT

#define  MAIN_FRAME_BUFFER_SZ 	(VIRT_FB_WIDTH_MAX * VIRT_FB_HEIGHT_MAX * BYTES_PER_PIXEL)
#define  SUB_FRAME_BUFFER_SZ	0 //(LCD_WIDTH * LCD_HEIGHT * BYTES_PER_PIXEL)

#define  TOTAL_FRAME_BUFFER_SZ	(MAIN_FRAME_BUFFER_SZ + SUB_FRAME_BUFFER_SZ)

/*graph-operate element
 */
 
void s3c_fb_init(void *lcdbase);

#define  lcd_ctrl_init  s3c_fb_init

#endif //__S3CFB_LCD_H__

