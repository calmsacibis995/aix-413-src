static char sccsid[] = "@(#)97	1.10  src/rspc/usr/lib/boot/softros/aixmon/gencmd.c, rspc_softros, rspc41J, 9520B_all 5/18/95 17:28:21";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: base_help
 *		disasm
 *		drive_select
 *		gencmd
 *		getblock
 *		more_prompt
 *		pre_jump_setup
 *		putblock
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

/* Generic command processor - gencmd.c                                 */

#include	"aixmon.h"
#include	"debug.h"
#include	<sys/iplcb.h>
#include	"iplcb_init.h"
#include	<scsi_api.h>

extern uint ProcessorBusHz;
extern unsigned char setup_flag;

#ifdef DEBUG
void gencmd(int argc,char **args)
{
int     index = 0;
int     i,j;
int	tdbl;
union
	{
	char cbuf[512];
	uint ibuf[128];
	} in;

int	mc_offset;			// Offset in disk header for control word

IPL_CB  *ipl_cb_ptr;                    /* IPL Control Block pointer    */

                                        /* Supported commands           */
char    *cmds[] = {"help","hid0","dl","jump","sinit","jumpa","shhz","chhz",""};

dbgmsg(AMON_D1,"- gencmd()");

while((strcmp(args[0],cmds[index]) != 0) && cmds[index][0] ) index ++;

dbgtst(AMON_D2) printf("Found index = %d\n",index);

switch(index)                                   /* Process the command  */
        {
        case    0:                              /* Help command         */
                switch(argc)
                        {
                        case    1:
                        base_help();
                        break;

                        case    99:
                        printf("I'll write help for help one of these days?\n");
                        break;

                        default:
                        printf("Help for %s\n",args[1]);
                        strcpy(args[0],args[1]);
                        args[1] = NULL;
                        argc = 99;
                        do_command(argc,args);
                        break;
                        }
        break;

        case    1:                      	/* Print HID0 reg     */
		if (processor == VER_601)
			printf("no HID0 on 601\n");
		else
			printf("HID0 = 0x%x\n",read_hid0());
        break;

        case    2:                      	/* Set the monitor debug lvl*/
		/*
		 * The levels go from AMON_D1 thru AMON_D32, which are
		 * actually defined as 0-31 so we can actually have 32
		 * levels.  But since 0 is used by the user to turn
		 * off debugging we use 1-32 to represent 0-31. Make sense ?
		 */
                if(argc == 99 || argc == 1)
			{
			printf("\ndl - Change debug level\n");
			printf("SYNTAX: dl level,[level],...\n");
			printf("Where:\n");
			for (j = 0; j < 32; j++)
			if (strcmp(dl_desc[j], ""))
			    printf("%5d - %s\n", j, dl_desc[j]);
			dbgtst(AMON_D2) printf("\nCurrent level = 0x%8.8X\n",aixmon_debug);
			break;
		    	}
		i = 1;
		while(args[i])
			{
			tdbl = atoi(args[i++]);
			if(tdbl == 0)
				aixmon_debug = 0;
			else
				if(tdbl > 32)
					aixmon_debug |= tdbl;
				else
					aixmon_debug |= (1 << (tdbl - 1));
		    	}
		dbgtst(AMON_D2) printf("\nCurrent level = 0x%8.8X\n",aixmon_debug);
        break;

        case    3:				/*=*=* jump =*=*=*=*=*=*=*=*=*=*/
                if(argc == 99)
                        {
                        printf("jump - jumps to the loaded image\n");
                        }
                else
                        {
                                                /* Ready for jump off   */
                        iplcb_init(&ipldata);           /* Setup iplcb  */

                        dbgtst(AMON_D20)
                                {
                                printf("Info from iplcb_init:\n");
                                printf("iplcbptr .. = 0x%8.8X\n",ipldata.iplcbptr);
                                printf("bitmap ptr  = 0x%8.8X\n",ipldata.bit_point);
                                printf("bytes / bit = 0x%8.8X\n",ipldata.bit_size);
                                printf("bitmap size = 0x%8.8X\n",ipldata.map_size);
				printf("load_addr   = 0x%8.8X\n",ipldata.load_addr);
				printf("image_size  = 0x%8.8X\n",ipldata.image_size);
				printf("jump_addr   = 0x%8.8X\n",ipldata.jump_addr);
				printf("Value of monitor_jump = %s\n",(monitor_jump)?"TRUE":"FALSE");
                                }

			if(status & STAT_VALID_BI)
				{
				    /* Do any pre setup of chips and registers and stuff  */
				pre_jump_setup(setup_flag);
                               				/* Go ahead make my day */
				if(monitor_jump)
					simple_jump(residual_save, ipldata.load_addr + ipldata.jump_addr);
				else
	                        	dojumpi( &ipldata, ipldata.load_addr ,
       						ipldata.image_size, ipldata.jump_addr);
				}
			printf("No valid boot image has been loaded can't jump\n");
                        }
        break;

	case	4:			// Force scsi init
	scsiinit(SINIT_FORCE);
	break;

        case    5:				/*=*=* jumpa =*=*=*=*=*=*=*=*=*=*/
                if(argc == 99)
                        {
                        printf("jumpa - jumps to the loaded image by jumping over\n");
			printf("        the AIXMON in the image\n");
                        }
                else
                        {
                                                /* Ready for jump off   */
                        iplcb_init(&ipldata);           /* Setup iplcb  */

                        dbgtst(AMON_D20)
                                {
                                printf("Info from iplcb_init:\n");
                                printf("iplcbptr .. = 0x%8.8X\n",ipldata.iplcbptr);
                                printf("bitmap ptr  = 0x%8.8X\n",ipldata.bit_point);
                                printf("bytes / bit = 0x%8.8X\n",ipldata.bit_size);
                                printf("bitmap size = 0x%8.8X\n",ipldata.map_size);
				printf("load_addr   = 0x%8.8X\n",ipldata.load_addr);
				printf("image_size  = 0x%8.8X\n",ipldata.image_size);
				printf("jump_addr   = 0x%8.8X\n",ipldata.jump_addr);
				printf("load_abs    = 0x%8.8X\n",ipldata.load_abs);
				printf("jump_abs    = 0x%8.8X\n",ipldata.jump_abs);
				printf("Value of monitor_jump = %s\n",(monitor_jump)?"TRUE":"FALSE");
                                }

			if((status & STAT_VALID_BI) && monitor_jump)
				{
				    /* Do any pre setup of chips and registers and stuff  */
				pre_jump_setup(setup_flag);
                               				/* Go ahead make my day */
	                        dojumpi( &ipldata, ipldata.load_abs ,
       					ipldata.image_size, ipldata.jump_abs);
				}
			printf("No valid boot image has been loaded can't jump\n");
                        }
        break;
	case 6:				/* Show ProcessorBusHz */
		printf("\nProcessorBusHz = %d\n",ProcessorBusHz);
	break;
	case 7:				/* Change ProcessorBusHz */
		if(argc == 99 || argc == 1)
			{
			printf("\nchhz - Change ProcessorBusHz\n");
			printf("SYNTAX: chhz Hz (ex. 33000000, 66000000, etc.)\n");
			break;
			}
		ProcessorBusHz = atoi(args[1]);
		printf("\nProcessorBusHz = %d\n",ProcessorBusHz);
	break;
        default:                        /*Don't know what this do help */
                dbgtst(AMON_D2)
                        printf("In default switch with %s and index = %d\n"
                                ,args[0],index);
		printf("Unknown command [%s] - type help for command list\n",args[0]);
//                base_help();		/* Display commend menu		*/
        break;
        }
}

/*************************************************************************/
/* base_help() -- Issue the basic help command                           */
/*************************************************************************/

base_help()
{
int     i;
char	line[256];
char	*pad = "                              ";

dbgmsg(AMON_D1,"- base_help()");

i=0;
while(command[i].cmd[0] != 0)
        {
	strcpy(line,command[i].cmd);
	strcat(line,command[i].help);
	strcat(line,pad);
	line[38] = 0;
        printf("%s",line );
	i++;
	if(command[i].cmd[0] != 0)
		{
	        printf("  %s %s\n",command[i].cmd,command[i].help);
       		i++;
		}
	else
		printf("\n");
        }

printf("\nEnter help <command name> for help on specific commands\n");
}

/*************************************************************************/
/* more_prompt() -- Put up the more prompt and wait for a carrier ret	 */
/*************************************************************************/

more_prompt()
{
printf("\n<more>");
(void)getchar();
printf("\n");
}
#endif /*DEBUG*/

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/* Hardware Setup routine -- Initialize hardware prior to calling AIX	     */
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

pre_jump_setup(unsigned char setup_flag)
{
int	in;


/* Setup real time clock */
outbyte(0x80000070,0x0b);
outbyte(0x80000071,0x02);
outbyte(0x80000070,0x0A);
outbyte(0x80000071,0x20);

if (setup_flag & 0x1) {
	/* This code is executed when version == 0 && revision == 0 */
	/* Enable posted write buffer in SIO chip */
	if (pci_type == 0) {
		/*
		 * For PCI 1.0 types, do this...
		 */
		in = inbyte(PCI_CONTROL_REG);
		outbyte(PCI_CONTROL_REG, (in | 0x05));
	} else {
		/*
		 * for 2.0 types, do this...
		 */
		out32le(0x80000cf8, 0x80005840);  /* set up address reg */
		in = in8(0x80000cfc);
		in = in | 0x05;
		out8(0x80000cfc, in);
	}

	/* Disable keyboard and mouse */
	Keyboard_Init();
	Disable_Keyboard();
	Disable_Mouse();

	/* Disable the 8259 controller */

#define INTA00 0x80000020       /* 8259 master control port address       */
#define INTA01 0x80000021       /* 8259 master mask port                  */
#define INTB00 0x800000A0       /* 8259 slave control port address        */
#define INTB01 0x800000A1       /* 8259 slave mask port                   */

/* Initialization words for both controllers                              */
/* ICW1                                                                   */
/*   - LTIM = 0  Edge triggered                                           */
/*   - ADI  = 1  Interval = 4   (As per Gary Young)                       */
/*   - SNGL = 0  Multiple 8259 controllers in the system                  */
/*   - ICA  = 1  Sending ICW4                                             */
#define ICW1 0x15

/* ICW2A                                                                  */
/*   - Master interrupts are numbered from 0 to 7                         */
#define ICW2A 0x00

/* ICW2B                                                                  */
/*   - Slave interrupts are numbered from 8 to 15                         */
#define ICW2B 0x08

/* ICW3A  (Note that logical pin 16(SP) is logical 1 on this chip = MASTER*/
/*   - Slave is on interrupt 2 (logical pin 20)                           */
#define ICW3A 0x04

/* ICW3B (Note that logical pin 16(SP) is logical 0 on this chip = SLAVE  */
/*   - Slave id is 2 (See hardware book for table)                        */
#define ICW3B 0x02

/* ICW4                                                                   */
/*   - SFNM = 0  Special fully nested mode is NOT programmed              */
/*   - BUF  = 0  Buffered mode is NOT programmed (SP, pin 16, selects mast*/
/*   - M/S  = 0  Ignored unless BUF=1, which it doesn't                   */
/*   - AEOI = 0  Automatic end of interrupt(EOI) is NOT programmed        */
/*   - uPM  = 1  MCS-86 operation is programmed (only 1 status byte)      */
#define ICW4 0x01

	outbyte(INTA00, ICW1);   	/* Initialize the master 8259 */
	outbyte(INTA01, ICW2A);
	outbyte(INTA01, ICW3A);
	outbyte(INTA01, ICW4);
	outbyte(INTA01, 0xff);   	/* Mask all interrupts via master */

	outbyte(INTB00, ICW1);   	/* Initialize the slave 8259 */
	outbyte(INTB01, ICW2B);
	outbyte(INTB01, ICW3B);
	outbyte(INTB01, ICW4);
	outbyte(INTB01, 0xff);   	/* Mask off all interrupts via slave */
}

if (setup_flag & 0x2) {
	/*
	 * then this is a true IBM sandalfoot with Version/Revision 0/0
	 * of firmware.  The setup_flag is initialized in aixmon.c
	 */
	/*
	 * initialize any devices in the PCI slots, except the ISA bus
	 * NOTE: we're assuming PCI type 1 here, which means direct access
	 */
	for (in = 12; in <= 14; in++ ) {
		/*
		 * read configuration space for each PCI device, starting
		 * with the integrated scsi.  12 is devfunc 0x60, 13 is
		 * devfunc 0x68 (slot 1), and 14 is devfunc 0x70 (slot 2)
		 */
		if (in32(0x80800000 | ( 1 << in )) != 0xFFFFFFFFUL) {
			/*
			 * if it doesn't return (-1), then there is a PCI
			 * device there, and we're going to send zeroes to the
			 * least significant 3 bits of that command register
			 */
			outbyte((0x80800000 | ( 1 << in ) | 4),
				(inbyte(0x80800000 | ( 1 << in ) | 4) & 0xF8));
		}
	}
}
}

#ifdef DEBUG
/*****************************************************************/
/*      Disassembler - uses ROS disassembler - for AIXMON        */
/*****************************************************************/

void disasm(int argc, char **args)
{
uint	address;
uint	num_o_ins;
char	tbuf[20];


dbgmsg(AMON_D1,"disasm()");

if(argc < 2 || argc == 99)
	{
	printf("disasm:\n");
	printf("SYNTAX: disasm <address> <number of instructions>\n");
	printf("        This option will disassemble the instructions beginning\n");
	printf("        from address for the number of instructions indicated\n");
	return;
	}

stoui(args[1],&address,16);
stoui(args[2],&num_o_ins,16);


dbgtst(AMON_D4)
	{
	printf("address .... = 0x%8.8X\n",address);
	printf("instructions = 0x%8.8X\n",num_o_ins);
	}

disassemble ( address, address, 1, num_o_ins * 4);

return;
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// getblock() -- Get a disk block and copy it to a buffer
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

static int	last_got_block;			// Keep last got block
static int	last_got_drive;			// Keep last got drive

void getblock(int argc, char **argv)
{

int	drive;
int	block;
char  	*buff;
int	rc;

*(int *)(&buff) = LOAD_ADDR;			/* Load address to constant */

dbgmsg(AMON_D1,"- getblock()");

if(argc == 99 || argc != 3)
	{
	printf("getb     - Copy a block from hardfile to Buffer \n");
	printf("      blocks.\n");
	printf("SYNTAX: getb <drive_number> <block number>\n");
	return;
	}

stoui(argv[1],&drive,16);
stoui(argv[2],&block,16);

if(is_drive_valid(drive,FALSE))
	Selected_Disk = drive;
else
	{
	printf("Invalid drive selected\n");
	return;
	}

if((rc = scsi_read(buff,drive,block,1)) != 0)
	{
	printf("Unable to read from scsi, rc = %d\n",rc);
	return;
	}

status |= STAT_VALID_GB;
last_got_block = block;
last_got_drive = drive;

printf("Block loaded in buffer at 0x%8.8X\n",buff);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// putblock() -- Put a put the current disk block buffer back to disk
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void putblock(int argc, char **argv)
{

int	drive;
int	block;
char  	*buff;
int	fini,rc;
char	ans;

*(int *)(&buff) = LOAD_ADDR;			/* Load address to constant */

dbgmsg(AMON_D1,"- putblock()");

if(argc == 99 || argc > 3 )
	{
	printf("putb     - Copy a block from Buffer to hardfile \n");
	printf("	   Puts it back to the same block number\n");
	printf("	   You may specify a different hardfile\n");
	printf("SYNTAX: putb [drive_number] [block_number]\n");
	return;
	}

block = last_got_block;
drive = last_got_drive;

if(argc > 1)
	stoui(argv[1],&drive,16);

if(argc > 2)
	stoui(argv[2],&block,16);

if(!is_drive_valid(drive,FALSE))
	{
	printf("Invalid drive selected\n");
	return;
	}

if(drive != last_got_drive || block != last_got_block)
	{
	printf("The selected location is not the same one used for get\n");
	fini = FALSE;
	while(!fini)
		{
		printf("OK to continue? (Y/N): ");
		ans = getchar();
		switch(ans)
			{
			case	'y':
			case	'Y':
			fini = TRUE;
			break;

			case	'n':
			case	'N':
			return;
			break;

			default:
			printf("\nPlease answer (Y)es or (N)o\n");
			break;
			}
		}
	}

if((rc = scsi_write(buff,drive,block,1)) != 0)
	{
	printf("Unable to write to scsi, rc = %d\n",rc);
	return;
	}

status &= ~STAT_VALID_GB;

printf("Block written to hardfile %d \n",drive);
}


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// drive_select() -- Select the default drive for loadi
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

void drive_select(int argc, char **argv)
{

int	drive;
int	i;

dbgmsg(AMON_D1,"- drive_select()");

if(argc == 99 || argc > 2 )
	{
	printf("drive - Select default hardfile to use for AIXMON\n");
	printf("SYNTAX: drive [drive_number] \n");
	return;
	}

if(argc == 1)
	{
	printf("Current = %d\n",Selected_Disk);
	printf("Available:\n");
	for(i=0;i<7;i++)		// Limit to 7 drives for now
		{
		printf("    %d  -",i);
		if(!is_drive_valid(i,TRUE))
			break;
		printf("\n");
		}

	return;
	}

stoui(argv[1],&drive,16);


if(is_drive_valid(drive,FALSE))
	Selected_Disk = drive;
else
	printf("Invalid drive selected\n");


return;

}

#endif /*DEBUG*/
