static char sccsid[] = "@(#)27  1.9  src/bos/usr/bin/ate/signal.c, cmdate, bos411, 9428A410j 12/8/93 16:33:26";
/* 
 * COMPONENT_NAME: BOS signal.c
 * 
 * FUNCTIONS: MSGSTR, sigrout, sig_set 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/****************************************************************************/
/*                                                                          */
/* Module name:      signal.c                                               */
/* Description:      trap signals; do whatever is needed based on signal    */
/* Functions:        sigrout - signal handler                               */
/*                   sig_set - set up signal traps                           */
/*                                                                          */
/* Compiler:         See Makefile                                           */
/* External refs:    modem.h - global variables                             */
/*   Called by:      main in main.c (sig_set)                                */
/*   Receives:       signal                                                 */
/*   Returns:        nothing                                                */
/*   Abnormal exits: none                                                   */
/*   Calls:          vtdraw in portvt.c (refresh screen)                    */
/*                   redraw in portrw.c (repaint connection screen after a  */
/*                     menu)                                                */
/*                   retopt, setopt in setups.c (set terminal parameters)   */
/*                   initmod in setups.c (set port parameters)              */
/*                   cls in cls.c (clear screen)                            */
/*                   hangup in hangup.c (end connection)                    */
/*                   message in message.c (print user errors)               */
/*   Modifies:       userin, retcode                                        */
/*                                                                          */
/****************************************************************************/

#include <regex.h>
#include <langinfo.h>
#include "modem.h"

#include <nl_types.h>
#include "ate_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ATE,n,s) 

int rpmatch();		/* returns a yes or no resonse */
extern int exmask;	/* select exception mask in connect.c */

#define MAXSTR	10	/* max number of bytes in yes/no response */

/* ------------------------------------------------------------------------
   sigrout - signal router.  Do processing based on signal type:
      SIGINT  - interrupt signal generated by user (^r).  'state' is a global
                variable used to determine what actions are required.
      SIGTERM - software termination signal
      SIGHUP  - hang up signal
      SIGALRM - alarm clock signal
   ------------------------------------------------------------------------ */
void sigrout(int sig)
{
 int i;
 char line[MAXSTR];              	    /* buffer for yes/no response */
 char *yresp = "y";
 char *nresp = "n";
 char *ptr ;
 char c;

#ifdef DEBUG
kk = sprintf(ss,"entering sigrout with %d\n",sig);
write(fe,ss,kk);
#endif DEBUG

 retcode = EXIT;                            /* set return code to exit calling routine */
 
 switch(sig)
   {
   case SIGINT  : signal(SIGINT,(void(*)(int)) sigrout);   /* reset trap */
                  switch (state)
                    {
/* main    */       case 0 :
/* cmd     */       case 1 : break;
/* direc   */       case 2 : fclose(fd);    /* close directory file */
                             break;
/* conn    */       case 3 : if (connect)             /* a conn is made */
                               {
                               retopt();              /* original term params */
                               cls();                 /* clear screen */
			/* get the 'yes' and 'no' response string for locale */
				yresp =  nl_langinfo(YESSTR);
				ptr = (char *) malloc(sizeof(yresp));
				strcpy(ptr,yresp);
				yresp = ptr ;
				if(!yresp) yresp = "y";
				nresp =  nl_langinfo(NOSTR);
				if(!nresp) nresp = "n";

                               printf(MSGSTR(DISCONN, "Disconnect (%s or %s)?  "), yresp, nresp); /*MSG*/
			       fflush(stdout);
			       i = 0;
			       while ((c = getchar()) != '\n' && c != (char)EOF)
					if (i < MAXSTR - 1)
						line[i++] = c;
			       line[i] = '\0';
			/* determine what the response was */
			       i = rpmatch(line);
			       if ((i == 0) || (i < 0)) 
                                 {
                                 retcode = (int)NULL;     /* retcode = normal */
				 exmask=0;	      /* reset exception mask */
                                 setopt();            /* term params for conn */
				 if (vt100) vtdraw(); /* refresh vt100 screen */
				 else redraw();	      /* refresh normal scrn */
                                 }
                               else 
                                 hangup();            /* terminate connection */
                               }
			     else
			     {
			       if(locfile)
			       {
				ttyunlock(port);	/* get rid of lock file */
				locfile=OFF;  /* reset lockfile indicator */
			       }
			     }
                             break;
/* help    */       case 4 : break;
/* alter,modify */  case 5 : break;
/* send    */       case 6 : close(fs);     /* close send file */
                                            /* reset port for xmodem */
                             initmod(lpath,OFF,ON,0,0,0,0);
                             break;
/* receive */       case 7 : close(fr);     /* close receive file */
                                            /* reset port for xmodem */
                             initmod(lpath,OFF,ON,0,0,0,0);
                             break;
                    default: printf(MSGSTR(NOSUCH, "No such state as %d\r\n"),state); /*MSG*/
                             break;
                   }
                  break;
             
   case SIGTERM : signal(SIGTERM,(void(*)(int)) sigrout);  /* software termination signal */
                  hangup();                 /* end connection */
                  retcode=QUIT;             /* end program */
                  break;
                  
   case SIGHUP  : signal(SIGHUP,(void(*)(int)) sigrout);   /* hangup  - loss of Cxr Detect */
                  message(56);              /* no carrier */
                  hangup();                 /* end connection */
                  break;
                  
   case SIGALRM : signal(SIGALRM,(void(*)(int)) sigrout);  /* reset the trap */
                  retcode=(int)NULL;             /* set retcode to normal */
                  break;
                  
   default      : hangup();                 /* terminate connection */
                  break;
   }
   
 alarm(0);                                  /* turn off the alarm */

#ifdef DEBUG
kk = sprintf(ss,"leaving sigrout\n");
write(fe,ss,kk);
#endif DEBUG

}


/* -------------------------------------------------------------------------
   Sigset - set up signal traps
   ------------------------------------------------------------------------- */
sig_set()
{

#ifdef DEBUG
kk = sprintf(ss,"entering sig_set\n");
write(fe,ss,kk);
#endif DEBUG

 signal(SIGINT,(void(*)(int)) sigrout);        /* interrupt */
 signal(SIGTERM,(void(*)(int)) sigrout);       /* software termination sig from kill */
 signal(SIGHUP,(void(*)(int)) sigrout);        /* hangup */
 signal(SIGALRM,(void(*)(int)) sigrout);       /* alarm clock */

#ifdef DEBUG
kk = sprintf(ss,"leaving sig_set\n");
write(fe,ss,kk);
#endif DEBUG

}