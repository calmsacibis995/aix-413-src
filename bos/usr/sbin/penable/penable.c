static char sccsid[] = "@(#)25  1.33  src/bos/usr/sbin/penable/penable.c, cmdoper, bos41J, 9524E_all 6/13/95 16:15:38";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: penable, pdisable, pstart, phold, pshare, pdelay
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
     The action line in the inittab file will change to 
     action=respawn if penable, pshare, or pdelay are used, and to
     action=off if pdisable or phold is used. If pdisable is used it will
     kill whatever process is on that port after setting to off.
     If pshare is used then the command line 'command="getty /dev/name"' will
     change to 'command="getty -u /dev/name' and if pdelay is used it will
     change to 'command="getty -r /dev/name"'.

     Note that this code was rewritten from code that assumed /etc/inittab
     was a stanza file like /etc/ports.  Therefore, some variable names
     reflect that history.
*/

#include <odmi.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <IN/standard.h>
#include <IN/CSdefs.h>
#include <IN/ERdefs.h>
#include <IN/AFdefs.h>
#include <utmp.h>
#include <locale.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/flock.h>
#include <sys/stat.h>
#include "penable.h"   /* defines and structures for penable */
extern struct utmp *getutent();
extern void setutent();
FILE *fopen_with_lock(char *filename);

int noprewht(char *s,char e);
int updstan(char rq);
int updcmd(char rq);
bool justquery = FALSE;

#include <nl_types.h>
#include "penable_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_PENABLE,Num,Str)


main(int argc,char **argv)
{
  char *p;    				/* pointer to device name */
  bool mustsignal;  			/* must signal init */
  int rtnval = 0;    			/* return value */
  int pstatus;    			/* status of current device */
  struct devlist *pv;  			/* malloc list of devices */
  char requeststat;
  char *lines[MAXGETTYS+1];		/* array of devices which have gettys */
  int ilines = 0;			/* index into this array */


  (void) setlocale(LC_ALL, "");
  catd = catopen(MF_PENABLE, NL_CAT_LOCALE);

  					/* get the action requested */
  if ((request = getname(argv[0])) == BAD)
  { 
    fprintf(stderr, MSGSTR(M_CMDNAME,"Unknown command name %s.\n"),argv[0]);
    exit(-1);
  } /* end if */

  					/* is it just a status command? */
  if (argc == 1)
  {
    justquery = TRUE;
    if ((fpinit = fopen(itfn, "r")) == (FILE *)NULL)    /* open inittab */
    {
      fprintf(stderr, MSGSTR(M_OPEN,"Cannot open %s\n"),itfn);
      exit(-1);
    } /* end if */
    while ((pstatus = getnxpor()) != BAD)   	/* read and get ports */
    {
      if((request == START) && (pstatus > 0))   /* pstart */
      {
        noprewht(linebuf,':'); 	/* output the name of the current line */
        switch (pstatus) {	/* what is the status of current port */
        case ENABLE:
          fprintf(stdout, MSGSTR(M_ENABLE," ENABLE\n"));
          break;
        case SHARE:
          fprintf(stdout, MSGSTR(M_SHARE," SHARE\n"));
          break;
        case DELAY:
          fprintf(stdout, MSGSTR(M_DELAY," DELAY\n"));
          break;
        case HOLD:
          fprintf(stdout, MSGSTR(M_HOLD," HOLD\n"));
          break;
        case DISABLE:
          fprintf(stdout, MSGSTR(M_DISABLE," DISABLE\n"));
          break;
	case UNDEF:
	  fprintf(stdout, MSGSTR(M_UNDEF," UNDEFINED\n"));
	  break;
	case UNAVAIL:
	  fprintf(stdout, MSGSTR(M_UNAVAIL," UNAVAILABLE\n"));
	  break;
        default:
          fprintf(stdout, MSGSTR(M_UNKNOWN," UNKNOWN\n"));
          break;
        } /* end switch */
      } /* end if */
      else if (pstatus == request) 
      { 				/* port status matches the desired */
        noprewht(linebuf,':');  	/* status, then print the name */
        fprintf(stdout,"\n"); 		/* need a newline */
      } /* end else if */
    } /* end while */
    fclose(fpinit); 			/* close inittab */
    exit(0);
  } /* end if argc == 1 */

  p = ttyname(0); 			/* get name of this port */
  if (p && *p)
    strcpy(mytty, undev(p)); 		/* copy just name (no path) to mytty */
  {
    register int i;
    register char *q;

    for (i = 1; i < argc; i++)   	/* loop through the arguments */
    {
      p = argv[i];    			/* get name */
      if (*p == '-')   			/* is it a flag */
      {
        switch (*(p+1)) { 		/* what flag is it? */
        case 'a':  			/* do request on all the ports */
          allflag = TRUE;
          break;
        case 'i':  		/* NOP */
          break;
        default:
          fprintf(stderr, MSGSTR(M_INVALARG,"Invalid argument.\n"));
          break;
        } /* end switch */
      } /* end if *p == - */
      else 
      { 		/* else add the name of the port */
        if((pv = adddev(p, (short) FOUND)) == (struct devlist *)NULL)
          rtnval = 2;
      } /* end else */
    } /* end for */
  } /* end block */

  {
    register struct devlist *d;
    register int avail;
    char devname[MAXNAMLEN]; 		/* hold the device name */

    if ((fpinit = fopen_with_lock(itfn)) == (FILE *)NULL)  	 /* open inittab */
    {
      fprintf(stderr, MSGSTR(M_OPEN,"Cannot open %s\n"),itfn);
      exit(-1);
    } /* end if */

    mustsignal = (bool)FALSE;

    if(allflag == TRUE) 
    { 						/* read and get ports */
      while((pstatus = getnxpor()) != BAD) 
      {
		/* don't change unless request is for disable or hold*/
	if ((request != DISABLE && request != HOLD ) && 
		(pstatus == UNAVAIL || pstatus == UNDEF)) {
		continue;
	}
        stanname(linebuf,devname); 		/* get device name */
        if(strcmp("console", devname) && strcmp(mytty, devname)) 
        {
          if(request == START) 
          {
	    switch((requeststat = get_request(devname)))
	    {
              case ENABLE:
              case SHARE:
              case DELAY:
		break;
	      case BAD:
              	fprintf(stderr,MSGSTR(M_BADREQ,"Bad request for %s\n"),devname);
	      	exit(1);
              case HOLD:
              case DISABLE:
	      default:
		continue;
	    } /* end switch */
          } /* end if */
	  else 
	    requeststat =request;
	  if (lines[ilines] = gettyline(requeststat,devname))
	    if (ilines < MAXGETTYS)
		ilines++;
          updstan(requeststat);      /* update buffer for this request */ 

          if (!updinittab())			/* update inittab */
            fprintf(stderr,MSGSTR(M_UPDINIT,"Couldn't update inittab for %s\n"),
	      devname);
          mustsignal = (bool)TRUE;
        } /* end if */
      } /* end while */
    } /* end if allflag == TRUE */
    else 
    {
      avail = 1;         	 /* in case ports is empty read and get ports */
      while((pstatus = getnxpor()) != BAD) 
      {
        stanname(linebuf,devname); 		/* get device name */
        avail = 0;
        for (d = fstdev; d != NULL; d = d->dl_next) 
        {
          if(d->dl_slct == FOUND) 
          {
            ++avail;
            if(strcmp(undev(d->dl_name),devname) == 0) 
            {
              if(request == START) 
	      {
	        switch((requeststat = get_request(d->dl_name)))
	        {
                  case ENABLE:
                  case SHARE:
                  case DELAY:
		    break;
	          case BAD:
              	    fprintf(stderr,MSGSTR(M_BADREQ,"Bad request for %s\n"),
			devname);
	      	    exit(1);
                  case DISABLE:
                  case HOLD:
	          default:
		    continue;
	        } /* end switch */
              }	/* end if */
	      else
	        requeststat =request;
	      if (lines[ilines] = gettyline(requeststat,devname))
		if (ilines < MAXGETTYS)
		  ilines++;
              updstan(requeststat); 		/* update buffer */
              if (!updinittab())		/* update inittab */
                fprintf(stderr,MSGSTR(M_UPDINIT,
		  "Couldn't update inittab for %s\n"), devname);
              mustsignal = (bool)TRUE;
              d->dl_slct = UNSELECTED;
              --avail;
            } /* end if */
          } /* end if d->dl_slct == FOUND */
        } /* end for */
        if(avail == 0)
          break;
      } /* end while */
    } /* end else */
    /* If updinittab() was called above, this close will cause 
       the old "hidden" copy of inittab to really be unlinked. */
    fclose(fpinit); 
  } /* end block */

  if (mustsignal == (bool)TRUE) {
    /* kill all gettys */
    while (ilines--)
	killgetty(lines[ilines]);

    kill(1,S_enable);   		/* send signal to init */
  }
  else
    fprintf(stderr, MSGSTR(M_NOACT,"No action taken.\n"));
  return(rtnval);
} /* end main */

struct devlist *adddev(char *d,short select)
{
  register struct devlist *de;
  struct stat statbuf;


  if((de = (struct devlist *)malloc((size_t)sizeof(struct devlist))) == NULL) 
  {
    fprintf(stderr, MSGSTR(M_NOMEM,"Cannot malloc memory.\n"));
    exit(1);
  } /* end if */
  if(d[0] == '/')
    strncpy(de->dl_name,d,(size_t)((sizeof de->dl_name) - 1));
  else if (isalpha(d[0]) != 0)
  { 				/* used "ttyXX" or "cons" format ? */
    strcpy(de->dl_name, devpfx);
    if (!strncmp(d, "cons", (size_t)4))
      strcat(de->dl_name, "console");
    else
      strcat(de->dl_name, d);
  } /* end else if */
  else
  { 					/* assume used "XX" format */
      strcpy(de->dl_name, devpfx);
      strcat(de->dl_name, ttypfx);
      strcat(de->dl_name, d);
  } /* end else */
  if(stat(de->dl_name,&statbuf) < 0) 
  {
    fprintf(stderr, MSGSTR(M_STAT,"Cannot stat %s.\n"),de->dl_name);
    free((void *)de);
    return(NULL);
  } /* end if */
  if (portavail(de->dl_name) != AVAIL) {
    fprintf(stderr, MSGSTR(M_AVAIL,"Device %s not in available state.\n"),
		    de->dl_name);
    free((void *)de);
    return(NULL);
  }

  de->dl_slct = select;
  if(request != START && request != ENABLE && request != SHARE && 
	request != DELAY && (strcmp(de->dl_name,"/dev/console") == 0))
    de->dl_slct = UNSELECTED;
  de->dl_next = NULL;
  if(fstdev == NULL)
    fstdev = de;
  else
    lstdev->dl_next = de;
  lstdev = de;

  return(de);
} /* end adddev */

/*
 * This routine is called to strip off the "/dev/" on a device name.
 */
char *undev(char *s)
{
  register char *p;
  char dev[] = "/dev/";
  short i=0;

  if (s == NULL || s[0] == '\0')
    return(s);
  
  if ((strncmp(dev,s,5)) == 0)
    p = &s[5];
  else
    p = &s[0];
  return(p);                		/* return start of device name */
} /* end undev */

/* routine to return the action for which the program was invoked */
int getname(char *p)  		/* program name */
{
  p = (char *)basename(p);
  if(strcmp(p, "pstart") == 0) return(START);
  if(strcmp(p,"pdisable") == 0) return(DISABLE);
  if(strcmp(p,"penable") == 0) return(ENABLE);
  if(strcmp(p,"pshare") == 0) return(SHARE);
  if(strcmp(p,"pdelay") == 0) return(DELAY);
  if(strcmp(p,"phold") == 0) return(HOLD);
  return(BAD); 				/* return with error */
} /* end getname */

/* Output string without preceeding white space until end character is hit. */
int noprewht(char *s,char e) 
{
  while(*s == ' ' || *s == '\t')
    s++;
  if (!strncmp(s, "cons", 4))		/* Special case "cons" entry. */
    fputs("console", stdout);
  else
    while(*s != e)fputc((int)*s++,stdout);
  return(1);
} /* end noprewht */

int getnxpor() 	/* routine to get the status of the next port in inittab */
{
  if(getget() != BAD) 	/* get the next line with a getty command in it */
    return(getpstat()); 	/* get and return the status of the port */
  else
    return(BAD); 		/* end of file */
} /* end getnxpor */

 /* routine to get a line with "*getty *".  It is assumed to be a port */
 /* lines beginning with a colon are ignored, as they are by init */
int getget()
{
  char  *s;

  while (fgets(linebuf, MAXLLEN, fpinit)) 
  { 					/* skip leading white space */
    for (s = linebuf; *s == ' ' || *s == '\t'; s++);
    if(*s != ':' && isgetty(s) != BAD)
      return(TRUE); 			/* getty command line found */
  } /* end while */
  	/* So that it doesn't try to use the last line in the file. */
  linebuf[0] = '\0';
  return(BAD); 				/* EOF and no = "*getty*" */
} /* end getget */

/* routine is see if the word getty is in this line */
int isgetty(char *s) 
{
  int i;  /* D15699 */

  /* move to the fourth column which is the command */
  for (i=1; i < 4 && s != NULL; s=strchr(s,':'), i++, s++);
  if (s == NULL) return(BAD);

  while(1) 
  {
    while(*s != '/') 			  /* find the / in /etc/getty */
    {
      if(*s == '\n')return(BAD); 		/* no getty here */
      if(*s == '\0')return(BAD); 		/* no getty here */
      if(*s == '#')return(BAD);                 /* comments have started */
      s++;
    } /* end while */
    if(strncmp(GETTY, s, (size_t)strlen(GETTY)) == 0) 	/* we have getty */
      return(TRUE);
    s++; 					/* skip this g */
  } /* end while */
} /* end isgetty */

 /* routine to get port status of current line */
int getpstat()
{
  char devname[MAXNAMLEN]; 		/* hold the device name */
  char act[MAXACT];
  char cmd[MAXCMDL];
  int rc;

  stanname(linebuf, devname); 		/* store the device name */
  if ((allflag == TRUE) && (rc = portavail(devname)) != AVAIL) {
    return(rc);
  }
  if (!getact(linebuf, act))
    return(BAD); 			/* can't know status without action */
  if (justquery) {
    if (strncmp("off",act,(size_t)3) == 0)
      return(DISABLE);
    if (strncmp("hold",act,(size_t)4) == 0)
      return(HOLD);
  }
  if(strncmp("off",act,(size_t)3) == 0 || strncmp("hold",act,(size_t)4) == 0) 
  {
    if(isportact(devname))	/* port has process then must be on hold */
      return(HOLD); 
    else
      return(DISABLE);  		/* no process then must be disabled */
  } /* end if */
			  /* only respawn or ondemand means it is enabled */
  if ((strncmp("respawn",act,(size_t)7) == 0) || 
				(strncmp("ondemand",act,(size_t)8) == 0)) 
  {
    if (!getcmd(linebuf, cmd))  	/* need command line to know if */
      return(BAD);         		/* port shared, delayed, enabled */

				    	/* if -u is used then it is shared */
				    	/* if -r is used then it is delayed */
    					/* else enabled */
    return((int)whatflg(cmd));
  } /* end if */
  else 					/* anything else is not correct */
    return(BAD);
} /* end getpstat */

/* Get command specified in from */
int getcmd(char *from, char *to)
{
  char  *s = from;
  int  i;
 							 /* skip over name */
  for (; *s != ':' && *s != '\n' && *s != '\0'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  							/* skip over runlevel */
  for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  							/* skip over action */
  for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  						/* skip leading white space */
  for (s++; *s == ' ' || *s == '\t'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  						/* copy command to "to" */
  for (i = 0; *s != '\n' && *s != '\0'; s++, i++)
    to[i] = *s;
  to[i] = '\0';
  return(i);
} /* end getcmd */

/* Get action (respawn, off, etc.) specified in from */
int getact(char *from, char *to)
{
  char  *s = from;
  int  i;
  							/* skip over name */
  for (; *s != ':' && *s != '\n' && *s != '\0'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  							/* skip over runlevel */
  for (s++; *s != ':' && *s != '\n' && *s != '\0'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  						/* skip leading white space */
  for (s++; *s == ' ' || *s == '\t'; s++);
  if (*s == '\n' || *s == '\0')
    return(0);
  						/* copy action to "to" */
  for (i = 0; *s != ':' && *s != ' ' && *s != '\t' && *s != '\n' && 
      *s != '\0'; s++, i++)
    to[i] = *s;
  to[i] = '\0';
  return(i);
} /* end getact */

 /* routine to get device name and store */
 /* sb  pointer to the buffer */
 /* s  destination */
int stanname(char *sb,char *s)
{
  register char *t = s;

  *t = '\0';
  while (*sb != ':') {
    switch (*sb) {
    case '\n':
    case '\0':
      return(BAD);
    case ' ':
    case '\t':
      sb++;
      break;
    default:
      *t++ = *sb++;
    }
  }
  *t = '\0';
  if (!(strncmp(s, "cons", (size_t)4)))
    strcpy(s, "console");
  return(TRUE);
} /* end stanname */


 /* returns the name of the port getty is running on */
 /* rq		the request */
 /* devname	the device name */
char *gettyline(char rq, char *devname)
{
  /* return NULL if no getty needs to be killed,
   * otherwise returns a pointer to malloc'd space containing the name
   * of the line where the getty is running.
   */

  char action[MAXACT];

  switch(rq) {
  case ENABLE:
  case SHARE:
  case DELAY:
  case HOLD:
    /* for enable, share, delay and hold we kill the getty
     * if it isn't already off or on hold.
     */
    if (getact(linebuf,action) && strcmp(action, "off") != 0
    && strcmp(action, "hold") != 0) {
	char *l = malloc(strlen(devname)+1);
	strcpy(l, devname);
	return(l);
    }
  }
  return(NULL);
}

 /* sends SIGTERM to all login processes running on the port */
 /* devname	the device name */
void killgetty(char *devname)
{
  if (!isportact(devname)) {
    struct utmp *b;

    setutent();
    while((b = getutent()) != NULL)
      if (strcmp(devname, b->ut_line) == 0)
	if (b->ut_type == LOGIN_PROCESS)
	    kill(b->ut_pid, SIGTERM);
  }
}

 /* update current file with this request */
 /* rq  the request */
int updstan(char rq)
{
  char devname[MAXNAMLEN]; 		/* name of port to update */

  switch(rq) { 				/* what is the request */
  case DISABLE:
    stanname(linebuf,devname); 		/* get the port name */
    updact("off"); 			/* change action line in stanbuf */
    break;
  case HOLD:
    updact("hold"); 			/* change action line in stanbuf */
    break;
  case START:
    updact("respawn"); 			/* change action line in stanbuf */
    break;
  case ENABLE:
  case SHARE:
  case DELAY:
    updact("respawn"); 			/* change action line in stanbuf */
    updcmd(rq); 		/* update command line based on the request */
    break;
  default:
    return(BAD);
  } /* end switch */
  return(TRUE);
} /* end updstan */

/* get the current flag in the getty command */
char whatflg(char *s) 
{
  while(*s != '\n' && *s != '\0')  	 /* go to the end of the line */
  {
    if(*s++ == '-')   				/* we have a flag */
    {
      if (*s == 'u')
        return(SHARE); 				/* found a shared flag */
      if (*s == 'r')
        return(DELAY); 				/* found a delay flag */
    } /* end if */
  } /* end while */
  return(ENABLE); 		/* no shared or delay so must be enable */
} /* end whatflg */

 /* update the action with new action */
int updact(char *s)
{
  char newbuf[MAXLLEN]; 		/* Hold new stanza information */
  char *oldp = linebuf; 		/* pointer to the old buffer */
  char *newp = newbuf; 			/* pointer in the new buffer */
 					 /* copy over name */
  for (; *oldp != ':' && *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);
  					/* copy over runlevel */
  for (*newp++ = *oldp++; *oldp != ':' && *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);
  						/* copy in ':' */
  *newp++ = *oldp++;
						  /* copy in new action */
  for (; *s != ':' && *s != '\n' && *s != '\0';)
    *newp++ = *s++;
						  /* skip over old action */
  for (; *oldp != ':' && *oldp != '\n' && *oldp != '\0'; oldp++);
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);
  					/* copy over rest of old line */
  for (*newp++ = *oldp++; *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;

  *newp++ = '\n'; 					/* add a newline */
  *newp = '\0';
  strcpy(linebuf, newbuf); 	/* put new buffer into the main buffer */
  return(TRUE);
} /* end updact */

 /* update the getty line without the flag  */
char *noflg(char *nb,char *ob,char c)
{
  while(*ob != '\n') 
  {
    if(*ob == '-')   				/* we have a flag */
    {
      if(*(ob + 1) == c)   			/* right flag to strip */
      {
        ob += 2; 				/* skip the flag */
        while (*ob == ' ' || *ob == '\t')
          ob++;
      } /* end if */
      else 
        *nb++ = *ob++; 				/* move the minus */
    } /* end if */
    else 
      *nb++ = *ob++; 				/* move in the character */
  } /* end while */
  *nb++ = *ob++; 			/* add a newline in the new buffer */
  return(nb);
} /* end noflg */

 /* change the getty line's flag  */
char *chgflg(char *nb,char *ob,char cfrom,char cto)
{
  while((*nb++ = *ob++) != '\n') 
  {
    if(*ob == '-') 
    {
      if(*(ob + 1) == cfrom)   			/* right flag to change */
      {
        *nb++ = *ob++; 					/* copy the '-' */
        *nb++ = cto; 				/* put in the new flag */
        ob++; 					/* skip the old flag */
      } /* end if */
    } /* end if */
  } /* end while */
  return(nb);
} /* end chgflg */

 /* add flag to the getty line */
char *addflg(char *nb,char *ob,char c)
{
  while(*ob != '\n') 
  {
    if(strncmp(ob,GETTY,(size_t)strlen(GETTY)) == 0)  	/* found getty */
    {
      while((*nb++ = *ob++) != ' '); 		/* move the word getty */
      *nb++ = '-'; 				/* add first part of flag */
      *nb++ = c; 				/* add the flag */
      *nb++ = ' '; 				/* add space */
    } /* end if */
    else *nb++ = *ob++;
  } /* end while */
  *nb++ = *ob++; 			/* add a newline in the new buffer */
  return(nb);
} /* end addflg */

 /* no changes but move the getty line */
char *nochg(char *nb,char *ob)
{
  while((*nb++ = *ob++) != '\n');
  return(nb);
} /* end nochg */

 /* update the command line with the new assignment in linebuf */
int updcmd(char rq)
{
  char curflg; 				/* hold status of current getty */
  char newbuf[MAXLLEN]; 		/* Hold new stanza information */
  char *oldp = linebuf; 		/* pointer to the old buffer */
  char *newp = newbuf; 			/* pointer in the new buffer */
  					/* copy over name */
  for (; *oldp != ':' && *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);
  					/* copy over runlevel */
  for (*newp++ = *oldp++; *oldp != ':' && *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);

  					/* copy over action */
  for (*newp++ = *oldp++; *oldp != ':' && *oldp != '\n' && *oldp != '\0';)
    *newp++ = *oldp++;
  if (*oldp == '\n' || *oldp == '\0')
    return(BAD);

  					/* copy in ':' */
  *newp++ = *oldp++;

  					/* skip leading white space */
  while ((*oldp == ' ') || (*oldp == '\t'))
    oldp++; 

  curflg = whatflg(oldp); 		/* get the current flag in this getty */

  switch(rq) {
  case ENABLE:
    switch(curflg) { 		/* check on current flag to know what to do */
      case SHARE:
        newp = noflg(newp,oldp,'u'); 		/* strip u flag */
        break;
      case DELAY:
        newp = noflg(newp,oldp,'r'); 		/* strip r flag */
        break;
      default:
        newp = nochg(newp,oldp); 		/* copy rest of line */
        break;
    } /* end switch */
    break;
  case SHARE:
    switch(curflg) {
      case ENABLE:
        newp = addflg(newp,oldp,'u'); 		/* add -u */
        break;
      case DELAY: 			/* change from -r to the -u flag */
        newp = chgflg(newp,oldp,'r','u'); 
        break;
      default:
        newp = nochg(newp,oldp);
        break;
    } /* end switch */
    break;
  case DELAY:
    switch(curflg) {
      case ENABLE:
        newp = addflg(newp,oldp,'r');
        break;
      case SHARE:
        newp = chgflg(newp,oldp,'u','r');
        break;
      default:
        newp = nochg(newp,oldp);
        break;
    } /* end switch */
    break;
  } /* end switch */
  *newp = '\0';
  strcpy(linebuf,newbuf);
  return(TRUE);
} /* end updcmd */

/* update inittab file */
int updinittab()
{
  int  fd;
  int  i;
  static int savefd = -1;
  char  itfn2buf[ITFN2SZ];
  char  curbuf[MAXLLEN];
  char  devname[MAXNAMLEN];
  char  curdev[MAXNAMLEN];
  FILE  *fpinit2;
  						/* create temp file */
  strcpy(itfn2buf, itfn2);
  if ((fd = mkstemp(itfn2buf)) <= 0) 
  {
    fprintf(stderr, MSGSTR(M_TMPFILE,"Unable to create temp file.\n"));
    return(0);
  } /* end if */
 						 /* open inittab */
  if ((fpinit2 = fopen_with_lock(itfn)) == NULL)  
  { 
    fprintf(stderr, MSGSTR(M_OPEN,"Cannot open %s\n"), itfn);
    exit(-1);
  } /* end if */
  				/* Update temp file with new information */
  stanname(linebuf, devname);
  while (fgets(curbuf, MAXLLEN, fpinit2)) 
  {
    stanname(curbuf, curdev);
    if (*curdev && !strcmp(devname, curdev)) 
    { 					/* write new version of line */
      i = strlen(linebuf);
      if (write(fd,linebuf, i) != i) 
      {
        fprintf(stderr, MSGSTR(M_UPDATE,"Unable to update temp file.\n"));
        unlink(itfn2buf);
        close(fd);
        return(0);
      } /* end if */
    } /* end if */
    else 
    { 			/* write back original line (with white space) */
      i = strlen(curbuf);
      if (write(fd, curbuf, i) != i) 
      {
        fprintf(stderr,MSGSTR(M_UPDATE,"Unable to update temp file.\n"));
        unlink(itfn2buf);
        close(fd);
        return(0);
      } /* end if */
    } /* end else */
  } /* end while */

  if(lock_fd(fd))  {
	fprintf(stderr,MSGSTR(M_NLOCK,"Unable to lock file %s.\n"),itfn2buf);
        close(fd);
        return(0);
  }

  if ((fsync(fd)) == -1) 
  {
        fprintf(stderr,MSGSTR(M_UPDATE,"Unable to update temp file.\n"));
        unlink(itfn2buf);
        close(fd);
        return(0);
   }

  	/* Since inittab is still open via fpinit there will be 2 copies
    	   of inittab (what we want) until fpinit is closed. */
  if (rename(itfn2buf, itfn)) 
  {
    fprintf(stderr,MSGSTR(M_RENAME,"Unable to rename temp file.\n"));
    unlink(itfn2buf);
    return(0);
  } /* end if */

  /* Keep the new "/etc/inittab" open and hence locked till the next *
   * time we rename another temp file, at which point we will close  *
   * this one as.                                                    *
   */
  if(savefd > 0)
  	close(savefd);
  savefd=fd;
  fclose(fpinit2);
  return(1);
} /* end updinittab */

/* routine to determine if a port is active 
   portname  name of the port */
int isportact(char *portname) 
{
  struct utmp *b; 				/* pointer to that buffer */

  setutent(); 						/* rewind utmp file */
  while((b = getutent()) != NULL) 
  {
    if(strcmp(portname,b->ut_line) == 0) 
    {
      if(b->ut_type == USER_PROCESS) 
        return(1); 					/* Active */
      break;
    } /* end if */
  } /* end while */
  return(0); 						/* not active */
} /* end isportact */

static struct tty_entry *ttystatus = (struct tty_entry *)NULL;

/*
 * Get_request() added to support the new 'penable' function 'pstart'.
 * This is needed because because when doing a 'pstart' you must lookup
 * how the port is to be enabled.
 */
char get_request(char *name) 		/* look up status for name in odm */
{
    struct tty_entry *get_tty_status();
    struct tty_entry *curstatus;
    char *unname = undev(name);

    if (!ttystatus)
        ttystatus = get_tty_status();
    for (curstatus = ttystatus; curstatus && curstatus->ty_name; curstatus++)
        if (!strcmp(curstatus->ty_name, unname))
            return(curstatus->ty_status);
    return('\0');

} /* end get_request */

 /* get status for all ttys from odm */
struct tty_entry *get_tty_status()
{
    struct CuDv *cudv,*cuhftdv;
    struct listinfo cudv_status,cuhft_status;
    char criteria[MAX_CRITELEM_LEN];
    int howmany, i,totent;
    struct tty_entry *ttystatusp;
    void add_to_stat(struct tty_entry * ttystatusp, struct CuDv *cudv);
    						/* initialize the odm */
    odm_initialize(); 
    						/* initialize the criteria */
    sprintf( criteria, "name like tty* and status = %d", AVAILABLE );
    						/* get a list of tty objects */
    if ((cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_status,1,1))==
            (struct CuDv *) -1)
      return(NULL);

    sprintf( criteria, "name like hft* and status = %d", AVAILABLE );
    						/* get a list of tty objects */
    if ((cuhftdv = odm_get_list(CuDv_CLASS,criteria,&cuhft_status,1,1))==
            (struct CuDv *) -1)
      return(NULL);
    						/* check the number returned */
    if ((totent=cudv_status.num+cuhft_status.num) == 0)
      return(NULL);

    ttystatus = (struct tty_entry *)
      malloc((totent+1)*sizeof(struct tty_entry));
    if(NULL == ttystatus) 
    {
      fprintf(stderr, MSGSTR(M_NOMEM,"Cannot malloc memory.\n"));
      exit(1);
    } /* end if */
    							/* loop for each tty */
    for(i = 0, ttystatusp = ttystatus; i < cudv_status.num; i++, cudv++)
    {
      add_to_stat(ttystatusp,cudv);
      ttystatusp++;
    } /* end for */
    for(i = 0; i < cuhft_status.num; i++, cuhftdv++)
    {
      ttystatusp->ty_name = cuhftdv->name;
      ttystatusp->ty_status = DISABLE;
      ttystatusp->ty_config = cuhftdv->status;
      ttystatusp++;
    } /* end for */
    ttystatusp->ty_name = NULL;
    						/* terminate the ODM */
    odm_terminate();
    return(ttystatus);
} /* end get_tty_status */

/* convert odm status strings to constants */
int lookup (char *statstr) 
{
    if (strcmp (statstr, "enable") == 0)
      return(ENABLE);
    if (strcmp (statstr, "disable") == 0)
      return(DISABLE);
    if (strcmp (statstr, "share") == 0)
      return(SHARE);
    if (strcmp (statstr, "delay") == 0)
      return(DELAY);
    return('\0');
} /* end lookup */

/*
 * portavail - called with a port name, looks in ODM
 * to see if this port is defined, and whether it is available
 *
 * returns:
 *    AVAIL - device is available
 *    UNAVAIL - device is defined, but not available
 *    UNDEF - device is not defined
 */
portavail(devname)
char *devname;
{
    struct tty_entry *get_tty_status();
    struct tty_entry *curstatus;
    char *unname = undev(devname);

	/* console is always defined... */
  if (strcmp(unname,"console") == 0) {
    return AVAIL;
  }

	/* read ODM data if not already read in */
    if (!ttystatus)
        ttystatus = get_tty_status();

	/* look through for data about this tty */
    for (curstatus = ttystatus; curstatus && curstatus->ty_name; curstatus++) {
        if (!strcmp(curstatus->ty_name, unname)) {

		/* device is defined, is it available ? */
	    if (curstatus->ty_config == AVAILABLE) {
		return(AVAIL);
	    } else {
		return(UNAVAIL);
	    }
	}
    }

    return UNDEF;
}

void add_to_stat(struct tty_entry * ttystatusp, struct CuDv *cudv)
{
      int howmany;
      struct CuAt *cuat;
      extern void *getattr(char *, char *, int, int *);

      ttystatusp->ty_name = cudv->name;
      if ((cuat = getattr(cudv->name,"login",0,&howmany)) == 
        (struct CuAt *)NULL)
        return;
      if (howmany)
        ttystatusp->ty_status = lookup (cuat->value);
      ttystatusp->ty_config = cudv->status;
      return;
}

FILE *fopen_with_lock(char *filename)
{
	int rc,n;
	struct stat s1,s2;
	FILE *fp;

restat:
	if((fp=fopen(filename,"r+")) == NULL ) {
		fprintf(stderr, MSGSTR(M_OPEN,"Cannot open %s\n"),filename);
		return(NULL);
	}
	if(rc=fstat(fileno(fp),&s1)) {
		fprintf(stderr,MSGSTR(M_STAT,"Unable to stat file %s.\n"),filename);
		return(NULL);
	}
        /* serialize changes to the inittab file by getting lock */
    	if (lock_fd(fileno(fp)) < 0) {
		fprintf(stderr,MSGSTR(M_NLOCK,"Unable to lock file %s.\n"),filename);
        	return(NULL);
    	}
	if(rc=stat(filename,&s2)) {
		fprintf(stderr,MSGSTR(M_STAT,"Unable to stat file %s.\n"),filename);
		return(NULL);
	}
        /* while we were waiting for a lock on "/etc/inittab" *
         * another penable could have changed the file and    *
         * made a new entry for the file using rename(). So   *
         * check if the current inittab is the same as the one*
         * we locked.                                         *
         */
	if( s1.st_ino != s2.st_ino ) {
             	fclose(fp);
		goto restat; 
	}
        return(fp);

}

int lock_fd(int fd)
{
	struct flock fl;
    	fl.l_type = F_WRLCK; fl.l_whence = SEEK_SET;
    	fl.l_start = (off_t)0; fl.l_len = (off_t)1;
    	fl.l_sysid = 0; fl.l_pid = 0; fl.l_vfs = 0;
    	return (lockfx(fd,F_SETLKW,&fl));
}
