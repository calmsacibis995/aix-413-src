static char sccsid[] = "@(#)10	1.1  src/rspc/usr/lib/boot/softros/lib/libc/string.c, rspc_softros, rspc411, GOLD410 4/18/94 15:47:39";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: memchr
 *		memcmp
 *		memcpy
 *		memmove
 *		memset
 *		strcat
 *		strchr
 *		strcmp
 *		strcoll
 *		strcpy
 *		strcspn
 *		strerror
 *		strlen
 *		strncat
 *		strncmp
 *		strncpy
 *		strpbrk
 *		strrchr
 *		strspn
 *		strstr
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

#include <string.h>
#include <stddef.h>
#include <errno.h>

/*---------------------------------------------------------------------------*/
/*  Memory Functions:                                                        */
/*---------------------------------------------------------------------------*/

void *memchr( const void *s, int c, size_t n )
    {
	register unsigned char	ch   = (unsigned char  )c;
	register unsigned char	*str = (unsigned char *)s;
	register int i;

	for ( i = 0; i < n; i++ )
	    {
	    if ( *str == ch )		/* match */
		return( str );		/* so return pointer */
	    else if ( *str == '\0' )	/* end of string */
		return( NULL );		/* so return NULL */
	    str++;			/* next character */
	    }
	return( NULL );			/* never found a match */
    }

int memcmp( const void *s1, const void *s2, size_t n )
    {
	register int i;
	register char *str1 = (char *)s1;
	register char *str2 = (char *)s2;

	for ( i = 0; i < n; i++, str1++, str2++ )
	    if ( *str1 != *str2 )	/* found difference */
		return(( int )( *str1 - *str2 ));
	return( 0 );			/* no difference up to n */
    }

void *memcpy( void *s1, const void *s2, size_t n )
    {
	return( memmove( s1, s2, n ));
    }

void *memmove( void *s1, const void *s2, size_t n )
    {
	register int i;
	register unsigned char *dest = (unsigned char *)s1;
	register unsigned char *src  = (unsigned char *)s2;

	/*
	 *  Does source overlap the beginning of the destination?
	 *  If so, copy backwards:
	 */
	if (( src < dest ) && (( src + n ) >= dest ))
	    {
	    dest = dest + n - 1;	/* end of destination string */
	    src  = src  + n - 1;	/* end of source string */
	    for ( i = 0; i < n; i++ )	/* copy from end */
		*dest-- = *src--;
	    }
	else				/* else, copy from beginning */
	    for ( i = 0; i < n; i++ )
	        *dest++ = *src++;	/* else, copy each char up to n */
	return( s1 );			/* return dest string */
    }

void *memset( void *s1, int c, size_t n )
    {
	register int i;
	register unsigned char *dest = (unsigned char *)s1;
	register unsigned char ch    = (unsigned char  )c;

	for ( i = 0; i < n; i++ )
	    *dest++ = ch;		/* fill array with character */

	return( s1 );
    }

/*---------------------------------------------------------------------------*/
/*  String Functions:                                                        */
/*---------------------------------------------------------------------------*/


char *strcat( char *s1, const char *s2 )
    {
	register char *dest = (char *)s1;
	register char *src  = (char *)s2;

	while ( *dest != '\0' )			/* look for dest null char */
	    dest++;
	while (( *dest++ = *src++ ) != '\0')	/* copy up to src null char */
	    ;
	return( s1 );
    }

char *strchr( const char *s, int c )
    {
        register char *str = (char *)s;

        do {
            if ( *str == (char)c )              /* check each character */
                return( str );                  /* for a match */
        } while ( *str++ != '\0' );             /* until end of string. */

        return( NULL );                         /* no match */
    }

int strcmp( const char *s1, const char *s2 )
    {
	register char *str1 = (char *)s1;
	register char *str2 = (char *)s2;

	while ( *str1 && ( *str1 == *str2 )) 	/* until different or end, */
	    str1++, str2++;			/* next char */
	return(( int )( *str1 - *str2 ));	/* value of difference */
    }

int strcoll( const char *s1, const char *s2 )
    {
	return( strcmp( s1, s2 ));		/* same as string compare */
    }

char *strcpy( char *s1, const char *s2 )
    {
	register char *dest = (char *)s1;
	register char *src  = (char *)s2;
						/* copy up to NULL */
	while (( *dest++ = *src++ ) != '\0' )
	    ;
	return( s1 );
    }

size_t strcspn( const char *s1, const char *s2 )
    {
	register char *str1 = (char *)s1;

	for ( ; *str1 != '\0'; str1++ )		/* for each char in str1 */
	    if ( strchr( s2, *str1 ))		/* found it in s2? */
		break;				/* quit */
	return( str1 - s1 );			/* part that didn't match */
    }

size_t strlen( const char *s )
    {
	register int i = 0;

	while ( s[i] != '\0' )			/* up to NULL */
	    ++i;
	return( i );
    }

char *strncat( char *s1, const char *s2, size_t n )
    {
	register char *dest = (char *)s1;
	register char *src  = (char *)s2;

	while ( *dest != '\0' )			/* look for dest null char */
	    dest++;
	while ( n-- && ( *src != '\0'))		/* copy up to n or end */
	    *dest++ = *src++;
	*dest = '\0';				/* the final touch */
	return( s1 );
    }

int strncmp( char *s1, const char *s2, size_t n )
    {
	register char *str1 = (char *)s1;
	register char *str2 = (char *)s2;

	for ( ; n--; str1++, str2++ )		/* decrement n */
	    if ( *str1 == *str2 )
		continue;			/* while matching */
	    else				/* doesn't match? */
		return( *str1 - *str2 );	/* return difference */
	return( 0 );				/* match up to n */
    }

char *strncpy( char *s1, const char *s2, size_t n )
    {
	register char *dest = (char *)s1;
	register char *src  = (char *)s2;
						/* copy up to NULL or n = 0 */
	while ( n-- && ( *dest++ = *src++ ) != '\0' )
	    ;
	return( s1 );
    }

char *strpbrk( const char *s1, const char *s2 )
    {
	register char *str1, *str2;
						/* for each char in str1, */
	for ( str1 = (char *)s1; *str1 != '\0'; str1++ )
						/* see if it exists in str2 */
	    for ( str2 = (char *)s2; *str2 != '\0'; str2++ )
		if ( *str2 == *str1 )		/* if so, */
		    return( str1 );		/* return pointer to match */
	return( NULL );				/* no match, return NULL */
    }

char *strrchr( const char *s, int c )
    {
	register char *str;
	register char *match = NULL;		/* assume no match */

	for ( str = (char *)s; *str != '\0'; str++ )
	    if ( *str == (char)c )		/* found a match? */
		match = str;			/* save latest pointer */
	return( match );			/* return result */
    }

size_t strspn( const char *s1, const char *s2 )
    {
	register char *str1, *str2;
	register int match;
						/* for each char in str1, */
	for ( str1 = (char *)s1; *str1 != '\0'; str1++ )
	    {					/* see if it exists in str2 */
	    match = FALSE;
	    for ( str2 = (char *)s2; *str2 != '\0'; str2++ )
		if ( *str1 == *str2 )
		    {
		    match = TRUE;
		    break;
		    }
	    if (! match )			/* if not, */
		return( str1 - s1 );		/* return length of portion */
	    }					/* that matched */
	return( str1 - s1 );			/* all match, return s1 len */
    }

char *strstr( const char *s1, const char *s2 )
    {
	register char *str;
	register int  len = strlen( s2 );
						/* for each possible substr, */
	for ( str = (char *)s1; *str != '\0'; str++ )
	    if ( !strncmp( str, s2, len ))	/* equal to second string? */
		return( str );			/* then return it */
	return( NULL );				/* never found a match */
    }

char *strerror( int errnum )
    {
	register char	*str;

	switch ( errnum )
	    {
	    case EPERM:   
		str = "operation not permitted"; 
		break;
	    case ENOENT:  
		str = "no such file or directory"; 
		break;
	    case ESRCH:   
		str = "no such process"; 
		break;
	    case EINTR:   
		str = "interrupted system call"; 
		break;
	    case EIO: 
		str = "I/O error"; 
		break;
	    case ENXIO: 
		str = "no such device or address"; 
		break;
	    case E2BIG: 
		str = "arg list too long"; 
		break;
	    case ENOEXEC: 
		str = "exec format error"; 
		break;
	    case EBADF: 
		str = "bad file descriptor"; 
		break;
	    case ECHILD: 
		str = "no child processes"; 
		break;
	    case EAGAIN: 
		str = "resource temporarily unavailable"; 
		break;
	    case ENOMEM: 
		str = "not enough space"; 
		break;
	    case EACCES: 
		str = "permission denied"; 
		break;
	    case EFAULT: 
		str = "bad address"; 
		break;
	    case ENOTBLK: 
		str = "block device required"; 
		break;
	    case EBUSY: 
		str = "resource busy"; 
		break;
	    case EEXIST: 
		str = "file exists"; 
		break;
	    case EXDEV: 
		str = "improper link"; 
		break;
	    case ENODEV: 
		str = "no such device"; 
		break;
	    case ENOTDIR: 
		str = "not a directory"; 
		break;
	    case EISDIR: 
		str = "is a directory"; 
		break;
	    case EINVAL: 
		str = "invalid argument"; 
		break;
	    case ENFILE: 
		str = "too many open files in system"; 
		break;
	    case EMFILE: 
		str = "too many open files"; 
		break;
	    case ENOTTY: 
		str = "inappropriate I/O control operation"; 
		break;
	    case ETXTBSY: 
		str = "text file busy"; 
		break;
	    case EFBIG: 
		str = "file too large"; 
		break;
	    case ENOSPC: 
		str = "no space left on device"; 
		break;
	    case ESPIPE: 
		str = "invalid seek"; 
		break;
	    case EROFS: 
		str = "read only file system"; 
		break;
	    case EMLINK: 
		str = "too many links"; 
		break;
	    case EPIPE: 
		str = "broken pipe"; 
		break;
	    case EDOM: 
		str = "domain error within math function"; 
		break;
	    case ERANGE: 
		str = "result too large"; 
		break;
	    case ENOMSG: 
		str = "no message of desired type"; 
		break;
	    case ECHRNG: 
		str = "channel number out of range"; 
		break;
	    case EDEADLK: 
		str = "resource deadlock avoided"; 
		break;
	    case ENOTREADY: 
		str = "device not ready"; 
		break;
	    case EWRPROTECT: 
		str = "write-protected media"; 
		break;
	    case EFORMAT: 
		str = "unformatted media"; 
		break;
	    case ENOLCK: 
		str = "no locks available"; 
		break;
	    case ENOCONNECT: 
		str = "no connection"; 
		break;
	    case ENAMETOOLONG: 
		str = "file name too long"; 
		break;
	    case ENOTEMPTY: 
		str = "directory not empty"; 
		break;
	    case ENOSYS: 
		str = "function not implemented POSIX"; 
		break;
	    default:
		str = "unknown error";
		break;
	}
	return( str );
    }

