static char sccsid[] = "@(#)25	1.13  src/bos/usr/sbin/install/inuumsg/inuumsg.c, cmdinstl, bos411, 9428A410j 2/11/94 15:13:12";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: main (inuumsg)
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

/*
 * INCLUDE FILES
 */
#include <locale.h>             /* for call setlocale */
#include <stdio.h>
#include <nl_types.h>

#include "inuumsg_msg.h"

main(argc, argv)
/*
 * NAME: inuumsg
 *
 * FUNCTION: Issue a message from the message catalog made up of
 *		LPP owner-requested messages for their instal and update
 *		procedures.
 *
 * EXECUTION ENVIRONMENT: User command.
 *
 * RETURNS: None.
 *
 */
int argc;
char *argv[];
{
#include "def_msg.h"			/* default messages	*/
  int	  msgnum;
  char    *ptr;
  nl_catd cat_id;
  char    err_def_msg[100];
  
  setlocale(LC_ALL,"");
  cat_id = catopen(MF_INUUMSG,NL_CAT_LOCALE);

  if (argc == 1)
    exit (1);

  /* get message number from command line */
  msgnum = (long) atoi(argv[1]);

  if (msgnum >= (sizeof(def_msg) / sizeof(def_msg[0])) ||
      msgnum <= 0) {
	ptr = def_msg[INU_MSG_NOT_FOUND];
        fprintf(stderr,catgets(cat_id,MS_INUUMSG,INU_MSG_NOT_FOUND, ptr), 
				msgnum);
        exit (1);
  }

  ptr = def_msg[msgnum];

  fprintf(stderr,catgets(cat_id,MS_INUUMSG,msgnum, ptr),
                         argv[2],argv[3],argv[4],argv[5]);

  exit(0);
}
