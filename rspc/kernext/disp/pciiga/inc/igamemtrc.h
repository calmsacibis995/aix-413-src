/* @(#)58	1.1  src/rspc/kernext/disp/pciiga/inc/igamemtrc.h, pciiga, rspc41B, 9504A 12/15/94 16:31:50 */

/*
 * COMPONENT_NAME: (pciiga) Icecube Device Driver 
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_IGAMEMTRC
#define _H_IGAMEMTRC

#define IGADD_MEMTRACE_TITLE_SIZE	(16)
#define IGADD_MEMTRACE_SIZE		(1024)

typedef struct igadd_trace_entry {

        ulong	id;
        ulong	parm1;
        ulong	parm2;
        ulong	parm3;

} igadd_trace_entry_t;


typedef struct igadd_trace_header {

	char			title[IGADD_MEMTRACE_TITLE_SIZE];

	ulong			devno, pd, ddf, dds;

	ulong			num_wraps;

	igadd_trace_entry_t 	*top; 
	igadd_trace_entry_t 	*next; 
	igadd_trace_entry_t 	*last; 

	ulong			spare_1;
	ulong			spare_2;
	ulong			spare_3;
	ulong			spare_4;

} igadd_trace_header_t;

#endif /* _H_IGAMEMTRC */

