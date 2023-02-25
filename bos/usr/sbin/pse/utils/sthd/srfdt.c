static char sccsid[] = "@(#)76	1.1  src/bos/usr/sbin/pse/utils/sthd/srfdt.c, cmdpse, bos411, 9428A410j 5/7/91 13:53:04";
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
 ** srfdt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"

#define	TEST_STR	"hello from the passed file descriptor"

#ifndef errno
extern	int	errno;
#endif

staticf	boolean	recvfd_check(   int fd, struct strrecvfd * strrp, int ret_val, int expected_errno   );
staticf	boolean	sendfd_check(   int fd, int arg_fd, int ret_val, int expected_errno   );
staticf	void	srfd_nonstream(   int fd, int state   );
staticf	void	srfd_stream(   int fd, int state   );
static	char	msg[] = "Hi there, this is a test!";

void
srfdt (state)
	int	state;
{
	int	fd;
	struct strrecvfd strr;

	printf("\nTesting send/recv fd\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	/* TODO: add ERROR and HANGUP testing */

	printf("testing send fd with bogus fd value\n");
	sendfd_check(fd, -1, -1, EBADF);

#ifdef	NON_STREAM_SENDFD_OK
	srfd_nonstream(fd, state);
#endif
	srfd_stream(fd, state);
	if (state & ONDELAY) {
		printf("testing ONDELAY recv fd\n");
		recvfd_check(fd, &strr, -1, EAGAIN);
	}
	stream_close(fd);
}

staticf	void
srfd_stream (fd, state)
	int	fd;
	int	state;
{
	struct strbuf ctrl, data;
	char	buf[128];
	char	cbuf[256], dbuf[256];
	int	fd2, fd3;
	int	flags;
	boolean	msg_eaten;
	struct strrecvfd rfd;

	printf("testing send/recv fd using streams files\n");
	if ( (fd2 = stream_open(echo_name, 0)) == -1 ) {
		printe(true, "open of %s failed", echo_name);
		return;
	}
	printf("testing good send fd\n");
	sendfd_check(fd, fd2, 0, 0);
	nread_check(fd, 1, 0, 0);

#ifndef NO_BOGUS_ADDRESSES
#ifdef	EARLY_ADDR_CHECK
	/* The bogus address in the following test is expected to be detected before the PASSFP message is removed
	** from the stream head queue.  In this case, we can proceed to test the valid RECVFD on that message.
	*/
	msg_eaten = false;
#else
	/* The bogus address in the following test is expected to be detected during the copyout operation after the
	** PASSFP message has been removed from the stream head queue.  In this case, we will need to repeat the SENDFD
	** in order to test the real RECVFD.  Note that in this case, the file associated with fd2 will not really get
	** closed until the process exits, so below we abandon fd2 just to keep the test clean.
	*/
	msg_eaten = true;
#endif
	printf("testing recv fd with bogus address\n");
	recvfd_check(fd, BOGUS_ADDRESS, -1, EFAULT);
	nread_check(fd, msg_eaten ? 0 : 1, 0, 0);
#else
	msg_eaten = false;
#endif

	if ( msg_eaten ) {
		/* Get a new stream for fd2 (see note above). */
		if ( (fd2 = stream_open(echo_name, 0)) == -1 ) {
			printe(true, "open of %s failed", echo_name);
			return;
		}
		/* Redo the SENDFD so we have something to receive. */
		printf("testing good send fd again\n");
		sendfd_check(fd, fd2, 0, 0);
		nread_check(fd, 1, 0, 0);
	}

	printf("testing good recv fd with passed stream still open\n");
	if (recvfd_check(fd, &rfd, 0, 0)) {
		ctrl.len = -1;
		data.buf = msg;
		data.len = sizeof(msg);
		if ( putmsg(rfd.fd, &ctrl, &data, 0) == -1 )
			printe(true, "putmsg failed on received fd %d", rfd.fd);
		else {
			ctrl.buf = cbuf;
			ctrl.maxlen = sizeof(cbuf);
			data.buf = dbuf;
			data.maxlen = sizeof(dbuf);
			flags = 0;
			if ( getmsg(rfd.fd, &ctrl, &data, &flags) == -1 )
				printe(true, "getmsg failed on received fd %d", rfd.fd);
			else {
				dbuf[data.len] = '\0';
				printf("passed fd sez: %s\n", dbuf);
			}
		}
		stream_close(rfd.fd);
	}

	if ( (fd3 = stream_open(echo_name, 0)) != -1 ) {
		/* In this test, we do a successful sendfd, but the far side goes away without ever doing a recvfd. */
		printf("testing another good send fd\n");
		sendfd_check(fd3, fd2, 0, 0);
		nread_check(fd3, 1, 0, 0);

		printf("closing receiving stream without doing recvfd\n");
		stream_close(fd3);
	} else
		printf("couldn't open %s, %s\n", echo_name, errmsg(0));

	ctrl.len = -1;
	data.buf = msg;
	data.len = sizeof(msg);
	printf("testing send fd with data on the stream\n");
	if ( putmsg(fd2, &ctrl, &data, 0) != -1 ) {
		if ( sendfd_check(fd, fd2, 0, 0) ) {
			nread_check(fd, 1, 0, 0);
			printf("closing passed stream this time\n");
			stream_close(fd2);
			printf("testing good recv fd with passed stream closed by sender\n");
			if ( recvfd_check(fd, &rfd, 0, 0)) {
				ctrl.len = -1;
				data.buf = msg;
				data.len = sizeof(msg);
				ctrl.buf = cbuf;
				ctrl.maxlen = sizeof(cbuf);
				data.buf = dbuf;
				data.maxlen = sizeof(dbuf);
				flags = 0;
				if ( getmsg(rfd.fd, &ctrl, &data, &flags) != -1 ) {
					dbuf[data.len] = '\0';
					printf("passed fd sez: %s\n", dbuf);
				} else
					printe(true, "getmsg failed on received fd %d", rfd.fd);
				stream_close(rfd.fd);
			}
		} else
			stream_close(fd2);
	} else {
		printe(true, "putmsg failed");
		stream_close(fd2);
	}

	printf("testing recv fd with regular data on read queue\n");
	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		if ( (fd3 = stream_open(echo_name, 0)) != -1 ) {
			if (sendfd_check(fd, fd3, 0, 0))
				recvfd_check(fd, &rfd, -1, EBADMSG);
			stream_close(fd3);
		} else
			printe(true, "couldn't open %s", echo_name);
	} else
		printe(true, "write failed");
	flush_check(fd, state, 0);
	nread_check(fd, 0, 0, 0);
}

staticf	void
srfd_nonstream (fd, state)
	int	fd;
	int	state;
{
	char	buf[128];
	struct strrecvfd strr;
	
	printf("testing send/recv fd using non-streams files\n");
	printf("testing good send fd\n");
	sendfd_check(fd, 0, 0, 0);
	nread_check(fd, 1, 0, 0);

	printf("testing good recv fd\n");
	if (recvfd_check(fd, &strr, 0, 0)) {
		if (stream_write(strr.fd, TEST_STR, sizeof(TEST_STR)) != sizeof(TEST_STR))
			printe(false, "recv fd got bogus file descriptor %d", strr.fd);
		else
			printf("passed fd is ok\n");
		stream_close(strr.fd);
	}

	printf("testing recv fd with regular data on read queue\n");
	if (stream_write(fd, buf, sizeof(buf) == sizeof(buf))) {
		if (sendfd_check(fd, 0, 0, 0))
			recvfd_check(fd, &strr, -1, EBADMSG);
	} else
		printe(true, "write failed");
	flush_check(fd, state, 0);
	nread_check(fd, 0, 0, 0);
}

static boolean
recvfd_check (fd, strrp, ret_val, expected_errno)
	int	fd;
	struct strrecvfd * strrp;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_RECVFD, (char *)strrp)) {
	case 0:
		if (ret_val != 0)
			return printe(false, "recv fd successful when expecting %d", ret_val);
		return printok("recv fd successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "recv fd failed");
		if (errno != expected_errno)
			return printe(true, "recv fd failed incorrectly, expecting errno %d", expected_errno);
		return printok("recv fd failed as expected with errno %d", errno);
	default:
		if (i1 != ret_val)
			return printe(false, "recv fd returned %d when expecting %d", i1, ret_val);
		return printok("recv fd returned %d as expected", i1);
	}
}

static boolean
sendfd_check (fd, arg_fd, ret_val, expected_errno)
	int	fd;
	int	arg_fd;
	int	ret_val;
	int	expected_errno;
{
	int	i1;

	switch (i1 = stream_ioctl(fd, I_SENDFD, arg_fd)) {
	case 0:
		if (ret_val != 0)
			return printe(false, "send fd successful when expecting %d", ret_val);
		return printok("send fd successful");
	case -1:
		if (ret_val != -1)
			return printe(true, "send fd failed");
		if (errno != expected_errno)
			return printe(true, "send fd failed incorrectly, expecting errno %d", expected_errno);
		return printok("send fd failed as expected with errno %d", errno);
	default:
		if (i1 != ret_val)
			return printe(false, "send fd returned %d when expecting %d", i1, ret_val);
		return printok("send fd returned %d as expected", i1);
	}
}
