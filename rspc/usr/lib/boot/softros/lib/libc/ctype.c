static char sccsid[] = "@(#)07	1.1  src/rspc/usr/lib/boot/softros/lib/libc/ctype.c, rspc_softros, rspc411, GOLD410 4/18/94 15:47:29";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: isalnum
 *		isalpha
 *		iscntrl
 *		isdigit
 *		isgraph
 *		islower
 *		isprint
 *		ispunct
 *		isspace
 *		isupper
 *		isxdigit
 *		tolower
 *		toupper
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>

int isalnum( int c )
    {
	return( isalpha( c ) || isdigit( c ));
    }

int isalpha( int c )
    {
	return( isupper( c ) || islower( c ));
    }

int iscntrl( int c )
    {
 	return((( c >= 0 ) && ( c <= 0x1F )) || ( c == 0x7F ));
    }

int isdigit( int c )
    {
	return(( c >= '0' ) && ( c <= '9' ));
    }

int isgraph( int c )
    {
	return(( c >= 0x21 ) && ( c <= 0x7E ));
    }

int islower( int c )
    {
	return(( c >= 'a' ) && ( c <= 'z' ));
    }

int isprint( int c )
    {
	return(( c >= 0x20 ) && ( c <= 0x7E ));
    }
	
int ispunct( int c )
    {
	return( isprint( c ) && ( c != ' ' ) && !isalnum( c ));
    }

int isspace( int c )
    {
	return(( c == ' '  ) || ( c == '\f' ) || ( c == '\n' ) ||
	       ( c == '\r' ) || ( c == '\t' ) || ( c == '\v' ));
    }

int isupper( int c )
    {
	return(( c >= 'A' ) && ( c <= 'Z' ));
    }

int isxdigit( int c )
    {
	return((( c >= 'A' ) && ( c <= 'F' )) ||
	       (( c >= 'a' ) && ( c <= 'f' )) ||
	       (( c >= '0' ) && ( c <= '9' )));
    }

int tolower( int c )
    {
	if (!isupper( c ))		/* if can't convert, return char */
	    return( c );
	else
	    return( c + 'a' - 'A' );
    }

int toupper( int c )
    {
	if (!islower( c ))		/* if can't convert, return char */
	    return( c );
	else
	    return( c - ( 'a' - 'A' ));
    }

