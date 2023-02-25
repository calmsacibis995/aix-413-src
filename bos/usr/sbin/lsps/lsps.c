static char sccsid[] = "@(#)76	1.10.1.11  src/bos/usr/sbin/lsps/lsps.c, cmdps, bos41J, 9515A_all 4/6/95 18:01:13";
/*
 * COMPONENT_NAME: (CMDPS) paging space commands
 *
 * FUNCTIONS: chps, lsps, mkps, rmps
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/vminfo.h>
#include <sys/cfgodm.h>
#include <sys/errno.h>
#include "cmdps.h"

/* Macros to determine values shown by lsps.
 * PAGES2MB converts from number of 4k pages to number of megabytes (MB).
 * PPS2MB converts from number of partitions of size 1<<p to megabytes (MB).
 * PERCENT_USED calculates percentage of space used
 */
#define PAGES2MB(s)		((s) / 256)
#define PPS2MB(n,p)		((n) * (1<<(p-20)))
#define PERCENT_USED(free,tot)	(100 - (100 * (free)) / (tot))

/* Reasonable buffer sizes.
 */
#define ODMBUF	40		/* maximum odm character string		*/
#define	CMDBUF	256		/* maximum command line string		*/

/* Directory for device special files.
 */
#define DEV		"/dev"
#define DEVNULL		"/dev/null"
#define NULLDEVNAME	"---"

/* ODM constants
 */
#define	TYPEA		"type"
#define PGTYPE		"paging"
#define PVPREFIX	"hdisk"
#define ODMPATH		"/etc/objrepos"
#define NFSTYPE		"swapfilename"

/* Paging device types
 */
#define LV_PAGING	1
#define	NFS_PAGING	2

/* Strings for the different paging types
 */
#define LV_PAGING_STR	"lv"
#define NFS_PAGING_STR	"nfs"

/* Flags and parameters
 */
static char	*cmd = NILSTR;		/* command name			*/
static int	aflg = 0;		/* auto flag			*/
static char	*aparm = NILSTR;	/*   and parameter		*/
static int	cflg = 0;		/* colon format			*/
static int	lflg = 0;		/* list format			*/
static int	nflg = 0;		/* swapon now flag		*/
static int	sflg = 0;		/* size flag			*/
static int	tflg = 0;		/* type flag			*/
static int	sumflg = 0;		/* summary flag			*/
static int	size = 0;		/*   and parameter		*/
static char	*vgname = NILSTR;	/* volume group name		*/
static char	*pvname = NILSTR;	/* physical volume name		*/
static char	*psname = NILSTR;	/* paging space name		*/
static char	*hostname = NILSTR;	/* hostname of NFS paging server*/
static char	*pathname = NILSTR;	/* pathname of swap file on NFS serv*/

/* Other variables in common.
 */
static char	lvid[ODMBUF];		/* logical volume identifier	*/
static char	vgid[ODMBUF];		/* volume group identifier	*/
static char	cmdstr[CMDBUF];		/* command string		*/
static char	devname[CMDBUF];	/* device special file name	*/
static char	*stanzafile;		/* stanza file name		*/
static char	*yesstr;		/* yes in appropriate language  */
static char	*nostr;			/* no in appropriate language	*/
static char	*aixyes = "y";		/* aix value for yes		*/
static char	*aixno	= "n";		/* aix value for no		*/

/* Attributes shown by lsps command.
 */
FILE *pvstream;				/* stream to get pv per ps	        */
int flagpv; 				/* got all ps attributes if flagpv=0    */
struct lsinfo
{
	struct lsinfo *next;		/* chain to next		*/
	char	psname[ODMBUF];		/* paging space name		*/
	char	pvname[ODMBUF];		/* physical volume name		*/
	char	vgname[ODMBUF];		/* volume group name		*/
	char	hostname[ODMBUF];	/* host name of paging server	*/
	char	filename[ODMBUF];	/* swap file name on server	*/
	int	lps;			/* size in logical partitions	*/
	int	size;			/* size in megabytes		*/
	int	used;			/* percent used 		*/
	char	*active;		/* indicates if active		*/
	char	*automatic;		/* indicates if automatic	*/
	char	type;			/* what type of paging device	*/
};

struct lsinfo *psitem;	/* current item in list			*/

/* Externs
 */
extern char *basename();

/* Local routines
 */
int chps();
int lsps();
int mkps();
int rmps();
static void usage();
static void fail();
static FILE *Popen();
static void get_psnames();
static int get_psattr();
static int get_psattr_nfs();
static int get_lvid_lvnm();
static int get_vgid_vgnm();
static int get_vgnm_lvid();
static int get_pvnm_lvid();
static int get_ppsz_lvid();
static int get_ppsz_vgid();
static int get_lvsz_lvid();
static int gen_lvnm();
static void gen_dvnm();
static void del_dvnm();
static int chk_type();
static int chk_type_nfs();
static int add_auto();
static int del_auto();
static int qry_auto();

main (int argc, char *argv[])
{
	int rc;
  
	(void) setlocale (LC_ALL,"");

	nls_catd = catopen (MF_CMDPS,NL_CAT_LOCALE);

	/* Handle interrupt/break
	 */

	/* Determine command name and call appropriate routine.
	 * Default command is lsps.
	 */
	cmd = basename(argv[0]);
	if (!strcmp(cmd, "chps"))
		rc = chps(argc, argv);
	else if (!strcmp(cmd, "lsps"))
		rc = lsps(argc, argv);
	else if (!strcmp(cmd, "mkps"))
		rc = mkps(argc, argv);
	else if (!strcmp(cmd, "rmps"))
		rc = rmps(argc, argv);
	else
	{
		fprintf(stderr, MSGSTR(WARN_NAME), cmd);
		rc = lsps(argc, argv);
	}

	exit(rc);
}

/*
 * NAME: chps
 *
 * FUNCTION: change paging space logical volume
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * Usage: chps [-s NewLPs] [-a {y|n}] Psname
 *
 * (RECOVERY OPERATION):
 *
 * (DATA STRUCTURES):
 *
 * RETURNS:
 *
 */
chps (int argc, char *argv[])
{
	int opt;		/* command line option flag		*/
	int newlps;		/* size to extend in logical partitions	*/
	int pg_type;		/* what type of paging device is this	*/

	/* Parse command line flags and associated parameters.
	 */
	while ((opt = getopt(argc, argv, "a:s:")) != EOF)
	{
		switch (opt)
		{
		case 'a':	/* automatic */
			aflg++;
			aparm = optarg;
			if (strcmp(aparm,"y") && strcmp(aparm,"n"))
			{
				/* -a parameter invalid
				 */
				fprintf(stderr, MSGSTR(BAD_APARM), cmd);
				usage(CHPS_USAGE);
			}
				
			break;

		case 's':	/* size */
			sflg++;
			size = atoi(optarg);
			break;

		default:	/* bad parameter */
			usage(CHPS_USAGE);
			break;
		}
	}
     
	/* Get paging space name from command line.
	 */
	switch(argc - optind)
	{
	case 0:
		/* No paging space name specified.
		 */
		fprintf(stderr, MSGSTR(NO_PS), cmd);
		usage(CHPS_USAGE);
		break;

	case 1:
		/* Paging space name specified.
		 * Make sure it's a paging space logical volume.
		 */
		psname = argv[optind];
		if (chk_type_nfs(NFSTYPE, psname) == 0) {
			pg_type = NFS_PAGING;
		}else if (chk_type(PGTYPE, psname) == 0) {
			pg_type = LV_PAGING;
		}else{
			fprintf(stderr, MSGSTR(BAD_PSNAME), cmd, psname);
			usage(CHPS_USAGE);
		}
		break;

	default:
		/* Too many parameters specified.
		 */
		usage(CHPS_USAGE);
		break;
	}

	/* Change size if specified.
	 */
	if (sflg)
	{
		if (pg_type == NFS_PAGING) {
			/* size not valid with NFS paging device
			 */
			fprintf(stderr, MSGSTR(BAD_SPARM_NFS), cmd);
			fail(CHPS_FAIL);
		}
		/* Size is input in number of new partitions.
		 */
		newlps = size;
		if (newlps > 0)
		{
			/* Call extendlv to extend the logical volume.
			 */
			sprintf(cmdstr, "/usr/sbin/extendlv %s %d", psname, newlps);
			if (system(cmdstr))
				fail(CHPS_FAIL);

			/* If paging space is active then issue swapon
			 * system call for extend to take effect.
			 */
			gen_dvnm(psname, devname);
			if (swapqry(devname, NULL) == 0)	
			{
				if (swapon(devname))
					fprintf(stderr, MSGSTR(WARN_SWAPON),
						cmd, psname);
			}
		}
		else if (newlps < 0)
		{
			/* size too small.
			 */
			fprintf(stderr, MSGSTR(BAD_SPARM), cmd);
			fail(CHPS_FAIL);
		}
	}

	/* Add or remove from automatic list if specified.
	 */
	if (aflg)
	{
		if (!strcmp(aparm, "y"))
		{
			/* Add to list if it's not already there.
			 * Issue warning message if add fails.
			 */
			if (add_auto(psname))
			{
				fprintf(stderr, MSGSTR(WARN_AUTO),
					cmd, stanzafile);
			}
		}
		else
		{
			/* Remove from list.
			 * Issue warning message if delete fails.
			 */
			if (del_auto(psname))
			{
				fprintf(stderr, MSGSTR(WARN_AUTO),
					cmd, stanzafile);
			}
		}
	}

	return(0);
}

/*
 * NAME: lsps
 *
 * FUNCTION: list attributes of paging space logical volume
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * Usage: lsps {-s | [-c | -l] {-a | Psname | -t {lv|nfs} } }
 *
 * (RECOVERY OPERATION):
 *
 * (DATA STRUCTURES):
 *
 * RETURNS:
 *
 */
lsps (int argc, char *argv[])
{
	int opt;		/* command line option flag		*/
	int pg_type;		/* what type of paging space is it	*/
	struct pginfo pginfo;	/* swapqry() info for active pg spaces	*/
	struct lsinfo *pslist;	/* list of paging space lvs to display	*/
	int numps, thisps;	/* number of paging space lvs		*/
	char *format;		/* format string for attribute display	*/
	int print_type;		/* what type of paging device to display*/
	int tot_size;		/* total size of paging space (4K blks) */
	int not_alloc;		/* unallocated paging space (4K blks)   */
	int NoOfTrials = 5;	/* If out of RESOURCES try the most 5 times */

	pg_type = LV_PAGING;
	print_type = LV_PAGING;
	
	/* Parse command line flags and associated parameters.
	 */
	while ((opt = getopt(argc, argv, "aclst:")) != EOF)
	{
		switch (opt)
		{
		case 'a':	/* all */
			aflg++;
			break;

		case 'c':	/* colon format */
			cflg++;
			break;

		case 'l':	/* list format */
			lflg++;
			break;
		case 't':
			if (tflg)
				usage(LSPS_USAGE);
			tflg++; /* type flag */
			if (strcmp(LV_PAGING_STR, optarg) == 0) {
				print_type = LV_PAGING;
			}else if (strcmp(NFS_PAGING_STR, optarg) == 0) {
				print_type = NFS_PAGING;
			}else{
				usage(LSPS_USAGE);
			}
			break;
		case 's':	/* summary information */
			sumflg++;
			break;
		default:	/* bad parameter */
			usage(LSPS_USAGE);
			break;
		}
	}
     
	/*
	 * If summary information requested, skip remainder of 
	 * parameter checking and simply display the information.
	 */
	if (sumflg) {

		/* Retrieve total size of paging space in 4K blocks */
		tot_size = psdanger(0);

		/* Retrieve number of unallocated 4K blocks */
		not_alloc = psdanger(-1);

		/* Display information and exit */
		printf(MSGSTR(LSPS_SUMHEADER));
		printf("      %3dMB             %3d%%\n", 
			PAGES2MB(tot_size), PERCENT_USED(not_alloc,tot_size));
		return(0);
	}

	/* Get paging space name from command line.
	 */
	switch(argc - optind)
	{
	case 0:
		/* No paging space name specified.
		 */
		if (aflg || tflg)
		{
			psname = NILSTR;
		}
		else
		{
			/* Neither -a, -t nor paging space name specified.
			 */
			fprintf(stderr, MSGSTR(LSPS_AORPS), cmd);
			usage(LSPS_USAGE);
		}
		break;

	case 1:
		/* Paging space name specified.
		 * Make sure it's a paging space logical volume.
		 * -or- NFS paging/swap device.
		 */
		psname = argv[optind];
		if (chk_type_nfs(NFSTYPE, psname) == 0) {
			pg_type = NFS_PAGING;
		}else if (chk_type(PGTYPE, psname) == 0) {
			pg_type = LV_PAGING;
		}else{
			fprintf(stderr, MSGSTR(BAD_PSNAME), cmd, psname);
			usage(LSPS_USAGE);
		}
		break;

	default:
		/* Too many parameters specified.
		 */
		usage(LSPS_USAGE);
		break;
	}

	/* Check for invalid combinations of flags.
	 */
	if (cflg && lflg)
	{
		fprintf(stderr, MSGSTR(LSPS_CANDL), cmd);
		usage(LSPS_USAGE);
	}

	if (aflg && strcmp(psname, NILSTR))
	{
		fprintf(stderr, MSGSTR(LSPS_AANDPS), cmd);
		usage(LSPS_USAGE);
	}

	if (tflg && strcmp(psname, NILSTR))
	{
		usage(LSPS_USAGE);
	}

	if (aflg && tflg)
	{
		usage(LSPS_USAGE);
	}

	/* Determine how many paging space logical volumes to show.
	 */
	if (aflg || tflg)
	{
		/* Get names of all paging space logical volumes.
		 */
		get_psnames(&pslist, &numps);
	}
	else
	{
		/* Just one name from command line.
		 */
		pslist = (struct lsinfo *) malloc (sizeof(struct lsinfo));
		strcpy(pslist->psname, psname);
		pslist->type = pg_type;
		numps = 1;
	}

	/* Get yes and no strings.
	 */
	yesstr = MSGSTR(LSPS_YES);
	nostr = MSGSTR(LSPS_NO);
	/* For each paging space.
	 */
	for (thisps=0,psitem=pslist; thisps<numps; thisps++,psitem=psitem->next)
	{
		if (lflg)
		{
			/* List format (-l) just display paging space names.
			 * Make sure that if the user specified the -t
			 * flag that we print only the type that matches.
			 */
			if (tflg) {
				if (psitem->type == print_type)
					fprintf(stdout, "%s\n", 
						psitem->psname);
			}else{
				fprintf(stdout, "%s\n", psitem->psname);
			}
		}
		else
		{
			/* Print header if first pass.
			 */
			if (thisps == 0)
			{
				/* Use the default message text for the
				 * colon-format headers since the SMIT
				 * dialogues depend on these messages not
				 * being translated and they in fact have
				 * been translated by some.
				 */
				if (cflg)
				    if (print_type == LV_PAGING)
					fprintf(stdout, msgstr[LSPS_CHEADER]);
				    else
					fprintf(stdout, msgstr[LSPS_CHEADER_NFS]);
				else
				    if (print_type == LV_PAGING)
					fprintf(stdout, MSGSTR(LSPS_HEADER));
				    else
					fprintf(stdout, MSGSTR(LSPS_HEADER_NFS));
			}

			/* Let's stop early if the user specified the -t
			 * flag and the type of the paging space found
			 * does not match what the user would like to see
			 */
			if (tflg && print_type != psitem->type) {
				continue;
			}
			/* Get attributes to list.
			 */
	/* Reset flagpv to one.
	*/ 
	flagpv = 1;
	/* Begin loop to retrieve attributes of a physical volume one by one per ps.  
	*/
	while (--NoOfTrials && flagpv != 0 ) 
	{
			/* If hit EOF of stream in get_psattr() below, flagpv = 0,
			then get out of loop */
			if ((psitem->type == LV_PAGING && get_psattr(psitem)) ||
			    (psitem->type == NFS_PAGING && get_psattr_nfs(psitem)))
			{
				if (psitem->type == LV_PAGING && flagpv == 0) 
					continue;		/* if EOF, continue.*/
				fprintf(stderr, MSGSTR(LSPS_FAIL),
					cmd, psitem->psname);
				continue;
			}
		
			/* If active paging space, use swapqry info.
			 */
			gen_dvnm(psitem->psname, devname);
			if(swapqry(devname, &pginfo) == 0)
			{
				psitem->active = yesstr;
				psitem->used = PERCENT_USED(pginfo.free,
								pginfo.size);
	 			if(psitem->type == NFS_PAGING)
					psitem->size = PAGES2MB(pginfo.size);
			}

			/* Now display attributes.
			 */
			if (cflg)
			{
				/* For the colon format output, the size
				 * is in logical partitions and the yes/no
				 * strings must be the aix values y or n.
				 */
				format = MSGSTR(LSPS_CFORMAT);
				if (psitem->active == yesstr)
					psitem->active = aixyes;
				else
					psitem->active = aixno;
				if (psitem->automatic == yesstr)
					psitem->automatic = aixyes;
				else
					psitem->automatic = aixno;
				if (print_type == LV_PAGING)
					fprintf(stdout, format,
						psitem->psname, psitem->pvname,
						psitem->vgname, psitem->lps,
						psitem->used, psitem->active,
						psitem->automatic, 
						(psitem->type == LV_PAGING ?
						 LV_PAGING_STR : NFS_PAGING_STR));
				else
					fprintf(stdout, format,
						psitem->psname, 
						psitem->hostname,
						psitem->filename, psitem->lps,
						psitem->used, psitem->active,
						psitem->automatic, 
						(psitem->type == LV_PAGING ?
						 LV_PAGING_STR : NFS_PAGING_STR));
			}
			else
			{
				if (print_type == LV_PAGING) {
					format = MSGSTR(LSPS_FORMAT);
					fprintf(stdout, format,
						psitem->psname, psitem->pvname,
						psitem->vgname, psitem->size,
						psitem->used, psitem->active,
						psitem->automatic,
						(psitem->type == LV_PAGING ?
						 LV_PAGING_STR : NFS_PAGING_STR));
				}else{
					format = MSGSTR(LSPS_FORMAT);
					fprintf(stdout, format,
						psitem->psname, 
						psitem->hostname,
						psitem->filename, psitem->size,
						psitem->used, psitem->active,
						psitem->automatic,
						(psitem->type == LV_PAGING ?
						 LV_PAGING_STR : NFS_PAGING_STR));
				}
			}
		}
		NoOfTrials = 5;	/* reset to try at most 5 times */
	} /* END while loop */
	}/* end of 'for each psitem' */

	return(0);
}
		
/*
 * NAME: mkps
 *
 * FUNCTION: make paging space logical volume
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * Usage: mkps [-a] [-n] [-t lv] -s NumLPs Vgname [Pvname]
 *		-or-
 *	  mkps [-a] [-n] -t nfs hostname pathname
 *
 * (RECOVERY OPERATION):
 *
 * (DATA STRUCTURES):
 *
 * RETURNS:
 *
 */
mkps (int argc, char *argv[])
{
	int opt;		/* command line option flag		*/
	int numlps;		/* size to extend in logical partitions	*/
	char genname[ODMBUF];	/* generated paging space name		*/
	int  mk_type;		/* what type of paging space to make	*/

	mk_type = LV_PAGING;

	/* Parse command line flags and associated parameters.
	 */
	while ((opt = getopt(argc, argv, "ans:t:")) != EOF)
	{
		switch (opt)
		{
		case 'a':	/* automatic */
			aflg++;
			break;

		case 'n':	/* activate */
			nflg++;
			break;

		case 's':	/* size */
			sflg++;
			size = atoi(optarg);
			break;

		case 't':	/* type */
			if (strcmp(LV_PAGING_STR, optarg) == 0) {
				mk_type = LV_PAGING;
			}else if (strcmp(NFS_PAGING_STR, optarg) == 0) {
				mk_type = NFS_PAGING;
			}else{
				usage(MKPS_USAGE);
			}
			break;
		default:	/* bad parameter */
			usage(MKPS_USAGE);
			break;
		}
	}
    
	if (mk_type == NFS_PAGING && sflg) {
		usage(MKPS_USAGE);
	} 

	/* Get remaining command line parameters.
	 */
	switch(argc - optind)
	{
	case 0:
		/* No vgname specified.
		 */
		fprintf(stderr, MSGSTR(NO_VG), cmd);
		usage(MKPS_USAGE);
		break;

	case 1:
		/* Just vgname specified.
		 * -or- Just hostname or pathname specified (If NFS type)
		 */
		if (mk_type == NFS_PAGING)
			usage(MKPS_USAGE);
		vgname = argv[optind];
		pvname = NILSTR;
		break;

	case 2:
		/* Both vgname and pvname specified.
		 * -or- hostname and pathname specified (If NFS type)
		 */
		if (mk_type == LV_PAGING) {
			vgname = argv[optind];
			pvname = argv[optind+1];
		}else{
			hostname = argv[optind];
			pathname = argv[optind + 1];
		}
		break;

	default:
		/* Too many parameters specified.
		 */
		usage(MKPS_USAGE);
		break;
	}
	if (mk_type == LV_PAGING) {
		/* Size is input in number of partitions.
	 	*/
		numlps = size;
	
		/* Generate logical volume name to use for mklv.
	 	* (Either do this or parse the message from mklv
	 	* "mklv: logical volume name xxxx generated")
	 	*/
		if (gen_lvnm(PGTYPE, genname))
			fail(MKPS_FAIL);
		psname = genname;

		/* Call mklv with the following options:
	 	* -t PGTYPE	: logical volume type for paging space
	 	* -a c		: intra-policy = center (allocate in center of disk)
	 	* -u 1		: upperbound = 1 (allocate from one physical disk)
	 	* -b n		: don't relocate bad blocks
	 	* -y psname	: specify the logical volume name to use
	 	* vgname	: volume group that logical volume will belong to
	 	* numlps	: number of logical partions
	 	* pvname	: physical volume to allocate from
	 	*/
		sprintf(cmdstr, "/usr/sbin/mklv -t %s -a c -u 1 -b n -y %s %s %d %s",
			PGTYPE, psname, vgname, numlps, pvname);
		if (system(cmdstr))
			fail(MKPS_FAIL);

	}else{
		int rc;			/* return code			  */
		FILE *stream;		/* stream between program and cmd */
		
	
		sprintf(cmdstr, 
		"/usr/sbin/mkdev -c swap -s nfs -t paging -a 'hostname=%s swapfilename=%s", 
			hostname, pathname);
		stream = Popen(cmdstr, "r");
		if (stream == NULL)
		{
			fail(MKPS_FAIL);
		}
		else
		{
			fscanf(stream, "%9s", psname);
			rc = pclose(stream);
		}
		if (rc != 0)
			fail(MKPS_FAIL);	
	}   
	/* Activate the paging space if specified.
	 */
	if (nflg)
	{
		gen_dvnm(psname, devname);
		if (swapon(devname))
			fprintf(stderr, MSGSTR(WARN_SWAPON), cmd, psname);
	}

	/* Add to automatic list if specified.
	 */
	if (aflg)
	{
		/* Add to list.
		 * Issue warning message if add fails.
		 */
		if (add_auto(psname))
		{
			fprintf(stderr, MSGSTR(WARN_AUTO),
				cmd, stanzafile);
		}
	}

	return(0);
}

/*
 * NAME: rmps
 *
 * FUNCTION: remove paging space logical volume
 *
 * EXECUTION ENVIRONMENT:
 *
 * (NOTES:)
 *
 * Usage: rmps Psname
 *
 * (RECOVERY OPERATION):
 *
 * (DATA STRUCTURES):
 *
 * RETURNS:
 *
 */
rmps (int argc, char *argv[])
{
	int pg_type;		/* What type of paging space to remove */

	/* Get paging space name from command line.
	 */
	switch(argc - 1)
	{
	case 0:
		/* No paging space name specified.
		 */
		fprintf(stderr, MSGSTR(NO_PS), cmd);
		usage(RMPS_USAGE);
		break;

	case 1:
		/* Paging space name specified.
		 * Make sure it's a paging space logical volume.
		 */
		psname = argv[optind];
		if (chk_type_nfs(NFSTYPE, psname) == 0) {
			pg_type = NFS_PAGING;
		}else if (chk_type(PGTYPE, psname) == 0) {
			pg_type = LV_PAGING;
		}else{
			fprintf(stderr, MSGSTR(BAD_PSNAME), cmd, psname);
			usage(RMPS_USAGE);
		}
		break;

	default:
		/* Too many parameters specified.
		 */
		usage(RMPS_USAGE);
		break;
	}

	/* If paging space is active then cannot remove it.
	 */
	gen_dvnm(psname, devname);
	if (swapqry(devname, NULL) == 0)	
	{
		fprintf(stderr, MSGSTR(RMPS_ACTIVE), cmd, psname);
		fail(RMPS_FAIL);
	}
	else
	{
		/* Remove the logical volume -or- NFS paging device
		 */
		if (pg_type == NFS_PAGING)
			sprintf(cmdstr, "/usr/sbin/rmdev -d -l %s", psname);
		else
			sprintf(cmdstr, "/usr/sbin/rmlv -f %s", psname);
		if (system(cmdstr))
			fail(RMPS_FAIL);
	}

	/* Remove stanza from stanza file.
	 */
	if (del_auto(psname))
	{
		fprintf(stderr, MSGSTR(WARN_AUTO), cmd, stanzafile);
	}

	if (pg_type == LV_PAGING) {
		/* Remove device special files
		 * (since rmlv doesn't remove them).
		 */
		del_dvnm(psname);
	}

	return(0);
}

/*
 * NAME: usage
 *
 * FUNCTION: display specified usage message and exit
 */
static
void
usage (int msgnum)
{
	fprintf(stderr, MSGSTR(msgnum), cmd);
	exit(1);
}

/*
 * NAME: fail
 *
 * FUNCTION: display specified failure message and exit
 */
static
void
fail (int msgnum)
{
	fprintf(stderr, MSGSTR(msgnum), cmd, psname);
	exit(1);
}

/*
 * NAME: Popen
 *
 * FUNCTION: Try popen() on Cmd and retry if a failure occurs.
 *
 * RETURNS:
 */
static FILE *
Popen(char *Cmd,char *mode)
{
	int PopenTrials = 5 ;
	FILE * Stream ;
	while (--PopenTrials > 0) {
		if ((Stream = popen(Cmd,mode)) == NULL)
			sleep(1);
		else
			break ;
	}
	if (PopenTrials == 0) {
		perror(cmd);
		fprintf(stderr, MSGSTR(LSPS_FAIL), cmd, psitem->psname);
		fflush(stderr);
		flagpv = 0 ;
	}
	return(Stream);
}

/*
 * NAME: get_psnames
 *
 * FUNCTION: retrieve names of all paging space logical volumes
 *
 * (NOTES:)
 *
 * uses odm_get_list()
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
void
get_psnames (struct lsinfo **list, int *cnt)
{
	struct CuAt *cuatp;		/* odm query struct		*/
	struct objlistinfo stat_info;	/* odm query status		*/
	char crit[CMDBUF];		/* odm search criteria		*/
	struct lsinfo *infop, *lastp;
	int num;

	/* Prepare to use odm.
	 */
	odm_initialize();
	odm_set_path(ODMPATH);

	/* Set up criteria and issue odm query.
	 */
	sprintf(crit,"attribute = '%s' and value = '%s'", TYPEA, PGTYPE);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* If query fails just return NULL list.
	 */
	if ((int)cuatp == -1)
	{
		*cnt = 0;
		*list = (struct lsinfo *) NULL;
		odm_terminate();
		return;
	}

	/* Allocate a lsinfo struct for each paging space found.
	 */
	lastp = NULL;
	for (num=0; num<stat_info.num; num++)
	{
		/* Allocate new lsinfo structure.
		 */
		if (!(infop = (struct lsinfo *)
				malloc (sizeof(struct lsinfo))))
			break;

		/* Link to last one.
		 */
		infop->next = lastp;
		lastp = infop;

		/* Fill out paging space name.
		 */
		strcpy (infop->psname, cuatp->name);
		/* Set the type of this paging device
		 */
		infop->type = LV_PAGING;
		cuatp++;
	}

	*cnt = num;
	*list = lastp;

	/* Set up criteria and issue odm query.
	 */
	sprintf(crit,"attribute = '%s'", NFSTYPE);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* If query fails just return NULL list.
	 */
	if ((int)cuatp == -1)
	{
		odm_terminate();
		return;
	}

	/* Allocate a lsinfo struct for each paging space found.
	 */
	for (num=0; num<stat_info.num; num++)
	{
		/* Allocate new lsinfo structure.
		 */
		if (!(infop = (struct lsinfo *)
				malloc (sizeof(struct lsinfo))))
			break;

		/* Link to last one.
		 */
		infop->next = lastp;
		lastp = infop;

		/* Fill out paging space name.
		 */
		strcpy (infop->psname, cuatp->name);
		/* Set the type of this paging device
		 */
		infop->type = NFS_PAGING;
		cuatp++;
	}

	*cnt += num;
	*list = lastp;

	/* done using odm.
	 */
	odm_terminate();

	return;
}

/*
 * NAME: get_psattr
 *
 * FUNCTION: retrieve paging space attributes displayed by lsps.
 *
 * (NOTES:)
 *
 * uses getlvodm()
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_psattr (struct lsinfo *lsitem)
{
	static char psid[ODMBUF];	/* logical volume identifier	*/
	int l2_ppsize;			/* partition size in power-of-2 */
	int numlps;			/* number of logical partitions */
	FILE *stream;			/* stream between this program and cmd	*/

	/* First get logical volume id from name.
	 */
	if (get_lvid_lvnm(lsitem->psname, psid))
		return(1);

	/* Get physical volume name from logical volume id.
	 */
	if (get_pvnm_lvid(psid, lsitem->pvname))
		return(1);

	/* Get volume group name from logical volume id.
	 */
	if (get_vgnm_lvid(psid, lsitem->vgname))
		return(1);

	/* Get logical volume size from logical volume id.
	 */
	if (get_lvsz_lvid(lsitem->psname, &numlps))
		return(1);

	/* Get partition size from logical volume id
	 * (in power-of-2 bytes).
	 */
	if (get_ppsz_lvid(psid, &l2_ppsize))
		return(1);

	/* Save size in both partitions and megabytes.
	 */
	lsitem->lps = numlps;
	lsitem->size = PPS2MB(numlps, l2_ppsize);

	/* Determine if in automatic list.
	 */
	if (qry_auto(lsitem->psname) == 0)
		lsitem->automatic = yesstr;
	else
		lsitem->automatic = nostr;
	
	/* Set remaining fields to indicate inactive paging space.
	 */
	lsitem->used = 0;
	lsitem->active = nostr;
	lsitem->type = LV_PAGING;
	strcpy(lsitem->hostname, NULLDEVNAME);
	strcpy(lsitem->filename, NULLDEVNAME);

	return(0);
}   

/*
 * NAME: get_psattr_nfs
 *
 * FUNCTION: retrieve paging space attributes displayed by lsps for an NFS
 *	paging device
 *
 * (NOTES:)
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_psattr_nfs (struct lsinfo *lsitem)
{
	struct CuAt *cuatp;		/* odm query struct		*/
	struct objlistinfo stat_info;	/* odm query status		*/
	char crit[CMDBUF];		/* odm search criteria		*/

	/* Reset flagpv to zero.
	*/
	flagpv = 0;

	/* Prepare to use odm.
	 */
	odm_initialize();
	odm_set_path(ODMPATH);

	/* Set up criteria and issue odm query.
	 */
	sprintf(crit,"attribute = hostname and name = '%s'", lsitem->psname);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* If query fails just return failure
	 */
	if ((int)cuatp == -1 || stat_info.num == 0)
	{
		odm_terminate();
		return(1);
	}

	/* Get the host name of the paging server and place it in the
	 * physical volumn name
	 */
	strcpy(lsitem->hostname, cuatp->value);

	/* Set up criteria and issue odm query.
	 */
	sprintf(crit,"attribute = swapfilename and name = '%s'", lsitem->psname);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* If query fails just return failure
	 */
	if ((int)cuatp == -1)
	{
		odm_terminate();
		return(1);
	}

	/* done using odm.
	 */
	odm_terminate();

	/* Get the file name of the paging file on the server
	 */
	strcpy(lsitem->filename, cuatp->value);

	lsitem->lps = 0;
	lsitem->size = 0;

	/* Determine if in automatic list.
	 */
	if (qry_auto(lsitem->psname) == 0)
		lsitem->automatic = yesstr;
	else
		lsitem->automatic = nostr;
	
	/* Set remaining fields to indicate inactive paging space.
	 */
	lsitem->used = 0;
	lsitem->active = nostr;
	strcpy(lsitem->pvname, NULLDEVNAME);
	strcpy(lsitem->vgname, NULLDEVNAME);
	return(0);
}

/*
 * NAME: get_lvid_lvnm
 *
 * FUNCTION: retrieve logical volume id given logical volume name
 *
 * (NOTES:)
 *
 * uses getlvodm()
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_lvid_lvnm (char *name, char id[])
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/getlvodm -l %s", name);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		id = NILSTR;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%22s", id);
		rc = pclose(stream);
	}
	
	return(rc);
}   

/*
 * NAME: get_vgid_vgnm
 *
 * FUNCTION: retrieve volume group id given volume group name
 *
 * (NOTES:)
 *
 * uses getlvodm()
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_vgid_vgnm (char *name, char id[])
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/getlvodm -v %s", name);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		id = NILSTR;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%22s", id);
		rc = pclose(stream);
	}
	
	return(rc);
}   

/*
 * NAME: get_vgnm_lvid
 *
 * FUNCTION: retrieve volume group name given logical volume id
 *
 * (NOTES:)
 *
 * uses getlvodm()
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_vgnm_lvid (char id[], char *name)
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/getlvodm -b %s", id);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		name = NILSTR;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%s", name);
		rc = pclose(stream);
	}
	
	return(rc);
}   

/*
 * NAME: get_pvnm_lvid
 *
 * FUNCTION: retrieve physical volume name given logical volume id
 *
 * (NOTES:)
 *
 * uses output of lslv -l
 * only retrieves first name even if more exist
 *
 * RETURNS:
 *
 *   0	if successful
 *   1	if not successful
 */
static
int
get_pvnm_lvid (char id[], char *name)
{
	int rc = 0;					/* return code */
	int i;
	if (flagpv == 1)
	{
		sprintf(cmdstr, "/usr/sbin/lslv -l %s | /usr/bin/sed '1,2d' | /usr/bin/cut -d: -f1",
			id, PVPREFIX);
		pvstream = Popen(cmdstr, "r");
		if (pvstream == NULL)
		{
			name = NILSTR;
			return(1);
		}
	}
	if ((i = fscanf(pvstream, "%s", name)) == EOF) 
	{
		flagpv = 0;
		rc = pclose(pvstream);
		if (rc != 0)
			fail(LSPS_FAIL);	
		return(1);
	}
	return(rc);
}


/*
 * NAME: get_ppsz_lvid
 *
 * FUNCTION: retrieve physical partition size given logical volume id
 *
 * (NOTES:)
 *
 * uses lquerylv()
 *
 * RETURNS:
 *
 */
static
int
get_ppsz_lvid (char id[], int *ppsize)
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/lquerylv -L %s -s", id);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		*ppsize = 0;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%d", ppsize);
		rc = pclose(stream);
	}
	
	return(rc);
}

/*
 * NAME: get_ppsz_vgid
 *
 * FUNCTION: retrieve physical partition size given volume group id
 *
 * (NOTES:)
 *
 * uses lqueryvg()
 * 
 * RETURNS:
 *
 */
static
int
get_ppsz_vgid (char id[], int *ppsize)
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/lqueryvg -g %s -s", id);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		*ppsize = 0;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%d", ppsize);
		rc = pclose(stream);
	}
	
	return(rc);
}

/*
 * NAME: get_lvsz_lvid
 *
 * FUNCTION: retrieve logical volume size given logical volume id
 *
 * (NOTES:)
 *
 * uses lslv()
 *
 * RETURNS:
 *
 */
static
int
get_lvsz_lvid (char *psname, int *lvsize)
{
	int rc = 0;			/* return code */
	fscanf(pvstream, "%d", lvsize);
	flagpv ++;			/* increment flagpv so to skip 
					   opening the stream again. */
	return(rc);
}

/*
 * NAME: gen_lvnm
 *
 * FUNCTION: Generate unique logical volume name.
 *
 * (NOTES:)
 *
 * uses getlvname()
 *
 * RETURNS:
 *
 */
static
int
gen_lvnm (char *type, char *name)
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/

	sprintf(cmdstr, "/usr/sbin/getlvname -Y %s", type);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		name = NULL;
		rc = 1;
	}
	else
	{
		fscanf(stream, "%s", name);
		rc = pclose(stream);
	}
	
	return(rc);
}

/*
 * NAME: gen_dvnm
 *
 * FUNCTION: Generate device special file name for paging space lv
 *
 * (NOTES:)
 *
 * RETURNS:
 *
 */
static
void
gen_dvnm (char *name, char dvname[])
{
	sprintf(dvname, "%s/%s", DEV, name);
}

/*
 * NAME: del_dvnm
 *
 * FUNCTION: Delete device special files for a paging space lv
 *
 * (NOTES:)
 *
 * RETURNS:
 *
 */
static
void
del_dvnm (char *name)
{
	/* Remove device special files
	 * /dev/xxxx and /dev/rxxxx
	 */
	sprintf(cmdstr, "/usr/bin/rm -f %s/%s %s/r%s", DEV, name, DEV, name);
	(void) system(cmdstr);
}

/*
 * NAME: chk_type_nfs
 *
 * FUNCTION: Check the type of an NFS paging device
 *
 * (NOTES:)
 *
 * RETURNS:
 *
 *  0	if type matches
 *  1	if type doesn't match
 */
static
int
chk_type_nfs (char *type, char *name)
{
	int rc;			/* return code				*/
	struct CuAt *cuatp;		/* odm query struct		*/
	struct objlistinfo stat_info;	/* odm query status		*/
	char crit[CMDBUF];		/* odm search criteria		*/

	/* Prepare to use odm.
	 */
	odm_initialize();
	odm_set_path(ODMPATH);

	/* Set up criteria and issue odm query.
	 */
	sprintf(crit,"attribute = %s and name = '%s'", type, name);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* If query fails just return failure
	 */
	if ((int)cuatp == -1)
	{
		odm_terminate();
		return(1);
	}
	odm_terminate();
	if (stat_info.num == 0) {
		return(1);
	}else{
		return(0);
	}
}

/*
 * NAME: chk_type
 *
 * FUNCTION: Check the type of logical volume.
 *
 * (NOTES:)
 *
 * uses getlvodm()
 *
 * RETURNS:
 *
 *  0	if type matches
 *  1	if type doesn't match
 */
static
int
chk_type (char *type, char *name)
{
	int rc;			/* return code				*/
	FILE *stream;		/* stream between this program and cmd	*/
	char chklvid[ODMBUF];	/* logical volume id			*/
	char chktype[ODMBUF];	/* logical volume type			*/

	/* First get the logical volume id from the name.
	 */
	if (get_lvid_lvnm(name, chklvid))
		return(1);

	sprintf(cmdstr, "/usr/sbin/getlvodm -y %s", chklvid);
	stream = Popen(cmdstr, "r");
	if (stream == NULL)
	{
		rc = 1;
	}
	else
	{
		fscanf(stream, "%s", chktype);
		rc = pclose(stream);
		if (!strcmp(chktype, type))
			rc = 0;
		else
			rc = 1;
	}

	return(rc);
}

/*
 * NAME: add_auto
 *
 * FUNCTION: add paging space to automatic list
 *
 * (NOTES:)
 *
 * modifies /etc/swapspaces
 *
 * RETURNS:
 *
 *  0	if successful (added or already present)
 *  1	if not successful
 */
static
int
add_auto (char *name)
{
	int rc;			/* return code				*/

	/* Set stanza file pathname from environment variable
	 * or default.
	if (!(stanzafile = (char *)getenv(ENVSWAP)))
	 */
		stanzafile = ETCSWAP;

	/* First determine if stanza is already present.
	 */
	sprintf(cmdstr, "/usr/bin/grep -p '^%s:' %s > %s 2>&1",
		name, stanzafile, DEVNULL);
	rc = system(cmdstr);
	
	switch(WEXITSTATUS(rc))
	{
	case 0:
		/* Stanza already present.
		 * Just indicate success.
		 */
		rc = 0;
		break;

	case 1:
		/* No stanza was found.
		 * Append stanza of the form:
		 *
		 * psname:
		 *	dev = /dev/psname
		 *
		 */
		gen_dvnm(name, devname);
		sprintf(cmdstr, "/usr/bin/echo '\n%s:\n\tdev = %s\n\n' >> %s",
			name, devname, stanzafile);	
		if (system(cmdstr))
			rc = 1;
		else
			rc = 0;
		break;

	case 2:
	default:
		/* Syntax error or file inaccessible or 'system' failed.
		 * Indicate failure.
		 */
		rc = 1;
		break;
	}

	return(rc);
}
    
/*
 * NAME: del_auto
 *
 * FUNCTION: removes paging space from automatic list
 *
 * (NOTES:)
 *
 * creates a temporary file /tmp/swaps.PID
 * modifies /etc/swapspaces
 *
 * RETURNS:
 *
 *  0	if successful (stanza removed or was not present)
 *  1	if not successful
 */
static
int
del_auto (char *name)
{
	int rc;			/* return code				*/
	char *temp;		/* pathname of temporary file		*/

	/* Set stanza file pathname from environment variable
	 * or default.
	if (!(stanzafile = (char *)getenv(ENVSWAP)))
	 */
		stanzafile = ETCSWAP;

	/* Set temporary file name.
	 */
	if (!(temp = tempnam(NULL, "swap")))
		temp = "/tmp/swap";

	/* Remove stanza and place result in temporary file.
	 */
	sprintf(cmdstr, "/usr/bin/grep -v -p '^%s:' %s > %s 2>%s",
		name, stanzafile, temp, DEVNULL);	
	rc = system(cmdstr);

	switch(WEXITSTATUS(rc))
	{
	case 0:
	case 1:
		/* Stanza was found and removed or was not found.
		 * Replace stanza file with temporary file.
		 */
		sprintf(cmdstr, "/usr/bin/mv %s %s", temp, stanzafile);	
		if (system(cmdstr))
		{
			/* Replacement failed.
			 * Remove temporary file and indicate failure.
			 */
			sprintf(cmdstr, "/usr/bin/rm -f %s", temp);	
			rc = system(cmdstr);
			rc = 1;
		}
		else
			rc = 0;
		break;

	case 2:
	default:
		/* Syntax error or file inaccessible or 'system' failed.
		 * Remove temporary file and indicate failure.
		 */
		sprintf(cmdstr, "/usr/bin/rm -f %s", temp);	
		rc = system(cmdstr);
		rc = 1;
		break;
	}

	return(rc);
}

/*
 * NAME: qry_auto
 *
 * FUNCTION: determine if paging space is in automatic list
 *
 * (NOTES:)
 *
 * RETURNS:
 *
 *  0	if in autotmatic list
 *  1	if not in automatic list
 */
static
int
qry_auto(char *name)
{
	int rc;			/* return code				*/

	/* Set stanza file pathname from environment variable
	 * or default.
	if (!(stanzafile = (char *)getenv(ENVSWAP)))
	 */
		stanzafile = ETCSWAP;

	/* Determine if stanza is present.
	 */
	sprintf(cmdstr, "/usr/bin/grep -p '^%s:' %s > %s 2>&1",
		name, stanzafile, DEVNULL);	
	rc = system(cmdstr);
	
	switch(WEXITSTATUS(rc))
	{
	case 0:
		/* Stanza present.
		 */
		rc = 0;
		break;

	case 1:
		/* No stanza was found.
		 */
		rc = 1;
		break;

	case 2:
	default:
		/* Syntax error or file inaccessible or 'system' failed.
		 * Indicate not found.
		 */
		rc = 1;
		break;
	}

	return(rc);
}
