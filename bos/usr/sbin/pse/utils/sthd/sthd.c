static char sccsid[] = "@(#)77	1.1  src/bos/usr/sbin/pse/utils/sthd/sthd.c, cmdpse, bos411, 9428A410j 5/7/91 13:53:07";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** sthd.c 2.5, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"
#ifndef	INTERNAL_ONDELAY
#include <fcntl.h>
#endif
#include <pse/echo.h>
#include <sys/stream.h>
#include <ctype.h>
#include <sys/poll.h>
#include <stdio.h>
#include <signal.h>

#define	strneql(s1, s2)	(strncmp(s1, s2, strlen(s1)) == 0)

	void	atmarkt(   int state   );
	void	autot(   int state   );
	void	bandt(   int state   );
	void	closewaitt(   int state   );
staticf	void	emptyt(   int state   );
staticf	void	errort(   int state   );
staticf	int	error_ioctl(   int fd, int err_cnt, int read_err, int write_err   );
	void	fdinsertt(   int state   );
	void	findt(   int state   );
	void	gpmsgt(   int state   );
	void	gppmsgt(   int state   );
	void	linkt(   int state   );
	void	lookt(   int state   );
	void	listt(   int state   );
extern	char	* malloc(   int size   );
	void	nreadt(   int state   );
	void	pause_now(   void   );
	void	peekt(   int state   );
	void	plinkt(   int state   );
	void	pollt(   int state   );
	void	popt(   int state   );
	void	pusht(   int state   );
	void	rdoptt(   int state   );
	void	readt(   int state   );
staticf	int	reopen(   int fd  );
#ifdef	MPS
	void	seekt(   int state   );
#endif
	void	setoptst(   int state   );
	boolean	set_error_state(   int fd, int err_to_set   );
	boolean	set_hangup_state(   int fd   );
	boolean	set_ondelay_mode(   int fd   );
#ifndef NO_SIGNALS
	void	sigt(   int state   );
#endif
	void	srfdt(   int state   );
	void	strt(   int state   );
	void	wroptt(   int state   );

typedef struct cmd_s {
	char	* cmd_name;
	void	(*cmd_pfi)();
	char	* cmd_description;
} CMD, * CMDP;

static CMD cmd_tbl[] = {
	{ "atmark",	atmarkt,	"test I_ATMARK ioctl" },
	{ "autopush",	autot,		"test SAD driver and autopushing" },
	{ "band",	bandt,		"test ioctls on priority band messages" },
	{ "closewait",	closewaitt,	"test I_GETCLTIME and I_SETCLTIME ioctls" },
	{ "empty",	emptyt,		"test ioctls on an empty stream" },
	{ "error",	errort,		"test M_ERROR messages" },
	{ "fdinsert",	fdinsertt,	"test I_FDINSERT ioctl" },
	{ "find",	findt,		"test I_FIND ioctl" },
	{ "gpmsg",	gpmsgt,		"test getmsg and putmsg system calls" },
	{ "gppmsg",	gppmsgt,	"test getpmsg and putpmsg system calls" },
	{ "link",	linkt,		"test I_LINK and I_UNLINK ioctls" },
	{ "look",	lookt,		"test I_LOOK ioctl" },
	{ "list",	listt,		"test I_LIST ioctl" },
	{ "nread",	nreadt,		"test I_NREAD ioctl" },
	{ "peek",	peekt,		"test I_PEEK ioctl" },
	{ "plink",	plinkt,		"test I_PLINK and I_PUNLINK ioctls" },
	{ "poll",	pollt,		"test poll system call" },
	{ "pop",	popt,		"test I_POP ioctl" },
	{ "push",	pusht,		"test I_PUSH ioctl" },
	{ "read",	readt,		"test read and write system calls" },
	{ "readopt",	rdoptt,		"test read options (RNORM, RMSGD, etc)" },
#ifdef	MPS
	{ "seek",	seekt,		"test read seek/reset features" },
#endif
	{ "setopts",	setoptst,	"test M_SETOPTS messages" },
#ifndef NO_SIGNALS
	{ "signals",	sigt,		"test Stream head signals" },
#endif
	{ "srfd",	srfdt,		"test I_SENDFD and I_RECVFD ioctls" },
	{ "str",	strt,		"test I_STR ioctls" },
	{ "wropt",	wroptt,		"test write option" },
	{ 0 }
};

	char	* echo_name = "/dev/echo";
	char	* errm_name = "errm";
#ifndef errno
extern	int	errno;
#endif
	char	* nuls_name = "/dev/nuls";
extern	char	noshare * optarg;
extern	int	noshare optind;
	char	* pass_name = "pass";
#ifdef DO_PAUSE
	int	pause_between_tests = 1;
#else
	int	pause_between_tests;
#endif
	char	* sad_name = "/dev/sad";
	char	* spass_name = "spass";
static	char	usage_str[] = "usage: sthd [-a] [-i] [-p] [-e|-h|-o] [-s size]";

main (argc, argv)
	int	argc;
	char	** argv;
{
	char	buf[100];
	int	c;
	CMDP	cmd;
	boolean	do_all;
	int	i1;
	int	state;

	set_program_name(argv[0]);
	do_all = false;
	state = 0;
	while ((c = getopt(argc, argv, "aAiIpPs:S:oOeEhH")) != -1) {
		switch (c) {
		case 'a': case 'A':	/* Do all tests */
			do_all = true;
			break;
		case 'e': case 'E':	/* Test in error state */
			state |= ERROR;
			break;
		case 'h': case 'H':	/* Test in hangup state */
			state |= HANGUP;
			break;
		case 'o': case 'O':	/* Test O_NDELAY mode */
			state |= ONDELAY;
			break;
		case 'p': case 'P':	/* Pause between each test */
			pause_between_tests = true;
			break;
#if 0
		case 's': case 'S':
			buf_len = atoi(optarg);
			break;
#endif
		case '?':
		default:
			usage(usage_str);
		}
	}
	if (do_all) {
		for (cmd = cmd_tbl; cmd->cmd_pfi; cmd++) {
			if (cmd != cmd_tbl)
				PAUSE_HERE();
			(*cmd->cmd_pfi)(state);
		}
		exit(0);
	}
	for (;;) {
		write(1, "test to run? ", 13);
		if ((i1 = read(0, buf, sizeof(buf))) <= 0)
			break;
		buf[--i1] = '\0';
		if (i1 == 0)
			continue;
		if (strncmp(buf, "quit", i1) == 0)
			exit(0);
		if (buf[0] == '?'  ||  strncmp(buf, "help", i1) == 0) {
			for (cmd = cmd_tbl; cmd->cmd_pfi; cmd++)
				printf("%-8s\t%s\n", cmd->cmd_name, cmd->cmd_description);
			continue;
		}
		for (cmd = cmd_tbl; cmd->cmd_pfi; cmd++) {
			if (strncmp(buf, cmd->cmd_name, i1) == 0) {
				(*cmd->cmd_pfi)(state);
				break;
			}
		}
		write(1, "\n", 1);
	}
}

static int
reopen (fd)
	int	fd;
{
	stream_close(fd);
	return stream_open("/dev/echo", 2);
}

staticf void
emptyt (state)
	int	state;
{
	char	buf[FMNAMESZ+1];
	int	fd;
	boolean	i1;

	printf("Testing behavior of empty stream\n");
	/* TODO: add ondelay, error and hangup testing */

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "couldn't open %s", echo_name);
		return;
	}
	if (stream_ioctl(fd, I_POP, 0) == -1  &&  errno == EINVAL)
		printf("pop into empty stream failed as expected\n");
	else
		printe(true, "pop into empty stream succeeded");

	printf("testing look with empty stream\n");
	if (stream_ioctl(fd, I_LOOK, buf) != -1  ||  errno != EINVAL)
		printe(true, "look failed incorrectly");

	printf("testing stream_ioctl with bogus command value\n");
	if (stream_ioctl(fd, -1, 0) == -1  &&  errno == EINVAL)
		printf("stream_ioctl failed as expected with errno %d\n", EINVAL);
	else
		printe(true, "stream_ioctl didn't fail as expected");

#ifdef INTERNAL_ONDELAY
	printf("testing get delay flag\n");
	if (stream_ioctl(fd, I_GETDELAY, &i1) != -1) {
		if (i1 == ((state & ONDELAY) ? 1 : 0))
			printf("getdelay returned %d as expected\n", i1);
		else
			printe(false, "getdelay returned %d when expecting %d\n", i1, (state & ONDELAY) ? 1 : 0);
	} else
		printe(true, "getdelay failed\n");

	printf("testing set delay flag\n");
	if (stream_ioctl(fd, I_SETDELAY, (state & ONDELAY) ? 0 : 1) != -1)
		printf("setdelay succeeded\n");
	else
		printe(true, "setdelay failed\n");

	printf("checking set delay flag\n");
	if (stream_ioctl(fd, I_GETDELAY, &i1) != -1) {
		if (i1 == ((state & ONDELAY) ? 0 : 1))
			printf("getdelay returned %d as expected\n", i1);
		else
			printe(false, "getdelay returned %d when expecting %d\n", i1, (state & ONDELAY) ? 0 : 1);
	} else
		printe(true, "getdelay failed\n");
	stream_ioctl(fd, I_SETDELAY, (state & ONDELAY) ? 1 : 0);
#endif
	stream_close(fd);
}

staticf void
errort (state)
	int	state;
{
	int	fd;
	int	i1;

	printf("Testing M_ERROR reply messages to M_IOCTLs\n");

	printf("testing M_ERROR with only one error value\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "couldn't open %s", echo_name);
		return;
	}
	switch (i1 = error_ioctl(fd, 1, E2BIG, ENOEXEC)) {
	case -1:
		if (errno == E2BIG)
			printf("ioctl failed with expected error\n");
		else
			printe(true, "ioctl failed with unexpected error");
		break;
	default:
		printe(true, "ioctl returned %d when expecting failure", i1);
		break;
	}
	stream_close(fd);

	printf("testing M_ERROR with both read and write errors set\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "couldn't open %s", echo_name);
		return;
	}
	switch (i1 = error_ioctl(fd, 2, E2BIG, ENOEXEC)) {
	case -1:
		if (errno == ENOEXEC)
			printf("ioctl failed with expected write error\n");
		else
			printe(true, "ioctl failed with unexpected error");
		break;
	default:
		printe(true, "ioctl returned %d when expecting failure", i1);
		break;
	}
	stream_close(fd);

	printf("testing M_ERROR with only write error set\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "couldn't open %s", echo_name);
		return;
	}
	switch (i1 = error_ioctl(fd, 2, 0, ENOEXEC)) {
	case -1:
		if (errno == ENOEXEC)
			printf("ioctl failed with expected write error\n");
		else
			printe(true, "ioctl failed with unexpected error");
		break;
	default:
		printe(true, "ioctl returned %d when expecting failure", i1);
		break;
	}
	stream_close(fd);

	printf("testing M_ERROR with only read error set\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "couldn't open %s", echo_name);
		return;
	}
	switch (i1 = error_ioctl(fd, 2, E2BIG, 0)) {
	case -1:
		if (errno == E2BIG)
			printf("ioctl failed with expected read error\n");
		else
			printe(true, "ioctl failed with unexpected error");
		break;
	default:
		printe(true, "ioctl returned %d when expecting failure", i1);
		break;
	}
	stream_close(fd);
}

boolean
flush_check (fd, state, expected_errno)
	int	fd;
	int	state;
	int	expected_errno;
{
	switch (stream_ioctl(fd, I_FLUSH, FLUSHRW)) {
	case -1:
		if (state & (ERROR | HANGUP)) {
			if (errno == expected_errno)
				return true;
			return printe(true, "flush failed incorrectly");
		}
		return printe(true, "flush failed");
	case 0:
		if (state & (HANGUP | ERROR))
			return printe(false, "flush successful");
		return true;
	}
}

#ifdef INTERNAL_GETMSG
int
getmsg (fd, ctlptr, dataptr, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	* flags;
{
	struct strpeek	strp;
	int	i1;

	memset(&strp, '\0', sizeof(struct strpeek));
	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.maxlen = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.maxlen = -1;
	strp.flags = *flags;
	if ((i1 = stream_ioctl(fd, I_GETMSG, &strp)) >= 0) {
		if (ctlptr)
			*ctlptr = strp.ctlbuf;
		if (dataptr)
			*dataptr = strp.databuf;
		*flags = strp.flags;
	}
	return i1;
}
#endif

#ifdef INTERNAL_GETPMSG
int
getpmsg (fd, ctlptr, dataptr, bandp, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	* bandp;
	int	* flags;
{
	struct strpmsg	strp;
	int	i1;

	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.maxlen = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.maxlen = -1;
	strp.band = *bandp;
	strp.flags = *flags;
	if ((i1 = stream_ioctl(fd, I_GETPMSG, (char *)&strp)) >= 0) {
		if (ctlptr)
			*ctlptr = strp.ctlbuf;
		if (dataptr)
			*dataptr = strp.databuf;
		*bandp = strp.band;
		*flags = strp.flags;
	}
	return i1;
}
#endif

void
pause_now () {
	if (!pause_between_tests)
		return;
	printf("\nPausing... (type <return> to continue)\n");
	fflush(stdout);
	getchar();
}

#ifdef INTERNAL_POLL
int
poll (fds, nfds, timeout)
	struct pollfd	* fds;
	unsigned long	nfds;
	int	timeout;
{
	struct strpoll	strpl;

	strpl.nfds = nfds;
	strpl.pollfdp = fds;
	strpl.timeout = timeout;
	return stream_ioctl(fds[0].fd, I_POLL, &strpl);
}
#endif

void
popt (state)
	int	state;
{
	int	fd;

	printf("Testing pop stream_ioctl\n");

	if ((fd = stream_open(nuls_name, 2)) == -1) {
		printe(true, "open of %s failed", nuls_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "push of %s failed", pass_name);
		stream_close(fd);
		return;
	}
	if (stream_ioctl(fd, I_POP, 0) == -1) {
		if (state & (ERROR | HANGUP)) {
			if (errno == EIO)
				printok("pop failed as expected with errno %d", errno);
			else
				printe(true, "pop failed incorrectly, expecting errno %d", EIO);
		} else
			printe(true, "pop failed");
	}
	switch (stream_ioctl(fd, I_FIND, pass_name)) {
	case 0:
		printok("pop succeeded, and subsequent find of '%s' returned 0", pass_name);
		break;
	case 1:
		printe(false, "find succeeded after pop");
		break;
	default:
		printe(true, "find failed after pop");
		break;
	}
	stream_close(fd);
}

#ifdef INTERNAL_PUTMSG
int
putmsg (fd, ctlptr, dataptr, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	flags;
{
	struct strpeek	strp;

	memset(&strp, '\0', sizeof(struct strpeek));
	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.len = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.len = -1;
	strp.flags = flags;
	return stream_ioctl(fd, I_PUTMSG, &strp);
}
#endif

#ifdef INTERNAL_PUTPMSG
int
putpmsg (fd, ctlptr, dataptr, band, flags)
	int	fd;
	struct strbuf	* ctlptr;
	struct strbuf	* dataptr;
	int	band;
	int	flags;
{
	struct strpmsg	strp;

	if (ctlptr)
		strp.ctlbuf = *ctlptr;
	else
		strp.ctlbuf.len = -1;
	if (dataptr)
		strp.databuf = *dataptr;
	else
		strp.databuf.len = -1;
	strp.band = band;
	strp.flags = flags;
	return stream_ioctl(fd, I_PUTPMSG, &strp);
}
#endif

boolean
request_sig (fd, buf, buf_len, pri)
	int	fd;
	char	* buf;
	int	buf_len;
	int	pri;
{
	struct strioctl	stri;
	eblk_t	* ep;

	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = sizeof(eblk_t) + sizeof(ushort);
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	ep = (eblk_t *)buf;
	ep->eb_type = pri ? M_PCSIG : M_SIG;
	ep->eb_flag = 0;
	ep->eb_len = sizeof(uchar);
	*(uchar *)&ep[1] = (uchar)SIGPOLL;
	if (stream_ioctl(fd, I_STR, &stri) == -1)
		return printe(true, "I_STR to request signal failed");
	return printok("signal requested");
}

staticf int
error_ioctl (fd, err_cnt, read_err, write_err)
	int	fd;
	int	err_cnt;
	int	read_err;
	int	write_err;
{
	char	buf[sizeof(eblk_t) + 4];
	eblk_t	* eb;
	struct strioctl	stri;

	eb = (eblk_t *)buf;
	eb->eb_type = M_ERROR;
	eb->eb_flag = 0;
	eb->eb_len = err_cnt;
	buf[sizeof(eblk_t)] = (char)read_err;
	if (err_cnt == 2)
		buf[sizeof(eblk_t)+1] = (char)write_err;
	stri.ic_cmd = ECHO_GENMSG_NOREPLY;
	stri.ic_len = sizeof(eblk_t) + err_cnt;
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	return stream_ioctl(fd, I_STR, &stri);
}

boolean
set_error_state (fd, err_to_set)
	int	fd;
	int	err_to_set;
{
	char	buf[sizeof(eblk_t) + 4];
	eblk_t	* eb;
	struct strioctl	stri;

	eb = (eblk_t *)buf;
	eb->eb_type = M_ERROR;
	eb->eb_flag = 0;
	eb->eb_len = 1;
	buf[sizeof(eblk_t)] = (char)err_to_set;
	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = sizeof(eblk_t);
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	if (stream_ioctl(fd, I_STR, &stri) == -1)
		return printe(true, "I_STR to set error failed");
	return printok("error state set");
}

boolean
set_hangup_state (fd)
	int	fd;
{
	struct strioctl	stri;
	eblk_t	eblk;

	eblk.eb_type = M_HANGUP;
	eblk.eb_flag = 0;
	eblk.eb_len = 0;
	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = sizeof(eblk_t);
	stri.ic_timout = -1;
	stri.ic_dp = (char *)&eblk;
	if (stream_ioctl(fd, I_STR, &stri) == -1)
		return printe(true, "I_STR to set hangup failed");
	return printok("hangup state set");
}

boolean
clear_ondelay_mode (fd)
	int	fd;
{
#ifndef	INTERNAL_ONDELAY
	if (fcntl(fd, F_SETFL, 0) == -1)	/* NOTE: this may not be right... */
		return printe(true, "attempt to clear ondelay mode failed");
#else
	if (stream_ioctl(fd, I_SETDELAY, 0) == -1)
		return printe(true, "attempt to clear ondelay mode failed");
#endif
	return printok("ondelay mode cleared");
}

boolean
set_ondelay_mode (fd)
	int	fd;
{
#ifndef	INTERNAL_ONDELAY
	if (fcntl(fd, F_SETFL, O_NDELAY) == -1)
		return printe(true, "attempt to set ondelay mode failed");
#else
	if (stream_ioctl(fd, I_SETDELAY, 1) == -1)
		return printe(true, "attempt to set ondelay mode failed");
#endif
	return printok("ondelay mode set");
}
