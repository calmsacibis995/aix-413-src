/*
 *   COMPONENT_NAME: rspccfg
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* header file representing the Master boot record format and includes for  */
/* managing the boot record extensions for RSPC Platforms		    */

/*----- Structures ----------------------------------------------------------*/

typedef struct
	{
	unsigned char  boot_ind,                /* Boot indicator	     */
                       begin_h,                 /* Begin head		     */
                       begin_s,                 /* Begin sector		     */
                       begin_c;                 /* Begin cylinder	     */
        unsigned char  syst_ind,                /* System indicator	     */
                       end_h,                   /* End head		     */
                       end_s,                   /* End sector		     */
                       end_c;                   /* End cylinder		     */
        unsigned int   RBA,                     /* Relative block address    */
                       sectors;                 /* Number of sectors	     */
        }  partition_table_entry;

typedef struct
	{
	/*unsigned char reserved_area[0x1BE];*/     /* Reserved space	     */
        partition_table_entry partition[4];     /* Partition i		     */
        }  boot_partition_table;

typedef struct
	{
	unsigned char  pc_compat[512];		/* PC Compatibility block    */
	unsigned int   entry_offset;		/* Program entry point	     */
	unsigned int   prog_length;		/* Program length in bytes   */
	/*unsigned char  data_block[504];*/	/* Data block should be 504  */
						/* Calculated later	     */
	} partition_header;

typedef struct
	{
	unsigned int   load_image_offset,      /* Entry point offset	     */
                       load_image_length;      /* Load image length	     */
        }  load_image_s2;

typedef struct
	{
	unsigned char  reserved_area[512];     /* PC compatibility block     */
        load_image_s2  sector_2;               /* Sector 2 of load image     */
        }  load_image;

typedef struct
	{
	unsigned int	signature;		/* Signature for boot prog   */
	unsigned int	Spare1;			/* Spare field		     */
	unsigned int	Spare2;
	unsigned int	Spart3;
	unsigned int	jump_offset;		/* Jump offset in boot image */
	unsigned int	Reserved;		/* Reserved, used by start.s */
	unsigned int 	header_size;		/* Size of header  	     */
	unsigned int	header_block_size;	/* Offset to AIX boot image  */
	unsigned int	image_length;		/* Size of boot program      */
	unsigned int	bss_offset;		/* Address of bss section    */
	unsigned int 	bss_length;		/* Length of bss section     */
	unsigned int	mode_control;		/* Boot mode control word    */
	} hints;

/*----- Defines    ----------------------------------------------------------*/

#define BOOT_SIGNATURE  0x55AA          /* Partition table signature	     */
#define AIXMON_SIG	0x4149584D	/* AIXMON Signature		     */
#define	SEC_SIZE	512
#define DATA_BLOCK_LEN	504		/* Size of data block for sec align  */
#define MASTERS 	0x41		/* Masters partition type indicator  */
#define NOT_ALLOCATED 	0x00		/* Unallocated partition	     */
#define NO_BOOT 	0x00		/* Boot indicator		     */
#define BOOTABLE_PART 	0x01

#define RS_TOC_NUM 0x1df		/* magic number for RISC6000 kernel  */
#define OLD_TOC_NUM 0x19f		/* magic number for old kernel       */

#define ROS_DASD        0x00000001	/* Device types for bootinfo	     */
#define ROS_FLOP        0x00000002
#define ROS_CDROM       0x00000003
#define ROS_TAPE        0x00000004

#define MODE_MASK       0x000000FF              /* Mask to available bits */
#define LODI_MODE       0x00000040              /* 0 = Manual   1 = Auto  */
#define JUMPI_MODE      0x00000080              /* 0 = Manual   1 = Auto  */

#define PART_START	0x1BE		/* Offset to start of partition tbl  */
#define SIG_START	0x1FE		/* Offset to start of signature	     */

#define M_LVOL		0
#define M_FILE		1

#define dbugit()        if(debug_flag)
