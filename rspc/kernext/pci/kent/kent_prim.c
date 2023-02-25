static char sccsid[] = "@(#)63	1.2  src/rspc/kernext/pci/kent/kent_prim.c, pcient, rspc41J, 9514A_all 4/3/95 19:05:43";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: kent_logerr
 *		kent_pio_retry
 *		kent_trace
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

#include "kent_proto.h"
#include <sys/errno.h>
#include <sys/lockname.h>
#include <sys/dump.h>

/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_pio_retry
 *
 * FUNCTION: This routine is called when a pio routine returns an
 *	exception. It will retry the PIO.  
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Called by interrupt and processes level
 *	This routine is invoked by the PIO_xxxX routines
 *
 * RETURNS:
 *	Boolean: set to whether the pio was successful within the 
 *		PIO_RETRY_COUNT attempts
 */

int
kent_pio_retry (
	kent_acs_t	*p_acs,	   	/* tells which adapter this came from*/
	enum pio_func 	iofunc,		/* io function to retry	*/
	void 		*ioaddr,	/* io address of the exception	*/
	long		ioparam,	/* parameter to PIO routine	*/
	int		cnt)		/* for string copies */

{
	int	retry_count = PIO_RETRY_COUNT;
	int	ipri;
	int 	rc;


	while (retry_count > 0)
	{
		/* 
		 * retry the pio function, return if successful
		 */
		switch (iofunc)
		{
			case PUTC:
				rc = BUS_PUTCX((char *)ioaddr,
							(char)ioparam);
				break;
			case PUTSR:
				rc = BUS_PUTSRX((short *)ioaddr,
							(short)ioparam);
				break;
			case PUTLR:
				rc = BUS_PUTLRX((long *)ioaddr,
							(long)ioparam);
				break;
			case GETC:
				rc = BUS_GETCX((char *)ioaddr,
							(char *)ioparam);
				break;
			case GETSR:
				rc = BUS_GETSRX((short *)ioaddr,
							(short *)ioparam);
				break;
			case GETLR:
				rc = BUS_GETLRX((long *)ioaddr,
							(long *)ioparam);
				break;
			case PUTSTR:
				rc = BUS_PUTSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			case GETSTR:
				rc = BUS_GETSTRX((long *)ioaddr,
							(long *)ioparam,
							cnt);
				break;
			default:
				KENT_ASSERT(0);
		}

		if (rc == 0)
		{
			/* ok */
			return(FALSE);
		}

		retry_count--;
	}
	return (TRUE);
}

/*
 * NAME: kent_trace
 *                                                                    
 * FUNCTION: 
 *	This routine puts a trace entry into the internal device
 *	driver trace table.
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES:  See the design for the trace format
 *
 * RETURNS:  none
 */  


void
kent_trace (	register uchar	str[],	/* trace data Footprint */
		register ulong	arg2,	/* trace data */
		register ulong	arg3,	/* trace data */
		register ulong	arg4)	/* trace data */

{
	register int	i;
	register char	*p_str;
	register int	ipri;

	ipri = disable_lock(KENT_OPLEVEL,&kent_tbl.trace_lock);
	/*
	 * get the current trace point index 
	 */
	i= kent_tbl.trace.next;

	p_str = &str[0];

	/*
	 * copy the trace point data into the global trace table.
	 */
	kent_tbl.trace.table[i] = *(ulong *)p_str;
	++i;
	kent_tbl.trace.table[i] = arg2;
	++i;
	kent_tbl.trace.table[i] = arg3;
	++i;
	kent_tbl.trace.table[i] = arg4;
	++i;


	if ( i < KENT_TRACE_SIZE )
	{
		kent_tbl.trace.table[i] = 0x21212121;	/* end delimeter */
		kent_tbl.trace.next = i;
	}
	else
	{
		kent_tbl.trace.table[0] = 0x21212121;	/* end delimeter */
		kent_tbl.trace.next = 0;
	}


	unlock_enable(ipri,&kent_tbl.trace_lock);

	return;
} /* end kent_trace */


/*
 * NAME: kent_logerr()
 *                                                                    
 * FUNCTION: 
 *	This routine logs an error to the system
 *                                                                    
 * EXECUTION ENVIRONMENT: process or interrupt environment
 *                                                                   
 * NOTES: 
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES: None
 *
 * RETURNS:  none
 */  

void
kent_logerr (	kent_acs_t	*p_acs,
		ulong		errid,
		int		line,
		char		*p_file,
		int		status1,
		int		status2,
		int		status3)
{ 

	int		i;
	kent_errlog_t	log;
	char		errbuf[300];
	int 		iocc;


	bzero(&log, sizeof(kent_errlog_t));

	/*
	 * Store the error id into the log entry
	 */
	log.errhead.error_id = errid;

	/* plug in the resource name for this device */
	strncpy( log.errhead.resource_name, p_acs->dds.lname, ERR_NAMESIZE);

 	for (i=0;i<ENT_NADR_LENGTH; ++i)
		if (p_acs->dds.use_alt_addr)
	  		log.src_addr[i] = p_acs->dds.alt_addr[i];
		else
	  		log.src_addr[i] = p_acs->addrs.src_addr[i];

	/* plug in the line number and filename */
	sprintf(errbuf, "line: %d file: %s", line, p_file);

	strncpy(log.file, errbuf, (size_t)sizeof(log.file));

	log.ent_genstats = p_acs->ls.ent_gen_stats;

	log.state = p_acs->dev.state;
	log.iox = p_acs->dev.iox;
	log.pio_rc = p_acs->dev.pio_rc;
	log.status1 = status1;
	log.status2 = status2;
	log.status3 = status3;

	/* log the error */
	errsave(&log, sizeof(kent_errlog_t) );
	return;
} /* end kent_logerr() */

