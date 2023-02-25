static char sccsid[] = "@(#)72	1.1  src/bos/usr/sbin/pse/utils/sthd/rdoptt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:56";
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
 ** rdoptt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#ifdef NETWARE
#define	sleep(seconds)	StreamDelay(MS_TO_TICKS(seconds * 1000))
#endif

staticf	void	rnormt(), rmsgdt(), rmsgnt();
staticf	void	test_bogus_values(   int fd, int read_mode   );
staticf	boolean	read_check();
staticf	boolean	getopt_check(), setopt_check();
staticf	void	rprotdatt(   int fd, int state   );
staticf	void	rprotdist(   int fd, int state   );
staticf	void	rprotnormt(   int fd, int state   );

#ifndef errno
extern	int	errno;
#endif

void
readt (state)
	int	state;
{
	char	buf[512];
	int	fd;

	printf("Testing read and write\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		if (!set_error_state(fd, EIO)) {
			stream_close(fd);
			return;
		}
		printf("testing write in error state\n");
		write_check(fd, buf, sizeof(buf), -1, EIO);

		printf("testing read in error state\n");
		read_check(fd, buf, sizeof(buf), -1, EIO);

		stream_close(fd);
		return;
	}

	write_check(fd, buf, sizeof(buf), sizeof(buf), 0);

	if (state & HANGUP) {
		if (!set_hangup_state(fd)) {
			stream_close(fd);
			return;
		}
		printf("testing write in HANGUP state\n");
		write_check(fd, buf, sizeof(buf), -1, EINVAL);
	}

	printf("reading half of the data\n");
	read_check(fd, buf, sizeof(buf)/2, sizeof(buf)/2, 0);

	printf("reading the rest of the data\n");
	read_check(fd, buf, sizeof(buf)/2, sizeof(buf)/2, 0);

	if (state & HANGUP) {
		printf("testing read in HANGUP state\n");
		read_check(fd, buf, sizeof(buf), 0, 0);
	}
	stream_close(fd);
}

void
rdoptt (state)
	int	state;
{
	int	fd;
	int	i1;

	printf("Testing read modes\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		printf("testing I_GRDOPT and I_SRDOPT in error state\n");
		if (set_error_state(fd, ENXIO)) {
			getopt_check(fd, &i1, 0, -1, ENXIO);
			setopt_check(fd, RNORM, -1, ENXIO);
		}
		stream_close(fd);
		return;
	}

	getopt_check(fd, &i1, RNORM | RPROTNORM, 0, 0);

	rnormt(fd, state);
	PAUSE_HERE();

	rmsgdt(fd, state);
	PAUSE_HERE();

	rmsgnt(fd, state);
	PAUSE_HERE();

	rprotnormt(fd, state);
	PAUSE_HERE();

	rprotdatt(fd, state);
	PAUSE_HERE();

	rprotdist(fd, state);
	PAUSE_HERE();

	test_bogus_values(fd, RMSGN);
	stream_close(fd);
}

staticf void
rmsgdt (fd, state)
	int	fd;
	int	state;
{
	char	buf[64];
	int	i1;

	printf("testing RMSGD mode\n");

	if (!setopt_check(fd, RMSGD, 0, 0))
		return;
	getopt_check(fd, &i1, RMSGD | RPROTNORM, 0, 0);

	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	if (read_check(fd, buf, sizeof(buf)/2, sizeof(buf)/2, 0)) {
		nread_check(fd, 0, 0, 0);
		if (stream_ioctl(fd, I_FLUSH, FLUSHR) != -1)
			printf("flush succeeded\n");
		else
			printe(true, "flush failed");
		nread_check(fd, 0, 0, 0);
	}
}

staticf void
rmsgnt (fd, state)
	int	fd;
	int	state;
{
	char	buf[300];
	int	i1;

	printf("testing RMSGN mode\n");
	if (!setopt_check(fd, RMSGN, 0, 0))
		return;
	getopt_check(fd, &i1, RMSGN | RPROTNORM, 0, 0);

	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	if (read_check(fd, buf, sizeof(buf)/2, sizeof(buf)/2, 0)) {
		nread_check(fd, 1, sizeof(buf)/2, 0);
		if (stream_ioctl(fd, I_FLUSH, FLUSHR) != -1)
			printf("flush succeeded\n");
		else
			printe(true, "flush failed");
		nread_check(fd, 0, 0, 0);
	}
}

staticf void
rnormt (fd, state)
	int	fd;
	int	state;
{
	char	buf[256];

	printf("testing RNORM mode\n");
	if (!setopt_check(fd, RNORM, 0, 0))
		return;

	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	if (read_check(fd, buf, sizeof(buf)/2, sizeof(buf)/2, 0)) {
		nread_check(fd, 1, sizeof(buf)/2, 0);
		if (stream_ioctl(fd, I_FLUSH, FLUSHR) != -1)
			printf("flush succeeded\n");
		else
			printe(true, "flush failed");
		nread_check(fd, 0, 0, 0);
	}
}

staticf void
rprotnormt (fd, state)
	int	fd;
	int	state;
{
	char	buf[64];
	struct strbuf	ctlbuf, databuf;

	printf("testing RPROTNORM mode\n");
	if (!setopt_check(fd, RPROTNORM | RNORM, 0, 0))
		return;

	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	databuf.buf = buf;
	databuf.len = sizeof(buf);
	printf("testing read after putmsg with both control and data\n");
	if (putmsg_check(fd, &ctlbuf, &databuf, 0, 0, 0)) {
		if (read_check(fd, buf, sizeof(buf), -1, EBADMSG)) {
			nread_check(fd, 1, sizeof(buf), 0);
			printf("flushing\n");
			flush_check(fd, 0, 0);
			nread_check(fd, 0, 0, 0);
		}
	}

	printf("testing read after normal write\n");
	if (write_check(fd, buf, sizeof(buf), sizeof(buf), 0)) {
		if (!read_check(fd, buf, sizeof(buf), sizeof(buf), 0))
			flush_check(fd, 0, 0);
	}
	nread_check(fd, 0, 0, 0);
}

staticf void
rprotdatt (fd, state)
	int	fd;
	int	state;
{
	struct strbuf	ctlbuf, databuf;
	char	buf1[64];
	char	buf2[64];
	char	buf3[128];

	printf("testing RPROTDAT mode\n");
	if (!setopt_check(fd, RPROTDAT | RNORM, 0, 0))
		return;

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after putmsg with both control and data\n");
	if (putmsg_check(fd, &ctlbuf, &databuf, 0, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf3), 0)) {
			if (buf3[0] != 1)
				printe(false, "read did not return correct byte 0 (%c should be 1)\n", buf3[0]);
			if (buf3[64] != 2)
				printe(false, "read did not return correct byte 64 (%c should be 0)\n", buf3[64]);
			nread_check(fd, 0, 0, 0);
		}
	}

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after RS_HIPRI putmsg with both control and data\n");
	if (putmsg_check(fd, &ctlbuf, &databuf, RS_HIPRI, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf3), 0)) {
			if (buf3[0] != 1)
				printe(false, "read did not return correct byte 0 (%c should be 1)\n", buf3[0]);
			if (buf3[64] != 2)
				printe(false, "read did not return correct byte 64 (%c should be 0)\n", buf3[64]);
			nread_check(fd, 0, 0, 0);
		} else
			flush_check(fd, 0, 0);
	}

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after putpmsg on band %d\n", DEFAULT_BAND);
	if (putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf3), 0)) {
			if (buf3[0] != 1)
				printe(false, "read did not return correct byte 0 (%c should be 1)\n", buf3[0]);
			if (buf3[64] != 2)
				printe(false, "read did not return correct byte 64 (%c should be 0)\n", buf3[64]);
			nread_check(fd, 0, 0, 0);
		} else
			flushband_check(fd, DEFAULT_BAND, 0, 0);
	}

	printf("testing read after normal write\n");
	if (write_check(fd, buf1, sizeof(buf1), sizeof(buf1), 0)) {
		if (!read_check(fd, buf2, sizeof(buf2), sizeof(buf2), 0))
			flush_check(fd, 0, 0);
		nread_check(fd, 0, 0, 0);
	}
}

staticf void
rprotdist (fd, state)
	int	fd;
	int	state;
{
	struct strbuf	ctlbuf, databuf;
	char	buf1[64];
	char	buf2[64];
	char	buf3[128];

	printf("testing RPROTDIS mode\n");
	if (!setopt_check(fd, RPROTDIS | RNORM, 0, 0))
		return;

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after putmsg with both control and data\n");
	if (putmsg_check(fd, &ctlbuf, &databuf, 0, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf2), 0)) {
			if (buf3[0] != 2)
				printe(false, "read did not return correct byte 0 (%c should be 0)\n", buf3[0]);
			nread_check(fd, 0, 0, 0);
		} else
			flush_check(fd, 0, 0);
	}

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after RS_HIPRI putmsg with both control and data\n");
	if (putmsg_check(fd, &ctlbuf, &databuf, RS_HIPRI, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf2), 0)) {
			if (buf3[0] != 2)
				printe(false, "read did not return correct byte 0 (%c should be 0)\n", buf3[0]);
			nread_check(fd, 0, 0, 0);
		} else
			flush_check(fd, 0, 0);
	}

	buf1[0] = 1;
	ctlbuf.buf = buf1;
	ctlbuf.len = sizeof(buf1);
	buf2[0] = 2;
	databuf.buf = buf2;
	databuf.len = sizeof(buf2);
	printf("testing read after putpmsg on band %d\n", DEFAULT_BAND);
	if (putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0)) {
		if (read_check(fd, buf3, sizeof(buf3), sizeof(buf2), 0)) {
			if (buf3[0] != 2)
				printe(false, "read did not return correct byte 0 (%c should be 0)\n", buf3[0]);
			nread_check(fd, 0, 0, 0);
		} else
			flushband_check(fd, DEFAULT_BAND, 0, 0);
	}

	printf("testing read after normal write\n");
	if (write_check(fd, buf1, sizeof(buf1), sizeof(buf1), 0)) {
		if (!read_check(fd, buf2, sizeof(buf2), sizeof(buf2), 0))
			flush_check(fd, 0, 0);
		nread_check(fd, 0, 0, 0);
	}
}

staticf boolean
read_check (fd, buf, len, ret_val, expected_errno)
	int	fd;
	char	* buf;
	int	len;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	i1 = stream_read(fd, buf, len);
	if (i1 == -1) {
		if (ret_val != -1)
			return printe(true, "read failed");
		if (errno != expected_errno)
			return printe(true, "read failed incorrectly, expecting errno %d",expected_errno);
		return printok("read failed as expected with errno %d", errno);
	}
	if (ret_val != i1)
		return printe(false, "read returned %d when expecting %d", i1, ret_val);
	return printok("read returned %d as expected", i1);
}

staticf boolean
setopt_check (fd, arg, ret_val, expected_errno)
	int	fd;
	int	arg;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_SRDOPT, arg)) {
	case 0:
		if (ret_val != i1)
			return printe(false, "set read opt successful when expecting %d", ret_val);
		return printok("set read opt successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "set read opt failed");
		if (errno != expected_errno)
			return printe(true, "set read opt failed incorrectly, expecting errno %d",expected_errno);
		return printok("set read opt failed as expected with errno %d", errno);
	default:
		if (ret_val != i1)
			return printe(false, "set read opt returned %d when expecting %d", i1, ret_val);
		return printok("set read opt returned %d as expected", i1);
	}
}

staticf boolean
swropt_check (fd, arg, ret_val, expected_errno)
	int	fd;
	int	arg;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_SWROPT, arg)) {
	case 0:
		if (ret_val != i1)
			return printe(false, "set write opt successful when expecting %d", ret_val);
		return printok("set write opt successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "set write opt failed");
		if (errno != expected_errno)
			return printe(true, "set write opt failed incorrectly, expecting errno %d",expected_errno);
		return printok("set write opt failed as expected with errno %d", errno);
	default:
		if (ret_val != i1)
			return printe(false, "set write opt returned %d when expecting %d", i1, ret_val);
		return printok("set write opt returned %d as expected", i1);
	}
}

staticf void
test_bogus_values (fd, arg)
	int	fd;
	int	arg;
{
	setopt_check(fd, -1, -1, EINVAL);
	setopt_check(fd, 12, -1, EINVAL);
#ifndef NO_BOGUS_ADDRESSES
	getopt_check(fd, BOGUS_ADDRESS, 0, -1, EFAULT);
#endif
	setopt_check(fd, arg, 0, 0);
}

staticf boolean
write_check (fd, buf, len, ret_val, expected_errno)
	int	fd;
	char	* buf;
	int	len;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	i1 = stream_write(fd, buf, len);
	if (i1 == -1) {
		if (ret_val != -1)
			return printe(true, "write failed");
		if (errno != expected_errno)
			return printe(true, "write failed incorrectly, expecting errno %d",expected_errno);
		return printok("write failed as expected with errno %d", errno);
	}
	if (ret_val != i1)
		return printe(false, "write returned %d when expecting %d", i1, ret_val);
	return printok("write returned %d as expected", i1);
}

staticf boolean
getopt_check (fd, argp, expected_mode, ret_val, expected_errno)
	int	fd;
	int	* argp;
	int	expected_mode;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_GRDOPT, argp)) {
	case 0:
		if (ret_val != i1)
			return printe(false, "get read opt successful when expecting %d", ret_val);
		if (*argp != expected_mode)
			return printe(false, "get read opt received mode %d when expecting %d", *argp,expected_mode);
		return printok("get read opt successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "get read opt failed");
		if (errno != expected_errno)
			return printe(true, "get read opt failed incorrectly, expecting errno %d",expected_errno);
		return printok("get read opt failed as expected with errno %d", errno);
	default:
		if (ret_val != i1)
			return printe(false, "get read opt returned %d when expecting %d", i1, ret_val);
		return printok("get read opt returned %d as expected", i1);
	}
}

staticf boolean
gwropt_check (fd, argp, expected_mode, ret_val, expected_errno)
	int	fd;
	int	* argp;
	int	expected_mode;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_GWROPT, argp)) {
	case 0:
		if (ret_val != i1)
			return printe(false, "get write opt successful when expecting %d", ret_val);
		if (*argp != expected_mode)
			return printe(false, "get write opt received mode %d when expecting %d", *argp,expected_mode);
		return printok("get write opt successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "get write opt failed");
		if (errno != expected_errno)
			return printe(true, "get write opt failed incorrectly, expecting errno %d",expected_errno);
		return printok("get write opt failed as expected with errno %d", errno);
	default:
		if (ret_val != i1)
			return printe(false, "get write opt returned %d when expecting %d", i1, ret_val);
		return printok("get write opt returned %d as expected", i1);
	}
}

wroptt (state)
	int	state;
{
	char	buf[64];
	int	fd;
	int	i1;

	printf("testing write option on zero-length messages\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	printf("testing I_SWROPT with invalid arg\n");
	swropt_check(fd, -1, -1, EINVAL);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing I_GWROPT with bad address\n");
	gwropt_check(fd, BOGUS_ADDRESS, 0, -1, EFAULT);
#endif

	printf("testing I_GWROPT on initial state of stream\n");
	gwropt_check(fd, &i1, SNDZERO, 0, 0);

	printf("setting write option for no zero-length messages\n");
	swropt_check(fd, 0, 0, 0);

	printf("testing I_GWROPT to make sure the state was set\n");
	gwropt_check(fd, &i1, 0, 0, 0);

	printf("writing a zero-length message\n");
	if (stream_write(fd, buf, 0) == 0) {
		printf("waiting for 2 seconds\n");
		sleep(2);
		printf("checking for message on queue\n");
		nread_check(fd, 0, 0, 0);
	}
	printf("setting write option for zero-length messages\n");
	swropt_check(fd, SNDZERO, 0, 0);

	printf("testing I_GWROPT to make sure the state was set\n");
	gwropt_check(fd, &i1, SNDZERO, 0, 0);

	printf("writing a zero-length message\n");
	if (stream_write(fd, buf, 0) == 0) {
		printf("waiting for 2 seconds\n");
		sleep(2);
		printf("checking for message on queue\n");
		nread_check(fd, 1, 0, 0);
	}

	stream_close(fd);
}
