static char sccsid[] = "@(#)15	1.8  src/bos/usr/sbin/install/inulib/inu_signal.c, cmdinstl, bos411, 9430C411a 8/1/94 08:56:01";
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS: 
 *	inu_signal
 *	inu_ignore_all_sigs
 *	inu_setall_sigs
 *	inu_init_sigs
 *	inu_default_sigs
 *	inu_before_system_sigs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>

void (* inu_signal(int sig, void (* funky)()))(int);
void inu_set_abort_handler(void (* funky)());
void inu_ignore_all_sigs();
void inu_setall_sigs(void (* funky)());
void inu_init_sigs();
void inu_default_sigs();
void inu_before_system_sigs();

/* function pointer to signal handler */
static void (* inu_sig_funk)()=SIG_DFL;
static void (* inu_sig_hup)()=SIG_DFL;

/* array of signals to act on */
static int sigs[]=
{
	SIGINT,
	SIGQUIT
};
#define NUMSIGS sizeof(sigs)/sizeof(int)


/* setup a new style signal handler */
void (*inu_signal(int sig, void (* funky)()))(int)
{
	struct sigaction action;
	struct sigaction O_action;
	bzero(&action,sizeof(action));
	action.sa_handler=funky;
	sigaction(sig,&action,&O_action);
	return(O_action.sa_handler);
}

/* set what the abort signal handler should be */
void inu_set_abort_handler(void (* funky)())
{
	inu_sig_funk=funky;
	inu_setall_sigs(funky);
	if((inu_sig_hup=inu_signal(SIGHUP,funky))==SIG_IGN)
		inu_signal(SIGHUP,SIG_IGN);
	else
		inu_sig_hup=funky;
}

/* ignore all our chosen signals */
void inu_ignore_all_sigs()
{
	inu_setall_sigs(SIG_IGN);
	inu_signal(SIGHUP,SIG_IGN);
}

/* set all signals to have funky as there signal catcher */
void inu_setall_sigs(void (* funky)())
{
	int i;

	for(i=0;i<NUMSIGS;i++)
		inu_signal(sigs[i],funky);
}

/* initialize signals to the "our" signal handler */
void inu_init_sigs()
{
	inu_setall_sigs(inu_sig_funk);
	inu_signal(SIGHUP,inu_sig_hup);
}

/* set out set of signals to the default signal handlers */
void inu_default_sigs()
{
	inu_setall_sigs(SIG_DFL);
	inu_signal(SIGHUP,inu_sig_hup);
}

/* initialize signals to the DEFAULT signal handler but ignore
 * some for critical code.
 */
void inu_before_system_sigs()
{
	inu_default_sigs();
}
