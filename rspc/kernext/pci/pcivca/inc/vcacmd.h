/* @(#)71  1.5 src/rspc/kernext/pci/pcivca/inc/vcacmd.h, pcivca, rspc41J, 9507C 1/27/95 04:09:05 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcacmd.h -  header file
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

#include "vca.h"

/*
 * Define IOCTL flags
 */
#define VCA_IOC_DISPLAY_SW              _IOW('v',15,struct display_sw *)
#define VCA_IOC_XWIN_PAN		_IOW('v',16,struct xwin_pan *)
#define	VCA_IOC_IO_ACS			_IO('v',17) 
#define VCA_IOC_CCD_CFG			_IOR('v',18,struct ccd_cfg *)
#define	VCA_IOC_XPANEL			_IOW('v',19,struct xpanel *)
#define VCA_IOC_REG_IN			_IOW('v',20,struct reg_in *)
#define VCA_IOC_REG_OUT			_IOW('v',21,struct reg_out *)
#define VCA_IOC_BT_IN			_IOW('v',22,struct bt_in *)
#define VCA_IOC_BT_OUT			_IOW('v',23,struct bt_out *)


/*
 * structure for VCA_IOC_DISPLAY_SW
 */

struct  display_sw
{
        int     lcd_out;            /* LCD Power control */
#define VCA_LCD_ON      1
#define VCA_LCD_OFF     0
        int     crt_out;            /* DAC Power Control */
#define VCA_CRT_ON      1
#define VCA_CRT_OFF     0

};


/*
 * structure for VCA_IOC_X_PAN
 */

struct	xwin_pan
{
	uint	view_x, view_y;		/* view port geometry */

};

/*
 * structure for VCA_IOC_CCD_CFG
 */

struct	ccd_cfg
{
	uint	ccd_status;		/* CCD status 
        				    1 - available 
					    0 - not available */
#define	CCD_ON	1
#define	CCD_OFF	0
};


/*
 * structure for VCA_IOC_XPANEL
 */

struct	xpanel
{
	uint	phys_w, phys_h;		/* phisical screen size */
	uint	log_w, log_h;		/* logical screen size */
	uint	offset_x, offset_y;	/* overlay offset X,Y */
	uint	vram_clk;		/* VRAM clock timming */
	uint	vram_offset;		/* VRAM OFFSET */
	uint	v_sync;			/* vsync */
	uint	v_total;		/* vertical total lines */
};


/*
 * structure for VCA_IOC_REG_IN
 */

struct	reg_in
{
	uint	addr;			/* V7310 Register */
	uchar	data;			/* Register value */
};

/*
 * structure for VCA_IOC_REG_OUT
 */

struct	reg_out
{
	uint	addr;			/* V7310 Register */
	uchar	data;			/* Register value */
};


/*
 * structure for VCA_IOC_BT_IN
 */

struct	bt_in
{
	uint	addr;			/* Bt812 Register */
	uchar	data;			/* Register value */
};

/*
 * structure for VCA_IOC_BT_OUT
 */

struct	bt_out
{
	uint	addr;			/* Bt812 Register */
	uchar	data;			/* Register value */
};


/* extension parameter  for open device */
#define VCA_CCD_CFG     1
#define VCA_XSERVER     2
#define VCA_IN_OUT     	3
#define VCA_NTSC     	4

