/* linux/arch/arm/mach-s3c6410/include/mach/dma.h
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * S3C6410 - DMA support
 */

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H __FILE__
#include "pl080.h"

#define S3C_DMA_CHANNELS	(16)

#define ELFIN_DMA0_BASE      0x75000000
#define ELFIN_DMA1_BASE      0x75100000
#define ELFIN_SDMA0_BASE     0x7DB00000
#define ELFIN_SDMA1_BASE     0x7DC00000

#ifndef __ASSEMBLY__

/* see mach-s3c2410/dma.h for notes on dma channel numbers */

/* Note, for the S3C64XX architecture we keep the DMACH_
 * defines in the order they are allocated to [S]DMA0/[S]DMA1
 * so that is easy to do DHACH_ -> DMA controller conversion
 */
enum s3c_dma_ch_id {
	/* DMA0/SDMA0 */
	DMACH_DT_PROP = -1, /* not yet supported, do not use */
	DMACH_UART0 = 0,
	DMACH_UART0_SRC2,
	DMACH_UART1,
	DMACH_UART1_SRC2,
	DMACH_UART2,
	DMACH_UART2_SRC2,
	DMACH_UART3,
	DMACH_UART3_SRC2,
	DMACH_PCM0_TX,
	DMACH_PCM0_RX,
	DMACH_I2S0_OUT,
	DMACH_I2S0_IN,
	DMACH_SPI0_TX,
	DMACH_SPI0_RX,
	DMACH_HSI_I2SV40_TX,
	DMACH_HSI_I2SV40_RX,

	/* DMA1/SDMA1 */
	DMACH_PCM1_TX = 16,
	DMACH_PCM1_RX,
	DMACH_I2S1_OUT,
	DMACH_I2S1_IN,
	DMACH_SPI1_TX,
	DMACH_SPI1_RX,
	DMACH_AC97_PCMOUT,
	DMACH_AC97_PCMIN,
	DMACH_AC97_MICIN,
	DMACH_PWM,
	DMACH_IRDA,
	DMACH_EXTERNAL,
	DMACH_RES1,
	DMACH_RES2,
	DMACH_SECURITY_RX,	/* SDMA1 only */
	DMACH_SECURITY_TX,	/* SDMA1 only */
	DMACH_MAX		/* the end */
};

static inline bool samsung_dma_has_circular(void)
{
	return true;
}

static inline bool samsung_dma_is_dmadev(void)
{
	return false;
}
#define S3C2410_DMAF_CIRCULAR		(1 << 0)

/* enum s3c2410_chan_op
 *
 * operation codes passed to the DMA code by the user, and also used
 * to inform the current channel owner of any changes to the system state
 */

enum s3c6410_chan_op {
    DMAOP_START,
    DMAOP_STOP,
    DMAOP_PAUSE,
    DMAOP_RESUME,
    DMAOP_FLUSH,
    DMAOP_TIMEOUT,      /* internal signal to handler */
    DMAOP_STARTED,      /* indicate channel started */
};

enum s3c6410_dma_buffresult {
    DMA_RES_OK,
    DMA_RES_ERR,
    DMA_RES_ABORT
};

/* s3c_dma_cbfn_t
 *
 * buffer callback routine type
 */
typedef void (*s3c_dma_cbfn_t)(struct s3c_dma_chan *, void *buf, int size, int result);
typedef int  (*s3c_dma_opfn_t)(struct s3c_dma_chan *, int chan_op );

#define DMACH_LOW_LEVEL (1<<28) /* use this to specifiy hardware ch no */

/*  s3c64xx_dma_buff - S3C64XX DMA buffer descriptor
 * @next: Pointer to next buffer in queue or ring.
 * @pw: Client provided identifier
 * @lli: Pointer to hardware descriptor this buffer is associated with.
 * @lli_dma: Hardare address of the descriptor.
 */
struct s3c64xx_dma_buff {
	struct s3c64xx_dma_buff *next;

	void *pw;
	struct pl080s_lli *lli;
	dma_addr_t  lli_dma;
};

/* dma channel state information */
struct s3c64xx_dmac {
    struct device dev;
    struct clk *clk;
    void __iomem *regs;
    struct s3c_dma_chan *channels;
    enum s3c_dma_ch_id ch_id;
};

struct s3c_dma_chan {
	unsigned char		 number;      /* number of this dma channel */
	unsigned char		 in_use;      /* channel allocated */
	unsigned char		 bit;	      /* bit for enable/disable/etc */
	unsigned char		 hw_width;
	unsigned char		 peripheral;

	unsigned int		 flags;
	enum dma_data_direction	 source;


	dma_addr_t		dev_addr;

	struct s3c64xx_dmac	*dmac;		/* pointer to controller */

	void __iomem		*regs;

	/* cdriver callbacks */
	s3c_dma_cbfn_t	 callback_fn;	/* buffer done callback */
	s3c_dma_opfn_t	 op_fn;		/* channel op callback */

	/* buffer list and information */
	struct s3c64xx_dma_buff	*curr;		/* current dma buffer */
	struct s3c64xx_dma_buff	*next;		/* next buffer to load */
	struct s3c64xx_dma_buff	*end;		/* end of queue */

	/* note, when channel is running in circular mode, curr is the
	 * first buffer enqueued, end is the last and curr is where the
	 * last buffer-done event is set-at. The buffers are not freed
	 * and the last buffer hardware descriptor points back to the
	 * first.
	 */
};

/* DMAS (see manual chapter 8) */
typedef struct S3C64XX_DMA {
    S3C64XX_REG32 SrcAddr;
    S3C64XX_REG32 DestAddr;
    S3C64XX_REG32 LliAddr;
    S3C64XX_REG32 Control0;
    S3C64XX_REG32 Control1;
    S3C64XX_REG32 Config;
    S3C64XX_REG32 ConfigExp;
	S3C64XX_REG32 reserve;
} /*__attribute__((__packed__))*/ S3C64XX_DMA_T;

typedef struct {
	S3C64XX_REG32 DMACIntStatus;
	S3C64XX_REG32 DMACIntTCStatus;
	S3C64XX_REG32 DMACIntTCClear;
	S3C64XX_REG32 DMACIntErrorStatus;
	S3C64XX_REG32 DMACIntErrClr;
	S3C64XX_REG32 DMACRawIntTCStatus;
	S3C64XX_REG32 DMACRawIntErrorStatus; 
	S3C64XX_REG32 DMACEnbldChns;
	S3C64XX_REG32 DMACSoftBReq;
	S3C64XX_REG32 DMACSoftSReq;
	S3C64XX_REG32 reserve[2];
	S3C64XX_REG32 DMACConfiguration;
	S3C64XX_REG32 DMACSync;
    struct S3C64XX_DMA dma[8];
} /*__attribute__((__packed__))*/ S3C64XX_DMAS;

static inline S3C64XX_DMAS * S3C64XX_GetBase_DMA0(void)
{
   return (S3C64XX_DMAS *)ELFIN_DMA0_BASE;
}

static inline S3C64XX_DMAS * S3C64XX_GetBase_DMA1(void)
{
   return (S3C64XX_DMAS *)ELFIN_DMA1_BASE;
}

static inline S3C64XX_DMAS * S3C64XX_GetBase_SDMA0(void)
{
	return (S3C64XX_DMAS *)ELFIN_SDMA0_BASE;
}

static inline S3C64XX_DMAS * S3C64XX_GetBase_SDMA1(void)
{
	return (S3C64XX_DMAS *)ELFIN_SDMA1_BASE;
}

#endif /* #ifndef __ASSEMBLY__ */

#endif /* __ASM_ARCH_IRQ_H */
