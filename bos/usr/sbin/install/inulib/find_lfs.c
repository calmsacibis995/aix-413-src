static char sccsid[] = "@(#)58  1.7  src/bos/usr/sbin/install/inulib/find_lfs.c, cmdinstl, bos411, 9428A410j 6/17/93 16:12:56";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: find_lfs
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: find_lfs
 *
 * FUNCTION:
 *	Find the (logical) filesystem on the target machine where the
 *	file or directory pointed to by dir will reside (but it may
 *	not currently exist).
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 *	Uses global variables fs[] , fs_vfsnum[] and nfs .
 *
 * ON ENTRY:
 *	dir	pointer to string containing file to locate
 *
 * RETURNS:
 *	index into fs[] indicating which filesystem the
 *	file dir will be in.
 */

#include <inu_eval_size.h>
#include <inulib.h>
#include <sys/param.h>

/*
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inudef.h>
*/

static int get_vfsnumber();
extern ulong_t *fs_vfsnum;

int find_lfs(char *dir)			/* name to locate		*/
{
	register int	i;		/* generic counters		*/
	ulong_t vfs_num;

	/* find the fs that will contain this dir */

	/*  
	 * The special directories to indicate size requirements:	
	 * fs[0] - PAGESPACE
	 * fs[1] - INSTWORK
	 * fs[2] - /usr/lpp/SAVESPACE
	 * fs[3] - /lpp/SAVESPACE
	 * fs[4] - /usr/share/lpp/SAVESPACE
	 * may be matched by name ( strcmp will do the job )
	*/
	for ( i=0; i < FIRST_REAL_FS ; i++ )
		if ( strcmp ( dir, fs[i] ) == 0 ) return(i);
	 
	/* compare the file system vfs numbers */

	if ( get_vfsnumber ( dir , &vfs_num ) == 0)
		for (i = 3; i <= nfs ; i++) 
			if ( vfs_num == fs_vfsnum[i] )
				return(i);
	return(-1);
}

/*
 * Scans dir name from right to left to decide if any subdir is
 * part a file system and returns that  vfs number ( unique ) in vfsn .
 * (stat call takes care of symbolic names or mount points )
 * At least it should match "/".
 */
static int
get_vfsnumber( dir , vfsn)
char *dir;
ulong_t *vfsn;
{
	int rc = -1, startf = 0;
	register char *cp;
	struct stat sbuf;
	char tmp_dir[MAXPATHLEN];
	extern char *strrchr();

	if ( dir == NULL )
		return(rc);
	strcpy ( tmp_dir, dir );
	cp = &tmp_dir[0];

	while ( cp != NULL ) {
		/*
		 * stat will fail if 'tmp_dir' file does not exist
		 */
		if ( stat( tmp_dir , &sbuf ) == 0 ){ 
			*vfsn = sbuf.st_vfs;	 /* vfs number */	
			rc = 0;
			break;
		}
		/* 
		 * if we already checked the root, break
		 */
		if ( startf )
			break;
		if ( (cp = strrchr ( tmp_dir, '/' )) != NULL ){
			/*
			 * if we are at the start we need to
			 * get "/" ( root dir )
			 */
			if ( cp == tmp_dir ){ 
				cp++;
				startf = 1;
			}
			*cp = '\0';		/* break */
		}
	} 
	return( rc );
}
