static char sccsid[] = "@(#)90	1.12  src/bos/usr/sbin/crash/file.c, cmdcrash, bos411, 9430C411a 7/22/94 15:44:12";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prfile
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
extern char *gnodeType();
extern char *socketType();

#include	"crash.h"
#include	"sys/file.h"
#include	"sys/inode.h"
#include	"sys/vnode.h"
#include 	<sys/socket.h>
#include 	<sys/socketvar.h>

prfile(c, all)
	register  int  c;
	int	all;
{
	struct	file	fileS;
	struct vnode  vnodeS;
	long          filePtr;

	if(c == -1)
		return;
	if(c >= (int)v.ve_file) {
		printf( catgets(scmc_catd, MS_crsh, FILE_MSG_1, "File table entry %4d above mmaximum limit.\n\tSpecify a value less than %d.\n") , c,v.ve_file);
		return;
	}

	if(readmem(&fileS, filePtr = (long)SYM_VALUE(File)+c*sizeof(fileS),
	           sizeof fileS, CURPROC) != sizeof fileS) {
		printf( catgets(scmc_catd, MS_crsh, FILE_MSG_2, "0452-061: Cannot read file table entry %4d.\n") , c);
		return;
	}

	if(!all && fileS.f_count == 0)
		return;

	if (fileS.f_vnode) {
		switch ( fileS.f_type )
		{
			case DTYPE_VNODE :
				if( readmem(&vnodeS, (unsigned)fileS.f_vnode, sizeof vnodeS,
				            CURPROC) != sizeof vnodeS)
				{
					printf( catgets(scmc_catd, MS_crsh, FILE_MSG_3,
							        "0452-062: Cannot read vnode associated with file table entry %4d.\n"),
					        c);
					return;
				}
			case DTYPE_GNODE :
			{
				struct gnode gnodeS ;
				ulong gnodePtr = (fileS.f_type == DTYPE_GNODE ?
				                  fileS.f_vnode : vnodeS.v_gnode);

				if ( ! gnodePtr || readmem(&gnodeS, gnodePtr, sizeof gnodeS,
				     CURPROC) != sizeof gnodeS )
				{
					printf( catgets(scmc_catd, MS_crsh, FILE_MSG_10,
					        "0452-063: Cannot read gnode associated with file table entry %4d.\n"),
					        c);
					return;
				}
				printf( "%4d   0x%8.8x   %3d   inode.%-10s   0x%8.8x  ",
				        c, filePtr, fileS.f_count, gnodeType( &gnodeS ),
				        (ulong) gnodeS.gn_data );
				break ;
			}
			case DTYPE_SOCKET :
			{
				struct socket socketS ;
				if( readmem(&socketS, (unsigned)fileS.f_vnode,
				            sizeof socketS, CURPROC) != sizeof socketS)
				{
					printf( catgets(scmc_catd, MS_crsh, FILE_MSG_3,
							        "0452-062: Cannot read vnode associated with file table entry %4d.\n"),
					        c);
					return;
				}
				printf( "%4d   0x%8.8x   %3d   socket.%-9s   0x%8.8x  ",
				        c, filePtr, fileS.f_count, socketType( &socketS ),
				        (ulong) fileS.f_vnode );
				break ;
			}
			default :
				printf( catgets(scmc_catd, MS_crsh, FILE_MSG_3,
						        "0452-062: Cannot read vnode associated with file table entry %4d.\n"),
				        c);
				return;
		}
	}
	else
		printf( "%4d   0x%8.8x   %3d       -                   -      ",
		        c, filePtr, fileS.f_count );

       printf(" 0x%8.8x", fileS.f_flag);
	if ( fileS.f_flag & FREAD )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_4, " read" ) );
	if ( fileS.f_flag & FWRITE )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_5, " write" ) ) ;
	if ( fileS.f_flag & FAPPEND )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_6, " append" ) ) ;
	if ( fileS.f_flag & FSYNC )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_7, " sync" ) ) ;
	if ( fileS.f_flag & FMPX )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_8, " mpx" ) ) ;
	if ( fileS.f_flag & FNDELAY )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_9, " ndelay") );
	if ( fileS.f_flag & FNONBLOCK )
		printf(  catgets(scmc_catd, MS_crsh, FILE_MSG_11, " nonblock") );
	printf("\n");
}
