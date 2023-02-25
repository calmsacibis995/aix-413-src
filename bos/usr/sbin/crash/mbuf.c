 static char sccsid[] = "@(#)84  1.14  src/bos/usr/sbin/crash/mbuf.c, cmdcrash, bos411, 9430C411a 7/19/94 16:50:41";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prmbuf, mtrack
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	"crash.h"
#include	<sys/param.h>
#include	<sys/mbuf.h>


/*
 * NAME: prmbuf
 *                                                                    
 * FUNCTION: displays the mbuf structures in the system
 *                                                                    
 */  
prmbuf(addr)
struct mbuf	*addr;
{
	struct	mbuf	mbuf;
	char		*type;
	int		dptr;

	/*
	 * All mbufs will be aligned on 256 byte boundary.  We check
	 * here to ensure that the given address is aligned correctly
	 */
	if ((int)addr & (sizeof(struct mbuf) - 1)) {
		printf( catgets(scmc_catd, MS_crsh, MBUF_MSG_1, "Not a valid mbuf pointer:%8x, must be on %d byte boundary!\n") ,
			addr, sizeof(struct mbuf));
		return;
	}

	/* Run the chain of mbufs and display each one */
	for (; addr != NULL; addr = mbuf.m_next) {

		/* Read in the next mbuf structure */
		if (readmem(&mbuf, addr, sizeof(mbuf), CURPROC) 
		    != sizeof(mbuf)) {
			printf( catgets(scmc_catd, MS_crsh, MBUF_MSG_2, "0452-159: Cannot read mbuf structure at address %8x.\n") , addr);
			return;
		}

		/* Convert the type code to a descriptive ascii string */
		        if(mbuf.m_type == MT_FREE)       	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_3, "free")  ;
			else if(mbuf.m_type == MT_DATA)		 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_4, "data")  ;
			else if(mbuf.m_type == MT_HEADER)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_5, "header")  ;
			else if(mbuf.m_type == MT_SOCKET)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_6, "socket")  ;
			else if(mbuf.m_type == MT_PCB)	         type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_7, "pcb")  ;
			else if(mbuf.m_type == MT_RTABLE)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_8, "rtable")  ;
			else if(mbuf.m_type == MT_HTABLE)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_9, "htable")  ;
			else if(mbuf.m_type == MT_ATABLE)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_10, "atable")  ;
			else if(mbuf.m_type == MT_SONAME)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_11, "soname")  ;
			else if(mbuf.m_type == MT_SOOPTS)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_13, "soopts")  ;
			else if(mbuf.m_type == MT_FTABLE)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_14, "ftable")  ;
			else if(mbuf.m_type == MT_RIGHTS)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_15, "rights")  ;
			else if(mbuf.m_type == MT_IFADDR)	 type =	 catgets(scmc_catd, MS_crsh, MBUF_MSG_16, "ifaddr")  ;
			else                            	 type =  catgets(scmc_catd, MS_crsh, MBUF_MSG_17, "weird") ;

		/* Calculate the address of the start of data for the mbuf */
		dptr = mbuf.m_data;

		/* Display the mbuf structure */
		printf("0x%08x  %4d  %6s  0x%08x  0x%08x\n", addr,
			mbuf.m_len, type, mbuf.m_act, dptr);
		/*
		 * show all data associated with the mbuf.
		 */
		{
			/* 
			 * CLBYTES is the largest amount of data that
			 * can be associated with a cluster.
			 */
			int 		buf[CLBYTES/sizeof(int)];
			register int	i, size;

			size = mbuf.m_len;

			/* read in the data area */
			if (readmem(buf, dptr, size, CURPROC) != size) {
		       	    printf( catgets(scmc_catd, MS_crsh, MBUF_MSG_19, 
                            "0452-172: Cannot read mbuf data at address %8x.\n") , dptr);
			    return;
			}

			/* Display the data to the screen */
			for (i = 0; i < size/sizeof(int); i++) {
				if (!(i % 6))
					printf("\n\t");
	 			printf("0x%08x ",buf[i]);
			}
			printf((i % 6)?"\n\n":"\n");
		}
	}
}

char *types[] = {
    "free",
    "data",
    "header",
    "socket",
    "pcb",
    "rtable",
    "htable",
    "atable",
    "soname",
    "zombie",
    "soopts",
    "ftable",
    "rights",
    "ifaddr",
    "bogus"
};

mtrack(struct mbuf *addr)
{
	struct mbuf m;
	int t;
	static indent = 0;

	while (addr) {
		if (!readmem(&m, (int)addr, sizeof(m))) {
			printf("%08x not in dump\n", addr);
			return;
		}

		t = m.m_type;
		if (t < 0 || t > (sizeof(types) / sizeof(types[0])))
			t = (sizeof(types) / sizeof(types[0])) - 1;
		printf("%*s%08x %6s %4d -> %08x => %08x\n", indent, "", addr,
		       types[t], m.m_len, m.m_next, m.m_act);
		if (M_HASCL(&m)) {
			int cluster = (int)m.m_data;

			printf("%*s%08x (cluster) + %08x\n", indent, "",
				cluster & ~CLOFSET, cluster & CLOFSET);
		}

		if (m.m_next) {
			++indent;
			mtrack(m.m_next);
			--indent;
		}
		addr = m.m_act;
	}
}
