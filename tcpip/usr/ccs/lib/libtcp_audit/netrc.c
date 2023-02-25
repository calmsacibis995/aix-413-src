static char sccsid[] = "@(#)66        1.4  src/tcpip/usr/ccs/lib/libtcp_audit/netrc.c, tcpip, tcpip411, GOLD410 3/3/90 17:21:27";
/* 
 * COMPONENT_NAME: TCPIP netrc.c
 * 
 * FUNCTIONS: is_netrc_restricted 
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Used to restrict the usage of $HOME/.netrc file if a "tcpip"
 * stanza exists in CONFIG_FILE with the proper attributes.
 * Example:
 *		tcpip:
 *			netrc = ftp,rexec
 *
 *
 * Got rid of the old stanza routines so we are no longer dependent
 * on them.
 *
*/

#ifdef _CSECURITY

#include <stdio.h>
#define CONFIG_FILE "/etc/security/config"
char word[128];

is_netrc_restricted(program)
char *program;
{
	FILE *config_file;
	int got_tcpip = 0;
	char buf[512];
	char *p, *q;
	char *strchr();

	if (program == NULL)
		return(0);
	if ((config_file = fopen(CONFIG_FILE, "r")) == NULL)
		return(0);
	while(!got_tcpip && (p = fgets(buf,sizeof(buf),config_file)) != NULL) {
		rd_nxt_wd(p);
		if (strcmp(word, "tcpip:") == 0)
			got_tcpip++;
		else
			continue;
	}
	if (got_tcpip) {
		while ((p = fgets(buf, sizeof(buf), config_file)) != NULL) {
			rd_nxt_wd(p);
			if (strcmp(word, "netrc") == 0) {
				p = strchr(p, '=');
				if (p != NULL) {
					rd_nxt_wd(++p);
				   	q = word;
					for (;;) {
					   p = strchr(q, ',');
					   if (p == NULL) {
					      if (strcmp(q, program) == 0)
							return(1);
					      else
							return(0);
					   } else {
					      *p = NULL;
					      if (strcmp(q, program) == 0)
							return(1);
					   }
					   q = ++p;
					}
				} else
					return(0);
			}
		}
	}
	return(0);
}
/*
 * rd_nxt_wd() - read the next work on the line.
 *
 */

rd_nxt_wd(s)
char *s;
{
	int i = 0;

	/* Eat up leading white spaces */
	while (isspace(*s))
		s++;
	/* Copy the word into the word array */
	while (isprint(*s) && (*s != '\n') && (*s != ' ')) {
		word[i++] = *s;
		s++;
	}
	word[i] = NULL;
}
#endif _CSECURITY
