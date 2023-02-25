char    sccs_id[] = " @(#)04 1.9  src/bos/usr/sbin/nimclient/do_nim.c, cmdnim, bos411, 9434B411a  8/19/94  10:19:05";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nimclient/do_nim.c
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* ---------------------------- do_nim
 *
 * NAME: do_nim
 *
 * FUNCTION:
 *	Execute the nim command on the Master. 
 *	Look at the stdout from the remote command and if any present 
 *	execute it. 
 *
 * DATA STRUCTURES:
 *   parameters:
 *	argc	number of cmdline args
 *	argv	the args
 *
 * ----------------------------------------------------------------------- */

#include "cmdnim.h"

extern int disp_mntpnt;

do_nim( int argc, char *argv[], int *rc )
{

struct	stat	stat_sout; 
	FILE	*fp_sout; 
	FILE	*outptr = NULL;
	char	c_sout[MAX_TMP]; 	

	int	argcnt=1;

	char	pull_em[MAX_TMP]; 
	char	*Args[METH_MAX_ARGS];

	char 	*chmod_args[] = { CHMOD, "755", c_sout, NULL };
	char 	*ksh_args[] = { KSH, "-c", c_sout, NULL };
	char 	*send_failure[] = { NIMCLIENT, "-R", "failure", NULL };

	*rc = 0;
	Args[0] = NIM;

	/* 
	 * Add the pull request attribute.
	 * (Note: leave this attribute as the first thing on the nim command
	 *  line.  It is important for nim to know this attribute as early as 
         *  possible for certain error message handling.)
	 */ 
	sprintf(pull_em, "-a%s=Yes", ATTR_PULL_REQUEST_T ); 
	Args[argcnt++] = pull_em ; 

	/*
	 * copy the argv pointers
	 */ 
	for ( ; argcnt != (argc +1); argcnt++)
		Args[argcnt] = argv[argcnt-1]; 

	/*
	 * Add our nim name
	 */ 
	Args[argcnt++] = niminfo.nim_name ; 

	if ( disp_mntpnt )
	{
		/* pass this program's stdout FILE so output gets displayed */
		outptr = (FILE *) stdout;
	}
	else
	{
		/*
		 * Build tmp file for the sdtout (if any) 
		 */ 
		sprintf( c_sout, "%s/nc.%ld", niminfo.tmps, time(NULL) ); 
		if ( (fp_sout=fopen( c_sout, "w") ) == NULL) {
			export_errstr( ERR_ERRNO, c_sout, NULL, NULL );
			return(FAILURE);
		}
		/* 
		 * Make sure we'll be able to exec it later... 
		 */ 
		if ( (client_exec(rc, NULL, chmod_args ) != SUCCESS) ||
			( *rc != 0 ) )
			return( FAILURE ); 

		outptr = fp_sout;
	}

	/* 
	 * Call the master... 
	 */ 
	if ( (To_Master(ATTR_MASTER_T,rc,&outptr,argcnt, Args) != SUCCESS) ||
		( *rc != 0 ) )
		return( FAILURE ); 

	if ( ! disp_mntpnt )
	{
		fclose(fp_sout);
	
		/*
		 * Test if anything written to file ? 
		 */ 
		if ( stat(c_sout, &stat_sout) != 0 )
			ERROR( ERR_SYS, c_sout, NULL, NULL );
	
		if ( stat_sout.st_size > 1 )
		{
			/* 
		 	 * Execute the output from the remote command. 
		 	 */ 
			if ( client_exec(rc, NULL, ksh_args ) == FAILURE )
				/* falied due to problem in client_exec */
				nim_error( 0, NULL, NULL, NULL );
			else if ( *rc != 0 )
				/* command which we executed failed: display info */
				/* NOTE that because we pass ATTR_PULL_REQUEST, the */
				/*		nim_script gets built with a call to update the */
				/*		result from the script (ie, don't do it here) */
				nim_error( ERR_METHOD, niminfo.nim_name, niminfo.errstr, NULL );
		}
	}
	return( SUCCESS );	
} 
