static char sccsid[] = "@(#)37	1.9.1.3  src/bos/usr/sbin/mkdev/cfglib.c, cmdcfg, bos411, 9428A410j 11/12/93 16:03:39";
/*
 * COMPONENT_NAME: (CMDDEV)  Device specific config support cmds
 *
 * FUNCTIONS:  args_from, compound_opts, maxcat, add_attr, stat_str,
 *		catstr, attr_str, NLS_attr, do_error, meth_error
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/priv.h>
#include <sys/audit.h>
#include "hlcmds.h"
#include <nl_types.h>

#define DBG	0



/*****************************************************************************/
/*				START OF CODE				     */
/*****************************************************************************/


/*------------------------------- args_from   --------------------------------
 * NAME: args_from
 *                                                                    
 * FUNCTION : read a line from the specified file, chops it into seperate
 *		arguments, then calls the specified parsing routine to
 *		process the line
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
 static int nested_calls=0;

args_from( filename, parse, cmd )
char *filename;				/* name of file to read from */
int (*parse)();				/* ptr to parsing routine */
char *cmd;				/* command name */
{  FILE *fp;				/* file ptr */
   int argc;				/* temp argc */
   char **argv;				/* array of args */
   extern int optind;			/* from getopt */
   int tmp_optind;			/* temp optind */
   int i;				/* loop indexing */

	if (nested_calls++) return;  /* prohibit nested calls */

	/* open the file */
	if ((fp = fopen(filename,"r")) == NULL)
	   do_error( ERRSTR(CFG_OPEN_ERR), cmd,
	             filename,NULL, EXIT_FATAL, "DEV_Create" );

	/* alloc space */
	argv = malloc( 10 * sizeof(char *) );
	
	/* command name is arg[0] */
	argv[0] = malloc( strlen(cmd) + 1 );
	strcpy( argv[0], cmd );

	/* read each arg from the file & put into argv */
	argc = 1;
	do
	{  if ((argc % 10) == 0)
	   {  /* allocate more space for parameter pointers */
	      if ((argv = realloc(argv,(argc+10) * sizeof(char *))) == NULL)
		 do_error(ERRSTR(CFG_MALLOC_FAILED), cmd,
			  NULL, NULL, EXIT_FATAL,"Dev_Create");
	   }
	}
	while( getnextarg (fp,&argv[argc++]) != EOF);
    argc--;
    fclose( fp );

	/* NOTE - since the parse routines use getopt & this is a */
	/*      recursive call, we must save the current optind */
	tmp_optind = optind;
	optind = 1;

	/* call the parse routine */
	(*parse)( argc, argv );

	/* restore the original optind */
	optind = tmp_optind;

/* decrement the counter on the nesting level */
	nested_calls--;
}


/*------------------------------- maxcat      --------------------------------
 * NAME:maxcat
 *                                                                    
 * FUNCTION : appends s2 to s1 if the length of s1+s2 is less than max
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	0 - if not enough room to append s2 to s1
 *	1 - if s2 successfully appended
 *	   
 *---------------------------------------------------------------------------*/
maxcat( s1, s2, max )
char *s1;		/* string to be appended to */
char *s2;		/* string to append */
int max;		/* maximum size of s1 */
{ int i,j;		/* temp variables */

	/* is there enough room to append s2 ? */
	if ( ((i=strlen(s1)) + strlen(s2)) >= (max-1) )
	   return( FALSE ); /* not enough room */

	/* append s2 up to max */
	for (j=0; (i < max-1) && (s2[j] != '\0'); i++, j++)
	   s1[i] = s2[j];

	s1[i] = '\0';

	/* return 1 - there is enough room */
	return( TRUE );
}


/*------------------------------- stat_str    --------------------------------
 * NAME:stat_str
 *                                                                    
 * FUNCTION: returns the string corresponding to the specified status code
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
char *stat_str( status, string )
short status;		/* status code */	
char *string;		/* corresonding string */
{
	switch (status)
	{ case DEFINED    : strcpy( string,
						 MSGSTR(CFG_MSG_DEFINED));
				       break;
	  case AVAILABLE  : strcpy( string,
						 MSGSTR(CFG_MSG_AVAILABLE));
				       break;
	  case STOPPED    : strcpy( string,
						 MSGSTR(CFG_MSG_STOPPED) );
				       break;
	  default		     : strcpy( string,
						 MSGSTR(CFG_MSG_UNKNOWN) );
				       break;
	}

	return( string );
}


/*------------------------------- catstr      --------------------------------
 * NAME: catstr
 *                                                                    
 * FUNCTION: appends str2 to str1; if there isn't enough room in str1, str1
 *		is enlarged
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
#define BUFFER_SIZE	1024
catstr( str1, str2, max )
char **str1;			/* string buffer */
char *str2;			/* string to append */ 
int *max;			/* max chars in str1 */
{ int total;

	/* has any memory been allocated yet??? */
	if (*max == 0)
	{  /* allocate the initial buffer */
	   *str1 = (char *)malloc(BUFFER_SIZE);
	   **str1 = '\0';
	   *max = BUFFER_SIZE;
	}

	/* is there enough room??? */
	total = strlen(*str1) + strlen(str2);
	if (total >= *max)
	{  /* not enough room - allocate a larger buffer */
	   while (total >= *max)
	      *max += BUFFER_SIZE;
	   *str1 = (char *)realloc( *str1, *max );
	}

#if 0
	if ((strlen(*str1)+strlen(str2)) >= *max)
	{  /* not enough room - allocate another BUFFER_SIZE */
	   *max += BUFFER_SIZE;
	   *str1 = (char *)realloc( *str1, *max );
	}
#endif

	/* append str2 to the end of str1 */
	strcat( *str1, str2 );
}


/*------------------------------- NLS_attr    --------------------------------
 * NAME:NLS_attr
 *                                                                    
 * FUNCTION: convert the string of attribute names into a string of NLS
 *		attribute names
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	pointer to a string of NLS names
 *	   
 *---------------------------------------------------------------------------*/
char *NLS_attr( names )
char *names;				      /* str of attribute names to convert */
{ 
	char              *ptr;				        /* temp string ptr         */
	char              crit[MAX_CRITELEM_LEN];  	/* search criteria         */
	struct PdAt       *pdat;			  		/* list of PdAt structs    */
	struct listinfo   pdat_info;	  			/* PdAt listinfo           */
	struct PdDv       *pddv;			  		/* PdDv struct             */
	struct listinfo   pddv_info;	  			                           
	char              *nls = NULL;			  	/* ptr to the NLS string   */
	int               max = 0;				  	/* max char in nls         */
									
    setlocale(LC_ALL, "");
	ptr  = names;

	/* find the NLS text for each attribute name in the string */
    while (*ptr != '\0')
	{  
		if ((ptr = strchr(ptr,' ')) != NULL){
			*ptr = '\0';
			ptr++;
		}

		/* find the PdAt name */
		sprintf( crit, "attribute = \'%s\'", names );
		if((pdat = (struct PdAt *)
			odm_get_list(PdAt_CLASS,crit,&pdat_info,1,1)) == NULL)
			pdat_info.num = 0;

		if (pdat_info.num){
			/* get the corresponding PdDv entry */
			sprintf( crit, "uniquetype = \'%s\'", pdat->uniquetype );
			pddv = (struct PdDv *)
			odm_get_list( PdDv_CLASS, crit, &pddv_info, 1, 1);
			if (pddv == NULL )
				pddv_info.num = 0;

			if (pddv_info.num){
				catstr( &nls,
					    catgets( catopen( (int)pddv->catalog, NL_CAT_LOCALE ),
                                 (int)pddv->setno,
                                 (int)pdat->nls_index,   
                                 names                                
                        ),
                        &max                                              
                );
				odm_free_list( pddv, &pddv_info );
			}
			else
				catstr( &nls, names, &max );

			odm_free_list( pdat, &pdat_info );
		}
		else
			catstr( &nls, names, &max );

		catstr( &nls, " ", &max );

		names = ptr;
	}
    return( nls );
}


/*------------------------------- do_error    --------------------------------
 * NAME:do_error
 *                                                                    
 * FUNCTION: write the specified error code & exit
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
do_error( fmt, routine, param, usage, exit_status, cmd )
char *fmt;		/* string to print: format */
char *routine;		/* string to print: command name */
char *param;		/* string to print: parameter if any */
char *usage;		/* string to print: usage if any */
int exit_status;	/* exit status */
char *cmd;		/* high level command name */
{
	char *temp;
	int len;

	len = (fmt?strlen(fmt):0) + (routine?strlen(routine):0) +
		 (param?strlen(param):0) + 1;

	temp = (char *)malloc( len );

	sprintf( temp, fmt?fmt:"", routine?routine:"", param?param:"" );
	fprintf( stderr, temp );
	if(usage)
		fprintf(stderr,usage);

	/* auditing */
	privilege(PRIV_ACQUIRE);
	auditlog( cmd, -1, temp, strlen(temp)+1 );
	privilege(PRIV_LAPSE);

	free( (void *) temp );

	if (exit_status != EXIT_NOT)
	{  /* close the ODM - just in case */
	   odm_terminate();

	   /* bye-bye */
	   exit( exit_status ); 
	}
}


/*------------------------------- meth_error  --------------------------------
 * NAME:meth_error
 *                                                                    
 * FUNCTION: process's the stderr output from a method; expecting the first
 *		field to be an error code, which may (optional) be followed
 *		by error text
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES: 
 *
 * DATA STRUCTURES: 
 *
 * RETURNS: 
 *	   
 *---------------------------------------------------------------------------*/
meth_error( error_code, err_ptr, exit_status, cmd, routine )
int error_code;			/* error code returned from method */
char *err_ptr;			/* ptr to buffer containing the stderr output*/
int exit_status;		/* status to exit with */
char *cmd;			/* hlcmd name specified by security */
char *routine;			/* routine name */
{
	/* was there an error ???? */
	if ((error_code < 0) || (error_code > E_LAST_ERROR))
	   error_code = E_LAST_ERROR;
	else if ((error_code == 0) || (error_code == E_FINDCHILD))
	   return( 0 );

	/* write & log the error */
	fprintf(stderr,ERRSTR(CFG_METHERR),routine);
	do_error( METHERR(error_code), NULL, NULL, NULL, EXIT_NOT, cmd );

	/* write out stderr from method */
	fprintf(stderr,"%s\n",err_ptr);

	/* exit if we need to */
	if (exit_status == EXIT_NOT)
	   return( TRUE );
	else
	{  /* close the ODM - just in case */
	   odm_terminate();

	   /* bye-bye */
	   exit( exit_status ); 
	}
}



/*------------------------------- getnextarg   --------------------------------
* NAME: getnextarg
*
* FUNCTION : reads a string from an input stream. Newlines are expected
* 	   in the input stream. Strings are not read across newlines.
*	   Quoted strings will be read as a single string
*		Lines with only a newline are skipped
*
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* DATA STRUCTURES:
*
* RETURNS:
*	 -1 if end of file or if the input line has greater length than LINEMAX 
*	  0 if string is read 
*
*---------------------------------------------------------------------------*/

#define LINEMAX  512
/* these static arguments are manintained for repeated calls */
static char fbuffer[LINEMAX+1]= {0}; /* contains 1 line read from file */
static char argbuffer[LINEMAX+1];	 /* contains 1 string read from fbuffer */
static argindex = 0;				 /* index into arbuffer */
static char *fch=fbuffer;			 /* pointer into fbuffer */

int getnextarg (fp,dest)
FILE *fp;  			/* already open input file stream */
char **dest;			/* data area to copy string to */
{	int rc=0;		/* return code */
	char *rptr,*ch; 	/* string pointer */
	int start;		/* index to start of buffer to copy */

/* clear variables */
		argindex = 0;
		memset(argbuffer,0,LINEMAX+1);

/* copy next non-white string from input line buffer to argument buffer */
		rc = fetch_string(fch,argbuffer,LINEMAX);

		if (rc == EOF)  /* End of current line buffer? */
		{
/* read the next non-empty line from the file into the argbuffer*/

			while ((rptr = fgets(fbuffer,LINEMAX,fp)) != NULL)
			{
/* keep reading past lines of all white space or lines with only a newline*/	
			   if (strlen(fbuffer) > 1)
				{
				  ch=fbuffer;
				  start = 0;
				  while ((*ch == ' ' || *ch == '\t') && start++ < LINEMAX) ch++;
				  if (start < LINEMAX && *ch != '\n') break;  
				}
			}

		 	if (rptr == NULL)	
				return(EOF);  /* End of FILE ? */
			else 
			{ 
/* Is there a newline - else the line is too long */
/* This protects against reading too far into a binary file */
				if ((rptr = strrchr(fbuffer,'\n')) == NULL)
				{
					/* need to report some kind of error */
					return(EOF);
				}

			/* Now try again to extract one parameter from the non-empty line
				just read from the file into argbuffer */
			fch = fbuffer;
			rc = fetch_string(fch,argbuffer,LINEMAX);
			if (rc == EOF)
				return(EOF);
			}
		
		}

/* Now we need to check if quotes are enclosing a multi-field parameter */

			argindex = strlen(argbuffer);
			fch += rc;
			start = 0;
/* do we have quotes? */
			if (argbuffer[0] == '"' || argbuffer[0] == '\'' )
			{				
			/* then build the string up to closing quotes */
				while( (rc = *fch++) != '\0' &&
						rc != argbuffer[0] && rc != '\n' && argindex < LINEMAX) 
							argbuffer[argindex++] = rc;
				start = 1; /* don't copy in the closing quote itself*/
			}
/* malloc space for the pointer and copy to the destination address */
			*dest = malloc(strlen(argbuffer) + 1);
			strcpy(*dest,&argbuffer[start]);
			return(0);
}

/*------------------------------ fetch_string   --------------------------------
* NAME: fetch_string
*
* FUNCTION :  Copies the next non-white space substring from src
*			  to dest, looking at most at maxlen characers.
*
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* DATA STRUCTURES:
*
* RETURNS:
*    -1 if the src string is null or contains only white space
*     else returns the number of characters copied
*
*---------------------------------------------------------------------------*/

static int fetch_string(src,dest,maxlen)
char *src;			/* input string */
char *dest;			/* output string */
int maxlen;			/* maximum buffer length */
{
int count=0;		/* number of chars copied */

/* is it a empty string ? */
	if ( strlen(src) == 0) return(EOF);

/* remove leading white space */
	while ((*src == ' ' || *src == '\t')
		&& (*src != '\0' && *src != '\n')  && ++count <= maxlen) src++;		

/* Do we have a line of white space only? */
	if (*src == '\0' || *src == '\n') return(EOF);

/* copy non-white charcters */
	while (*src != ' ' && *src != '\t' && *src != '\n'
			 && *src != '\0'  && ++count < maxlen) 
		*dest++ = *src++;

	return(count);	
}
