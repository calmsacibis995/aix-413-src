static char sccsid[] = "@(#)47  1.8  src/rspc/kernext/isa/ppa/POWER/pppio_bot.c, sysxprnt, rspc41J, 9510A 3/6/95 23:23:50";
/*
 *
 *   COMPONENT_NAME: (SYSXPRNT) Parallel Port Printer Device Driver
 *
 *   FUNCTIONS: ppsendchar
 *              ppreadchar (moved from ppddm_bot.c)
 *              ppwritechar
 *              ppoutchar
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/ppdd.h>
#include <sys/ddtrace.h>
#include <errno.h>
#include <sys/lpio.h>
#include <sys/trchkid.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>

#ifdef	PM_PHASE2
#include <sys/pmdev.h>
#endif	/* PM_PHASE2 */

/* Initial Value for Control Register */


/*
 * NAME: ppsendchar
 *
 * FUNCTION: send a character to the printer
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (NOTES:)
 *      To send a character to the printer first setup the loc struct
 *      then call PIO assist passing it the loc struct and the address of
 *      ppsendch and the recovery half ppsendchrc.
 *
 * RETURNS:
 *          - PPBUSY:     printer is busy
 *          - PPDONE:     if the out queue has been exhausted
 *          - PPCONTINUE: if a character is sent to the printer successfully
 *          - ENXIO:      if a failure occurs
 *
 */

int ppsendchar(
struct pp *pp)
{
	int rc ;
	struct ppiost lc ;               /* structure for read char   */

#ifdef	PM_PHASE2
	if (pp->pmh != NULL)
		pp->pmh->activity = PM_ACTIVITY_OCCURRED;
#endif	/* PM_PHASE2 */

	lc.pp = pp ;
#ifdef _PP_ISA_BUS
	lc.busacc = (int)iomem_att(&lc.pp->iomap);
#else
	lc.busacc = (int)BUSIO_ATT(lc.pp->bus_id, 0) ;
#endif	/* _PP_ISA_BUS */

	rc = ppoutchar(pp, &lc);

#ifdef _PP_ISA_BUS
	iomem_det(lc.busacc);
#else
	BUSIO_DET(lc.busacc) ;
#endif	/* _PP_ISA_BUS */

	return( rc );
}


/*
 * NAME: ppreadchar
 *
 * FUNCTION:
 *         This will read one character from the printer bus and save
 *         it in the structure passed to it(ppiost).
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (RECOVERY OPERATION:) If an error occurs during this operation then
 *      the ppreadcharrc function is call by PIO recovery to complete the
 *      operation.
 *
 * RETURNS: data read from the requested port. This is passed in data of
 */

int ppreadchar(
	struct ppiost *loc )              /* address of io structure    */
{
	int retcd ;
	struct pp * pp = loc->pp ;

#ifdef	PM_PHASE2
	if (pp->pmh != NULL)
		pp->pmh->activity = PM_ACTIVITY_OCCURRED;
#endif	/* PM_PHASE2 */

#ifdef _PP_ISA_BUS
	loc->busacc = (int)iomem_att(&pp->iomap);
#else
	loc->busacc = (int)BUSIO_ATT(pp->bus_id, 0) ;
#endif	/* _PP_ISA_BUS */

	retcd = PP_PIO_GETC ;

#ifdef _PP_ISA_BUS
	iomem_det(loc->busacc);
#else
	BUSIO_DET(loc->busacc) ;
#endif	/* _PP_ISA_BUS */

	return(retcd) ;
}


/*
 * NAME: ppwritechar
 *
 * FUNCTION:
 *         This will write one character to the printer bus from the struct
 *         passed to it.
 *
 * EXECUTION ENVIRONMENT:
 *      This function will execute in both levels and can not page fault
 *
 * (RECOVERY OPERATION:) If an error occurs during this operation then
 *      the ppwritecharrc function is call by PIO recovery to complete the
 *      operation.
 *
 * RETURNS:
 *       -1     if an perminet pio error is detected
 *       0      if data writen
 */

int ppwritechar(
struct ppiost *loc )                /* address of io structure */
{
	int rc ;

#ifdef	PM_PHASE2
	struct pp * pp = loc->pp ;

	if (pp->pmh != NULL)
		pp->pmh->activity = PM_ACTIVITY_OCCURRED;
#endif	/* PM_PHASE2 */

#ifdef _PP_ISA_BUS
	loc->busacc = (int)iomem_att(&loc->pp->iomap);
#else
	loc->busacc = (int)BUSIO_ATT(loc->pp->bus_id, 0) ;
#endif	/* _PP_ISA_BUS */

	rc = PP_PIO_PUTC ;

#ifdef _PP_ISA_BUS
	iomem_det(loc->busacc);
#else
	BUSIO_DET(loc->busacc) ;
#endif	/* _PP_ISA_BUS */
	if (rc)
		return ( -1) ;
	return(0) ;
}


int ppoutchar(
struct pp *pp,
struct ppiost *loc)
{
	int stat;

	/* if no data to send return with PPDONE   */
	if ( pp->ppmodes & PLOT ) {
		if ( pp->buf_index >= pp->buf_count ) {
			return(PPDONE);
		}
	} else {
		if( pp->prt.outq.c_cc == 0 ) {
			return(PPDONE);
		}
	}

	/* read the status register */
	loc->address = pp->statreg ;
	stat = PP_PIO_GETC ;
	if (stat == -1)
		return(ENXIO) ;
	if( !(ppready((char)stat, pp)) ) {
		if( PP_FATAL_TST(stat) ) {
			/* Error is set and ONLY Error is set   **
			** FF Needed flag       */

			pp->flags.ppff_needed = 1 ;
		}
		return(PPBUSY);
	}
	if ( pp->ppmodes & PLOT ) {
		loc->data = pp->buffer[pp->buf_index++] ;
	} else {
		loc->data = (uchar) getc(&pp->prt.outq) ;
	}

	/* load data reg */
	loc->address = pp->datareg ;
	if ( PP_PIO_PUTC )
		return(ENXIO);
	loc->address = pp->ctrlreg ;
	loc->data = PP_STROBE | PP_INIT ;	/* Strobe */
	if ( PP_PIO_PUTC )
		return(ENXIO);
	loc->data = PP_INIT ;
	if ( PP_PIO_PUTC )
		return(ENXIO);
	pp->flags.ppcharsent = 1;
	return(PPCONTINUE);
}
