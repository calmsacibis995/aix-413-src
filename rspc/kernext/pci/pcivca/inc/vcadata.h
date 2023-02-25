/* @(#)74  1.9 src/rspc/kernext/pci/pcivca/inc/vcadata.h, pcivca, rspc41J, 9520A_all 5/11/95 20:46:17 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vcadata.h - general header file
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

/*************	ASCII V7310 chip Initialize data array *************/

static unsigned char V7310VGAParm[] = {	
	0x20, 0x00,		/* acquisition mode: stop  */
	0x21, 0xe1,		/* acquisition control: bit5 reserved */
	0x2d, 0x3f,		/* Horizontal scaling-down */
	0x2e, 0x3f,		/* Vertical scaling-down */
	0x40, 0x00,		/* overlay area control: base */
	0x4d, 0xc0,		/* synchronize signal: Output inverted */
	0x92, 0xc0,		/* pixel format select: RGB666 */
	0x94, 0x00,		/* Bus Master control */
	0xa0, 0x00,		/* interrupt disable: bit 3 and bit 2 */
	0xa2, 0x00,		/* interrupt status write permition: disable */
        0xde, 0x00              /* clut control: don't clut snoop */
};
					

/*************	Bt812  Initialize data array on VGA *************/

static unsigned char PALInParm[] = {
   0x00, 0x20,              /* VOUT1_Y select */
                            /* (reserved) */
   0x02, 0x00,              /* command register_2, status */
   0x03, 0xc0,              /* command register_3, output format YCrCB 4:2:2 */
   0x04, 0x00,              /* command register_4, operation mode select */
   0x05, 0x74,              /* command register_5, input format */
   0x06, 0x40,              /* command register_6, clock definition */
   0x07, 0x00,              /* command register_7, video timing definition */
   0x08, 0x20,              /* brightness adjust  + 12% */
   0x09, 0x80,              /* contrast adjust    100% */
   0x0a, 0x80,              /* saturation adjust  100% */
   0x0b, 0x00,              /* hue adjust         0 degree */
   0x0c, 0x94,              /* HCLOCK low */
   0x0d, 0x03,              /*        high */
   0x0e, 0x9f,              /* HDELAY low */
   0x0f, 0x00,              /*        high */
   0x10, 0x80,              /* ACTIVE_PIXELS low 640 */
   0x11, 0x02,              /*               high */
   0x12, 0x21,              /* VDELAY low 33 lines */
   0x13, 0x00,              /*        high */
   0x14, 0xe0,              /* ACTIVE_LINES low 480 */
   0x15, 0x01,              /*              high */
   0x16, 0x5d,              /* P (subcarrier freq) register_0 */
   0x17, 0xd1,              /* P (subcarrier freq) register_1 */
   0x18, 0x13,              /* P (subcarrier freq) register_2 */
   0x19, 0x69,              /* Automatic Gain Control(AGC) delay */
   0x1a, 0x53,              /* burst delay */
   0x1b, 0x00,              /* sample rate convertion low */
   0x1c, 0x00               /*                        high */
};

static unsigned char NTSCInParm[] = {
   0x00, 0x00,              /* VOUT0_Y, VOUT0_C select */
                            /* (reserved) */
   0x02, 0x00,              /* command register_2, status */
   0x03, 0xc0,              /* command register_3, output format YCrCB 4:2:2 */
   0x04, 0x00,              /* command register_4, operation mode select */
   0x05, 0x00,              /* command register_5, input format */
   0x06, 0x40,              /* command register_6, clock definition */
   0x07, 0x00,              /* command register_7, video timing definition */
   0x08, 0x30,              /* brightness adjust  + 18% */
   0x09, 0x80,              /* contrast adjust    100% */
   0x0a, 0x80,              /* saturation adjust  100% */
   0x0b, 0x00,              /* hue adjust          0 degree */
   0x0c, 0x8e,              /* HCLOCK low */
   0x0d, 0x03,              /*        high */
   0x0e, 0xbf,              /* HDELAY low   was 0x89 */
   0x0f, 0x00,              /*        high */
   0x10, 0x80,              /* ACTIVE_PIXELS low  640 */
   0x11, 0x02,              /*               high */
   0x12, 0x16,              /* VDELAY low  22 lines  was 21 */
   0x13, 0x00,              /*        high */
   0x14, 0xe0,              /* ACTIVE_LINES low  480 */
   0x15, 0x01,              /*              high */
   0x16, 0x00,              /* P (subcarrier freq) register_0 */
   0x17, 0x00,              /* P (subcarrier freq) register_1 */
   0x18, 0x10,              /* P (subcarrier freq) register_2 */
   0x19, 0x5e,              /* Automatic Gain Control(AGC) delay */
   0x1a, 0x53,              /* burst delay */
   0x1b, 0x00,              /* sample rate convertion low  */
   0x1c, 0x00               /*                        high */
};


/*************  Set V7310 register on NTSC *************/

static unsigned char v7310_ntsc_640[] = {
   0x20, 0x81, 0x21, 0x2d, 0x22, 0x7a, 0x23, 0x00, 0x24, 0x00, 0x25, 0x00,
   0x26, 0xfa, 0x27, 0x02, 0x28, 0xff, 0x29, 0x01, 0x2a, 0x00, 0x2b, 0x00,
   0x2c, 0x00, 0x2d, 0x22, 0x2e, 0x1d, 0x40, 0x09, 0x41, 0x2a, 0x42, 0x00, 
   0x43, 0x10, 0x44, 0x00, 0x45, 0xa9, 0x46, 0x03, 0x47, 0xd8, 0x48, 0x01, 
   0x4c, 0x6e, 0x4d, 0x31, 0x60, 0x00, 0x61, 0xd0, 0x62, 0x09, 0x91, 0x14, 
   0xb4, 0x0b
};

static unsigned char v7310_ntsc_800[] = {
   0x20, 0x81, 0x21, 0x2d, 0x22, 0x7a, 0x23, 0x00, 0x24, 0x00, 0x25, 0x00,
   0x26, 0x9a, 0x27, 0x03, 0x28, 0x58, 0x29, 0x02, 0x2a, 0x00, 0x2b, 0x00,
   0x2c, 0x00, 0x2d, 0x1c, 0x2e, 0x18, 0x30, 0x16, 0x40, 0x09, 0x41, 0x2a,
   0x42, 0x00, 0x43, 0x10, 0x44, 0x00, 0x45, 0xa9, 0x46, 0x03, 0x47, 0xd8,
   0x48, 0x01, 0x4c, 0x68, 0x4d, 0x31, 0x60, 0x00, 0x61, 0xd0, 0x62, 0x09,
   0x91, 0x14, 0xb4, 0x0b
};


/*************	ASCII V7310 Register for Power Management *************/

static unsigned char VSaveReg[] = {
   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
   0x2c, 0x2d, 0x2e, 0x30, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
   0x48, 0x4c, 0x4d, 0x4e, 0x4f, 0x60, 0x61, 0x62, 0x90, 0x91, 0x92, 0x93,
   0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
   0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xb0,
   0xb4, 0xd0, 0xd1, 0xde
};