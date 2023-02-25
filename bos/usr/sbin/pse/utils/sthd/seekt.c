static char sccsid[] = "@(#)73	1.1  src/bos/usr/sbin/pse/utils/sthd/seekt.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:58";
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
 ** seekt.c 2.2, last change 11/19/90
 **/


#include <pse/common.h>
#include "sthd.h"
#include <sys/stream.h>
#include <sys/stropts.h>
#include <pse/echo.h>

/* The following are copied out of common.h so we don't have to define KERNEL */
/* Extra MPS mblk type */
#define	M_MI		64
/* Subfields for M_MI messages */
#define	M_MI_READ_RESET	1
#define	M_MI_READ_SEEK	2
#define	M_MI_READ_END	4

struct strseek {
	u32	strs_cmd;
	int	strs_whence;
	long	strs_to;
};

extern	int	errno;
static	char	msg[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static	char	cmp[] = "ABCDEFGHIJKLM*OPQRSTUVWXYZ";

void
seekt (state)
	int	state;
{
	char	buf[MAX_STACK_BUF];
	int	fd;
	int	i1;
	u16	* up;

	if (state & (ERROR | HANGUP)) {
		printok("\nCan not test seek in ERROR or HANGUP mode");
		return;
	}
	printf("\ntesting M_MI_READ_SEEK, M_MI_READ_RESET, and M_MI_READ_END messages\n");
	fd = stream_open(echo_name, 2);
	if ( fd == -1 ) {
		printe(true, "couldn't open '%s'", echo_name);
		return;
	}
	if ( stream_ioctl(fd, I_SRDOPT, RFILL) == -1 ) {
		printe(true, "couldn't set RFILL read mode");
		goto bye;
	}
	
	printf("writing message 'ABCDEFGHIJKLM'\n");
	if (stream_write(fd, "ABCDEFGHIJKLM", 13) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	nread_check(fd, 1, 13, 0);
	printf("seeking to 14 beyond start of buffer\n");
	if (send_seek(fd, buf, M_MI_READ_SEEK, 0, 14L)) {
		printe(true, "couldn't send M_MI_READ_SEEK message");
		goto bye;
	}
	printf("writing message 'OPQRSTUVWXYZ'\n");
	if (stream_write(fd, "OPQRSTUVWXYZ", 12) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	printf("seeking to where the 'N' should go, and writing in a '*'\n");
	if (send_seek(fd, buf, M_MI_READ_SEEK, 0, 13L)) {
		printe(true, "couldn't send M_MI_READ_SEEK message");
		goto bye;
	}
	if (stream_write(fd, "*", 1) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	printf("sending M_MI_READ_END\n");
	if (send_seek(fd, buf, M_MI_READ_END, 0, 0L)) {
		printe(true, "couldn't send M_MI_READ_END message");
		goto bye;
	}
	printf("reading back message\n");
	if ((i1 = stream_read(fd, buf, sizeof(buf))) != 26) {
		if ( i1 == -1 )
			printe(true, "read failed");
		else
			printe(false, "read returned %d, expected %d", i1, sizeof(msg));
		goto bye;
	}
	buf[i1] = '\0';
	if ( strcmp(buf, cmp) != 0 ) {
		printe(false, "got back '%s', expected '%s'", buf, cmp);
		goto bye;
	}
	printf("got back '%s', as expected\n", buf);
	printf("doing it all again, but seeking from current pointer instead of start of buffer\n");
	if (stream_write(fd, "ABCDEFGHIJKLM", 13) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	nread_check(fd, 1, 13, 0);
	if (send_seek(fd, buf, M_MI_READ_SEEK, 1, 1L)) {
		printe(true, "couldn't send M_MI_READ_SEEK message");
		goto bye;
	}
	if (stream_write(fd, "OPQRSTUVWXYZ", 12) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	if (send_seek(fd, buf, M_MI_READ_SEEK, 1, -13L)) {
		printe(true, "couldn't send M_MI_READ_SEEK message");
		goto bye;
	}
	if (stream_write(fd, "*", 1) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	if (send_seek(fd, buf, M_MI_READ_END, 0, 0L)) {
		printe(true, "couldn't send M_MI_READ_END message");
		goto bye;
	}
	printf("reading back message\n");
	if ((i1 = stream_read(fd, buf, sizeof(buf))) != 26) {
		if ( i1 == -1 )
			printe(true, "read failed");
		else
			printe(false, "read returned %d, expected %d", i1, sizeof(msg));
		goto bye;
	}
	buf[i1] = '\0';
	if ( strcmp(buf, cmp) != 0 ) {
		printe(false, "got back '%s', expected '%s'", buf, cmp);
		goto bye;
	}
	printf("got back '%s', as expected\n", buf);
	printf("writing alphabet again\n");
	if (stream_write(fd, msg, 26) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	printf("resetting read buffer\n");
	if (send_seek(fd, buf, M_MI_READ_RESET, 0, 0L)) {
		printe(true, "couldn't send M_MI_READ_RESET message");
		goto bye;
	}
	printf("writing alphabet one more time\n");
	if (stream_write(fd, msg, 26) == -1) {
		printe(true, "write failed");
		goto bye;
	}
	printf("sending M_MI_READ_END\n");
	if (send_seek(fd, buf, M_MI_READ_END, 0, 0L)) {
		printe(true, "couldn't send M_MI_READ_END message");
		goto bye;
	}
	printf("reading back message\n");
	if ((i1 = stream_read(fd, buf, sizeof(buf))) != 26) {
		if ( i1 == -1 )
			printe(true, "read failed");
		else
			printe(false, "read returned %d, expected %d", i1, 26);
		goto bye;
	}
	buf[i1] = '\0';
	if ( strcmp(buf, msg) != 0 ) {
		printe(false, "got back '%s', expected '%s'", buf, msg);
		goto bye;
	}
	printf("got back '%s', as expected\n", buf);

bye:;
	stream_close(fd);
	return;
}

staticf int
send_seek (fd, buf, cmd, whence, to)
	int	fd;
	char	* buf;
	u32	cmd;
	int	whence;
	long	to;
{
	struct strseek * strs;
	struct strioctl	stri;
	eblk_t	* ep;

	stri.ic_cmd = ECHO_GENMSG;
	stri.ic_len = sizeof(eblk_t) + sizeof(struct strseek);
	stri.ic_timout = -1;
	stri.ic_dp = buf;
	ep = (eblk_t *)buf;
	ep->eb_type = M_MI;
	ep->eb_flag = 0;
	ep->eb_len = sizeof(struct strseek);
	strs = (struct strseek *)&ep[1];
	strs->strs_cmd = cmd;
	strs->strs_whence = whence;
	strs->strs_to = to;
	return stream_ioctl(fd, I_STR, &stri);
}
