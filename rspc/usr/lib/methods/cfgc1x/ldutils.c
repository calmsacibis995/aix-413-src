/* @(#)68    1.2  src/rspc/usr/lib/methods/cfgc1x/ldutils.c, isax25, rspc411, GOLD410  5/17/94  10:34:28 */
/*
 * COMPONENT_NAME: (ISAX25) cfgc1x
 *
 * FUNCTIONS: readmem(), writemem(), readio_reg(), writeio_reg()  
 *            issuecmd(), getbuffers(), intwait(), reset(),        
 *            getprimstat()                                       
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
?                                                                         ?
?                                                                         ?
? PURPOSE: Utility routines used by the microcode loader in lducode.c     ?
?                                                                         ?
? ALGORITHM DESCRIPTION: Described under each function                    ?
?                                                                         ?
? EXTERNAL VARIABLES USED: None                                           ?
?                                                                         ?
? PUBLIC VARIABLES DECLARED: readmem                                      ?
?                            writemem                                     ?
?                            readio_reg                                   ?
?                            writeio_reg                                  ?
?                            issuecmd                                     ?
?                            getbuffers                                   ?
?                            intwait                                      ?
?                            reset                                        ?
?                                                                         ?
? EXTERNAL ROUTINES CALLED:  None                                         ?
?                                                                         ?
읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mdio.h>
#include "cfgdebug.h"
#include "lducode.h"

/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
 ? Error codes returned by these routines  ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
#define   E_BUSIO               -100
#define   E_BUSMEM              -101
#define   E_INVALID_IB          -102
#define   E_CMD_REJECTED        -103
#define   E_TIMEOUT             -104

/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
 ? Macro to return the minimum of 2 integers ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
#define   min(x,y) (x<=y)?x:y

int readmem(fd, pos, page, offset, num, buffer)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Reads data from a specified address in the Shared Storage window to a    ?
 ? user space buffer.                                                       ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         number of page to read data from                                 ?
 ?         offset in page to read data from                                 ?
 ?         number of bytes to read                                          ?
 ?         user space buffer to read data into                              ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0        */
POS_INFO *pos;         /* Adapters POS reg info       */
uchar page;            /* Page to read                */
int offset;            /* Offset in page              */
uint num;              /* Number of bytes to read     */
uchar *buffer;         /* Copy data read here         */
{
   MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */
   uint absolute_addr;   /* Physical address of card's bus memory */
   uchar old_page;       /* Old CPU Page register value           */
   int count;            /* Number of bytes read on each loop     */
   int rc;               /* Return code from function calls       */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Save old CPU page register ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (rc = readio_reg(fd, pos->baseio + 5,&old_page))
      return(E_BUSIO);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Update CPU register if necessary   ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/

   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, page))
         return(E_BUSIO);
   }

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Translate page/offset into Microchannel address?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   absolute_addr = pos->win_base_addr + offset;

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Copy data from card, adjusting the CPU page register ?
    ? when necessary                                       ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   do
   {
      count = min((pos->win_size - offset), num);
      mddRecord.md_sla = 0;
      mddRecord.md_size = count;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = buffer;
      mddRecord.md_addr = absolute_addr;

      /* if (ioctl(fd, MIOBUSGET, &mddRecord)) */
      rc=ioctl(fd, MIOMEMGET, &mddRecord);
      DEBUG_3("readmem: addr=0x%X, data=0x%X, rc=%d\n",
		absolute_addr, *buffer, rc);
      DEBUG_3("readmem: md_size=0x%X, md_incr=0x%X, md_sla=%d\n",
		mddRecord.md_size, mddRecord.md_incr, mddRecord.md_sla);
      if (rc)
      {
         return(E_BUSMEM);
      }

      if (count < num)
      {
         num -= count;
         offset = 0;
         absolute_addr = pos->win_base_addr;
         buffer += count;
         if (rc = writeio_reg(fd, pos->baseio + 5, ++page))
            return(E_BUSIO);
      }
      else
         break;
   } while (1);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Restore old CPU page register if necessary ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, old_page))
         return(E_BUSIO);
   }

   /*컴컴컴컴컴컴컴컴컴?
    ? Return success   ?
    읕컴컴컴컴컴컴컴컴?*/
   return(0);
}


int writemem(fd, pos, page, offset, num, buffer)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Writes data to a specified address in the Shared Storage window from a   ?
 ? user space buffer.                                                       ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         number of page to write data to                                  ?
 ?         offset in page to write data to                                  ?
 ?         number of bytes to write                                         ?
 ?         user space buffer to copy data from                              ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0        */
POS_INFO *pos;         /* Adapters POS reg info       */
uchar page;            /* Page to read                */
int offset;            /* Offset in page              */
uint num;              /* Number of bytes to write    */
uchar *buffer;         /* Write data copied from here */
{
   MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */
   uint absolute_addr;   /* Physical address of card's bus memory */
   uchar old_page;       /* Old CPU Page register value           */
   int count;            /* Number of bytes read on each loop     */
   int rc;               /* Return code from function calls       */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Save old CPU page register ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (rc = readio_reg(fd, pos->baseio + 5,&old_page))
      return(E_BUSIO);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Update CPU register if necessary   ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, page))
         return(E_BUSIO);
   }

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Translate page/offset into Microchannel address?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   absolute_addr = (pos->win_base_addr + offset);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Copy data to card, adjusting the CPU page register   ?
    ? when necessary                                       ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   do
   {
      count = min((pos->win_size - offset), num);
      mddRecord.md_size = count;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = buffer;
      mddRecord.md_addr = absolute_addr;

      /* if (ioctl(fd, MIOBUSPUT, &mddRecord)) */
      rc = ioctl(fd, MIOMEMPUT, &mddRecord);
      DEBUG_3("writemem: addr=0x%X, data=0x%X, rc=%d\n",
		absolute_addr, *buffer, rc);
      if (rc)
         return(E_BUSMEM);

      if (count < num)
      {
         num -= count;
         offset = 0;
         absolute_addr = pos->win_base_addr;
         buffer += count;
         if (rc = writeio_reg(fd, pos->baseio + 5, ++page))
            return(E_BUSIO);
      }
      else
         break;
   } while (1);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Restore old CPU page register if necessary ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (old_page != page)
   {
      if (rc = writeio_reg(fd, pos->baseio + 5, old_page))
         return(E_BUSIO);
   }

   /*컴컴컴컴컴컴컴컴컴?
    ? Return success   ?
    읕컴컴컴컴컴컴컴컴?*/
   return(0);
}


int readio_reg(fd, ioaddr, data_ptr)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Reads an 8 bit byte from a given I/O port and copies to user space       ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         I/O address to read from                                         ?
 ?         user space buffer to copy data to                                ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0  */
int ioaddr;            /* I/O address          */
uchar *data_ptr;       /* Put reg contents here */
{
      MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */

      mddRecord.md_size = 1;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = data_ptr;
      mddRecord.md_addr = ioaddr;

      if(ioctl(fd, MIOBUSGET, &mddRecord))
	  return(E_BUSIO);
      return(0);
}

int writeio_reg(fd, ioaddr, data)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Writes an 8 bit byte to a given I/O port                                 ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         I/O address to write to                                          ?
 ?         Byte to write to port                                            ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0  */
int ioaddr;            /* Base I/O address      */
uchar data;            /* Data to write         */
{
      MACH_DD_IO mddRecord; /* machine dd ioctl buffer               */

      mddRecord.md_size = 1;
      mddRecord.md_incr = MV_BYTE;
      mddRecord.md_data = &data;
      mddRecord.md_addr = ioaddr;

      if(ioctl(fd, MIOBUSPUT, &mddRecord))
         return(E_BUSIO);

      return(0);
}

int issuecmd(fd, pos, tasknum, cmd, timeout, parms_length, parms)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Issues a command to a task (or the ROS) on the card                      ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         Task number to issue command to                                  ?
 ?         Command code to send to task                                     ?
 ?         Time in ms for command to be accepted                            ?
 ?         Length of command parameters                                     ?
 ?         Pointer to command parameters                                    ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number to issue command to       */
uchar cmd;             /* Command                               */
uint timeout;          /* Time to wait for response             */
int parms_length;      /* Parms for command                     */
char *parms;           /* Ptr to parms                          */
{
   MACH_DD_IO mddRecord;
   uchar pc_select;
   uchar obinfo[3];
   uchar buf[2];
   ushort bcb_addr;
   uchar ob_page;
   uint ob_offset;
   int i;
   int rc;

    /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
     쿣erify valid PC Select Byte  ?
     읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (rc = readmem(fd, pos, 0, 0x440, 1, &pc_select))
      return(rc);
   if (pc_select != 0xFF)
      return(E_INVALID_IB);

   /*컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Get task's BCB offset  ?
    읕컴컴컴컴컴컴컴컴컴컴컴?*/
   if (rc = readmem(fd, pos, 0, 0x45A, 2, buf))
      return(rc);
   bcb_addr = (buf[1] << 8) | buf[0];
   bcb_addr += (tasknum << 4);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Write command code in BCB   ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (rc = writemem(fd, pos, 0, bcb_addr, 1, &cmd))
      return(rc);

    /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
     ? Write task number in PC Select byte ?
     읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (rc = writemem(fd, pos, 0, 0x440, 1, &tasknum))
      return(rc);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Write any parms to output buffer ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (parms_length)
   {
      /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
       ? Read in task's output buffer address  ?
       읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
      if (rc = readmem(fd, pos, 0, bcb_addr + 0x0D, 3, obinfo))
         return(rc);

      ob_page = obinfo[2];
      ob_offset = (obinfo[1] << 8) | obinfo[0];

      /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
       ? Write parms to output buffer   ?
       읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
      if (rc = writemem(fd, pos, ob_page, ob_offset, parms_length, parms))
         return(rc);
   }

   /*컴컴컴컴컴컴컴컴컴컴?
    ? Interrupt adapter  ?
    읕컴컴컴컴컴컴컴컴컴?*/
   if (rc = writeio_reg(fd, pos->baseio + 2, 0x09))
            return(E_BUSIO);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Wait for command acknowledgement  ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   for (i = 0; (i < timeout) || (i == 0); ++i)
   {
      if (rc = readmem(fd, pos, 0, 0x440, 1, &pc_select))
         return(rc);
      if (pc_select == 0xFF) break;
      if (timeout)
         usleep(1000);
   }

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Did we timeout or was command rejected ? ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (pc_select != 0xFF)
   {
      if (pc_select == 0xFE)
         return(E_CMD_REJECTED);
      else
         return(E_TIMEOUT);
   }
   else
      return(0);
}

int intwait(fd, pos, tasknum, timeout)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Waits for an adapter interrupt from a specific task                      ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         Task number expected to interrupt                                ?
 ?         Time in ms to wit for interrupt                                  ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number interrupting              */
int timeout;           /* Time to wait for interrupt            */
{

   int i;              /* Loop counter                     */
   int rc;             /* Return code from function calls  */
   uchar int_id;       /* Interface Block's Int ID byte    */
   uchar rest_int_id;  /* Used to restore original Int ID  */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Wait for interrupt from card   ?
    ?  By looking at Interrupt ID byte?
    ?  in  Interface Block            ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   for (i = 0; (i < timeout) || (i == 0); ++i)
   {
      if (rc = readmem(fd, pos, 0, 0x441, 1, &int_id))
         return(rc);
      if (int_id == tasknum) break;
      if (timeout)
         usleep(1000);
   }
   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ?  Restore int id for next interrupt?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (int_id != 0xFF)
   {
      rest_int_id = 0xFF;
      if (rc = writemem(fd, pos, 0, 0x441, 1, &rest_int_id))
      {
         return(rc);
      }
   }

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Return E_TIMEOUT if int id has not changed ?
    ? else return SUCCESS                        ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (int_id == tasknum)
      return(0);
   else
      return(E_TIMEOUT);

}

int getbuffers(fd, pos, tasknum, bufs)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Gets a tasks BCB page/offset  addresses                                  ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         Buffer to write addresses                                        ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number interrupting              */
BUFFER_ADDRS *bufs;    /* Store addresses here                  */
{
   ushort bcb_addr;    /* Starting address of task's BCB  */
   uchar bcb[16];      /* Tasks BCB entry                 */
   int rc;             /* Return code from function calls */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Get task's BCB pointer and calculate address of BCB ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
  if (rc = readmem(fd, pos, 0, 0x45A, 2, bcb))
   {
      return(rc);
   }

   bcb_addr = (bcb[1] << 8) | bcb[0];
   bcb_addr += (tasknum << 4);

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Read in task's  buffer addresses?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   if (rc = readmem(fd, pos, 0, bcb_addr, 15, bcb))
   {
      return(rc);
   }

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Copy addresses to caller's buffer ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   bufs->sspage = bcb[5];
   bufs->ssoffset = (bcb[4] << 8)  | bcb[3];
   bufs->inpage = bcb[10];
   bufs->inoffset = (bcb[9] << 8)  | bcb[8];
   bufs->outpage = bcb[15];
   bufs->outoffset = (bcb[14] << 8)  | bcb[13];

   return(0);
}

int getprimstat(fd, pos, tasknum, primstat)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Gets a tasks primary status byte                                         ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?         Buffer to write PSB                                              ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
uchar tasknum;         /* Task number                           */
uchar *primstat;       /* Store PSB here                        */
{
   ushort bcb_addr;    /* Starting address of task's BCB  */
   uchar bcb[16];      /* Tasks BCB entry                 */
   int rc;             /* Return code from function calls */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
    ? Read task's PSB from Interface Block                ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴*/
   return(readmem(fd, pos, 0, 0x47C + tasknum, 1, primstat));
}

int reset(fd, pos)
/*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
 ? Resets an adapter                                                        ?
 ?                                                                          ?
 ? INPUTS: file descriptor for machine device driver                        ?
 ?         POS register info of target adapter                              ?
 ?                                                                          ?
 ? OUTPUTS: return code       0       no errors                             ?
 ?                            < 0     Error                                 ?
 읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
int fd;                /* Handle for /dev/bus0                  */
POS_INFO *pos;         /* Adapters POS reg info                 */
{
   uchar comreg;       /* Command register value          */
   uchar value=0;      /* I/O register value		*/
   ushort crd_id=0;    /* card ID value			*/
   uchar boot=0xC0;    /* adapter boot value		*/
   int rc;             /* Return code from function calls */
   int i;              /* Loop counter                    */

   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Do the hardware reset by setting COMREG to 0x11 (reset command)?
    ? then to 0 to clear it and finally to 0x10 to enable interrupts ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
   if (rc = writeio_reg(fd, pos->baseio + 6, 0x11))
      return(rc);

  sleep(1);
   if (rc = writeio_reg(fd, pos->baseio + 6, 0x00))
      return(rc);

  sleep(1);
   if (rc = writeio_reg(fd, pos->baseio + 6, 0x10))
      return(rc);

	
  sleep(1);
   /*컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?
    ? Wait up to 1 minute for adapter to complete the    ?
    ? reset process. Check PROMREADY bit in INITREG1 to  ?
    ? see if reset has completed successfully            ?
    읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴?*/
  for (i = 0; i < 60000; ++i)
  {
    if (value != 0xAA)
    {
      if (rc = writeio_reg(fd, pos->baseio + 2, 0xAA))
	return(rc);

      if (rc = readio_reg(fd, pos->baseio + 2, &value))
	return(rc);
      DEBUG_1("cfgampx reset: PTRREG=0x%X\n", value);

    }
    else
    {
      if (crd_id != 0xBAAB)
      {
	if (rc = readmem(fd, pos, 0, 0x40C, 2, &crd_id))
	  return(rc);
      DEBUG_1("cfgampx reset: CARDID=0x%X\n", crd_id);
      }
      else
      {
	if (boot == 0xC0)
	{
	  if (rc = readmem(fd, pos, 0, 0x47C, 1, &boot))
	    return(rc);
            DEBUG_1("cfgampx reset: boot=0x%X\n", boot);
	}
	else
	{
	      return(0);
	}
      }
    }
    usleep(1000);
  }
  return(E_TIMEOUT);
}

