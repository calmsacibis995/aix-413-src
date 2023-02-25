static char sccsid[] = "@(#)75	1.2  src/rspc/kernext/pcmcia/ssisa/pcm_io.c, pcmciaker, rspc41J, 9511A_all 3/13/95 20:58:19";
/*
 * COMPONENT_NAME: PCMCIAKER
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************
 *
 * NAME: pcmcia_ioreg
 *
 * FUNCTION: PCMCIA Socket Service subroutine
 *
 *     This routine reads from and writes to the adapter's I/O address.
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *     1        I/O error happened.
 *     2        An invalid operation code is specified.
 *
 ***************************************************************************/
#include "pcm_inc.h"
extern int pcmcia_intr_prior;

#ifdef DEBUG
extern int pcmcia_debug;
#endif

int
pcmcia_ioreg( struct adapter_def *ap, int op, struct pcmcia_ioreg *ioregp )
{
    int ret_code = 0;
	int old_intr_prior;
	struct io_map io_map;
	volatile uchar *ioaddr;

#ifdef DEBUG
if (pcmcia_debug){
	printf("Entering pcmcia_ioreg("
		   "io_addr = 0x%X, op = 0x%X, index = 0x%X, data = 0x%X )\n",
		   ap->ddi.io_addr, op, ioregp->index, ioregp->data );
}
#endif

	old_intr_prior = i_disable( pcmcia_intr_prior );

	io_map.key = IO_MEM_MAP;
	io_map.flags = 0;
	io_map.size = 4096; /* minimal size */
	io_map.bid = BID_VAL( BID_TYPE( ap->ddi.bus_id ),
			ISA_IOMEM, BID_NUM( ap->ddi.bus_id ) );
	io_map.busaddr = ap->ddi.io_addr ;

	ioaddr = iomem_att( &io_map );

    switch ( op )
	{
    case PCM_IOREAD:
        BUSIO_PUTC( ioaddr, ioregp->index );
        ioregp->data = (int) BUS_GETC( ioaddr + 1 );
        break;
    case PCM_IOWRITE:
        BUSIO_PUTC( ioaddr,     ioregp->index );
        BUSIO_PUTC( ioaddr + 1, ioregp->data  );
        break;
    default:
        ret_code = 2;
        break;
    } 
    iomem_det( (void *)ioaddr );

 end:
	i_enable( old_intr_prior );

#ifdef DEBUG
if (pcmcia_debug){
	printf("Exiting pcmcia_ioreg() = 0x%X: (index = 0x%x, data = 0x%x)\n",
		   ret_code, ioregp->index, ioregp->data );
}
#endif
    
    return ret_code;
}
