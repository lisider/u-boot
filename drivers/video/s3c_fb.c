#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/list.h>
#include <linux/fb.h>
#include <asm/io.h>
#include <lcd.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/gpio-regs.h>
#include <asm/arch/gpio-func.h>
#include "s3cfb-hw.h"

#ifdef CONFIG_S3C_LCD_LOGO
#include <bmp_logo.h>
#include <bmp_logo_data.h>

#define  palette_used  (BMP_LOGO_COLORS != 0)
#define  LOGO_WIN    (1)   //do not to be win0

#if LCD_BPP == LCD_COLOR16
#define  COLOR_KEY(r,g,b)   (((r&0x1F)<<19) | ((g&0x3F)<<10)|((b&0x1F)<<3))
#define  COMP_KEY(r,g,b)     (COLOR_KEY(r,g,b) | 0x737)
#else //#elif LCD_BPP == LCD_COLOR32
#define  COLOR_KEY(r,g,b)   (((r&0xFF)<<16) | ((g&0xFF)<<8)|(b&0xFF))
#define COMP_KEY(r,g,b)      COLOR_KEY(r,g,b)  
#endif
#endif //#ifdef CONFIG_S3C_LCD_LOGO

/* S3C_FB_MAX_WIN
 * Set to the maximum number of windows that any of the supported hardware
 * can use. Since the platform data uses this for an array size, having it
 * set to the maximum of any version of the hardware can do is safe.
 */
#define S3C_FB_MAX_WIN	(5)

typedef struct color_mode {
	u32 bits_per_pixel; /*bits per pixel*/
	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/		
}color_mode_t;

typedef struct s3c_fb_info {
	struct fb_info fb;
	struct fb_videomode mode;
	color_mode_t cm[S3C_FB_MAX_WIN];
}s3cfb_info_t;

static s3cfb_info_t g_fb_info;

/**
 * @valid_bpp: 1 bit per BPP setting to show valid bits-per-pixel.
 *
 * valid_bpp bit x is set if (x+1)BPP is supported.
 */
struct fb_win_variant {
	u32		valid_bpp;
};

#define VALID_BPP(x) (1 << ((x) - 1))
#define VALID_BPP124 (VALID_BPP(1) | VALID_BPP(2) | VALID_BPP(4))
#define VALID_BPP1248 (VALID_BPP124 | VALID_BPP(8))

static struct fb_win_variant fb_wins_priv[] = {
	{
		VALID_BPP1248 | VALID_BPP(16) | VALID_BPP(24),
	},
	{
		(VALID_BPP1248 | VALID_BPP(16) | VALID_BPP(18)
			| VALID_BPP(19) | VALID_BPP(24) | VALID_BPP(25)),
	},
	{
		(VALID_BPP1248 | VALID_BPP(16) | VALID_BPP(18)
			| VALID_BPP(19) | VALID_BPP(24) | VALID_BPP(25)),
	},
	{
		(VALID_BPP124  | VALID_BPP(16) | VALID_BPP(18)
			| VALID_BPP(19) | VALID_BPP(24) | VALID_BPP(25)),
	},
	{
		( VALID_BPP(1) | VALID_BPP(2) | VALID_BPP(16) 
			| VALID_BPP(18) | VALID_BPP(24) | VALID_BPP(25)),
	},
};

vidinfo_t panel_info = {
	LCD_WIDTH, //LCD width
	LCD_HEIGHT,//LCD height
	0,       //rotation of display: 0~3
	LCD_BPP, //bits per pixel = (1 << LCD_BPP)
	NULL,
	NULL
};

static int check_win_bpp(int win,int bpp)
{
	if(0 == ((1<<(bpp-1))&(fb_wins_priv[win].valid_bpp)))
		return -1;
	return 0;
}

static int fill_color_mode(color_mode_t *cm)
{
	/* always ensure these are zero, for drop through cases below */
	cm->transp.offset = 0;
	cm->transp.length = 0;

	switch (cm->bits_per_pixel) {
	case 1:
	case 2:
	case 4:
	case 8:
			cm->red.offset	= 0;
			cm->red.length	= cm->bits_per_pixel;
			cm->green	= cm->red;
			cm->blue	= cm->red;
		break;

	case 19:
		/* 666 with one bit alpha/transparency */
		cm->transp.offset	= 18;
		cm->transp.length	= 1;
	case 18:
		cm->bits_per_pixel	= 32;

		/* 666 format */
		cm->red.offset		= 12;
		cm->green.offset	= 6;
		cm->blue.offset	= 0;
		cm->red.length		= 6;
		cm->green.length	= 6;
		cm->blue.length	= 6;
		break;

	case 16:
		/* 16 bpp, 565 format */
		cm->red.offset		= 11;
		cm->green.offset	= 5;
		cm->blue.offset	= 0;
		cm->red.length		= 5;
		cm->green.length	= 6;
		cm->blue.length	= 5;
		break;

	case 28:
	case 25:
		cm->transp.length	= cm->bits_per_pixel - 24;
		cm->transp.offset	= 24;
		/* drop through */
	case 24:
		/* our 24bpp is unpacked, so 32bpp */
		cm->bits_per_pixel	= 32;
	case 32:
		cm->red.offset		= 16;
		cm->red.length		= 8;
		cm->green.offset	= 8;
		cm->green.length	= 8;
		cm->blue.offset	= 0;
		cm->blue.length	= 8;
		break;

	default:
		return -1;
	}

	return 0;
}

//Select NORMAL MODE for LCD bypass
static void lcd_bypass_disable(void)
{
	S3C64XX_MODEM *pmodem=S3C64XX_GetBase_MODEM();
	pmodem->MIFPCON  &= ~0x08;
}

static int fill_fb_info(struct fb_info *info)
{
	int win;
	struct fb_videomode *vm = info->mode;
	struct fb_var_screeninfo *var = &info->var;
	color_mode_t  *cm = (color_mode_t*)info->par;

	vm->refresh 	= REFRESH_FREQ;
	vm->left_margin	= HBP;
	vm->right_margin= HFP;
	vm->upper_margin= VBP;
	vm->lower_margin= VFP;
	vm->hsync_len	= HSPW;
	vm->vsync_len	= VSPW;
	vm->xres		= LCD_WIDTH;
	vm->yres		= LCD_HEIGHT;

	var->xres_virtual = VIRT_FB_WIDTH_MAX;
	var->yres_virtual = VIRT_FB_HEIGHT_MAX;
	var->xoffset = 0;
	var->yoffset = 0;

	cm[0].bits_per_pixel = BITS_PER_PIXEL;
	cm[1].bits_per_pixel = BITS_PER_PIXEL;
	cm[2].bits_per_pixel = BITS_PER_PIXEL;
	cm[3].bits_per_pixel = BITS_PER_PIXEL;
	cm[4].bits_per_pixel = BITS_PER_PIXEL;
	
	for(win=0;win<2;win++) {
		if(check_win_bpp(win, cm[win].bits_per_pixel))
			return -1;
		fill_color_mode(&cm[win]);
	}
	return 0;
}

#ifdef CONFIG_S3C_LCD_LOGO
static int fbinfo_bmplogo_init(struct fb_info *info)
{
	uchar *bmap = &bmp_logo_bitmap[0];
	struct fb_pixmap *img = &info->pixmap;
	ushort *fb16;
	ushort i,j;
#if LCD_BPP == LCD_COLOR16
#define ACTUAL_WIDTH (( (BMP_LOGO_WIDTH)>>1)<<1)  /* need 2-align when bpp16*/
#else //#elif LCD_BPP == LCD_COLOR32
#define ACTUAL_WIDTH BMP_LOGO_WIDTH  		
#endif
#define ACTUAL_LINE_SIZE (ACTUAL_WIDTH *BYTES_PER_PIXEL)

	img->blit_x=ACTUAL_WIDTH;
	img->blit_y=BMP_LOGO_HEIGHT;
	img->size = BMP_LOGO_HEIGHT * ACTUAL_LINE_SIZE;
	img->addr = NULL;
	fb16 = (ushort *)malloc(img->size);
	if(fb16)
		img->addr = (u8 *)fb16;
	else
		return -1;
	
	for (i = 0; i < BMP_LOGO_HEIGHT; ++i) {
		for (j = 0; j < ACTUAL_WIDTH; j++) {
			if(palette_used) {
				fb16[j] = bmp_logo_palette[bmap[j]-BMP_LOGO_OFFSET];
			} else {
				fb16[j] = (bmap[j*2+1]<<8)|bmap[j*2]; 
			}
		}
		bmap+=BMP_LOGO_WIDTH *BYTES_PER_PIXEL;
		fb16 += ACTUAL_WIDTH;
	}

	return 0;
}

static void set_logo_addr_and_osd(struct fb_info *info,int win)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	struct fb_pixmap *img = &info->pixmap;
	color_mode_t *cm=(color_mode_t*)info->par;
	struct fb_videomode *mode=info->mode;
	u32 pagewidth,offset;

	if(win == 0) return ; /* not allowed */
	
	pagewidth = img->blit_x * cm[win].bits_per_pixel/8;
	offset = 0;
	plcd->VIDWxADD0[win][0] = (ulong)img->addr;
	plcd->VIDWxADD1[win][0] = (ulong)img->addr + img->size;
	plcd->VIDWxADD2[win]    = VIDWxxADD2_OFFSIZE_F(offset)|VIDWxxADD2_PAGEWIDTH_F(pagewidth);
	plcd->VIDWxADD0[win][1] = plcd->VIDWxADD0[win][0];
	plcd->VIDWxADD1[win][1] = plcd->VIDWxADD1[win][0];
	plcd->window[win].VIDOSDA = VIDOSDxA_OSD_LTX_F(mode->xres - img->blit_x)|VIDOSDxA_OSD_LTY_F(0);
	plcd->window[win].VIDOSDB = VIDOSDxB_OSD_RBX_F(mode->xres - 1) |  
								VIDOSDxB_OSD_RBY_F(img->blit_y - 1);
	plcd->window[win].VIDOSDC = VIDOSDxC_ALPHA0_R(0x0A) | VIDOSDxC_ALPHA0_G(0x0A) | VIDOSDxC_ALPHA0_B(0x0A) | 
								VIDOSDxC_ALPHA1_R(0x00) | VIDOSDxC_ALPHA1_G(0x00) | VIDOSDxC_ALPHA1_B(0x00);
	 if (win < 3)
		plcd->window[win].VIDOSDD = img->blit_x * img->blit_y;
	 plcd->WxKEYCON[win-1][0] =  WxKEYCON0_KEYBLEN_ENABLE | \
	 							WxKEYCON0_KEYEN_F_ENABLE | \
	 							WxKEYCON0_DIRCON_MATCH_FG_IMAGE | \
	 							COMP_KEY(0,0,0);
	 plcd->WxKEYCON[win-1][1] =  COLOR_KEY(0,0,0);
}

#endif //CONFIG_S3C_LCD_LOGO

static int config_lcd_clk(ulong clk)
{
	ulong r,mclk,rate;
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();

	mclk=s3c_clk_get_rate_by_id(CLK_ID_DOUT_MPLL);

	printf("LCD source clock:%ld Hz, want: %ld Hz\n",mclk,clk);
	rate = mclk/clk;
	if(rate == 0) {
		return -1;
	} else if(rate > 256*16) {
		return -2;
	}
	if(rate > 256)	r = rate>>8;
	else  r = 1;
	s3c_clk_set_parent_by_id(CLK_ID_SCLK_LCD, CLK_ID_DOUT_MPLL);
	s3c_clk_set_rate_by_id(CLK_ID_SCLK_LCD,mclk/r);
	rate = rate/r;

	//config lcd clk from SYSCON
	r = plcd->VIDCON[0] ;
	r = (r & ~(3<<2)) | (1<<2);
	//config VCLK = CLKin/(clkval+1),where clkval>=1
	r = (r & ~(0xFF<<6)) |((rate-1)<<6);//clkval=1,So,VCLK=CLKLCD/2
	plcd->VIDCON[0] = r;
	//allow CLOCK to be divided by clkval
	plcd->VIDCON[0] = r | (1<<4);

	//enable the CLK_LCD output gate
	s3c_clk_enable_by_id(CLK_ID_SCLK_LCD, 1);

	printf("LCD working VCLK:%ld Hz\n",s3c_clk_get_rate_by_id(CLK_ID_SCLK_LCD)/rate);
	
	return 0;
}

static int set_lcd_pixclock(struct fb_videomode *mode)
{
	ulong pixclk;
	pixclk = mode->left_margin + mode->hsync_len + mode->right_margin + mode->xres;
	pixclk *= mode->upper_margin + mode->vsync_len + mode->lower_margin + mode->yres;
	pixclk *= mode->refresh;

	return config_lcd_clk(pixclk);
}

static void setup_backlight_on(void)
{
	//config LCD backlight pin
	gpio_set_cfg(S3C64XX_GPF(14),  S3C_GPIO_FUNC(0x2));/* pwm func */
	gpio_set_pull(S3C64XX_GPF(14), S3C_GPIO_PULL_UP);
	//gpio_set_value(S3C64XX_GPF(14), 1); /*  light on */
}

static void setup_lcd_gpio(void)
{
	unsigned start = 0, end = 0,func;
	ulong val = readl(S3C64XX_SPCON);
	val &= ~0x3;
	val |= 0x1;
	writel(val, S3C64XX_SPCON);//config LCD I/F pin - RGB I/F style

	//config LCD interface Pin
	start = S3C64XX_GPI(0);
	end  = S3C64XX_GPI(15);
	func = S3C_GPIO_FUNC(0x2);
	for (; start <= end ; start++) {
		gpio_set_cfg(start,  func);
		gpio_set_pull(start, S3C_GPIO_PULL_NONE);
	}
	start = S3C64XX_GPJ(0);
	end  = S3C64XX_GPJ(11);
	func = S3C_GPIO_FUNC(0x2);
	for (; start <= end ; start++) {
		gpio_set_cfg(start,  func);
		gpio_set_pull(start, S3C_GPIO_PULL_NONE);
	}
}


/**
 * fb_clear_win() - clear hardware window registers.
 * @sfb: The base resources for the hardware.
 * @win: The window to process.
 *
 * Reset the specific window registers to a known state.
 */
static void fb_clear_win(void)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	int n;
	for(n=0;n<S3C_FB_MAX_WIN;n++) {
		plcd->WINCON[n] = 0;
		plcd->window[n].VIDOSDA = 0;
		plcd->window[n].VIDOSDB = 0;
		plcd->window[n].VIDOSDC = 0;
	}
}

static void set_colorkey_default(void)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	int n;
	for(n=0;n<S3C_FB_MAX_WIN-1;n++) {
		plcd->WxKEYCON[n][0] = 0xFFFFFF;
		plcd->WxKEYCON[n][1] = 0xFFFFFF;
	}
}

static void set_lcd_timing(struct fb_videomode *mode)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	u32 data;

	data = VIDTCON0_VBPD(mode->upper_margin - 1) |
	       VIDTCON0_VFPD(mode->lower_margin - 1) |
	       VIDTCON0_VSPW(mode->vsync_len - 1);
	plcd->VIDTCON[0] = data;//set frame timing,VBP=13 lines,VFP=32 lines,VSPW=2 lines

	data = VIDTCON1_HBPD(mode->left_margin - 1) |
	       VIDTCON1_HFPD(mode->right_margin - 1) |
	       VIDTCON1_HSPW(mode->hsync_len - 1);
	plcd->VIDTCON[1] = data;//HBP=144 tclk,HFP=16 tclk;HSPW=10 tclk

	data = VIDTCON2_LINEVAL(mode->yres - 1) |
	       VIDTCON2_HOZVAL(mode->xres - 1);
	plcd->VIDTCON[2] = data;
}

static void set_win_addr_and_osd(struct fb_info *info)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	color_mode_t *cm=(color_mode_t*)info->par;
	struct fb_videomode *mode=info->mode;
	u32 pagewidth,offset,frame_size;
	int win;

	pagewidth = mode->xres * cm[0].bits_per_pixel/8;
	offset = (info->var.xres_virtual - mode->xres) * cm[0].bits_per_pixel/8;
	frame_size = (pagewidth + offset) * mode->yres;
	for(win=0;win<S3C_FB_MAX_WIN;win++) {
		plcd->VIDWxADD0[win][0] = info->fix.smem_start;
		plcd->VIDWxADD1[win][0] = info->fix.smem_start + frame_size;
		plcd->VIDWxADD2[win]    = VIDWxxADD2_OFFSIZE_F(offset)|VIDWxxADD2_PAGEWIDTH_F(pagewidth);
		if(win<2) {
			plcd->VIDWxADD0[win][1] = info->fix.smem_start + info->fix.smem_len;
			plcd->VIDWxADD1[win][1] = info->fix.smem_start + info->fix.smem_len +  
										mode->xres*mode->yres*cm[0].bits_per_pixel/8;
		}

		plcd->window[win].VIDOSDA = VIDOSDxA_OSD_LTX_F(0)|VIDOSDxA_OSD_LTY_F(0);
		plcd->window[win].VIDOSDB = VIDOSDxB_OSD_RBX_F(mode->xres - 1) |  
									VIDOSDxB_OSD_RBY_F(mode->yres - 1);
		if(win != 0) {
			plcd->window[win].VIDOSDC = VIDOSDxC_ALPHA0_R(0x0F) | VIDOSDxC_ALPHA0_G(0x0F) | VIDOSDxC_ALPHA0_B(0x0F) | 
										VIDOSDxC_ALPHA1_R(0x00) | VIDOSDxC_ALPHA1_G(0x00) | VIDOSDxC_ALPHA1_B(0x00);
		}
	}

	frame_size = mode->xres * mode->yres;
	plcd->window[0].VIDOSDC = frame_size;
	plcd->window[1].VIDOSDD = frame_size;
	plcd->window[2].VIDOSDD = frame_size;
}

static void set_win_mode(int win,color_mode_t *cm)
{
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	u32 data=0;
	
	switch (cm->bits_per_pixel) {
	case 1:
		data |= WINCONx_BPPMODE_F_1BPP;
		data |= WINCONx_BITSWP_ENABLE;
		data |= WINCONx_BURSTLEN_4WORD;
		break;
	case 2:
		data |= WINCONx_BPPMODE_F_2BPP;
		data |= WINCONx_BITSWP_ENABLE;
		data |= WINCONx_BURSTLEN_8WORD;
		break;
	case 4:
		data |= WINCONx_BPPMODE_F_4BPP;
		data |= WINCONx_BITSWP_ENABLE;
		data |= WINCONx_BURSTLEN_8WORD;
		break;
	case 8:
		if (cm->transp.length != 0)
			data |= WINCONx_BPPMODE_F_8BPP_NOPAL;
		else
			data |= WINCONx_BPPMODE_F_8BPP_PAL;
		data |= WINCONx_BURSTLEN_8WORD;
		data |= WINCONx_BYTSWP_ENABLE;
		break;
	case 16:
		if (cm->transp.length != 0)
			data |= WINCONx_BPPMODE_F_16BPP_A555;
		else
			data |= WINCONx_BPPMODE_F_16BPP_565;
		data |= WINCONx_HAWSWP_ENABLE;
		data |= WINCONx_BURSTLEN_16WORD;
#ifdef CONFIG_S3C_LCD_LOGO
		if(win == LOGO_WIN)
			data |= WINCONx_BLD_PIX_PIXEL;
#endif
		break;
	case 24:
	case 32:
		if (cm->red.length == 6) {
			if (cm->transp.length != 0)
				data |= WINCON1_BPPMODE_F_19BPP_A1666;
			else
				data |= WINCONx_BPPMODE_F_18BPP_666;
		} else if (cm->transp.length == 1)
			data |= WINCONx_BPPMODE_F_25BPP_A888
				| WINCONx_BLD_PIX_PIXEL;
		else if (cm->transp.length == 4)
			data |= WINCONx_BPPMODE_F_28BPP_A888
				| WINCONx_BLD_PIX_PIXEL | WINCONx_ALPHA_SEL_1;
		else
			data |= WINCONx_BPPMODE_F_24BPP_888;

		//data |= WINCONx_WSWP_ENABLE;
		data |= WINCONx_BURSTLEN_16WORD;
		break;
	}

	plcd->WINCON[win] = data;
	plcd->WINxMAP[win] = 0;
}

// Disable the video output and the Display control signal
static void lcd_disable (void)
{//win 0 disabale
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	plcd->VIDCON[0] &= ~(VIDCON0_ENVID_ENABLE | VIDCON0_ENVID_F_ENABLE);
}

// Enable the video output and the Display control signal
void lcd_enable (void)
{//win0 output
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	plcd->VIDCON[0] |= (VIDCON0_ENVID_ENABLE | VIDCON0_ENVID_F_ENABLE);
}

void s3c_fb_init(void *lcdbase)
{
	pr_info("SUDEBUG : %s,%s,line = %d\n",__FILE__,__func__,__LINE__);
	S3C64XX_LCD *plcd=S3C64XX_GetBase_LCD();
	struct fb_info *info = (struct fb_info *)&g_fb_info;
	int i;

	memset(info,0,sizeof(s3cfb_info_t));
	info->par=g_fb_info.cm;
	info->mode=&g_fb_info.mode;

	lcd_disable();
	
	/* 2'b00 - output format:RGB I/F */
	plcd->VIDCON[0] = VIDCON0_PNRMODE_RGB_P | VIDCON0_VIDOUT_RGB_IF;
	/* config the polarity of LCD control signals in RGB I/F */
	plcd->VIDCON[1] = VIDCON1_IVSYNC_INVERT | VIDCON1_IHSYNC_INVERT;

	lcd_bypass_disable();

	info->fix.smem_start = virt_to_phys(lcdbase);
	info->fix.smem_len	 = MAIN_FRAME_BUFFER_SZ;

	if(fill_fb_info(info)) {
		printf("fb:fail to fill fb_info!\n");
		return;
	}

#ifdef CONFIG_S3C_LCD_LOGO
	if(fbinfo_bmplogo_init(info)) {
		printf("fb:fail to malloc for bmp logo!\n");
		return;
	}
#endif // CONFIG_S3C_LCD_LOGO

	//LCD_CLK init
	if((i=set_lcd_pixclock(info->mode))<0)
	{
		printf("fb:require too %s clock\n",(-1==i)?"big":"small");
#ifdef CONFIG_S3C_LCD_LOGO
		if(info->pixmap.addr && (bmp_logo_bitmap !=info->pixmap.addr))
			free(info->pixmap.addr);
#endif // CONFIG_S3C_LCD_LOGO
		return;
	}
	setup_lcd_gpio();

	fb_clear_win();
	set_colorkey_default();

	set_lcd_timing(info->mode);
	set_win_addr_and_osd(info);
#ifdef CONFIG_S3C_LCD_LOGO
	set_logo_addr_and_osd(info,LOGO_WIN);
#endif // CONFIG_S3C_LCD_LOGO

	for(i=0;i<S3C_FB_MAX_WIN;i++)
		set_win_mode(i,&g_fb_info.cm[i]);

	//config palette memory access ,and palette format
	plcd->WPALCON = (0<<1)|(4<<3)|4;//0:LCD controller access;format:window 0/1 -18bit 666

	//main fb clean,as default color: black
	//for(i=0; i < MAIN_FRAME_BUFFER_SZ/4; i++)
	//    ((ulong *)lcdbase)[i] = 0x00000000;

	//enable window0 video out
	//for(i=0;i<2;i++)
		plcd->WINCON[0] |= 0x1;

#ifdef CONFIG_S3C_LCD_LOGO
	plcd->WINCON[LOGO_WIN] |= 0x1;
#endif // CONFIG_S3C_LCD_LOGO

	// Enable the video output and the Display control signal
	lcd_enable();

	setup_backlight_on();

	/* Enable flushing if we enabled dcache */
	lcd_set_flush_dcache(1);
}

/* There is nothing to do with color registers, we use true color */
void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}

