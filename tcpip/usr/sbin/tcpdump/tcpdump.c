static char sccsid[] = "@(#)68	1.9  src/tcpip/usr/sbin/tcpdump/tcpdump.c, tcpip, tcpip411, GOLD410 4/9/94 15:03:06";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: cleanup
 *		default_print
 *		lookup_printer
 *		main
 *		usage
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1987-1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * tcpdump - monitor tcp/ip traffic on an ethernet.
 *
 * First written in 1987 by Van Jacobson, Lawrence Berkeley Laboratory.
 * Mercilessly hacked and occasionally improved since then via the
 * combined efforts of Van, Steve McCanne and Craig Leres of LBL.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <netinet/in.h>

#include <net/bpf.h>

#ifdef	_AIX
#include <net/if_types.h>
#endif	/* _AIX */

#include <locale.h>
#include <nl_types.h>
#include "tcpdump_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TCPDUMP,n,s)

#include "interface.h"
#include "savefile.h"
#include "addrtoname.h"

int immediate=0;		/* recieve packets immediately */
int fflag;			/* don't translate "foreign" IP address */
int nflag;			/* leave addresses as numbers */
int Nflag;			/* remove domains from printed host names */
int pflag;			/* don't go promiscuous */
int qflag;			/* quick (shorter) output */
int tflag = 1;			/* print packet arrival time */
int eflag;			/* print ethernet or tokenring header */
int vflag;			/* verbose */
int xflag;			/* print packet in hex */
int Oflag = 1;			/* run filter code optimizer */
int Sflag;			/* print raw TCP sequence numbers */

int dflag;			/* print filter code */

char *program_name;

long thiszone;			/* gmt to local correction */

static void cleanup();

/* Length of saved portion of packet. */
int snaplen = DEFAULT_SNAPLEN;

static int if_fd = -1;

struct printer printers[] = {
	{ null_if_print, IFT_LOOP, "lo" },
	{ ether_if_print, IFT_ETHER, "en" },
	{ iso88023_if_print, IFT_ISO88023, "et" },
	{ tr_if_print, IFT_ISO88025, "tr" },
/*
	{ sl_if_print, IFT_SLIP, "sl" },
*/
	{ fddi_if_print, IFT_FDDI, "fi" },
	{ 0, 0, 0 },
};

void
(*lookup_printer(type))()
	int type;
{
	struct printer *p;

	for (p = printers; p->f; ++p)
		if (type == p->type)
			return p->f;

	error(MSGSTR(UNKNOWNLINK, "unknown data link type 0x%x"), type);
	/* NOTREACHED */
}

void
main(argc, argv)
	int argc;
	char **argv;
{
	struct bpf_program *parse();
	void bpf_dump();

	int cnt = -1, i;
	struct timeb zt;
	struct bpf_program *fcode;
	int op;
	void (*printit)();
	char *infile = 0;
	char *cmdbuf;
	int linktype;
	int err;
	u_long localnet;
	u_long netmask;

	char *RFileName = 0;	/* -r argument */
	char *WFileName = 0;	/* -w argument */

	char *device = 0;

	int precision = clock_sigfigs();

	extern char *optarg;
	extern int optind, opterr;

	program_name = argv[0];

	setlocale(LC_ALL,"");
	catd = catopen(MF_TCPDUMP,NL_CAT_LOCALE);

	opterr = 0;
	while ((op = getopt(argc, argv, "c:defF:i:IlnNOpqr:s:Stvw:xY")) != EOF)
		switch (op) {
		case 'c':
			cnt = atoi(optarg);
			break;

		case 'd':
			++dflag;
			break;

		case 'e':
			++eflag;
			break;

		case 'f':
			++fflag;
			break;

		case 'F':
			infile = optarg;
			break;

		case 'i':
			device = optarg;
			break;
		case 'I':
			immediate++;
			break;

		case 'l':
			setlinebuf(stdout);
			break;

		case 'n':
			++nflag;
			break;

		case 'N':
			++Nflag;
			break;

		case 'O':
			Oflag = 0;
			break;

		case 'p':
			++pflag;
			break;

		case 'q':
			++qflag;
			break;

		case 'r':
 			RFileName = optarg;
 			break;

		case 's':
			snaplen = atoi(optarg);
			break;

		case 'S':
			++Sflag;
			break;

		case 't':
			--tflag;
			break;

		case 'v':
			++vflag;
			break;

		case 'w':
 			WFileName = optarg;
 			break;
#ifdef YYDEBUG
		case 'Y':
			{
			extern int yydebug;
			yydebug = 1;
			}
			break;
#endif
		case 'x':
			++xflag;
			break;

		default:
			usage();
			/* NOTREACHED */
		}

	if (tflag > 0) {
		struct timeval now;
		struct timezone tz;

		if (gettimeofday(&now, &tz) < 0) {
			perror("tcpdump: gettimeofday");
			exit(1);
		}
		thiszone = tz.tz_minuteswest * -60;
#ifdef	HUH_XXX
		if (localtime((time_t *)&now.tv_sec)->tm_isdst)
			thiszone += 3600;
#endif
	}

	(void) bpf_load();

	if (RFileName) {
		/*
		 * We don't need network access, so set it back to the user id.
		 * Also, this prevents the user from reading anyone's
		 * trace file.
		 */
		setuid(getuid());

		err = sf_read_init(RFileName, &linktype, &thiszone, &snaplen,
			&precision);
		if (err)
			sf_err(err);
		localnet = 0;
		netmask = 0;
		if (fflag != 0)
			error(MSGSTR(NO_FR, "-f and -r options are incompatible"));
	} else {
		if (device == 0) {
			device = lookup_device();
			if (device == 0)
				error(MSGSTR(CANTFIND, "can't find any interfaces"));
		}
		if_fd = initdevice(device, pflag, &linktype);
		lookup_net(device, &localnet, &netmask);
		/*
		 * Let user own process after socket has been opened.
		 */
		setuid(getuid());
	}

	if (infile) 
		cmdbuf = read_infile(infile);
	else
		cmdbuf = copy_argv(&argv[optind]);

	fcode = parse(cmdbuf, Oflag, linktype, netmask);
	if (dflag) {
		bpf_dump(fcode, dflag);
		exit(0);
	}
	init_addrtoname(fflag, localnet, netmask);

	(void)signal(SIGTERM, cleanup);
	(void)signal(SIGINT, cleanup);
	(void)signal(SIGHUP, cleanup);

	printit = lookup_printer(linktype);

	if (WFileName) {
		sf_write_init(WFileName, linktype, thiszone, snaplen, 
			      precision);
		printit = sf_write;
	}
	if (RFileName) {
		err = sf_read(fcode, cnt, snaplen, printit);
		if (err)
			sf_err(err);
	} else {
		fprintf(stderr, MSGSTR(LISTEN,
			 "%s: listening on %s\n"), program_name, device);
		fflush(stderr);
		readloop(cnt, if_fd, fcode, printit);
	}
	exit(0);
}

/* make a clean exit on interrupts */
static void
cleanup()
{
	if (if_fd >= 0) {
		putc('\n', stderr);
		wrapup(if_fd);
	}
	exit(0);
}

#define MAX_REASONABLE_PACKET_LENGTH 65535
void
default_print(sp, length)
	register u_short *sp;
	register int length;
{
	register u_int i;
	register int nshorts;

	nshorts = (unsigned) length / sizeof(u_short);
	if (length > MAX_REASONABLE_PACKET_LENGTH) {
		(void)printf(MSGSTR(UPACLEN,
			"[ unreasonable packet length: %d ]"), length);
		nshorts = 100;
	}
	i = 0;
	while (--nshorts >= 0) {
		if ((i++ % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %04x", ntohs(*sp++));
	}
	if (length & 1) {
		if ((i % 8) == 0)
			(void)printf("\n\t\t\t");
		(void)printf(" %02x", *(u_char *)sp);
	}
}

void
usage()
{
	extern char version[];

	(void)fprintf(stderr, MSGSTR(USAGE1, "Version %s\n"), version);
	(void)fprintf(stderr, MSGSTR(USAGE2,
"Usage: tcpdump [-defIlnOpqtvx] [-c count] [-i interface]\n"));
	(void)fprintf(stderr, MSGSTR(USAGE3,
"\t\t[-F filename] [-r filename] [-w filename] [expr]\n"));
	exit(-1);
}
