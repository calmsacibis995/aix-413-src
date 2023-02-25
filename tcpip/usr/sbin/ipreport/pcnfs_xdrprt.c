static char sccsid[] = "@(#)93	1.4  src/tcpip/usr/sbin/ipreport/pcnfs_xdrprt.c, tcpras, tcpip411, GOLD410 4/1/93 17:54:20";
/*
 * COMPONENT_NAME: TCPIP pcnfs_xdrprt.c
 *
 * ORIGINS: 24 27
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 * 
 * This file contains the XDR and PRT routines to decode and print PCnfs
 * rpc packets for the ipreport command.
 * 
 * It was mostly stolen and modified from
 * nfs312:com/pcnfs/pcnfsd/pcnfsd.c
 * 
 * Created by on Tue Aug 27 14:51:47 CDT 1991
 */



#include <uinfo.h>
#include <sys/types.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <pwd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <netdb.h>
#include "ipr.h"

int             buggit = 0;
static	char * pc_beg="PC:  ";

/*
 * *************** RPC parameters ******************** 
		see ipr.h
*/

/*
 * ************* Other #define's ********************** 
 */
#ifndef SPOOLDIR
# define SPOOLDIR	"/usr/tmp"
#endif SPOOLDIR

#ifndef MAXPATHLEN
# define MAXPATHLEN 1024
#endif
#define	zchar		0x5b

#define assert(ex) {if (!(ex)) \
    {fprintf(stderr,"Assertion failed: line %d of %s: \"%s\"\n", \
    __LINE__, __FILE__, "ex"); \
    sleep (30); exit(1);}}

/*
 * *********** XDR structures, etc. ******************** 
 */
enum arstat {
	AUTH_RES_OK, AUTH_RES_FAKE, AUTH_RES_FAIL
};
enum pirstat {
	PI_RES_OK, PI_RES_NO_SUCH_PRINTER, PI_RES_FAIL
};
enum psrstat {
	PS_RES_OK, PS_RES_ALREADY, PS_RES_NULL, PS_RES_NO_FILE,
	PS_RES_FAIL
};

struct auth_args {
	char           *aa_ident;
	char           *aa_password;
};

struct auth_results {
	enum arstat     ar_stat;
	long            ar_uid;
	long            ar_gid;
};

struct pr_init_args {
	char           *pia_client;
	char           *pia_printername;
};

struct pr_init_results {
	enum pirstat    pir_stat;
	char           *pir_spooldir;
};

struct pr_start_args {
	char           *psa_client;
	char           *psa_printername;
	char           *psa_username;
	char           *psa_filename;	/* within the spooldir */
	char           *psa_options;
};

struct pr_start_results {
	enum psrstat    psr_stat;
};


/*
 * ****************** Misc. ************************ 
 */

char           *authproc();
char           *pr_start();
char           *pr_init();
struct passwd  *getpwnam();
struct stat     statbuf;

char            pathname[MAXPATHLEN];
char            new_pathname[MAXPATHLEN];
char            spoolname[MAXPATHLEN];

/*
 * ************** Support procedures *********************** 
 */
scramble(s1, s2)
	char           *s1;
	char           *s2;
{
	while (*s1) {
		*s2++ = (*s1 ^ zchar) & 0x7f;
		s1++;
	}
	*s2 = 0;
}

free_child()
{
	int             pid;
	int             pstatus;

	pid = wait(&pstatus);	/* clear exit of child process */

	if (buggit || pstatus)
		fprintf(stderr, "FREE_CHILD: process #%d exited with status 0X%x\n",
			pid, pstatus);
	return;
}

/*
 * *************** XDR procedures ***************** 
 */
bool_t
xdr_auth_args(xdrs, aap)
	XDR            *xdrs;
	struct auth_args *aap;
{
	return (xdr_string(xdrs, &aap->aa_ident, 32) &&
		xdr_string(xdrs, &aap->aa_password, 64));
}

bool_t
xdr_auth_results(xdrs, arp)
	XDR            *xdrs;
	struct auth_results *arp;
{
	return (xdr_enum(xdrs, &arp->ar_stat) &&
		xdr_long(xdrs, &arp->ar_uid) &&
		xdr_long(xdrs, &arp->ar_gid));
}

bool_t
xdr_pr_init_args(xdrs, aap)
	XDR            *xdrs;
	struct pr_init_args *aap;
{
	return (xdr_string(xdrs, &aap->pia_client, 64) &&
		xdr_string(xdrs, &aap->pia_printername, 64));
}

bool_t
xdr_pr_init_results(xdrs, arp)
	XDR            *xdrs;
	struct pr_init_results *arp;
{
	return (xdr_enum(xdrs, &arp->pir_stat) &&
		xdr_string(xdrs, &arp->pir_spooldir, 255));
}

bool_t
xdr_pr_start_args(xdrs, aap)
	XDR            *xdrs;
	struct pr_start_args *aap;
{
	return (xdr_string(xdrs, &aap->psa_client, 64) &&
		xdr_string(xdrs, &aap->psa_printername, 64) &&
		xdr_string(xdrs, &aap->psa_username, 64) &&
		xdr_string(xdrs, &aap->psa_filename, 64) &&
		xdr_string(xdrs, &aap->psa_options, 64));
}

bool_t
xdr_pr_start_results(xdrs, arp)
	XDR            *xdrs;
	struct pr_start_results *arp;
{
	return (xdr_enum(xdrs, &arp->psr_stat));
}


/*
 * *************** PRT procedures ***************** 
 */
prt_ar_stat(arstat)
int	arstat;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	switch (arstat) {
		case AUTH_RES_FAIL: printf("AUTH_RES_FAIL"); break; 
		case AUTH_RES_FAKE: printf("AUTH_RES_FAKE"); break; 
		case AUTH_RES_OK:   printf("AUTH_RES_OK");   break; 
		default:	    printf("Bad ARSTAT!");   break; 
	}
	printf(" (%d)\n",arstat);
}

prt_pir_stat(pires)
int pires;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	switch (pires) {
	case PI_RES_FAIL:            printf("PI_RES_FAIL ");            break; 
	case PI_RES_NO_SUCH_PRINTER: printf("PI_RES_NO_SUCH_PRINTER "); break; 
	case PI_RES_OK:              printf("PI_RES_OK ");              break; 
	default:	             printf("Bad PI RES! ");            break; 
	}
	printf(" (%d)\n",pires);
}

prt_ps_res(psres)
int psres;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	switch (psres) {
		case     PS_RES_ALREADY: printf("PS_RES_ALREADY "); break; 
		case     PS_RES_FAIL:    printf("PS_RES_FAIL ");    break; 
		case     PS_RES_NO_FILE: printf("PS_RES_NO_FILE "); break; 
		case     PS_RES_NULL:    printf("PS_RES_NULL ");    break; 
		case     PS_RES_OK:      printf("PS_RES_OK ");      break; 
		default:                 printf("Bad PS RES! ");    break; 
	}
	printf(" (%d)\n",psres);
}

bool_t
prt_auth_args(aap)
	struct auth_args *aap;
{
        char            username[32];

	reset_beg_line(pc_beg);

        scramble(aap->aa_ident, username);


	printf("%s",beg_line);
	printf("(Scrambled)   Ident=%s Passwd=%s\n", 
				aap->aa_ident, aap->aa_password);
	printf("%s",beg_line);
	printf("(Unscrambled) Ident=%s \n", username );
	return (TRUE);
}

bool_t
prt_auth_results(arp)
	struct auth_results *arp;
{
	reset_beg_line(pc_beg);

	prt_ar_stat(arp->ar_stat);

	printf("%s",beg_line);
	printf("Uid=%u Gid=%u\n",arp->ar_uid,arp->ar_gid);
}

bool_t
prt_pr_init_args(aap)
	struct pr_init_args *aap;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	printf("Client=%s Printername=%s\n", 	aap->pia_client, 
						aap->pia_printername);
}

bool_t
prt_pr_init_results(arp)
	struct pr_init_results *arp;
{
	reset_beg_line(pc_beg);

	prt_pir_stat(arp->pir_stat);

	printf("%s",beg_line);
	printf("Spooldir=%s\n", arp->pir_spooldir);
}

bool_t
prt_pr_start_args(aap)
	struct pr_start_args *aap;
{
	reset_beg_line(pc_beg);
	printf("%s",beg_line);

	printf("Client=%s Printername=%s Username=%s Filename=%s Options=%s\n",
				aap->psa_client,
				aap->psa_printername,
				aap->psa_username,
				aap->psa_filename,
				aap->psa_options);
}

bool_t
prt_pr_start_results(arp)
	struct pr_start_results *arp;
{
	reset_beg_line(pc_beg);

	prt_ps_res(arp->psr_stat);
}
