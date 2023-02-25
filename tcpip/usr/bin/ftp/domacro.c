static char sccsid[] = "@(#)76	1.5  src/tcpip/usr/bin/ftp/domacro.c, tcpfilexfr, tcpip411, GOLD410 3/25/91 14:35:39";
/* 
 * COMPONENT_NAME: TCPIP domacro.c
 * 
 * FUNCTIONS: MSGSTR, domacro 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "ftp_var.h"

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ttychars.h>

#include <nl_types.h>
#include "ftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTP,n,s) 

domacro(argc, argv)
	int argc;
	char *argv[];
{
	register int i, j;
	register char *cp1, *cp2;
	int count = 2, loopflg = 0;
	char line2[200];
	extern char **glob(), *globerr;
	struct cmd *getcmd(), *c;
	extern struct cmd cmdtab[];

	if (argc < 2) {
		(void) strcat(line, " ");
		printf(MSGSTR(MACRO_NAME, "(macro name) ")); /*MSG*/
		(void) gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		printf(MSGSTR(USAGE_MACRO, "Usage: %s macro_name.\n"), argv[0]); /*MSG*/
		code = -1;
		return;
	}
	for (i = 0; i < macnum; ++i) {
		if (!strncmp(argv[1], macros[i].mac_name, 9)) {
			break;
		}
	}
	if (i == macnum) {
		printf(MSGSTR(MACRO_NFOUND, "'%s' macro not found.\n"), argv[1]); /*MSG*/
		code = -1;
		return;
	}
	(void) strcpy(line2, line);
top_of_loop:
	cp1 = macros[i].mac_start;
	while (cp1 != macros[i].mac_end) {
		while (isspace(*cp1)) {
			cp1++;
		}
		cp2 = line;
		while (*cp1 != '\0') {
		      switch(*cp1) {
		   	    case '\\':
				 *cp2++ = *++cp1;
				 break;
			    case '$':
				 if (isdigit(*(cp1+1))) {
				    j = 0;
				    while (isdigit(*++cp1)) {
					  j = 10*j +  *cp1 - '0';
				    }
				    cp1--;
				    if (argc - 2 >= j) {
					(void) strcpy(cp2, argv[j+1]);
					cp2 += strlen(argv[j+1]);
				    }
				    break;
				 }
				 if (*(cp1+1) == 'i') {
					loopflg = 1;
					cp1++;
					if (count < argc) {
					   (void) strcpy(cp2, argv[count]);
					   cp2 += strlen(argv[count]);
					}
					break;
				}
				/* intentional drop through */
			    default:
				*cp2++ = *cp1;
				break;
		      }
		      if (*cp1 != '\0') {
			 cp1++;
		      }
		}
		*cp2 = '\0';
		makeargv();
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n")); /*MSG*/
			code = -1;
		}
		else if (c == 0) {
			printf(MSGSTR(INVALID, "?Invalid command\n")); /*MSG*/
			code = -1;
		}
		else if (c->c_conn && !connected) {
			printf(MSGSTR(NOT_CONN, "Not connected.\n")); /*MSG*/
			code = -1;
		}
		else {
			if (verbose) {
				printf("%s\n",line);
			}
			(*c->c_handler)(margc, margv);
			if (bell && c->c_bell) {
/*				(void) putchar(CTRL(g)); */
/* Temporary change to get around compiler problem. mvs. */
				(void) putchar('g'&037); 
			}
			(void) strcpy(line, line2);
			makeargv();
			argc = margc;
			argv = margv;
		}
		if (cp1 != macros[i].mac_end) {
			cp1++;
		}
	}
	if (loopflg && ++count < argc) {
		goto top_of_loop;
	}
}
