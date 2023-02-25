static char sccsid[] = "@(#)68	1.3  src/bos/usr/sbin/pse/utils/sthd/pollt.c, cmdpse, bos411, 9428A410j 5/22/91 11:41:53";
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
 ** pollt.c 2.1
 **/


#include <pse/common.h>
#include "sthd.h"
#include <sys/poll.h>

static	boolean	poll_check();

extern	int	errno;
static	char	ibuf[1024];

void
pollt (state)
	int	state;
{
	struct bandinfo	bi;
	char	buf[256];
	struct strbuf	ctlbuf;
	int	fd;
	struct pollfd	fds[4];
	int	mask, i1;

	printf("\nTesting poll\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}
	mask = 0;
	if (state & HANGUP) {
		set_hangup_state(fd);
		mask |= POLLHUP;
	}
	if (state & ERROR) {
		set_error_state(fd, ENXIO);
		mask |= POLLERR;
	}

	printf("testing poll with bogus nfds\n");
	fds[0].fd = fd;
	fds[0].events = POLLOUT;
	poll_check(fds, (ulong)-1, -1, -1, EINVAL);

	printf("testing poll with bogus fd\n");
	fds[1].fd = 99;
	fds[1].events = POLLOUT;
	if (poll_check(fds, (ulong)2, 1, 2, 0)) {
		if (fds[0].revents != (mask == 0 ? POLLOUT : mask))
			printe(false, "poll returned revents 0x%x for fd %d", fds[0].revents, fd);
		if (fds[1].revents == POLLNVAL)
			printf("poll set POLLNVAL as expected\n");
		else
			printe(false, "poll set 0x%x revents, not POLLNVAL", fds[1].revents);
	}

	printf("testing poll for POLLOUT and POLLWRNORM\n");
	fds[0].fd = fd;
	i1 = (mask == 0) ? (POLLOUT | POLLWRNORM) : mask;
	fds[0].events = (POLLOUT | POLLWRNORM);
	fds[0].revents = -1;
	if (poll_check(fds, (ulong)1, -1, 1, 0)) {
		if (fds[0].revents == i1)
			printf("poll set revents with 0x%x as expected\n", i1);
		else
			printe(false, "poll set 0x%x revents, not POLLOUT | POLLWRNORM", fds[0].revents);
	}

	if (mask != 0) {	/* Finished testing in Hangup or Error state */
		stream_close(fd);
		return;
	}

	printf("testing poll for POLLIN\n");
	fds[0].events = POLLIN;
	fds[0].revents = -1;
	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		if (poll_check(fds, (ulong)1, -1, 1, 0)) {
			if (fds[0].revents != -1  &&  (fds[0].revents & POLLIN))
				printf("poll set POLLIN as expected\n");
			else
				printe(false, "poll set 0x%x revents, not POLLIN", fds[0].revents);
		}
		printf("testing poll for POLLRDNORM on M_DATA message\n");
		fds[0].events = POLLRDNORM;
		fds[0].revents = -1;
		if (poll_check(fds, (ulong)1, -1, 1, 0)) {
			if (fds[0].revents != -1  &&  (fds[0].revents & POLLRDNORM))
				printf("poll set POLLRDNORM as expected\n");
			else
				printe(false, "poll set 0x%x revents, not POLLRDNORM", fds[0].revents);
		}
	} else
		printe(true, "write failed");
	flush_check(fd, 0, 0);

	printf("testing poll for POLLPRI\n");
	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	if (putmsg(fd, &ctlbuf, nilp(struct strbuf), RS_HIPRI) == 0) {
		fds[0].events = POLLPRI;
		fds[0].revents = -1;
		if (poll_check(fds, (ulong)1, -1, 1, 0)) {
			if (fds[0].revents != -1  &&  (fds[0].revents & POLLPRI))
				printf("poll set POLLPRI as expected\n");
			else
				printe(false, "poll set 0x%x revents, not POLLPRI", fds[0].revents);
		}
	} else
		printe(true, "putmsg failed");
	flush_check(fd, state, 0);

	PAUSE_HERE();
	printf("testing poll for POLLRDBAND\n");
	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	if (putpmsg(fd, &ctlbuf, nilp(struct strbuf), DEFAULT_BAND, MSG_BAND) == 0) {
		fds[0].events = POLLRDBAND;
		fds[0].revents = -1;
		if (poll_check(fds, (ulong)1, -1, 1, 0)) {
			if (fds[0].revents != -1  &&  (fds[0].revents & POLLRDBAND))
				printf("poll set POLLRDBAND as expected\n");
			else
				printe(false, "poll set 0x%x revents, not POLLRDBAND", fds[0].revents);
		}
	} else
		printe(true, "putpmsg failed");
	bi.bi_flag = FLUSHRW;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);

	printf("testing poll for POLLWRBAND\n");
	printf("looping doing putpmsg until canput fails (make sure the band gets created)\n");
	ctlbuf.buf = ibuf;
	ctlbuf.len = sizeof(ibuf);
	while (stream_ioctl(fd, I_CANPUT, (char *)DEFAULT_BAND) == 1) {
		if (!putpmsg_check(fd, &ctlbuf, &ctlbuf, DEFAULT_BAND, MSG_BAND, 0, 0)) {
			stream_close(fd);
			return;
		}
	}
	bi.bi_flag = FLUSHRW;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);
	printf("now trying POLLWRBAND\n");
	fds[0].fd = fd;
	fds[0].events = POLLWRBAND;
	fds[0].revents = -1;
	if (poll_check(fds, (ulong)1, -1, 1, 0)) {
		if (fds[0].revents == POLLWRBAND)
			printf("poll set revents with 0x%x as expected\n", i1);
		else
			printe(false, "poll set 0x%x revents, not POLLWRBAND", fds[0].revents);
	}

#ifdef OUTDEF
	printf("testing poll for POLLMSG\n");
	fds[0].fd = fd;
	fds[0].events = POLLMSG;
	fds[0].revents = -1;
	if (poll_check(fds, (ulong)1, -1, 1, 0)) {
		if (fds[0].revents == POLLMSG)
			printf("poll set revents with 0x%x as expected\n", i1);
		else
			printe(false, "poll set 0x%x revents, not POLLMSG", fds[0].revents);
	}
#endif

	printf("testing poll for timeout\n");
	fds[0].events = POLLIN;
	poll_check(fds, (ulong)1, 5000, 0, 0);

	printf("testing non-blocking poll\n");
	poll_check(fds, (ulong)1, 0, 0, 0);

	printf("testing poll with -1 fd and timeout\n");
	fds[0].revents = -1;
	fds[1].fd = -1;
	fds[1].events = POLLOUT;
	fds[1].revents = -1;
	if (poll_check(fds, (ulong)2, 5000, 0, 0)) {
		if (fds[1].revents == 0)
			printf("poll ignored the -1 fd as expected\n");
		else
			printe(false, "poll set 0x%x revents, not 0", fds[1].revents);
	}

	printf("testing poll with all -1 fds and timeout\n");
	for (i1 = 0; i1 < 4; i1++) {
		fds[i1].fd = -1;
		fds[i1].events = POLLOUT;
		fds[i1].revents = -1;
	}
	if (poll_check(fds, (ulong)4, 5000, 0, 0)) {
		if (fds[0].revents == 0  &&  fds[1].revents == 0
		&&  fds[2].revents == 0  &&  fds[3].revents == 0)
			printf("poll ignored the -1 fds as expected\n");
		else
			printe(false, "poll set revents 0%x 0%x 0%x 0%x, not 0 0 0 0", fds[0].revents, fds[1].revents,
			fds[2].revents, fds[3].revents);
	}
	stream_close(fd);
}

static boolean
poll_check (fds, nfds, timeout, ret_val, expected_errno)
	struct pollfd	* fds;
	ulong	nfds;
	int	timeout;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = poll(fds, nfds, timeout)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "poll failed");
		if (errno != expected_errno)
			return printe(true, "poll failed incorrectly, expecting errno %d", expected_errno);
		return printok("poll failed as expected");
	default:
		if (ret_val != i1)
			return printe(false, "poll returned %d when expecting %d", i1, ret_val);
		return printok("poll returned %d as expected", i1);
	}
}
