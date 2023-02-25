/* @(#)78	1.5  src/rspc/usr/lib/boot/softros/aixmon/res_patch.h, rspc_softros, rspc41J, 9520A_all 5/12/95 15:28:38  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_RES_PATCH
#define _H_RES_PATCH

typedef struct _PNPHEAP_CLUES {
	/* Allocated resource description */
	unsigned char *A_ptr;
	unsigned short A_count;
	/* Possible resource description */
	unsigned char *P_ptr;
	unsigned short P_count;
	/* Compatible device identifiers */
	unsigned char *C_ptr;
	unsigned short C_count;
} PNPHEAP_CLUES;

unsigned char L2_io_port_patch[]={
	0x47, 0x01, 0x14, 0x08, 0x14, 0x08, 0x01, 0x01,
	0x47, 0x01, 0x1C, 0x08, 0x1C, 0x08, 0x01, 0x01
};

RESIDUAL SF_res_patch={
	{0},			/* ResidualLength */
	{0},				/* Version */
	{1},				/* Revision */
	{0},				/* EC */
	{				/* VitalProductData */
		/* this is a place holder */
		0
	},				/* end of VitalProductData */
	{1},				/* MaxNumCpus */
	{1},				/* ActualNumCpus */
	{				/* Cpus */
		/* this is a place holder */
		0
	},				/* end of Cpus */
	{0x0},				/* TotalMemory */
	{0x0},				/* GoodMemory */
	{0x0},				/* ActualNumMemSegs */
	{				/* Segs */
		/* this is a place holder */
		0
	},				/* end of Segs array */
	{0x0},				/* ActualNumMemories */
	{				/* Memories */
		/* this is a place holder */
		0
	},				/* end of Memories */
	{0x00000011},			/* ActualNumDevices */
	{				/* Devices */
		/*
		 * there can be as many as MAX_DEVICES of these array elements,
		 * but the the number of items here should be enough for any
		 * sandalfoot system.  Those that aren't initialized here
		 * should be initialized to zero by the compiler.
		 */
	  {			/* Devices array element 0 */
	    {			    /* DeviceId */
	      {0x00000080},		/* BusId */
	      {0x244D000A},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x05},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 0 */
	  {			/* Devices array element 1 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x244D000B},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x08},			/* SubType */
	      {0x01},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 1 */
	  {			/* Devices array element 2 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00B00},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x03},			/* SubType */
	      {0x01},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 2 */
	  {			/* Devices array element 3 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x244D0008},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x05},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 3 */
	  {			/* Devices array element 4 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00000},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x01},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 4 */
	  {			/* Devices array element 5 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00200},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x01},			/* SubType */
	      {0x01},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 5 */
	  {			/* Devices array element 6 */
	    {			    /* DeviceId */
	      {0x00000080},		/* BusId */
	      {0x41D00A03},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x06},			/* BaseType */
	      {0x04},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 6 */
	  {			/* Devices array element 7 */
	    {			    /* DeviceId */
	      {0x00000004},		/* BusId */
	      {0x41D00A00},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x06},			/* BaseType */
	      {0x01},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x58,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 7 */
	  {			/* Devices array element 8 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x244D000E},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00006800},		/* Flags */
	      {0x04},			/* BaseType */
	      {0x01},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 8 */
	  {			/* Devices array element 9 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00700},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00006200},		/* Flags */
	      {0x01},			/* BaseType */
	      {0x02},			/* SubType */
	      {0x02},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 9 */
	  {			/* Devices array element 10 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00400},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002083},		/* Flags */
	      {0x07},			/* BaseType */
	      {0x01},			/* SubType */
	      {0x02},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 10 */
	  {			/* Devices array element 11 */
	    {			    /* DeviceId */
	      {0x00000004},		/* BusId */
	      {0x244D000F},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002380},		/* Flags */
	      {0x01},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x60,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 11 */
	  {			/* Devices array element 12 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00501},		/* DevId */
	      {0x00000001},		/* SerialNum */
	      {0x00002183},		/* Flags */
	      {0x07},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x04},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 12 */
	  {			/* Devices array element 13 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00501},		/* DevId */
	      {0x00000002},		/* SerialNum */
	      {0x00002183},		/* Flags */
	      {0x07},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x04},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 13 */
	  {			/* Devices array element 14 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00100},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002800},		/* Flags */
	      {0x08},			/* BaseType */
	      {0x02},			/* SubType */
	      {0x01},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 14 */
	  {			/* Devices array element 15 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00303},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x0000289A},		/* Flags */
	      {0x09},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 15 */
	  {			/* Devices array element 16 */
	    {			    /* DeviceId */
	      {0x00000001},		/* BusId */
	      {0x41D00F03},		/* DevId */
	      {0xFFFFFFFF},		/* SerialNum */
	      {0x00002892},		/* Flags */
	      {0x09},			/* BaseType */
	      {0x02},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  },			/* end of Devices array element 16 */
	  {			/* Devices array element 17 */
	    {			    /* DeviceId */
	      {0x00000000},		/* BusId */
	      {0x00000000},		/* DevId */
	      {0x00000000},		/* SerialNum */
	      {0x00000000},		/* Flags */
	      {0x00},			/* BaseType */
	      {0x00},			/* SubType */
	      {0x00},			/* Interface */
	      {0x00}			/* Spare */
	    },			    /* end of DeviceId */
	    {			    /* BusAccess */
	      0x00,			/* union member */
	      0x00,			/* union member */
	      0x0000			/* union member */
	    }			    /* end of BusAccess */
	  }			/* end of Devices array element 17 */
	}				/* end of Devices */
};

unsigned char dev0_Alloc_data[]={
	0x75, 0x01, 0x24, 0x4D, 0x01, 0x80, 0x78
};
unsigned char dev0_Poss_data[]={0x78};
unsigned char dev0_Comp_data[]={0x78};

unsigned char dev1_Alloc_data[]={
	0x47, 0x01, 0x08, 0x08, 0x08, 0x08, 0x01, 0x01,
	0x75, 0x01, 0x24, 0x4D, 0xF0, 0x8F, 0x78
};
unsigned char dev1_Poss_data[]={0x78};
unsigned char dev1_Comp_data[]={0x78};

unsigned char dev2_Alloc_data[]={
	0x22, 0x00, 0x01, 0x47, 0x01, 0x70, 0x00, 0x70,
	0x00, 0x01, 0x02, 0x75, 0x01, 0x24, 0x4D, 0x00,
	0x8F, 0x78
};
unsigned char dev2_Poss_data[]={0x78};
unsigned char dev2_Comp_data[]={0x78};

unsigned char dev3_Alloc_data[]={
	0x47, 0x01, 0x74, 0x00, 0x74, 0x00, 0x01, 0x02,
	0x47, 0x01, 0x77, 0x00, 0x77, 0x00, 0x01, 0x01,
	0x78
};
unsigned char dev3_Poss_data[]={0x78};
unsigned char dev3_Comp_data[]={0x78};

unsigned char dev4_Alloc_data[]={
	0x22, 0x04, 0x00, 0x47, 0x01, 0x20, 0x00, 0x20,
	0x00, 0x01, 0x02, 0x47, 0x01, 0xA0, 0x00, 0xA0,
	0x00, 0x01, 0x02, 0x47, 0x01, 0xD0, 0x04, 0xD0,
	0x04, 0x01, 0x02, 0x75, 0x01, 0x24, 0x4D, 0x00,
	0x81, 0x84, 0x15, 0x00, 0x09, 0x03, 0x20, 0x00,
	0x00, 0xF0, 0xFF, 0xFF, 0xBF, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x78
};
unsigned char dev4_Poss_data[]={0x78};
unsigned char dev4_Comp_data[]={0x78};

unsigned char dev5_Alloc_data[]={
	0x47, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10,
	0x47, 0x01, 0x80, 0x00, 0x80, 0x00, 0x01, 0x20,
	0x47, 0x01, 0xC0, 0x00, 0xC0, 0x00, 0x01, 0x20,
	0x47, 0x01, 0x0A, 0x04, 0x0A, 0x04, 0x01, 0x02,
	0x47, 0x01, 0x10, 0x04, 0x10, 0x04, 0x01, 0x30,
	0x47, 0x01, 0x81, 0x04, 0x81, 0x04, 0x01, 0x0B,
	0x47, 0x01, 0xD6, 0x04, 0xD6, 0x04, 0x01, 0x01,
	0x2A, 0x10, 0x04, 0x75, 0x01, 0x24, 0x4D, 0x00,
	0x81, 0x78
};
unsigned char dev5_Poss_data[]={0x78};
unsigned char dev5_Comp_data[]={0x78};

unsigned char dev6_Alloc_data[]={
	0x75, 0x01, 0x24, 0x4D, 0x01, 0x80, 0x84, 0x45,
	0x00, 0x03, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58,
	0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0x00, 0x60, 0x01, 0x00, 0x0D, 0x80,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x68,
	0x01, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00,
	0x0F, 0x00, 0x02, 0x70, 0x01, 0x00, 0x0F, 0x00,
	0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x84, 0x15,
	0x00, 0x09, 0x01, 0x20, 0x00, 0x00, 0xF0, 0xEF,
	0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x15,
	0x00, 0x09, 0x01, 0x20, 0x00, 0x00, 0xF0, 0xFF,
	0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x06,
	0x00, 0x06, 0x40, 0x8A, 0xF7, 0x01, 0x02, 0x84,
	0x1D, 0x00, 0x05, 0x01, 0x01, 0x01, 0x00, 0x00,
	0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x84,
	0x1D, 0x00, 0x05, 0x01, 0x01, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84,
	0x1D, 0x00, 0x05, 0x01, 0x01, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x84,
	0x1D, 0x00, 0x05, 0x01, 0x01, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x78
};
unsigned char dev6_Poss_data[]={0x78};
unsigned char dev6_Comp_data[]={0x78};

unsigned char dev7_Alloc_data[]={
	0x75, 0x01, 0x24, 0x4D, 0x00, 0x81, 0x4B, 0x98,
	0x03, 0x02, 0x47, 0x01, 0x00, 0x08, 0x00, 0x08,
	0x01, 0x30, 0x47, 0x01, 0x34, 0x08, 0x34, 0x08,
	0x01, 0x4C, 0x47, 0x01, 0x80, 0x08, 0x80, 0x08,
	0x01, 0x80, 0x84, 0x06, 0x00, 0x06, 0x90, 0xE2,
	0x7D, 0x00, 0x03, 0x84, 0x1D, 0x00, 0x05, 0x00,
	0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x84, 0x1D, 0x00, 0x05, 0x00,
	0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x84, 0x23, 0x00, 0x0A, 0x01,
	0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03,
	0x00, 0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07,
	0x00, 0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B,
	0x00, 0x0C, 0x00, 0x0D, 0x00, 0x0E, 0x00, 0xFF,
	0xFF, 0x78
};
unsigned char dev7_Poss_data[]={0x78};
unsigned char dev7_Comp_data[]={0x78};

unsigned char dev8_Alloc_data[]={
	0x22, 0x00, 0x04, 0x47, 0x01, 0x30, 0x08, 0x30,
	0x08, 0x01, 0x04, 0x2A, 0x40, 0x52, 0x2A, 0x80,
	0x52, 0x78
};
unsigned char dev8_Poss_data[]={0x78};
unsigned char dev8_Comp_data[]={0x78};

unsigned char dev9_Alloc_data[]={
	0x22, 0x40, 0x00, 0x4B, 0xF0, 0x03, 0x06, 0x4B,
	0xF7, 0x03, 0x01, 0x2A, 0x04, 0x68, 0x84, 0x05,
	0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x78

};
unsigned char dev9_Poss_data[]={0x78};
unsigned char dev9_Comp_data[]={0x78};

unsigned char dev10_Alloc_data[]={
	0x22, 0x80, 0x00, 0x4B, 0xBC, 0x03, 0x03, 0x78
};
unsigned char dev10_Poss_data[]={0x78};
unsigned char dev10_Comp_data[]={0x78};

unsigned char dev11_Alloc_data[]={
	0x84, 0x03, 0x00, 0x07, 0x01, 0x07, 0x78
};
unsigned char dev11_Poss_data[]={0x78};
unsigned char dev11_Comp_data[]={0x78};

unsigned char dev12_Alloc_data[]={
	0x22, 0x10, 0x00, 0x4B, 0xF8, 0x03, 0x08, 0x78
};
unsigned char dev12_Poss_data[]={
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xF8, 0x03, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xF8, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0x20, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xE8, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0x38, 0x03, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xE8, 0x03, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xE8, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0x38, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xE0, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0x28, 0x02, 0x08,
	0x38, 0x78
};
unsigned char dev12_Comp_data[]={0x78};

unsigned char dev13_Alloc_data[]={
	0x22, 0x08, 0x00, 0x4B, 0xF8, 0x02, 0x08, 0x78
};
unsigned char dev13_Poss_data[]={
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xF8, 0x03, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xF8, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0x20, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xE8, 0x02, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0x38, 0x03, 0x08,
	0x30, 0x22, 0x10, 0x00, 0x4B, 0xE8, 0x03, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xE8, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0x38, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0xE0, 0x02, 0x08,
	0x30, 0x22, 0x08, 0x00, 0x4B, 0x28, 0x02, 0x08,
	0x38, 0x78
};
unsigned char dev13_Comp_data[]={0x78};

unsigned char dev14_Alloc_data[]={
	0x22, 0x01, 0x00, 0x47, 0x01, 0x40, 0x00, 0x40,
	0x00, 0x01, 0x04, 0x47, 0x01, 0x61, 0x00, 0x61,
	0x00, 0x01, 0x01, 0x75, 0x01, 0x24, 0x4D, 0x10,
	0x8F, 0x78
};
unsigned char dev14_Poss_data[]={0x78};
unsigned char dev14_Comp_data[]={0x78};

unsigned char dev15_Alloc_data[]={
	0x22, 0x02, 0x00, 0x47, 0x01, 0x60, 0x00, 0x60,
	0x00, 0x01, 0x01, 0x47, 0x01, 0x64, 0x00, 0x64,
	0x00, 0x01, 0x01, 0x78
};
unsigned char dev15_Poss_data[]={0x78};
unsigned char dev15_Comp_data[]={0x78};

unsigned char dev16_Alloc_data[]={
	0x22, 0x00, 0x10, 0x47, 0x01, 0x60, 0x00, 0x60,
	0x00, 0x01, 0x01, 0x47, 0x01, 0x64, 0x00, 0x64,
	0x00, 0x01, 0x01, 0x78
};
unsigned char dev16_Poss_data[]={0x78};
unsigned char dev16_Comp_data[]={0x78};

PNPHEAP_CLUES offset_clues[]={
	{
		dev0_Alloc_data, 7,
		dev0_Poss_data, 1,
		dev0_Comp_data, 1
	},
	{
		dev1_Alloc_data, 15,
		dev1_Poss_data, 1,
		dev1_Comp_data, 1
	},
	{
		dev2_Alloc_data, 18,
		dev2_Poss_data, 1,
		dev2_Comp_data, 1
	},
	{
		dev3_Alloc_data, 17,
		dev3_Poss_data, 1,
		dev3_Comp_data, 1
	},
	{
		dev4_Alloc_data, 58,
		dev4_Poss_data, 1,
		dev4_Comp_data, 1
	},
	{
		dev5_Alloc_data, 66,
		dev5_Poss_data, 1,
		dev5_Comp_data, 1
	},
	{
		dev6_Alloc_data, 264,
		dev6_Poss_data, 1,
		dev6_Comp_data, 1
	},
	{
		dev7_Alloc_data, 146,
		dev7_Poss_data, 1,
		dev7_Comp_data, 1
	},
	{
		dev8_Alloc_data, 18,
		dev8_Poss_data, 1,
		dev8_Comp_data, 1
	},
	{
		dev9_Alloc_data, 23,
		dev9_Poss_data, 1,
		dev9_Comp_data, 1
	},
	{
		dev10_Alloc_data, 8,
		dev10_Poss_data, 1,
		dev10_Comp_data, 1
	},
	{
		dev11_Alloc_data, 7,
		dev11_Poss_data, 1,
		dev11_Comp_data, 1
	},
	{
		dev12_Alloc_data, 8,
		dev12_Poss_data, 82,
		dev12_Comp_data, 1
	},
	{
		dev13_Alloc_data, 8,
		dev13_Poss_data, 82,
		dev13_Comp_data, 1
	},
	{
		dev14_Alloc_data, 26,
		dev14_Poss_data, 1,
		dev14_Comp_data, 1
	},
	{
		dev15_Alloc_data, 20,
		dev15_Poss_data, 1,
		dev15_Comp_data, 1
	},
	{
		dev16_Alloc_data, 20,
		dev16_Poss_data, 1,
		dev16_Comp_data, 1
	}
};

#endif /* _H_RES_PATCH */