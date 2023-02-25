static char sccsid[] = "@(#)44	1.32.3.9  src/bos/usr/sbin/restore/restore.c, cmdarch, bos411, 9428A410j 4/8/94 12:17:14";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: restore
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This is the restore front end */

#define _ILS_MACROS

#include <nl_types.h>
#include "restore_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_RESTORE,N,S)
nl_catd catd;

#include <stdio.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <sys/tape.h>
#include <sys/ioctl.h>
#include <sys/termio.h>
#include <sys/stat.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/sysmacros.h>
#include <sys/limits.h>
#include <sys/param.h>
#include <dumprestor.h>
#include <errno.h>
#include <stdlib.h>
#include <locale.h>
#include <math.h>
#include "util.h"
#include <ctype.h>

#define SNGL_VOL_MNT "Please mount volume %d on %s\n...\
 and press Enter to continue\n"

#define MULT_VOL_MNT "Please mount volumes %d-%d on %s-%d\n...\
 and press Enter to continue\n"

#define TAPE_DEVICE (devinfo.devtype == DD_SCTAPE)
#define FIXED_BLK_MODE (devinfo.un.scmt.blksize > 0)
#define DEF_BLKSIZE 512

#define OPTSIZE 3
#define OPTIONS "?ABhmvyf:b:s:tTxX:rRdipqM"

char ByNameProgram[12] = "restbyname";
char ByInodeProgram[13] = "restbyinode";
int byname, byinode, bflag, qflag, pflag, Bflag, Xflag, volnum;
int Rflag;
int tty = 0;
char *media;
char *setmedia();
int skipnum;
int nblocks;

struct devinfo devinfo;

void waitret(void);
void peekmedia(char *,int);

main(argc, argv)
int argc;
char *argv[];
{
	/* parse command line and construct a vector as argv for
	 * the backend.  We need to peek at the tape to determine
	 * the type of backup that is stored.  If old format then
	 * we exec the old restore program, else we invoke the new
	 * Berkeley based program.
	 */
	
	char **av;
	int ac;
	int i, j;
	int c;
	char **arg_vec;
	char Backend[PATH_MAX];
	char string[PATH_MAX];
	char *blocks=NULL;
	char numbuf[7];
	int xflag = 0, Mflag = 0;

	(void) setlocale(LC_ALL,"");
	catd = catopen("restore.cat", NL_CAT_LOCALE);

	ac = argc;
	av = argv;
	byname = byinode = skipnum = bflag = pflag = qflag = Xflag = Rflag = 0;
	nblocks = DEF_CLUSTER;
	volnum = 1;
	c = 0;
	media = setmedia(ac, av);

	/* if user indicated that input was to come from stdin, then
	 * we don't peek at the media.  We use the flags given to 
	 * determine the type of backup format.  For Berkeley, the
	 * 'B' flag has to be given.
	 */
	if ((strcmp(media, "-")) && !Rflag && !Xflag && !is_namepipe(media)) {
		/* peekmedia sets the global flag of byname or byinode */
		(void)peekmedia(media, skipnum);
	}else
		if (Bflag || Rflag) 
			byinode = 1;
		else 
			byname = 1;

	arg_vec = (char **)calloc((size_t)(argc+20), (size_t)sizeof (char **));
	j = 1;
	while ((c = getopt(argc, argv, OPTIONS)) != EOF) {
		switch(c) {
	        /* These options apply to both backends */ 
		case 'b':
			/* work has been done by peekmedia(), and this 
			 * option will be put into the arg_vec before 
			 * passing it to the backend. 
			 */
			j--;
			break;
		case 'x':
			xflag = 1;
		case 'r':
		case 't':
		case 'T':
		case 'v':
		case 'q':
		case 'f':
			if (byinode && c == 'T') c = 't';
			if (byinode && c == 'q') {
				--j;
				break;
			}
			arg_vec[j] = (char *)malloc((size_t)OPTSIZE);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			if (c == 'f') {
				++j;
				arg_vec[j] = optarg;
			}
			break;
		/* These options are only for restbyinode. */
		case 'R':
		case 'h':
		case 'm':
		case 'y':
		case 'i':
			if (byname) 
				conflict(c);
			arg_vec[j] = (char *)malloc((size_t)OPTSIZE);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			break;
		case 'B':
			if (byname) 
				conflict(c);
			arg_vec[j] = (char *)malloc((size_t)OPTSIZE+1);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = 'f';
			arg_vec[j][2] = '-';
			arg_vec[j][3] = '\0';
			break;
		/* These options are only for restbyname. */
		case 'X':
			Xflag = 1;
		case 'A':
		case 'p':
		case 'M':
		case 'd':
			if (byinode)
				conflict(c);
			if (c == 'M') {
				c = 'h';
				Mflag = 1;
			}
			arg_vec[j] = (char *)malloc((size_t)OPTSIZE);
			arg_vec[j][0] = '-';
			arg_vec[j][1] = c;
			arg_vec[j][2] = '\0';
			if (c == 'X') {
				++j;
				arg_vec[j] = optarg;
			}
			break;
		case 's':
		/* work has been done by peekmedia */
			j--;
			break;
		case '?':
		default:
			usage();
			break;
		}
		++j;
	}

	if (Mflag && (!xflag && !Xflag)) {
		fprintf(stderr, MSGSTR(MFLAG, 
			"Specify -x or -X when using -M.\n"));
		usage();
	}

	/* if the media is tape or user specified a number of blocks
	 * with the -b option then pass nblocks which has already
	 * been initialized in peekmedia() to the backend.
	 */
	if (TAPE_DEVICE) {
		arg_vec[j]=(char *)malloc((size_t)OPTSIZE);
		arg_vec[j][0]='-';
		arg_vec[j][1]='b';
		arg_vec[j][2]='\0';
		j++;
		sprintf(numbuf,"%d",nblocks);
		arg_vec[j] = numbuf;
		j++;
	}

	/* since we have already looked at the media, pass the -q 
	 * option to the backend so the user doesn't get prompted
	 * a second time.
	 */
	if (byname && !qflag) {
		arg_vec[j] = (char *)malloc((size_t)OPTSIZE);
		arg_vec[j][0] = '-';
		arg_vec[j][1] = 'q';
		arg_vec[j][2] = '\0';
		j++;
	}

	/* now copy all the filenames specified on the command line */
	for (; optind < argc; optind++) {
		arg_vec[j++] = argv[optind];
	}
	arg_vec[j] = NULL;

	if (byname)
		arg_vec[0] = ByNameProgram;
	else 
		arg_vec[0] = ByInodeProgram;
	sprintf(Backend, "%s/%s", "/usr/sbin", arg_vec[0]);

	execv(Backend, arg_vec);
	sprintf(string, MSGSTR(EXECF, "Exec of %s failed"), Backend);
	perror(string);
	exit(1);
}

conflict(c)
char c;
{
	if (byname)
		fprintf(stderr, MSGSTR(BADOP1, 
		"Option %c not available with by name backups.\n"), c);
	else
		fprintf(stderr, MSGSTR(BADOP2, 
		"Option %c not available with by inode backups.\n"), c);
	exit(1);
}

usage()
{
	fprintf(stderr, MSGSTR(USAGE, "For by name backups:\n"));
	fprintf(stderr, MSGSTR(USAGE1, "\trestore -x[vqMd] [-f device] [-s number] [-b number] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE2, "\trestore -T|-t [-qv] [-f device] [-s number] [-b number]\n"));
	fprintf(stderr, MSGSTR(USAGE3, "\trestore -X number [-Mdqv] [-f device] [-s number] [-b number] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE4, "For version 2 inode backups:\n"));
	fprintf(stderr, MSGSTR(USAGE5, "\trestore -r[d] [-f device] [-s number] [-b number] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE6, "For inode backups:\n"));
	fprintf(stderr, MSGSTR(USAGE7, "\trestore -t[hqvyB] [-f device] [-s number] [-b number] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE8, "\trestore -x[hmqvyB] [-f device] [-s number] [-b number] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE9, "\trestore -i[hmqvy] [-f device] [-s number] [-b number]\n"));
	fprintf(stderr, MSGSTR(USAGE10, "\trestore -r[qvyB] [-f device] [-s number] [-b number]\n"));
	fprintf(stderr, MSGSTR(USAGE11, "\trestore -R[vyB] [-f device] [-s number] [-b number]\n"));
	exit(1);
}


/* Function: int get_media(const char *, char *)
 *
 * Description: This function determines the media
 * device and prompts, if necessary, the user for
 * said media.
 *
 * Return value: The function returns a valid
 * file descriptor for the specified media device.
 *
 * Side effects: The devname parameter may be 
 * changed by this function.  If devname is a range of 
 * devices (ie,/dev/rfd0-4) then devname will be changed 
 * to the first device in the range (ie,/dev/rfd0).
 */
int
get_media(const char *media, char *devname)
{
        int range = 0;
	short notready;
	short prompt;
	char *devptr;
        int max_unit, min_unit;
	int fd;

        notready = 1;
        devname[0] = '\0';
        prompt = qflag;

        if (media == (char *)NULL)
                strcat(devname, "/dev/rfd0");
        else
                if (((devptr = strchr(media, '-')) != NULL) &&
                    isdigit(*(devptr-1)) && isdigit(*(devptr+1)))  {
                        range = 1;
                        strncat(devname, media, devptr - media);
                        max_unit = atoi(devptr+1);
			devptr--;
                        while(isdigit(*devptr))
				devptr--;
                        min_unit = atoi(devptr+1);
                } else
                        strcpy(devname, media);

        /* loop until we get a valid media */
        while (notready) {
                if (!prompt) {
                        if (!range)
                                fprintf(stderr,
                                MSGSTR(MNTV, SNGL_VOL_MNT), volnum, devname);
                        else
                                fprintf(stderr, MSGSTR(MNT, MULT_VOL_MNT),
                                volnum, volnum+(max_unit-min_unit), devname, max_unit );
                        (void)waitret();
                }
                if ((fd = open(devname, O_RDONLY, 0)) < 0) {
                        fprintf(stderr,MSGSTR(NODRIVE,
                                "cannot open %s: "), devname);
                        perror("");
                        prompt = 0;
                }else
                        notready = 0;
        }
	return(fd);
}


/* Function: void get_devinfo(int, const char *)
 *
 * Description: This function tries to get information
 * on the specified device.  It does this by issuing an 
 * ioctl() IOCINFO. If the ioctl() fails, fstat() is used
 * to determine if we are trying to restore from a regular 
 * file.
 *
 * Return value: The function has no explicit return
 * value. It does exit if an error occurs.
 *
 * Side effects: The global struct devinfo is initialized.
 */
void
get_devinfo(int fd, const char *devname)
{
	struct stat statbuf;

        if (ioctl(fd, IOCINFO, &devinfo) < 0) {
                if (fstat(fd, &statbuf) < 0) {
                        fprintf(stderr, MSGSTR(STATFAIL, "Can't get info on %s\n"), devname);
                        exit(1);
                }
                /* don't complain if the user is restoring from a disk file */
                if ((statbuf.st_mode & (S_IFREG|S_IFLNK)) == 0) {
                        fprintf(stderr, MSGSTR(IOCTLE,
                                "Can't get device info\n"));
                        exit (1);
                }
        }
}


/* Function: void reset_tapehead(int, int)
 *
 * Description: This function will put the tapehead
 * back at the beginning of the archive that was
 * specified with the -s option.  The -s option
 * requires that the user specify a no-rewind 
 * no-retension tape device.
 *
 * Return value: The function has no explicit return
 * value. The function will exit if an error occurs.
 *
 * Side effects: None.
 */
void
reset_tapehead(int fd, int skipnum)
{
        int at_bot= 0;
        struct stop op;

	if (skipnum || pflag) {
		op.st_op = STRSF;
		op.st_count = 1;
		if (ioctl(fd, STIOCTOP, &op) < 0) {
			/* EIO means we're at beginning of tape 
			 * and that's okay, any other error is bad.
			 */
			if (errno != EIO) {
				fprintf(stderr, MSGSTR(IOCTLT,
				"Can't skip tape records\n"));
				exit(1);
			} else
                                /* tell the get_on_eot_side_of_filemark
                                 * kludge that follows this that we are at
                                 * beginning of tape.
                                 */
                                        at_bot++;
		}

		/* GET_ON_EOT_SIDE_OF_FILEMARK KLUDGE
	 	 * this kludge is here because the above reverse 
		 * skip file leaves the tape head on the beginning
		 * of tape (bot) side of the file marker, which will
		 * cause the next read to get bad data. So we have to
		 * issue a skip file forward(STFSF) to get the tape 
		 * head on the end of tape (eot) side of the file
		 * mark where it should be. However, we don't need to
		 * do all this if we are at the bot.
		 * Pretty bad huh?
		 */
		if (!at_bot) {
			op.st_op = STFSF;
			op.st_count = 1;
			if (ioctl(fd, STIOCTOP, &op) < 0) {
				fprintf(stderr, MSGSTR(IOCTLT,
				"Can't skip tape records\n"));
				exit(1);
			}
		}
	}
}


/* Function: void set_nblocks(void)
 *
 * Description: This function sets the global variable
 * nblocks to a valid number of blocks based on the
 * the physical tape blocksize when the tape is in fixed
 * block mode and/or the user specified the number of 
 * blocks with the -b option.
 * For variable block mode, nblocks will be either the
 * default number of blocks (100) or a user specified
 * value.  There is no validation of nblocks when the
 * tape device is in variable block mode.
 *
 * Return value: The function returns no explicit value,
 * and does exit with a non-zero value on error.
 *
 * Side effects: The global variable nblocks is set to
 * a valid value.
 */
void
set_nblocks(void)
{
	if (FIXED_BLK_MODE) {
		int bsize;

		bsize = devinfo.un.scmt.blksize;

		/* if the current tape blocksize a multiple of
		 * the blocksize we are trying to read at then 
		 * we can just return
		 */
		if (((nblocks * DEF_BLKSIZE) % bsize) == 0)
			return;

		/* if we can get a valid number of blocks by
		 * mulitplying the blocksize by a factor of 100
		 * (DEF_CLUSTER) and dividing by DEF_BLKSIZE then return
		 */
		nblocks = (DEF_CLUSTER * bsize) / DEF_BLKSIZE;
		if (nblocks <= ctob(MAXBLK)) {
			if (bflag)
                        	printf(MSGSTR(CHBLKSZ, 
					"Changing user specified number of blocks to %d\n"),nblocks);
			return;
        	}

		/* if the number of blocks in blocksize is
		 * valid (ie, less than the max) then return
		 */
		nblocks = bsize / DEF_BLKSIZE;
		if (nblocks <= ctob(MAXBLK)) {
			if (bflag)
                    	    printf(MSGSTR(CHBLKSZ, 
				"Changing user specified number of blocks to %d\n"),nblocks);
			return;
		}
		
		/* the current tape blocksize is larger than
		 * restore can read so print error message and exit
		 */
		fprintf(stderr,MSGSTR(BIGBLK,
			"blocks required (%d) to read tape blocksize of (%d)\n\t exceeds maximum number of blocks (%d) allowed by restore.\n"),
			bsize/DEF_BLKSIZE,bsize,ctob(MAXBLK));
		exit(1);
	}
}


/* Function: void peekmedia(char *, int)
 *
 * Description: Peekmedia determines what type of device 
 * the medium resides on and then does a read to determine
 * if the archive format is in byname or byinode format.
 *
 * Return value: The function returns no explicit value,
 * and does exit with a non-zero value on error.
 *
 * Side effects: One of the global variables, byname or 
 * byinode, is set to 1. The global struct devinfo is
 * initialized.
 */
void
peekmedia(char *media, int skipnum)
{
	int fd;
	char *tapebuf;	
	struct stop op;
	int loc, ext;
	char devname[PATH_MAX+1];
	char *tmp;
	int readsize, bufsize;

	readsize = bufsize = 0;

	fd = get_media(media, &devname);

	get_devinfo(fd, devname);

	switch (devinfo.devtype) {
	case DD_TAPE:
	case DD_SCTAPE:
		if ((tmp = strrchr(devname,'.')) == NULL)
			ext = 0;
		else {
			tmp++;
			ext = *tmp - '0';
		}
		if ((ext == 0) || (ext == 4)) {
		 	/* If the user specifed -s option without 
		 	 * specifying a no-rewind device then tell
			 * them of their mistake and exit.
		 	 */
			if (skipnum) {
				fprintf(stderr, MSGSTR(NOREW,
				"A no-rewind, no-retension tape device must be used with the -s option."));
				exit(1);
			}
		}else if ((ext == 1) || (ext == 5)) {
			/* if user specified a no-rewind tape device without 
			 * the -s option then default skipnum to 1.
			 */
			if (!skipnum)
				skipnum = 1;
		} else {
			fprintf(stderr, MSGSTR(BADREW,
				"The no-rewind tape device specified is not valid.\n"));
			exit(1);
		}
				
		/* If skip option specified, then get the tape head to the
		 * beginning of the specified archive by using file skip
		 * forward (STFSF) and ioctl().  
		 */
		if (skipnum) {
			op.st_op = STFSF;
			op.st_count = skipnum - 1;
			if (ioctl(fd, STIOCTOP, &op) < 0) {
				fprintf(stderr, MSGSTR(IOCTLT, 
					"Can't skip tape records\n"));
				exit(1);
			}
		}

		set_nblocks();

		/* For fixed block mode, define the read buffer 
		 * size to be the physical tape block size.
		 * For variable mode, define the read buffer size
		 * to be DEF_BLKSIZE.
		 */
		if (FIXED_BLK_MODE)
			bufsize = devinfo.un.scmt.blksize;
		else
			bufsize = DEF_BLKSIZE;

		if ((tapebuf = (char *)malloc(bufsize)) == NULL) {
			fprintf(stderr, MSGSTR(NOMEM,
				"Not enough memory available at this time.\n"));
			exit(1);
		}

		/* Read the first block from the tape, which 
		 * should contain the backup volume header.
		 */
		if (FIXED_BLK_MODE)
			readsize = read(fd, tapebuf, bufsize);
		else
			readsize = readx(fd, tapebuf, bufsize, TAPE_SHORT_READ);

		if (readsize < 0) {
			fprintf(stderr, MSGSTR(READERR, 
				"Media error: can't read backup\n\t "));
			perror(devname);
			exit(1);
		}

		if (readsize == 0) {
			fprintf(stderr, MSGSTR(EOFERR, "unexpected EOF\n"));
			exit(1);
		}

		/* Since the backend, which is exec'd, opens 
		 * the tape device and expects that the tape
		 * head is at the beginning of the archive,
		 * we have to put the tape head back where
		 * it was before we open'd it and read().
		 */
		reset_tapehead(fd,skipnum);
		break;
	default:

		/* For any other media we can just do 
		 * a read() and close the file.
	 	 */
		tapebuf = (char *)malloc((size_t)TP_BSIZE);
		if (skipnum) {
			fprintf(stderr, MSGSTR(SKIPE, 
				"skip option only available for tape\n"));
			exit(1);
		}
		/* get our current position. */
		loc = lseek(fd, 0, 1);
		/* Before giving up when or if the read fails 
		 * with TP_BSIZE (1024) then seek back to previous 
		 * read location and try again with UBSIZE (user 
		 * blocksize of 512).
		 */
		if (read(fd, tapebuf, TP_BSIZE) < 0) {
			lseek(fd, loc, 0);
			if (read(fd, tapebuf, UBSIZE) < 0) {
				fprintf(stderr, MSGSTR(READERR, 
					"Media error: can't read backup\n\t "));
				perror(devname);
				exit(1);
			}
		}
		break;
	}
	/* Now look at the block just read and determine if the 
	 * archive is in byname or byinode format.
	 */
	switch(checkmedia(tapebuf)) {
	case 0: 
		byinode++;
		break;
	case 1: 
		byname++;
		break;
	default:  
		fprintf(stderr, MSGSTR(BADFMT,
			"The archive is not in backup format.\n"));
		exit(1);
	}

	close(fd);
}

#undef spcl
/* Function: int checkmedia(char *bufp)
 * 
 * Description: This function tries to determine if the first
 * block of archive data contains a recognizable byname or 
 * byinode MAGIC number.
 *
 * Return value: Returns 1 if the medium is in byname format, 
 * 0 if in byinode (Berkeley dump) format, or -1 otherwise.
 *
 * Side effects: None.
 */
int
checkmedia(char *bufp)
{
	ushort  magic;   /* magic number to look for */
	ushort  pmagic;  /* packed magic number to look for */
	union u_spcl *i_ptr;
	union fs_rec *n_ptr;

        /* "magic" is the magic number in the byte order
         * that it will be on medium.
         */
	magic = MAGIC;
	wshort(magic);
	pmagic = PACKED_MAGIC;
	wshort(pmagic);

	i_ptr = (union u_spcl *)bufp;
	n_ptr = (union fs_rec *)bufp;
	if (i_ptr->s_spcl.c_magic == (int)NFS_MAGIC && i_ptr->s_spcl.c_volume == 1)
		return(0);
	if ((n_ptr->v.h.magic == magic) || (n_ptr->v.h.magic == pmagic))
		return(1);
	return(-1);
}


/*
 * setmedia scans the argument list to see if -f was asserted, if so
 * then set the media to that, else return NULL.
 */
char *
setmedia(int ac, char **av)
{
	int c;
	int fflag;
	char *medium;

	fflag = 0;
	while ((c = getopt(ac, av, "?ABhmvyf:b:s:tTxX:rRdipqM")) != EOF) {
		switch(c) {
		case 'b':
			bflag++;
			nblocks = atoi(optarg);
                        /*
                        * Check for max cluster size.
                        * Actually, this check is overkill
                        * since MAXBLK applies only to
                        * transfers to raw devices. ctob(X)
                        * works out to be (X * 4096).
                        */
                        if(nblocks * DEF_BLKSIZE > ctob(MAXBLK))
                        {
                                int xclus = nblocks;
                                nblocks = (ctob(MAXBLK))/DEF_BLKSIZE;
                                fprintf(stderr, MSGSTR(TOOLRG,
                                "cluster size %d too large, reduced to %d\n"),
                                xclus, nblocks);
                        }
                        if(nblocks < (BSIZE / DEF_BLKSIZE))
                        {
                                int xclus = nblocks;
                                nblocks = (BSIZE / DEF_BLKSIZE);
                                fprintf(stderr, MSGSTR(TOOSML,
                                "cluster size %d too small, enlarged to %d\n"),
                                xclus, nblocks);
                        }

			break;
		case 'f':
			fflag = 1;
			medium = optarg;
			break;

			/* Bflag indicates that input is from stdin and the 
			 * archive is in Berkeley dump format.  
			 */
		case 'B':
			Bflag = 1;
			Xflag = 1;
			break;
		case 'X':
			volnum = atoi(optarg);
			Xflag = 1;
			break;
		case 'q':
			qflag = 1;
			break;
		case 'p':
			pflag++;
			break;
		case 'R':
			Rflag++;
			break;
		case 's':
			skipnum = atoi(optarg);
			if (skipnum <= 0 || skipnum > 100) {
				fprintf(stderr, MSGSTR(SKIPINV, 
					"invalid skip number specified\n"));
				exit(1);
			}
			break;
		case '?':
			usage();
		default:
			break;
		}
	}
	if (Bflag && fflag && (*medium != '-'))
		usage();
	optind = 1;
	opterr = 1;
	return(fflag ? medium : (char *)NULL);
}


/* Function: void waitret(void)
 *
 * Description: This function waits for the user
 * to hit the enter key.  It is called after the
 * user has been prompted to mount media and hit
 * enter to continue.
 *
 * Return value: No explict return value.  If
 * an error occurs while reading from stdin the
 * function exits with a non-zero value.
 * 
 * Side effects: None.
 */
void
waitret(void)
{
	char ans[20];

	if (tty != -1) {
		ioctl(tty, TCFLSH, 0);
		if (read(tty, ans, sizeof(ans)) == 0) {
			fprintf(stderr, MSGSTR(NOSTDIN,
			"cannot read from stdin\n"));
			exit(1);
		}
	}
}


/*
 * We have to treat sockets and named pipes the same way
 * that stdin is treated, i.e., do NOT do a peekmedia
 */
is_namepipe(char *media)
{
    struct stat sbuf;

    if (media == (char *)NULL) return 0;
    /* if can't stat the media, then let peekmedia handle the error */
    if (stat(media, &sbuf) < 0) return 0;
    if (S_ISFIFO(sbuf.st_mode) || S_ISSOCK(sbuf.st_mode)) return 1;
    return 0;
}
