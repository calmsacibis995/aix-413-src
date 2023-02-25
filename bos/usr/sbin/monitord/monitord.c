static char sccsid[] = "@(#)92	1.2  src/bos/usr/sbin/monitord/monitord.c, netls, bos411, 9428A410j 4/4/94 08:10:39";
/*
 *   COMPONENT_NAME: netls
 *
 *   FUNCTIONS: check_args
 *		get_saved_hbeat
 *		main
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <nl_types.h>
#include "monitord.h"

nl_catd admin_catd;     /* Message catalog pointer.  Used by all modules.  */
int softstop = 0;	/* Softstop flag.  0=hardstop, 1=softstop.         */

/*
 * NAME:	main
 *
 * FUNCTION: 	Check command line arguments and whether the daemon has 
 *		already been started.  If everything ok, then start the
 *		monitord daemon.
 * 
 * RETURNS:	-1 on exit when system error.
 *		 1 on exit when user error.
 */
void main(int argc, char *argv[])
{
  int lock;
  pid_t forkpid = 0;
  int hbeat_interval = 0;

  (void) setlocale(LC_ALL,"");
  admin_catd = catopen("monitord_admin.cat", NL_CAT_LOCALE); 

  if (geteuid() != 0)
  {
    fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, NOT_ROOT,
                "You must have root privileges to start monitord.\n"));
    exit(1);
  }


  hbeat_interval = check_args(argc, argv);  /* check the command line args */
                                            /* and set the heartbeat       */

  if (hbeat_interval < 0)                   /* there was a cmd syntax err  */
    exit(1);

  /* Check for the existence of the lock file.  If it exists, then the     */
  /* daemon has already been started, so exit.                             */
  if ((lock = open(LOCK_FILE, O_RDONLY)) < 0)
  {
    #ifdef NETLS_DEBUG
    printf("spawning the daemon.\n");
    #endif

    if ((lock = open(LOCK_FILE, O_WRONLY|O_CREAT, 0600)) < 0)
    {
      log_sys(1, catgets(admin_catd, MONITORD_ADMIN, LOCK_UNABLE,
      "0349-010 Monitord is unable to create the lockfile /etc/security/monitord_lock\n"));
      exit(-1);
    }
    else
    {
      close(lock);                    /* don't need to keep the file open */

      if ( (forkpid = fork()) < 0)
      {
        log_sys(1, catgets(admin_catd, MONITORD_ADMIN, FORK_DAEMON, 
                 "0349-011 Monitord is unable to fork the daemon process.\n"));
	exit(-1);
      }
      else
        if (forkpid == 0)
          server(hbeat_interval);   /* child becomes daemon */
        else
          exit(0);                  /* parent exits */
    }
  }
  else
  {
    fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, LOCK_MSG,
    "Monitord is running, or cleanup is required.\n : Remove /etc/security/monitord_lock before restarting.\n"));
    log_sys(0, catgets(admin_catd, MONITORD_ADMIN, LOCK_MSG,
    "Monitord is running, or cleanup is required.\n : Remove /etc/security/monitord_lock before restarting.\n"));
    exit(-1);
  }

  exit(1);
}


/*
 * NAME:	check_args
 *
 * FUNCTION:	Checks the command line argument for the user-defined heart-
 *              beat interval.  If the syntax is incorrect, then print an 
 *              error msg and return -1 for the heartbeat.  If a heartbeat
 *		interval is defined by the user, then save it to the config
 *		file.  If no heartbeat interval is defined by the user then
 *		attempt to read a previously defined interval from the config
 *		file.  If no saved interval value is retrieved, then return
 *		the default interval.
 *
 * RETURNS:	Heartbeat interval specified by the user.
 *		-1 if invalid arguments specified.
 */
int check_args (int argc, char *argv[])
{
  int i;
  int c;
  int tflg = 0;
  #ifdef SOFTSTOP
  int sflg = 0;
  #endif
  int options = 0;
  char char_hbeat[32];
  int int_hbeat = -1;
  int errflg = 0;
  int hbeat_len = 0;
  long saved_hbeat = 0;
  char heart_sav[75];
  int heart_savlen = 0;
  int savefd;

  #ifdef SOFTSTOP
  while ((c = getopt(argc, argv, "t:s")) != EOF)
  #else
  while ((c = getopt(argc, argv, "t:")) != EOF)
  #endif
  {
    options++;
    switch(c)
    {
      case 't': strcpy(char_hbeat, optarg);
                tflg++;
                break;
  #ifdef SOFTSTOP
      case 's': sflg++;
                break;
  #endif
      case '?':
      default : errflg++;
    }

    if (errflg)
    {
      #ifdef SOFTSTOP
      fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, SOFT_USAGE, 
              "usage: monitord [-s] [-t Minutes]\n"));
      #else
      fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, USAGE, 
              "usage: monitord [-t Minutes]\n"));
      #endif
      return(-1);
    }
  }


  if (tflg)
  {
    /* was previously in "for" condition, but pulled out for performance. */
    hbeat_len = strlen(char_hbeat);

    for (i = 0; i < hbeat_len; i++)
      if ( isdigit(char_hbeat[i]) == 0 )
      {
        fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, BAD_MINUTES,
                "monitord: minutes argument must be a whole number.\n"));
        return(-1);
      }

    /* multiply the specified number of minutes by 60 to get seconds */
    int_hbeat = (atoi(char_hbeat) * 60);

    if (int_hbeat == 0)
      int_hbeat = MAX_H_INTERVAL;
  }

  #ifdef SOFTSTOP
  if (sflg)
  {
    #ifdef NETLS_DEBUG
    printf("softstop!\n");
    #endif
    softstop = 1;
  }
  #endif

  if ( (options == 0) && (argc > 1) )
  {
    #ifdef SOFTSTOP
    fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, SOFT_USAGE, 
            "usage: monitord [-s] [-t Minutes]\n"));
    #else
    fprintf(stderr, catgets(admin_catd, MONITORD_ADMIN, USAGE, 
            "usage: monitord [-t Minutes]\n"));
    #endif
    return(-1);
  }

  if (int_hbeat < 0)   /* No hbeat set in command line option, so check  */
  {                    /* for a saved value in the config file.          */
    saved_hbeat = get_saved_hbeat();

    if (saved_hbeat < 0)               /* no saved value, or config file */
      int_hbeat = DEF_H_INTERVAL;      /* was not valid, so use default  */
    else
      if (saved_hbeat == 0)            /* saved value was "INFINITE"     */
        int_hbeat = MAX_H_INTERVAL;
      else                             /* saved value was some ok number */
        int_hbeat = (saved_hbeat * 60);
  }
  else                /* The hbeat interval was set by the administrator */
  {                   /* and should be saved to the config file.         */
    unlink(CONFIG_FILE);

    strcpy(heart_sav, KEYWORD);

    if (int_hbeat == MAX_H_INTERVAL)
      strcat(heart_sav, MAX_KEYWORD);
    else
      strcat(heart_sav, char_hbeat);

    strcat(heart_sav, "\n");

    heart_savlen = strlen(heart_sav);

    savefd = open(CONFIG_FILE, O_WRONLY|O_CREAT|O_APPEND, 0600);
    if (savefd >= 0)
    {
      write(savefd, heart_sav, heart_savlen);
      close(savefd);
    }
    else
    {
    log_sys(1, catgets(admin_catd, MONITORD_ADMIN, CFG_NOSAVE,
    "0349-019 Monitord could not write to the file /etc/security/monitord.cfg\n"));
    }
  }


  #ifdef NETLS_DEBUG
  printf("checkargs returning %d for the hearbeat interval.\n", int_hbeat);
  #endif
  return(int_hbeat);

}


/*
 * NAME:	get_saved_hbeat
 *
 * FUNCTION:	Reads the contents of the configuration file, where the
 *		administrator-defined heartbeat interval was saved.
 *
 * RETURNS:	-1 if the config file does not exists, or if the contents
 *			are invalid.
 *		Otherwise, returns the long int value of the saved heartbeat.
 */
long get_saved_hbeat()
{
  int parsefd;				/* file descriptor for reading     */
  char buf[MAXCHARS],			/* buffer for all chars read	   */
       line[MAX_TOK_SZ];		/* buffer for chars read on a line */
  int numchars = MAXCHARS,		/* number of chars read from file  */
      i = 0,				/* index for the character buffer  */
      j = 0;				/* index for the line buffer       */
  long hbeat_val = -1;


  /* ignore errors from the open() because the file does not have to exist */
  if ((parsefd = open(CONFIG_FILE, O_RDONLY)) < 0)
  {
    hbeat_val = -1;
  }

  /* initialize the line buffer to a zero-length string */
  line[0] = '\0';

  /* keep reading until file is all read out */
  while (numchars == MAXCHARS)
  {
    numchars = read(parsefd, buf, MAXCHARS);
    #ifdef NETLS_DEBUG
    printf("\n\nnumchars read = %d\n\n", numchars);
    #endif

    /* pack each line into a string which can be picked apart and compared */
    /* against known and expected values.				   */
    for (i=0; i < numchars; i++)
    {
      while( (buf[i] == ' ') || (buf[i] == '\t') )
        i++;
      buf[i] = toupper(buf[i]);
      if ( (buf[i] == '\n') || (i == numchars) )   /* a complete line has  */
      {						   /* been read            */
        line[j] = '\0';
        #ifdef NETLS_DEBUG
        printf("line = <%s>\n", line);
        #endif

	/* check to see if the first part of the line read matches the     */
        /* expected token.						   */
        if (strncmp(line,KEYWORD,KEYWORDLEN) == 0)
        {
          /* The next character following the keyword should begin the     */
          /* heartbeat value.                                              */
          hbeat_val = atoi(&line[KEYWORDLEN]);

          /* The atoi() function returns '0' for both invalid numbers, and */
          /* for the character zero.  For this reason, use a token to      */
          /* identify what would normally be recognized as zero.           */
          if (hbeat_val == 0)
            if (strcmp(&line[10], MAX_KEYWORD) == 0)
              hbeat_val = 0;
            else
              hbeat_val = -1;
          #ifdef NETLS_DEBUG
          printf("HEARTBEAT= found\n");
          printf("remaining: <%s>\n", (char *) &line[10]);
          printf("heartbeat = %d\n", hbeat_val);
          #endif
          break;
        }
        else		    /* The line read was invalid. Reset for reading */
        {                   /* the next line in the buffer.                 */
          j = 0;
        }
      }
      else		    /* copy the character read into the line buffer */
      {
        line[j] = buf[i];
        j++;
        if (j == MAX_TOK_SZ)   /* line contains more chars than the largest */
          j = 0;	       /* expected value.  Recycle buffer space.    */
      }
    }
  }

  #ifdef NETLS_DEBUG
  printf("returning saved hbeat value: %d\n", hbeat_val);
  #endif

  return(hbeat_val);
}

