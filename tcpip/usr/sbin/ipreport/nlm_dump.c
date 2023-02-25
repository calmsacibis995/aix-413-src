static char sccsid[] = "@(#)91	1.8  src/tcpip/usr/sbin/ipreport/nlm_dump.c, tcpras, tcpip411, 9436E411a 9/9/94 10:10:34";
/*
 * COMPONENT_NAME: TCPIP nlm_dump.c
 *
 * ORIGINS: 24 27
 *
 * FUNCTIONS: prt_fsh_access, prt_fsh_mode, prt_nlm_cancargs, prt_nlm_holder, 
 *            prt_nlm_lock, prt_nlm_lockargs, prt_nlm_notify, prt_nlm_res, 
 *            prt_nlm_share, prt_nlm_shareargs, prt_nlm_shareres, 
 *            prt_nlm_stat, prt_nlm_stats, prt_nlm_testargs, 
 *            prt_nlm_testres, prt_nlm_testrply, prt_nlm_unlockargs 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 * 
 * 
 * This code is modified xdr_nlm.c stolen from lockd- 
 * Tue Aug 20 16:41:32 CDT 1991
 * 
 */


#include "prot_lock.h"
#include "ipr.h"

extern char *lok_beg;

prt_bool(x)
bool_t x;
{
	if (x)
		printf("TRUE ");
	else
		printf("FALSE ");
}

prt_netobj(no)
	struct netobj *no;
{
	hex_dump(no->n_bytes,no->n_len);
}

prt_netobj_fh(no)
	struct netobj *no;
{
	reset_beg_line(lok_beg);

	printf("%s",lok_beg);
	printf( "         ");
	hex_dump_line(no->n_bytes,no->n_len/2);
	printf( "\n");
	printf("%s",lok_beg);
	printf( "         ");
	hex_dump_line((char *)no->n_bytes+(no->n_len/2),no->n_len/2);
	printf( "\n");
}

bool_t
prt_nlm_stats(objp)
	nlm_stats *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);
	printf("Status=");
	switch(*objp) {
	case nlm_granted:
		printf("GRANTED");
		break;
	case nlm_denied:
		printf("DENIED");
		break;
	case nlm_denied_nolocks:
		printf("DENIED_NOLOCKS");
		break;
	case nlm_blocked:
		printf("BLOCKED");
		break;
	case nlm_denied_grace_period:
		printf("DENIED_GRACE_PERIOD");
		break;
	case nlm_deadlck:
		printf("DENIED_DEADLOCK");
		break;
	default:
		printf("Protocol Error, Bad nlm_stats");
		break;
	}
	printf(" (%d)\n",*objp);
	return (*objp);
}

bool_t
prt_nlm_holder(objp)
	nlm_holder *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);

	printf("(Start Holder Struct):\n");

	printf("%s",beg_line);
	printf("Exclusive=");
	prt_bool(objp->exclusive);
	printf("Server ID=%d ",objp->svid);

	prt_netobj(&objp->oh);

	printf("%s",beg_line);
	printf("Offset=0x%x ",objp->l_offset);
	printf("Length=0x%x\n", objp->l_len);

	printf("%s",beg_line);
	printf("(End Holder Struct):\n");

	return (TRUE);
}

bool_t
prt_nlm_testrply(objp)
	nlm_testrply *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);

	printf("NLM Testrply:\n");

	prt_nlm_stats(&objp->stat);

	switch (objp->stat) {
	case nlm_denied:
		prt_nlm_holder(&objp->nlm_testrply_u.holder);
		break;
	}
}

prt_nlm_stat(objp)
	nlm_stat *objp;
{
	prt_nlm_stats(&objp->stat) ;
}

prt_nlm_res(objp)
	remote_result *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("Result: Cookie=\n");

	prt_netobj(&objp->cookie);
	prt_nlm_stat(&objp->stat);

	return (TRUE);
}

bool_t
prt_nlm_testres(objp)
	remote_result *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("nlm_testres: Cookie=\n");
	prt_netobj(&objp->cookie); 
	printf("nlm_testres: stat:\n");
	prt_nlm_testrply(&objp->stat);

	return (TRUE);
}


bool_t
prt_owner(no)
	struct netobj *no;
{
	char *ohb;
	int *pidp;
	char *pid="      ";

	printf("%s",beg_line);
	printf("Owner handle: \n");
	prt_netobj(no); 

	printf("%s",beg_line);
	ohb= no->n_bytes;
	bcopy(ohb+strlen(ohb)+1,pid,sizeof(int));
	pidp= (int *) (pid);
	printf("OH is opaque but is probably Hostname=%s, Pid=%d\n",
			ohb, *pidp);
	return (TRUE);
}


bool_t
prt_nlm_lock(objp)
	struct alock *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);

	printf("(Start aLock Struct) \n");

	printf("%s",beg_line);
	printf("Caller Name=%s\n",objp->caller_name);

	printf("%s",beg_line);
	printf("File handle: \n");
	prt_netobj_fh(&objp->fh); 

	prt_owner(&objp->oh); 

	printf("%s",beg_line);
	printf("Server ID=%d ",objp->svid);

	printf("Offset=0x%x ",objp->l_offset);
	printf("Length=0x%x\n", objp->l_len);

	printf("%s",beg_line);
	printf("(End aLock Struct)\n");

	return (TRUE);
}

bool_t
prt_nlm_lockargs(objp)
	reclock *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("(Start Lock Args)\n");

	printf("%s",beg_line);
	printf("Reclock=\n");
	prt_netobj( &objp->cookie);

	printf("%s",beg_line);
	printf("block=");
	prt_bool( objp->block);
	printf("exclusive=");
	prt_bool( objp->exclusive);
	printf("\n");

	prt_nlm_lock( &objp->alock);

	printf("%s",beg_line);
	printf("Reclaim=");
	prt_bool( objp->reclaim);
	printf("State=%d\n",objp->state);

	printf("%s",beg_line);
	printf("(End Lock Args)\n");

	return (TRUE);
}




bool_t
prt_nlm_cancargs(objp)
	reclock *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("Cancel Arguments:\n");

	printf("%s",beg_line);
	printf("Cookie=\n");
	prt_netobj(&objp->cookie);

	printf("%s",beg_line);
	printf("Block=");
	prt_bool(objp->block);
	printf("Exclusive=");
	prt_bool(objp->exclusive);
	printf("\n");

	prt_nlm_lock(&objp->alock);

	return (TRUE);
}




bool_t
prt_nlm_testargs(objp)
	reclock *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("Test Arguments:\n");

	prt_netobj(&objp->cookie);

	printf("%s",beg_line);
	printf("Exclusive=");
	prt_bool(objp->exclusive);
	printf("\n");

	prt_nlm_lock(&objp->alock);

	return (TRUE);
}




bool_t
prt_nlm_unlockargs(objp)
	reclock *objp;
{
	reset_beg_line(lok_beg);


	printf("%s",beg_line);
	printf("Unlock Arguments: Cookie=\n");
	prt_netobj(&objp->cookie);

	prt_nlm_lock(&objp->alock);
	return (TRUE);
}


bool_t
prt_fsh_mode(objp)
	fsh_mode *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);

	switch (*objp) {
		case    fsm_DN:  printf("fsm_DN\n");  break; 
		case    fsm_DR:  printf("fsm_DR\n");  break; 
		case    fsm_DW:  printf("fsm_DW\n");  break; 
		case    fsm_DRW: printf("fsm_DRW\n"); break; 
		default:         printf("Bad fsh_mode\n"); break; 
	}

	return (TRUE);
}




bool_t
prt_fsh_access(objp)
	fsh_access *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);

	switch (*objp) {
		case    fsa_NONE: printf("fsa_NONE\n");     break; 
		case    fsa_R:    printf("fsa_DR\n");       break; 
		case    fsa_W:    printf("fsa_DW\n");       break; 
		case    fsa_RW:   printf("fsa_DRW\n");      break; 
		default :         printf("Bad fsh_mode\n"); break; 
	}

	return (TRUE);
}




bool_t
prt_nlm_share(objp)
	nlm_share *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("SHARE: Caller_name=%s\n",objp->caller_name );

	printf("%s",beg_line);
	printf("File Handle: \n");
	prt_netobj_fh(&objp->fh);

	prt_owner(&objp->oh);

	printf("%s",beg_line);
	printf("Mode=%d Access=%d\n",objp->mode,objp->access );

	return (TRUE);
}




bool_t
prt_nlm_shareargs(objp)
	nlm_shareargs *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("Share arguments: Cookie=\n");
	prt_netobj(&objp->cookie);
	prt_nlm_share(&objp->share);

	printf("%s",beg_line);
	printf("Reclaim=");
	prt_bool(objp->reclaim);
	printf("\n");
	return (TRUE);
}




bool_t
prt_nlm_shareres(objp)
	nlm_shareres *objp;
{
	reset_beg_line(lok_beg);
	printf("%s",beg_line);
	printf("Share Results: Cookie=\n");
	prt_netobj(&objp->cookie);
	prt_nlm_stats(&objp->stat);
	printf("%s",beg_line);
	printf("Sequence=%d \n",objp->sequence);
	return (TRUE);
}




bool_t
prt_nlm_notify(objp)
	nlm_notify *objp;
{
	reset_beg_line(lok_beg);

	printf("%s",beg_line);
	printf("Notify: Name=%s State=%d \n",objp->name,objp->state);

	return (TRUE);
}


