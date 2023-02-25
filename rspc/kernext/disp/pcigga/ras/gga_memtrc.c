static char sccsid[] = "@(#)30	1.1  src/rspc/kernext/disp/pcigga/ras/gga_memtrc.c, pcigga, rspc41B, 9504A 1/9/95 13:43:33";

/*
 * COMPONENT_NAME: (pcigga) Glacier Device Driver 
 *
 * FUNCTIONS: 
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

#include "gga_INCLUDES.h" 

long gga_memtrace_init (pd)
	struct	phys_displays	*pd;
{

	struct gga_ddf	*ddf;
	ggadd_trace_entry_t 	*trace_buffer;		/* ptr to trace buf */

	char trace_name[GGADD_MEMTRACE_TITLE_SIZE] = " rdh_ecart_ddagg" ;  

	int  	i ; 
 
	ddf = (struct gga_ddf *) pd->free_area;


  	/***************************************************************** 
 	    First check if a buffer has already been allocated.   
   	*****************************************************************/

	BUGLPR (dbg_memtrc, 0, ("gga_memtrc_init()\n") );
  	if((ddf->memtrace.top) != NULL){ 
		/* There seems to a buffer already allocated  */
		/* so don't return an error. Should never see */
		/* this unless a code change in bbl_init()    */
		/* causes this to be initialized out of order */
  		return (0);
  	}

  	/***************************************************************** 
 	    Next allocate space for the internal trace buffer.   
   	*****************************************************************/

  	trace_buffer = (ggadd_trace_entry_t *) 
			xmalloc ((GGADD_MEMTRACE_SIZE *
			sizeof(ggadd_trace_entry_t)), 4,
			pinned_heap);

  	if(trace_buffer == NULL )
	{
        	return (-1);
  	}

	/* Zero out the trace buffer */
	bzero(trace_buffer,
		(GGADD_MEMTRACE_SIZE * sizeof(ggadd_trace_entry_t)));

  	/***************************************************************** 
 	    Now set up the trace header (in ddf). 
   	*****************************************************************/

    	for (i=0; i<GGADD_MEMTRACE_TITLE_SIZE; i++){
		ddf->memtrace.title[i] =
			trace_name[GGADD_MEMTRACE_TITLE_SIZE - 1 - i]; 
	}

	ddf->memtrace.devno = (ulong) pd->devno;
	ddf->memtrace.pd = (ulong) pd;
	ddf->memtrace.ddf = (ulong) ddf;
	ddf->memtrace.dds = (ulong) pd->odmdds;

	ddf->memtrace.num_wraps = 0;

    	ddf->memtrace.top  = trace_buffer; 
    	ddf->memtrace.next = trace_buffer; 
    	ddf->memtrace.last = trace_buffer + (GGADD_MEMTRACE_SIZE - 1) ;

	ddf->memtrace.spare_1 = 0;
	ddf->memtrace.spare_2 = 0;
	ddf->memtrace.spare_3 = 0;
	ddf->memtrace.spare_4 = 0;

	return(0);

}


long gga_memtrace_term (ddf)
	struct gga_ddf	*ddf;
{
  	/***************************************************************** 
 	    First check if a buffer has not been allocated.   
   	*****************************************************************/

  	if((ddf->memtrace.top) == NULL){ 
  		return (-1);
  	}

  	/***************************************************************** 
 	    Next free the space for the internal trace buffer.   
   	*****************************************************************/

	xmfree (ddf->memtrace.top, pinned_heap);

	return(0);
}


long gga_memtrace (ddf, hook, parm1, parm2, parm3)
	struct gga_ddf  * ddf;
	ulong		hook;
	ulong		parm1;
	ulong		parm2;
	ulong		parm3;
{
	ggadd_trace_entry_t 	*trace_buffer;		/* ptr to trace buf */
 
  	/***************************************************************** 
           Make the trace entry 
   	*****************************************************************/

	trace_buffer = ddf->memtrace.next; 

	trace_buffer->id     = hook; 
	trace_buffer->parm1  = parm1; 
	trace_buffer->parm2  = parm2; 
	trace_buffer->parm3  = parm3; 


  	/***************************************************************** 
           Increment the trace entry
   	*****************************************************************/

	trace_buffer++;
	if(trace_buffer < ddf->memtrace.last){
		ddf->memtrace.next = trace_buffer; 
	}else{ /* wrap */ 
		ddf->memtrace.next = ddf->memtrace.top ;
		ddf->memtrace.num_wraps++;
	}
}

