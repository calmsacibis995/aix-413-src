static char sccsid[] = "@(#)75	1.1  src/bos/usr/sbin/pse/utils/sthd/sigt.c, cmdpse, bos411, 9428A410j 5/7/91 13:53:02";
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
 ** sigt.c 2.2, last change 4/9/91
 **/


#include <pse/common.h>
#include "sthd.h"
#include <signal.h>
#include <sys/stream.h>
#include <pse/echo.h>

#ifdef	sleep
#undef	sleep
#endif

#ifdef NETWARE
#define	sleep(seconds)	StreamDelay(MS_TO_TICKS(seconds * 1000))
#endif

extern	int	errno;

extern	char	* malloc();

staticf	boolean	getsig_check(   int fd, int * maskp, int expected_mask, int ret_val, int errno_val   );
staticf	boolean	setsig_check(   int fd, int mask, int ret_val, int errno_val   );
staticf	void	get_bandurg_sig(   int fd   );
staticf	void	get_error_sig(   void  );
staticf	void	get_hangup_sig(   void   );
staticf	void	get_input_sig(   int fd   );
staticf	void	get_hipri_sig(   int fd   );
staticf	void	get_output_sig(), get_msg_sig();
staticf	void	get_rdband_sig(   int fd   );
staticf	void	get_rdnorm_sig(   int fd   );
staticf	int	got_sig(   int sig   );
staticf	int	got_sigurg(   int sig   );
staticf	void	get_wrnorm_sig(   int fd   );

static	int	got_one, got_urgent_sig;

void
sigt (state)
	int	state;
{
	int	fd;
	int	mask;

	printf("Testing getsig ioctl\n");

	if ((fd = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (signal(SIGPOLL, got_sig) == -1) {
		printe(true, "signal() failed");
		return;
	}

	if (signal(SIGURG, got_sigurg) == -1) {
		printe(true, "signal() failed");
		return;
	}

	printf("testing getsig before signals have been requested\n");
	getsig_check(fd, &mask, 0, -1, EINVAL);

	printf("testing setsig with mask 0 before signals have been requested\n");
	setsig_check(fd, 0, -1, EINVAL);

	printf("requesting S_INPUT signals\n");
	setsig_check(fd, S_INPUT, 0, 0);

#ifndef NO_BOGUS_ADDRESSES
	printf("testing getsig with bogus address parameter\n");
	getsig_check(fd, BOGUS_ADDRESS, 0, -1, EFAULT);
#endif

	printf("checking signal settings with getsig\n");
	mask = S_INPUT;
	getsig_check(fd, &mask, mask, 0, 0);

	printf("testing setsig with invalid mask\n");
	setsig_check(fd, -1, -1, EINVAL);

	printf("making sure signal settings did not get changed\n");
	mask = S_INPUT;
	getsig_check(fd, &mask, mask, 0, 0);
	PAUSE_HERE();

	get_input_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_rdnorm_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_rdband_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_bandurg_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_hipri_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_output_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_wrnorm_sig(fd);
	setsig_check(fd, 0, 0, 0);
	PAUSE_HERE();

	get_msg_sig(fd);
	PAUSE_HERE();

	get_error_sig();
	PAUSE_HERE();

	get_hangup_sig();
	PAUSE_HERE();

	printf("clearing signal settings with setsig\n");
	setsig_check(fd, 0, 0, 0);

	signal(SIGPOLL, SIG_IGN);
	flush_check(fd, state, 0);
	nread_check(fd, 0, 0, 0);

	stream_close(fd);
}

staticf int
got_sig (sig)
	int	sig;
{
	signal(sig, got_sig);
	got_one = true;
	return 1;
}

staticf int
got_sigurg (sig)
	int	sig;
{
	signal(sig, got_sigurg);
	got_urgent_sig = true;
	return 1;
}

staticf void
get_error_sig ()
{
	int	fd1;

	printf("testing ERROR signals\n");
	if ((fd1 = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (!setsig_check(fd1, S_ERROR, 0, 0))
		return;
	got_one = false;
	set_error_state(fd1, ERANGE);
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");
	stream_close(fd1);
}

staticf void
get_hangup_sig ()
{
	int	fd1;

	printf("testing HANGUP signals\n");
	if ((fd1 = stream_open(echo_name, 2)) == -1) {
		printe(true, "open of %s failed", echo_name);
		return;
	}

	if (!setsig_check(fd1, S_HANGUP, 0, 0))
		return;
	got_one = false;
	set_hangup_state(fd1);
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");
	stream_close(fd1);
}

staticf void
get_hipri_sig (fd)
	int	fd;
{
	char	buf[128];
	int	flags;
	struct strbuf	ctlbuf, databuf;

	printf("testing HIPRI signals\n");
	if (!setsig_check(fd, S_HIPRI, 0, 0))
		return;

	got_one = false;
	printf("writing a buffer, signal should hit\n");
	ctlbuf.buf = buf;
	ctlbuf.len = 10;
	ctlbuf.maxlen = 10;
	databuf.buf = buf;
	databuf.len = sizeof(buf);
	databuf.maxlen = sizeof(buf);
	if (putmsg(fd, &ctlbuf, &databuf, RS_HIPRI) == -1) {
		printe(true, "putmsg failed");
		return;
	}
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");
	got_one = false;
	printf("writing more and no signal should hit\n");
	if (putmsg(fd, &ctlbuf, &databuf, RS_HIPRI) == -1) {
		printe(true, "putmsg failed");
		return;
	}
	if (!got_one)
		sleep(1);
	if (!got_one)
		printf("no signal on second buffer, test successful\n");
	else
		printe(false, "signal hit on second message");
	/* Use getmsg here so that the V.3 Stream head will clear its PCPROTO bit. */
	/* Flush doesn't clear the bit and the next M_PCPROTO will be improperly ignored. */
	flags = 0;
	if (getmsg(fd, &ctlbuf, &databuf, &flags) == -1) {
		printe(true, "getmsg failed (flushing)");
		if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
			printe(true, "flush failed");
	}

	printf("testing HIPRI signals with regular data on the queue\n");
	if (stream_write(fd, buf, sizeof(buf)) == -1) {
		printe(true, "write failed");
		return;
	}
	got_one = false;
	printf("writing a HIPRI message after regular data, signal should hit\n");
	ctlbuf.buf = buf;
	ctlbuf.len = 10;
	databuf.buf = buf;
	databuf.len = sizeof(buf)/2;
	if (putmsg(fd, &ctlbuf, &databuf, RS_HIPRI) == -1) {
		printe(true, "putmsg failed");
		return;
	}
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on HIPRI message");

	/* Call getmsg to remove the M_PCPROTO message, see comment above. */
	flags = 0;
	if (getmsg(fd, &ctlbuf, &databuf, &flags) == -1)
		printe(true, "getmsg failed");
	/* Flush to remove the M_DATA message */
	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");
}

staticf void
get_input_sig (fd)
	int	fd;
{
	char	buf[128];

	printf("testing INPUT signals\n");
	if (!setsig_check(fd, S_INPUT, 0, 0))
		return;

	got_one = false;
	printf("writing a buffer, signal should hit\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");

	got_one = false;
	printf("writing more, no signal should happen\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	sleep(1);
	if (got_one)
		printe(false, "another signal hit");
	else
		printf("no signal showed, test successful\n");
	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");
}

staticf void
get_msg_sig (fd)
	int	fd;
{
	char	buf[128];

	printf("testing MSG signals\n");
	if (!setsig_check(fd, S_MSG, 0, 0))
		return;

	printf("testing M_PCSIG\n");
	got_one = 0;
	if (!request_sig(fd, buf, sizeof(buf), true))
		return;
	if (!got_one)
		sleep(1);
	if (got_one)
		printf("received signal, test successful\n");
	else
		printe(false, "no signal received");

	printf("testing M_SIG\n");
	got_one = 0;
	if (!request_sig(fd, buf, sizeof(buf), false))
		return;
	if (!got_one)
		sleep(1);
	if (got_one)
		printf("received signal, test successful\n");
	else
		printe(false, "no signal received");

	printf("testing M_PCSIG with data on read queue\n");
	if (stream_write(fd, buf, sizeof(buf)) == sizeof(buf)) {
		got_one = 0;
		if (!request_sig(fd, buf, sizeof(buf), true))
			return;
		if (!got_one)
			sleep(1);
		if (got_one)
			printf("received signal, test successful\n");
		else
			printe(false, "no signal received");

		printf("testing M_SIG with data on read queue\n");
		got_one = 0;
		if (!request_sig(fd, buf, sizeof(buf), false))
			return;
		if (!got_one)
			sleep(1);
		printf("reading data\n");
		if (got_one)
			printe(false, "signal happened before data read");
		else if (stream_read(fd, buf, sizeof(buf)) == sizeof(buf)) {
			if (!got_one)
				sleep(1);
			if (got_one)
				printf("received signal, test successful\n");
			else
				printe(false, "no signal received");
		} else
			printe(true, "read failed");
	} else
		printe(true, "write failed");

	flush_check(fd, 0, 0);
}

staticf void
get_output_sig (fd)
	int	fd;
{
	char	buf[1024];
	int	len, i1;

	printf("testing OUTPUT signals\n");
	if (!setsig_check(fd, S_OUTPUT, 0, 0))
		return;

	set_ondelay_mode(fd);
	len = 0;
	printf("writing to limit\n");
	while ((len = stream_write(fd, buf, sizeof(buf))) == sizeof(buf))
		noop;
	if (len != -1  ||  errno != EAGAIN) {
		i1 = errno;
		flush_check(fd, 0, 0);
		errno = i1;
		if (len == -1)
			printe(true, "write failed");
		else
			printe(false, "write returned %d", len);
		return;
	}
	got_one = false;
	printf("going to read one below low_water, signal should hit\n");
	while ((len = stream_read(fd, buf, sizeof(buf))) == sizeof(buf))
		noop;
	i1 = errno;
	flush_check(fd, 0, 0);
	errno = i1;
	if (len != -1  ||  errno != EAGAIN) {
		if (len == -1)
			printe(true, "read failed");
		else
			printe(false, "read returned %d", len);
		return;
	}
	if (got_one)
		printok("signal hit, test successful");
	else
		printe(false, "no signal");
}

staticf void
get_wrnorm_sig (fd)
	int	fd;
{
	char	buf[1024];
	int	len, i1;

	printf("testing WRNORM signals\n");
	if (!setsig_check(fd, S_WRNORM, 0, 0))
		return;

	set_ondelay_mode(fd);
	len = 0;
	printf("writing to limit\n");
	while ((len = stream_write(fd, buf, sizeof(buf))) == sizeof(buf))
		noop;
	if (len != -1  ||  errno != EAGAIN) {
		i1 = errno;
		flush_check(fd, 0, 0);
		errno = i1;
		if (len == -1)
			printe(true, "write failed");
		else
			printe(false, "write returned %d", len);
		return;
	}
	got_one = false;
	printf("going to read one below low_water, signal should hit\n");
	while ((len = stream_read(fd, buf, sizeof(buf))) == sizeof(buf))
		noop;
	i1 = errno;
	flush_check(fd, 0, 0);
	errno = i1;
	if (len != -1  ||  errno != EAGAIN) {
		if (len == -1)
			printe(true, "read failed");
		else
			printe(false, "read returned %d", len);
		return;
	}
	if (got_one)
		printok("signal hit, test successful");
	else
		printe(false, "no signal");
}

staticf void
get_bandurg_sig (fd)
	int	fd;
{
	struct bandinfo	bi;
	char	buf[128];
	struct strbuf	ctlbuf;

	printf("testing RDBAND | BANDURG signals\n");
	if (!setsig_check(fd, S_RDBAND | S_BANDURG, 0, 0))
		return;

	got_urgent_sig = false;
	printf("writing a banded message, signal should hit\n");
	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	putpmsg_check(fd, &ctlbuf, nilp(struct strbuf), 5, MSG_BAND, 0, 0);
	if (got_urgent_sig == false)
		sleep(1);
	if (got_urgent_sig)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");

	printf("flushing stream\n");
	bi.bi_flag = FLUSHR;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);

	got_urgent_sig = false;
	printf("writing a regular message, no signal should happen\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	sleep(1);
	if (got_urgent_sig)
		printe(false, "another signal hit");
	else
		printf("no signal showed, test successful\n");

	printf("flushing stream\n");
	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");
}

staticf void
get_rdband_sig (fd)
	int	fd;
{
	struct bandinfo	bi;
	char	buf[128];
	struct strbuf	ctlbuf;

	printf("testing RDBAND signals\n");
	if (!setsig_check(fd, S_RDBAND, 0, 0))
		return;

	got_one = false;
	printf("writing a banded message, signal should hit\n");
	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	putpmsg_check(fd, &ctlbuf, nilp(struct strbuf), 5, MSG_BAND, 0, 0);
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");

	printf("flushing stream\n");
	bi.bi_flag = FLUSHR;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);

	got_one = false;
	printf("writing a regular message, no signal should happen\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	sleep(1);
	if (got_one)
		printe(false, "another signal hit");
	else
		printf("no signal showed, test successful\n");

	printf("flushing stream\n");
	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");

}

staticf void
get_rdnorm_sig (fd)
	int	fd;
{
	struct bandinfo	bi;
	char	buf[128];
	struct strbuf	ctlbuf;

	printf("testing RDNORM signals\n");
	if (!setsig_check(fd, S_RDNORM, 0, 0))
		return;

	got_one = false;
	printf("writing a buffer, signal should hit\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	if (got_one == false)
		sleep(1);
	if (got_one)
		printf("got signal as expected\n");
	else
		printe(false, "no signal on first buffer");
	got_one = false;
	printf("writing more, no signal should happen\n");
	if (stream_write(fd, buf, sizeof(buf)) != sizeof(buf)) {
		printe(true, "write failed");
		return;
	}
	sleep(1);
	if (got_one)
		printe(false, "another signal hit");
	else
		printf("no signal showed, test successful\n");

	printf("flushing stream\n");
	if (stream_ioctl(fd, I_FLUSH, FLUSHR) == -1)
		printe(true, "flush failed");

	got_one = false;
	printf("writing a banded message, no signal should happen\n");
	ctlbuf.buf = buf;
	ctlbuf.len = sizeof(buf);
	putpmsg_check(fd, &ctlbuf, nilp(struct strbuf), 5, MSG_BAND, 0, 0);
	sleep(1);
	if (got_one)
		printe(false, "signal hit");
	else
		printf("no signal showed, test successful\n");

	printf("flushing stream\n");
	bi.bi_flag = FLUSHR;
	bi.bi_pri = DEFAULT_BAND;
	flushband_check(fd, &bi, 0, 0);
}

staticf boolean
getsig_check (fd, maskp, expected_mask, ret_val, errno_val)
	int	fd;
	int	* maskp;
	int	expected_mask;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	i1 = stream_ioctl(fd, I_GETSIG, (char *)maskp);
	if (i1 == ret_val) {
		switch (ret_val) {
		case -1:
			if (errno == errno_val)
				return printok("getsig failed as expected with errno %d", errno);
			return printe(true, "getsig failed incorrectly, expected errno %d", errno_val);
		case 0:
			if (*maskp == expected_mask)
				return printok("getsig retrieved mask 0x%x as expected", *maskp);
			return printe(false, "getsig should have retrieved mask 0x%x, not 0x%x", expected_mask, *maskp);
		default:
			return printok("getsig returned %d as expected", i1);
		}
	}
	if (i1 == -1)
		return printe(true, "getsig failed");
	return printe(false, "getsig should have returned %d, not %d", ret_val, i1);
}

staticf boolean
setsig_check (fd, mask, ret_val, errno_val)
	int	fd;
	int	mask;
	int	ret_val;
	int	errno_val;
{
	int	i1;

	i1 = stream_ioctl(fd, I_SETSIG, mask);
	if (i1 == ret_val) {
		if (i1 == -1) {
			if (errno == errno_val)
				return printok("setsig failed as expected with errno %d", errno);
			return printe(true, "setsig failed incorrectly, expected errno %d", errno_val);
		}
		return printok("setsig succeeded");
	}
	if (i1 == -1)
		return printe(true, "setsig failed");
	return printe(false, "setsig should have returned %d, not %d", ret_val, i1);
}
