/* @(#)28	1.1  src/rspc/kernext/disp/pcigga/inc/ggamemtrc.h, pcigga, rspc41B, 9504A 1/9/95 13:26:07 */

/*
 * COMPONENT_NAME: (pcigga) Glacier Device Driver 
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

#ifndef _H_GGAMEMTRC
#define _H_GGAMEMTRC

#define GGADD_MEMTRACE_TITLE_SIZE	(16)
#define GGADD_MEMTRACE_SIZE		(1024)

typedef struct ggadd_trace_entry {

        ulong	id;
        ulong	parm1;
        ulong	parm2;
        ulong	parm3;

} ggadd_trace_entry_t;


typedef struct ggadd_trace_header {

	char			title[GGADD_MEMTRACE_TITLE_SIZE];

	ulong			devno, pd, ddf, dds;

	ulong			num_wraps;

	ggadd_trace_entry_t 	*top; 
	ggadd_trace_entry_t 	*next; 
	ggadd_trace_entry_t 	*last; 

	ulong			spare_1;
	ulong			spare_2;
	ulong			spare_3;
	ulong			spare_4;

} ggadd_trace_header_t;

#endif /* _H_GGAMEMTRC */

