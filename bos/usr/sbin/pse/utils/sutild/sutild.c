static char sccsid[] = "@(#)85	1.1  src/bos/usr/sbin/pse/utils/sutild/sutild.c, cmdpse, bos411, 9428A410j 5/7/91 13:54:42";
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

/** Copyright (c) 1990, 1991  Mentat Inc.
 ** sutild.c 2.4, last change 4/7/91
 **/


#include <pse/common.h>
#include <sys/stropts.h>
#include <pse/tmod.h>
#include <stdio.h>

#ifndef	NULL_DEV_NAME
#define	NULL_DEV_NAME	"/dev/nuls"
#endif

#ifndef	TMOD_NAME
#define	TMOD_NAME	"tmod"
#endif

typedef struct cmd_s {
	char	* cmd_name;
	int	cmd_ioctl_val;
	char	* cmd_description;
} CMD, * CMDP;

static CMD cmd_tbl[] = {
 { "adjmsg",	TMOD_ADJMSG,	"test adjmsg"	},
 { "bufcall",	TMOD_BUFCALL,	"test bufcall and unbufcall" },
 { "canput",	TMOD_CANPUT,	"test bcanput and canput" },
 { "copy",	TMOD_COPY,	"test copyb and copymsg" },
 { "dup",	TMOD_DUP,	"test dupb and dupmsg" },
 { "esballoc",	TMOD_ESBALLOC,	"test esballoc" },
 { "flush",	TMOD_FLUSH,	"test flushband and flushq" },
 { "getadmin",	TMOD_GETADMIN,	"test getadmin" },
 { "getmid",	TMOD_GETMID,	"test getmid" },
 { "getq",	TMOD_GETQ,	"test getq and putq" },
 { "insq",	TMOD_INSQ,	"test insq" },
 { "linkb",	TMOD_LINKB,	"test linkb, unlinkb, and msgdsize" },
 { "pullupmsg",	TMOD_PULLUPMSG,	"test pullupmsg" },
 { "putq",	TMOD_PUTQ,	"test getq and putq" },
 { "putbq",	TMOD_PUTBQ,	"test putbq" },
 { "qenable",	TMOD_QENABLE,	"test qenable" },
 { "rmvb",	TMOD_RMVB,	"test rmvb" },
 { "rmvq",	TMOD_RMVQ,	"test rmvq" },
 { "strlog",	TMOD_STRLOG,	"test strlog" },
 { "strqget",	TMOD_STRQGET,	"test strqget" },
 { "strqset",	TMOD_STRQSET,	"test strqset" },
 { 0 }
};

extern	char	noshare * optarg;
extern	int	noshare optind;

main (argc, argv)
	int	argc;
	char	** argv;
{
	char	buf[128];
	int	c;
	CMDP	cmd;
	char	* device_name;
	int	do_all;
	int	i1;
	int	pause_between_tests;
	int	fd;
	char	* module_name;
	struct strioctl	stri;

	device_name = NULL_DEV_NAME;
	module_name = TMOD_NAME;
	do_all = false;
	pause_between_tests = false;
	while ((c = getopt(argc, argv, "aAdDmMpP")) != -1) {
		switch (c) {
		case 'a':
		case 'A':
			do_all = true;
			break;
		case 'd':
		case 'D':
			device_name = getarg(argc, argv);
			break;
		case 'm':
		case 'M':
			module_name = getarg(argc, argv);
			break;
		case 'p':
		case 'P':
			pause_between_tests = true;
			break;
		default:
			printf("sutild [-a] [-p] [-d device_name] [-m module_name]\n");
			exit(1);
		}
	}
	fd = stream_open(device_name, 2);
	if (fd == -1) {
		printf("open of %s failed, %s\n", device_name, errmsg(0));
		exit(1);
	}
	if (stream_ioctl(fd, I_PUSH, module_name) == -1) {
		printf("push of %s failed, %s\n", module_name, errmsg(0));
		stream_close(fd);
		exit(1);
	}

	/* Initialize I_STR ioctl structure, only the ic_cmd field changes. */
	stri.ic_dp = nilp(char);
	stri.ic_len = 0;
	stri.ic_timout = -1;

	printf("This program exercises the STREAMS utility routines.\n");
	printf("Output from each test is passed to the log device.\n");
	fflush(stdout);

	if (do_all) {
		for (cmd = cmd_tbl; cmd->cmd_name; cmd++) {
			if (cmd != cmd_tbl  &&  pause_between_tests) {
				printf("type a <return> to %s\n", cmd->cmd_description);
				fflush(stdout);
				getchar();
			}
			stri.ic_cmd = cmd->cmd_ioctl_val;
			if (stream_ioctl(fd, I_STR, (char *)&stri) == -1)
				printf("ioctl failed, %s\n", errmsg(0));
		}
		stream_close(fd);
		exit(0);
	}

	/* Interactive test loop */
	for (;;) {
		write(1, "test to run? ", 13);
		if ((i1 = read(0, buf, sizeof(buf))) <= 0)
			break;
		buf[--i1] = '\0';
		if (i1 == 0)
			continue;
		if (strncmp(buf, "quit", i1) == 0)
			exit(0);
		if (buf[0] == '?'  ||  strncmp(buf, "help", i1) == 0) {
			for (cmd = cmd_tbl; cmd->cmd_name; cmd++)
				printf("%-8s\t%s\n", cmd->cmd_name, cmd->cmd_description);
			continue;
		}
		for (cmd = cmd_tbl; cmd->cmd_name; cmd++) {
			if (strncmp(buf, cmd->cmd_name, i1) == 0) {
				stri.ic_cmd = cmd->cmd_ioctl_val;
				if (stream_ioctl(fd, I_STR, (char *)&stri) == -1)
					printf("ioctl failed, %s\n", errmsg(0));
				break;
			}
		}
		if (!cmd->cmd_name)
			printf("Command \"%s\" not found.\nType \"help\" or \"?\" for a list of all valid commands.", buf);
		write(1, "\n", 1);
	}
	stream_close(fd);
}
