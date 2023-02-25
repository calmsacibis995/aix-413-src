/* @(#)24       1.3  src/rspc/kernext/inc/pcmciacs.h, pcmciaker, rspc41J, 9509A_all 2/20/95 17:33:04  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: PCMCIA Card Services Interface
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#if !defined( H_PCMCIACS )
# define H_PCMCIACS
#include <sys/types.h>

/* Client Service Functions */

#define	CSGetCardServicesInfo	  	0x0b
#define	CSRegisterClient	  	0x10
#define	CSDeregisterClient	  	0x02
#define	CSGetStatus		  	0x0c
#define	CSResetCard		  	0x11
#define	CSSetEventMask	  		0x31
#define	CSGetEventMask	  		0x2e

/* Resource Management Functions */

#define	CSRequestIO		  	0x1f
#define	CSReleaseIO		  	0x1b
#define	CSRequestIRQ		  	0x20
#define	CSReleaseIRQ		  	0x1c
#define	CSRequestWindow	  		0x21
#define	CSReleaseWindow	  		0x1d
#define	CSModifyWindow	  		0x17
#define	CSMapMemPage		  	0x14
#define	CSRequestSocketMask	  	0x22
#define	CSReleaseSocketMask	  	0x2f
#define	CSRequestConfiguration  	0x30
#define	CSGetConfigurationInfo  	0x04
#define	CSModifyConfiguration	  	0x27
#define	CSReleaseConfiguration  	0x1e
#define CSAccessConfigurationRegister 	0x36

/* Bulk Memory Service Functions */

#define	CSOpenMemory		  	0x18     /* unsupported */
#define	CSReadMemory		  	0x19     /* unsupported */
#define	CSWriteMemory		  	0x24     /* unsupported */
#define	CSCopyMemory		  	0x01     /* unsupported */
#define	CSRegisterEraseQueue	  	0x0f     /* unsupported */
#define	CSCheckEraseQueue	  	0x26     /* unsupported */
#define	CSDeregisterEraseQueue  	0x25     /* unsupported */
#define	CSCloseMemory		  	0x00     /* unsupported */

/* Client Utility Functions */

#define	CSGetFirstTuple	  		0x07
#define	CSGetNextTuple	  		0x0a
#define	CSGetTupleData	  		0x0d
#define	CSGetFirstRegion	  	0x06     /* unsupported */
#define	CSGetNextRegion	  		0x09     /* unsupported */
#define	CSGetFirstPartition	  	0x05     /* unsupported */
#define	CSGetNextPartition	  	0x08     /* unsupported */

/* Advanced Client Service Functions */

#define	CSReturnSSEntry	  		0x23     /* unsupported */
#define	CSMapLogSocket	  		0x12
#define	CSMapPhySocket	  		0x15
#define	CSMapLogWindow	  		0x13
#define	CSMapPhyWindow	  		0x16
#define	CSRegisterMTD		  	0x1a     /* unsupported */
#define	CSRegisterTimer	  		0x28     /* unsupported */
#define	CSSetRegion		  	0x29     /* unsupported */
#define	CSValidateCIS		  	0x2b
#define	CSRequestExclusive	  	0x2c
#define	CSReleaseExclusive	  	0x2d
#define	CSGetFirstClient	  	0x0e
#define	CSGetNextClient	  		0x2a
#define	CSGetClientInfo	  		0x03
#define CSAddSocketServices	  	0x32     /* unsupported */
#define	CSReplaceSocketServices 	0x33     /* unsupported */
#define	CSVendorSpecific	  	0x34     /* unsupported */
#define	CSAdjustResourceinfo	  	0x35     /* unsupported */

#define CSF_MAX			  	0x37

/* PCMCIA Return codes (Directly from the 2.0 Specification). */

#define	CSR_SUCCESS		  	0x00
#define	CSR_BAD_ADAPTER		  	0x01
#define	CSR_BAD_ATTRIBUTE	  	0x02
#define	CSR_BAD_BASE		  	0x03
#define	CSR_BAD_EDC		  	0x04
#define	CSR_BAD_IRQ		  	0x06
#define	CSR_BAD_OFFSET		  	0x07
#define	CSR_BAD_PAGE		  	0x08
#define	CSR_READ_FAILURE	  	0x09
#define	CSR_BAD_SIZE		  	0x0a
#define	CSR_BAD_SOCKET		  	0x0b
#define	CSR_BAD_TYPE		  	0x0d
#define	CSR_BAD_VCC		  	0x0e
#define	CSR_BAD_VPP		  	0x0f
#define	CSR_BAD_WINDOW		  	0x11
#define	CSR_WRITE_FAILURE	  	0x12
#define	CSR_NO_CARD		  	0x14
#define	CSR_UNSUPPORTED_FUNCTION  	0x15
#define	CSR_UNSUPPORTED_MODE	  	0x16
#define	CSR_BAD_SPEED		  	0x17
#define	CSR_BUSY		  	0x18
#define	CSR_GENERAL_FAILURE	  	0x19
#define	CSR_WRITE_PROTECTED	  	0x1a
#define	CSR_BAD_ARG_LENGTH	  	0x1b
#define	CSR_BAD_ARGS		  	0x1c
#define	CSR_CONFIGURATION_LOCKED  	0x1d
#define	CSR_IN_USE		  	0x1e
#define	CSR_NO_MORE_ITEMS	  	0x1f
#define	CSR_OUT_OF_RESOURCE	  	0x20
#define	CSR_BAD_HANDLE		  	0x21

#define	CSE_PM_RESUME			0x0b
#define	CSE_PM_SUSPEND			0x0c
#define	CSE_BATTERY_DEAD		0x01
#define	CSE_BATTERY_LOW			0x02
#define	CSE_CARD_INSERTION		0x40
#define	CSE_CARD_LOCK			0x03
#define	CSE_CARD_READY			0x04
#define	CSE_CARD_REMOVAL		0x05
#define	CSE_CARD_RESET			0x11
#define	CSE_CARD_UNLOCK			0x06
#define	CSE_EJECTION_COMPLETE		0x07
#define	CSE_EJECTION_REQUEST		0x08
#define	CSE_ERASE_COMPLETE		0x81
#define	CSE_EXCLUSIVE_COMPLETE		0x0d
#define	CSE_EXCLUSIVE_REQUEST		0x0e
#define	CSE_INSERTION_COMPLETE		0x09
#define	CSE_INSERTION_REQUEST		0x0a
#define	CSE_REGISTRATION_COMPLETE	0x82
#define	CSE_RESET_COMPLETE		0x80
#define	CSE_RESET_PHYSICAL		0x0f
#define	CSE_RESET_REQUEST		0x10
#define	CSE_MTD_REQUEST			0x12
#define	CSE_CLIENT_INFO			0x14
#define	CSE_TIMER_EXPIRED		0x15
#define	CSE_SS_UPDATE			0x16
#define CSE_WRITE_PROTECT		0x17


/***************
 * TUPLE CODES *
 ***************/

#define CISTPL_NULL		0x00	/* Null tuple - ignore		*/
#define CISTPL_DEVICE		0x01	/* Device information tuple	*/
#define CISTPL_CHECKSUM		0x10	/* Checksum control tuple	*/
#define CISTPL_LONGLINK_A	0x11	/* Long link control tuple 
					   (to attribute memory)	*/
#define CISTPL_LONGLINK_C	0x12	/* Long link control tuple 
					   (to common memory)	        */
#define CISTPL_LINKTARGET	0x13	/* Link target control tuple	*/
#define CISTPL_NO_LINK		0x14	/* No link control tuple	*/
#define CISTPL_VERS_1		0x15	/* Level 1 version/product 
					   information tuple            */
#define CISTPL_ALTSTR		0x16	/* Alternate-language-string tuple */
#define CISTPL_DEVICE_A		0x17	/* Attribute memory 
					   device information           */
#define CISTPL_JEDEC_C		0x18	/* JEDEC programming information
					   (for common memory)	        */
#define CISTPL_JEDEC_A		0x19	/* JEDEC programming information 
						(for attribute memory)	*/
#define CISTPL_CONFIG		0x1a	/* Configuration tuple		*/
#define CISTPL_CFTABLE_ENTRY	0x1b	/* Configuration-table-entry tuple */
#define CISTPL_DEVICE_OC	0x1c	/* Other operating conditions device 
					   - common memory		*/
#define CISTPL_DEVICE_OA	0x1d	/* Other operating conditions 
						- attribute memory	*/
#define CISTPL_DEVICE_GEO	0x1e	/* Device geometry info tuple	*/
#define CISTPL_DEVICE_GEO_A	0x1f	/* Device geometry info tuple	*/

/* Layer 2 Data Recording Format Tuples */

#define CISTPL_MANFID		0x20	/* Manufacturer Identification tuple */
#define CISTPL_FUNCID		0x21	/* Function identificaton tuple	*/
#define CISTPL_FUNCE		0x22	/* Function extension tuples	*/
#define CISTPL_SWIL		0x23	/* Software interleave tuple	*/
#define CISTPL_VERS_2		0x40	/* Level-2 version tuple	*/
#define CISTPL_FORMAT		0x41	/* Format tuple			*/
#define CISTPL_GEOMETRY		0x42	/* Geometry tuple		*/
#define CISTPL_BYTEORDER	0x43	/* Byte order tuple		*/
#define CISTPL_DATE		0x44	/* Card initialization date tuple */
#define CISTPL_BATTERY	 	0x45	/* Battery replacement date tuple */

/* Layer 3 Data Organization Tuples */

#define CISTPL_ORG		0x46	/* Organization tuple	*/

/* Layer 4 System-Specific Standard Tuples */

#define CISTPL_END		0xff	/* End-of-list tuple	*/


extern int CardServices(int, int *, void *, int, void *);

/* Actual Packet size is defined by arglen not the size of structure for
the packet that has variable size records */


typedef struct{
    int	Socket;
    unsigned char Action; /* I	Read/Write opration		*/
#define	CSCfgRegREAD	0
#define	CSCfgRegWRITE	1
    unsigned char Offset; /* I	Offset to Configuration Register */
    unsigned char Value;  /* I/O	Value to read or to write	*/ 
} CSAccCfgRegPkt;

#define CSSIGNATURE    "CS"
#define CSREVISION      0x0100
#define CSLEVEL         0x0210
#define CSVENDORSTRING "International Business Machines Corp."
typedef struct{
    int	         InfoLen;  /* O   length of data returned by CS 	*/
    char         signature[ sizeof( int ) ];
                           /* I/O ASCII 'CS' returned if CS installed 
			      (must be 0 before calling)                */
    int	 Count;            /* O   number of sockets 		        */
    unsigned int Revision; /* O   BCD value of vendor's CS revision     */
    unsigned int CSLevel;  /* O   BCD value of CS release 	        */
    int	VStrOff;           /* O   offset to vendor string 		*/
    int	VStrLen;           /* O   vendor string length 		        */
    char VendorString[1];  /* O   ASCIIZ vendor string buffer area      */
} CSGetCSInfoPkt;

typedef struct{
	int	MaxLen;
	int	InfoLen;
	unsigned int	Attributes;
#define	CSCliAttrMCDD	0x1
#define	CSCliAttrMTD	0x2
#define	CSCliAttrIOCDD	0x4
#define	CSCliAttrSHR	0x8
#define	CSCliAttrEXCL	0x10
#define CSCliAttrRSV   ~0x1f
#define	CSCliAttrSubF	0xff00
	union{
		char	b[1];
		struct {
			unsigned int	Revision;	/* bcd */
			unsigned int	CSLevel;	/* bcd */
			time_t	RevDate;
			int	NameOff;
			int	NameLen;
			int	VstrOff;
			int	VStringLen;
			char	b[1];
		} s; /* when CSCliInfoSubF bits == 0 */
	} un;
} CSGetCliInfoPkt;

typedef struct{
    int	Socket;
    unsigned int	Attributes;
/* 
 * Bit 0 Indicates the Exclusively Used 
 * Bit 8 Indicates the Valid Client 
 */
#define	CSCfgAttrExcl	  0x1      
#define	CSCfgAttrValidCli 0x100    
    /* 
     * CS fills the following fields by RequestConfiguration()
     */
    unsigned char Vcc;		/* O	Vcc setting			*/
    unsigned char Vpp1;		/* O	Vpp1 setting			*/
    unsigned char Vpp2;		/* O	Vpp2 setting			*/
    unsigned char IntType;	/* O	Memory or Memory+I/O interface	*/
    unsigned long ConfigBase;	/* O	Card base address of config reg */
    unsigned char Status;	/* O	Card Status register setting	*/
    unsigned char Pin;		/* O	Card Pin register setting	*/
    unsigned char Copy;		/* O	Card Socket/Copy register setting*/
    unsigned char Option;	/* O	Card Option register setting	*/
    unsigned char Present;	/* O	Card configration register present*/
    unsigned char FirstDevType;	/* O	from device ID tuple		*/
    unsigned char FuncCode;	/* O	from function ID tuple		*/
    unsigned char SysInitMask;	/* O	from function ID tuple		*/
    unsigned int  ManufCode;	/* O	from manufacture ID tuple	*/
    unsigned int  ManufInfo;	/* O	from manufacture ID tuple	*/
    unsigned char CardValues;	/* O	Valid card register values 	*/
#define	CSCfgCardOptValid	0x1
#define	CSCfgCardStatValid	0x2
#define	CSCfgCardPinValid	0x4
#define	CSCfgCardCopyValid	0x8
    unsigned char AssignedIRQ;	/* O	IRQ assinged to PC card		*/
    unsigned int  IRQAttributes;/* O	Attribute for for assinged IRQ	*/
    char*	  BasePort1;	/* O	Base port address for range	*/
    unsigned char NumPorts1;	/* O	Number of contigunous ports	*/
    unsigned char Attributes1;	/* O	bit-mapped			*/
    char*	  BasePort2;	/* O	Base port address for range	*/
    unsigned char NumPorts2;	/* O	Number of contigunous ports	*/
    unsigned char Attributes2;	/* O	bit-mapped			*/
    unsigned char IOAddrLines;	/* O	Number of address lines decoded	*/
} CSGetCfgInfoPkt;

typedef struct{
    unsigned int	Attributes;
#define	CSEvMaskThisSocket	0x1
    unsigned int	EventMask;
#define	CSEvMaskWPChg		0x1
#define CSEvMaskLockChg		0x2
#define	CSEvMaskEjReq		0x4
#define	CSEvMaskInReq		0x8
#define	CSEvMaskBatDead		0x10
#define	CSEvMaskBatLow		0x20
#define	CSEvMaskReadyChg	0x40
#define	CSEvMaskCDChg		0x80
#define	CSEvMaskPMChg		0x100
#define	CSEvMaskReset		0x200
#define	CSEvMaskSSUpdate	0x400
    int	Socket;
} CSEvMaskPkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
#define	CSGetCliThisSocket	0x1
} CSGetClientPkt;

typedef struct{
    
    int	Socket;                         /* I   Logical socket		*/
    unsigned int	Attributes;     /* I   bit-mapped		*/
#define	CSGetLinkTpls	0x1
    unsigned char	DesiredTuple;	/* I   Desired tuple code value	*/
    /*    unsigned char	reserved[ sizeof(int) - sizeof( char ) ];*/
    /* I   Reserved (reset to 0)	*/
    unsigned char        Reserved;
    /*
     *	Card Services initializes the following three value.
     * 	Client should preserve these values for subsequent GetNextTuple	  
     */
    unsigned int	Flags;		/* I/O CS tuple flag data	*/
    unsigned long	LinkOffset;	/* I/O CS link state information*/
    unsigned long	CISOffset;	/* I/O CS CIS state information	*/
    
    unsigned char	TupleCode;	/* O   Tuple found		*/
    unsigned char	TupleLink;	/* O   Link value for tuple found*/
} CSGetTplPkt;

typedef struct{
    int	Socket;
    unsigned int	CardState;
#define	CSStatWProtect	0x1
#define	CSStatLocked	0x2
#define	CSStatEjReq	0x4
#define	CSStatInReq	0x8
#define	CSStatBatDead	0x10
#define	CSStatBatLow	0x20
#define	CSStatReady	0x40
#define	CSStatDetected	0x80
    unsigned int	SocketState; 	
} CSGetStatPkt;

typedef struct{
    int	Socket;                   	/* I   Logical socket		*/
    unsigned int	Attributes; 	/* I   bit-mapped		*/
#define	CSGetLinkTpls	0x1
    unsigned char	DesiredTuple; 	/* I   Desired tuple code value	*/
    unsigned char       TupleOffset;    /* I/O Offset into tuple 
							from link byte	*/
    unsigned int	Flags;	      	/* I/O CS tuple flag data	*/
    unsigned long	LinkOffset;   	/* I/O CS link state information*/
    unsigned long	CISOffset;    	/* I/O CS CIS state information	*/
    int	TupleDataMax;         		/* I   Maximum size of tuple data area*/
    int	TupleDataLen;         		/* O   Number of bytes in tuple body*/
    char TupleData[1];         		/* O   Tuple data		*/
} CSGetTplDataPkt;


typedef struct{
	int	LogSocket;
	dev_t           PhyAdapter;
	unsigned char	PhySocket;
} CSMapSktPkt;

typedef struct{
	dev_t    	PhyAdapter;
	unsigned char	PhyWindow;
} CSMapWinPkt;

typedef struct{
	unsigned long	CardOffset; /* I	Card Offset Address	*/
	unsigned char	Page;       /* I	Page Number		*/
/*
 *	if the Paged bit in the Attributes field for a window is not set,
 *	this value must be zero indicating the first and only page 
 */
} CSMapMemPagePkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
#define	CSCfgIRQSteer	0x2
#define	CSCfgIRQChg		0x4
#define	CSCfgVccChg		0x8
#define	CSCfgVpp1Chg		0x10
#define	CSCfgVpp2Chg		0x20
	unsigned char	Vcc;
	unsigned char	Vpp1;
	unsigned char	Vpp2;
} CSModCfgPkt;

typedef struct{
	unsigned int	Attributes;
#define CSModWinAttMem	0x2
#define	CSModWinEnable	0x4
#define	CSModWinAccSpd	0x8
	unsigned int	AccessSpeed;
} CSModWinPkt;

typedef struct{
    unsigned int	Attributes;
#define	CSCliAttrMCDD	0x1
#define	CSCliAttrMTD	0x2
#define	CSCliAttrIOCDD	0x4
#define	CSCliAttrSHR	0x8
#define	CSCliAttrEXCL	0x10
    unsigned int	EventMask;
#define	CSEvMaskWPChg		0x1
#define CSEvMaskLockChg		0x2
#define	CSEvMaskEjReq		0x4
#define	CSEvMaskInReq		0x8
#define	CSEvMaskBatDead		0x10
#define	CSEvMaskBatLow		0x20
#define	CSEvMaskReadyChg	0x40
#define	CSEvMaskCDChg		0x80
#define	CSEvMaskPMChg		0x100
#define	CSEvMaskReset		0x200
#define	CSEvMaskSSUpdate	0x400
    char*    ClientData;
    unsigned int	Version;
} CSRegCliPkt;

/* 
  ClientData will not be used by Card Services, but callback is always
  called with ClientData value. If client device driver can be multiplexed
  by multiple devices, the data area pointed by ClientData should have
  data that can identify the device.
  */


typedef struct{
	int	Socket;
} CSRelCfgPkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
	/* Attributes bits are all reserved */
} CSExclPkt;

typedef struct{
    int	Socket;
    char*	        BasePort1;
    unsigned char	NumPorts1;
    unsigned char	Attributes1;
#define	CSIOSHARED	0x1
#define	CSIOFIRSTSHARED	0x2
#define	CSIOFORCEALIAS	0x4
#define	CSIO16BITS	0x8
#define CSIOATTRRSV    ~0xf
    char*	BasePort2;
    unsigned char	NumPorts2;
    unsigned char	Attributes2;
    unsigned char	IOAddrLines;
} CSIOPkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
	unsigned char	AssignedIRQ;
} CSRelIRQPkt;

typedef struct{
	int	Socket;
} CSRelSockMPkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
/* #define	CSCfgIRQSteer	0x2 : defined upper */
	unsigned char	Vcc;
	unsigned char	Vpp1;
	unsigned char	Vpp2;
	unsigned char	IntType;
#define CSCfgIFMem        0x01 		/* Memory-only interface    */
#define CSCfgIFIO         0x02 		/* I/O and memory interface */
	unsigned long	ConfigBase;
	unsigned char	Status;
	unsigned char	Pin;
	unsigned char	Copy;
	unsigned char	ConfigIndex;
	unsigned char	Present;
#define	CSCfgOptValid	0x1
#define	CSCfgStatValid	0x2
#define	CSCfgPinValid	0x4
#define	CSCfgCopyValid	0x8
} CSReqCfgPkt;

typedef struct{
	int	Socket;
	unsigned int	Attributes;
/* CSIRQEXCL, CSIRQTMSHR and CSIRQDYSHR is exclusively used */
#define	CSIRQExcl	0
#define	CSIRQTmShr	1
#define	CSIRQDyShr	2
#define	CSIRQFrcPulse	0x4
#define	CSIRQFstShr	0x8
#define	CSIRQPulseAlc	0x100
	unsigned char	AssignedIRQ;
	unsigned char	IRQInfo1;
#define	CSIRQNMI	0x1
#define	CSIRQIOCK	0x2
#define	CSIRQBERR	0x4
#define	CSIRQVEND	0x8
#define	CSIRQInfo2Used	0x10
#define	CSIRQLevel	0x20
#define	CSIRQPulse	0x40
#define	CSIRQShare	0x80
	unsigned int	IRQInfo2;
#define	CSIRQ0	0x1
#define	CSIRQ1	0x2
#define	CSIRQ2	0x4
#define	CSIRQ3	0x8
#define	CSIRQ4	0x10
#define	CSIRQ5	0x20
#define	CSIRQ6	0x40
#define	CSIRQ7	0x80
#define	CSIRQ8	0x100
#define	CSIRQ9	0x200
#define	CSIRQ10	0x400
#define	CSIRQ11	0x800
#define	CSIRQ12	0x1000
#define	CSIRQ13	0x2000
#define	CSIRQ14	0x4000
#define	CSIRQ15	0x8000

} CSReqIRQPkt;

typedef struct{
	int	Socket;
	unsigned int	EventMask;
#define	CSEvMaskWPChg		0x1
#define CSEvMaskLockChg		0x2
#define	CSEvMaskEjReq		0x4
#define	CSEvMaskInReq		0x8
#define	CSEvMaskBatDead		0x10
#define	CSEvMaskBatLow		0x20
#define	CSEvMaskReadyChg	0x40
#define	CSEvMaskCDChg		0x80
#define	CSEvMaskPMChg		0x100
#define	CSEvMaskReset		0x200
#define	CSEvMaskSSUpdate	0x400
} CSReqSockMPkt;

/*
where objectID means that input should be clientID and
the output should be windowID.
*/
typedef struct{
    int	                Socket;         /* I  */
    unsigned int	Attributes;
#define	CSWinAttr	  0x2
#define	CSWinEnable	  0x4
#define	CSWin16Bits	  0x8
#define	CSWinPaged	  0x10
#define	CSWinShared	  0x20
#define	CSWinFShared	  0x40
#define	CSWinOffset	  0x100
#define CSWinRSV         ~0x17E
    char*	        Base;           /* I/O	System Base Address	*/
    unsigned long	Size;   	/* I/O	Memory Window Size	*/
    unsigned int	AccessSpeed;	/* I	Window Speed Field	*/
#define CSWin250ns        0x1
#define CSWin200ns        0x2
#define CSWin150ns        0x3
#define CSWin100ns        0x4
#define CSWinWait         0x80
} CSReqWinPkt;

typedef struct{
    int	          Socket;       /* I   Logical Socket                   */
    unsigned int  Attributes;	/* I   all bits are reserved            */
} CSRstCardPkt;

typedef struct{
	int	Socket;
	int	Chains;
} CSValCISPkt;

#endif /* H_PCMCIACS */
