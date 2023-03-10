/* @(#)49	1.3  src/tcpip/usr/lib/libisode/pepsy-driver.h, isodelib7, tcpip411, GOLD410 5/26/93 15:07:29 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: CLR_OPT_PRESENT FDFLT_B FDFLT_F OPT_PRESENT SET_OPT_PRESENT
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pepsy-driver.h
 */

#ifndef _pepsy_driver_
#define _pepsy_driver_

/*
 * common definitions for the pepsy driver routines
 */
#define PEPSY_VERSION	2

/* find the default entry for this entry - for decoding
 * - assumes its the next or the one after or the one after that.
 * old version
#define FDFLT_B(p)	((((p) + 1)->pe_type == DFLT_B) ? ((p) + 1) : \
			((((p) + 2)->pe_type == DFLT_B) ? ((p) + 2): ((p) + 3)))
 */
#define FDFLT_B(p)	fdflt_b(p)	/* try a function */

/* find the entry for this default entry - for encoding
 * call a function - to keep looking till it finds it, this is the
 * alternative to the above scheme.
 */
#define FDFLT_F(p)	fdflt_f(p)	/* have to use a function */

/* assumes encoding tables */
#define OPT_PRESENT(p, parm)	(BITTEST(parm + p->pe_ucode, p->pe_tag))

/* assumes decoding tables */
#define SET_OPT_PRESENT(p, parm)	(BITSET(*parm + p->pe_ucode, p->pe_tag))
#define CLR_OPT_PRESENT(p, parm)	(BITCLR(*parm + p->pe_ucode, p->pe_tag))

#define NO_DATA_USED	(OK + 1)

#endif /* _pepsy_driver_ */
