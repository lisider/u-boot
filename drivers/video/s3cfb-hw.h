#ifndef __S3CFB_HW_H__
#define __S3CFB_HW_H__

/*--------------------------------------------------------------*/
/* Video Main Control 0 register - VIDCON0 */
#define VIDCON0_INTERLACE_F_PROGRESSIVE		(0<<29)
#define VIDCON0_INTERLACE_F_INTERLACE		(1<<29)
#define VIDCON0_INTERLACE_F_MASK			(1<<29)
#define VIDCON0_VIDOUT(x)  				(((x)&0x7)<<26)
#define VIDCON0_VIDOUT_RGB_IF			(0<<26)
#define VIDCON0_VIDOUT_TV				(1<<26)
#define VIDCON0_VIDOUT_I80IF0			(2<<26)
#define VIDCON0_VIDOUT_I80IF1			(3<<26)
#define VIDCON0_VIDOUT_TVNRGBIF 			(4<<26)
#define VIDCON0_VIDOUT_TVNI80IF0			(6<<26)
#define VIDCON0_VIDOUT_TVNI80IF1			(7<<26)
#define VIDCON0_VIDOUT_MASK				(7<<26)
#define VIDCON0_L1_DATA16(x)  			(((x)&0x7)<<23)
#define VIDCON0_L1_DATA16_SUB_16_MODE		(0<<23)
#define VIDCON0_L1_DATA16_SUB_16PLUS2_MODE		(1<<23)
#define VIDCON0_L1_DATA16_SUB_9PLUS9_MODE 		(2<<23)
#define VIDCON0_L1_DATA16_SUB_16PLUS8_MODE		(3<<23)
#define VIDCON0_L1_DATA16_SUB_18_MODE		(4<<23)
#define VIDCON0_L0_DATA16(x)  			(((x)&0x7)<<20)
#define VIDCON0_L0_DATA16_MAIN_16_MODE		(0<<20)
#define VIDCON0_L0_DATA16_MAIN_16PLUS2_MODE		(1<<20)
#define VIDCON0_L0_DATA16_MAIN_9PLUS9_MODE		(2<<20)
#define VIDCON0_L0_DATA16_MAIN_16PLUS8_MODE		(3<<20)
#define VIDCON0_L0_DATA16_MAIN_18_MODE		(4<<20)
#define VIDCON0_PNRMODE(x)  			(((x)&0x3)<<17)
#define VIDCON0_PNRMODE_RGB_P			(0<<17)
#define VIDCON0_PNRMODE_BGR_P			(1<<17)
#define VIDCON0_PNRMODE_RGB_S			(2<<17)
#define VIDCON0_PNRMODE_BGR_S			(3<<17)
#define VIDCON0_PNRMODE_MASK			(3<<17)
#define VIDCON0_CLKVALUP_ALWAYS 			(0<<16)
#define VIDCON0_CLKVALUP_ST_FRM 			(1<<16)
#define VIDCON0_CLKVAL_F(x)				(((x)&0xFF)<<6)
#define VIDCON0_VCLKEN_ENABLE			(1<<5)
#define VIDCON0_CLKDIR_DIVIDED   			(1<<4)
#define VIDCON0_CLKDIR_DIRECTED  			(0<<4)
#define VIDCON0_CLKSEL(x)   			(((x)&0x3)<<2)
#define VIDCON0_CLKSEL_F_HCLK	  		(0<<2)
#define VIDCON0_ENVID_ENABLE	    		(1 << 1)	/* 0:Disable 1:Enable LCD video output and logic immediatly */
#define VIDCON0_ENVID_DISABLE	    		(0 << 1)	/* 0:Disable 1:Enable LCD video output and logic immediatly */
#define VIDCON0_ENVID_F_ENABLE     			(1 << 0)	/* 0:Dis 1:Ena wait until Current frame end. */
#define VIDCON0_ENVID_F_DISABLE     		(0 << 0)	/* 0:Dis 1:Ena wait until Current frame end. */

/* Video Main Control 1 register - VIDCON1 */
#define VIDCON1_IVCLK_FALL_EDGE 			(0<<7)
#define VIDCON1_IVCLK_RISE_EDGE 			(1<<7)
#define VIDCON1_IHSYNC_NORMAL			(0<<6)
#define VIDCON1_IHSYNC_INVERT			(1<<6)
#define VIDCON1_IVSYNC_NORMAL			(0<<5)
#define VIDCON1_IVSYNC_INVERT			(1<<5)
#define VIDCON1_IVDEN_NORMAL			(0<<4)
#define VIDCON1_IVDEN_INVERT			(1<<4)

/* Video Main Control 2 register - VIDCON2 */
#define VIDCON2_EN601_DISABLE			(0<<23)
#define VIDCON2_EN601_ENABLE			(1<<23)
#define VIDCON2_EN601_MASK				(1<<23)
#define VIDCON2_TVFORMATSEL0_HARDWARE		(0<<14)
#define VIDCON2_TVFORMATSEL0_SOFTWARE		(1<<14)
#define VIDCON2_TVFORMATSEL0_MASK			(1<<14)
#define VIDCON2_TVFORMATSEL1_RGB			(0<<12)
#define VIDCON2_TVFORMATSEL1_YUV422			(1<<12)
#define VIDCON2_TVFORMATSEL1_YUV444			(2<<12)
#define VIDCON2_TVFORMATSEL1_MASK			(0x3<<12)
#define VIDCON2_ORGYUV_YCBCR			(0<<8)
#define VIDCON2_ORGYUV_CBCRY			(1<<8)
#define VIDCON2_ORGYUV_MASK				(1<<8)
#define VIDCON2_YUVORD_CBCR				(0<<7)
#define VIDCON2_YUVORD_CRCB				(1<<7)
#define VIDCON2_YUVORD_MASK				(1<<7)

/* VIDEO Time Control 0 register - VIDTCON0 */
#define VIDTCON0_VBPDE(x)				(((x)&0xFF)<<24)
#define VIDTCON0_VBPD(x)				(((x)&0xFF)<<16)
#define VIDTCON0_VFPD(x) 				(((x)&0xFF)<<8)
#define VIDTCON0_VSPW(x) 				(((x)&0xFF)<<0)

/* VIDEO Time Control 1 register - VIDTCON1 */
#define VIDTCON1_VFPDE(x)				(((x)&0xFF)<<24)
#define VIDTCON1_HBPD(x) 				(((x)&0xFF)<<16)
#define VIDTCON1_HFPD(x) 				(((x)&0xFF)<<8)
#define VIDTCON1_HSPW(x) 				(((x)&0xFF)<<0)

/* VIDEO Time Control 2 register - VIDTCON2 */
#define VIDTCON2_LINEVAL(x)  			(((x)&0x7FF)<<11) /* these bits determine the vertical size of lcd panel */
#define VIDTCON2_HOZVAL(x)   			(((x)&0x7FF)<<0) /* these bits determine the horizontal size of lcd panel*/


/* Window 0~4 Control register - WINCONx */
#define WINCONx_WIDE_NARROW(x)			(((x)&0x3)<<26)
#define WINCONx_ENLOCAL_DMA				(0<<22)
#define WINCONx_ENLOCAL					(1<<22)
#define WINCONx_ENLOCAL_MASK			(1<<22)
#define WINCONx_BUFSTA_MASK				(1<<21)
#define WINCONx_BUFSEL_0				(0<<20)
#define WINCONx_BUFSEL_1				(1<<20)
#define WINCONx_BUFSEL_MASK				(1<<20)
#define WINCONx_BUFAUTOEN_DISABLE			(0<<19)
#define WINCONx_BUFAUTOEN_ENABLE			(1<<19)
#define WINCONx_BUFAUTOEN_MASK			(1<<19)
#define WINCONx_BITSWP_DISABLE			(0<<18)
#define WINCONx_BITSWP_ENABLE			(1<<18)
#define WINCONx_BYTSWP_DISABLE			(0<<17)
#define WINCONx_BYTSWP_ENABLE			(1<<17)
#define WINCONx_HAWSWP_DISABLE			(0<<16)
#define WINCONx_HAWSWP_ENABLE			(1<<16)
#define WINCONx_WSWP_DISABLE			(0<<15)
#define WINCONx_WSWP_ENABLE				(1<<15)
#define WINCONx_INRGB_RGB		   		(0<<13)
#define WINCONx_INRGB_YUV		 		(1<<13)
#define WINCONx_INRGB_MASK				(1<<13)
#define WINCONx_BURSTLEN_MASK			(3<<9)
#define WINCONx_BURSTLEN_16WORD			(0<<9)
#define WINCONx_BURSTLEN_8WORD			(1<<9)
#define WINCONx_BURSTLEN_4WORD			(2<<9)
#define WINCONx_BLD_PIX_PLANE			(0<<6)
#define WINCONx_BLD_PIX_PIXEL			(1<<6)
#define WINCONx_BLD_PIX_MASK			(1<<6)
#define WINCONx_BPPMODE_F_1BPP			(0<<2)
#define WINCONx_BPPMODE_F_2BPP			(1<<2)
#define WINCONx_BPPMODE_F_4BPP			(2<<2)
#define WINCONx_BPPMODE_F_8BPP_PAL			(3<<2)
#define WINCONx_BPPMODE_F_8BPP_NOPAL		(4<<2)
#define WINCONx_BPPMODE_F_16BPP_565			(5<<2)
#define WINCONx_BPPMODE_F_16BPP_A555		(6<<2)
#define WINCONx_BPPMODE_F_18BPP_666			(8<<2)
#define WINCON1_BPPMODE_F_19BPP_A1666		(10<<2)
#define WINCONx_BPPMODE_F_24BPP_888			(11<<2)
#define WINCONx_BPPMODE_F_24BPP_A887		(0xc<<2)
#define WINCONx_BPPMODE_F_25BPP_A888		(0xd<<2)
#define WINCONx_BPPMODE_F_28BPP_A888		(0xd<<2)
#define WINCONx_BPPMODE_F_MASK			(0xf<<2)
#define WINCONx_ALPHA_SEL_0				(0<<1)
#define WINCONx_ALPHA_SEL_1				(1<<1)
#define WINCONx_ALPHA_SEL_MASK			(1<<1)
#define WINCONx_ENWIN_F_DISABLE 			(0<<0)
#define WINCONx_ENWIN_F_ENABLE			(1<<0)

/* Window 1-2 Control register - WINCON1 */
#define WINCON1_LOCALSEL_TV				(0<<23)
#define WINCON1_LOCALSEL_CAMERA			(1<<23)
#define WINCON1_LOCALSEL_MASK			(1<<23)
#define WINCON2_LOCALSEL_TV				(0<<23)
#define WINCON2_LOCALSEL_CAMERA			(1<<23)
#define WINCON2_LOCALSEL_MASK			(1<<23)

/* Window 0~4 Position Control A register - VIDOSDxA */
#define VIDOSDxA_OSD_LTX_F(x)			(((x)&0x7FF)<<11)
#define VIDOSDxA_OSD_LTY_F(x)			(((x)&0x7FF)<<0)

/* Window 0~4 Position Control B register - VIDOSDxB */
#define VIDOSDxB_OSD_RBX_F(x)			(((x)&0x7FF)<<11)
#define VIDOSDxB_OSD_RBY_F(x)			(((x)&0x7FF)<<0)

/* Window 0 Position Control C register - VIDOSD0C */
#define  VIDOSD0C_OSDSIZE(x)			(((x)&0xFFFFFF)<<0)

/* Window 1~4 Position Control C register - VIDOSDxC */
#define VIDOSDxC_ALPHA0_R(x)			(((x)&0xF)<<20)
#define VIDOSDxC_ALPHA0_G(x)			(((x)&0xF)<<16)
#define VIDOSDxC_ALPHA0_B(x)			(((x)&0xF)<<12)
#define VIDOSDxC_ALPHA1_R(x)			(((x)&0xF)<<8)
#define VIDOSDxC_ALPHA1_G(x)			(((x)&0xF)<<4)
#define VIDOSDxC_ALPHA1_B(x)			(((x)&0xF)<<0)

/* Window 1~2 Position Control D register - VIDOSDxD */
#define  VIDOSDxD_OSDSIZE(x)			(((x)&0xFFFFFF)<<0)

/* Frame buffer Start Address register - VIDWxxADD0 */
#define VIDWxxADD0_VBANK_F(x) 			(((x)&0xFF)<<23) /* the end address of the LCD frame buffer. */
#define VIDWxxADD0_VBASEU_F(x)			(((x)&0xFFFFFF)<<0) /* Virtual screen offset size (the number of byte). */

/* Frame buffer End Address register - VIDWxxADD1 */
#define VIDWxxADD1_VBASEL_F(x) 			(((x)&0xFFFFFF)<<0)  /* the end address of the LCD frame buffer. */

/* Frame buffer Size register - VIDWxxADD2 */
#define VIDWxxADD2_OFFSIZE_F(x)  			(((x)&0x1FFF)<<13) /* Virtual screen offset size (the number of byte). */
#define VIDWxxADD2_PAGEWIDTH_F(x)			(((x)&0x1FFF)<<0) /* Virtual screen page width (the number of byte). */

/* VIDEO Interrupt Control 0 register - VIDINTCON0 */
#define VIDINTCON0_FIFOINTERVAL(x)			(((x)&0x3F)<<20)
#define VIDINTCON0_SYSMAINCON_DISABLE		(0<<19)
#define VIDINTCON0_SYSMAINCON_ENABLE		(1<<19)
#define VIDINTCON0_SYSSUBCON_DISABLE		(0<<18)
#define VIDINTCON0_SYSSUBCON_ENABLE			(1<<18)
#define VIDINTCON0_SYSIFDONE_DISABLE		(0<<17)
#define VIDINTCON0_SYSIFDONE_ENABLE			(1<<17)
#define VIDINTCON0_FRAMESEL0_BACK			(0<<15)
#define VIDINTCON0_FRAMESEL0_VSYNC 			(1<<15)
#define VIDINTCON0_FRAMESEL0_ACTIVE			(2<<15)
#define VIDINTCON0_FRAMESEL0_FRONT 			(3<<15)
#define VIDINTCON0_FRAMESEL0_MASK 			(3<<15)
#define VIDINTCON0_FRAMESEL1_NONE			(0<<13)
#define VIDINTCON0_FRAMESEL1_BACK			(1<<13)
#define VIDINTCON0_FRAMESEL1_VSYNC 			(2<<13)
#define VIDINTCON0_FRAMESEL1_FRONT 			(3<<13)
#define VIDINTCON0_INTFRMEN_DISABLE			(0<<12)
#define VIDINTCON0_INTFRMEN_ENABLE 			(1<<12)
#define VIDINTCON0_FRAMEINT_MASK			(0x1F<<12)
#define VIDINTCON0_FIFOSEL_WIN4			(1<<11)
#define VIDINTCON0_FIFOSEL_WIN3			(1<<10)
#define VIDINTCON0_FIFOSEL_WIN2			(1<<9)
#define VIDINTCON0_FIFOSEL_WIN1			(1<<6)
#define VIDINTCON0_FIFOSEL_WIN0			(1<<5)
#define VIDINTCON0_FIFOSEL_ALL			(0x73<<5)
#define VIDINTCON0_FIFOLEVEL_25			(0<<2)
#define VIDINTCON0_FIFOLEVEL_50			(1<<2)
#define VIDINTCON0_FIFOLEVEL_75			(2<<2)
#define VIDINTCON0_FIFOLEVEL_EMPTY 			(3<<2)
#define VIDINTCON0_FIFOLEVEL_FULL			(4<<2)
#define VIDINTCON0_INTFIFOEN_DISABLE		(0<<1)
#define VIDINTCON0_INTFIFOEN_ENABLE			(1<<1)
#define VIDINTCON0_INTEN_DISABLE			(0<<0)
#define VIDINTCON0_INTEN_ENABLE			(1<<0)
#define VIDINTCON0_INTEN_MASK			(1<<0)

/* VIDEO Interrupt Control 1 register - VIDINTCON1 */
#define VIDINTCON1_INTI80PEND			(0<<2)
#define VIDINTCON1_INTFRMPEND			(1<<1)
#define VIDINTCON1_INTFIFOPEND			(1<<0)

/* WIN 1~4 Color Key 0 register - WxKEYCON0 */
#define WxKEYCON0_KEYBLEN_DISABLE 			(0<<26)
#define WxKEYCON0_KEYBLEN_ENABLE			(1<<26)
#define WxKEYCON0_KEYEN_F_DISABLE 			(0<<25)
#define WxKEYCON0_KEYEN_F_ENABLE			(1<<25)
#define WxKEYCON0_DIRCON_MATCH_FG_IMAGE		(0<<24)
#define WxKEYCON0_DIRCON_MATCH_BG_IMAGE		(1<<24)
#define WxKEYCON0_COMPKEY(x)			(((x)&0xFFFFFF)<<0)

/* WIN 1~4 Color Key 1 register - WxKEYCON1 */
#define WxKEYCON1_COLVAL(x)				(((x)&0xFFFFFF)<<0)

/* Dithering Control 1 register - DITHMODE */
#define DITHMODE_RDITHPOS_8BIT			(0<<5)
#define DITHMODE_RDITHPOS_6BIT			(1<<5)
#define DITHMODE_RDITHPOS_5BIT			(2<<5)
#define DITHMODE_GDITHPOS_8BIT			(0<<3)
#define DITHMODE_GDITHPOS_6BIT			(1<<3)
#define DITHMODE_GDITHPOS_5BIT			(2<<3)
#define DITHMODE_BDITHPOS_8BIT			(0<<1)
#define DITHMODE_BDITHPOS_6BIT			(1<<1)
#define DITHMODE_BDITHPOS_5BIT			(2<<1)
#define DITHMODE_RGB_DITHPOS_MASK			(0x3f<<1)
#define DITHMODE_DITHERING_DISABLE			(0<<0)
#define DITHMODE_DITHERING_ENABLE			(1<<0)
#define DITHMODE_DITHERING_MASK			(1<<0)

/* Window 0~4 Color map register - WINxMAP */
#define WINxMAP_MAPCOLEN_F_ENABLE			(1<<24)
#define WINxMAP_MAPCOLEN_F_DISABLE			(0<<24)
#define WINxMAP_MAPCOLOR				(((x)&0xFFFFFF)<<0)

/* Window Palette Control register - WPALCON */
#define WPALCON_PALUPDATEEN				(1<<9)
#define WPALCON_W4PAL_16BIT_A	 		(1<<8)		/* A:5:5:5 */
#define WPALCON_W4PAL_16BIT	 			(0<<8)		/*  5:6:5 */
#define WPALCON_W3PAL_16BIT_A	 		(1<<7)		/* A:5:5:5 */
#define WPALCON_W3PAL_16BIT	 			(0<<7)		/*  5:6:5 */
#define WPALCON_W2PAL_16BIT_A	 		(1<<6)		/* A:5:5:5 */
#define WPALCON_W2PAL_16BIT	 			(0<<6)		/*  5:6:5 */
#define WPALCON_W1PAL_25BIT_A	 		(0<<3)		/* A:8:8:8 */
#define WPALCON_W1PAL_24BIT				(1<<3)		/*  8:8:8 */
#define WPALCON_W1PAL_19BIT_A			(2<<3)		/* A:6:6:6 */
#define WPALCON_W1PAL_18BIT_A	 		(3<<3)		/* A:6:6:5 */
#define WPALCON_W1PAL_18BIT				(4<<3)		/*  6:6:6 */
#define WPALCON_W1PAL_16BIT_A	 		(5<<3)		/* A:5:5:5 */
#define WPALCON_W1PAL_16BIT	 			(6<<3)		/*  5:6:5 */
#define WPALCON_W0PAL_25BIT_A	 		(0<<0)		/* A:8:8:8 */
#define WPALCON_W0PAL_24BIT				(1<<0)		/*  8:8:8 */
#define WPALCON_W0PAL_19BIT_A			(2<<0)		/* A:6:6:6 */
#define WPALCON_W0PAL_18BIT_A	 		(3<<0)		/* A:6:6:5 */
#define WPALCON_W0PAL_18BIT				(4<<0)		/*  6:6:6 */
#define WPALCON_W0PAL_16BIT_A	 		(5<<0)		/* A:5:5:5 */
#define WPALCON_W0PAL_16BIT	 			(6<<0)		/*  5:6:5 */

/* I80/RGB Trigger Control register - TRIGCON */
#define TRIGCON_SWFRSTATUS_REQUESTED		(1<<2)
#define TRIGCON_SWFRSTATUS_NOT_REQUESTED		(0<<2)
#define TRIGCON_SWTRGCMD				(1<<1)
#define TRIGCON_TRGMODE_ENABLE			(1<<0)
#define TRIGCON_TRGMODE_DISABLE			(0<<0)

/* LCD I80 Interface Control 0 register - I80IFCONA0 */
#define I80IFCONAx_LCD_CS_SETUP(x) 			(((x)&0xF)<<16)
#define I80IFCONAx_LCD_WR_SETUP(x) 			(((x)&0xF)<<12)
#define I80IFCONAx_LCD_WR_ACT(x)			(((x)&0xF)<<8)
#define I80IFCONAx_LCD_WR_HOLD(x)			(((x)&0xF)<<4)


/***************************************************************************/
/*HOST IF registers */
/* Host I/F A - */
#define HOSTIFAREG(x)				((x) + S3C64XX_VA_HOSTIFA)
#define HOSTIFAREG_PHYS(x)				((x) + S3C64XX_PA_HOSTIFA)

/* Host I/F B - Modem I/F */
#define HOSTIFBREG(x)				((x) + S3C64XX_VA_HOSTIFB)
#define HOSTIFBREG_PHYS(x)				((x) + S3C64XX_PA_HOSTIFB)

#define HOSTIFB_INT2AP				HOSTIFBREG(0x8000)
#define HOSTIFB_INT2MSM				HOSTIFBREG(0x8004)
#define HOSTIFB_MIFCON				HOSTIFBREG(0x8008)
#define HOSTIFB_MIFPCON				HOSTIFBREG(0x800C)
#define HOSTIFB_MSMINTCLR				HOSTIFBREG(0x8010)

#define HOSTIFB_MIFCON_INT2MSM_DIS			(0x0<<3)
#define HOSTIFB_MIFCON_INT2MSM_EN			(0x1<<3)
#define HOSTIFB_MIFCON_INT2AP_DIS			(0x0<<2)
#define HOSTIFB_MIFCON_INT2AP_EN			(0x1<<2)
#define HOSTIFB_MIFCON_WAKEUP_DIS			(0x0<<1)
#define HOSTIFB_MIFCON_WAKEUP_EN			(0x1<<1)

#define HOSTIFB_MIFPCON_SEL_VSYNC_DIR_OUT		(0x0<<5)
#define HOSTIFB_MIFPCON_SEL_VSYNC_DIR_IN		(0x1<<5)
#define HOSTIFB_MIFPCON_INT2M_LEVEL_DIS		(0x0<<4)
#define HOSTIFB_MIFPCON_INT2M_LEVEL_EN		(0x1<<4)
#define HOSTIFB_MIFPCON_SEL_NORMAL			(0x0<<3)
#define HOSTIFB_MIFPCON_SEL_BYPASS			(0x1<<3)

#define HOSTIFB_MIFPCON_SEL_RS0			0
#define HOSTIFB_MIFPCON_SEL_RS1			1
#define HOSTIFB_MIFPCON_SEL_RS2			2
#define HOSTIFB_MIFPCON_SEL_RS3			3
#define HOSTIFB_MIFPCON_SEL_RS4			4
#define HOSTIFB_MIFPCON_SEL_RS5			5
#define HOSTIFB_MIFPCON_SEL_RS6			6

#endif //__S3CFB_HW_H__

