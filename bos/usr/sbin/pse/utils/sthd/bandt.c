static char sccsid[] = "@(#)54	1.1  src/bos/usr/sbin/pse/utils/sthd/bandt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:24";
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
 ** bandt.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>
#include "sthd.h"

staticf	boolean	canput_check(   int fd, int band, int ret_val, int errno_val   );
staticf	void	canputt(   int fd   );
staticf	boolean	ckband_check(   int fd, int band, int ret_val, int errno_val   );
staticf	void	ckbandt(   int fd   );
	boolean	flushband_check(   int fd, struct bandinfo * bi, int ret_val, int errno_val   );
staticf	void	flushbandt(   int fd   );
staticf	boolean	getband_check(   int fd, int * bandp, int ret_val, int errno_val   );
staticf	void	getbandt(   int fd   );

#ifndef errno
extern	int	errno;
#endif
extern	char	* echo_name;
extern	char	* nuls_name;
extern	char	* pass_name;

void
bandt (state)
	int	state;
{
	int	fd;
	char	buf[128];
	struct strbuf	ctlbuf;
	struct strbuf	databuf;

	printf("Testing band ioctls\n");
/* TODO: add ondelay, error and hangup testing... */

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of '%s' failed", echo_name);
		return;
	}
	if (stream_ioctl(fd, I_PUSH, pass_name) == -1) {
		printe(true, "push of '%s' failed", pass_name);
		return;
	}

	canputt(fd);
	PAUSE_HERE();

	ckbandt(fd);
	PAUSE_HERE();

	flushbandt(fd);
	PAUSE_HERE();

	getbandt(fd);

	stream_close(fd);
}

staticf void
canputt (fd)
	int	fd;
{
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	struct bandinfo	bi;
	char	buf[1024];
	int	cnt;
	int	i1;

	printf("testing canput ioctl with bad file descriptor (-1)\n");
	canput_check(-1, -1, -1, EBADF);

	printf("testing canput ioctl on non-existent band %d\n", DEFAULT_BAND+20);
	canput_check(fd, DEFAULT_BAND+20, 1, 0);

	printf("testing valid canput ioctl on band %d\n", 0);
	canput_check(fd, 0, 1, 0);

	printf("testing valid canput ioctl on band %d\n", DEFAULT_BAND);
	canput_check(fd, DEFAULT_BAND, 1, 0);

	ctlbuf.len = sizeof(buf);
	ctlbuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.buf = buf;
	printf("looping doing putpmsg until canput fails\n");
	cnt = 0;
	while ((i1 = stream_ioctl(fd, I_CANPUT, (char *)DEFAULT_BAND)) == 1) {
		if (!putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0))
			return;
		cnt++;
	}
	if (i1 == 0)
		printf("canput returned 0 after %d messages\n", cnt);
	else if (i1 == -1)
		printe(true, "canput failed");

	bi.bi_pri = DEFAULT_BAND;
	bi.bi_flag = FLUSHRW;
	flushband_check(fd, &bi, 0, 0);
	nread_check(fd, 0, 0, 0);

	printf("testing canput after flushband on band %d\n", DEFAULT_BAND);
	canput_check(fd, DEFAULT_BAND, 1, 0);
}

staticf void
ckbandt (fd)
	int	fd;
{
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	struct bandinfo	bi;
	char	buf[128];

	printf("testing ckband with bad file descriptor (-1)\n");
	ckband_check(-1, 0, -1, EBADF);

	nread_check(fd, 0, 0, 0);	/* just to make sure */

	printf("testing ckband with band %d (no message should be available)\n", DEFAULT_BAND);
	ckband_check(fd, DEFAULT_BAND, 0, 0);

	printf("testing putpmsg to band %d\n", DEFAULT_BAND);
	ctlbuf.len = sizeof(buf);
	ctlbuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.buf = buf;
	if (!putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0))
		return;
	printf("testing valid ckband with band %d\n", DEFAULT_BAND);
	ckband_check(fd, DEFAULT_BAND, 1, 0);

	bi.bi_pri = DEFAULT_BAND;
	bi.bi_flag = FLUSHRW;
	flushband_check(fd, &bi, 0, 0);

	printf("testing ckband with band %d (no message should be available)\n", DEFAULT_BAND);
	ckband_check(fd, DEFAULT_BAND, 0, 0);

	printf("testing ckband with band 0 (no message should be available)\n");
	ckband_check(fd, 0, 0, 0);

	printf("doing write for ckband test on band 0\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	printf("testing valid ckband with band 0\n");
	ckband_check(fd, 0, 1, 0);

	flush_check(fd, 0, 0);

	printf("testing ckband with band 0 (no message should be available)\n");
	ckband_check(fd, 0, 0, 0);
}

staticf void
flushbandt (fd)
	int	fd;
{
	struct bandinfo	bi;
	char	buf[128];
	struct strbuf	ctlbuf;
	struct strbuf	databuf;

	printf("testing flushband with bad file descriptor (-1)\n");
	flushband_check(-1, &bi, -1, EBADF);

	printf("testing flushband with bad flags\n");
	bi.bi_flag = -1;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, -1, EINVAL);

	printf("testing putpmsg to band %d\n", DEFAULT_BAND);
	ctlbuf.len = sizeof(buf);
	ctlbuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.buf = buf;
	if (!putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0))
		return;
	nread_check(fd, 1, sizeof(buf), 0);
	printf("testing flushband on band %d\n", DEFAULT_BAND);
	bi.bi_flag = FLUSHRW;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);
	nread_check(fd, 0, 0, 0);
}

staticf void
getbandt (fd)
	int	fd;
{
	int	band;
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	struct bandinfo	bi;
	char	buf[128];

	printf("testing getband with bad file descriptor (-1)\n");
	getband_check(-1, nilp(int), -1, EBADF);

	printf("testing getband when no message should be available\n");
	band = -1;
	getband_check(fd, &band, -1, ENODATA);

	printf("testing putpmsg to band %d\n", DEFAULT_BAND);
	ctlbuf.len = sizeof(buf);
	ctlbuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.buf = buf;
	if (!putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0))
		return;
	printf("testing valid getband with band %d\n", DEFAULT_BAND);
	band = -1;
	getband_check(fd, &band, 0, 0);
	if (band != DEFAULT_BAND)
		printe(false, "getband returned band %d, not %d", band, DEFAULT_BAND);
	else
		printf("getband returned band %d as expected\n", DEFAULT_BAND);

	bi.bi_pri = DEFAULT_BAND;
	bi.bi_flag = FLUSHRW;
	flushband_check(fd, &bi, 0, 0);

	printf("doing write for getband test on band 0\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	band = -1;
	getband_check(fd, &band, 0, 0);
	if (band != 0)
		printe(false, "getband returned band %d, not 0", band);
	else
		printf("getband returned band 0 as expected\n");

	flush_check(fd, 0, 0);

	printf("doing RS_HIPRI putmsg for getband test on band 0\n");
	if (putmsg(fd, &ctlbuf, &databuf, RS_HIPRI) == -1) {
		printe(true, "putmsg failed");
		return;
	}
	band = -1;
	getband_check(fd, &band, 0, 0);
	if (band != 0)
		printe(false, "getband returned band %d, not 0", band);
	else
		printf("getband returned band 0 as expected\n");

	flush_check(fd, 0, 0);
}

staticf boolean
canput_check (fd, band, ret_val, errno_val)
	int	fd;
	int	band;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_CANPUT, (char *)band)) {
	case 0:
		if (ret_val == 0)
			return printok("canput returned 0 as expected");
		return printe(false, "canput returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("canput failed as expected with errno %d", errno);
			return printe(true, "canput failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "canput failed");
	default:
		if (ret_val == i1)
			return printok("canput returned %d as expected", i1);
		return printe(false, "canput returned %d when expecting %d", i1, ret_val);
	}
}

staticf boolean
ckband_check (fd, band, ret_val, errno_val)
	int	fd;
	int	band;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_CKBAND, (char *)band)) {
	case 0:
		if (ret_val == 0)
			return printok("ckband returned 0 as expected");
		return printe(false, "ckband returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("ckband failed as expected with errno %d", errno);
			return printe(true, "ckband failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "ckband failed");
	default:
		if (ret_val == i1)
			return printok("ckband returned %d as expected", i1);
		return printe(false, "ckband returned %d when expecting %d", i1, ret_val);
	}
}

boolean
flushband_check (fd, bi, ret_val, errno_val)
	int	fd;
	struct bandinfo	* bi;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_FLUSHBAND, (char *)bi)) {
	case 0:
		if (ret_val == 0)
			return printok("flushband returned 0 as expected");
		return printe(false, "flushband returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("flushband failed as expected with errno %d", errno);
			return printe(true, "flushband failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "flushband failed");
	default:
		if (ret_val == i1)
			return printok("flushband returned %d as expected", i1);
		return printe(false, "flushband returned %d when expecting %d", i1, ret_val);
	}
}

staticf boolean
getband_check (fd, bandp, ret_val, errno_val)
	int	fd;
	int	* bandp;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_GETBAND, (char *)bandp)) {
	case 0:
		if (ret_val == 0)
			return true;
		return printe(false, "getband returned 0 when expecting %d", ret_val);
	case -1:
		if (ret_val == -1) {
			if (errno == errno_val)
				return printok("getband failed as expected with errno %d", errno);
			return printe(true, "getband failed incorrectly, expected errno %d", errno_val);
		}
		return printe(true, "getband failed");
	default:
		return printe(false, "getband returned %d when expecting %d", i1, ret_val);
	}
}
