/* @(#)79	1.1  src/tcpip/usr/sbin/ipreport/rusers.h, tcpras, tcpip411, GOLD410 10/24/91 18:14:38 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */

/*	(#)rusers.h	1.2 88/05/08 4.0NFSSRC SMI	*/
/* (#)92	1.4  rusers.h, cmdnfs, nfs320 12/13/89 14:41:27 */

/* 
 * (#) from SUN 1.6
 */

#define RUSERSPROC_NUM 1
#define RUSERSPROC_NAMES 2
#define RUSERSPROC_ALLNAMES 3
#define RUSERSPROG 100002
#define RUSERSVERS_ORIG 1
#define RUSERSVERS_IDLE 2
#define RUSERSVERS 2

#define MAXUSERS 100

/*
 * Structure of BSD utmp and wtmp files.
 * Needed because SUN in their infinite
 * wisdom chose to send it on the net.
 */
struct bsd_utmp {
	char	ut_line[8];	/* tty name */
	char	ut_name[8];	/* user id */
	char	ut_host[16];	/* host name, if remote */
	long	ut_time;	/* time on */
};

struct utmparr {
	struct bsd_utmp **uta_arr;
	int uta_cnt;
};

struct utmpidle {
	struct bsd_utmp ui_utmp;
	unsigned ui_idle;
};

struct utmpidlearr {
	struct utmpidle **uia_arr;
	int uia_cnt;
};

int xdr_utmparr();
int xdr_utmpidlearr();
