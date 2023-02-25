/* @(#)30	1.1  src/rspc/usr/lib/boot/softros/roslib/errno.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:13  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* @(#)75 1.1 rom/errno.h, libipl, sf100 5/13/93 14:09:06 */

/*---------------------------------------------------------------------------
 *
 *
 * 75
 *
 * Maintenance history:
 *
 *	24-Feb-93  M. McLemore		Original version, derived from AIX
 *                                      3.2 errno.h header.
 */

#ifndef _ERRNO
#define _ERRNO

extern int errno;

#define	EPERM		1	/* Operation not permitted		*/
#define	ENOENT		2	/* No such file or directory		*/
#define	ESRCH		3	/* No such process			*/
#define	EINTR		4	/* interrupted system call		*/
#define	EIO		5	/* I/O error				*/
#define	ENXIO		6	/* No such device or address		*/
#define	E2BIG		7	/* Arg list too long			*/
#define	ENOEXEC		8	/* Exec format error			*/
#define	EBADF		9	/* Bad file descriptor			*/
#define	ECHILD		10	/* No child processes			*/
#define	EAGAIN		11	/* Resource temporarily unavailable	*/
#define	ENOMEM		12	/* Not enough space			*/
#define	EACCES		13	/* Permission denied			*/
#define	EFAULT		14	/* Bad address				*/
#define	ENOTBLK		15	/* Block device required		*/
#define	EBUSY		16	/* Resource busy			*/
#define	EEXIST		17	/* File exists				*/
#define	EXDEV		18	/* Improper link			*/
#define	ENODEV		19	/* No such device			*/
#define	ENOTDIR		20	/* Not a directory			*/
#define	EISDIR		21	/* Is a directory			*/
#define	EINVAL		22	/* Invalid argument			*/
#define	ENFILE		23	/* Too many open files in system	*/
#define	EMFILE		24	/* Too many open files			*/
#define	ENOTTY		25	/* Inappropriate I/O control operation	*/
#define	ETXTBSY		26	/* Text file busy			*/
#define	EFBIG		27	/* File too large			*/
#define	ENOSPC		28	/* No space left on device		*/
#define	ESPIPE		29	/* Invalid seek				*/
#define	EROFS		30	/* Read only file system		*/
#define	EMLINK		31	/* Too many links			*/
#define	EPIPE		32	/* Broken pipe				*/
#define	EDOM		33	/* Domain error within math function	*/
#define	ERANGE		34	/* Result too large			*/
#define	ENOMSG		35	/* No message of desired type		*/
#define	ECHRNG		37	/* Channel number out of range		*/
#define EDEADLK 	45	/* Resource deadlock avoided		*/
#define ENOTREADY	46	/* Device not ready			*/
#define EWRPROTECT	47	/* Write-protected media 		*/
#define EFORMAT		48	/* Unformatted media 			*/
#define ENOLCK		49	/* No locks available 			*/
#define ENOCONNECT      50      /* no connection                	*/
#define	ENAMETOOLONG	86	/* File name too long		  	*/
#define ENOTEMPTY	87	/* Directory not empty 			*/
#define ENOSYS		109	/* Function not implemented  POSIX 	*/

#endif			/* _ERRNO */
