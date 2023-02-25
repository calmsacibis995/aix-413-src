/* @(#)13       1.8.1.3  src/tcpip/usr/bin/ftp/ftp_var.h, tcpfilexfr, tcpip41J, 9519A_all 5/5/95 17:00:24 */
/* 
 * COMPONENT_NAME: TCPIP ftp_var.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	ftp_var.h	5.3 (Berkeley) 3/7/86
 */

/*
 * FTP global variables.
 */

int mb_cur_max;			/* max amount of bytes in a char for locale */

/*
 * Options and other state info.
 */
int	trace;			/* trace packets exchanged */
int	hash;			/* print # for each buffer transferred */
#ifdef aiws	/* our compiler is finicky */
extern int	sendport;	/* use PORT cmd for each data connection */
#else
int	sendport;		/* use PORT cmd for each data connection */
#endif
int	verbose;		/* print messages coming back from server */
int	connected;		/* connected to server */
int	fromatty;		/* input is from a terminal */
int	interactive;		/* interactively prompt on m* cmds */
int	debug;			/* debugging level */
int	bell;			/* ring bell on cmd completion */
int	doglob;			/* glob local file names */
int	autologin;		/* establish user account on connection */
int	proxy;			/* proxy server connection active */
int	proxflag;		/* proxy connection exists */
int	sunique;		/* store files on server with unique name */
int	runique;		/* store local files with unique name */
int	mcase;			/* map upper to lower case for mget names */
int	ntflag;			/* use ntin ntout tables for name translation */
int	mapflag;		/* use mapin mapout templates on file names */
int	code;			/* return/reply code for ftp command */
int	crflag;			/* if 1, strip car. rets. on ascii gets */
char	pasv[64];		/* passive port for proxy data connection */
char	*altarg;		/* argv[1] with no shell-like preprocessing  */
char	ntin[17];		/* input translation table */
char	ntout[17];		/* output translation table */
#include <sys/param.h>
char	mapin[MAXPATHLEN];	/* input map template */
char	mapout[MAXPATHLEN];	/* output map template */
char	typename[32];		/* name of file transfer type */
char	typecode[2];		/* the code letter for the type */
int	type;			/* file transfer type */
char	structname[32];		/* name of file transfer structure */
int	stru;			/* file transfer structure */
char	formname[32];		/* name of file transfer format */
char	formcode[2];		/* the code letter for the form */
int	form;			/* file transfer format */
char	modename[32];		/* name of file transfer mode */
int	mode;			/* file transfer mode */
char	bytename[32];		/* local byte size in ascii */
int	bytesize;		/* local byte size in binary */
int	line_state;		/* current state for data formatting */
#define START_OF_FILE 1
#define START_OF_LINE 2
#define IN_LINE 3
#define DID_CR 4
#define DID_ESC 5
int	car_ctl_char;		/* carriage control char to use next */
int	next_bits;		/* no. of bits to the next logical byte */
int	useful_bits;		/* no. of bits to use from the current char */
int	space_bits;		/* no. of available bits in output char */
int	avail_bits;		/* no. of available bits in input char */
int	total_bits;		/* out_bits + useful_bits */
int	out_bits;		/* no. of bits used in output char */
int	out_char;		/* the output char */
off_t	write_count;		/* bytes written */
off_t	rest_mark;		/* seek point to restart transfer */
off_t	rest_count;		/* byte count needed for next restart marker */
int	rest_flag;		/* whether doing a restarted transfer */
char	rest_name[MAXPATHLEN];	/* restart control file filename */
int	exp_cmds;		/* experimental commands flag */
int   	copyflag;		/* toggle local copy flag */

char	*hostname;		/* name of host connected to */

struct	servent *sp;		/* service spec for tcp/ftp */
int	ftam_port ;		/* port number for tcp/ftp-ftam */
int	user_port ;		/* user entered port number */

#include <setjmp.h>
jmp_buf	toplevel;		/* non-local goto stuff for cmd scanner */

#include <stdio.h>
char	line[BUFSIZ];		/* input line buffer */
char	*stringbase;		/* current scan point in line buffer */
char	argbuf[BUFSIZ];		/* argument storage buffer */
char	*argbase;		/* current storage point in arg buffer */
int	margc;			/* count of arguments on input line */
char	*margv[100];		/* args parsed from input line */
int     cpend;                  /* flag: if != 0, then pending server reply */
int	mflag;			/* flag: if != 0, then active multi command */

int	options;		/* used during socket creation */

/*
 * Format of command table.
 */
struct cmd {
	char	*c_name;	/* name of command */
#ifdef	MSG
	int	help_id;	/* index into message table */
#endif
	char	*c_help;	/* help string */
	char	c_bell;		/* give bell when command completes */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
	int	(*c_handler)();	/* function to call */
};

struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

int macnum;			/* number of defined macros */
struct macel macros[16];
char macbuf[4096];

extern	char *tail();
extern	char *index();
extern	char *rindex();
extern	char *remglob();
extern	int errno;
extern	char *mktemp();
extern	char *strncpy();
extern	char *strncat();
extern	char *strcat();
extern	char *strcpy();

#define HASHBYTES 1024  /* number of bytes represented by each '#' mark */
