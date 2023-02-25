static char sccsid[] = "@(#)14	1.8  src/bos/usr/sbin/lsfs/jfs.c, cmdfs, bos411, 9428A410j 1/6/94 15:58:39";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: query_jfs, print_jfs_query, cr_jfs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "lsfs.h"


void		query_jfs (char*, int, int);
int		print_jfs_query (char*, int);
void		cr_jfs (char**,	char*, 	mode_t,	struct jfs_attrs*);
static int	print_jfs_query (char*, int);


/*
 * Define the order in which data is expected
 * when performing a query ala lsfs -q
 */
#define LV_SIZE		0
#define FS_SIZE		1
#define FRAG_SIZE	2
#define NBPI_SIZE	3
#define COMP_ALGOR	4
#define NUM_QUERY	5
#define QUERY_LEN	1024
#define	TEMP_LEN	128


/*
 *	query_jfs
 *
 *	Call the jfs query interface which is, for historical reasons,
 *	embeded in extendfs.
 *
 */

void
query_jfs (char *dev,
	   int query,
	   int	colon)
{
	char 	fs_data [QUERY_LEN];
	char	*q;
	int	len;

	if (extendfs ("jfs", dev, NILPTR(char), fs_data, query) == 0)
	{
		/*
		 * If the query worked then the query info is
		 * suspose to be the second string in the buffer.
		 * The first is the return code of the helper and
		 * is no longer of interest. The second string is
		 * passed to the print function.
		 */
		if ((len = strlen (fs_data)) > 0 && len < QUERY_LEN)
		{
			q = fs_data + len + 1;
			print_jfs_query (q, colon);
		}
	}
}

/*
 *	print_jfs_query
 *
 *	Print the query results. 
 *
 *
 */

static int
print_jfs_query (char *data,
		 int colon)
{
	char *tok;
	char tmp[TEMP_LEN];
	char out[QUERY_LEN];
	char *inter_fs 	= colon ? ":" : ", ";	/* Inter field seperator */
	char *intra_fs	= colon ? " " : ": ";	/* Intra field seperator */
	int i;

	/*
	 * Convention here is that op_extend is expected to write
	 * the query data to the buffer as a string in the following
	 * format and order:
	 * 	"lv_size,fs_size,frag_size,nbpi,compress"
	 *
	 * strtok should find only NUM_QUERY comma
	 * seperated tokens.
	 *
	 * Changing the format of this output will require adjustments
	 * to lsjfs.sh - a smit helper.
	 *
	 * Allow strtok to mangle data:
	 */

	if (*data != '\0')
	{
		*out = '\0';
		sprintf (out, "  (");

		for (i = LV_SIZE, tok = strtok (data, ","); tok;
		     tok = strtok (NULL, ","), i++)
		{
			switch (i)
			{
				case LV_SIZE:
				sprintf (tmp, MSGSTR(LVSIZE, "lv size%s%s%s"),
					 intra_fs, tok, inter_fs);
				break;
			
				case FS_SIZE:
				sprintf (tmp, MSGSTR(FSSIZE,"fs size%s%s%s"),
					 intra_fs, tok, inter_fs);
				break;

				case FRAG_SIZE:
				sprintf (tmp, MSGSTR(FGSIZE,"frag size%s%s%s"),
					 intra_fs, tok, inter_fs);
				break;

				case NBPI_SIZE:
				sprintf (tmp, MSGSTR(NBPISIZE,"nbpi%s%s%s"),
					 intra_fs, tok, inter_fs);
				break;

				case COMP_ALGOR:
				sprintf (tmp, MSGSTR(COMPALGOR,"compress%s%s"),
					 intra_fs, tok);
				break;

				default:
				*out = '\0';
				return (1);
			}
			strcat (out, tmp);
		}
		if (*out != '\0')
		{
			strcat (out, ")");
			printf ("%s\n", out);
		}
	}
	return (0);
}

/*
 *	cr_jfs
 *
 *	Do the high level jfs/lvm specific processing and call mkfs
 *	to make the filesystem.
 *	
 *	return the size of the filsystems
 */

void
cr_jfs (char  **dname,
	char  *mntpt,
	mode_t  devtype,
	struct jfs_attrs  *jfs_info)
{
	long  	vgpart;
	int	rc;
	int	log_made = 0;
	int	lv_made	 = 0;
	char  	options [OPT_LEN];

	DPRINTF("cr_jfs: dname = %s, mntpt = %s, devtype = %x, lv_size = %s\n",
		*dname, mntpt, devtype, jfs_info->lvsize);
	/*
	 * you must specify either an existing device name
	 * or size
	 */

	if (! devtype && jfs_info->lvsize == NILPTR(char))
	{
		fprintf(stderr, MSGSTR(NEEDSIZE,
		"%s: For journalled file systems, must specify size\n"),
			progname);
		exit(10);
	}
	/*
	 * can't specify both a device and a volume group...
	 */
	if (devtype == S_IFBLK && jfs_info->vgrp != NILPTR(char))
	{
		fprintf(stderr, MSGSTR(ONEOFVG,
			"%s: specify either volume group or device\n"),
			progname);
		exit(10);
	}
	/*
	 * if device name given, figure out the volume
	 * group it's in...
	 */
	if (jfs_info->vgrp == NILPTR(char) && *dname != NILPTR(char))
		jfs_info->vgrp = getvgrp(*dname);	

	/*
	 * get logical partition to UBSIZE factor
	 */
	if ((vgpart = getvgpart(jfs_info->vgrp, *dname)))
		LPTOUB = vgpart / UBSIZE;

	DPRINTF("LPTOUB is %ld\n", LPTOUB);
	/*
	 * existing device given
	 */
	if (devtype == S_IFBLK)
	{
		/*
		 * make sure this device isn't already
		 * being used for a filesystem
		 */
		if (being_used(makedevname(*dname)))
			exit(10);		

		if (jfs_info->lvsize != NILPTR(char))
		{
			fprintf(stderr, MSGSTR(SIZEIG,
	"%s: Warning: existing device name given, size argument ignored.\n"),
				progname);
		}
		/*
		 * query device for volume group &
		 * logical volume size
		 */
		jfs_info->lvsize = getlvsiz(*dname);
	}
	/*
	 * round size up to next logical partition size
	 * (remember that the command line size is in
	 *  UBSIZE blocks)
	 */
	jfs_info->size = (atol(jfs_info->lvsize) + (LPTOUB - 1)) / LPTOUB;

	/*
	 * need a log device?
	 * try to query the lvm for the log device
	 */
	if (jfs_info->logdev == NILPTR(char) &&
	    (jfs_info->logdev = querylog(jfs_info->vgrp)) == NILPTR(char))
	{
		/*
		 * didn't find one - make one
		 */
		if ((jfs_info->logdev = (mklv(jfs_info->vgrp, jfs_info->logsiz,
					 "-t %s", Logtype))) == NULL)
		{
			exit (10);
		}
		
		jfs_info->logdev = makedevname(jfs_info->logdev);

		if (rc = run_cmd ("echo y | /usr/sbin/logform %s",
				  jfs_info->logdev))
		{
			run_cmd("/usr/sbin/rmlv -f %s",
				getdevname(jfs_info->logdev));			
			exit(rc);
		}
		log_made++;
	}

	/*
	 * if logical volume doesn't exist, make it
	 */
	if (!devtype)
	{
		if (*dname)
		{
			*dname = mklv(jfs_info->vgrp, jfs_info->size,
				     "-y %s", **dname);
		}
		else
			*dname = mklv(jfs_info->vgrp, jfs_info->size, "");

		if (*dname == NULL)
		{
			if (log_made)
			{
				run_cmd("/usr/sbin/rmlv -f %s",
					getdevname(jfs_info->logdev));
			}
			exit (10);
		}
		lv_made++;
	}

	jfs_info->size *= LPTOUB;

	/*
	 * Fill in the jfs specific options to mkfs. No verification
	 * or default specification done here - that's a job for the
	 * helper.
	 */
	options[0] = '\0';	
	sprintf (options, "-o frag=%d,nbpi=%d,compress=%s",
		 jfs_info->fragsize,jfs_info->nbpi, jfs_info->compress);

	/*
	 * run mkfs on our logical volume
	 */
	if (Debugflag)
	{
		DPRINTF ("mkfs -V %s -v %s -l %s %s %s\n",
			 "jfs", *dname, mntpt, options, makedevname(*dname));
		fflush (stdout);
		run_cmd("echo y | /usr/sbin/mkfs -D 2 -V %s -v %s -l %s %s %s",
			"jfs", *dname, mntpt, options, makedevname(*dname));
	}
	else
	{
		if (rc =
		    run_cmd ("echo y | /usr/sbin/mkfs -V %s -v %s -l %s %s %s",
		     "jfs", *dname, mntpt, options, makedevname(*dname)))
		{
			/*
			 * If we made the log or lv then remove it
			 */
			if (lv_made)
			{
				run_cmd("/usr/sbin/rmlv -f %s",
					getdevname(*dname));
			}
			if (log_made)
			{
				run_cmd("/usr/sbin/rmlv -f %s",
					getdevname(jfs_info->logdev));
			}
			exit (rc);
		}
	fprintf (stdout, MSGSTR(MAXSIZE, 
"Based on the parameters chosen, the new %s JFS file system\nis limited to a maximum size of %u (512 byte blocks)\n\n"),  
		 mntpt, get_abs_sz (jfs_info->nbpi, jfs_info->fragsize, vgpart / UBSIZE));

	}

}
