static char sccsid[] = "@(#)55	1.2  src/bos/usr/sbin/pse/utils/sthd/closewt.c, cmdpse, bos411, 9428A410j 5/20/91 23:43:43";
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
 ** closewt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#ifdef NETWARE
#define	sleep(seconds)	StreamDelay(MS_TO_TICKS(seconds * 1000))
#endif

#ifndef errno
extern	int	errno;
#endif

staticf boolean	getcltime_check(   int fd, long * time_argp, long expected_time, int ret_val, int expected_errno   );
staticf boolean	setcltime_check(   int fd, long * time_arg, int ret_val, int expected_errno   );

void
closewaitt (state)
	int	state;
{
	char	buf[MAX_STACK_BUF];
	int	fd;
	long	l1;

	printf("Testing I_GETCLTIME and I_SETCLTIME ioctls\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, spass_name) == -1) {
		printe(true, "push of %s failed", spass_name);
		return;
	}

	if (state & ERROR) {
		if (set_error_state(fd, EIO)) {
			printf("testing getcltime in error state");
			getcltime_check(fd, &l1, 0L, -1, EIO);
			printf("testing setcltime in error state");
			setcltime_check(fd, &l1, -1, EIO);
		}
		stream_close(fd);
		return;
	}

	printf("testing setcltime with bad time value\n");
	l1 = -1;
	setcltime_check(fd, &l1, -1, EINVAL);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing setcltime with bad address\n");
	setcltime_check(fd, BOGUS_ADDRESS, -1, EFAULT);

	printf("testing getcltime with bad address\n");
	getcltime_check(fd, BOGUS_ADDRESS, 0L, -1, EFAULT);
#endif

	printf("setting close wait time to 5 seconds\n");
	l1 = 5000L;
	setcltime_check(fd, &l1, 0, 0);

	printf("setting the stream into ondelay mode\n");
	if (!set_ondelay_mode(fd)) {
		stream_close(fd);
		return;
	}

	printf("filling the stream with data\n");
	errno = 0; /* if fails for other than EAGAIN, give true reason */
	while (stream_write(fd, buf, sizeof(buf)) == sizeof(buf))
		noop;
	if (errno != EAGAIN) {
		printe(true, "write failed");
		stream_close(fd);
		return;
	}

	printf("closing the stream, this should not wait for data to drain\n");
	stream_close(fd);

	printf("opening another stream\n");
	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, spass_name) == -1) {
		printe(true, "push of %s failed", spass_name);
		return;
	}

	printf("setting close wait time to 5 seconds\n");
	l1 = 5000L;
	setcltime_check(fd, &l1, 0, 0);

	printf("setting the stream into ondelay mode\n");
	if (!set_ondelay_mode(fd)) {
		stream_close(fd);
		return;
	}

	printf("filling the stream with data\n");
	while (stream_write(fd, buf, sizeof(buf)) == sizeof(buf))
		noop;
	if (errno != EAGAIN) {
		printe(true, "write failed");
		stream_close(fd);
		return;
	}
	sleep(2);
	printf("filling the stream with more data\n");
	while (stream_write(fd, buf, sizeof(buf)) == sizeof(buf))
		noop;
	if (errno != EAGAIN) {
		printe(true, "write failed");
		stream_close(fd);
		return;
	}

	printf("clearing ondelay mode\n");
	if (clear_ondelay_mode(fd)) {
		printf("popping top module off stream (this may or may not wait)\n");
		if (stream_ioctl(fd, I_POP, 0) == -1)
			printe(true, "stream_ioctl failed");

		printf("closing the stream, this should wait for data to drain\n");
	}
	stream_close(fd);
}

staticf boolean
getcltime_check (fd, time_argp, expected_time, ret_val, expected_errno)
	int	fd;
	long	* time_argp;
	long	expected_time;
	int	ret_val, expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_GETCLTIME, time_argp)) {
	case 0:
		if (ret_val != 0)
			return printe(false, "getcltime returned 0 when expecting %d", ret_val);
		if (*time_argp == expected_time)
			return printok("getcltime retrieved time %ld as expected", *time_argp);
		return printe(false, "getcltime retrieved time %ld when expecting %ld", *time_argp, expected_time);
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("getcltime failed as expected with errno %d", errno);
			return printe(true, "getcltime failed incorrectly, expected errno %d", expected_errno);
		}
		return printe(true, "getcltime failed");
	default:
		return printe(false, "getcltime returned %d when expecting %d", i1, ret_val);
	}
}

staticf boolean
setcltime_check (fd, time_argp, ret_val, expected_errno)
	int	fd;
	long	* time_argp;
	int	ret_val, expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_SETCLTIME, time_argp)) {
	case 0:
		if (ret_val == 0)
			return printok("setcltime returned 0 as expected");
		return printe(false, "setcltime returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("setcltime failed as expected with errno %d", errno);
			return printe(true, "setcltime failed incorrectly, expected errno %d", expected_errno);
		}
		return printe(true, "setcltime failed");
	default:
		return printe(false, "setcltime returned %d when expecting %d", i1, ret_val);
	}
}
