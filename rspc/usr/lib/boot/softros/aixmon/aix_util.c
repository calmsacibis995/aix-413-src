static char sccsid[] = "@(#)90	1.3  src/rspc/usr/lib/boot/softros/aixmon/aix_util.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:05";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: dbugout
 *		format_boot_header
 *		hexdump
 *		is_drive_valid
 *		mkargs
 *		swap_endian
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

/* aix_util.c - General utilities for support of the aixmon routine	*/

#include "aixmon.h"
#include "scsi_api.h"
#include <sys/bootrecord.h>

#ifdef DEBUG
/***********************************************************************/
/**                                                                   **/
/**  dbugout(cmd,cc)   - Format and display hex dump of data block    **/
/**                                                                   **/
/**		This is left here for compatability. This version     **/
/**		of dbugout will dump cc bytes beginning at address    **/
/**		cmd and cause the addresses on the dump to begin at   **/
/**		cmd.						      **/
/**                                                                   **/
/***********************************************************************/

dbugout(char * cmd,int cc)
{
printf("\n");
hexdump(cmd,cc,(uint) cmd);
}

/***********************************************************************/
/**                                                                   **/
/**  hexdump(buf,cc,addr) - Format and display hex dump of data block **/
/**                                                                   **/
/**          Inputs:						      **/
/**		buf  - Pointer to input buffer to dump		      **/
/**		cc   - Character count to dump 			      **/
/**	        addr - Starting address for address column	      **/
/**                                                                   **/
/***********************************************************************/

hexdump(char * buf,int cc, uint addr )
{
int    i,j;
char   hstr[60];
char   astr[19];
char   tbuf[10];
int    tcnt;

tcnt = addr;
while(cc)
   {
   strcpy(astr,"[................]");
   strcpy(hstr,"");

   for(i=0,j=1;i<16 && cc>0;i++,j++)
      {
      cc--;
      sprintf(tbuf,"%02X",*(buf+i));
      strcat(hstr,tbuf);
      if(j==8)
          strcat(hstr," - ");
      else
          strcat(hstr," ");

      if(isprint(*(buf+i)))
          {
          astr[j] = *(buf+i);
          }
      }
   buf+=16;
   printf("%08X: %-50s %s\n",tcnt,hstr,astr);
   tcnt += 16;
   }

}

/***********************************************************************/
/**                                                                   **/
/**  mkargs(buf,argc) -- Converts buf into an argv structure          **/
/**                 Deletes extra spaces and returns pointer          **/
/**                                                                   **/
/***********************************************************************/

char ** mkargs(char *buf, int *argc)
{
char    *delim = " ,-\t";       	/* Field delimitors             */
int     i,alen,sspace,acount;
static char **args[50];         	/* Create the args structure    */

dbgmsg(AMON_D1," - mkargs()");

acount = 0;                     	/* Initialize parm count 	*/

alen = strlen(buf);			/* How long is it		*/
i    = 0;                       	/* Init counter          	*/
while(!isgraph(buf[i]) && i<alen) i++;  /* Find first character         */
args[acount++] = (i<alen)?(char **)&buf[i]:0;   /* Set first arg        */


for(;i<alen;i++)                        /* Parse the command string     */
        {
        if(strchr(delim,buf[i]))
                {
                buf[i] = 0;             /* Terminate this element       */
                while(!isgraph(buf[i]) && i<alen) i++;
		if(i<alen) args[acount++] = (char **)&buf[i];
                }
        }
args[acount] = NULL;                    /* Null terminate the args   */

dbgtst(AMON_D3)
        {
        printf("--->> Input string <<----\n");
        printf("%s\n",buf);
        dbugout(buf,alen);
        printf("--->> Found [%d] Command line args <<----\n",acount);
        for(i=0;args[i] != NULL;i++)
                printf("%s\n",args[i]);
        }

*argc = acount;
return((char **)args);

}

#endif /*DEBUG*/

/****************************************************************************/
/*****************    Endian Swapper    *************************************/
/****************************************************************************/

unsigned int swap_endian(unsigned int num)
{
   return(((num & 0x000000FF) << 24) + ((num & 0x0000FF00) << 8) +
          ((num & 0x00FF0000) >> 8) + ((num & 0xFF000000) >> 24));
}

#ifdef DEBUG

/****************************************************************************/
/*****************    Format Boot Header  ***********************************/
/****************************************************************************/

format_boot_header(IPL_REC *ipl_record)
{
printf("\n***** BOOT RECORD Contents *****\n");
printf("Magic # = %X - Valid magic = %s\n",ipl_record->IPL_record_id,
	(ipl_record->IPL_record_id == IPLRECID)?"TRUE":"FALSE");

printf("Media Formatted capacity ..... = 0x%X  (%d)\n",
	ipl_record->formatted_cap,ipl_record->formatted_cap );
printf("Last Head (diskette) ......... = 0x%X  (%d)\n",
	ipl_record->last_head,ipl_record->last_head );
printf("Sectors per Track (dskt) ..... = 0x%X  (%d)\n",
	ipl_record->last_sector,ipl_record->last_sector);
printf("Code length in sectors ....... = 0x%X  (%d)\n",
	ipl_record->boot_code_length,ipl_record->boot_code_length);
printf("Offset from start of boot code = 0x%X  (%d)\n",
	ipl_record->boot_code_offset,ipl_record->boot_code_offset);
printf("PSN of start of BLV .......... = 0x%X  (%d)\n",
	ipl_record->boot_lv_start,ipl_record->boot_lv_start);
printf("PSN of start of boot code .... = 0x%X  (%d)\n",
	ipl_record->boot_prg_start,ipl_record->boot_prg_start);
printf("Length of BLV in sectors ..... = 0x%X  (%d)\n",
	ipl_record->boot_lv_length,ipl_record->boot_lv_length);
printf("Load addr for boot code (blck) = 0x%X  (%d)\n",
	ipl_record->boot_load_add,ipl_record->boot_load_add);
printf("Boot fragmentation flag ...... = 0x%X  (%d)\n",
	ipl_record->boot_frag,ipl_record->boot_frag);
printf("Number of sectors for base cus = 0x%X  (%d)\n",
	ipl_record->basecn_length,ipl_record->basecn_length);
printf("Starting PSN for base cust ... = 0x%X  (%d)\n",
	ipl_record->basecn_start,ipl_record->basecn_start);
}

/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
// Check for valid drive information
/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

is_drive_valid(int drive, int show)
{
int	rc,i;
char	*t_buff;
DEVICE_Q_REC	*devreq;

if((t_buff = (char *)malloc(sizeof(DEVICE_Q_REC))) == NULL)		
	{
	printf("Malloc for temp buffer failed\n");
	return(FALSE);
	}

if (scsi_io(Device_Query,t_buff,drive,(int)NULL,(int)NULL)) 
	{
	printf("Device Query for drive %d failed\n",drive);
	free(t_buff);
	return(FALSE);
        } 

devreq = (DEVICE_Q_REC *)t_buff;                     

dbgtst(AMON_D10)
	{
	printf("Device Q information\n");
	printf("max_lba: 0x%8.8X  block_size: 0x%X  Device_Count: %d  Type: 0x%X \n",
		devreq->max_lba, devreq->block_size,devreq->Device_Count,devreq->type);
	printf("adapter: 0x%X  bus: 0x%X  id: 0x%X  lun: 0x%X\n",
		devreq->adapter, devreq->bus,devreq->id,devreq->lun);
	printf("Status:\n");
	printf("Ready: 0x%X\n",devreq->status.Ready);
	printf("VPD:\n");
	printf("Vendor: ");
	for(i=0;i<8;i++)
		printf("%02X",devreq->VPD.vendor[i]);
	printf("\n");
	printf("Product Name: ");
	for(i=0;i<16;i++)
		printf("%02X",devreq->VPD.product_name[i]);
	printf("  %s\n",devreq->VPD.product_name);
	}

if(show)
	printf(" %d %s",devreq->id,devreq->VPD.product_name);

if ((devreq->type != DASD) && (devreq->type != CDROM)) 
	{
	printf("Requested device not a disk or cdrom\n");
	free(t_buff);
	return(FALSE);
       	}

rc = FALSE;
				/* Build it into the table	*/
if( devreq->status.Ready) 
	{
	if(devreq->type == 5) mon_device_type = ROS_CDROM;
	if(devreq->type == 0) mon_device_type = ROS_DASD;
	mon_device_id = devreq->id;
	mon_device_lun = devreq->lun;
	rc = TRUE;
	}
free(t_buff);

return(rc);
}
#endif /*DEBUG*/
