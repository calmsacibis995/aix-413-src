static char sccsid [] = "@(#)66	1.6  src/bos/usr/sbin/mvdir/mvdir.c, cmdfiles, bos411, 9428A410j 11/16/93 08:33:11";
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: mvdir
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ctype.h>
#include <locale.h>
#include "mvdir_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_MVDIR,Num,Str)

int     Errors = 0;
unsigned char *path = NULL;
unsigned char *pointer;
struct  stat statb;

/*
 * NAME: mvdir source target
 * 
 * FUNCTION: Moves directories or files.
 * 
 */
main(argc,argv)
int argc;
char **argv;
{
     (void) setlocale (LC_ALL,"");
     catd = catopen(MF_MVDIR, NL_CAT_LOCALE);

     if(argc != 3) {
	  fprintf(stderr, MSGSTR(USAGE,"Usage: mvdir fromdir newname\n"));
	  exit(1);
     }

  /*
   * Allocate sufficient space for target path.
   */
     if ((path = (unsigned char *) malloc (strlen(argv[2]) + strlen(argv[1])
       + 2)) == NULL) {
	  perror("mvdir");
	  ++Errors;
	  exit(1);
     }

  /*
   * If target exists and is a directory,
   * set target path equal to target/source.
   * Otherwise, set path equal to target.
   */
     if ((stat(argv[2], &statb) == 0) &&
	((statb.st_mode & S_IFMT) == S_IFDIR)) {
	  strcpy (path, argv[2]);
	  strcat (path, "/");
	  if (pointer = strrchr (argv[1], '/')) {
	       strcat (path, (pointer + 1));
	  } else {
	       strcat (path, argv[1]);
	  }
     } else {
	  strcpy (path, argv[2]);
     }
     if ((rename(argv[1], path)) != 0) {
	  perror("mvdir");
	  ++Errors;
     }
     exit(Errors? 2: 0);
}
