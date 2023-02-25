/* @(#)78  1.12 src/rspc/kernext/pci/pcivca/inc/vcastr.h, pcivca, rspc41J, 9521A_all 5/22/95 06:38:25 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcastr.h -  header file
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
 *	Includes files for AIX V4 
 */

#include <sys/types.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/mdio.h>
#include <sys/conf.h>
#include <sys/intr.h>
#include <sys/uio.h>
#include <sys/pin.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include <sys/except.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <sys/systm.h>
#include <sys/syspest.h>
#include <sys/xmem.h>
#include <sys/display.h>
#include <sys/trchkid.h>
#include <sys/ddtrace.h>
#include <sys/watchdog.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/pmdev.h>
#include "vcadds.h"
#include "vcareg.h"
#include "vcamac.h"
#include "vcadata.h"
#include "vcacmd.h"
#include "vidpnp.h"
#include <sys/residual.h>
#include <sys/iplcb.h>

#define	PMG_ON  1
#define	PMG_OFF 0
#define	CRT_PMG_ON	3
#define	CRT_PMG_OFF	2
#define	LCD_PMG_ON	5
#define	LCD_PMG_OFF	4

#define	VCA_PAGE_NUM	1		/* 4k */
#define	V7310_VRAM_SIZE	0x9e340		/* 648kbytes */
#define	VCA_MAX_TSIZ	0xfa00		/* 64kbytes */	
#define	ACQ_DISP_WIDTH	640		/* acquisition display width */
#define	ACQ_DISP_HEIGHT	480		/* acquisition display height */
#define	MAX_VCA_MINORS	2		/* forture release ? */
#define	XGA_DISP_WIDTH	1024		/* XGA display width */
#define	SVGA_DISP_WIDTH	800		/* SVGA display width */
#define	HIGH_REFRESH_RATE	75	/* 75Hz */

#define	ACQ_EVENT	1		/* acquisition event type */
#define	BMT_EVENT	2		/* bus master transfer  event type */
#define	HSCAN_EVENT	3		/* H scanline event type */

#define	VRAM_READ	0
#define	VRAM_WRITE	1
#define	VRAM_TIME	2
#define	SINGLE_BUF	0
#define	DOUBLE_BUF	1
#define	FIRST_BUF	0
#define	SECOND_BUF	1
#define	CAPTURE_MODE	0
#define	WRITE_MODE	1

#define	ZOOM_VALUE1	1
#define	ZOOM_VALUE2	2
#define	ZOOM_VALUE3	4
#define	ZOOM_VALUE4	8

#define	VID0_Y		0x00
#define	VID1_Y		0x20
#define	VID2_Y		0x40
#define	VID3_Y		0x60
#define	VID0_C		0x00
#define	VID1_C		0x08
#define	VID2_C		0x10
#define	VID3_C		0x18

#define	LONGWAIT	delay(200)
#define	SHORTWAIT	delay(50)


/* This structure get video data from display device driver */
struct  VDDinfo {
         unsigned int    lcd_w, lcd_h;/* LCD Panel Physical Resolution */
         unsigned int    offset_x, offset_y;     /* logical offset x,y */
         unsigned int    vram_clk;               /* VRAM clock timming */
         unsigned int    vram_offset;            /* VRAM offset */
         unsigned int    v_sync;                 /* vsync */
         unsigned int    v_total;                /* vertical total lines */
};


/* This structure is the internal device structure used by the vca to keep 
 * track of all its work.
 */

struct vca_area
{             
        struct	intr vca_handler;	/* Interrupt handler structure */
	struct	io_map	pci_io_map;	/* memory mapping structure */
	struct  xmem    xp;		/* cross memory mapping structure */
	struct  VDDinfo vddinfo;	/* get video data from display D.D */
        dev_t	dev;			/* device number of this device */
        pid_t   ext_appl;		/* no externsion for application user */
	pid_t	ext_ccd;		/* extension parameter for CCD_CFG */
	pid_t	ext_ntsc;		/* extension parameter for NTSC */
	int	bus_number;		/* bus_number */
	int	int_priority;		/* interrupt priority */
	int	intr_level;		/* interrupt level */
	int	device_in_use;		/* status of device in use */
	int	request_is_blocked;	/* Request for hibernation/suspend */
	int	lcd_status,crt_status;	/* set LCD/CRT on/off */
	int	pmout_status;		/* CCD camera on/off */
	int	ntsc_in_use;		/* status of ntsc in use */
	int	request_of_ccd;		/* request for ccd in use */
	uint 	xres, yres;		/* physical screen size */
	uint	offset_x, offset_y;	/* overlay offset size */
	uint	vram_clk;		/* VRAM clock timming */
	uint	vram_offset;		/* VRAM offset */
	uint	v_sync;			/* vsync */
	uint	v_total;		/* vertical total lines */
	uint	pan_x, pan_y;		/* view port geometry */
	ushort	win_sx, win_sy;		/* current window start position */
	ushort	win_ex, win_ey;		/* current window end position */
	ushort	out_x, out_y;		/* current VRAM store position */
	int	ccd_status;		/* ccd camera status */
	int	ccd_id;			/* ccd camera id */
	uint 	io_addr;		/* Base I/O address */
	int 	dma_paddr[VCA_PAGE_NUM];/* DMA phisical address */
	caddr_t	dma_vaddr[VCA_PAGE_NUM];/* DMA virtual address */
	int	acq_event, hscan_event, bmt_event;	/* event type */
	uint	ovl_end;		/* overlay end position */
	int	zox, zoy;		/* zoom value */
	uint 	line_width;		/* VRAM line width */
	caddr_t	user_buff;		/* buffer in user space */
	ushort	acq_trans_w, acq_trans_h;
	uint	acq_addr1, acq_addr2;	/* acquisition start address */
	int	acq_mode;		/* motion, still or double buffer ? */
	int	intr_mode_set;		/* acquisition, AD control or playback*/
	int	buf_number;		/* using buffer number */
	int	trans_flag;		/* flag of BMT */
	int	video_in_flag;		/* 0: CCD 1: NTSC 2: PAL */
	int	res_chan_map[PORTNUM];	/* get residual data */
	int	ntsc_timing_in;		/* detect to NTSC Timing signal */
#ifdef	PM_SUPPORT
	int	pm_event, dma_activity;	/* activity flag for DMA */
#endif	/* PM_SUPPORT */
};


typedef struct {        /* PCI Config space */
       ushort 	VendorID;
       ushort 	DevID;
       ushort  	command;
       ushort 	status;
       uchar  	revID;
       uchar  	CacheLineSize;
       uchar  	latency;
       ulong   FrameAddressRegs[6];
       ulong   BaseAddressRegs[6];
} PCI_cfg;

#define PCI_BYTE_AT             1
#define PCI_SHORT_AT            2
#define PCI_WORD_AT             4

