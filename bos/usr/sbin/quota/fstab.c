static char sccsid[] = "@(#)21	1.4  src/bos/usr/sbin/quota/fstab.c, cmdquota, bos411, 9428A410j 3/7/91 18:29:26";
/*
 * COMPONENT_NAME: CMDQUOTA 
 *
 * FUNCTIONS: endfsent, getfsent, setfsent
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "fstab.h"
#include <ctype.h>
#include <string.h>
#include <jfs/quota.h>

#define	FSFIELDS 6	/* number of fields returned by fstabscan() */

static	struct fstab fs;       /* fstab structure returned           */
static	AFILE_t fs_file = 0;   /* attribute file information         */
static  int pass_no;           /* the pass number for the current fs */


/*
 * NAME: fstabscan
 *
 * FUNCTION: returns filesytesm stanzas from /etc/filesystems 
 *
 * RETURN VALUE DESCRIPTION:
 *     returns 0 if error otherwise 1.
 */
int
fstabscan(struct fstab *fs)  /* fs is the fstab structure filled by fstabscan */
{
	register ATTR_t cp;          /* attribute pair structure */
        register char 	*m		= (char *)NULL, 
			*userquota	= (char *)NULL,
			*groupquota	= (char *)NULL; 
	char 	usrname[100],
		grpname[100];
	int readonly = 0,
	    uquota   = 0,
	    gquota   = 0; 

	cp = AFnxtrec(fs_file);
	if (cp == NULL) return (EOF);

	fs->fs_file = cp->AT_value;	        /* stanza name */
	fs->fs_spec = AFgetatr(cp,"dev");       /* device type */
	fs->fs_vfstype = AFgetatr(cp,"vfs");    /* vfs type    */

        sprintf(usrname, "%s%s", qfextension[USRQUOTA], qfname);
        sprintf(grpname, "%s%s", qfextension[GRPQUOTA], qfname);

        m = AFgetatr(cp, "quota");

        if (m == NULL || !strcmp(m, "false"))
	{
	   m =  AFgetatr(cp,"mount"); 
	   while (*m)
	   {
		if (strcmp(m,"readonly")==0)
			readonly++;
		m += strlen(m) + 1;
	   }
	   if (readonly > 0)
		fs->fs_type = FSTAB_RO;
	   else
		fs->fs_type = FSTAB_RW;
        }
	else
	{
	   fs->fs_type = FSTAB_RQ;
    	   while (*m) 
 	   {
		if(!strcmp(m, usrname))
 		{
		   	uquota++;
		   	userquota = AFgetatr(cp, usrname);
	 	}
		else if (!strcmp(m, grpname))
		{
		   	gquota++;
			groupquota = AFgetatr(cp, grpname);
		}
            	m += strlen(m) + 1;
	   }
	}

        m = AFgetatr(cp, "options");

	fs->fs_mntops = (char *) malloc(strlen(m)      + 1 + /* comma */
			(userquota  ? (strlen(usrname) + 1 + /* equal */
 			               strlen(userquota))  
               			    :  strlen(usrname))
			        	  	       + 1 + /* comma */
			(groupquota ? (strlen(grpname) + 1 + /* equal */
				       strlen(groupquota))
       	                   	    :  strlen(grpname))
						       + 1 * sizeof(char));
	if (fs->fs_mntops == (char *) NULL)
	{
		perror("malloc");
		return(-1);
	}
		

	sprintf(fs->fs_mntops, "%s%s%s%s%s%s%s%s%s",
		m,
		(*m && uquota)        	 ? ","	      : (char *) NULL,
		uquota                	 ? usrname    : (char *) NULL,
		(uquota && userquota)  	 ? "="        : (char *) NULL,
	   	(uquota && userquota)  	 ? userquota  : (char *) NULL,	
		(*m || uquota) && gquota ? ","        : (char *) NULL,
		gquota     	      	 ? grpname    : (char *) NULL,
 		(gquota && groupquota) 	 ? "="        : (char *) NULL,
		(gquota && groupquota)   ? groupquota : (char *) NULL);

	m =  AFgetatr(cp, "check"); 
	if (m == NULL || !strcmp(m, "false"))
		fs->fs_check = -1;
	else if (!strcmp(m, "true"))
		fs->fs_check = 0;
	else
		fs->fs_check = atoi(m);

	fs->fs_freq = 0;
	fs->fs_passno = ++pass_no;
	readonly = 0;
	return (FSFIELDS);
}
	

/*
 * NAME: setfsent
 *
 * FUNCTION: opens and rewinds the file system file.
 *
 * RETURN VALUE DESCRIPTION: 
 *     returns 0 if error otherwise 1.
 */
int
setfsent()          
{
        pass_no = 0;
	if (fs_file) endfsent();

	if ((fs_file = AFopen(FSYSname,MAXREC,MAXATR)) == NULL) {
		fs_file = 0;
		return (0);
	}
	return (1);
}

/*
 * NAME: endfsent
 *
 * FUNCTION: closes the file system file.
 *
 * RETURN VALUE DESCRIPTION: 
 *	returns 0 if error otherwise 1
 */
int 
endfsent()           
{
	if (fs_file) {
		AFclose(fs_file);
		fs_file = 0;
	}
	return (1);
}

/*
 * NAME: getfsent
 *
 * FUNCTION: reads the next record of the file opening if necessary.
 *
 * RETURN VALUE DESCRIPTION: 
 *    returns a pointer to an object with fstab structure
 *    a null pointer is return if EOF or error
 */
struct fstab *
getfsent()   
{
	int nfields;    /* number of fields filled by fstabscan */

	if ((fs_file == 0) && (setfsent() == 0))
		return ((struct fstab *)0);
	nfields = fstabscan(&fs);
	if (nfields == EOF || nfields != FSFIELDS)
		return ((struct fstab *)0);
	return (&fs);
}
