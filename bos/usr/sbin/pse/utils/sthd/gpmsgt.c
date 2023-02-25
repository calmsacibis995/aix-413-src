static char sccsid[] = "@(#)59	1.1  src/bos/usr/sbin/pse/utils/sthd/gpmsgt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:34";
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
 ** gpmsgt.c 2.2, last change 4/9/91
 **/


#include <pse/clib.h>
#include <pse/common.h>
#include "sthd.h"

#define	NILS	(nilp(struct strbuf))

#ifndef errno
extern	int	errno;
#endif

	boolean	getmsg_check(   int fd, struct strbuf * ctlbuf, struct strbuf * databuf, int * fp, int ctl_len, int data_len, int ret_val, int errno_val   );
	boolean	putmsg_check(   int fd, struct strbuf * ctlbuf, struct strbuf * databuf, int flags, int ret_val, int expected_errno   );

void
gpmsgt (state)
	int	state;
{
	char	buf[MAX_STACK_BUF];
	int	fd;
	int	fd2;
	int	flags;
	boolean	msg_eaten;
	struct strbuf	ctlbuf, databuf;
	struct strrecvfd	strr;

	printf("Testing getmsg and putmsg\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		printf("Testing putmsg and getmsg in ERROR state\n");
		if (set_error_state(fd, ENXIO)) {
			putmsg_check(fd, NILS, &databuf, -1, -1, ENXIO);
			getmsg_check(fd, &ctlbuf, &databuf, &flags, 0, 0, -1, ENXIO);
		}
		stream_close(fd);
		return;
	}
	if (state & ONDELAY) {
		printf("setting ondelay mode\n");
		if (!set_ondelay_mode(fd))
			return;
	}

	printf("Testing putmsg\n");

	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	ctlbuf.maxlen = sizeof(buf);
	databuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.maxlen = sizeof(buf);

	printf("testing putmsg with bogus flags value\n");
	putmsg_check(fd, NILS, &databuf, -1, -1, EINVAL);

	printf("testing putmsg of %d control bytes, %d data bytes\n", sizeof(buf), sizeof(buf));
	putmsg_check(fd, &ctlbuf, &databuf, 0, 0, 0);

	printf("testing putmsg of RS_HIPRI %d control bytes, %d data bytes\n", sizeof(buf), sizeof(buf));
	putmsg_check(fd, &ctlbuf, &databuf, RS_HIPRI, 0, 0);

	printf("testing putmsg with RS_HIPRI and no control part\n");
	putmsg_check(fd, NILS, &databuf, RS_HIPRI, -1, EINVAL);

	printf("testing putmsg of %d data bytes\n", sizeof(buf) / 2);
	databuf.len = sizeof(buf) / 2;
	putmsg_check(fd, NILS, &databuf, 0, 0, 0);

	if (state & HANGUP) {
		printf("setting hangup state\n");
		if (!set_hangup_state(fd)) {
			stream_close(fd);
			return;
		}
		printf("testing putmsg in HANGUP state\n");
		putmsg_check(fd, NILS, &databuf, -1, -1, EINVAL);
	}

	printf("testing getmsg to get control and data\n");
	flags = RS_HIPRI;
	getmsg_check(fd, &ctlbuf, &databuf, &flags, sizeof(buf) ,sizeof(buf), 0, 0);

	printf("testing getmsg to get just data\n");
	flags = 0;
	getmsg_check(fd, NILS, &databuf, &flags, 0, sizeof(buf), MORECTL, 0);
	nread_check(fd, 2, 0, 0);

	printf("testing getmsg to get the control part\n");
	ctlbuf.maxlen = sizeof(buf);
	flags = 0;
	getmsg_check(fd, &ctlbuf, &databuf, &flags, sizeof(buf), -1, 0, 0);
	nread_check(fd, 1, (sizeof(buf) / 2), 0);

	printf("testing getmsg with -1 data maxlen\n");
	databuf.maxlen = -1;
	flags = 0;
	getmsg_check(fd, NILS, &databuf, &flags, 0, -1, MOREDATA, 0);

	printf("testing regular getmsg that should return MOREDATA\n");
	databuf.maxlen = sizeof(buf) / 4;
	flags = 0;
	getmsg_check(fd, NILS, &databuf, &flags, 0, sizeof(buf) / 4, MOREDATA, 0);
	nread_check(fd, 1, sizeof(buf) / 4, 0);

#ifndef NO_BOGUS_ADDRESSES
#ifdef	EARLY_ADDR_CHECK
	/* The bogus address in the following test is expected to be detected before a message
	** is removed from the stream head queue.  In this case, we will need to read that message.
	*/
	msg_eaten = false;
#else
	/* The bogus address in the following test is expected to be detected during the copyout
	** operation after the message is removed from the stream head queue.  In this case, we don't
	** worry about rereading that message.
	*/
	msg_eaten = true;
#endif
	printf("testing getmsg with bogus databuf.buf address\n");
	databuf.buf = BOGUS_ADDRESS;
	databuf.maxlen = sizeof(buf) / 4;
	flags = 0;
	getmsg_check(fd, NILS, &databuf, &flags, 0, 0, -1, EFAULT);
	nread_check(fd, msg_eaten ? 0 : 1, msg_eaten ? 0 : sizeof(buf) / 4, 0);
	databuf.buf = buf;
#else
	msg_eaten = false;
#endif

	if ( !msg_eaten ) {
		/* Read the message left over from the bogus address check. */
		databuf.maxlen = sizeof(buf) / 4;
		flags = 0;
		getmsg_check(fd, NILS, &databuf, &flags, 0, sizeof(buf) / 4, 0, 0);
		nread_check(fd, 0, 0, 0);
	}

	PAUSE_HERE();
	printf("testing putmsg of RS_HIPRI %d control bytes, %d data bytes\n", sizeof(buf), sizeof(buf));
	databuf.len = sizeof(buf);
	if (putmsg_check(fd, &ctlbuf, &databuf, RS_HIPRI, 0, 0)) {
		printf("testing getmsg of control part of RS_HIPRI message\n");
		ctlbuf.maxlen = sizeof(buf);
		ctlbuf.buf = buf;
		databuf.maxlen = -1;
		flags = RS_HIPRI;
		getmsg_check(fd, &ctlbuf, &databuf, &flags, sizeof(buf), -1, MOREDATA, 0);
		nread_check(fd, 1, sizeof(buf), 0);

		printf("now using getmsg to get the data part, expecting RS_HIPRI\n");
		ctlbuf.maxlen = -1;
		databuf.maxlen = sizeof(buf);
		databuf.buf = buf;
		flags = RS_HIPRI;
		getmsg_check(fd, &ctlbuf, &databuf, &flags, -1, sizeof(buf), 0, 0);
		nread_check(fd, 0, 0, 0);
		PAUSE_HERE();
	}

	printf("testing getmsg with bogus flags\n");
	flags = -1;
	databuf.maxlen = 0;
	getmsg_check(fd, NILS, &databuf, &flags, 0, 0, -1, EINVAL);
	nread_check(fd, 0, 0, 0);
	flags = 0;

	if (state & HANGUP) {
		printf("testing HANGUP getmsg on empty read queue\n");
		flags = 0;
		getmsg_check(fd, &ctlbuf, &databuf, &flags, 0, 0, 0, 0);
		stream_close(fd);
		return;
	}

	if (state & ONDELAY) {
		printf("testing ONDELAY getmsg on empty stream\n");
		getmsg_check(fd, &ctlbuf, &databuf, &flags, 0, 0, -1, EAGAIN);
	}

	printf("testing getmsg with 0 control maxlen\n");
	if (stream_write(fd, buf, sizeof(buf)) > 0) {
		printf("wrote %d bytes\n", sizeof(buf));
		ctlbuf.buf = buf;
		ctlbuf.maxlen = 0;
		databuf.maxlen = sizeof(buf);
		getmsg_check(fd, &ctlbuf, &databuf, &flags, -1, sizeof(buf), 0, 0);
		nread_check(fd, 0, 0);
	} else
		printe(true, "write failed");

	printf("testing getmsg after 0 length write\n");
	if (stream_write(fd, buf, 0) == 0) {
		flags = 0;
		getmsg_check(fd, NILS, &databuf, &flags, 0, 0, 0, 0);
		nread_check(fd, 0, 0);
	} else
		printe(true, "write failed");

	printf("testing getmsg with non-PROTO or DATA message on read queue\n");
	if ((fd2 = stream_open(nuls_name, 2)) == -1) {
		printe(true, "stream_open of %s failed", nuls_name);
		stream_close(fd);
		return;
	}
	if (stream_ioctl(fd, I_SENDFD, fd2) == 0) {
		flags = 0;
		getmsg_check(fd, NILS, &databuf, &flags, 0, 0, -1, EBADMSG);
		if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
			printe(true, "flush failed");
		if (!nread_check(fd, 0, 0, 0)) {
			printe(false, "flush didn't get rid of SENDFD message");
			if (stream_ioctl(fd, I_RECVFD, &strr) != -1)
				stream_close(strr.fd);
		}
	} else
		printe(true, "send fd ioctl failed");
	stream_close(fd2);

	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");
	nread_check(fd, 0, 0, 0);

	stream_close(fd);
}

boolean
getmsg_check (fd, ctlbuf, databuf, fp, ctl_len,	data_len, ret_val, errno_val)
	int	fd;
	struct strbuf	* ctlbuf;
	struct strbuf	* databuf;
	int	* fp;
	int	ctl_len;
	int	data_len;
	int	ret_val;
	int	errno_val;
{
	int	i1, flags;

	flags = *fp;
	switch (i1 = getmsg(fd, ctlbuf, databuf, fp)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "getmsg failed");
		if (errno != errno_val)
			return printe(true, "getmsg failed incorrectly");
		return printok("getmsg failed as expected with errno %d", errno);
	default:
		if (i1 != ret_val)
			return printe(false, "getmsg should have returned 0x%x not 0x%x",ret_val, i1);
		if ((databuf  &&  databuf->len != data_len)
		||  (ctlbuf  &&  ctlbuf->len != ctl_len)
		||  *fp != flags)
			return printe(false, "getmsg should have returned %d ctl %d data %d flags, not %d %d %d",
				ctl_len,data_len,flags,ctlbuf ? ctlbuf->len:0, databuf->len, *fp);
		return printok("getmsg got %d ctl bytes and %d data bytes as expected",ctl_len, data_len);
	}
}

boolean
putmsg_check (fd, ctlbuf, databuf, flags, ret_val, errno_val)
	int	fd;
	struct strbuf	* ctlbuf;
	struct strbuf	* databuf;
	int	flags;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = putmsg(fd, ctlbuf, databuf, flags)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "putmsg failed");
		if (errno != errno_val)
			return printe(true, "putmsg failed incorrectly");
		return printok("putmsg failed as expected with errno %d", errno);
	case 0:
		if (ret_val == 0)
			return printok("putmsg put %d ctl bytes and %d data bytes",ctlbuf ? ctlbuf->len : 0, databuf ? databuf->len : 0);
		fallthru;
	default:
		return printe(false, "putmsg should have returned %d not %d",ret_val, i1);
	}
}
