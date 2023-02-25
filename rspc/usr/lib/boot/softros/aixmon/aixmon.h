/* @(#)92	1.10  src/rspc/usr/lib/boot/softros/aixmon/aixmon.h, rspc_softros, rspc411, 9437C411a 9/16/94 10:26:51  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Header file for aixmon.c                                    */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include <ctype.h>
#include <types.h>

#include <stand.h>

#include <debug.h>

#include <boca_compat.h>

#include <model.h>


/**************************************************************************/
/****************** General Defines ***************************************/
/**************************************************************************/

#define STAT_VALID_BI	0x00000001		/* Valid boot image load  */
#define STAT_VALID_MC   0x00000002		/* Valid mode_control word*/
#define STAT_VALID_NV   0x00000004		/* Valid nvram buffer	  */
#define STAT_VALID_GB   0x00000008		/* Valid block in buffer  */

#define	LOAD_ADDR	0x00400000

#define ROS_DASD        0x00000001      /* Device types for bootinfo         */
#define ROS_FLOP        0x00000002
#define ROS_CDROM       0x00000003
#define ROS_TAPE        0x00000004
#define ROS_ENET	0x00000001
#define ROS_TOKEN	0x00000000

#define MODE_MASK       0x000000FF              /* Mask to available bits */
#define LODI_MODE       0x00000040              /* 0 = Manual   1 = Auto  */
#define JUMPI_MODE      0x00000080              /* 0 = Manual   1 = Auto  */

#define SINIT_FORCE     1			/* Force scsi init sequenc*/
#define SINIT_CHECK     0			/* Check if scsi init	  */


/**************************************************************************/
/****************** aixmon structure definition ***************************/
/**************************************************************************/

#ifdef DEBUG
struct cmd
        {
        char    *cmd;                   /* Name of command      */
        char    *help;                  /* First level help text        */
        void (*cmd_func)();             /* Command's function pointer   */
        };
#endif /*DEBUG*/

struct	ipldata
	{
	uint 	iplcbptr;		/* address of ipl control block	*/
	uchar 	*bit_point;		/* pointer to ram bit map	*/
	int 	bit_size;		/* number of byte per word	*/
	int 	map_size;		/* Size	of bit map		*/
	uint	load_addr;		/* Location bootimage loaded to */
	uint	image_size;		/* Size of bootimage in bytes	*/
	uint	jump_addr;		/* Jump offset from start of bi */
	uint	load_abs;		/* Location bootimage loaded to */
	uint	jump_abs;		/* Absolute jump to skip AIXMON */
	};

/**************************************************************************/
/****************** Sandalfoot memory data structure **********************/
/**************************************************************************/

/*   Definitions for system configuration parameters passed by aixrosmem to  */
/* its caller.  This data is for all of the 'computed values' in my list of  */
/* ROS data area descriptions.                                               */


#define ARCHITECTURE    2          /* POWER_PC                               */



#define SLB_ATTRIB 0               /* No slb - I believe 3                   */

#define SLB_SIZE 0                 /* No slb - I beleve 4 (for BATs)         */
#define ISLB_SIZE SLB_SIZE
#define DSLB_SIZE SLB_SIZE

#define SLB_ASC 0                  /* No slb - I believe 4 (fully assoc)     */
#define ISLB_ASC SLB_ASC
#define DSLB_ASC SLB_ASC

/* Values for rtc_type */
#define RTC_POWER 1                     /* rtc as defined by Power Arch.     */
#define RTC_POWER_PC 2                  /* rtc as defined by Power PC Arch.  */

/* This one might have to be figured out and set correctly                   */
#define RTCXINT  0                 /* I suspect these should be calculated   */
#define RTCXFRAC 0

/**************************************************************************/
/****************** Declare external functions ****************************/
/**************************************************************************/

				/* Func define for calling the relocator*/
				/* Arguments:				*/
				/* 	1 - Pointer to ipldata struct	*/
				/* 	2 - Start of bootimg addr	*/
				/* 	3 - length of boot image	*/
				/*  	4 - Jump address		*/

void dojumpi(struct ipldata *, uint, uint, uint);
unsigned int swap_endian(unsigned int);
char **mkargs(char *, int *);

void do_command(int, char **);
void gencmd(int, char **);
void jmm_modify(int, char **);
void jmm_dump(int, char**);
void jmm_reset();
void load_booti(int,char **);
//void con_set(int, char**);
void mem_read(int, char**);
void mem_write(int, char**);
void out_word(int, char**);
void in_word(int, char**);
void dump_words(int, char**);
void disasm(int, char**);
void hdf(int, char**);
void nv_load(int, char**);
void nv_store(int, char**);
void getblock(int, char**);
void putblock(int, char**);
void drive_select(int, char**);
void dump_residual(int, char**);
void nvram_dump(int, char**);

/**************************************************************************/
/****************** Declare global variables ******************************/
/**************************************************************************/


#ifdef IN_MAIN
#ifdef DEBUG
struct cmd command[] =          /* Setup command control struct */
        {"help","   - Display this menu",gencmd,
         "reset","  - reboot the machine",jmm_reset,
         "hid0","   - Print HID0 reg (603/604)",gencmd,
         "dl","     - Monitor Debug Level", gencmd,
         "loadi","  - Load image from diskette",load_booti,
	 "jump","   - Jump to loaded boot image",gencmd,
	 "jumpa","  - Jump directly to AIX image",gencmd,
         "dump","   - Display memory",jmm_dump,
         "dumpw","  - I/O space dump words",dump_words,
	 "drive","  - Select active hardfile ",drive_select,
	 "sinit","  - Force SCSI initialization",gencmd,
	 "hdf","    - Dumps from hardfile",hdf,
	 "getb","   - Get hardfile block ",getblock,
	 "putb","   - Put hardfile block",putblock,
 	 "disasm"," - Disassemble memory address",disasm,
         "mod","    - Modify memory, I/O",jmm_modify,
         "inb","    - Read a byte from I/O space", mem_read,
         "outb","   - Write a byte to I/O space",mem_write,
         "inw","    - Read a word from I/O space", in_word,
         "outw","   - Write a word to I/O space",out_word,
	 "resid","  - Dump Residual Data",dump_residual,
	 "mode","   - Modify the current boot mode ",gencmd,
	 "nvl","    - Load NVRAM contents ",nv_load,
	 "nvs","    - Store NVRAM buffer to NVRAM",nv_store,
	 "shhz","   - Show Processor Bus Speed",gencmd,
	 "chhz","   - Change Processor Bus Speed",gencmd,
         "","",NULL
         };

int	Selected_Disk = 0;		/* Default to first disk	 */

#endif /*DEBUG*/

int	status = 0;			/* Status field (general)	 */
int	mode_control;			/* Boot mode control		 */
int	header_block_size;		/* Offset to start of AIX image	 */
struct ipldata	ipldata;		/* IPL data information		 */
char	*mem_load_address;		/* Address in memory for boot image */
char	*residual_save;			/* Address of residual data area    */
unsigned int mem_jump_offset;		/* Jump offset to memory boot image */
char	monitor_jump = FALSE;		/* Monitor jump flag		 */
char	rostime[10];			/* Place holder for rostime	 */
int	mon_device_type;		/* Set devices when using loadi	 */
int     mon_device_id;
int     mon_device_lun;
int	model;	
int	processor;	
ushort	nvram_addr;
ushort	nvram_data;
uchar	pci_type;
uchar	scsi_bus_number;
uchar	scsi_dev_func;
uint	ProcessorBusHz;

#else

#ifdef DEBUG
extern struct cmd command[];
extern int	Selected_Disk;
#endif /* DEBUG */

extern int	status;
extern int	mode_control;
extern struct ipldata ipldata;
extern char	*mem_load_address;
extern char	*residual_save;
extern unsigned int mem_jump_offset;
extern char	monitor_jump;	
extern char	rostime[10];
extern int	mon_device_type;		
extern int	mon_device_id;
extern int	mon_device_lun;
extern int	model;	
extern int	processor;	
extern ushort	nvram_addr;
extern ushort	nvram_data;
extern uchar	pci_type;
extern uchar	scsi_bus_number;
extern uchar	scsi_dev_func;

#endif

/* Define some IO registers for Sandalfoot (601 or 604) only, PCI 1.0 */
#define SIO_START	0x80800800

#define PCI_CONTROL_REG		SIO_START + 0x40
