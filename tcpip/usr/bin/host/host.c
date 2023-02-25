static char sccsid[] = "@(#)59        1.12  src/tcpip/usr/bin/host/host.c, tcpadmin, tcpip411, GOLD410 2/14/94 14:12:21";
/* 
 * COMPONENT_NAME: TCPIP host.c
 * 
 * FUNCTIONS: MSGSTR, Mhost, usage 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * host.c - get the name and address of a host
 *
 * TO RUN:
 *	host hostname
 *
 *		where "hostname" is the name of a host
 *
 * TO COMPILE:
 *	cc -o host host.c /usr/lib/libsock.a
 *		( or whatever the socket library is these days )
 *
 */

#include "host_msg.h" 
#include <nl_types.h>
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HOST,n,s) 

#include <stdio.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

extern int errno;
struct hostent *gethostbyname();
struct hostent *gethostbyaddr();
long	inet_addr();
extern int isinet_addr();

char	*myname;		/* for usage */
char	*nullstring = "";
char	*astericks = "*************************************";
#include <locale.h>

main( argc, argv, envp )
int	argc;
char	*argv[];
char	*envp[];
{
	register char *name;
	register struct hostent *hp;
	long	addr;
	int	status;
	char    *c;

	setlocale(LC_ALL,"");
	catd = catopen(MF_HOST,NL_CAT_LOCALE);

	myname = argv[0];

	if( argc != 2 )
		usage (myname); /*MSG*/

	name = argv[1];

	/* see if it's a name or an address */
	if (!isinet_addr(name)) {
		if ((hp = gethostbyname (name)) == (struct hostent *) 0) {
			fprintf(stderr,MSGSTR(NME_NT_FND, 
                                            "host: name %s NOT FOUND\n"), name);
			exit(1);
	   	}
	}
	else {
		if( (addr = inet_addr( name )) == -1 ) {
			fprintf(stderr,MSGSTR(BAD_ADDR, 
                                        "host: address %s mis-formed\n"), name);
			exit(1);
		}
		if( (hp = gethostbyaddr( &addr, sizeof(addr), AF_INET )) ==
		    (struct hostent *)0 ) {
			fprintf(stderr,MSGSTR(ADDR_NT_FND, 
                                         "host: address %s NOT FOUND\n"), name);
			exit(1);
		}
	}

	/* print the info */
	{
	register char **cp;
	int	i;
	char	comma;
	extern char *inet_ntoa();

	printf( MSGSTR(ALIASES, "%s is %s"), hp->h_name, /*MSG*/
	    inet_ntoa(*(struct in_addr *)hp->h_addr) );
	if( *(hp->h_aliases) != NULL) {
	    printf( MSGSTR(ALIAS_TITLE, ",  Aliases:")); /*MSG*/
	    comma = ' ';
	    i = 10;
	    for (cp = hp->h_aliases; cp && *cp && **cp; cp++) {
		i += strlen(*cp) + 2;
		if (i > 75) {
		    printf( "\n\t");
		    comma = ' ';
		    i = 10;
		}
		printf(MSGSTR(PRNT_ALIAS, "%c %s"), comma, *cp); /*MSG*/
		comma = ',';
	    }
	}
	printf("\n");
	}

	exit( 0 );
}

/*
 * usage - print usage message
 */
usage( arg )
char *arg;
{
	printf(MSGSTR(USAGE, "    usage: host hostname\n")); /*MSG*/
	exit(1);
}

/* end of host.c */
