/* @(#)77  1.7 src/rspc/kernext/pci/pcivca/inc/vcareg.h, pcivca, rspc41J, 9515B_all 4/13/95 03:39:47 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcareg.h -  header file
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/*
 *	V7310 register addressing definitions
 */

#define	VCA_DATA_PORT	0x04		/* data register offset */

#define	VCA_INTENA	0x09		/* Interrupt enable */
#define	VCA_ACQMOD	0x20		/* Acquisition mode */
#define	VCA_AQRCTL	0x21		/* Acquisition rectangle control */
#define	VCA_AQRCHS	0x22	/* Acquisition rectangle horizontal start */ 
#define	VCA_AQRCVS	0x24	/* Acquisition rectangle vertical start */
#define	VCA_AQRCHE	0x26	/* Acquisition rectangle horizontal end */
#define	VCA_AQRCVE	0x28	/* Acquisition rectangle vertical end */
#define	VCA_VRAQPA	0x2a		/* Frame memory in start address */
#define	VCA_ACQHSC	0x2d		/* Horizontal scaling-down */
#define	VCA_ACQVSC	0x2e		/* Vertical scalling-down */
#define	VCA_AQLINS	0x30	/* Acquisition image vertical start line base */
#define	VCA_OVLCTL	0x40		/* Overlay area control */
#define	VCA_OVRCHS	0x41		/* Overlay window horizontal start */
#define	VCA_OVRCVS	0x43		/* Overlay window vertical start */
#define	VCA_OVRCHE	0x45		/* Overlay window horizontal end */
#define	VCA_OVRCVE	0x47		/* Overlay window vertical end */
#define	VCA_VRRCST	0x4c		/* Frame memory read clock timing */
#define	VCA_SNCZOM	0x4d	/* Synchronization signal polarity and zoom */
#define	VCA_VRROPA	0x60		/* Frame memory display address */
#define	VCA_FNCSEL	0x90		/* Function select */
#define	VCA_VRLINW	0x91		/* Frame memory line width */
#define	VCA_PXLFRM	0x92		/* Pixel format */
#define	VCA_PWRCTL	0x93		/* Power control */
#define	VCA_BMTCTL	0x94		/* DMA transfer control */
#define	VCA_BMTVPA	0x95		/* Frame memory transfer address */
#define	VCA_BMTMMA	0x98		/* System memory transfer address */
#define	VCA_BMTSIZ	0x9c		/* Transfer size */
#define	VCA_PWRCT2	0x9f		/* Power control 2 */
#define	VCA_XINTEN	0xa0		/* Extended interrupt enable */
#define	VCA_INTSTA	0xa1		/* Interrupt status */
#define	VCA_ISTAWE	0xa2		/* Interrupt status write enable */
#define	VCA_CVHSCL	0xa3		/* Interrupt H scan line */
#define	VCA_CVHSCH	0xa4		/* Interrupt H scan line */
#define	VCA_KCLGYL	0xa7		/* Color keying green/y lower limit */
#define	VCA_KCLRCL	0xa8		/* Color keying red/cr lower limit */
#define	VCA_KCLBCL	0xa9		/* Color keying blue/cb lower limit */
#define	VCA_KCLGYH	0xaa		/* Color keying green/y upper limit */
#define	VCA_KCLRCH	0xab		/* Color keying red/cr upper limit */
#define	VCA_KCLBCH	0xac		/* Color keying blue/cb upper limit */
#define	VCA_GIOCTL	0xb0		/* General purpose I/O control */
#define	VCA_GOTBT1	0xb1		/* General purpose output */
#define	VCA_GOTBT2	0xb3		/* General purpose output */
#define	VCA_GOTDAT	0xb4		/* General purpose output */
#define	VCA_GINBT1	0xb5		/* General purpose input */
#define	VCA_GINBT2	0xb6		/* General purpose input */
#define	VCA_GINDAT	0xb8		/* General purpose input */
#define	VCA_PXGBT1	0xb9		/* Memory frame interface selection */
#define	VCA_PXGOSL	0xbc		/* Memory frame interface selection */
#define	VCA_CLTCTL	0xde		/* clut control */

/*
 *	Bt812 register addressing definitions
 */
#define	BT_INPUT	0x00		/* Bt812 input select */
#define	BT_STATUS	0x02		/* Bt812 status */
#define	BT_BRIGHTNESS	0x08		/* Bt812 brightness adjust */
#define	BT_CONTRAST	0x09		/* Bt812 contrast adjust */
#define	BT_SATURATION	0x0a		/* Bt812 saturation adjust */
#define	BT_HUE		0x0b		/* Bt812 hue adjust */
#define	BT_SOFT_RESET	0xff		/* Bt812/Frame memory reset */


/*
 *	PCI configuration register addressing definitions
 */

#define VENDOR_ID_REG           0x00
#define DEVICE_ID_REG           0x02
#define COMMAND_REG             0x04
#define REV_ID_REG              0x08
#define LATENCY_REG             0x0d
#define FRAME_ADDRESS_REG       0x10
#define BASE_ADDRESS_REG        0x14

/* PCI configuration programming values */
#define CFG_SPACE_READ          0
#define CFG_SPACE_WRITE         1
#define	V7310_IOADDR		0x800
#define	PCI_BUS_ID		0xc0
#define	V7310_IOMAP_SIZE	0xff
#define	CFG_CMD_ENABLE		0x0500
#define	BT_IOMAP_SIZE		0x1c

/* offset values for windows X-Y arrays  */
#define	AQRCHS_OFFSET	0x49 
#define	AQRCVE_OFFSET	0x14

/* set overlay memory start address */
#define	OVLMEM_START_ADDR	0x51000	

/* Power Management I/O Register */
#define	PM_IO_REG	0x4100
#define	PM_OUTPUT_CTL	0x0c
#define	CCD_POWER_ON	0x20
#define	CCD_POWER_OFF	0xdf

#define	SET_BITS	0xff
#define	RESET_BITS	0x00
#define	SET_BYTES	0xffff
#define	RESET_BYTES	0x0000
#define	SET_BIT0	0x01
#define	SET_BIT1	0x02
#define	SET_BIT2	0x04
#define	SET_BIT3	0x08
#define	SET_BIT4	0x10
#define	SET_BIT5	0x20
#define	SET_BIT6	0x40
#define	SET_REV_BIT0	0xfe

/* Bt812 status register(02h) bit assign */
#define	VIDEO_CHECK_BITS 0x80
#define	COLOR_CHECK_BITS 0x08

/* V7310 acquisition mode register(20h) bit assign */
#define	MOTION_BITS	0x01
#define	STOP_BITS	0x02
#define	STILL_BITS	0x03
#define	VIDEO_INT_BITS	0x80

/* V7310 acquisition control register(21h) bit assign */
#define	AREA_IN_USE	0x01
#define	AREA_OUT_USE	0x03
#define	HSCALE_BITS	0x04
#define	VSCALE_BITS	0x08
#define	INT_SIGNAL_BITS	0xa0
#define	EXT_SIGNAL_BITS	0xe0

/* V7310 overlay area control(40h) bit assign */
#define	COLOR_KEY_ENABLE 0x02
#define	COLOR_KEY_AREA	0x03

/* V7310 function select register(90h) bit assign */
#define	LCD_POWER_OFF	0x00
#define	NTSC_BASE	0x01
#define	LCD_POWER_ON	0x40
#define	PATHRU_MODE	0x80
#define	LCD_PATHRU_MODE	0xc0
#define	POWER_OFF_NTSC	0x3d	
#define	POWER_ON_NTSC	0xfd	

/* V7310 sync signal zoom register(4dh) bit assign */
#define	SNCZOM_ACTIVE_HIGH	0x30	/* synchronize signal: active high */
#define	H_8TIMES	0x03		/* horizontal zoom factor */
#define	V_8TIMES	0x0c		/* vertical zoom factor */

/* V7310 acquisition start line offset(30h) bit assign */
#define	ACQ_START_PALIN	0x10		/* PAL input */
#define	ACQ_START_PALIN2	0x21	/* PAL input */
#define	ACQ_START_HIGH	0x0b		/* 75Hz offset value on NTSC */
#define	ACQ_START_LOW	0x16		/* 60Hz, 72Hz offset value on NTSC */

/* V7310 power control register bit assign */
#define	RCAOUT_ENABLE	0x80	/* B0h, B4h: RCAout enable */
#define	GIOCTL_INPUT	0x60	/* B0h: FIELDE, FIELD0 input off */
#define	GIOCTL_OUTPUT	0x1f
#define	GIOCTL_POWER_ON	0x90
#define	GOTDAT_POWER_ON	0x0b
#define	GOTDAT_BT_FRAME	0x04	/* B4h: Bt812/Frame memory off */
#define	GOTDAT_V7310_DAC	0x1b
#define	GOTDAT_WD_DAC	0x8b
#define	PWRCTL_BT_FRAME	0x06	/* 93h: Bt812/Frame memory off */
#define	PWRCTL_DAC_ON	0x10
#define	PWRCTL_DAC_OFF	0x30
#define	PWRCTL_POWER_OFF	0x3f
#define	PWRCT2_BT_FRAME	0x20	/* 9Fh: Bt812/Frame memory on */
#define	PWRCT2_POWER_OFF	0x14
#define	PWRCT2_FRAME_NTSC   	0x3c
#define	SIGNAL_PORT_OFF	0xc0

/* V7310 interrupt register (a0h) bit assign */
#define	ACQ_INTR_BITS	0x08
#define	HSCAN_INTR_BITS	0x04
#define	INTR_USE_BITS	0x0c

/* V7310 bus master register (94h) bit assign */
#define	BMT_INTR_BITS	0x80
#define	BMT_READ_BITS	0x18
#define	BMT_WRITE_BITS	0x1a
#define	BMT_INTR_ENABLE	0x04
#define	BMT_START	0x05

/* V7310 general I/O register bit assign */
#define	RD_WR_BITS	0x03
#define	WR_STR_BITS	0xfd
#define	RD_STR_BITS	0xfe
