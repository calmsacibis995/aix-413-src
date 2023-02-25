static char sccsid[] = "@(#)41	1.4  src/tcpip/usr/sbin/tcpdump/pcap.c, tcpip, tcpip411, GOLD410 2/4/94 14:59:05";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: bpf_open
 *		bpf_stats
 *		initdevice
 *		readloop
 *		wrapup
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
 * Copyright (c) 1990 The Regents of the University of California.
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
#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>			/* optionally get BSD define */
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <net/bpf.h>
#include <net/if.h>
#include <string.h>

#ifdef	_AIX
#include <net/if_types.h>
#endif	/* _AIX */

#include <nl_types.h>
#include "tcpdump_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TCPDUMP,n,s)

#include "interface.h"

extern int errno;
extern int immediate;

static void
bpf_stats(fd)
	int fd;
{
	struct bpf_stat s;
	
	if (ioctl(fd, BIOCGSTATS, &s) < 0)
		return;

	(void)fflush(stdout);
	(void)fprintf(stderr, MSGSTR(PACGOT,
		"%d packets received by filter\n"), s.bs_recv);
	(void)fprintf(stderr, MSGSTR(PACNOGOT,
		"%d packets dropped by kernel\n"), s.bs_drop);
}

void
readloop(cnt, if_fd, fp, printit)
	int cnt;
	int if_fd;
	struct bpf_program *fp;
	void (*printit)();
{
	u_char *buf;
	u_int bufsize;
	int cc;

	if (ioctl(if_fd, BIOCGBLEN, (caddr_t)&bufsize) < 0) {
		perror("tcpdump: BIOCGBLEN");
		exit(1);
	}
	buf = (u_char *)malloc(bufsize);

	if (ioctl(if_fd, BIOCSETF, (caddr_t)fp) < 0) {
		perror("tcpdump: BIOCSETF");
		exit(1);
	}
	while (1) {
		register u_char *bp, *ep;

		if ((cc = read(if_fd, (char *)buf, (int)bufsize)) < 0) {
			/* Don't choke when we get ptraced */
			if (errno == EINTR)
				continue;
#if defined(sun) && !defined(BSD)
			/*
			 * Due to a SunOS bug, after 2^31 bytes, the kernel
			 * file offset overflows and read fails with EINVAL.
			 * The lseek() to 0 will fix things.
			 */
			if (errno == EINVAL &&
			    (long)(tell(if_fd) + bufsize) < 0) {
				(void)lseek(if_fd, 0, 0);
				continue;
			}
#endif
			perror("tcpdump: read");
			exit(1);
		}
#ifdef DEBUG
		printf(MSGSTR(BYTES, "read %d bytes\n"), cc);
#endif
		/*
		 * Loop through each packet.
		 */
#define bhp ((struct bpf_hdr *)bp)
		bp = buf;
		ep = bp + cc;
		while (bp < ep) {
			register int caplen, hdrlen;
			if (cnt >= 0 && --cnt < 0)
				goto out;
			(*printit)(bp + (hdrlen = bhp->bh_hdrlen),
				   &bhp->bh_tstamp, bhp->bh_datalen,
				   caplen = bhp->bh_caplen);
			bp += BPF_WORDALIGN(caplen + hdrlen);
		}
#undef bhp
	}
 out:
	wrapup(if_fd);
}

wrapup(fd)
	int fd;
{
	bpf_stats(fd);
	close(fd);
}

static inline int
bpf_open()
{
	int fd;
	int n = 0;
	char device[sizeof "/dev/bpf000"];

	/*
	 * Go through all the minors and find one that isn't in use.
	 */
	do {
		(void)sprintf(device, "/dev/bpf%d", n++);
		fd = open(device, O_RDONLY);
	} while (fd < 0 && errno == EBUSY);

	if (fd < 0) {
		(void) fprintf(stderr, "tcpdump: ");
		perror(device);
		exit(-1);
	}
	return fd;
}

int
initdevice(device, pflag, linktype)
	char *device;
	int pflag;
	int *linktype;
{
	struct timeval timeout;
	int if_fd;
	struct ifreq ifr;
/* XXX
	struct bpf_version bv; 
*/
	
	if_fd = bpf_open();

/* XXX - add to kernel
	if (ioctl(if_fd, BIOCVERSION, (caddr_t)&bv) < 0)
		warning("kernel bpf interpreter may be out of date");
	else if (bv.bv_major != BPF_MAJOR_VERSION ||
		 bv.bv_minor < BPF_MINOR_VERSION)
		error(MSGSTR(BPFLANG, "requires bpf language %d.%d or higher; kernel is %d.%d"),
		      BPF_MAJOR_VERSION, BPF_MINOR_VERSION,
		      bv.bv_major, bv.bv_minor);
*/

	(void)strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
	if (ioctl(if_fd, BIOCSETIF, (caddr_t)&ifr) < 0) {
		(void) fprintf(stderr, "tcpdump: BIOCSETIF: ");
		perror(device);
		exit(-1);
	}
	/* Get the data link layer type. */
	if (ioctl(if_fd, BIOCGDLT, (caddr_t)linktype) < 0) {
		perror("tcpdump: BIOCGDLT");
		exit(-1);
	}
	/* set timeout */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	if (ioctl(if_fd, BIOCSRTIMEOUT, (caddr_t)&timeout) < 0) {
		perror("tcpdump: BIOCSRTIMEOUT");
		exit(-1);
	}
	/* set promiscuous mode if requested, but only for broadcast nets */
	if (pflag == 0) {
		switch (*linktype) {

		case IFT_SLIP:
			break;

		default:
			if (ioctl(if_fd, BIOCPROMISC, (void *)0) < 0) {
				fprintf(stderr, MSGSTR(PROMISC, 
"tcpdump: (%s):Promiscuous mode not supported for this interface.\n\tContinuing ...\n"),device);
			}
		}
	}
	if (immediate) {
		if (ioctl(if_fd, BIOCIMMEDIATE, (caddr_t)&immediate) < 0) {
			(void) fprintf(stderr, "tcpdump: BIOCIMMEDIATE: ");
			perror(device);
			exit(-1);
		}
	}
		
	return(if_fd);
}
