static char sccsid[] = "@(#)00	1.4  src/rspc/usr/lib/boot/softros/aixmon/load_booti.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:02";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: get_dskt_bootrec
 *		load_booti
 *		scsiinit
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

/** load_booti.c - Boot image loader routines by media type		*/

#include "aixmon.h"
#include <mbrecord.h>
#include "scsi_api.h"

#include <sys/bootrecord.h>

#define	NORMAL   0
#define SERVICE  1


struct  bootdata			/* Information from boot rec	*/
        {
	uint	valid;			/* Is there a vaild image here  */
	uint	bootable;		/* Is this part the default boot*/
	uint	img_start;		/* boot image starting block #  */
        uint    load_addr;              /* Location bootimage loaded to */
        uint    mon_load_addr;          /* Location of start of AIX img */
        uint    image_size;             /* Size of bootimage in blocks  */
        uint    jump_addr;              /* Jump oset to AIXMON in image */ 
	uint	mon_jump_addr;		/* Jump oset to skip AIXMON	*/
        };

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
        load_booti() - Load boot image from media into memory
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

#ifdef DEBUG
void load_booti(int argc, char ** args)
{
					/* Setup jump and load addresses   */
int	validop, rc, i;
char	op;
partition_table_entry ptable[4];	/* Create a partition table	   */
struct	bootdata  bootdata[2];		/* Boot data structures Serv/Norm  */
IPL_REC           *ipl_record;
load_image 	  *mp_image;		/* Masters partition image	   */
unsigned char     read_buf[SEC_SIZE*2];	/* buffer to hold boot record 	   */
unsigned char     two_buf[SEC_SIZE*3]; 	/* buffer to hold partition record */
int	          Drive,lba,count;
hints		  *boot_hints;

char  		  *load_add; 		/* Setup jump and load addresses   */

*(&load_add) = (char *)LOAD_ADDR;	/* Load address to constant 	   */


monitor_jump = TRUE;			/* Set monitor jump flag		*/
					/* Images loaded via monitor are complete
					   boot images with another copy of aixmon
					   bound into the image. This flag is used
					   to select relocate vs. no-relocate mode 
					   when jumping to entry point.
					*/


					/* Get all information from boot record */

if((scsiinit(SINIT_CHECK)) != 0)
	{
	printf("Unable to initialize scsi, I quit\n");
	return;
	}

Drive = Selected_Disk; 			// Set drive to selected disk
count = 1;
lba = 0;				

if((rc = scsi_read(read_buf,Drive,lba,count)) != 0)
	{
	printf("Unable to read from scsi, rc = %d\n",rc);
	return;
	}
                                        /* Check for valid boot record  */
ipl_record = (IPL_REC_PTR) read_buf;

if(ipl_record->IPL_record_id != IPLRECID)
	{	
	printf("No valid boot record found on hardfile\n");
	dbgtst(AMON_D6) format_boot_header(ipl_record);
	return;
	}

memcpy(&ptable, &read_buf[PART_START] , (sizeof(ptable)));

dbgtst(AMON_D6)
	{
	hexdump(ptable,sizeof(ptable),0);
	}

					// Look at the MBR first
					// two parts for now
for(i=0;i<2;i++)			
	{
	if(ptable[i].RBA != 0)
		{
		bootdata[i].bootable = FALSE;
		bootdata[i].valid = FALSE;
		if(ptable[i].syst_ind == 0x41)
			bootdata[i].bootable = TRUE;
		bootdata[i].img_start = swap_endian(ptable[i].RBA);
		bootdata[i].load_addr = (uint) load_add;
		bootdata[i].image_size = 0;
		bootdata[i].jump_addr = 0;
		if((rc = scsi_read(two_buf,Drive,bootdata[i].img_start,3)) != 0)
			{
			printf("Unable to read from scsi, rc = %d\n",rc);
			return;
			}
		if(strncmp((char *)&two_buf[1024],"AIXM",4) != 0)
			{
			dbgtst(AMON_D6) hexdump(two_buf,1536,0);
			continue;
			}
		bootdata[i].valid = TRUE;
		boot_hints = (hints *) &two_buf[1024];
		mp_image = (load_image *) two_buf;
		bootdata[i].jump_addr = swap_endian(mp_image->sector_2.load_image_offset);	
		bootdata[i].mon_load_addr = (uint) (load_add + (boot_hints->header_block_size*SEC_SIZE));
		bootdata[i].mon_jump_addr = boot_hints->jump_offset;
		bootdata[i].image_size = swap_endian(mp_image->sector_2.load_image_length);	
		dbgtst(AMON_D15)
			{
			printf("Boot hints for this image:\n");
			printf("Signature: .......... 0x%8.8X\n",boot_hints->signature);
			printf("Loaded Address: ..... 0x%8.8X\n",boot_hints->Reserved);
			printf("Jump Offset: ........ 0x%8.8X\n",boot_hints->jump_offset);
			printf("Header Size: ........ 0x%8.8X\n",boot_hints->header_size);
			printf("Header_block_size: .. 0x%8.8X\n",boot_hints->header_block_size);
			printf("Image_length: ....... 0x%8.8X\n",boot_hints->image_length);
			printf("BSS Offset: ......... 0x%8.8X\n",boot_hints->bss_offset);
			printf("BSS Length: ......... 0x%8.8X\n",boot_hints->bss_length);
			printf("Mode Control: ....... 0x%8.8X\n",boot_hints->mode_control);
			}
		}
	}
		
dbgtst(AMON_D6)
	{
	printf(" Bootable    Valid    Img_start  Load_addr  Image_size Jump_addr   Mon_Jump\n");
	printf("---------- ---------- ---------- ---------- ---------- ---------- ----------\n");
	for(i=0;i<2;i++)
		printf("%s   %s   0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X\n",
			(bootdata[i].bootable)?"   YES  ":"   NO   ",
			(bootdata[i].valid)?"   YES  ":"   NO   ",
			bootdata[i].img_start, bootdata[i].load_addr,
			bootdata[i].image_size, bootdata[i].jump_addr,
			bootdata[i].mon_jump_addr);
	}
	
validop = FALSE;
while(!validop)
	{
	printf("Select media to load boot image from:\n");
	printf("    0 - Nevermind\n");
	printf("    1 - Diskette\n");
	printf("    2 - Normal Disk Boot ");
	printf(" - %s\n",(bootdata[0].valid)?"Valid":"No Valid Image: Do not use");
	printf("    3 - Service Disk Boot");
	printf(" - %s\n",(bootdata[1].valid)?"Valid":"No Valid Image: Do not use");
	printf("Choice: ");

	op = getchar();

	if( op == '0' || op == '1' || 
	   ((bootdata[1].valid) && (op == '3')) || 
	   ((bootdata[0].valid) && (op == '2')))
		{
		validop = TRUE;
		printf("\n");
		}
	else
		{
		printf("\n** Invalid choice: Must choose 1,2 or 3 - Choose 0 to exit\n");
		}
	}

switch(op)
	{
	case	'0':
	monitor_jump = FALSE;
	status &= ~STAT_VALID_BI;
	break;

	case	'1':
	get_dskt_bootrec(&ipldata);
	mode_control &= 0xFFFFFFF0;
	mode_control |= ROS_DASD;
	break;
	
	case	'2':	
	load_boot_image(&ipldata,&bootdata[NORMAL]);
	mode_control &= 0xFFFFFFF0;
	mode_control |= ROS_DASD;
	break;

	case	'3':
	load_boot_image(&ipldata,&bootdata[SERVICE]);
	mode_control &= 0xFFFFFFF0;
	mode_control |= ROS_DASD;
	break;

	default:
	printf("Should never get here *ERROR* load_booti main loop\n");
	break;
	}
return;
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
        get_dskt_bootrec() - Routine to read a boot record
                        (From diskette for now)
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

get_dskt_bootrec(struct ipldata *ipldata)
{
uint	num_of_secs;		/* Number of sectors to read this time	*/
uint	remain_secs;		/* Sectors remaining to read		*/
uint	secs_done;		/* Sectors read				*/
uint	blocks;			/* Total number of blocks to read	*/
int	dskt_number;
int	rc;

load_image  *mp_image;		/* Masters partition image	   	*/
unsigned char two_buf[SEC_SIZE*3]; /* buffer to hold partition record 	*/

int	first_block;		/* First Block to read			*/
int 	drive;			/* Drive Number to read from		*/

char  	*load_add; 		/* Setup jump and load addresses   	*/

#ifdef DEBUG	/* diskette support only with DEBUG */
*(&load_add) = LOAD_ADDR;	/* Load address to constant 		*/

                                        /* Prompt for diskette insert   */
printf("Insert diskette #%d and then hit <Enter>: ",1);
(void)getchar();

drive = 0;
first_block = 0;

if((rc=diskette_read_RBA(first_block,3,drive,two_buf)) != 0)
	{
	if((rc=diskette_read_RBA(first_block,3,drive,two_buf)) != 0)
		{
		printf("diskette_read_RBA() - rc = %d",rc);
		return(-1);
		}
	}

                                        /* Check for valid boot record  */

if(strncmp((char *)&two_buf[1024],"AIXM",4) != 0)
	{
	dbgtst(AMON_D6) hexdump(two_buf,1536,0);
	printf("No valid AIX boot image found on diskette\n");
	return(-1);
	}

mp_image = (load_image *) two_buf;
ipldata->jump_addr = swap_endian(mp_image->sector_2.load_image_offset);	
ipldata->image_size = swap_endian(mp_image->sector_2.load_image_length);	
ipldata->load_addr = load_add;

blocks = ipldata->image_size/SEC_SIZE;

				/* Everything is ok, load the image	*/

printf("Boot image loading to ... 0x%8.8X\n",load_add);

num_of_secs = (blocks > 2880)?2880:blocks;
secs_done = 0;

first_block = 0;		// Skip over the boot header and cfg record

if((rc = diskette_read_RBA(first_block,num_of_secs,drive,load_add)) != 0)
	{
	if((rc = diskette_read_RBA(first_block,num_of_secs,drive,load_add)) != 0)
		{
		printf("diskette_read_RBA() error = %d\n",rc);
		return(-1);
		}
	}

load_add += (num_of_secs * SEC_SIZE);		// Get ready for next diskette
dskt_number = 2;
remain_secs = blocks - num_of_secs;
secs_done = num_of_secs;
first_block = 0;

while(remain_secs > 0)
	{ 				/* Load Next diskette	*/
	printf("Insert diskette #%d and then hit <Enter>: ",dskt_number);
	(void)getchar();
	dskt_number++;
	num_of_secs = (remain_secs > 2880)?2880:remain_secs;
	if((rc=diskette_read_RBA(first_block,num_of_secs,drive,load_add)) != 0)
		{
		if((rc=diskette_read_RBA(first_block,num_of_secs,drive,load_add)) != 0)
			{
			printf("diskette_read_RBA() - rc = %d",rc);
			return(-1);
			}
		}
	secs_done += num_of_secs;
	remain_secs = blocks - secs_done;
	load_add += (num_of_secs * SEC_SIZE);		
	}

if(secs_done  == blocks )
	{
	status |= STAT_VALID_BI;
	}
else
	{
	printf("secs_done = %d  Should have been = %d\n",secs_done,blocks);
	status |= STAT_VALID_BI;
	}

printf("Number of sectors loaded 0x%8.8X\n",blocks);
printf("Number of bytes computed 0x%8.8X\n",ipldata->image_size);
printf("Jump address computed at 0x%8.8X\n",ipldata->jump_addr);

#else
printf("No diskette support unless compiled with DEBUG.\n");
#endif /* DEBUG */

}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
        scsiinit() - Initialize the scsi subsystem
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

scsiinit(int	flag)
{
unsigned int config_data;
unsigned char cmd;
static	 initialized = FALSE;		// Just initialize it once
int	i,rc;
				// Don't do it again if not forced
if(initialized && (flag == SINIT_CHECK))
	return(0);

initialized = TRUE;

scsi_init();				// No return code fake it for now

return(0);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// get_block()	- Get a block from the hardfile
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

get_block(int Drive, int block, char *buff)
{
int	count;				// Number of blocks
int	rc;

count = 1;				// Get one block for now

if((rc = scsi_read(buff,Drive,block,count)) != 0)
	{
	printf("Unable to read from scsi, rc = %d\n",rc);
	return(1);
	}
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// put_block()	- Put a block to the hardfile
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

put_block(int Drive, int block, char *buff)
{
int	count;				// Number of blocks
int	rc;

count = 1;				// Put one block for now

if((rc = scsi_write(buff,Drive,block,count)) != 0)
	{
	printf("Unable to write to scsi, rc = %d\n",rc);
	return(1);
	}
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// hdf() - Hardfile debug facility
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void hdf(int argc, char **argv)
{
int	blocks;				// Number of blocks to read
int	start;				// Starting block number
int	drive;				// Drive number to read
int	current;			// Current block number
int	i;
char	inbuf[SEC_SIZE];		// Buffer

dbgmsg(AMON_D1,"- hdf()");

if(argc == 99 || argc != 4)
	{
	printf("hdf - Hardfile Dump Facility. Read and display hardfile\n");
	printf("      blocks.\n");
	printf("SYNTAX: hdf <drive_number> <block number> <number of blocks>\n");
	return;
	}

stoui(argv[1],&drive,16);
stoui(argv[2],&start,16);
stoui(argv[3],&blocks,16);

						// Check for valid drive number
if(!is_drive_valid(drive,FALSE))
	{
	printf("Invalid Drive:\n");
	return;
	}

for(current = start; current < (start+blocks);current++)
	{
	get_block(drive,current,(char *)&inbuf);
	hexdump(inbuf,SEC_SIZE, (current * SEC_SIZE));
	}
}
#endif /*DEBUG*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
        get_boot_image() - Locate the in memory boot image and prepare
			   to make the jump 
			   This is the normal boot path with no monitor
			   function enabled for Software ROS
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

get_boot_image(struct ipldata *ipldata)
{
load_image *li;

extern int header_block_size;

li = (load_image *)(mem_load_address - (header_block_size * SEC_SIZE));


				/* Setup pointers based on header  */
ipldata->jump_addr = mem_jump_offset;
ipldata->load_addr = (uint)mem_load_address;
ipldata->image_size = swap_endian(li->sector_2.load_image_length);
//ipldata->image_size = 0x3F0000;	// Constant until I get pointer from ROS

if(!(mode_control & JUMPI_MODE))
	{
	printf("Magic is 0x%8.8X \n",(int *) mem_load_address);
	printf("Image size .............. 0x%8.8X\n",ipldata->image_size);
	printf("Boot image loaded at .... 0x%8.8X\n",mem_load_address);
	printf("Saved address for jump .. 0x%8.8X\n",mem_jump_offset);
	}

status |= STAT_VALID_BI;
}

#ifdef DEBUG
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
        load_boot_image() - Load the boot image from hardfile into memory
	This boot path is for maintenance mode boot from the Firmware II
	monitor. 
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

load_boot_image(struct ipldata *ipldata, struct bootdata *bootdata) 
{
int	secs;				/* Sectors to read		  */
int	i,rc;
IPL_REC    *ipl_record;
unsigned char read_buf[SEC_SIZE];       /* Array to hold boot record 	*/
int	Drive,lba,count;
char  	*load_add; 			/* Setup jump and load addresses   */

*(&load_add) = (char *)LOAD_ADDR;	/* Load address to constant */

if((scsiinit(SINIT_CHECK)) != 0)
	{
	printf("Unable to initialize scsi, I quit\n");
	return(-1);
	}

Drive = Selected_Disk; 			// Set drive to selected disk

if((rc = scsi_read(read_buf,Drive,0,1)) != 0)
	{
	printf("Unable to read from scsi, rc = %d\n",rc);
	return(-1);
	}
                                        /* Check for valid boot record  */
ipl_record = (IPL_REC_PTR) read_buf;

dbgtst(AMON_D3) format_boot_header(ipl_record);

				/* Check for Magic Number	    */
if(ipl_record->IPL_record_id != IPLRECID)
	{
	printf("Valid boot record not found\n");
	return(1);
	}

				/* Load the image into memory	   */
count = (bootdata->image_size / SEC_SIZE) + 1;
lba = bootdata->img_start;

dbgtst(AMON_D6) printf("Loading to 0x%8.8X  drive: %d  block: 0x%8.8X  count: 0x%8.8X\n",
			load_add, Drive, lba, count);

if((scsi_read(load_add,Drive,lba,count)) != 0)
	{
	printf("Unable to read from scsi, I quit\n");
	return(-1);
	}

				/* Setup pointers based on header  */

ipldata->jump_addr = bootdata->jump_addr;
ipldata->load_addr = bootdata->load_addr;
ipldata->image_size = bootdata->image_size;
ipldata->jump_abs = bootdata->mon_jump_addr;
ipldata->load_abs = bootdata->mon_load_addr;

// Put a check here to check XCOFF Magic. That's the best we can do for now
//printf("Magic is 0x%8.8X \n",(short *) load_add);

status |= STAT_VALID_BI;

printf("Boot image loaded at ... 0x%8.8X\n",load_add);

printf("Jump address computed at  0x%8.8X\n",ipldata->jump_addr);

}
#endif /*DEBUG*/
