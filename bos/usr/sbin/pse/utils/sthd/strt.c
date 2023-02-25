static char sccsid[] = "@(#)79	1.1  src/bos/usr/sbin/pse/utils/sthd/strt.c, cmdpse, bos411, 9428A410j 5/7/91 13:53:13";
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
 ** strt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"
#include <pse/echo.h>

#ifndef errno
extern	int	errno;
#endif

staticf	boolean	ioctl_check(   int fd, int cmd, void * arg, int ret_val, int expected_errno   );
staticf	boolean	str_check(   int fd, struct strioctl * strp, int ret_val, int expected_errno   );
staticf	void	transparentt(   int state   );

void
strt (state)
	int	state;
{
	char	buf[128];
	int	fd;
	iecho_t	iecho;
	struct strioctl	stri;

	printf("Testing I_STR ioctls\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	stri.ic_cmd = ECHO_IOCACK;
	stri.ic_timout = -1;
	stri.ic_len = 0;
	stri.ic_dp = buf;

	if (state & ERROR) {
		printf("testing I_STR ioctl in error state\n");
		if (set_error_state(fd, EINTR))
			str_check(fd, &stri, -1, EINTR);
		stream_close(fd);
		return;
	}

	if (state & HANGUP) {
		printf("testing I_STR ioctl in hangup state\n");
		if (set_hangup_state(fd))
			str_check(fd, &stri, -1, EINVAL);
		stream_close(fd);
		return;
	}

	printf("testing str for basic iocack\n");
	str_check(fd, &stri, 0, 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing str with bogus strioctl address\n");
	str_check(fd, BOGUS_ADDRESS, -1, EFAULT);

	printf("testing str for iocack with data and bogus data address\n");
	stri.ic_cmd = ECHO_BIGDATA;
	stri.ic_len = 10;
	stri.ic_dp = (char *)-1;
	str_check(fd, &stri, -1, EFAULT);
#endif

	printf("testing str for basic iocack with return value\n");
	stri.ic_cmd = ECHO_IOCACK | ECHO_RVAL;
	stri.ic_len = sizeof(iecho_t);
	stri.ic_dp = (char *)&iecho;
	iecho.ie_rval = 4;
	str_check(fd, &stri, 4, 0);

	printf("testing str for iocack with error\n");
	stri.ic_cmd = ECHO_IOCACK | ECHO_RERROR;
	stri.ic_len = sizeof(iecho_t);
	iecho.ie_error = ENOSPC;
	str_check(fd, &stri, -1, ENOSPC);

	printf("testing str for iocack with data and return value\n");
	stri.ic_cmd = ECHO_DATA | ECHO_RVAL;
	stri.ic_len = sizeof(iecho_t);
	iecho.ie_rval = 64;
	str_check(fd, &stri, 64, 0);

	printf("testing str for iocack with invalid len\n");
	stri.ic_cmd = ECHO_IOCACK;
	stri.ic_len = -1;
	stri.ic_dp = buf;
	str_check(fd, &stri, -1, EINVAL);

	printf("testing str for basic iocnak\n");
	stri.ic_cmd = ECHO_IOCNAK;
	stri.ic_dp = buf;
	stri.ic_len = sizeof(buf);
	str_check(fd, &stri, -1, EINVAL);

	printf("testing str for iocnak with explicit error value\n");
	stri.ic_cmd = ECHO_IOCNAK | ECHO_RERROR;
	stri.ic_dp = (char *)&iecho;
	stri.ic_len = sizeof(iecho_t);
	iecho.ie_error = ENOSPC;
	str_check(fd, &stri, -1, ENOSPC);

	printf("testing str with invalid timeout value\n");
	stri.ic_cmd = ECHO_IOCACK;
	stri.ic_timout = -2;
	str_check(fd, &stri, -1, EINVAL);

	printf("testing str for timeout iocack\n");
	stri.ic_cmd = ECHO_NOREPLY;
	stri.ic_timout = 5;
	str_check(fd, &stri, -1, ETIME);

	if (state == ONDELAY) {
		printf("testing str after read -- checking blocking behaviour\n");
		if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
			printe(true, "write failed");
			stream_close(fd);
			return;
		}
		stri.ic_cmd = ECHO_IOCACK;
		stri.ic_timout = 1;
		str_check(fd, &stri, 0, 0);
		flush_check(fd, state, 0);
	}
	stream_close(fd);

	PAUSE_HERE();
	transparentt(state);
}

staticf void
transparentt (state)
	int	state;
{
	char	buf[128];
	int	fd;
	iecho_t	ie;

	printf("Testing transparent ioctls\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	printf("testing transparent ioctl for basic iocack\n");
	buf[0] = '\0';
	ie.ie_buf = buf;
	ie.ie_len = sizeof(buf);
	ioctl_check(fd, ECHO_IOCACK, (char *)&ie, 0, 0);
	if (buf[0] != 'A')
		printe(false, "buffer not changed by ioctl, expecting 'A', got '%c'", buf[0]);

	printf("testing transparent ioctl with bogus command\n");
	ioctl_check(fd, -1, 0, -1, EINVAL);

#ifndef NO_BOGUS_ADDRESSES
	if ( (state & (ERROR | HANGUP)) == 0 ) {
		printf("testing transparent ioctl for iocack with bogus data address\n");
		ioctl_check(fd, ECHO_DATA, BOGUS_ADDRESS, -1, EFAULT);
	}
#endif

	printf("testing transparent ioctl for basic iocack with return value\n");
	buf[0] = '\0';
	ie.ie_rval = 4;
	ie.ie_buf = buf;
	ie.ie_len = sizeof(buf);
	ioctl_check(fd, ECHO_IOCACK | ECHO_RVAL, (char *)&ie, 4, 0);
	if (buf[0] != 'A')
		printe(false, "buffer not changed by ioctl, expecting 'A', got '%c'", buf[0]);

	printf("testing str for iocack with error\n");
	ie.ie_error = ENOSPC;
	ie.ie_buf = buf;
	ie.ie_len = sizeof(buf);
	ioctl_check(fd, ECHO_IOCACK | ECHO_RERROR, (char *)&ie, -1, ENOSPC);

	printf("testing str for iocack with data and return value\n");
	ie.ie_rval = 64;
	ie.ie_buf = buf;
	ie.ie_len = sizeof(buf);
	buf[0] = '\0';
	ioctl_check(fd, ECHO_DATA | ECHO_RVAL, (char *)&ie, 64, 0);
	if (buf[0] != 'A')
		printe(false, "buffer not changed by ioctl, expecting 'A', got '%c'", buf[0]);

	printf("testing str for basic iocnak\n");
	ioctl_check(fd, ECHO_IOCNAK, (char *)&ie, -1, EINVAL);

	printf("testing str for iocnak with explicit error value\n");
	ie.ie_error = ENOSPC;
	ioctl_check(fd, ECHO_IOCNAK | ECHO_RERROR, (char *)&ie, -1, ENOSPC);

#if 0	/* Do transparent ioctls timeout?? */
	printf("testing str for timeout iocack\n");
	stri.ic_cmd = ECHO_NOREPLY;
	stri.ic_timout = 1;
	str_check(fd, &stri, -1, ETIME);
#endif

	if (state == ONDELAY) {
		printf("testing str after write -- checking blocking behaviour\n");
		if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
			printe(true, "write failed");
			stream_close(fd);
			return;
		}
		buf[0] = '\0';
		ie.ie_buf = buf;
		ie.ie_len = sizeof(buf);
		ioctl_check(fd, ECHO_IOCACK, (char *)&ie, 0, 0);
		if (buf[0] != 'A')
			printe(false, "buffer not changed by ioctl, expecting 'A', got '%c'", buf[0]);
		flush_check(fd, state, 0);
	}

	stream_close(fd);
}

staticf boolean
ioctl_check (fd, cmd, arg, ret_val, expected_errno)
	int	fd;
	int	cmd;
	void	* arg;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, cmd, arg)) {
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("ioctl failed as expected with errno %d", errno);
			return printe(true, "ioctl failed incorrectly, expecting errno %d", expected_errno);
		}
		return printe(true, "ioctl failed");
	default:
		if (i1 == ret_val)
			return printok("ioctl returned %d as expected", ret_val);
		return printe(false, "ioctl returned %d when expecting %d", i1, ret_val);
	}
}

staticf boolean
str_check (fd, strp, ret_val, expected_errno)
	int	fd;
	struct strioctl	* strp;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_STR, strp)) {
	case -1:
		if (ret_val == -1) {
			if (errno == expected_errno)
				return printok("str failed as expected with errno %d", errno);
			return printe(true, "str failed incorrectly, expecting errno %d", expected_errno);
		}
		return printe(true, "str failed");
	default:
		if (i1 == ret_val)
			return printok("str returned %d as expected", ret_val);
		return printe(false, "str returned %d when expecting %d", i1, ret_val);
	}
}
