static char sccsid[] = "@(#)06  1.28  src/bos/usr/sbin/acct/acctcom.c, cmdacct, bos41J, 9520B_all 5/18/95 10:09:45";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: aread, convtime, doexit, fatal, isdevnull, printhd, 
 *            println, usage, cmatch, cmset
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<time.h>
#include	<string.h>
#include	<sys/types.h>
#include	"acctdef.h"
#include	<grp.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/acct.h>
#include	<pwd.h>
#include        <locale.h>
#include	<sys/stat.h>
#include	<regex.h>
#include	<langinfo.h>

#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

#define MYKIND(flag)	((flag & ACCTF) == 0)
#define SU(flag)	((flag & ASU) == ASU)
#define PACCT		"/var/adm/pacct"
#define MEANSIZE	01
#define KCOREMIN	02
#define HOGFACTOR	04
#define	SEPTIME		010
#define	CPUFACTOR	020
#define IORW		040
#define pf(dble)	fprintf(stdout, TOOBIG(dble) ? fmtbig : "%8.2f ", dble)
#define	ps(rs)		fprintf(stdout, "%9.8s", rs)
#define	diag(string)		fprintf(stderr, "\t%s\n", string)
#define TSSIZE 128

char buf[TSSIZE];

struct	acct ab;
char	command_name[16];
char	timestring[16];
char	obuf[BUFSIZ];

double	cpucut,
	syscut,
	hogcut,
	iocut,
	realtot,
	kcoretot,
	iotot,
	rwtot;
long	daydiff,
	offset = -2,
	mem,
	cmdcount;
ulong	io,
	rw;
time_t	tstrt_b,
	tstrt_a,
	tend_b,
	tend_a,
	etime;
int	backward,
	flag_field,
	average,
	quiet,
	option,
	verbose = 1,
	uidflag,
	gidflag,
	unkid,	/*user doesn't have login on this machine*/
	errflg,
	su_user,
	fileout = 0,
	stdinflg,
	nfiles;
ulong	ac_mem;
dev_t	linedev	= -1;
uid_t	uidval,
	gidval;
int	cflag = 0;
int	already_read = 0;
regex_t	cname;	/* command name pattern to match*/

struct passwd *pw;
struct group *grp;
long	convtime(),
	tmsecs();
char timbuf[15],
	*ofile,
	*devtolin(),
	*uidtonam();
dev_t	lintodev();
FILE	*ostrm;
float   expand(),
	cputot,
	usertot,
	systot,
	sys,
	user,
	elapsed,
	cpu;

#define TOOBIG( x ) (x  > 99999.99) 
#define MIN(a,b) (((a)<(b))?(a):(b))
char    *fmtbig =       "%8.3g " ;

main(int argc, char **argv)
{
	register int	c;

	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	setbuf(stdout,obuf);
	while((c = getopt(argc, argv,
		"C:E:H:I:O:S:abe:fg:hikl:mn:o:qrs:tu:v")) != EOF) {
		switch(c) {
		case 'C':
			sscanf(optarg,"%lf",&cpucut);
			continue;
		case 'O':
			sscanf(optarg,"%lf",&syscut);
			continue;
		case 'H':
			sscanf(optarg,"%lf",&hogcut);
			continue;
		case 'I':
			sscanf(optarg,"%lf",&iocut);
			continue;
		case 'a':
			average++;
			continue;
		case 'b':
			backward++;
			continue;
		case 'g':
			if(sscanf(optarg,"%hu",&gidval) == 1)
				gidflag++;
			else if((grp=getgrnam(optarg)) == NULL)
				fatal( MSGSTR( GUNKNOWN, "Unknown group"), optarg); /*MSG*/
			else {
				gidval=grp->gr_gid;
				gidflag++;
			}
			continue;
		case 'h':
			option |= HOGFACTOR;
			continue;
		case 'i':
			option |= IORW;
			continue;
		case 'k':
			option |= KCOREMIN;
			continue;
		case 'm':
			option |= MEANSIZE;
			continue;
		case 'n':
			cflag=cmset(&cname, optarg);
			continue;
		case 't':
			option |= SEPTIME;
			continue;
		case 'r':
			option |= CPUFACTOR;
			continue;
		case 'v':
			verbose=0;
			continue;
		case 'l':
			linedev = lintodev(optarg);
			continue;
		case 'u':
			if(*optarg == '?')
				unkid++;
			else if(*optarg == '#')
				su_user++;
			else if(sscanf(optarg, "%lu", &uidval) == 1)
				uidflag++;
			else if((pw = getpwnam(optarg)) == NULL)
				fprintf(stderr, MSGSTR( UUNKNOWN, "%s: Unknown user %s\n"), /*MSG*/
					argv[0], optarg);
			else {
				uidval = pw->pw_uid;
				uidflag++;
			}
			continue;
		case 'q':
			quiet++;
			verbose=0;
			average++;
			continue;
		case 's':
			tend_a = convtime(optarg);
			continue;
		case 'S':
			tstrt_a = convtime(optarg);
			continue;
		case 'f':
			flag_field++;
			continue;
		case 'e':
			tstrt_b = convtime(optarg);
			continue;
		case 'E':
			tend_b = convtime(optarg);
			continue;
		case 'o':
			ofile = optarg;
			fileout++;
			if((ostrm = fopen(ofile, "w")) == NULL) {
				perror( MSGSTR( OPENERR, "open error on output file")); /*MSG*/
				errflg++;
			}
			continue;
		case '?':
			errflg++;
			continue;
		}
	}

	if(errflg) {
		usage();
		exit(1);
	}

	argv = &argv[optind];
	while(optind++ < argc) {
		dofile(*argv);
		argv++;
		nfiles++;
	}

	if(nfiles==0) {
		if(isatty(0) || isdevnull())
			dofile(PACCT);
		else {
			stdinflg = 1;
			backward = offset = 0;
			dofile(NULL);
		}
	}
	doexit(0);
}

dofile(fname)
char *fname;
{
	register struct acct *a = &ab;
	long curtime;
	time_t	ts_a = 0,
		ts_b = 0,
		te_a = 0,
		te_b = 0;
	long	daystart;
	long	nsize;
	struct tm *tm ;

	if(fname != NULL)
		if(freopen(fname, "r", stdin) == NULL) {
			fprintf(stderr, MSGSTR( CANTOPEN, "%s: cannot open %s\n"), "acctcom", fname); /*MSG*/
			return;
		}

	if(backward) {
		backward = 0;
		if(aread() == 0)
			return;
		backward = 1;
		nsize = sizeof(struct acct);	/* make sure offset is signed */
		fseek(stdin, (long)(-nsize), 2);
	} else {
		if(aread() == 0)
			return;
		if (stdinflg)
			already_read = 1;
		else
			rewind(stdin);
	}
	
	daydiff = a->ac_btime - (a->ac_btime % SECSINDAY);
	/* localtime corrects for timezone and daylight savings */
	tm = localtime(&a->ac_btime) ;
	daystart = a->ac_btime -
			((3600 * tm->tm_hour) + (60 * tm->tm_min) + tm->tm_sec);
	time(&curtime);
	if(daydiff < (curtime - (curtime % SECSINDAY))) {
		/*
		 * it is older than today
		 */

		strftime(buf,TSSIZE,"%c",localtime(&a->ac_btime));
		if(!fileout)
			fprintf(stdout,MSGSTR( ARFHD,
				"\nACCOUNTING RECORDS FROM:  %s"), buf);
	}

	if(tstrt_a) {
		ts_a = tstrt_a + daystart;
		fprintf(stdout,"\n");
		strftime(buf,TSSIZE,"%c",localtime(&ts_a));
		fprintf(stdout, MSGSTR( STAFT, "START AFT: %s"), buf);
	}
	if(tstrt_b) {
		ts_b = tstrt_b + daystart;
		fprintf(stdout,"\n");
		strftime(buf,TSSIZE,"%c",localtime(&ts_b));
		fprintf(stdout, MSGSTR( STBEF, "START BEF: %s"), buf); /*MSG*/
	}
	if(tend_a) {
		te_a = tend_a + daystart;
		fprintf(stdout,"\n");
		strftime(buf,TSSIZE,"%c",localtime(&te_a));
		fprintf(stdout, MSGSTR( ENDAFT, "END AFTER: %s"), buf); /*MSG*/
	}
	if(tend_b) {
		te_b = tend_b + daystart;
		fprintf(stdout,"\n");
		strftime(buf,TSSIZE,"%c",localtime(&te_b));
		fprintf(stdout, MSGSTR( ENDBEF, "END BEFOR: %s"), buf); /*MSG*/
	}
	if (!fileout)
		fprintf(stdout,"\n");
	if(ts_a) {
		if (te_b && ts_a > te_b) te_b += SECSINDAY;
	}
	/* If we are dealing with stdin, we may not by able to rewind
	 * the stdin (if it is connected to pipe).  In such a situation
	 * we set already_read to true.  This prevents a call to aread()
	 * to get the first record.  Instead we use the record already
	 * read. This was added to fix defect 107145. 
	 */
        while( already_read || (aread() != 0)) {
                already_read = 0; /* reset the already_read flag. does not need
				   * to be done every time it goes thru the
				   * loop, but checking is going to cost the 
				   * same as setting it.*/

                /*
                 * Validate the elapsed time against the user and system
                 * CPU times.  It is impossible for there to be more CPU
                 * time than wallclock time.  But we don't want to fake
                 * a too large wallclock time if the "real" time from the
                 * system is 0.00.
                 */

		elapsed = expand(a->ac_etime);
                sys = expand(a->ac_stime);
                user = expand(a->ac_utime);
                cpu = sys + user;

		etime = a->ac_btime + (long)elapsed;
		if(ts_a || ts_b || te_a || te_b) {

			if(te_a && (etime < te_a)) {
				if(backward) return;
				else continue;
			}
			if(te_b && (etime > te_b)) {
				if(backward) continue;
				else return;
			}
			if(ts_a && (a->ac_btime < ts_a))
				continue;
			if(ts_b && (a->ac_btime > ts_b))
				continue;
		}
		if(!MYKIND(a->ac_flag))
			continue;
		if(su_user && !SU(a->ac_flag))
			continue;
		ac_mem = MEM(a->ac_mem);
		mem = MEM(a->ac_mem) * ((cpu)?(cpu * HZ):1);
		bzero(command_name, sizeof(command_name));
		strncpy(command_name, a->ac_comm,
			MIN(sizeof(command_name)-1, sizeof(a->ac_comm)));
		io=expand_int(a->ac_io);
		rw=expand(a->ac_rw);
		if(cpucut && cpucut >= cpu)
			continue;
		if(syscut && syscut >= sys)
			continue;
		if(linedev != -1 && a->ac_tty != linedev)
			continue;
		if(uidflag && a->ac_uid != uidval)
			continue;
		if(gidflag && a->ac_gid != gidval)
			continue;
		if(cflag && !cmatch(a->ac_comm,&cname))
			continue;
		if(iocut && iocut > io)
			continue;
		if(unkid && uidtonam(a->ac_uid)[0] != '?')
			continue;
		if(verbose && (fileout == 0)) {
			printhd();
			verbose = 0;
		}
                if(hogcut && elapsed && hogcut >= cpu/elapsed)
                        continue;
		if(fileout)
			fwrite((void *)&ab, (size_t)sizeof(ab), (size_t)1, ostrm);
		else
			println();
		if(average) {
			cmdcount++;
			realtot += (double)elapsed;
			usertot += user;
			systot += sys;
			kcoretot += (double)mem;
			iotot += (double)io;
			rwtot += (double)rw;
		};
	}
}

aread()
{
	register flag;
	static	 ok = 1;

	if(fread((void *)(char *)&ab, (size_t)sizeof(struct acct), (size_t)1, stdin) != 1)
		flag = 0;
	else
		flag = 1;

	if(backward) {
		if(ok) {
			if(fseek(stdin,
				(long)(offset*sizeof(struct acct)), 1) != 0) {
					rewind(stdin);	/* get 1st record */
					ok = 0;
			}
		} else
			flag = 0;
	}
	return(flag);
}

printhd()
{
	fprintf(stdout, MSGSTR( CMSHED1, "COMMAND                      START    END          REAL")); /*MSG*/
	if(option & SEPTIME) {
		ps(MSGSTR( SYSSTR, "SYS"));
		ps(MSGSTR( USERSTR, "USER"));
	}else    ps(MSGSTR( CPUSTR, "CPU"));
	if(option & IORW){
		ps(MSGSTR( CHARSTR, "CHARS"));
		ps(MSGSTR( BLOCKSTR,"BLOCKS"));
	}
	if(option & CPUFACTOR)
		ps(MSGSTR( CPUSTR, "CPU"));
	if(option & HOGFACTOR)
		ps(MSGSTR( HOGSTR, "HOG"));
	if(!option || (option & MEANSIZE))
		ps(MSGSTR( MEANSTR, "MEAN"));
	if(option & KCOREMIN)
		ps(MSGSTR( KDCORESTR, "KCORE"));
	fprintf(stdout, "\n");
	fprintf(stdout, MSGSTR( CMSHED2, "NAME       USER     TTYNAME  TIME     TIME       (SECS)"));
	if(option & SEPTIME) {
		ps(MSGSTR( SECSTR, "(SECS)"));
		ps(MSGSTR( SECSTR, "(SECS)"));
	} else
		ps(MSGSTR( SECSTR, "(SECS)"));
	if(option & IORW) {
		ps(MSGSTR( TRNSFDSTR, "TRNSFD"));
		ps(MSGSTR( READSTR, "READ"));
	}
	if(option & CPUFACTOR)
		ps(MSGSTR( FACTORSTR, "FACTOR"));
	if(option & HOGFACTOR)
		ps(MSGSTR( FACTORSTR, "FACTOR"));
	if(!option || (option & MEANSIZE))
		ps(MSGSTR( SIZEKSTR, "SIZE(K)"));
	if(option & KCOREMIN)
		ps(MSGSTR( MINSTR, "MIN"));
	if(flag_field)
		fprintf(stdout, MSGSTR( FSTATSTR, "  F STAT"));
	fprintf(stdout, "\n");
	fflush(stdout);
}

println()
{

	char name[32];
	register struct acct *a = &ab;

	if(quiet)
		return;
	if(!SU(a->ac_flag))
		strcpy(name,command_name);
	else {
		strcpy(name,"#");
		strcat(name,command_name);
	}
	fprintf(stdout, "%-9.9s", name);
	strcpy(name,uidtonam(a->ac_uid));
	if(*name != '?')
		fprintf(stdout, "  %-8.8s", name);
	else
		fprintf(stdout, "  %-11lu",a->ac_uid);
	fprintf(stdout, " %-8.8s",a->ac_tty != -1? devtolin(a->ac_tty):"?");
	strftime(timbuf,15, "%T %y %n",localtime(&a->ac_btime));
	fprintf(stdout, "%.9s", timbuf);
	strftime(timbuf,15, "%T %y %n",localtime(&etime));
	fprintf(stdout, "%.9s", timbuf);
	fflush(stdout);
	pf((double)elapsed);
	if(option & SEPTIME) {
		pf(sys);
		pf(user);
	} else
		pf(cpu);
	if(option & IORW)
		fprintf(stdout, " %8u %8u",io,rw);
	if(option & CPUFACTOR)
                pf(cpu ? (user / cpu):0.0);
	if(option & HOGFACTOR)
                pf(elapsed ? (cpu / elapsed):0.0);
	if(!option || (option & MEANSIZE))
		pf((double) ac_mem);
	if(option & KCOREMIN)
		pf(MINS(ac_mem * cpu));
	if(flag_field)
		fprintf(stdout, " %1o %3o", a->ac_flag, a->ac_stat);
	fprintf(stdout, "\n");
}

/*
 * convtime converts time arg to internal value
 * arg has form hr:min:sec, min or sec are assumed to be 0 if omitted
 */
long
convtime(str)
char *str;
{
	long	hr = 0;
	long	min = 0;
	long	sec = 0;
	int i;
	int tin[3];
	char *t_format;
	char temp[30];

	min = sec = 0;

	t_format = nl_langinfo(T_FMT);
	strcpy(temp,t_format);

	if(sscanf(str, "%ld:%ld:%ld", &tin[0], &tin[1], &tin[2]) < 1) {
			fatal(MSGSTR( BADTIME, "bad time:"), str);
	}
	for(i=0;i < 3;i++) /* t_format %H:%M:%S */
	{
			switch(t_format[3*i + 1])
			{
				case 'h':
				case 'H':
					hr = tin[i];
					break;
				case 'm':
				case 'M':
					min = tin[i];
					break;
				case 's':
				case 'S':
					sec = tin[i];
					break;
			}
	}
	if((hr == 0) && (min == 0) && (sec == 0))
	{
		if(sscanf(str, "%ld:%ld:%ld", &hr, &min, &sec) < 1) {
			fatal(MSGSTR( BADTIME, "bad time:"), str);
	     }
	}
	sec += (min*60);
	sec += (hr*3600);
	return(sec);
}

cmatch(comm, re)
regex_t *re;
register char	*comm;
{

	char	xcomm[9];
	register i;

	for(i=0;i<8;i++){
		if(comm[i]==' '||comm[i]=='\0')
			break;
		xcomm[i] = comm[i];
	}
	xcomm[i] = '\0';

	return(!regexec(re,xcomm, (size_t)0, (regmatch_t *) NULL, 0));
}

cmset(re, pattern)
regex_t *re;
register char	*pattern;
{
	if (regcomp(re,pattern,REG_EXTENDED|REG_NOSUB) != 0)
		fatal(MSGSTR( PATRNERR, "pattern syntax"), NULL);
	return(1);
}

doexit(status)
{
	if(!average)
		exit(status);
	if(cmdcount) {
		fprintf(stdout, "\n%s=%ld ", MSGSTR(CMDSSTR, "cmds"), cmdcount);
		fprintf(stdout, "%s=%-6.2f ", MSGSTR(REALSTR, "Real"), realtot/cmdcount);
		cputot = systot + usertot;
		fprintf(stdout, "%s=%-6.2f ", MSGSTR(CPUSTR, "CPU"), cputot/cmdcount);
		fprintf(stdout, "%s=%-6.2f ", MSGSTR(USERSTR, "USER"), usertot/cmdcount);
		fprintf(stdout, "%s=%-6.2f ", MSGSTR(SYSSTR, "SYS"), systot/cmdcount);
		fprintf(stdout, "%s=%-8.2f ", MSGSTR(CHRSTR, "CHAR"), iotot/cmdcount);
		fprintf(stdout, "%s=%-8.2f ", MSGSTR(BLKSTR, "BLK"), rwtot/cmdcount);
		fprintf(stdout, "%s=%-4.2f ", MSGSTR(UTSTR, "USR/TOT"), usertot/cputot);
		fprintf(stdout, "%s=%-4.2f ", MSGSTR(HOGSTR, "HOG"), (cputot/realtot)*HZ);
		fprintf(stdout, "\n");
	}
	else
		fprintf(stdout, MSGSTR( NOCMDS, "\nNo commands matched\n"));
	exit(status);
}

isdevnull()
{
	struct stat	filearg;
	struct stat	devnull;

	if(fstat(0,&filearg) == -1) {
		fprintf(stderr,MSGSTR( NOSTATIN, "acctcom: cannot stat stdin\n"));
		return(0);
	}
	if(stat("/dev/null",&devnull) == -1) {
		fprintf(stderr,MSGSTR( NOSTATNULL, "acctcom: cannot stat /dev/null\n"));
		return(0);
	}

	if(filearg.st_rdev == devnull.st_rdev) return(1);
	else return(0);
}

fatal(s1, s2)
char *s1, *s2;
{
	fprintf(stderr,"acctcom: %s %s\n", s1, s2);
	exit(1);
}

usage()
{
	fprintf(stderr, MSGSTR( COMUSAGE1, "Usage: acctcom [options] [files]\n"));
	fprintf(stderr, MSGSTR( COMUSAGE2, "\nWhere options can be:\n"));
	diag(MSGSTR( COMUSAGE3, "-b	read backwards through file"));
	diag(MSGSTR( COMUSAGE4, "-f	print the fork/exec flag and exit status"));
	diag(MSGSTR( COMUSAGE5, "-h	print hog factor (total-CPU-time/elapsed-time)"));
	diag(MSGSTR( COMUSAGE6, "-i	print I/O counts"));
	diag(MSGSTR( COMUSAGE7, "-k	show total Kcore minutes instead of memory size"));
	diag(MSGSTR( COMUSAGE8, "-m	show mean memory size"));
	diag(MSGSTR( COMUSAGE9, "-r	show CPU factor (user-time/(sys-time + user-time))"));
	diag(MSGSTR( COMUSAGE10, "-t	show separate system and user CPU times"));
	diag(MSGSTR( COMUSAGE11, "-v	don't print column headings"));
	diag(MSGSTR( COMUSAGE12, "-a	print average statistics of selected commands"));
	diag(MSGSTR( COMUSAGE13, "-q	print average statistics only"));
	diag(MSGSTR( COMUSAGE14, "-l line	\tshow processes belonging to terminal /dev/line"));
	diag(MSGSTR( COMUSAGE15, "-u user	\tshow processes belonging to user name or user ID"));
	diag(MSGSTR( COMUSAGE16, "-u #	\tshow processes executed by super-user"));
	diag(MSGSTR( COMUSAGE17, "-u ?	\tshow processes executed by unknown UID's"));
	diag(MSGSTR( COMUSAGE18, "-g group	show processes belonging to group name of group ID"));
	diag(MSGSTR( COMUSAGE19, "-s time	\tshow processes ending after time (hh[:mm[:ss]])"));
	diag(MSGSTR( COMUSAGE20, "-e time	\tshow processes starting before time"));
	diag(MSGSTR( COMUSAGE21, "-S time	\tshow processes starting after time"));
	diag(MSGSTR( COMUSAGE22, "-E time	\tshow processes ending before time"));
	diag(MSGSTR( COMUSAGE23, "-n regex	select commands matching the ed(1) regular expression"));
	diag(MSGSTR( COMUSAGE24, "-o file	\tdo not print, put selected pacct records into file"));
	diag(MSGSTR( COMUSAGE25, "-H factor	show processes that exceed hog factor"));
	diag(MSGSTR( COMUSAGE26, "-O sec	\tshow processes that exceed CPU system time sec"));
	diag(MSGSTR( COMUSAGE27, "-C sec	\tshow processes that exceed total CPU time sec"));
	diag(MSGSTR( COMUSAGE28, "-I chars	show processes that transfer more than char chars"));
}
