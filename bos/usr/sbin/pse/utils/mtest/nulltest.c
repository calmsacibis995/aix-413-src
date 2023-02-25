static char sccsid[] = "@(#)38	1.2  src/bos/usr/sbin/pse/utils/mtest/nulltest.c, cmdpse, bos411, 9428A410j 5/22/91 11:41:23";
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
 ** nulltest.c 2.2
 **/


#include <pse/common.h>
#include "mtest.h"
#include <sys/stropts.h>
#include <tiuser.h>
#include <pse/echo.h>

extern	void	etime(   int cnt, int len, TIMER * t1, char * msg   );
extern	char	* malloc(   int len   );
staticf	boolean	nparse(   char * cmd, int * rqst, char * dev, int * cnt, int * len   );
	int	nullclient(   char * cmd   );
extern	char	* strtok(   char * str, char * sep   );

staticf	boolean
nparse (cmd, rqstp, dev, cnt, len)
	char	* cmd;
	int	* rqstp;
	char	* dev;
	int	* cnt;
	int	* len;
{
	char	buf[128];
	char	* cp1;
	char	* sep = " \t\n";

	strcpy(buf, cmd);
	if ( !(cp1 = strtok(buf, sep)) )
		return false;
	if ( strncmp(cp1, "nullwrite", 5) == 0 )
		*rqstp = CLIENT_WRITE;
	else if ( strncmp(cp1, "nullread", 5) == 0 )
		*rqstp = CLIENT_READ;
	else if ( strncmp(cp1, "nullget", 5) == 0 )
		*rqstp = CLIENT_RECV;
	else if ( strncmp(cp1, "nullput", 5) == 0 )
		*rqstp = CLIENT_SEND;
	else if ( strncmp(cp1, "nullcput", 5) == 0 )
		*rqstp = CLIENT_NSND;
	else if ( strncmp(cp1, "nullsndu", 5) == 0 )
		*rqstp = CLIENT_SNDU;
#ifdef	CLIENT_NWPUT
	else if ( strncmp(cp1, "nullnwput", 5) == 0 )
		*rqstp = CLIENT_NWPUT;
#endif
	else
		return false;
	if ( !(cp1 = strtok(nilp(char), sep)) )
		return false;
	strcpy(dev, cp1);
	if ( !(cp1 = strtok(nilp(char), sep))  ||  (*cnt = atoi(cp1)) <= 0 )
		return false;
	if ( !(cp1 = strtok(nilp(char), sep))  ||  (*len = atoi(cp1)) <= 0 )
		return false;
	return true;
}
int
nullclient (cmd)
	char	* cmd;
{
	char	* buf;
	int	cnt;
	char	* cp1;
	struct strbuf ctl;
	struct strbuf data;
	char	dev[32];
	char	* endp;
	int	fd;
	int	i1;
	int	i2;
	int	len;
	int	rqst;
	TIMER	t1;
	char	nul_addr[12];
	struct strioctl stri;
	struct t_unitdata	tud;

	EnterCritSec();
	/* Critical around nparse until we replace use of strtok */
	if ( !nparse(cmd, &rqst, dev, &cnt, &len) ) {
		ExitCritSec();
		free(cmd);
		warn("nullclient: couldn't parse command '%s'\n", cmd);
		ExitThread(0, 1);
	}
	ExitCritSec();
	free(cmd);
	if ( (buf = malloc(MAX(len, 128))) == nilp(char) ) {
		warn("nullclient: out of memory\n");
		ExitThread(0, 1);
	}
	if ((fd = stream_open(dev, 2, nilp(struct t_info))) == -1) {
		warn("nullclient: stream_open of '%s' failed, %s\n", dev, errmsg(0));
		free(buf);
		ExitThread(0, 1);
	}
	switch ( rqst ) {
	case CLIENT_WRITE:
		printf("nullclient: (%d) writing to '%s'\n", fd, dev);
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( stream_write(fd, buf, len) == -1 ) {
				warn("nullclient: stream_write failed, %s\n", errmsg(0));
				goto bye;
			}
		}
		break;
	case CLIENT_READ:
		stri.ic_cmd = ECHO_FEED_ME;
		stri.ic_timout = -1;
		stri.ic_len = 0;
		stri.ic_dp = nilp(char);
		(void) stream_ioctl(fd, I_STR, &stri);
		printf("nullclient: (%d) reading from '%s'\n", fd, dev);
		endp = &buf[len];
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			for ( cp1 = buf; cp1 < endp; cp1 += i2 ) {
				i2 = stream_read(fd, cp1, endp - cp1);
				if ( i2 <= 0 ) {
					warn("nullclient: stream_read failed, %s\n", errmsg(0));
					goto bye;
				}
			}
		}
		break;
	case CLIENT_RECV:
		stri.ic_cmd = ECHO_FEED_ME;
		stri.ic_timout = -1;
		stri.ic_len = 0;
		stri.ic_dp = nilp(char);
		(void) stream_ioctl(fd, I_STR, &stri);
		printf("nullclient: (%d) reading from '%s'\n", fd, dev);
		endp = &buf[len];
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			for ( cp1 = buf; cp1 < endp; cp1 += data.len ) {
				data.maxlen = endp - cp1;
				data.buf = cp1;
				data.len = 0;
				i2 = 0;
				if ( getmsg(fd, nilp(struct strbuf)
				, &data, &i2) == -1 ) {
					warn("nullclient: getmsg failed, %s\n", errmsg(0));
					goto bye;
				}
			}
		}
		break;
	case CLIENT_SEND:
		data.maxlen = 0;
		data.len = len;
		data.buf = buf;
		printf("nullclient: (%d) sending to '%s'\n", fd, dev);
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( putmsg(fd, nilp(struct strbuf), &data, 0) == -1 ) {
				warn("nullclient: putmsg failed, %s\n", errmsg(0));
				goto bye;
			}
		}
		break;
	case CLIENT_NSND:
		ctl.maxlen = 0;
		ctl.len = 36;
		ctl.buf = buf;
		data.maxlen = 0;
		data.len = len;
		data.buf = buf;
		printf("nullclient: (%d) sending to '%s'\n", fd, dev);
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( putmsg(fd, &ctl, &data, 0) == -1 ) {
				warn("nullclient: putmsg failed, %s\n", errmsg(0));
				goto bye;
			}
		}
		break;
	case CLIENT_SNDU:
		if (stream_ioctl(fd, I_PUSH, "timod") == -1) {
			warn("nulsndu: push of timod failed, %s\n", errmsg(0));
			goto bye;
		}
		if (t_sync(fd) == -1) {
			t_error("nullclient: t_sync failed");
			goto bye;
		}
		tud.addr.buf = nul_addr;
		tud.addr.len = sizeof(nul_addr);
		tud.opt.buf = nilp(char);
		tud.opt.len = 0;
		tud.udata.buf = buf;
		tud.udata.len = len;
		printf("nullclient: (%d) sending\n", fd);
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( t_sndudata(fd, &tud) == -1 ) {
				t_error("nullclient: t_sndudata failed");
				goto bye;
			}
		}
		break;
#ifdef	CLIENT_NWPUT
	case CLIENT_NWPUT:
		ctl.maxlen = 0;
		ctl.len = 36;
		ctl.buf = buf;
		data.maxlen = 0;
		data.len = len;
		data.buf = buf;
		printf("nullclient: (%d) sending to '%s'\n", fd, dev);
		{
extern	int	StreamPutmsg(int, struct strbuf *, struct strbuf *, int, int *);
		int	handle = ((int *)fd)[2];
		int	local_errno = 0;
		SET_TIME(&t1);
		for ( i1 = cnt; i1--; ) {
			if ( StreamPutmsg(handle, &ctl, &data, 0, &local_errno) == -1 ) {
				warn("nullclient: putmsg failed, %s\n", errmsg(local_errno));
				goto bye;
			}
		}
		}
		break;
#endif
	}
	etime(cnt, len, &t1, buf);
	printf("nullclient(%d): Cnt: %d, len %d, %s\n", fd, cnt, len, buf);
bye:;
	stream_close(fd);
	free(buf);
	return 0;
}
