static char sccsid[] = "@(#)66	1.1  src/bos/usr/sbin/pse/utils/sthd/peekt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:46";
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
 ** peekt.c 2.2, last change 4/9/91
 **/


#include <pse/clib.h>
#include <pse/common.h>
#include "sthd.h"

#ifndef errno
extern	int	errno;
#endif

static	boolean	peek_check(   int fd, struct strpeek * strp, int expected_len, int ret_val, int errno_val   );

void
peekt (state)
	int	state;
{
	char	buf[256];
	struct strbuf	databuf;
	int	fd;
	int	fd2;
	struct strpeek	strp;
	struct strrecvfd	strr;

	printf("Testing peek ioctl\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}
	strp.ctlbuf.buf = buf;
	strp.ctlbuf.maxlen = sizeof(buf);
	strp.ctlbuf.len = 0;
	strp.databuf.buf = buf;
	strp.databuf.maxlen = sizeof(buf);
	strp.databuf.len = sizeof(buf);

	if (state & ERROR) {
		printf("testing peek in error state\n");
		if (set_error_state(fd, ENXIO))
			peek_check(fd, &strp, 0, -1, ENXIO);
		stream_close(fd);
		return;
	}

	if (!(state & HANGUP)) {
	        if ((fd2 = stream_open(echo_name, 2)) == -1)
        	        fatal("stream_open of %s failed, %s\n", echo_name, errmsg(0));
		printf("doing sendfd for peek test\n");
		if (stream_ioctl(fd, I_SENDFD, fd2) != -1) {
			printf("testing peek with PASSFP message on queue\n");
			strp.flags = 0;
			peek_check(fd, &strp, 0, -1, EBADMSG);
			if (stream_ioctl(fd, I_RECVFD, &strr) == -1) {
				printe(true, "couldn't read PASSFP message");
				stream_close(fd);
				return;
			}
			stream_close(fd2);
			stream_close(strr.fd);
		} else {
			printe(true, "sendfd failed");
			stream_close(fd2);
			stream_close(fd);
			return;
		}
	}

	printf("doing putmsg for peek test\n");
	databuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.maxlen = sizeof(buf);
	if (putmsg(fd, nilp(struct strbuf), &databuf, 0) == 0) {
		printf("put %d data bytes\n", sizeof(buf));
	} else {
		printe(true, "putmsg failed");
		return;
	}

	if (state & HANGUP  &&  !set_hangup_state(fd)) {
		stream_close(fd);
		return;
	}

	printf("testing RS_HIPRI peek with no such message available\n");
	strp.flags = RS_HIPRI;
	strp.databuf.maxlen = sizeof(buf);
	peek_check(fd, &strp, 0, 0, 0);

	printf("testing good peek\n");
	strp.flags = 0;
	peek_check(fd, &strp, sizeof(buf), 1, 0);
	nread_check(fd, 1, sizeof(buf), 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing peek with bogus databuf.buf value\n");
	strp.databuf.buf = BOGUS_ADDRESS;
	peek_check(fd, &strp, 0, -1, EFAULT);
	strp.databuf.buf = buf;
#endif

	printf("testing peek with bogus flags\n");
	strp.flags = -1;
	peek_check(fd, &strp, sizeof(buf), 0, 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing peek with bogus strpeek structure address\n");
	peek_check(fd, BOGUS_ADDRESS, 0, -1, EFAULT);
#endif

	printf("testing peek on empty queue\n");
	flush_check(fd, state, 0);
	peek_check(fd, &strp, 0, 0, 0);

	stream_close(fd);
}

static boolean
peek_check (fd, strp, expected_len, ret_val, errno_val)
	int	fd;
	struct strpeek	* strp;
	int	expected_len;
	int	ret_val, errno_val;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_PEEK, strp)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "peek failed");
		if (errno_val != errno)
			return printe(true, "peek failed incorrectly");
		printf("peek failed as expected with errno %d\n", errno_val);
		break;
	case 1:
		if (ret_val != 1)
			return printe(false, "peek returned 1 when %d expected", ret_val);
		if (strp->databuf.len != expected_len)
			return printe(false, "peek should have returned %d len not %d",expected_len, strp->databuf.len);
		printf("peek returned 1 and got %d bytes as expected\n", expected_len);
		break;
	default:
		if (ret_val != i1)
			return printe(false, "peek returned %d when %d expected", i1, ret_val);
		printf("peek returned %d as expected\n", i1);
		break;
	}
	return true;
}
