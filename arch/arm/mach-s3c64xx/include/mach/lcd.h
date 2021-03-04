#ifndef __S3CFB_LCD_H__
#define __S3CFB_LCD_H__

#ifndef LCD_COLOR16
#error "LCD_COLOR16 is still undef ,please define it first"
#endif
#define LCD_BPP		LCD_COLOR16

/**********************************************************************
 * LCD config
 **********************************************************************
 */
//timing
#define  VBP	(13)	//Vertical back porch
#define  VFP	(32)	//Vertical front porch
#define  VSPW	(2)		//Vertical sync pulse width
#define  HBP	(144)	//Horizontal back porch
#define  HFP	(16)	//Horizontal front porch
#define  HSPW	(10)	//Horizontal sync pulse width

#define REFRESH_FREQ	(70)
/**********************************************************************/
#if LCD_BPP == LCD_COLOR16
#define  BITS_PER_PIXEL		16
#define  BYTES_PER_PIXEL       2
#else //#elif LCD_BPP == LCD_COLOR32
#define  BITS_PER_PIXEL		24
#define  BYTES_PER_PIXEL       4
#endif

#define  LCD_WIDTH				(640)
#define  LCD_HEIGHT				(480)

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

