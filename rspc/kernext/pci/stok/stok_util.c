static char sccsid[] = "@(#)54  1.2  src/rspc/kernext/pci/stok/stok_util.c, pcitok, rspc41J, 9515A_all 4/11/95 13:41:28";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: cdt_add
 *		cdt_del
 *		cdt_func
 *		hexdump
 *		pio_retry
 *		reset_adapter
 *		stok_logerr
 *		stok_trace
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/mbuf.h>
#include <sys/malloc.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/cdli.h>
#include <sys/ndd.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

extern stok_dd_ctl_t  stok_dd_ctl;


/*****************************************************************************/
/*
 * NAME:     pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an exception.
 *  	Retry the pio function and do error logging.
 *
 * EXECUTION ENVIRONMENT: process and interrupt.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      
 * INPUT:
 *      p_dev_ctl       - device control structure.
 *      excpt_code      - exception code from first pio 
 *      iofunc          - io function to retry
 *      ioaddr          - io address of the exception
 *      ioparam         - parameter to PIO routine
 *      cnt             - for string copies
 *
 * RETURNS:  0 - retry successfully
 *        exception code
 */
/*****************************************************************************/
int pio_retry (
stok_dev_ctl_t  *p_dev_ctl,	/* adapter control structure        */
enum pio_func   iofunc,		/* io function to retry	            */
int   		ioaddr,		/* io address of the exception	    */
long  		ioparam,	/* parameter to PIO routine	    */
int  		cnt)		/* for string copies                */

{
  ndd_t         *p_ndd = &(NDD);
  ushort        param1;
  ushort        param2;
  int	        retry_count = PIO_RETRY_COUNT;
  ndd_statblk_t stat_blk;      /* status block               */
  xmit_elem_t   *xelm;         /* transmit element structure */
  int  		excpt_code;	/* exception code from original PIO */

  TRACE_SYS(STOK_OTHER, "UioB", p_dev_ctl, excpt_code, (ulong)iofunc);
  TRACE_SYS(STOK_OTHER, "UioC", (ulong)ioaddr, ioparam, 0);

  while (TRUE) {

  	/* 
         * Checks if out of retries
         */
  	if (retry_count <= 0) {
  		break;
  	}
  	retry_count--;

  	/* 
         * Retries the pio function, return if successful
         */
  	switch (iofunc)
  	{
  	case PUTCX:
  		excpt_code = BUS_PUTCX((char *)ioaddr, (char)ioparam);
  		break;
  	case PUTSX:
  		excpt_code = BUS_PUTSX((short *)ioaddr, (short)ioparam);
  		break;
  	case PUTSRX:
  		excpt_code = BUS_PUTSRX((short *)ioaddr,(short)ioparam);
  		break;
  	case PUTLX:
  		excpt_code = BUS_PUTLX((long *)ioaddr,(long)(ioparam));
  		break;
  	case PUTLRX:
  		excpt_code=BUS_PUTLRX((long *)ioaddr,(long)(ioparam));
  		break;
  	case GETCX:
  		excpt_code = BUS_GETCX((char *)ioaddr, (char *)ioparam);
  		break;
  	case GETSX:
  		excpt_code=BUS_GETSX((short *)ioaddr,(short *)ioparam);
  		break;
  	case GETSRX:
  		excpt_code=BUS_GETSRX((short *)ioaddr,(short *)ioparam);
  		break;
  	case GETLX:
  		excpt_code=BUS_GETLX((long *)ioaddr,(long *)ioparam);
  		break;
  	case GETLRX:
  		excpt_code=BUS_GETLRX((long *)ioaddr,(long *)ioparam);
  		break;
  	case PUTSTRX:
  		excpt_code = BUS_PUTSTRX((long *)ioaddr, 
  						(long *)ioparam, cnt);
  		break;
  	case GETSTRX:
  		excpt_code = BUS_GETSTRX((long *)ioaddr, 
  						(long *)ioparam, cnt);
  		break;
  	default:
  		ASSERT(0);
  	} /* End of switch (iofunc) */

  	if (excpt_code == 0) {
  		break;
  	}

  } /* End of while (TRUE) */

  TRACE_SYS(STOK_OTHER, "UioE", p_dev_ctl, (ulong)excpt_code, 0);
  return (excpt_code);

}

/*****************************************************************************/
/*
 * NAME:     cdt_func
 *
 * FUNCTION: return the address of the component dump table 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	stok_close
 *	stok_open
 *
 * INPUT:
 *  none.
 *
 * RETURNS:  the address of the component dump table
 */
/*****************************************************************************/
struct cdt *cdt_func()
{

  return((struct cdt *)&CDT->head);

}
/*****************************************************************************/
/*
 * NAME:     cdt_add
 *
 * FUNCTION: add an entry to the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	stok_open
 *
 * INPUT:
 *  name		- character string of the name of the data structure
 *  addr		- address of the data structure
 *  len		- length of the data structure
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
void cdt_add(
char *name,    /* label string for area dumped */
char *addr,    /* address of the area to be dumped */
int   len)     /* amount of data to be dumped */

{
  struct cdt_entry *p_entry;
  int             size;
  stok_cdt_t      *p_old, *p_new;


  TRACE_SYS(STOK_OTHER, "OdaB", (ulong)name, (ulong)addr, len);

  if (CDT->cdt_max_entry <= CDT->count) {
        /*
         * enlarge the entries by STOK_CDT_SIZE
         */
	CDT->cdt_max_entry = CDT->count + STOK_CDT_SIZE;
        size = 8 + sizeof (struct cdt_head) + 
                   (CDT->cdt_max_entry * sizeof(struct cdt_entry));

        /* grow dump table */
        p_new = (stok_cdt_t *) xmalloc (size,  MEM_ALIGN, pinned_heap);

        if (p_new == NULL)
        {
                /*
                 * can't expand dump table drop entry
                 */
  		TRACE_BOTH(STOK_ERR, "Oda1", (ulong)name, (ulong)addr, len);
                return;
        }
        /*
         * initialize new dump table with old values
         * (cdt_len is copied as well)
         */
        size = 8 + sizeof (struct cdt_head) + 
                   (CDT->count * sizeof(struct cdt_entry));
        bcopy (CDT, p_new, size);

        /*
         * swap curent (old) dump table with new
         *      Notice p_kent_cdt will always have a valid
         *      pointer to avoid errors if the dump function
         *      were called during the swap
         */
        p_old = CDT;
        CDT = p_new;
        xmfree (p_old, pinned_heap);
  }

  p_entry = &CDT->entry[CDT->count];
  strcpy(p_entry->d_name, name);
  p_entry->d_len = len;
  p_entry->d_ptr = addr;
  p_entry->d_xmemdp = NULL;

  CDT->count++;
  CDT->head._cdt_len += sizeof(struct cdt_entry);

  TRACE_SYS(STOK_OTHER, "OdaE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     cdt_del
 *
 * FUNCTION: delete an entry from the component dump table
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * CALLED FROM:
 *	stok_open
 *	stok_close
 *
 * INPUT:
 *  name	- character string of the name of the data structure
 *  addr	- address of the data structure
 *  len		- length of the data structure
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
void cdt_del(
char *name,    /* label string for area dumped */
char *addr,    /* address of the area to be dumped */
int   len)     /* amount of data to be dumped */

{
  struct cdt_entry *p_entry;
  int i;



  TRACE_SYS(STOK_OTHER, "OddB", (ulong)name, (ulong)addr, len);

  /* 
   * Searchs the entry in the table (match only the memory pointer) 
   */
  for (p_entry = &CDT->entry[0], i = 0; i < CDT->count; p_entry++, i++) {
  	if (p_entry->d_ptr == addr)
  		break;
  }

  /* 
   * If found the entry, remove the entry by re-arrange the table 
   */
  if (i < CDT->count) {
  	for (; i < CDT->count; p_entry++, i++) {
  		strcpy(p_entry->d_name, CDT->entry[i+1].d_name);
  		p_entry->d_len = CDT->entry[i+1].d_len;
  		p_entry->d_ptr = CDT->entry[i+1].d_ptr;
  	}
  	CDT->count--;
  	CDT->head._cdt_len -= sizeof(struct cdt_entry);

  }
  TRACE_SYS(STOK_OTHER, "OddE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     stok_trace
 *
 * FUNCTION: Put a trace into the internal trace table and the external
 *       system trace.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *  This routine is only called through macros when the DEBUG is defined.
 *  
 * CALLED FROM:
 *
 * INPUT:
 *  	hook		- trace hook 
 *  	tag		- four letter trace ID
 *  	arg1 to arg 3   - arguments to trace
 *
 * RETURNS:  none.
 *
 */
/*****************************************************************************/
stok_trace(
ulong  hook,	/* trace hook */
char  	*tag,	/* trace ID */
ulong  arg1,	/* 1st argument */
ulong  arg2,	/* 2nd argument */
ulong  arg3)	/* 3rd argument */

{

  int i;
  int ipri;

  ipri = disable_lock(PL_IMP, &TRACE_LOCK);

  /*
   * Copys the trace point into the internal trace table
   */
  i = stok_dd_ctl.trace.next_entry;
  stok_dd_ctl.trace.table[i] = *(ulong *)tag;
  stok_dd_ctl.trace.table[i+1] = arg1;
  stok_dd_ctl.trace.table[i+2] = arg2;
  stok_dd_ctl.trace.table[i+3] = arg3;

  if ((i += 4 ) >= ((STOK_TRACE_SIZE) - 4)) {
        i = 4;
  }
  stok_dd_ctl.trace.next_entry = i;

  strcpy(((char *)(&stok_dd_ctl.trace.table[i])), STOK_TRACE_CUR);

  unlock_enable(ipri, &TRACE_LOCK);

  /*
   * Calls the external trace routine
   */
  TRCHKGT(hook | HKTY_GT | 4, *(ulong *)tag, arg1, arg2, arg3, 0);

}


/*****************************************************************************/
/*
 * NAME:     stok_logerr
 *
 * FUNCTION: Collect information for making system error log entry.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * INPUT:
 *      p_dev_ctl       - pointer to device control area
 *      errid           - error id for logging
 *      line            - code line number
 *      fname           - code file name
 *      parm1           - log 4 bytes data 1
 *      parm2           - log 4 bytes data 2
 *      parm3           - log 4 bytes data 3
 *
 */
/*****************************************************************************/
void stok_logerr(
  stok_dev_ctl_t *p_dev_ctl,       /* pointer to dev_ctl area */
  ulong    errid,              	/* Error id for logging */
  int      line,               	/* line number */
  char     *p_fname,           	/* file name */
  ulong    parm1,              	/* log 4 bytes data 1 */
  ulong    parm2,              	/* log 4 bytes data 2 */
  ulong    parm3)              	/* log 4 bytes data 3 */

{
  struct  error_log_def   log;
  uchar   lbuf[80];
  int i;


  TRACE_SYS(STOK_OTHER, "ElgB", (ulong)p_dev_ctl, (ulong)errid, line);
  TRACE_SYS(STOK_OTHER, "ElgC", parm1, parm2, parm3);

  /* 
   * Loads the error id and device driver name into the log entry 
   */
  log.errhead.error_id = errid;
  strncpy(log.errhead.resource_name, DDS.lname, ERR_NAMESIZE);

  /* 
   * Puts the line number and filename in the table 
   */
  sprintf(lbuf, "line: %d file: %s", line, p_fname);
  strncpy(log.fname, lbuf, sizeof(log.fname));

  /* 
   * Loads Network address in use value into the table  
   */
  for (i = 0; i < CTOK_NADR_LENGTH; i++)
          log.stok_addr[i] = WRK.stok_addr[i];

  /* 
   * Starts fill in the table with data  
   */
  log.parm1 = parm1;
  log.parm2 = parm2;
  log.parm3 = parm3;

  /* 
   * Logs the error message 
   */
  errsave(&log, sizeof(struct error_log_def));

  TRACE_SYS(STOK_OTHER, "ElgE", (ulong)p_dev_ctl, 0, 0);

}

/****************************************************************************/
/*
 * NAME: reset_adapter
 *
 * FUNCTION:
 *     This function send a adapter reset request to the adapter
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine executes on the interrupt level or process thread.
 *
 * CALLED FROM:
 *	stok_open
 *	stok_close
 *	stok_bug_out
 *	stok_enter_limbo
 *
 */
/****************************************************************************/

int reset_adapter( stok_dev_ctl_t  *p_dev_ctl) {
  
  ushort        bmctl = CHK_DIS_R, i;
  ulong	        halt = FALSE, ioa;

 TRACE_SYS(STOK_OTHER, "LubB", (int)p_dev_ctl, 0, 0);

  /* requests Tx & Rx channel disable */
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTSRX(ioa + BMCTL_SUM, bmctl);
  PIO_PUTSRX(ioa + BMCTL_SUM, bmctl);

  /* make sure DBA of Tx1 is not moving */
  TX_DISABLE(TX1DBA_L);

  /* make sure DBA of Tx2 is not moving */
  TX_DISABLE(TX2DBA_L);

  /* check for DMA disable */
  i = 0;
  do {
        if (i++ > 2000) {
                TRACE_BOTH(STOK_ERR, "Lub1", (int)p_dev_ctl, bmctl, 0);
		halt = TRUE;
                break;
        }

        io_delay(10);
        PIO_GETSRX(ioa + BMCTL_SUM, &bmctl);
  } while ((bmctl != CHK_DIS) & (bmctl != TX_DIS));

/* F63S   
  if (halt) {
  	iomem_det((void *)ioa);            
	return (halt);
  }
*/

  /*
   * Turn on soft reset
   */
  PIO_PUTSRX( ioa + BCTL, SOFT_RST_MSK);
  io_delay(10);         /* Wait for adapter to reset */
  PIO_PUTSRX( ioa + BCTL, 0x0000);
  iomem_det((void *)ioa);            /* restore I/O Bus          */

  if (WRK.pio_rc) {
        TRACE_BOTH(STOK_ERR, "Lub4", (int)p_dev_ctl, WRK.pio_rc, 0);
  }
 TRACE_SYS(STOK_OTHER, "LubE", (int)p_dev_ctl, 0, 0);

  return (WRK.pio_rc);

}


/*****************************************************************************/
/*
 * NAME: hexdump
 *
 * FUNCTION: Display an array of type char in ASCII, and HEX.
 *
 * EXECUTION ENVIRONMENT: process or interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *
 * INPUT:
 *      data       - data pointer 
 *      len        - length of data 
 *
 * RETURNS:
 *      none.
 */
/*****************************************************************************/

hexdump(data,len)
char *data;
long len;
{

  int     i,j,k;
  char    str[18];

  printf("hexdump(): length = %x\n",len);
  i=j=k=0;
  while(i<len)
  {
          j=(int) data[i++];
          if(j>=32 && j<=126)
                  str[k++]=(char) j;
          else
                  str[k++]='.';
          printf("%02x ",j);
          if(!(i%8)) {
                  printf("  ");
                  str[k++]=' ';
          }
          if(!(i%16)) {
                  str[k]='\0';
                  printf("     %s\n",str);
                  k=0;
          }
  }
  while(i%16)
  {
          if(!(i%8)) {
                  printf("  ");
	  }
          printf("   ");
          i++;
  }
  str[k]='\0';
  printf("       %s\n\n",str);
  ;
}

