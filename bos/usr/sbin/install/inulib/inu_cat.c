static char sccsid[] = "@(#)59  1.8  src/bos/usr/sbin/install/inulib/inu_cat.c, cmdinstl, bos411, 9428A410j 5/4/94 11:19:32";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_cat (inulib)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include <stdio.h>
#include "inulib.h"

/*-----------------------------------------------------------------------
 * NAME: inu_cat
 *
 * FUNCTION:
 *	Provide file I/O function similar to cat(1) and cp(1).
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * NOTES:
 *	Copies contents of file1 to file2.  If file1 is "-"
 *	then use stdin, if file2 is "-" then use stdout.
 *	If file2 is "&" then use stderr.
 *
 * ON ENTRY:
 *	infile	pointer to string with name of input file.
 *	outfile	pointer to string with name of output file.
 *
 * RETURNS:
 *	SUCCESS 	if the file was cat'ed.
 *	FAILURE 	if either file couldn't be opened.
 *----------------------------------------------------------------------*/

int inu_cat (char *infile, 
             char *outfile,
             char *mode)     /* access mode for opening outfile */
{
  FILE	*fp1;		/* file pointer to input file	*/
  FILE	*fp2;		/* file pointer to output file	*/
  char  buf[4096];		/* character			*/
  int   i;

  int	closefp1;	/* flag meaning we didn't use stdin		*/
  int	closefp2;	/* flag meaning we didn't use stdout or stderr	*/

  closefp1 = TRUE;	/* close files if not stdin, stdout, or stderr	*/
  closefp2 = TRUE;	/* (i.e. only close them if we opened them)	*/

  /* Determine infile */
  if (strcmp (infile, "-") == 0)           /* if infile is stdin */
  {
     fp1 = stdin;
     closefp1 = FALSE;
  }
  else                                    /* if infile is filename */
  {
     fp1 = fopen (infile, "r");
  }

  /* Determine outfile */
  if (strcmp (outfile, "-") == 0)         /* if outfile is stdout */
  {
     fp2 = stdout;
     closefp2 = FALSE;
  }
  else if (strcmp (outfile, "&") == 0)    /* if outfile is stderr */
  {
     fp2 = stderr;
     closefp2 = FALSE;
  }
  else                                    /* if outfile is filename */
  { 
     fp2 = fopen (outfile, mode);
  }

  /* if error opening either file */
  if ((fp1 == NULL) || (fp2 == NULL))
  {
     return (FAILURE);
  }
  else
  {
     i = fread (buf,sizeof(char),sizeof(buf),fp1);
     /* just put out each char, regardless of */
     while (i > 0)   /* whether it is NLS or not              */
     {
        fwrite (buf,sizeof(char),i,fp2);
        i = fread (buf,sizeof(char),sizeof(buf),fp1);
     }

     if (closefp1) 
        (void) fclose(fp1);
     if (closefp2) 
        (void) fclose(fp2);

     return(SUCCESS);
  }
}
