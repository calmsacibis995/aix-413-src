static char sccsid[] = "@(#)60	1.1  src/bos/usr/sbin/pse/utils/sthd/gppmsgt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:36";
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
 ** gppmsgt.c 2.2, last change 4/9/91
 **/


#include <pse/clib.h>
#include <pse/common.h>
#include "sthd.h"

#define	NILS	(nilp(struct strbuf))

#ifndef errno
extern	int	errno;
#endif

	boolean	getpmsg_check(   int fd, struct strbuf * ctlbuf, struct strbuf * databuf, int * bandp,
		int * fp, int ctl_len, int data_len, int ret_val, int errno_val   );
	boolean	putpmsg_check(   int fd, struct strbuf * ctlbuf, struct strbuf * databuf, int band,
		int flags, int ret_val, int errno_val   );

void
gppmsgt (state)
	int	state;
{
	int	band;
	char	buf[512];
	int	fd;
	int	fd2;
	int	flags;
	boolean	msg_eaten;
	struct strbuf	ctlbuf, databuf;
	struct strrecvfd	strr;

	printf("Testing putpmsg and getpmsg\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (state & ERROR) {
		printf("Testing putpmsg and getpmsg in ERROR state\n");
		if (set_error_state(fd, ENXIO)) {
			putpmsg_check(fd, NILS, &databuf, 0, -1, -1, ENXIO);
			getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, 0, 0, -1, ENXIO);
		}
		stream_close(fd);
		return;
	}
	if (state & ONDELAY) {
		printf("setting ondelay mode\n");
		if (!set_ondelay_mode(fd)) {
			stream_close(fd);
			return;
		}
	}

	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	ctlbuf.maxlen = sizeof(buf);
	databuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.maxlen = sizeof(buf);

	printf("testing putpmsg with bogus flags value (-1)\n");
	putpmsg_check(fd, NILS, &databuf, 0, -1, -1, EINVAL);

	printf("testing putpmsg with bogus flags value (0)\n");
	putpmsg_check(fd, NILS, &databuf, 0, 0, -1, EINVAL);

	printf("testing putpmsg with bogus band value\n");
	putpmsg_check(fd, NILS, &databuf, -1, -1, -1, EINVAL);

	printf("testing putpmsg of %d control bytes, %d data bytes, band %d\n", sizeof(buf), sizeof(buf), DEFAULT_BAND);
	putpmsg_check(fd, &ctlbuf, &databuf, DEFAULT_BAND, MSG_BAND, 0, 0);

	printf("testing putpmsg of MSG_HIPRI %d control bytes, %d data bytes, band 0\n", sizeof(buf), sizeof(buf));
	putpmsg_check(fd, &ctlbuf, &databuf, 0, MSG_HIPRI, 0, 0);

	PAUSE_HERE();
	printf("testing putpmsg with MSG_HIPRI and band %d\n", DEFAULT_BAND);
	putpmsg_check(fd, NILS, &databuf, DEFAULT_BAND, MSG_HIPRI, -1, EINVAL);

	printf("testing putpmsg with MSG_HIPRI and no control part\n");
	putpmsg_check(fd, NILS, &databuf, 0, MSG_HIPRI, -1, EINVAL);

	printf("testing putpmsg with MSG_BAND and no control or data parts\n");
	putpmsg_check(fd, NILS, NILS, DEFAULT_BAND, MSG_BAND, 0, 0);

	printf("testing putpmsg of %d data bytes, band 0\n", sizeof(buf) / 2);
	databuf.len = sizeof(buf) / 2;
	putpmsg_check(fd, NILS, &databuf, 0, MSG_BAND, 0, 0);

	if (state & HANGUP) {
		printf("setting hangup state\n");
		if (!set_hangup_state(fd)) {
			stream_close(fd);
			return;
		}
		printf("testing putpmsg in hangup state\n");
		putpmsg_check(fd, NILS, &databuf, 0, MSG_BAND, -1, EINVAL);
	}

	PAUSE_HERE();
	printf("Testing getpmsg()\n");

	printf("testing getpmsg to get control and data on MSG_HIPRI\n");
	flags = MSG_HIPRI;
	band = 0;
	if (getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, sizeof(buf), sizeof(buf), 0, 0)) {
		if (flags != MSG_HIPRI)
			printe(false, "getmsg retrieved flags 0x%x, not flags MSG_HIPRI", flags);
		if (band != 0)
			printe(false, "getmsg retrieved band %d message, not band 0", band);
		nread_check(fd, 2, sizeof(buf), 0);
	}

	printf("testing getpmsg to get just MSG_BAND data from band %d\n", DEFAULT_BAND-1);
	flags = MSG_BAND;
	band = DEFAULT_BAND - 1;
	if (getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, sizeof(buf), MORECTL, 0)) {
		if (flags != MSG_BAND)
			printe(false, "getmsg retrieved flags 0x%x, not flags MSG_BAND", flags);
		if (band != DEFAULT_BAND)
			printe(false, "getmsg retrieved band %d message, not band %d", band, DEFAULT_BAND);
			nread_check(fd, 2, 0, 0);
		nread_check(fd, 2, 0, 0);
	}

	printf("testing getpmsg to get the control part (using MSG_ANY)\n");
	ctlbuf.maxlen = sizeof(buf);
	flags = MSG_ANY;
	band = DEFAULT_BAND;
	if (getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, sizeof(buf), -1, 0, 0)) {
		if (flags != MSG_BAND)
			printe(false, "getmsg retrieved flags 0x%x, not flags MSG_BAND", flags);
		if (band != DEFAULT_BAND)
			printe(false, "getmsg retrieved band %d message, not band %d", band, DEFAULT_BAND);
		nread_check(fd, 1, sizeof(buf) / 2, 0);
	}

	printf("testing getpmsg with -1 data maxlen\n");
	databuf.maxlen = -1;
	flags = MSG_ANY;
	band = 0;
	if (getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, -1, MOREDATA, 0)) {
		if (flags != MSG_BAND)
			printe(false, "getmsg retrieved flags 0x%x, not flags MSG_BAND", flags);
		if (band != 0)
			printe(false, "getmsg retrieved band %d message, not band 0", band);
		nread_check(fd, 1, sizeof(buf) / 2, 0);
	}

	printf("testing regular MSG_ANY getpmsg that should return MOREDATA\n");
	databuf.maxlen = sizeof(buf) / 4;
	flags = MSG_ANY;
	band = 0;
	if (getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, sizeof(buf) / 4, MOREDATA, 0))
		nread_check(fd, 1, sizeof(buf) / 4, 0);

#ifndef NO_BOGUS_ADDRESSES
#ifdef	EARLY_ADDR_CHECK
	/* The bogus address in the following test is expected to be detected before a message
	** is removed from the stream head queue. In this case, we will need to read that message.
	*/
	msg_eaten = false;
#else
	/* The bogus address in the following test is expected to be detected during the copyout
	** operation after the message is removed from the stream head queue.  In this case, we don't
	** worry about rereading that message.
	*/
	msg_eaten = true;
#endif
	printf("testing getpmsg with bogus databuf.buf address\n");
	databuf.buf = BOGUS_ADDRESS;
	databuf.maxlen = sizeof(buf) / 4;
	flags = MSG_ANY;
	getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, 0, -1, EFAULT);
	nread_check(fd, msg_eaten ? 0 : 1, msg_eaten ? 0 : sizeof(buf) / 4, 0);
	databuf.buf = buf;
#else
	msg_eaten = false;
#endif

	if ( !msg_eaten ) {
		/* Read the message left over from the bogus address check. */
		databuf.maxlen = sizeof(buf) / 4;
		flags = MSG_ANY;
		getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, sizeof(buf) / 4, 0, 0);
		nread_check(fd, 0, 0, 0);
	}

	if (state & HANGUP) {
		printf("testing HANGUP getpmsg on empty read queue\n");
		flags = MSG_ANY;
		band = 0;
		getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, 0, 0, 0, 0);
		stream_close(fd);
		return;
	}
	PAUSE_HERE();
	
	printf("testing putpmsg of MSG_HIPRI %d control bytes, %d data bytes\n", sizeof(buf), sizeof(buf));
	ctlbuf.len = sizeof(buf);
	databuf.len = sizeof(buf);
	putpmsg_check(fd, &ctlbuf, &databuf, 0, MSG_HIPRI, 0, 0);
	nread_check(fd, 1, sizeof(buf), 0);

	printf("testing getpmsg of control part of MSG_HIPRI message\n");
	ctlbuf.maxlen = sizeof(buf);
	ctlbuf.buf = buf;
	databuf.maxlen = -1;
	flags = MSG_HIPRI;
	band = 0;
	getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, sizeof(buf), -1, MOREDATA, 0);
	nread_check(fd, 1, sizeof(buf), 0);

	printf("now using getpmsg to get the data part, expecting MSG_HIPRI\n");
	ctlbuf.maxlen = -1;
	databuf.maxlen = sizeof(buf);
	databuf.buf = buf;
	flags = MSG_HIPRI;
	band = 0;
	getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, -1, sizeof(buf), 0, 0);
	nread_check(fd, 0, 0, 0);
	PAUSE_HERE();

	printf("testing getpmsg with bogus flags\n");
	flags = -1;
	databuf.maxlen = 0;
	getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, 0, -1, EINVAL);
	nread_check(fd, 0, 0, 0);
	flags = 0;

	if (state & ONDELAY) {
		printf("testing ONDELAY getpmsg on empty stream\n");
		flags = MSG_ANY;
		band = 0;
		getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, 0, 0, -1, EAGAIN);
	}

	printf("testing getpmsg with 0 control maxlen (MSG_ANY and band 0)\n");
	if (stream_write(fd, buf, sizeof(buf)) > 0) {
		printf("wrote %d bytes\n", sizeof(buf));
		ctlbuf.buf = buf;
		ctlbuf.maxlen = 0;
		databuf.maxlen = sizeof(buf);
		flags = MSG_ANY;
		band = 0;
		getpmsg_check(fd, &ctlbuf, &databuf, &band, &flags, -1, sizeof(buf), 0, 0);
		nread_check(fd, 0, 0);
	} else
		printe(true, "write failed");

	printf("testing getpmsg after 0 length write (MSG_ANY and band 0)\n");
	if (stream_write(fd, buf, 0) == 0) {
		flags = MSG_ANY;
		band = 0;
		getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, 0, 0, 0);
		nread_check(fd, 0, 0);
	} else
		printe(true, "write failed");

	printf("testing getpmsg with non-PROTO or DATA message on read queue\n");
	fd2 = stream_open(nuls_name, 2);
	if ( fd2 == -1 )
		fatal("stream_open of %s failed, %s\n", nuls_name, errmsg(0));
	if (stream_ioctl(fd, I_SENDFD, fd2) == 0) {
		flags = MSG_ANY;
		band = 0;
		getpmsg_check(fd, NILS, &databuf, &band, &flags, 0, 0, -1, EBADMSG);
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
	stream_close(fd);
}

boolean
getpmsg_check (fd, ctlbuf, databuf, bandp, fp, ctl_len, data_len, ret_val, errno_val)
	int	fd;
	struct strbuf	* ctlbuf;
	struct strbuf	* databuf;
	int	* bandp;
	int	* fp;
	int	ctl_len;
	int	data_len;
	int	ret_val;
	int	errno_val;
{
	int	band;
	int	i1, flags;

	flags = *fp;
	band = *bandp;
	switch (i1 = getpmsg(fd, ctlbuf, databuf, bandp, fp)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "getpmsg failed");
		if (errno != errno_val)
			return printe(true, "getpmsg failed incorrectly");
		return printok("getpmsg failed as expected with errno %d", errno);
	default:
		if (i1 != ret_val)
			return printe(false, "getpmsg should have returned 0x%x not 0x%x",ret_val, i1);
		if ((databuf  &&  databuf->len != data_len)
		||  (ctlbuf  &&  ctlbuf->len != ctl_len))
			return printe(false, "getpmsg should have returned ctl %d data %d, not %d %d",
				ctl_len, data_len, ctlbuf ? ctlbuf->len : 0, databuf ? databuf->len : 0);
		return printok("getpmsg got %d ctl bytes and %d data bytes as expected",ctl_len, data_len);
	}
}

boolean
putpmsg_check (fd, ctlbuf, databuf, band, flags, ret_val, errno_val)
	int	fd;
	struct strbuf	* ctlbuf;
	struct strbuf	* databuf;
	int	band;
	int	flags;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	switch (i1 = putpmsg(fd, ctlbuf, databuf, band, flags)) {
	case -1:
		if (ret_val != -1)
			return printe(true, "putpmsg failed");
		if (errno != errno_val)
			return printe(true, "putpmsg failed incorrectly");
		return printok("putpmsg failed as expected with errno %d", errno);
	case 0:
		if (ret_val == 0)
			return printok("putpmsg put %d ctl bytes and %d data bytes",ctlbuf ? ctlbuf->len : 0, databuf ? databuf->len : 0);
		fallthru;
	default:
		return printe(false, "putpmsg should have returned %d not %d",ret_val, i1);
	}
}
