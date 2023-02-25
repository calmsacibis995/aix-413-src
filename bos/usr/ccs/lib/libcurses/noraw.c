#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)12  1.10  src/bos/usr/ccs/lib/libcurses/noraw.c, libcurses, bos411, 9428A410j 9/3/93 14:46:53";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: noraw
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/noraw.c	1.9"		*/



#include	"curses_inc.h"

noraw()
{
#ifdef	SYSV
    /* Enable interrupt characters */
    PROGTTY.c_lflag |= (ISIG|ICANON);
    PROGTTY.c_cc[VEOF] = _CTRL('D');
    PROGTTY.c_cc[VEOL] = 0;
    PROGTTY.c_iflag |= IXON;
#else	/* SYSV */
    PROGTTY.sg_flags &= ~(RAW|CBREAK);
#endif	/* SYSV */

#ifdef	DEBUG
#ifdef	SYSV
    if (outf)
	fprintf(outf, "noraw(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.c_lflag);
#else	/* SYSV */
    if (outf)
	fprintf(outf, "noraw(), file %x, flags %x\n",
	    cur_term->Filedes, PROGTTY.sg_flags);
#endif	/* SYSV */
#endif	/* DEBUG */

    cur_term->_fl_rawmode = FALSE;
    cur_term->_delay = -1;
    reset_prog_mode();
#ifdef	FIONREAD
    cur_term->timeout = 0;
#endif	/* FIONREAD */
    return (OK);
}
