static char sccsid[] = "@(#)11	1.17  src/bos/usr/sbin/init/getcmd.c, cmdoper, bos411, 9428A410j 4/28/94 09:17:25";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */


/*
 * FUNCTION: /etc/inittab file parser for /etc/init
 */

#include	<sys/types.h>
#include	<sys/param.h> /* included for NPROC & NOFILE */
#include	<signal.h>
#include	<utmp.h>
#include	<errno.h>
#include	<termio.h>
#include	<ctype.h>
#include	<sys/dir.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include 	<pwd.h>
#include	<sys/inode.h>
#include	<sys/user.h>
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	<sys/NLchar.h>
#ifdef TOTLOGIN
#include	<sys/ipc.h>
#include	<sys/sem.h>
#endif TOTLOGIN
#ifdef CSECURITY
#include	<sys/audit.h>
#endif	CSECURITY
#include	<stdio.h>

#include	"init_msg.h"
#include	"init.h"


/**********************/
/****    getcmd    ****/
/**********************/

/*	"getcmd" parses lines from /etc/inittab.  Each time it finds	*/
/*	a command line it will return TRUE as well as fill the passed	*/
/*	CMD_LINE structure and the shell command string.  When the end	*/
/*	of /etc/inittab is reached, FALSE is returned.			*/
/*									*/
/*	/etc/inittab is automatically opened if it is not currently	*/
/*	open and is closed when the end of the file is reached.		*/

static FILE *fp_inittab = NULL;
static int run_brc = 0;

getcmd(cmd,shcmd)
register struct CMD_LINE *cmd;
char *shcmd;
{
  extern FILE *fp_inittab;
  int i, answer, proceed, errnum;
  register char *ptr;
  register int c;
  register int state;
  char lastc,*ptr1;
  extern int errno;
  static char *actions[] = { 
    "off", "hold", "respawn","ondemand","once","wait","boot",
    "bootwait","powerfail","powerwait","initdefault",
    "sysinit",
  };
  static short act_masks[] = 
  {
    M_OFF, M_HOLD, M_RESPAWN,M_ONDEMAND,M_ONCE,M_WAIT,M_BOOT,M_BOOTWAIT,
    M_PF,M_PWAIT,M_INITDEFAULT,M_SYSINIT,
  };
  struct stat filestatus;

#ifdef	XDEBUG
  debug("We have entered getcmd().\n");
#endif
  if (fp_inittab == NULL)
  {
    /* Be very persistent in trying to open /etc/inittab.   */
    for (i=0; i < 3;i++)
    {
      if ((fp_inittab = fopen(INITTAB,"r")) != NULL) 
	break;
      else 
      {
        errnum = errno;	/* Remember for error message */
        timer(3);	/* Wait 3 seconds to see if file appears. */
      }
    }
   
    /*
    ** find out if we have a zero length /etc/inittab.
    */
    if (fp_inittab) {
       stat(INITTAB,&filestatus);
       if (filestatus.st_size==0) {
          fclose(fp_inittab);
          fp_inittab = NULL;
       }
    }

    /* If unable to open /etc/inittab, print error message and return */
    /* FALSE to caller. */
    /* In addition, run /sbin/rc.boot to enable the console.  P26716 */
    if (fp_inittab == NULL)
    {
      console(LOG, M_OPEN, "Cannot open %s  errno: %d\n", INITTAB, errnum);

      if (!run_brc) {
        run_brc = 1;
        etcbrc();
      }

      return (FAILURE);
    }
  }

  /*
  **  At this point /etc/inittab exists and has a nonzero length
  */
  run_brc = 1;     /*  never run etcbrc() if /etc/inittab exists  */

  /* Keep getting commands from /etc/inittab until you find a good */
  /* one or run out of file. */
  for (answer= FALSE; answer == FALSE;)
  {
    /* Zero out the cmd itself before trying next line. */
    memset((char *)cmd,NULL,sizeof(struct CMD_LINE));

    /* Read in lines of /etc/inittab, parsing at colons, until a line */
    /* is read in which doesn't end with a backslash.  Do not start */
    /* if the first character read is an EOF.  Note that this means */
    /* that should a line fail to end in a newline, it will still */
    /* be processed, since the "for" will terminate normally once */
    /* started, regardless of whether line terminates with a newline */
    /* or an EOF. */
    state = FAILURE;
    if ((c = fgetc(fp_inittab)) != EOF) 
      for (proceed = TRUE, ptr = shcmd, state = ID, lastc = '\0'; 
           proceed && c != EOF; 
           lastc = c , c = fgetc(fp_inittab))
      {

        /* If we are not in the FAILURE state and haven't yet reached */
        /* the shell command field, process the line, otherwise just */
        /* look for a real end of line. */
        if (state != FAILURE && state != COMMAND)
        {

          /* Squeeze out spaces and tabs. */
          if (c == ' ' || c == '\t') continue;

          /* If the character is a ':', then check the previous field for */
          /* correctness and advance to the next field. */
          if ( c == ':' )
          {
            switch (state)
            {

            /* Check to see that there are up to IDENT_LEN characters for id. */
              case ID :
                if ((i = ptr - shcmd) < 1 || i > IDENT_LEN )
                {
                  state = FAILURE;
                } 
                else 
                {
                  memcpy(&cmd->c_id[0],shcmd,i);
                  ptr = shcmd;	/* Reset pointer */
                  state = LEVELS;
                }
                break;

              case LEVELS :

              /* Build a mask for all the levels that this command will be */
              /* legal */
                for (cmd->c_levels= 0,ptr1= shcmd ; ptr1 < ptr; ptr1++)
                {
                  if (*ptr1 >= '0' && *ptr1 <= '9')
                    cmd->c_levels |= (MASK0 << (*ptr1 - '0'));
                  else if (*ptr1 >= 'a' && *ptr1 <= 'c')
                    cmd->c_levels |= (MASKa << (*ptr1 - 'a'));
                  else if(*ptr1 == 's' || *ptr1 == 'S')
                    cmd->c_levels |= MASKSU;
                  else if(*ptr1 == 'm' || *ptr1 == 'M')
                    cmd->c_levels |= MASKSU;
                  else 
                  {
               	    state = FAILURE;
               	    break;
                  }
                }
                if (state != FAILURE)
                {
                  state = ACTION;
                  ptr = shcmd;	/* Reset the buffer */
                }
                break;

              case ACTION :

            /* Null terminate string in shcmd buffer and then try to match */
            /* against legal actions.  If the field is of length 0, then the */
            /* default of "RESPAWN" is used if the id is numeric, otherwise */
            /* the default is "OFF". */
                if (ptr == shcmd)
                {
                  if (isdigit(cmd->c_id[0])
                      && (cmd->c_id[1] == '\0' || isdigit(cmd->c_id[1]))
                      && (cmd->c_id[2] == '\0' || isdigit(cmd->c_id[2]))
                      && (cmd->c_id[3] == '\0' || isdigit(cmd->c_id[3]))
                      && (cmd->c_id[4] == '\0' || isdigit(cmd->c_id[4]))
                      && (cmd->c_id[5] == '\0' || isdigit(cmd->c_id[5]))
                      && (cmd->c_id[6] == '\0' || isdigit(cmd->c_id[6]))
                      && (cmd->c_id[7] == '\0' || isdigit(cmd->c_id[7]))
                      && (cmd->c_id[8] == '\0' || isdigit(cmd->c_id[8]))
                      && (cmd->c_id[9] == '\0' || isdigit(cmd->c_id[9]))
                      && (cmd->c_id[10] == '\0' || isdigit(cmd->c_id[10]))
                      && (cmd->c_id[11] == '\0' || isdigit(cmd->c_id[11]))
                      && (cmd->c_id[12] == '\0' || isdigit(cmd->c_id[12]))
                      && (cmd->c_id[13] == '\0' || isdigit(cmd->c_id[13])) )
                  	cmd->c_action = M_RESPAWN;
                  else cmd->c_action = M_OFF;
                } 
                else 
                {
                  for (cmd->c_action=0,i= 0,*ptr = '\0'; 
                       i < sizeof(actions)/sizeof(char *);i++)
                  {
                    if (strcmp(shcmd,actions[i]) == 0)
                    {
                      if((cmd->c_levels & MASKSU) && 
                         !(act_masks[i] & (M_INITDEFAULT | M_PF | M_PWAIT)))
                        cmd->c_action = 0;
                      else cmd->c_action = act_masks[i];
                      break;
                    }
                  }
                }

            /* If the action didn't match any legal action, set state to */
            /* FAILURE. */
                if (cmd->c_action == 0) state = FAILURE;
                else 
                {
                  state = COMMAND;

              /* Insert the prefix string of "exec " into the command buffer */
              /* before inserting any characters. */
                  strcpy(shcmd,"exec ");
                }
                ptr = shcmd + EXEC;
                break;
            } /* switch(state) */
              continue;
          } /* end if(c == :) */
        } /* end if (state != FAILURE) */

      /* If the character is a '\n', then this is the end of a line. */
      /* If the '\n' wasn't preceded by a backslash, it is also the end */
      /* of an /etc/inittab command.  If it was preceded by a backslash */
      /* then the next line is a continuation.  Note that the */
      /* continuation '\n' falls through and is treated like other */
      /* characters and is stored in the shell command line. */
        if (c == '\n')
          if (lastc != '\\')
          {
            proceed = FALSE;
            *ptr = '\0';
            break;
          }

        /* For all other characters just stuff them into the command */
        /* as long as there aren't too many of them. Make sure there is */
        /* room for a terminating '\0' also. */
        if (ptr >= (shcmd + MAXCMDL-1))
          state = FAILURE;
        else
          *ptr++ = c;

        /* If the character we just stored was a quoted backslash, then */
        /* change "c" to '\0', so that this backslash will not cause a */
        /* subsequent '\n' to appear quoted.  In otherwords '\' '\' '\n' */
        /* is the real end of a command, while '\' '\n' is a continuation. */
        if ( c == '\\' && lastc == '\\')
          c = '\0';
      } /* end for(proceed == TRUE...) */

  /* Make sure all the fields are properly specified for a good */
  /* command line. */
    if (state == COMMAND)
    {
      answer = TRUE;
      cmd->c_command = shcmd;

      /* If no default level was supplied, insert all numerical levels. */
      if (cmd->c_levels == 0)
        cmd->c_levels = MASK0 | MASK1 | MASK2 |  MASK3 | MASK4 | MASK5 | MASK6 | MASK7 | MASK8 | MASK9;

      /* If no action has been supplied, declare this entry to be OFF. */
      if (cmd->c_action == 0) 
        cmd->c_action = M_OFF;

        /* If no shell command has been supplied, make sure there is */
        /* a null string in the command field. */
        /* EXEC is the length of the string "exec " minus null at the end. */
      if (ptr == (shcmd + EXEC))
        *shcmd = '\0';
    }
    else 
      answer = FALSE;

    /* If we have reached the end of /etc/inittab, then close it and */
    /* quit trying to find a good command line. */
    if (c == EOF)
    {
      fclose(fp_inittab);
      fp_inittab = NULL;
      return(FALSE);
    }
  } /* end for(answer = FALSE...) */
  return(answer);
}


/*
**   If no /etc/inittab file could be found, then execute /sbin/rc.boot 3.
*/
etcbrc()
{
   int pid, rc;

   if ((pid=fork()) == 0) {  /*  this execl to be considered magic  */
      execl("/bin/sh", "INITSH", "-c", "exec /sbin/rc.boot 3 > /dev/console 2>&1", 0);
      console(NOLOG, M_BRCEXEC,"etcbrc: could not exec");
   }
   else if (pid > 0)
      wait((int *) 0);
   else
      console(NOLOG, M_BRCFORK,"etcbrc: could not fork");
}
