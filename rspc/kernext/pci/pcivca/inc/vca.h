/* @(#)69  1.4 src/rspc/kernext/pci/pcivca/inc/vca.h, pcivca, rspc41J, 9520B_all 5/18/95 02:31:48 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vca.h - general header file
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
#ifndef	_H_VCA
#define	_H_VCA

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <unistd.h>
#include <sys/watchdog.h>
#include <sys/intr.h>

/*
 * Define IOCTL flags
 */
#define	VCA_IOC_NTSC_SW			_IOW('v',1,struct ntsc_sw *)
#define VCA_IOC_CAMERA_SW		_IOW('v',2,struct camera_sw *)
#define VCA_IOC_INPUT_SW		_IOW('v',3,struct input_sw *)
#define VCA_IOC_INITIALIZE		_IOW('v',4,struct vca_init_data *)
#define VCA_IOC_ACQUISITION		_IOW('v',5,struct acquisition *)
#define VCA_IOC_OVERLAY			_IOW('v',6,struct overlay *)
#define VCA_IOC_CAPTURE			_IOW('v',7,struct capture *)
#define VCA_IOC_AD_CTRL			_IOW('v',8,struct adcon_ctrl *)
#define VCA_IOC_VRAM_READ		_IOW('v',9,struct vram_xfer *)
#define VCA_IOC_VRAM_WRITE		_IOW('v',10,struct vram_xfer *)
#define VCA_IOC_GET_INPUTSW		_IOW('v',11,struct video_info *)
#define VCA_IOC_SET_INPUTSW		_IOW('v',12,struct input_source *)


/*
 *  structure for  VCA_IOC_NTSC_SW
 */
struct	ntsc_sw
{
        int     ntsc_out;           	/* ntsc output on/off */
#define NTSC_ON    1
#define NTSC_OFF   0

};


/*
 *  structure for  VCA_IOC_CAMERA_SW
 */
struct	camera_sw
{
        int     camera;           	/* AD converter or  NTSC output */
#define AD_CON    0
#define NTSC_OUT  1
};



/*
 * structure for VCA_IOC_INPUT_SW
 */

struct input_sw 
{
	int ntsc_in;			/* CCD camera, external Video */
#define CCD_IN		0
#define VIDEO_EX	1
};



/*
 * structure for VCA_IOC_INITIALIZE
 */

struct vca_init_data 
{
	uchar 	width;		/* 1-32: "VRAM line width" = width*32 pixels */
};


/*
 * structure for VCA_IOC_ACQUISITION
 */

struct 	acquisition
{
	int acq_mode;		/* stop acquisition, still, start motion
					and start double buffered motion */
#define STOP_ACQ 		0
#define STILL			1
#define	START_ACQ		2
#define START_DOUBLE_BUF	3
	int acq_area;		/* inside of area, outside of area and
					all of the input */
#define AREA_IN  		0
#define AREA_OUT  		1
#define AREA_ALL  		2
	ushort acq_w, acq_h;		/* acquisition area W and H */
	ushort acq_x, acq_y;		/* acquisition position X and Y */
	uchar 	scale_x, scale_y;	/* scaling ratio (0-64)
					divide scale_x(or scale_y) by 64 */
	uint vram_x, vram_y;		/* VRAM store position X and Y */
};



/*
 * structure for VCA_IOC_OVERLAY
 */
struct overlay
{
	int	ovl_mode;		/* overlay mode select */
#define	NOCOLOR_ALL	0		/* colorkeys unused and overlayed to 
							whole display */
#define	NOCOLOR_AREA	1	/* colorkeys unused and overlay area used */
#define	COLOR_ALL	2		/* colorkeys used and overlayed to
							whole display */
#define	COLOR_AREA	3	/* colorkeys used and overlay area used */
	uchar	ovl_area;		/* overlay area select */
#define	F0_AREA		1
#define	F1_AREA		2
#define	F2_AREA		4
#define	F3_AREA		8
	uint 	vram_w, vram_h;		/* VRAM area W and H */
	uint	vram_x, vram_y;		/* VRAM position X and Y */
	uchar	zoom_x, zoom_y;		/* zooming ratio */
#define	NO_ZOOM		0
#define	ZOOM_TWICE	1
#define	ZOOM_4TIMES	2
#define	ZOOM_8TIMES	3
	ushort	ovl_x, ovl_y;		/* overlay position X and Y */
	
	uchar  red_l;			/* lowest red colorkey */
	uchar  red_h;			/* highest red colorkey */
	uchar  green_l;			/* lowest green colorkey */
	uchar  green_h;			/* highest green colorkey */
	uchar  blue_l;			/* lowest blue colorkey */
	uchar  blue_h;			/* highest blue colorkey */

};


/* 
 * structure for VCA_IOC_CAPTURE
 */
struct	capture
{
	int 	read_mode;		/* Data transfer mode */
#define	SYNC_ACQ	0		/* synchronous to acquisition */
#define	ASYNC_ACQ	1		/* asynchronous to acquisition */
	char	*buff;			/* pointer to buffer in user space */
};


/* 
 * structure for VCA_IOC_AD_CTRL
 */
struct	adcon_ctrl
{
	char 	bright;			/* Brightness adjust default: 0
						-128 thru 126, step 2 */
	uchar 	contrast;		/* Contrast adjust default: 128
						0 thru 254, step 2 */
	uchar 	saturation;		/* Saturation adjust default: 128
						0 thru 254, step 2 */
	char 	hue;			/* Hue adjust default: 0
						-128 thru 126, step 2 */
};



/* 
 * structure for VCA_IOC_VRAM_READ and VCA_IOC_VRAM_READ
 */

struct vram_xfer
{
	uint	vram_w, vram_h;		/* VRAM area W and H */
	uint	vram_x, vram_y;		/* VRAM position X and Y */
	char	*buff;			/* pointer to buffer in user space */
};


#define PORTNUM         	4            	/* Video channel number */
#define	PORT0			0
#define	PORT1			1
#define	PORT2			2
#define	PORT3			3
#define V7310_Nosignal       0x00            	/* No Signal */
#define V7310_NTSCC          0x01            	/* NTSC Composite Video */
#define V7310_NTSCS          0x02            	/* NTSC S-Video */
#define V7310_PALC           0x04            	/* PAL Composite Video */
#define V7310_PALS           0x08            	/* PAL S-Video  */
#define V7310_CCDCamera      0x80            	/* Internal CCD Camera */

/*
 * structure for VCA_IOC_GET_INPUT_SW
 */
struct	video_info
{
	 uint    VCMap[PORTNUM];		/* Video channel Map */
};


/*
 * structure for VCA_IOC_SET_INPUT_SW
 */
struct	input_source
{
	uint    port_id;
        uint    video_in;
};

#endif	/* _H_VCA */
