static char sccsid[] = "@(#)23	1.6  src/bos/usr/sbin/trace/trc_test.c, cmdtrace, bos411, 9428A410j 6/11/91 09:23:18";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: trc_test, snap
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FUNCTION: do these subcommands to support sitb testing:
 * trchk  count                 Do trchk  system call 'count' times
 * trchkl count                 Do trchkl system call 'count' times
 * trchkg count                 Do trchkg system call 'count' times
 * trcgen count                 Do trcgen system call 'count' times
 * read file count              Read first count words from file
 * tfilt                        Zero out timestamps in a trcfile
 */

/*
 * This code is not part of any option of the supported trace commands.
 * This code will soon be moved to sitb source tree.
 * For that reason, no messages from this file will be added to the 
 * cmdtrace message catalog and no ILS function will be added.
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/trcctl.h>
#include <sys/trchkid.h>
#include "trace.h"

extern optind;
extern char *optarg;

static char *Subcommand;
static int Argc;
static char **Argv;
static unsigned Start;
static unsigned Count;
static unsigned Hook;

trc_test(argc,argv)
char *argv[];
{

	if(argc < 2)
		trc_test_usage();
	Subcommand = argv[1];

	Argc = argc-1;
	Argv = argv+1;
	if(streq(Subcommand,"trchk"))
		s_trchk();
	if(streq(Subcommand,"trchkl"))
		s_trchkl();
	if(streq(Subcommand,"trchkg"))
		s_trchkg();
	if(streq(Subcommand,"trcgen"))
		s_trcgen();
	if(streq(Subcommand,"read"))
		s_trcread();
	if(streq(Subcommand,"tfilt"))
		s_tfilt();
	if(streq(Subcommand,"panic"))
		s_panic();
	trc_test_usage();
}

static trc_test_usage()
{

	cat_eprint(0,"\
usage: trc_test subcommand subargs\n\
\n\
subcommands:\n\
 trchk  -n start count        Do trchk  system call 'count' times\n\
 trchkl -n start count        Do trchkl system call 'count' times\n\
 trchkg -n start count        Do trchkg system call 'count' times\n\
 trcgen -n start count        Do trcgen system call 'count' times\n\
 read file -n start count     Read first count words from file\n\
 tfilt                        Zero out timestamps in a trcfile\n\
");
	exit(1);
}

static trchk_getargs()
{
	int c;

	while((c = getopt(Argc,Argv,"n:")) != EOF) {
		switch(c) {
		case 'n':
			Start = strtol(optarg,0,16);
			break;
		case '?':
			trchk_usage();
		}
	}
	if(optind == Argc)
		trchk_usage();
	Count = atoi(Argv[optind]);
	Hook = Start == 0 ? 1 << 20 : (Start % 4096) << 20;
}

static trchk_usage()
{

	cat_eprint(0,"usage: %s count\n",Subcommand);
	exit(1);
}

static s_trchk()
{
	int i;

	trchk_getargs();
	for(i = 0; i < Count; i++)
		TRCHK(Hook | i & 0xFFFF);
	exit(0);
}

static s_trchkl()
{
	int i;

	trchk_getargs();
	for(i = 0; i < Count; i++)
		TRCHKL(Hook,i);
	exit(0);
}

static s_trchkg()
{
	int i;

	trchk_getargs();
	for(i = 0; i < Count; i++)
		TRCHKG(Hook,i,'a','b','c','d');
	exit(0);
}

static s_trcgen()
{
	int i;
	int len;

	trchk_getargs();
	len = strlen("hello world ");	/* make sure a multiple of 4 */
	for(i = 0; i < Count; i++)		/* to avoid "garbage" bytes */
		TRCGEN(0,Hook,i,len,"hello world");
	exit(0);
}

static s_trcread()
{
	int count;
	char *filename;
	int Mn;
	int rv;
	int fd;

	if(Argc != 3) {
		cat_eprint(0,"usage: trcread filename bytecount");
		exit(1);
	}
	filename = Argv[1];
	count = atoi(Argv[2]);
	if((fd = open(filename,0)) < 0) {
		perror(filename);
		exit(1);
	}
	while(--count >= 0) {
		rv = read(fd,&Mn,4);
		if(rv < 0) {
			perror("read");
			exit(1);
		}
		if(rv == 0)
			printf("0 ");
		else
			printf("%08x ",Mn);
	}
	exit(0);
}

/*
 * Zero-out timestamps
 * This routine is a portion of the trcrpt getevent() routine.
 */

static unsigned Logidx;
static unsigned Hookword;

static s_tfilt()
{
	int type;
	int size;
	char buf[512];

	for(;;) {
		if(jread(buf,4) != 4)	/* get hookword */
			exit(0);
		Hookword = ((int *)buf)[0];
		type = HKWDTOTYPE(Hookword);
		if(type < 0 || HKWDTOTYPE(HKTY_LAST) < type) {
			cat_eprint(0,"bad type %d. file index %d|%x\n",type,Logidx);
			exit(1);
		}
		Logidx += 4;
		if((size = HKWDTOLEN(Hookword) * 4) == 0) {					/* calc. for trcgen */
			size = 4 + 4 + HKWDTOWLEN(Hookword) * 4;	/* HW + d1 + buffer */
		}
		if(ISTIMESTAMPED(Hookword))					/* + timestamp */
			size += 4;
		if(size > 4)									/* read in rest */
			if(jread(buf+4,size-4) != size-4)
				break;
		if(ISTIMESTAMPED(Hookword))						/* zero timestamp */
			*((int *)&buf[size-4]) = 0;
		jwrite(buf,size);
		Logidx += size;
	}
}

/*
 * avoid fread/fwrite
 */
static jread(buf,count)
char *buf;
{
	int n;
	int c;

	n = 0;
	while(--count >= 0) {
		if((c = getchar()) == EOF)
			break;
		*buf++ = c;
		n++;
	}
	return(n);
}

static jwrite(buf,count)
char *buf;
{
	int c;

	while(--count >= 0) {
		c = *buf++;
		putchar(c);
	}
}

static int Offflg,Onflg,Clearflg;
struct snapdata {
	int sn_base;
	int sn_size;
	int sn_idx;
	int sn_wrapcount;
};
static int Utilfd;

snap(argc,argv)
char *argv[];
{
	int outfd,memfd;
	int idx;
	int count;
	int rv;
	int c;
	struct snapdata snapdata;
	char buf[512];

	outfd = 1;
	while((c = getopt(argc,argv,"xynf:")) != EOF) {
		switch(c) {
		case 'x':
			Clearflg++;
			break;
		case 'y':
			Onflg = 1;
			Offflg = 0;
			break;
		case 'n':
			Offflg = 1;
			Onflg = 0;
			break;
		case 'f':
			if((outfd = open(argv[1],O_RDWR|O_TRUNC|O_CREAT,0666)) < 0) {
				perror(argv[1]);
				exit(1);
			}
			break;
		case '?':
			snap_usage();
		}
	}
	if((Utilfd = open("/dev/sysutil",0)) < 0) {
		perror("sysutil");
		exit(1);
	}
	if(ioctl(Utilfd,UTRCSNAPINFO,&snapdata) < 0) {
		perror("subsnap");
		exit(1);
	}
	printf("base=%x idx=%x wrap=%x\n",
		snapdata.sn_base,snapdata.sn_idx,snapdata.sn_wrapcount);
	if((memfd = open("/dev/sysmem",0)) < 0) {
		perror("sysmem");
		exit(1);
	}
	if(Clearflg) {
		ioctl(Utilfd,UTRCSNAPCTL,2);
		exit(0);
	}
	if(Onflg) {
		ioctl(Utilfd,UTRCSNAPCTL,1);
		exit(0);
	}
	if(Offflg) {
		ioctl(Utilfd,UTRCSNAPCTL,0);
		exit(0);
	}
	count = snapdata.sn_idx;
	idx = 0;
	while(count) {
		lseek(memfd,snapdata.sn_base+idx,0);
		rv = read(memfd,buf,MIN(count,512));
		write(outfd,buf,rv);
		idx += rv;
		count -= rv;
	}
	sync();
	exit(0);
}

static snap_usage()
{

	cat_eprint("\n\
usage:   snap -y -n -x -f outfile\n\
\n\
output snap information to stdout.\n\
-y   turns snap on\n\
-n   turns snap off\n\
-x   clears out the kernel snap buffer\n");
	exit(1);
}

static s_panic()
{
	int fd;

	if((fd = open("/dev/sysutil",0)) < 0) {
		perror("open /dev/sysutil");
		exit(1);
	}
	ioctl(fd,UTRCPANIC,0);
	exit(1);
}

