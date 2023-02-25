static char     sccsid[] = "@(#)89      1.16  src/bos/usr/sbin/install/instfix/instfix.c, cmdinstl, bos41J, 9521B_all 5/25/95 16:18:48";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: main (instfix) , fix_is_contained, findfileset,
 *	help, bailout, instfix_args, query_keywords, list_keywords,
 *	inst_keyword, inst_keylist,
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*--------------------------------------------------------------------------*
 * INCLUDE FILES
 *--------------------------------------------------------------------------*/

#define MAIN_PROGRAM
#include <stdio.h>
#include <swvpd.h>
#include <swvpd0.h>
#include <inudef.h>
#include "instfix.h"
#include <inu_toc.h>
#include <inulib.h>
#include <inuerr.h>
#include <fcntl.h>
#include <errno.h>

/* Make FAILURE a reasonable return value instead of -1 as defined in the VPD */
#ifdef FAILURE
 #undef FAILURE
 #define FAILURE 1
#endif

/*--------------------------------------------------------------------------*
 * Special globals.
 *--------------------------------------------------------------------------*/

int userblksize = 512;
extern int errno;
extern cmp fix_is_contained(prod_t *, int, int, int, int);
extern cmp findfileset(char *, prod_t *);

static char *instfix_path = 
    "PATH=/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/bin/X11:/sbin";

TOC_t           *TocPtr;             /* The pointer to the Media toc */

#define TMPDIR instfix.TMPDIR
#define INFILE instfix.INFILE
#define DEVICE instfix.DEVICE
#define KEYWORDS instfix.KEYWORDS
#define KEYTYPE instfix.KEYTYPE
#define STRING instfix.STRING

void inu_sig_abort();
extern char *get_value_from_string(char *, const char *, const int,char *);
extern char *strcpy(char *, const char *);
uid_t uid;


/*--------------------------------------------------------------------------*
**
** NAME: main (instfix)
**
** FUNCTION:
**  Install or update a Optional Program Product fix by keyword,
**  list the contents of the installation medium by keyword,
**  
**
** EXECUTION ENVIRONMENT:
**  instfix can be executed as a stand-alone command, or from a SMIT menu.
**  The user must have root authority.
**
**  The valid media in which instfix can work with:
**      Any media supported by open_toc (in inulib).
**
**  Flags:
** 	-T list the keywords from the TOC
**      -s requires -T.  Search for string.
**	-d device
**	-k keyword or string of keywords
**	-f file
**	-i return information only (requires -k or -f)
**	-p output filesets to stdout and do not invoke installp
**		(requires -k or -f)
**	-q quiet output
**	-v verbose output
**	-c colon-separated output
**	-F full set of filesets required
**	-t restrict keyword search to certain type
**
**  The instfix -k command builds a list of options that satisfy the
**  fixes requested and passes that list to installp or print the list.
**
**  instfix -f allows the user to specify an input file and pass 
**  multiple keywords to be processed.
** 
**  When the -i flag is given, instfix returns information about 
**  the given keyword or list of keywords from the input file.
**
** RETURNS:
**
**--------------------------------------------------------------------------*/

main (int             argc,
      char           **argv)
{
    int             rc;                  /* generic return code */
    char            buf[PATH_MAX+1];   	 /* generic string */
    char	    outfile[PATH_MAX+1]; /* output file passed to installp */
    nl_catd         catd;          	 /* file pointer to message catalog */
    FILE	    *fp;		/* stream pointer */
    int		    fd; 		/* file descriptor */

    typedef struct stat stat_t;		/* declarations for stat call */
    stat_t	statbuf;

    inu_cmdname = "instfix";
    TocPtr = NIL (TOC_t);

    /*-----------------------------------------------------------------------*
     * set stdout and stderr to no buffering
     * (this is so command will work correctly in smit)
     *-----------------------------------------------------------------------*/

    setbuf (stdout, NIL (char));
    setbuf (stderr, NIL (char));

    setlocale (LC_ALL, "");

    /*-----------------------------------------------------------------------*
     * Open the message catalog
     *-----------------------------------------------------------------------*/

    CATOPEN ();

    /*-----------------------------------------------------------------------*
     * Set up a path while we run, so we don't rely on the users path being
     * correct and the install shell scripts don't have to put absolute path
     * names for each command it executes.
     *-----------------------------------------------------------------------*/
    if (putenv(instfix_path) != SUCCESS)
	bailout(INUSETVAR);

    /*-----------------------------------------------------------------------*
     * Set signals to just remove our files if a signal is trapped...
     *-----------------------------------------------------------------------*/

    inu_set_abort_handler ((void (*) (int)) bailout);

    umask ((mode_t) (S_IWGRP | S_IWOTH));    /* set umask to 022     */

    /*-----------------------------------------------------------------------*
     * process command line and validate command line;
     *-----------------------------------------------------------------------*/

    if ((rc=instfix_args (argc, argv)) != SUCCESS)
    {
	help();
	bailout (rc);
    }
    /*-----------------------------------------------------------------------*
     * Make tmp directory via mktemp; mkdir;
     *-----------------------------------------------------------------------*/

    strcpy (TMPDIR, P_tmpdir);
    strcat (TMPDIR, "instfixXXXXXX");
    if (mktemp (TMPDIR) == NIL (char))
    {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_TMPDIR_E,
                                 CMN_NO_MK_TMPDIR_D), inu_cmdname);
        bailout (INUOPEN);
    }
    else
    {
        if (mkdir (TMPDIR, (mode_t) 0700) != 0)
        {
            inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_NO_MK_TMPDIR_E,
                                     CMN_NO_MK_TMPDIR_D), inu_cmdname);
            bailout (INUOPEN);
        }
    }

    /*-------------------------------------------------------------------*
     * i_FLAG and a_FLAG do not require a device.  Everything else does
     *-------------------------------------------------------------------*/

    /* if i flag */
    if (i_FLAG || a_FLAG)
    {
	rc = query_keywords();
	bailout(rc);
    } else {
	if (geteuid() != 0) {
	    errno = EACCES;
	    perror("instfix");
	    bailout(EACCES);
	}
    }

    /*-------------------------------------------------------------------*
     * check for existence of device
     *-------------------------------------------------------------------*/

    if ((rc = access (DEVICE, F_OK)) != SUCCESS)
    {
	inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_BAD_DEV_E, 
		CMN_BAD_DEV_D), inu_cmdname, DEVICE);
	bailout (INUACCS);
    }

    /*-------------------------------------------------------------------*
     * open_toc() will determine the type of device we're dealing with
     * and call the appropriate sub-function to read the toc off of the
     * media.
     *-------------------------------------------------------------------*/
    
    if ((rc = chdir (TMPDIR)) != SUCCESS)
    {
	inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_CHDIR_E,
				 CMN_CANT_CHDIR_D), inu_cmdname, TMPDIR);
	bailout (INUCHDIR);
	
    }

    /* open the toc */
    if ((rc = open_toc (&TocPtr, DEVICE, TRUE, CALLER_INSTFIX))
	!= SUCCESS)

	bailout (INUNOTIN);

    /* if T flag */
    if (T_FLAG)
    {
	rc = list_keywords();
	bailout(rc);
    }

    else 
    {   /* if k flag or f flag */
	/* build the tmp file you will pass to installp */
	
	strcpy(outfile, TMPDIR);
	strcat(outfile, "/options");
	
	/* open the tmp file which contain fileset list */
	if ((fp = fopen(outfile, "w")) == NIL(FILE)) 
	{
	    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
           		CMN_CANT_OPEN_D), inu_cmdname, outfile);

	    bailout(rc);
	}
	
	/* process cmd line keywords */
	if (k_FLAG)
	{
	    if (rc = inst_keyword(fp) != SUCCESS)
		bailout(rc);
	}
	
	/* process a list of keywords */
	if (f_FLAG)
	{
	    if (rc = inst_keylist(fp) != SUCCESS)
		bailout(rc);
	}
	
    }	
	
    fclose(fp);

    /* Check to see if there's anything in our list */
    fd = open(outfile,O_RDONLY);

    if ((rc = fstat(fd,&statbuf)) != SUCCESS)
    {
	inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
                CMN_CANT_OPEN_D), inu_cmdname, outfile);
	bailout(rc);
    }

    if (statbuf.st_size == 0)
    {
	inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, ISF_NO_KEYWORD_LIST_I,
		 ISF_NO_KEYWORD_LIST_D),inu_cmdname);
	bailout(SUCCESS);
    }

    close(fd);

    if (p_FLAG)
    {
	/* only output the files once */
	strcpy(buf,"sort -u < ");
	strcat(buf,outfile);
    }
    else
    {
	/* call installp with tempfile */
	strcpy(buf,"installp -f ");
	strcat(buf,outfile);
	strcat(buf," -aBgX "); 
	strcat(buf," -d ");
	strcat(buf,DEVICE);
    }
    rc=system(buf);
    bailout(rc);

}


/*---------------------------------------------------------------

  Install a list of keywords 

  When using the -f flag, the input file is assumed to
	have the following format:

TOPOFFILE
keyword comments...
nextkeyword comments...
....
BOTTOMOFFILE

i.e. the only tokens that are meaningful are the first tokens on 
the input line.  This format is used as it is also the format of
the output of the instfix -T command *
-------------------------------------------------------------------*/

int inst_keylist(
		 FILE *fp 	/* output file */
		 )
{
    int rc;                   /* return code */
    FILE *infile;             /* input file stream */
    char line[MAX_INPUT];     /* buffer to hold input line */
    char *ptr;                /* pointer to tokens in input line */
    int  any_success=FALSE;   /* TRUE if any keywords return a match */


    /* open the input file */
    if (strcmp (INFILE, "-") == 0)  /* if the input file is stdin */
	infile=stdin;
    else
    {
	if ((infile = fopen(INFILE, "r")) == NIL(FILE))
	{
	    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
                               CMN_CANT_OPEN_D), inu_cmdname, INFILE);
	    return (FAILURE);
	}
    }

    while ((fgets(line,MAX_INPUT,infile)) != NULL)
    {
	/* ignore empty lines */
	if ((ptr=strtok(line, " \t\n")) == NIL (char))
	    continue;

	strcpy(KEYWORDS,ptr);

	if ((rc = inst_keyword(fp)) == SUCCESS)
	    any_success=TRUE; 
    }

    if (any_success)
       return(SUCCESS);
    else
       return(FAILURE);
}


/* Process line of keywords */
/* OUTPUT: writes the fileset information about the
	keyword to the output file */

int inst_keyword(
		 FILE *fp
		 )
{
    lpp_t     lpp_entry;           /* is referenced fileset installed? */
    Option_t *op;                  /* pointer to top of option list */
    char     *ptr;                 /* place holder in fixdata string */
    char     *keyword;             /* single keyword from cmd line */
    char      tc;                  /* terminating character */
    int       relevant_fix = 0;    /* fix applies to installed filesets? */
    int       fileset_exists;      /* is referenced fileset installed? */
    int       any_success=FALSE;   /* TRUE if any keywords return a match */
    int       found,expected;      /* keyword found,expected indicators */
    int       rc;                  /* generic return code */

    vpdlocalpath(VPD_USR_PATH);
    vpdremotepath(VPD_SHARE_PATH);
    vpdlocal();

    for (keyword=strtok(KEYWORDS," \n\t"); keyword != NIL (char); 
	 keyword=strtok(NULL," \n\t"))
    {
	/*  for all the options in the toc */
	found = 0;
	relevant_fix = 0;

	for (op = TocPtr -> options; op != NIL (Option_t) ;op = op -> next)
	{
	    /* look at all the *keywords lists to see if we have a 
	       match.  get_value_from_string is used instead of 
	       strtok because it does not insert NULLs into the
	       string like strtok does (subsequent calls would have 
	       altered fixdata fields */

	    for (ptr = get_value_from_string(op->fixdata," \t\n",TRUE,&tc);
		 ptr != NIL (char);
		 ptr=get_value_from_string(NULL," \t\n",TRUE,&tc))
	    {
		/* if tc is newline, formatting error */
		if (tc=='\n')
		{
		    inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON, CMN_BAD_TOC_E,
				CMN_BAD_TOC_D),inu_cmdname);
		    return (FAILURE);
		}
		    
		/* if you have a match */
		if(strcmp(keyword,ptr) == 0) 
		{
		    if (found==0)
		    {
			/* next thing on the line is the number of filesets
			   required to fix the problem	*/

			expected=atoi(get_value_from_string(NULL," \t\n",TRUE,&tc));

			if (expected < 1)
			{
			/* If number of filesets not specified, then assume
			 * that 1 is enough and let the requisites fall where
			 * they may...
			 *  inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON, CMN_BAD_TOC_E,
                         * 	     CMN_BAD_TOC_D),inu_cmdname);
			 *  return (FAILURE);
			 */
			    expected=1;
                	}
		    	found=1;
		    }
		    else
			found++;
		    
		    /* Check to see if this fileset exists on this system. */
		    strcpy(lpp_entry.name, op->name);
    		    fileset_exists = vpdget(LPP_TABLE,LPP_NAME,&lpp_entry);
		    if (fileset_exists != VPD_OK) {
			/* Check the share part */
			vpdremote();
			fileset_exists = vpdget(LPP_TABLE,LPP_NAME,&lpp_entry);
        	        vpd_free_vchars (LPP_TABLE, &lpp_entry); 
			vpdlocal();
		    } else {
        	        vpd_free_vchars (LPP_TABLE, &lpp_entry); 
		    }
		    if (fileset_exists == VPD_OK) {
			relevant_fix = 1;

		        /* write it to the temp file */
		        if (*(op->level.ptf) == '\0')
			    fprintf(fp, "%s %02d.%02d.%04d.%04d\n",
				     op->name, op->level.ver, op->level.rel,
				     op->level.mod,op->level.fix);
		        else	
			    fprintf(fp, "%s %02d.%02d.%04d.%04d.%s\n",
				     op -> name,
				     op -> level.ver,
				     op -> level.rel,
				     op -> level.mod,
				     op -> level.fix,
				     op -> level.ptf);
		
		    }
		    /* fall out to the next option */ 
		    break;
		}

		if (tc!='\n')
		    /* eat the rest of the line */
		    ptr=get_value_from_string(NULL,"\n",FALSE,&tc);
		
	    }
	}   
    
       /* if you didn't have any matches, indicate error */
        if (found == 0)
           inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, ISF_NO_KEYWORD_DATA_I,
                                ISF_NO_KEYWORD_DATA_D), inu_cmdname, keyword);
        else if (found < expected)
           inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, 
                                        ISF_NOT_ALL_FILESETS_FOUND_I, 
                                        ISF_NOT_ALL_FILESETS_FOUND_D), keyword);
        else if (!relevant_fix)
           inu_msg (SCREEN_MSG, MSGSTR (MS_INUCOMMON, 
                                        ISF_NO_RELEVANT_FILESETS_I,
                                        ISF_NO_RELEVANT_FILESETS_D), keyword);
        else
           any_success = TRUE;

    }

    if (any_success)
       return(SUCCESS);
    else
       return(FAILURE);

}

/* handle the T option */
int list_keywords ()
{
    int rc;			/* return code */
    char *ptr;			/* place holder in fixdata strings */
    Option_t *op;               /* pointer to top of option list */
    FILE  *fp;			/* output file stream pointer */
    char  buf[20 + PATH_MAX+1]; /* buffer to hold sort command */
    char  matches[PATH_MAX+1];  /* matches file path name (temp file) */
    char  sorted[PATH_MAX+1];   /* sorted file path name (temp file) */
    char  outbuf[MAX_INPUT];	/* buffer in which to build output string */
    char  *lastapar;		/* last apar seen while scanning list */
    char  line[MAX_INPUT];	/* input line buffer */
    char  save[MAX_INPUT];	/* save line buffer */
    char  tc;			/* Terminating character of a string */
    int   expected,count;       /* expected number of filesets, count of filesets */
    int   reqcount_omitted;     /* expected number of filesets omitted */

    /* create matches file path */
    strcpy(matches, TMPDIR);
    strcat(matches, "/matches");
    
    strcpy(sorted, TMPDIR);
    strcat(sorted, "/sorted");
    
    /* open the matches file for write */
    if ((fp = fopen(matches, "w")) == NIL(FILE))
    {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
				   CMN_CANT_OPEN_D), inu_cmdname, matches);
        bailout(rc);
    }
    
    /* for all options in the toc */
    for (op = TocPtr -> options; op != NIL (Option_t) ;op = op -> next)
    {
	/* output all matches to matches file */
	for (ptr = get_value_from_string(op->fixdata," \t\n",TRUE,&tc);
	     ptr != NIL (char);
	     ptr=get_value_from_string(NULL," \t\n",TRUE,&tc))
	{
	    if (tc == '\n')
	    {
		inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON, CMN_BAD_TOC_E,
					  CMN_BAD_TOC_D),inu_cmdname);
		return (FAILURE);
            }
	    
	    /* copy the first token (apar) to the output buffer */
	    strcpy(outbuf,ptr);
	    strcpy(save,ptr);
	    
	    /* append a blank */
	    strcat(outbuf," ");
	    strcat(save," ");
	    
	    /* get and append the second token plus a blank */
	    ptr=get_value_from_string(NULL," \t\n",TRUE,&tc);
	    reqcount_omitted = 0;
	    if (atoi(ptr) == 0) {
		reqcount_omitted = 1;
		strcat(outbuf,"1 ");
	    
	        /* append the option (fileset) name plus a blank */
	        strcat(outbuf,op->name);
	        strcat(outbuf," ");
	    }
 	    strcat(outbuf,ptr);
	    strcat(outbuf," ");
	    
	    
	    if (!reqcount_omitted) {
	        /* append the option (fileset) name plus a blank */
	        strcat(outbuf,op->name);
	        strcat(outbuf," ");
	    }
	    
	    /* if we haven't hit the end of the line, the rest of the
	       line is the abstract.  Append the abstract */
	    if (tc != '\n')
		ptr=get_value_from_string(NULL,"\n",TRUE,&tc);
	    
 	    strcat(outbuf,ptr);
 	    strcat(save,ptr);
 	    strcat(outbuf,"\n");
 	    strcat(save,"\n");
	    
	    
	    /* if s_FLAG, limit the output */
	    if (s_FLAG)
	    {
		if(strstr(save,STRING) != NIL (char))
		    fprintf(fp,"%s",outbuf);
	    }
	    else /* output everything */
		fprintf(fp,"%s",outbuf);
	    
	}
	
    }
    /* close and sort temp file */
    fclose(fp);
    
    /* sort -u eliminates duplicate output */
    strcpy(buf,"sort -u +0 -3 < ");
    strcat(buf,matches);
    strcat(buf," > ");
    strcat(buf,sorted);
    
    system(buf);

    /* 
      Fixes on the media may not be complete.  The next section
      ensures that we only display complete fixes
      */

    /* 
      the 'sorted' file contains:
      apar #fixes_expected fileset_name abstract
      (one line per apar)
      */

    /* open the sorted file for read */
    if ((fp = fopen(sorted, "r")) == NIL(FILE))
    {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
				   CMN_CANT_OPEN_D), inu_cmdname, sorted);
        bailout(rc);
    }
    
    lastapar=outbuf;
    strcpy(lastapar,NULL);
    expected=0;
    count=0;
    
    /* read all the lines in the sorted file */
    while ((fgets(line,MAX_INPUT,fp)) != NULL)
    {
        /* look at the first token */
	ptr=get_value_from_string(line," \t",TRUE,&tc);
	
	/* if it is the same as the last line in the sorted file
	   then bump the count and go on */
	if (strcmp(ptr,lastapar) == 0)
	    count++;
	
	/* If it isn't the same, process the last one */
	else
	{
	    if (strcmp(lastapar,NULL) == 0)
	    {
		strcpy(lastapar,ptr);
	        expected=atoi(ptr=get_value_from_string(NULL," \t",TRUE,&tc));
		count=1;
		/* copy the line for future use */
		strcpy(save,line);
		continue;
	    }
		
		

	    /* 
	      if we've seen the expected number of occurrences, and this
	      isn't the first one, then go ahead and display the
	      apar and the abstract.  Reset the count and expected values.
	      */

	    if (count >= expected) 
	    {
		/* skip three tokens, then copy abstract */
		ptr=strtok(save," \t");
		strcpy(outbuf,ptr);
		ptr=strtok(NULL," \t");
		ptr=strtok(NULL," \t");
		ptr=strtok(NULL,"\n");
		strcat(outbuf," ");
		strcat(outbuf,ptr);
		printf("%s\n",outbuf);
	    }
	    expected=atoi(ptr=get_value_from_string(NULL," \t",TRUE,&tc));
	    count = 1;
	    /* copy the line for future use */
	    strcpy(save,line);
	    ptr=strtok(line," \t");
	    strcpy(lastapar,ptr);
	}
    }

    /* take care of the lines we did not process before getting to EOF */
    if (((count >= expected) && (strcmp(lastapar,NULL) != 0)))
    {
	/* skip three tokens, then copy abstract */
	ptr=strtok(save," \t");
	strcpy(outbuf,ptr);
	ptr=strtok(NULL," \t");
	ptr=strtok(NULL," \t");
	ptr=strtok(NULL,"\n");
	strcat(outbuf," ");
	strcat(outbuf,ptr);
	printf("%s\n",outbuf);
    }
    
    return (SUCCESS);
    
}

/*---------------------------------------------------------------
  search for keywords from the command line or multiple keywords 
	from a file 

  When using the -f flag, the input file is assumed to
        have the following format:

TOPOFFILE
keyword comments...
nextkeyword comments...
....
BOTTOMOFFILE

i.e. the only tokens that are meaningful are the first tokens on 
the input line.  This format is used as it is also the format of
the output of the instfix -T command *
---------------------------------------------------------------*/

int query_keywords()
{

    FILE  *infile ;  /*  input stream */
    char  *ptr;      
    int   rc,remember;
    char  line[MAX_INPUT];
    
    if (k_FLAG)
    {
	/* process cmd line queries */
	return(query_cmdline());	
    }
    if (f_FLAG)
    {
	/* query for all keywords in the list */
	/* open the input file */

        if (strcmp (INFILE, "-") == 0)  /* if the input file is a "-" then */
            infile = stdin;

        else
        {
           
            if ((infile = fopen(INFILE, "r")) == NIL(FILE))
            {
                inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E,
                                           CMN_CANT_OPEN_D), 
                         inu_cmdname, infile);

                return (FAILURE);
            }
        }

	/* for every line in the input file */
	remember=SUCCESS;
	while ((fgets(line,MAX_INPUT,infile)) != NULL)
	{
	    /* ignore empty lines */
	    if ((ptr=strtok(line, " \t\n")) == NIL (char))
		continue;

	    strcpy(KEYWORDS,ptr);
	    
	    rc = query_one(KEYWORDS, (fix_t *) NULL);
	    if (rc != SUCCESS)
		remember=rc;
	}

	return(remember);
    }
    if (i_FLAG) {
	return(query_all());
    }
    if (a_FLAG) {
	return(query_symptoms());
    }

}

int query_cmdline()
{
    char *keyword;
    char tc;
    char *ptr;
    int  keywordlen;
    int  rc,remember;
    int  separator = FALSE;
	
    if ((c_FLAG) && (!q_FLAG)) {
	 inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_C_OUTPUT_HDR_I,
                                              ISF_C_OUTPUT_HDR_D)); 
    }
    remember=SUCCESS;
    ptr=KEYWORDS;
    keywordlen=strlen(ptr);
    for (keyword=strtok(ptr," \t"); 
	 keyword != NIL (char) && ((int)(ptr-KEYWORDS) < keywordlen);
         keyword=strtok(ptr," \t"))
    {
        if (separator) {
	   inu_msg(LOG_MSG,"==============================================================================\n");
	} else {
	   if (a_FLAG && !c_FLAG) {
	       separator = TRUE;
	   }
	}
	rc=query_one(keyword, (fix_t *) NULL);
	ptr+=strlen(keyword)+1;
	if (rc != SUCCESS)
	    remember=rc;
    }

    return(remember);

}

/* query a single keyword and report on whether relevant filesets are applied */
int query_one (char *keyword, fix_t *knownfix)
{
    fix_t	query_fixinfo;  /* local fixinfo entry */
    fix_t	*fixinfo;	/* local or passed fixinfo entry */
    prod_t	installed_level;/* product database entry */
    int		onefound;	/* indicates that at least 1 fileset is found */
    int		allfound;	/* indicates that all filesets are found */
    int		anydownlevel;	/* all installed filesets at required level */
    int 	rc;		/* return code */
    char 	*ptr;		/* pointer to fix info data */
    char	fileset[MAX_INPUT]; /* Fileset identified in fix database */
    char	tc;		/* terminating character */
    cmp		fileset_status; /* not installed, downlevel, exact, uplevel */

    /* search fix data base for keyword */
    vpdlocalpath(VPD_LOCAL_PATH);
    vpdlocal();
    if (knownfix == NULL) {
	fixinfo = &query_fixinfo;
        strcpy(fixinfo->name,keyword); 
        if (t_FLAG) {
            strcpy(fixinfo->type,KEYTYPE); 
        }
    } else {
	fixinfo = knownfix;
    }
    
    /* get the first entry of the correct name */
    if ((knownfix != (fix_t *) NULL) || 
	((rc=vpdget(FIX_TABLE, FIX_NAME | ((t_FLAG) ? FIX_TYPE : 0),
		    fixinfo)) == VPD_OK))
    {

	/*
	if (( v_FLAG ) && (! c_FLAG )) {
            inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_PROC_QUERY_I,
		     ISF_PROC_QUERY_D),inu_cmdname,keyword);
	}
	*/

	if (( v_FLAG || a_FLAG ) && (! c_FLAG )) {
	    inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_ABSTRACT_I,
				  ISF_ABSTRACT_D),keyword,fixinfo->abstract);
	}

	if (( a_FLAG ) && (! c_FLAG )) {
	        fprintf(stdout,"%s ", keyword);
	        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_SYMPTOM_I,
				      ISF_SYMPTOM_D),fixinfo->symptom);
	}

	if (a_FLAG) { 
	    if (! i_FLAG ) {
	        return(SUCCESS);
	    } else {
	        inu_msg(LOG_MSG,"----------------------------\n");
	    }
	}

	/* for all filesets listed */
	allfound = TRUE;
	anydownlevel = FALSE;
	onefound = FALSE;
        for (ptr = get_value_from_string(fixinfo->filesets,"\n",TRUE,&tc);
                 ptr != NIL (char);
                 ptr=get_value_from_string(NULL,"\n",TRUE,&tc))
	{
	    /* copy the fileset name */	
	    strcpy(fileset,ptr);
	    
	    /* indicate whether it is applied on the
	       system */

	    fileset_status = findfileset(ptr, &installed_level);
	    switch (fileset_status) {
		case NOT_INSTALLED:
		    if (v_FLAG) {
			if (!c_FLAG) {
	                    fputs("    ",stdout);
			    if ((ptr = strchr (fileset, ':')) != NULL) {
				*ptr = '\0';
			    }
		            inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, 
					  ISF_FILESET_NOT_FOUND_I,
					  ISF_FILESET_NOT_FOUND_D),fileset);
			} else {
	                    inu_msg(LOG_MSG,"%s:%s:0.0.0.0:!:%s\n",
				    keyword,
				    fileset,
				    fixinfo->abstract);
			}
		    }
		    if (allfound) {
		        allfound = FALSE;
		    }
		    break;
		case DOWN_LEVEL:
		    if (v_FLAG && !c_FLAG) {
	                fputs("    ",stdout);
		        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, 
					  ISF_FILESET_NOT_FOUND_I,
					  ISF_FILESET_NOT_FOUND_D),fileset);
		    }
		    if (c_FLAG) {
	                    inu_msg(LOG_MSG,"%s:%s:%d.%d.%d.%d:-:%s\n",
				    keyword,
				    fileset,
				    installed_level.ver,
				    installed_level.rel,
				    installed_level.mod,
				    installed_level.fix,
				    fixinfo->abstract);
		    }
		    if (allfound) {
		        allfound = FALSE;
		    }
		    if (!anydownlevel) {
		        anydownlevel = TRUE;
		    }
		    break;
		case EXACT_MATCH:
		case SUPERSEDED:
		    if (! onefound) {
		        onefound = TRUE;
		    }
		    if ((v_FLAG) && (!c_FLAG)) {
	                fputs("    ",stdout);
		        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, 
					  ISF_FILESET_FOUND_I,
					  ISF_FILESET_FOUND_D),fileset);
		    }
		    if (c_FLAG) {
	                    inu_msg(LOG_MSG,"%s:%s:%d.%d.%d.%d:%c:%s\n",
				    keyword,
				    fileset,
				    installed_level.ver,
				    installed_level.rel,
				    installed_level.mod,
				    installed_level.fix,
				    (fileset_status == EXACT_MATCH) ? '=' : '+',
				    fixinfo->abstract);
		    }
	            break;
	    }
	    
	}
	/* If at least one fileset was found at the right level,
	 * AND either all of the filesets were found OR all of the
	 * filesets which were found were at the required level and the
	 * user did not required that ALL filesets be found, then return
	 * success.  Got it?
	 */
	if ((onefound) && ((allfound) || ((!anydownlevel) && (!F_FLAG))))
	{
	    if ((!q_FLAG) && (!c_FLAG)) {
	        fputs("    ",stdout);
	        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, 
				      ISF_ALL_FILESETS_FOUND_I,
				      ISF_ALL_FILESETS_FOUND_D), keyword);
	    }
	    rc=SUCCESS;
	}
	else
	{
	    if ((!q_FLAG) && (!c_FLAG)) {
	        fputs("    ",stdout);
	        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, 
				      ISF_NOT_ALL_FILESETS_FOUND_I,
				      ISF_NOT_ALL_FILESETS_FOUND_D), keyword);
	    }
	    rc=FAILURE;
	}
    }
    
    else 
    {
	if (rc == VPD_NOMATCH)
	{
	    /*
	    if ((v_FLAG) && (!c_FLAG)) {
                inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON, ISF_PROC_QUERY_I,
		     ISF_PROC_QUERY_D),inu_cmdname,keyword);
	    }
	    */
	    if (!q_FLAG) {
		if (!c_FLAG) {
	            fputs("    ",stderr);
	            inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON, 
				      ISF_NO_FIX_DATA_I,
				      ISF_NO_FIX_DATA_D), keyword);
		} else {
	            inu_msg(LOG_MSG,"%s: :0.0.0.0:?: \n", keyword);
		}
	    }
	}
	else /* VPD_SYS */
	{
	    if (!q_FLAG) {
		if (!c_FLAG) {
	            inu_msg(FAIL_MSG,MSGSTR(MS_INUCOMMON, 
				    ISF_VPD_ERROR_E,
				    ISF_VPD_ERROR_D), inu_cmdname);
		} else {
	            inu_msg(LOG_MSG,"%s: :0.0.0.0:?: \n", keyword);
		}
	    }
	    
	    bailout (1);
	}
	
    }
    
    /* free the vchar memory used */
    if (knownfix == (fix_t *) NULL) {
        vpd_free_vchars (FIX_TABLE, fixinfo); 
    }
    
    return (rc);
}

/* Retrieve all of the fix information */
int query_all ()
{

    fix_t	fixinfo;	/* fix database entry */
    int		rc;		/* Generic return value */
    int		separator = FALSE;

    /* search fix data base for keyword */
    vpdlocalpath(VPD_LOCAL_PATH);
    vpdlocal();

    if (t_FLAG) {
        strcpy(fixinfo.type,KEYTYPE); 
    }
    
    /* get the first entry of the correct name */
    rc=vpdget(FIX_TABLE, ((t_FLAG) ? FIX_TYPE : 0), &fixinfo);
    while (rc == VPD_OK)
    {
        if (separator) {
	   inu_msg(LOG_MSG,"==============================================================================\n");
	} else {
	   if (a_FLAG && !c_FLAG) {
	       separator = TRUE;
	   }
	}
	query_one(fixinfo.name, &fixinfo);
        vpd_free_vchars (FIX_TABLE, &fixinfo); 
	rc = vpdgetnxt (FIX_TABLE, &fixinfo);
    }

    return(SUCCESS);
}

/* Retrieve all of the symptoms from the fix database */
int query_symptoms ()
{

    fix_t	fixinfo;	/* fix database entry */
    int 	separator = FALSE;
    int		rc;		/* Generic return value */

    /* search fix data base for keyword */
    vpdlocalpath(VPD_LOCAL_PATH);
    vpdlocal();

    if (t_FLAG) {
        strcpy(fixinfo.type,KEYTYPE); 
    }
    
    /* get the first entry of the correct name */
    rc=vpdget(FIX_TABLE, ((t_FLAG) ? FIX_TYPE : 0), &fixinfo);
    while (rc == VPD_OK)
    {

	if (separator) {
	    inu_msg(LOG_MSG,"==============================================================================\n");
	} else {
	    separator = TRUE;
	}

	if (( v_FLAG || a_FLAG ) && (! c_FLAG )) {
	    inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_ABSTRACT_I,
				  ISF_ABSTRACT_D),fixinfo.name,fixinfo.abstract);
	}

	if (( a_FLAG ) && (! c_FLAG )) {
	        fprintf(stdout,"%s ", fixinfo.name);
	        inu_msg(LOG_MSG,MSGSTR(MS_INUCOMMON, ISF_SYMPTOM_I,
				      ISF_SYMPTOM_D),fixinfo.symptom);
	}
	vpd_free_vchars (FIX_TABLE, &fixinfo);
	rc = vpdgetnxt (FIX_TABLE, &fixinfo);
    }
    return(SUCCESS);
}


int instfix_args (int  sargc,     /* number of arguments passed to instfix */
		  char **sargv)   /* argument list passed to instfix */
{

    extern char *optarg;
    int c;
    char *ptr;
    int i;
    char buf[PATH_MAX+1];

    if (sargc < 1)
    {
	help();
	bailout(1);
    }

    while ((c = getopt(sargc, sargv, "d:f:iak:s:t:TpcqvF")) != EOF)
    {
	switch(c)
	{

	case 'd':
	    /*set d flag */		
	    d_FLAG = TRUE_C;

	    /* remember device */
	    if ((ptr = strcpy(DEVICE ,optarg)) == NIL (char))

		return(FAILURE);

	    /*--------------------------------------------------------
	     * If DEVICE isn't a full path then it is a file, so prepend our
	     * current directory to it.
	     *------------------------------------------------------*/

	    if (*DEVICE != '/')
	    {
		if ((getcwd (buf, (int) sizeof (buf))) == NIL (char))
		{
		    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
					       CMN_BAD_DEV_E, CMN_BAD_DEV_D),
			     inu_cmdname, DEVICE);
		    bailout (INUUNKWN);
		}
		else
		{
		    strcat (buf, "/");
		    strcat (buf, DEVICE);
		    strcpy (DEVICE, buf);
		}
	    }

	    break;

	    
	case 'f':

	    /* set f flag */
	    f_FLAG = TRUE_C;

	    /* remember file */
	    if ((ptr = strcpy(INFILE, optarg)) == NIL (char))

		return(FAILURE);

	    if (*INFILE != '/' && strcmp(INFILE,"-") != 0) 
	    {
		if ((getcwd (buf, (int) sizeof (buf))) == NIL (char))
		{
		    inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
					       CMN_BAD_DEV_E, CMN_BAD_DEV_D),
			     inu_cmdname, INFILE);
		    bailout (INUUNKWN);
		}
		else
		{
		    strcat (buf, "/");
		    strcat (buf, INFILE);
		    strcpy (INFILE, buf);
		}
	    }
	    
	    break;

	case 'c':
	    /* colon-separated output */
	    c_FLAG = TRUE_C;

	    break;

	case 'F':
	    /* Full set of filesets required */
	    F_FLAG = TRUE_C;

	    break;

	case 'q':
	    /* verbose output */
	    q_FLAG = TRUE_C;

	    break;


	case 'v':
	    /* verbose output */
	    v_FLAG = TRUE_C;

	    break;

	case 'i':

	    /* set i flag - information only */
	    i_FLAG = TRUE_C;

	    break;

	case 'a':

	    /* set a flag  - show symptom */
	    a_FLAG = TRUE_C;

	    break;

	case 'k':
 
	    /* get keywords */		
	    k_FLAG = TRUE_C;

	    /* remember keywords */
	    if ((ptr = strcpy(KEYWORDS ,optarg)) == NIL (char))

		return(FAILURE);

	    break;

	case 's':
	
	    /* set s flag */
	    s_FLAG = TRUE_C;

	    /* set T flag implicitly */
	    T_FLAG = TRUE_C;

	    /* remember string */
	    if ((ptr = strcpy(STRING ,optarg)) == NIL (char))

		return (FAILURE);

	    break;
	    
	case 'T':
	    /*set T flag */		
	    T_FLAG = TRUE_C;
	    break;

	case 't':
	    /*set t flag  - restrict search to specific keyword type */		
	    t_FLAG = TRUE_C;

	    /* remember type flag */
	    if (optarg == NIL (char))

		return(FAILURE);

	    KEYTYPE[0] = optarg[0];
	    KEYTYPE[1] = '\0';

	    break;

	case 'p':
	    /*set p flag */		
	    p_FLAG = TRUE_C;
	    break;

	default:
	    break;
	}
    }

    /* 
	ensure one of i, k, f, or T are set,
	ensure device given unless i option selected
	ensure i_FLAG has either keyword or file
	ensure p_FLAG has either keyword or file
	ensure we don't have k_FLAG and f_FLAG together
	ensure we don't have i_FLAG and p_FLAG together
	ensure i_FLAG present if a_FLAG used
     */

    if (!(i_FLAG || a_FLAG || k_FLAG || f_FLAG || T_FLAG) || 
	(!i_FLAG && !d_FLAG && !a_FLAG) || 
	(p_FLAG && !(k_FLAG || f_FLAG)) ||
	(k_FLAG && f_FLAG) ||
	(i_FLAG && p_FLAG)
	)
	return(FAILURE);

    return(SUCCESS);
}

bailout (
	 int rc
	 )
{
    if ((chdir ("/")) == SUCCESS)

	(void) inu_rm(TMPDIR); 
    /* terminate vpd */
    vpdterm();

    exit(rc);
}

help()
{
    inu_msg(SCREEN_MSG,MSGSTR(MS_INUCOMMON,ISF_HELP_I,ISF_HELP_D));
}

cmp findfileset (
		char	*ptr,		/* ptr to string containing fs vrmf */
		prod_t	*highest_level	/* info about fileset level installed */
		)
{
    prod_t prod_entry;		/* product table entry */
    cmp fileset_status;		/* not_installed, downlevel, exact, uplevel */
    cmp overall_status;		/* cumulative status across all levels */
    int rc;			/* generic return code */

    int ver;			/* version */	
    int rel;			/* release */
    int mod;			/* modification level */
    int fix;			/* fix level */

    /* if our query has a ptf id in it, just search for that */

    /* init odm query entry */
    strcpy(prod_entry.lpp_name,strtok(ptr,":"));
    errno = 0;

    ver=atoi(strtok(NULL,"."));
    if (errno)
	return(FALSE);

    rel=atoi(strtok(NULL,"."));
    if (errno)
	return(FALSE);

    mod=atoi(strtok(NULL,"."));
    if (errno)
	return(FALSE);

    fix=atoi(strtok(NULL,".\t\n: "));
    if (errno)
	return(FALSE);

    overall_status = NOT_INSTALLED; /* Initialize state */

    if (c_FLAG) {
       highest_level->ver = 0;
       highest_level->rel = 0;
       highest_level->mod = 0;
       highest_level->fix = 0;
    }

    /* find odm entry with this name (vpdget) */
    /* for all matching entries */
    for (rc = vpdget(PRODUCT_TABLE,PROD_LPP_NAME,&prod_entry);
	     rc == VPD_OK;
	     rc = vpdgetnxt(PRODUCT_TABLE,&prod_entry))
    {
	fileset_status = fix_is_contained(&prod_entry,ver,rel,mod,fix);
	/* If colon-format output requested, keep track of the highest 
	 * installed level of the fileset 
	 */
	if (c_FLAG) {
	    if (fix_is_contained(&prod_entry, highest_level->ver,
	    			 highest_level->rel, highest_level->mod,
				 highest_level->fix) == SUPERSEDED) {
	        highest_level->ver = prod_entry.ver;
	        highest_level->rel = prod_entry.rel;
	        highest_level->mod = prod_entry.mod;
	        highest_level->fix = prod_entry.fix;
	    }
	}
	vpd_free_vchars (PRODUCT_TABLE, &prod_entry);

	if (fileset_status > overall_status ) {
	    overall_status = fileset_status;
	}
    }

    /* check the share part to see if the fileset is installed */

    if (overall_status == NOT_INSTALLED) {
    	vpdlocalpath(VPD_USR_PATH);
    	vpdremotepath(VPD_SHARE_PATH);
	vpdremote();
    	for (rc = vpdget(PRODUCT_TABLE,PROD_LPP_NAME,&prod_entry);
		     rc == VPD_OK;
		     rc = vpdgetnxt(PRODUCT_TABLE,&prod_entry))
    	{
		fileset_status = fix_is_contained(&prod_entry,ver,rel,mod,fix);
		/* If colon-format output requested, keep track of the highest 
		 * installed level of the fileset 
		 */
		if (c_FLAG) {
	    if (fix_is_contained(&prod_entry, highest_level->ver,
	    			 highest_level->rel, highest_level->mod,
					 highest_level->fix) == SUPERSEDED) {
		        highest_level->ver = prod_entry.ver;
        	        highest_level->rel = prod_entry.rel;
                	highest_level->mod = prod_entry.mod;
                	highest_level->fix = prod_entry.fix;
            	}
        	}
        	vpd_free_vchars (PRODUCT_TABLE, &prod_entry);
	
        	if (fileset_status > overall_status ) {
            	overall_status = fileset_status;
        	}
    	}
	vpdlocal();
   }

    return (overall_status);
	
}

cmp fix_is_contained ( 
		      prod_t  *prod_entry,
		      int ver,
		      int rel,
		      int mod,
		      int fix
		      )
{		

    switch (prod_entry->state)
    {
    case ST_APPLIED:
    case ST_COMMITTED:
    case ST_COMMITTING:
	break;
    default:
	return(NOT_INSTALLED);
    }
    /* version in fixdata > version in entry on machine? */
    if (ver > prod_entry->ver)
	return(DOWN_LEVEL);
    else
	if (ver < prod_entry->ver)
	    return (SUPERSEDED);

    /* version is equal */
    
    /* release in fixdata > version in entry on machine */
    if (rel > prod_entry->rel)
	return(DOWN_LEVEL);
    else
	if (rel < prod_entry->rel)
	    return (SUPERSEDED);

    /* release is equal */

    /* mod level in fixdata > mod level in entry on machine */
    if (mod > prod_entry->mod)
	return(DOWN_LEVEL);
    else
	if (mod < prod_entry->mod)
	    return (SUPERSEDED);

    /* mod is equal */

    /* fix level in fixdata > fix level in entry on machine */
    if (fix > prod_entry->fix)
	return(DOWN_LEVEL);

    if (fix < prod_entry->fix)
	return(SUPERSEDED);

    /* must be OK */
    return (EXACT_MATCH);
}
