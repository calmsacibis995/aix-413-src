static char sccsid[] = "@(#)24	1.11  src/bos/usr/sbin/quota/quot.c, cmdquota, bos411, 9428A410j 12/23/93 12:12:12";
/* 
 * COMPONENT_NAME: CMDQUOTA 
 *
 * FUNCTIONS: 	main, quotall, check, acct, qcmp, report, groupname,
 *		inset, make_blank_entry, place_blank_entry, fill_in,
 *		create_names_entry, set_hash_table, free_hash_table,
 *		getname
 *	 
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980, 1990 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Berkeley   sccsid = "@(#)quot.c	4.14 (Berkeley) 88/04/18"
 */

#include <stdio.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <sys/param.h>
#include <fcntl.h>
#include "fstab.h"	/* AIX does not include fs_vfstype in struct fstab */
#include <locale.h>
#include <ctype.h>
#include <libfs/libfs.h>


#include "quot_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOT,n,s)

#define BLOCK		512            /* Block size I want to report      */

struct du {
	struct	du *next;
	long	blocks;
	long	blocks30;
	long	blocks60;
	long	blocks90;
	long	nfiles;
	uid_t	uid;
	gid_t	gid;
#define	NDU	2048
} du[NDU];

int	ndu;
#define	DUHASH	8209	/* smallest prime >= 4 * NDU */
#define	HASH(u)	((u) % DUHASH)
struct	du *duhash[DUHASH];

#define	TSIZE	500
int	sizes[TSIZE];
int	gflg = 0;
int	nflg = 0;
int	fflg = 0;
int	cflg = 0;
int	vflg = 0;
int	hflg = 0;
long	now  = 0;
long	overflow = 0;
unsigned	ino = 0;

/* 
 * Function prototypes
 */
void	quotall	();
char	*malloc	();
int	check	(char *, char *);
void	acct	(struct dinode *, int);
int	qcmp	(register struct du *, register struct du *);
void	report	();
struct	dinode *ginode	();
static	free_hash_table	();
static	set_hash_table	();
char	*groupname	(gid_t);
char	*getname		(register uid_t);
static	struct entry *make_blank_entry	();
static	struct entry *inset	 	(register uid_t);
static	struct entry *place_blank_entry	(register uid_t);


main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	char ch;
	time_t time();

	setlocale( LC_ALL, "" );
	catd = catopen(MF_QUOT,NL_CAT_LOCALE);

	while ((ch = (char) getopt(argc, argv, "cfghnv")) != (char) EOF)
		switch((char)ch) {
		case 'c':
			cflg++; break;
		case 'f':
			fflg++; break;
		case 'g':		/* undocumented */
			gflg++; break;
		case 'h':	
			hflg++; break;
		case 'n':
			nflg++; break;
		case 'v':		/* undocumented */
			vflg++; break;
		case '?':
		default:
			fputs(MSGSTR(USAGE,
			      "usage: quot [-cfhn] [filesystem ...]\n"),stderr);
			exit(1);
		}
	argc -= optind;
	argv += optind;

	(void)time(&now);
	if (argc)
		for (; *argv; ++argv) {
			if (check(*argv, (char *)NULL) == 0)
				report();
		}
	else
		quotall();
	exit(0);
}

void 
quotall()
{
        struct fstab *fs;

        if (setfsent() == 0) {
                perror(FSYSname);
                exit(1);
        }
        while ((fs = getfsent()) != NULL) {
		if (strcmp(fs->fs_vfstype, "jfs"))
			continue;

		if (check(fs->fs_spec, fs->fs_file) == 0)
			report();
        }
        endfsent();
}

int 
check(char *file, char *fsdir)
{
	int 			i, c, fd;
        ino_t                   imax;
        fdaddr_t		fmax;
	struct 	du 		**dp;
	struct 	dinode 		di;
	struct 	superblock	sblock; 

	/*
	 * Initialize tables between checks;
	 * because of the qsort done in report()
	 * the hash tables must be rebuilt each time.
	 */
	for (i = 0; i < TSIZE; i++)
		sizes[i] = 0;
	overflow = 0;

	for (dp = duhash; dp < &duhash[DUHASH]; dp++)
		*dp = 0;
	ndu = 0;

        if ((fd = fsopen(file, O_RDONLY)) < 0) {
        	fprintf(stderr, "quot: ");
                perror(file);
                return(1);
        }

	sync();
        switch (get_super(fd, &sblock)) {
		case LIBFS_SUCCESS:
			break;
		default:
			fprintf(stderr, MSGSTR(CANTREAD,
			   "quot: Cannot read super block on filesystem %s.\n"),
			   file);
			return(1);
	}

	printf("%s", file);
	if (fsdir == NULL) {
		register struct fstab *fs = getfsspec(file);
		if (fs != NULL)
			fsdir = fs->fs_file;
	}
	if (fsdir != NULL && *fsdir != '\0')
		printf(" (%s)", fsdir);
	printf(":\n");

	if (nflg) {
		if (isdigit(c = getchar()))
			(void)ungetc(c, stdin);
		else while (c != '\n' && c != EOF)
			c = getchar();
	}

        /*
         * Get the max number of inodes and blocks
         */
        fsmax (&imax, &fmax);
        if ((imax == 0) || (fmax.f.addr <= 0)) {
                fprintf(stderr, MSGSTR(BADFSIZE,
                        "quot: super block has corrupted filesystem size\n"));
                return(1);
        }

        for (ino = 0; ino < imax; ino++) {
		/*
		 * We are going to skip the filesystem overhead inodes
	  	 * and process only those that exist in the filesystem
		 * namespace.
	 	 */
                if ((ino <= SPECIAL_I) && (ino != ROOTDIR_I))
			continue;

                if ((get_inode (fd,  &di, ino)) != LIBFS_SUCCESS)
                        continue;
                if (di.di_nlink > 0) {
                        /*
                         * If the mode masks off and doesn't give me a
                         * standard file type (like IFDIR, IFREG, IFBLK...),
                         * then ignore it.
                         */
                        if ((di.di_mode & IFMT) == 0)
                                continue;
			acct(&di, sblock.s_fragsize);
		}
	}
	close(fd);
	return (0);
}

void 
acct(struct dinode *ip, int fragsize)
{
	register struct du *dp;
	struct du **hp;
	long size;
	int n;
	static fino;

	/*
	 * By default, take block count in inode.  Otherwise (-h),
	 * take the size field and estimate the blocks allocated.
	 * The latter does not account for holes in files.
	 */
	if (!hflg)
		size = (ip->di_nblocks * fragsize) / BLOCK;
	else
	{
		/* 
		 * If my size field indicates that I am occupying a partial
		 * block, then any division will be off by 1 block.
		 * Take the number of blocks that I should encompass and 
		 * multiply it by the fragment size, then switch it to my
		 * recording size.
		 * Otherwise I am occupying an exact block and just switch
		 * it to the recording size.
		 */
		if (ip->di_size % fragsize)
			size =((ip->di_size / fragsize + 1) * fragsize) / BLOCK;
		else
			size = ip->di_size / BLOCK;

	}


	if (cflg) {
		if ((ip->di_mode&IFMT) != IFDIR && (ip->di_mode&IFMT) != IFREG)
			return;

		if (size >= TSIZE) {
			overflow += size;
			size = TSIZE-1;
		}
		sizes[size]++;
		return;
	}
	hp = &duhash[HASH(ip->di_uid)];
	for (dp = *hp; dp; dp = dp->next)
		if (dp->uid == ip->di_uid && (!gflg || dp->gid == ip->di_gid))
			break;
	if (dp == 0) {
		if (ndu >= NDU)
			return;
		dp = &du[ndu++];
		dp->next = *hp;
		*hp = dp;
		dp->uid = ip->di_uid;
		dp->gid = ip->di_gid;
		dp->nfiles = 0;
		dp->blocks = 0;
		dp->blocks30 = 0;
		dp->blocks60 = 0;
		dp->blocks90 = 0;
	}
	dp->blocks += size;
#define	DAY (60 * 60 * 24)	/* seconds per day */
	if (now - ip->di_atime > 30 * DAY)
		dp->blocks30 += size;
	if (now - ip->di_atime > 60 * DAY)
		dp->blocks60 += size;
	if (now - ip->di_atime > 90 * DAY)
		dp->blocks90 += size;
	dp->nfiles++;
	while (nflg) {
		register char *np;

		if (fino == 0)
			if (scanf("%d", &fino) <= 0)
				return;
		if (fino > ino)
			return;
		if (fino < ino) {
			while ((n = getchar()) != '\n' && n != EOF)
				;
			fino = 0;
			continue;
		}
		if (np = getname(dp->uid))
			printf("%.7s\t", np);
		else
			printf("%u\t", ip->di_uid);
		if (gflg) {
			if (np = groupname(dp->gid))
				printf("%.7s\t", np);
			else
				printf("%u\t", ip->di_gid);
		}
		while ((n = getchar()) == ' ' || n == '\t')
			;
		putchar(n);
		while (n != EOF && n != '\n') {
			n = getchar();
			putchar(n);
		}
		fino = 0;
		break;
	}
}


int 
qcmp(register struct du *p1, register struct du *p2)
{
	char *s1, *s2;

	if (p1->blocks > p2->blocks)
		return (-1);
	if (p1->blocks < p2->blocks)
		return (1);
	s1 = getname(p1->uid);
	if (s1 == 0)
		return (0);
	s2 = getname(p2->uid);
	if (s2 == 0)
		return (0);
	return (strcmp(s1, s2));
}

void 
report()
{
	register i;
	register struct du *dp;

	if (nflg)
		return;
	if (cflg) {
		register long t = 0;

		for (i = 0; i < TSIZE - 1; i++)
			if (sizes[i]) {
				t += i*sizes[i];
				printf("%d\t%d\t%ld\n", i, sizes[i], t);
			}
		printf("%d\t%d\t%ld\n",
		    TSIZE - 1, sizes[TSIZE - 1], overflow + t);
		return;
	}
	qsort(du, ndu, sizeof (du[0]), qcmp);
	for (dp = du; dp < &du[ndu]; dp++) {
		register char *cp;

		if (dp->blocks == 0)
			return;
		printf("%5d\t", dp->blocks);
		if (fflg)
			printf("%5d\t", dp->nfiles);
		if (cp = getname(dp->uid))
			printf("%-8.8s", cp);
		else
			printf("#%-8d", dp->uid);
		if (gflg) {
			if (cp = groupname(dp->gid))
				printf(" %-8.8s", cp);
			else
				printf(" #%-8d", dp->gid);
		}
		if (vflg)
			printf("\t%5ld\t%5ld\t%5ld",
			    dp->blocks30, dp->blocks60, dp->blocks90);
		printf("\n");
	}
}

#include <grp.h>
#include <utmp.h>

struct	utmp utmp;

#define NGID	2048L
#define	NMAX	(sizeof (utmp.ut_name))

char	gnames[NGID][NMAX+1];
char	outrangename[NMAX+1];
gid_t	outrangegid = -1;

char *
groupname(gid_t gid)
{
	register struct group *gr;
	static init;

	if (gid < (unsigned)NGID && gnames[gid][0])
		return (&gnames[gid][0]);
	if (gid == outrangegid)
		return (outrangename);
rescan:
	if (init == 2) {
		if (gid < NGID)
			return (0);
		setgrent();
		while (gr = getgrent()) {
			if (gr->gr_gid != gid)
				continue;
			outrangegid = gr->gr_gid;
			strncpy(outrangename, gr->gr_name, NMAX);
			endgrent();
			return (outrangename);
		}
		endgrent();
		return (0);
	}
	if (init == 0) {
		setgrent();
		init = 1;
	}
	while (gr = getgrent()) {
		if (((signed)gr->gr_gid < 0L) || ((signed)gr->gr_gid >= NGID)) {
			if (gr->gr_gid == gid) {
				outrangegid = gr->gr_gid;
				strncpy(outrangename, gr->gr_name, NMAX);
				return (outrangename);
			}
			continue;
		}
		if (gnames[gr->gr_gid][0])
			continue;
		strncpy(gnames[gr->gr_gid], gr->gr_name, NMAX);
		if (gr->gr_gid == gid)
			return (&gnames[gid][0]);
	}
	init = 2;
	goto rescan;
}

/*
 * getname:
 *	purpose:
 *		getname returns the user from the password file
 *		associated with a specified user-id. The user-id
 *		is passed as an integer parameter to the routine.
 *
 *	If a user whose id matches the parameter uid is found in the
 *	password file, the name corresponding to that user is returned.
 *	If no match is found, NULL is returned.  If malloc() fails,
 *	NULL is returned.  If id is -1, all allocated memory is freed.
 *
 *	During the search for a particular user-id, a hash table of names
 *	is built for storing user-id's and their corresponding names
 *	from the password file. When the routine is first called, entries
 *	are read from the password file until a match for uid, or the
 *	end of the password file, is found. Any entries not matching uid
 *	and not already stored in the names structure are then stored in
 *	names.
 *
 *	On subsequent calls to getname, the structure names is checked
 *	first for a match for uid. If no match is found, and  if the entire
 *	password file has not been stored, entries from it are stored in
 *	names as before.
 *
 *	When the entire password file has been stored, only names
 *	is checked for a matching user-id on subsequent calls to getname.
 */

#include <pwd.h>			/* passwd struct include file */
#include <utmp.h>			/* utmp struct include file */
#include <stdio.h>

#define HASHBITS 6			/* number of bits to hash */
#define HASHSIZE (1<<HASHBITS)		/* limit on the number of buckets
					   in the hash table */
#define HASHMASK (HASHSIZE-1)		/* mask of bits to hash */
#define hash(uid) ((uid)&HASHMASK)	/* determines in which of the HASHSIZE
					   buckets uid would belong */
static struct utmp ut;

/* struct for storing an entry of the hash table */
struct entry {
	char a_name[NMAX+1];		/* stores the name of the user  */
	uid_t a_uid;			/* stores the uid of the user   */
	struct entry *next;		/* stores the pointer to the
					   next entry of the hash table */
};

static struct entry *names[HASHSIZE];	/* the hash table of users */

/* 
 * Function: inset()
 *
 * Purpose:  Parses the hash table looking for the specified uid.  
 *
 * Returns:  Entry corresponding to the uid is returned if it exists, 
 *	     otherwise it returns NULL
 */
static struct entry *
inset(register uid_t uid)
{
	register struct entry *temp;

	for (temp = names[hash(uid)]; temp != NULL; temp = temp->next)
		if (temp->a_uid == uid)
			return(temp);
	return(NULL);
}

/* allocates space for an entry in names, setting next field to NULL */
static struct entry *
make_blank_entry()
{
	register struct entry *blank_entry;

	blank_entry = (struct entry*)(malloc(sizeof(struct entry)));
	if (blank_entry == NULL)
		return(NULL);
	blank_entry->next = NULL;
	return(blank_entry);
}

/* inserts a blank entry into the correct position of names for a user
   whose id is uid */
static struct entry *
place_blank_entry(register uid_t uid)
{
	register struct entry **temp;

	temp = &names[hash(uid)];
	while (*temp != NULL)
		temp = &((*temp)->next);
	return(*temp = make_blank_entry());
}

/* inserts the data of an entry from the password file (stored in pw_entry)
   into an entry of names (the parameter blank_entry) */
static
fill_in(register struct entry *blank_entry, register struct passwd *pw_entry)
{
	strncpy(blank_entry->a_name, pw_entry->pw_name, NMAX);
	blank_entry->a_name[NMAX] = '\0';
	blank_entry->a_uid = pw_entry->pw_uid;
}

/* Function: create_names_entry()
 * 
 * Purpose: Creates an entry in the names hash table and stores the 
 *  	    data from the password file in the parameter passwd_entry 
 */
static struct entry *
create_names_entry(register struct passwd *passwd_entry)
{
	register struct entry *new_entry;
	int fill_in(register struct entry *, register struct passwd *);

	new_entry = place_blank_entry(passwd_entry->pw_uid);
	if (new_entry == (struct entry *) NULL)
		return((struct entry *) NULL);
	fill_in(new_entry, passwd_entry);
	return(new_entry);
}

/* initializes hash table */
static 
set_hash_table()
{
	register int i;

	for (i = 0; i < HASHSIZE; i++)
		names[i] = NULL;
}

/* Function: free_hash_table()
 *
 * Purpose: Parses the hash table of names and frees all allocated entries.
 */
static 
free_hash_table()
{
	register int i;
	register struct entry *temp;

	for (i = 0; i < HASHSIZE; i++)
		while ((temp = names[i]) != NULL) {
			names[i] = temp->next;
			free((char *)temp);
		}
}

/* returns the name of the user in the passwords file whose id is uid.
   NULL is returned if none exists */
char *
getname(register uid_t uid)
{
	register struct passwd *pw;	/* pre-defined struct */
	struct entry *create_names_entry(register struct passwd *passwd_entry);
	static init;			/* indicates status of the password file
						0 = file unopen
						1 = file open
						2 = file entirely read
					 */
	/* pre-defined routines for accessing the password routine */
	struct passwd *getpwent();
	register struct entry *Location;

	if (uid == -1) {
		if (init != 0)
			free_hash_table();
		if (init == 1)
			endpwent();
		init = 0;
		return(NULL);
	}

	if (init == 0) {
		set_hash_table();
		setpwent();
		init = 1;
	}

	Location = inset(uid);		/* check if user is in names */
	if (Location != NULL)
		return(Location->a_name); /* user already stored in names */
	if (init == 2)
		return(NULL);		/* entire password file has been
					   stored in names */

       /* continue reading entries from the password file, storing any in
	  names whose uid is not already located in names, stopping when
	  a match is found for the uid or the entire password file has
	  been stored */

	while ((pw = getpwent()) != NULL) {
		Location = inset(pw->pw_uid);
		if (Location != NULL)
			continue;
		if (create_names_entry(pw) == (struct entry *) NULL)
			return(NULL);
		if (pw->pw_uid == uid)
			return(pw->pw_name);
	}
	init = 2;
	endpwent();
	return(NULL);
}
