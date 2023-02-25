
/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)98	1.5  src/tenplus/lib/obj/s_getline.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:39";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_getline
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*****************************************************************************
* s_getline:  reads the next line into a buffer from the specified file      *
*                                                                            *
* Characters are read from the specified file until end-of-line is reached.  *
* The end of a line is reached when a newline is read or EOF is reached.     *
* The newline character is not put in the buffer.  A pointer to the          *
* allocated buffer containing the line read is returned.  A NULL pointer     *
* is returned if EOF reached and the line length is zero.                    *
*****************************************************************************/
#include <stdio.h>
#include <database.h>

#include <libobj.h>

#define AMOUNT (100 * MB_LEN_MAX)

char *s_getline(FILE *fp)
{
    register index = 0;
    register c;
    char *buf;


    buf = s_alloc(AMOUNT, T_CHAR, NULL);

    for(;;) {
        c = fgetc(fp);
        if (c == EOF || c == '\n')
            break;

        if (index >= obj_count(buf))
            buf = s_realloc(buf, obj_count(buf) + AMOUNT);

        buf[index] = c;
        ++index;
    }

    if (c == EOF && index == 0) {
        s_free(buf);
        return(NULL);
    }

    buf = s_realloc(buf, index);
    return(buf);
}
