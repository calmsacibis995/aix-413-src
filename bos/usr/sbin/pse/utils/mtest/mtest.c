static char sccsid[] = "@(#)36	1.1  src/bos/usr/sbin/pse/utils/mtest/mtest.c, cmdpse, bos411, 9428A410j 5/7/91 13:45:20";
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

/** Copyright (c) 1989  Mentat Inc.
 ** mtest.c 1.13, last change 10/22/90
 **/


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <pse/common.h>
#include "mtest.h"

/* Portability notes:
**	- All OS dependencies are isolated in osmtest.h and osmtest.c.  These include handling of millisecond timing,
**	  and terminal input which does not block active threads.
**	- Timers are handled by invoking the macro SET_TIME(timerp) defined in osmtest.h.  It is invoked with an argument
**	  of type (TIMER *), also defined in osmtest.h.
**	- The function etime is supplied in osmtest.c.  It computes the difference in milliseconds between the time passed
**	  in and the current time, and uses that difference to print a report line.
**	- The routine os_get_cmd is supplied in osmtest.c.  It returns a full line from the terminal but must not block
**	  active threads while waiting for input.
**	- Environments that do not have an implementation of Mentat Threads must somehow provide fakes for CreateThread,
**	  ExitThread, ThreadSwitch, EnterCritSec, ExitCritSec, and ThreadSuspend in osmtest.c.
**	- UNIX implementations which rely on fork() to fake threads should compile with FORKER defined.  This results in
**	  some extra closes to make sure that both the parent and child close down streams.
*/

extern	int	CreateThread(   pfi_t pfi, u32 * thread_id_ptr, char * stptr, u32 stlen, u32 arg   );
extern	void	fatal(   char * fmt, ...   );
extern	char	* os_get_cmd(   void   );
extern	void	warn(   char * fmt, ...   );

#ifdef	XNSTLI
/* XNS/TLI Test Family */
extern	int	xdgclient(   char * xaddr, int cnt, int len   );
extern	int	xdgsrvr(   void   );
extern	int	xsclient(   char * cmd   );
extern	int	xssrvr(   void   );
static	CMD	xns_tli_send = { "send", "send enet_addr_hex_str send_cnt send_len [rep_cnt]" };
static	CMD	xns_tli_recv = { "recv", "recv enet_addr_hex_str recv_cnt recv_len [rep_cnt]" };
static	CMD	xns_tli_sndu = { "sndu", "sndu enet_addr_hex_str send_cnt send_len" };
#ifndef DOS
static	CMD	xns_tli_write = { "write", "write enet_addr_hex_str write_cnt write_len [rep_cnt]" };
static	CMD	xns_tli_read = { "read", "read enet_addr_hex_str read_cnt read_len [rep_cnt]" };
static	CMDP	xstli_cmds[] = { &xns_tli_send, &xns_tli_recv, &xns_tli_write, &xns_tli_read, nil(CMDP) };
#else
static	CMDP	xstli_cmds[] = { &xns_tli_send, &xns_tli_recv, nil(CMDP) };
#endif
static	CMDP	xdgtli_cmds[] = { &xns_tli_sndu, nil(CMDP) };
static	TESTS	xns_tli_tests[] = {
	{ xssrvr,	"XS TLI",	xstli_cmds,	xsclient },
	{ xdgsrvr,	"XDG TLI",	xdgtli_cmds,	xdgclient },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	xns_tli = { "XNS TLI Tests", xns_tli_tests };
#else
static	PFM	xns_tli = { "", nilp(TESTS) };
#endif

#ifdef	IPTLI
/* IP/TLI Test Family */
extern	int	tcpclient(   char * cmd   );
extern	int	tcpsrvr(   void   );
extern	int	udpclient(   char * ipaddr, int cnt, int len   );
extern	int	udpsrvr(   void   );
static	CMD	ip_tli_send = { "tcpsend", "tcpsend ip_addr send_cnt send_len [rep_cnt]" };
static	CMD	ip_tli_recv = { "tcprecv", "tcprecv ip_addr recv_cnt recv_len [rep_cnt]" };
#ifndef DOS
static	CMD	ip_tli_write = { "tcpwrite", "tcpwrite ip_addr write_cnt write_len [rep_cnt]" };
static	CMD	ip_tli_read = { "tcpread", "tcpread ip_addr read_cnt read_len [rep_cnt]" };
static	CMDP	tcptli_cmds[] = { &ip_tli_send, &ip_tli_recv, &ip_tli_write, &ip_tli_read, nil(CMDP) };
#else
static	CMDP	tcptli_cmds[] = { &ip_tli_send, &ip_tli_recv, nil(CMDP) };
#endif
static	CMD	ip_tli_udisc = { "udpdis", "udpdis ip_addr send_cnt send_len" };
static	CMD	ip_tli_udpecho = { "udpecho", "udpecho ip_addr send_cnt send_len" };
static	CMD	ip_tli_udpim = { "udpim", "udpim ip_addr send_cnt send_len" };
static	CMD	ip_tli_sndu = { "udpsnd", "udpsnd ip_addr send_cnt send_len" };
static	CMDP	udptli_cmds[] = { &ip_tli_udpecho, &ip_tli_udisc, &ip_tli_udpim, &ip_tli_sndu, nil(CMDP) };
static	TESTS	ip_tli_tests[] = {
	{ udpsrvr,	"UDP TLI",	udptli_cmds,	udpclient },
	{ tcpsrvr,	"TCP TLI",	tcptli_cmds,	tcpclient },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	ip_tli = { "IP TLI Tests", ip_tli_tests };
#else
static	PFM	ip_tli = { "", nilp(TESTS) };
#endif

#ifdef	IPSOCK
/* IP/Socket Test Family */
extern	int	so_tcpclient(   char * cmd   );
extern	int	so_tcpsrvr(   void   );
extern	int	so_udpclient(   char * ipaddr, int cnt, int len   );
extern	int	so_udpsrvr(   void   );
static	CMD	ip_so_send = { "stcpsend", "stcpsend ip_addr send_cnt send_len [rep_cnt]" };
static	CMD	ip_so_recv = { "stcprecv", "stcprecv ip_addr recv_cnt recv_len [rep_cnt]" };
#ifndef DOS
static	CMD	ip_so_write = { "stcpwrite", "stcpwrite ip_addr write_cnt write_len [rep_cnt]" };
static	CMD	ip_so_read = { "stcpread", "stcpread ip_addr read_cnt read_len [rep_cnt]" };
static	CMDP	tcpso_cmds[] = { &ip_so_send, &ip_so_recv, &ip_so_write, &ip_so_read, nil(CMDP) };
#else
static	CMDP	tcpso_cmds[] = { &ip_so_send, &ip_so_recv, nil(CMDP) };
#endif
static	CMD	ip_so_udisc = { "sudpdis", "sudpdis ip_addr send_cnt send_len" };
static	CMD	ip_so_udpecho = { "sudpecho", "sudpecho ip_addr send_cnt send_len" };
static	CMD	ip_so_udpim = { "sudpim", "sudpim ip_addr send_cnt send_len" };
static	CMD	ip_so_sndu = { "sudpsnd", "sudpsnd ip_addr send_cnt send_len" };
static	CMDP	udpso_cmds[] = { &ip_so_udpecho, &ip_so_udisc, &ip_so_udpim, &ip_so_sndu, nil(CMDP) };
static	TESTS	ip_so_tests[] = {
	{ so_udpsrvr,	"UDP Socket",	udpso_cmds,	so_udpclient },
	{ so_tcpsrvr,	"TCP Socket",	tcpso_cmds,	so_tcpclient },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	ip_so = { "IP Socket Tests", ip_so_tests };
#else
static	PFM	ip_so = { "", nilp(TESTS) };
#endif

#ifdef	XNSNP
/* XNS/Named Pipes Test Family */
extern	int	xnpcaller(   char * cmd   );
extern	int	xnpclient(   char * cmd   );
extern	int	xnpcsrvr(    void   );
extern	int	xnpsrvr(    void   );
static	CMD	xsnp_write = { "xnpwrite", "xnpwrite rmt_host_name write_cnt write_len" };
static	CMD	xsnp_read = { "xnpread", "xnpread rmt_host_name read_cnt read_len" };
static	CMD	xsnp_trans = { "xnptrans", "xnptrans rmt_host_name trans_cnt trans_len" };
static	CMD	xsnp_call = { "xnpcall", "xnpcall rmt_host_name call_cnt call_len" };
static	CMDP	xsnp_cmds[] = { &xsnp_write, &xsnp_read, &xsnp_trans, nil(CMDP) };
static	CMDP	xsnpc_cmds[] = { &xsnp_call, nil(CMDP) };
static	TESTS	xns_np_tests[] = {
	{ xnpsrvr,	"XS NP",	xsnp_cmds,	xnpclient },
	{ xnpcsrvr,	"XS CALLNP",	xsnpc_cmds,	xnpcaller },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	xns_np = { "XNS Named Pipes Tests", xns_np_tests };
#else
static	PFM	xns_np = { "", nilp(TESTS) };
#endif

#ifdef	NNP
/* Novell Named Pipes Test Family */
extern	int	nnpcaller(   char * cmd   );
extern	int	nnpclient(   char * cmd   );
extern	int	nnpcsrvr(    void   );
extern	int	nnpmisc(   char * cmd   );
extern	int	nnpmsrvr(   void   );
extern	int	nnpsrvr(    void   );
static	CMD	nnp_write = { "npwrite", "npwrite rmt_host_name write_cnt write_len" };
static	CMD	nnp_read = { "npread", "npread rmt_host_name read_cnt read_len" };
static	CMD	nnp_trans = { "nptrans", "nptrans rmt_host_name trans_cnt trans_len" };
static	CMD	nnp_rtrans = { "nprtrans", "nprtrans rmt_host_name rtrans_cnt rtrans_len" };
static	CMD	nnp_call = { "npcall", "npcall rmt_host_name call_cnt call_len [timeout]" };
static	CMD	nnp_peek = { "nppeek", "nppeek rmt_host_name write_cnt peek_cnt" };
static	CMD	nnp_get = { "npget", "npget rmt_host_name" };
static	CMDP	nnp_cmds[] = { &nnp_write, &nnp_read, &nnp_trans, &nnp_rtrans, nil(CMDP) };
static	CMDP	nnpc_cmds[] = { &nnp_call, nil(CMDP) };
static	CMDP	nnpm_cmds[] = { &nnp_peek, &nnp_get, nil(CMDP) };
static	TESTS	nnp_tests[] = {
	{ nnpsrvr,	"Novell NP",	nnp_cmds,	nnpclient },
	{ nnpcsrvr,	"Novell CallNP",nnpc_cmds,	nnpcaller },
	{ nnpmsrvr,	"Novell Misc NP", nnpm_cmds,	nnpmisc },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	nnp = { "Novell Named Pipes Tests", nnp_tests };
#else
static	PFM	nnp = { "", nilp(TESTS) };
#endif

#ifdef	NULLTEST
/* Null device test family */
extern	int	nullclient(   char * cmd   );
static	CMD	null_write = { "nullwrite", "nullwrite device cnt len" };
static	CMD	null_put = { "nullput", "nullput device cnt len" };
static	CMD	null_cput = { "nullcput", "nullcput device cnt len" };
static	CMD	null_sndu = { "nullsndu", "nullsndu device cnt len" };
static	CMD	null_read = { "nullread", "nullread device cnt len" };
static	CMD	null_get = { "nullget", "nullget device cnt len" };
#ifdef	CLIENT_NWPUT
static	CMD	null_nwput = { "nullnwput", "nullnwput device cnt len" };
static	CMDP	null_cmds[] = { &null_write, &null_put, &null_sndu
	, &null_cput, &null_read, &null_get, &null_nwput, nil(CMDP) };
#else
static	CMDP	null_cmds[] = { &null_write, &null_put, &null_sndu
	, &null_cput, &null_read, &null_get, nil(CMDP) };
#endif
static	TESTS	null_tests[] = {
	{ nil(pfi_t),	"Null Tests",	null_cmds,	nullclient },
	{ nil(pfi_t), 	nilp(char),	nilp(CMDP),	nil(pfi_t) },
};
static	PFM	null = { "Null Device Tests", null_tests };
#else
static	PFM	null = { "", nilp(TESTS) };
#endif

/* List of implemented test families */
static	PFMP	families[] = { &xns_tli, &nnp, &xns_np, &ip_tli, &ip_so, &null, nil(PFMP) };

extern	char	noshare * optarg;
extern	int	noshare optind;
	boolean	single_thread = false;
static	char	usage_str[] = "[-c | -s cmd]";

main (argc, argv)
	int	argc;
	char	** argv;
{
	int	ch;
	char	* cmd;
reg	char	* cp1;
reg	CMDP	* cmdp;
	boolean	do_help;
	int	err;
	char	* service = nilp(char);
reg	TESTSP	test;
reg	PFMPP	pfpp;

	set_program_name(argv[0]);
	while ( (ch = getopt(argc, argv, "s:S:cC")) != EOF ) {
		switch ( ch ) {
		case 's':
		case 'S':
			service = optarg;
			fallthru;
		case 'c':
		case 'C':
			single_thread = true;
			continue;
		default:
			usage(usage_str);
			/*NOTREACHED*/
		}
		continue;
	}
	if ( service ) {
		for ( pfpp = families; *pfpp != nil(PFMP); pfpp++ ) {
			for ( test = (*pfpp)->pfm_tests; test && test->tests_name != nilp(char); test++ ) {
				for ( cmdp = test->tests_cmds; *cmdp != nil(CMDP); cmdp++ ) {
					if ( strncmp((*cmdp)->cmd_cmd, service, strlen((*cmdp)->cmd_cmd)) == 0 ) {
						printf("mtest: starting server for '%s'\n", test->tests_name);
						(*test->tests_server)();
						exit(1);
					}
				}
			}
		}
		fatal("couldn't find server for '%s'\n", service);
	}
	if ( !single_thread ) {
		/* Start the server threads */
		for ( pfpp = families; *pfpp != nil(PFMP); pfpp++ ) {
			for ( test = (*pfpp)->pfm_tests; test && test->tests_name != nilp(char); test++ ) {
				if ( test->tests_server == nil(pfi_t) )
					continue;
				if ( (err = CreateThread(test->tests_server, nilp(u32), nilp(char), (u32)STACK_SIZE, nil(u32))) != 0 )
					warn("Couldn't create server thread for %s, (error code %d)\n", test->tests_name, err);
			}
		}
	}
	while ( cmd = os_get_cmd() ) {
		for ( cp1 = cmd; isspace(*cp1); cp1++ )
			noop;
		do_help = false;
		switch ( *cp1 ) {
		case '?':
			do_help = true;
			break;
		case 'q':
		case 'Q':
			ExitThread(1, 1);
		default:
			break;
		}
		cmdp = nilp(CMDP);
		for ( pfpp = families; *pfpp != nil(PFMP); pfpp++ ) {
			for ( test = (*pfpp)->pfm_tests; test && test->tests_name != nilp(char); test++ ) {
				for ( cmdp = test->tests_cmds; *cmdp != nil(CMDP); cmdp++ ) {
					if ( !do_help ) {
						if ( strncmp((*cmdp)->cmd_cmd, cp1, strlen((*cmdp)->cmd_cmd)) == 0 )
							goto gotit;
					} else
						printf("%s\n", (*cmdp)->cmd_help);
				}
			}
		}
		if ( do_help ) {
			printf("quit\n");
			free(cmd);
			continue;
		}
		warn("command not recognized\n");
		free(cmd);
		continue;
gotit:;
		if ( single_thread ) {
			(*test->tests_client)(cmd);
			continue;
		}
		if ( (err = CreateThread(test->tests_client, nilp(u32), nilp(char), (u32)STACK_SIZE, (u32)cmd)) != 0 )
			warn("Couldn't create client thread (error code %d)\n", err);
	}
}
