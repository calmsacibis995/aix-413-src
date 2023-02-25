static char sccsid[] = "@(#)25        1.13  src/tcpip/usr/bin/tftp/aix_tftp.c, tcpfilexfr, tcpip411, GOLD410 3/8/94 16:23:06";
/* 
 * COMPONENT_NAME: TCPIP aix_tftp.c
 * 
 * FUNCTIONS: MSGSTR, aix_tftp, aix_usage
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * aix_tftp.c - command line compatibility mode for tftp/utftp
 *	provides old interface to the stock 4.3 BSD tftp
 *
 * if invoked by "utftp" it performs automatic overwrite
 *
 *
 * There are now a few extensions to 
 * syntax: -o means to overwrite an existing file on a read; and 
 * "-" may be used as the local file name to mean the standard 
 * {input|output}. 
 */ 
 
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/stat.h>

#include	<netdb.h> 
#include	<sys/socket.h> 
#include	<netinet/in.h> 
#include	<arpa/tftp.h>
#include	<termio.h>
 
#include <nl_types.h>
#include "tftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTP,n,s) 

 
/*
 * externs and global variables
 */
extern setmode();			/* sets xfer mode */
extern struct sockaddr_in sin;		/* address of foreign host */

extern char mode[];			/* xfer mode (text string) */
extern char hostname[];			/* name of foreign host */
extern int connected;			/* 1 if foreign host specified */
extern short port;			/* port for tftp service */
 
int file_stdin = 0;                     /* set = 1 if stdin is used */
struct termio out, in, deftio;          /* define termio if input from stdin */
 
#define SEND	0			/* read/write mode flags */
#define RECV	1
 
aix_tftp( argc, argv )
int	argc; 
char	**argv; 
{ 
	int	dir;			/* connection direction */ 
	char	*c_mode;		/* conn. mode string */ 
	struct	stat	stb;		/* file status buffer */ 
	int	ovrw = FALSE;		/* overwrite existing file? */ 
	int	len, fd, rc=0;
	register char	*arg;		/* arg pointer */ 
	struct hostent *host;
	 

	if (argc < 5 || argc > 6 || argv[1][0] != '-') { 
		aix_usage( argv[0] );
	} 
	 
	for (arg = &argv[1][1]; *arg != NULL; arg++) 
		switch (*arg) { 
		case 'r': 
		case 'g': 
			dir = RECV; 
			break; 
		case 'o': 
			ovrw = TRUE; 
			dir = RECV; 
			break; 
		case 'w': 
		case 'p': 
			dir = SEND; 
			break; 
		default: 
			aix_usage( argv[0] );
		} 
 
	/*
	 * get and set the transfer mode, from command line or default.
	 */
	if (argc > 5) { 
		c_mode = argv[5]; 
		if( (strcmp (c_mode, "netascii") != 0) &&
#ifdef NEVER /* well no mail yet */
		    (strcmp (c_mode, "mail") != 0) &&
#endif NEVER /* XXX NYD MAIL not done TBD */
		    (strcmp (c_mode, "image") != 0) ) {
			aix_usage( argv[0] );
		    }		
		    if(strcmp (c_mode, "image") == 0) {
			strcpy(c_mode, "octet");
			}
	} else { 
		c_mode = "netascii"; 
	} 
	(void)setmode( c_mode );
 

	/*
	 * get the foreign host, by name or by number
	 * the address is filled into "sin" which is a global
	 */
	if (!isinet_addr(argv[3])) {
		if ((host = gethostbyname (argv[3])) == (struct hostent *) 0) {
                        fprintf(stderr,MSGSTR(NME_NT_FND,
                                          "tftp: Unknown host %s\n"), argv[3]);
			connected = 0;
                        exit(1);
                }
		sin.sin_family = host->h_addrtype;
		bcopy(host->h_addr, &sin.sin_addr, host->h_length);
		strcpy(hostname, host->h_name);
	} else {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(argv[3]);
		if (sin.sin_addr.s_addr == -1) {
			connected = 0;
			fprintf (stderr, MSGSTR(UNKNOWN_HOST, 
				"tftp: Address %s misformed\n"), argv[3]); 
			exit(1); 
		}
		strcpy(hostname, argv[3]);
	} 
	sin.sin_port = port;	/* port set in main() */
	connected = 1;

	 
	/*
	 * if reading a file, and the destination exists and is not
	 * standard out, either: (utftp) print an error msg, or
	 * (tftp) ask to unlink it.
	 */
	if (dir == RECV && !ovrw && stat (argv[2], &stb) >= 0 && 
	     strcmp (argv[2], "-") != 0) {
		if ((arg = strrchr(argv[0], '/')) == NULL) {
			arg = &argv[0][0];
		} else {
			arg++;
		}
                /* Now to see if argv[2] is a directory or a file */
                if ((stb.st_mode & S_IFMT) == S_IFDIR) {
                        fprintf (stderr, MSGSTR(DIR_EXISTS, "Cannot overwrite directory: %s\n"), argv[2]);
                        exit(1);
                }

		if (*arg == 'u') {
			fprintf (stderr, MSGSTR(FILE_EXISTS, "File already exists; use '-o' to overwrite\n"));  /*MSG*/
			exit(1); 
		} else {
			char reply_line[BUFSIZ];

			fprintf (stderr, MSGSTR(FILE_EXT_UNLINK, "File already exists; unlink? ")); /*MSG*/
			(void) gets(reply_line);
			if (rpmatch(reply_line)) {
				if (unlink (argv[2]) < 0) {
					fprintf (stderr, MSGSTR(CANT_UNLINK, "Can't unlink; use '-o'\n")); /*MSG*/
					exit(1);
                                }
                        } else {
				exit(1);
			}
		}
	} 
 

	if( dir == RECV ) { 
		/*
		 * for read, we open the local file, then call BSD tftp to
		 * receive the file.
		 */
		if (strcmp ( argv[2], "-") == 0) 
			fd = dup(fileno (stdout));
		else if ((fd = open( argv[2],O_WRONLY | O_CREAT | O_TRUNC,
								0644)) < 0) { 
			fprintf (stderr, 
			MSGSTR(LOCAL_FILE_ERR, "Local file error\nCode = 2\nCan't open local file\n"));  /*MSG*/
			return (1); 
		} 
		/* fd of sink, source file name, xfr mode */
		rc = recvfile( fd, argv[4], mode , argv[2] );

#ifdef NEVER	/* need to clear up a bit - BSD never cared much for tftp */
XX		succ = tftp_read (cn, argv[2]); 
XX		if (!succ) 
XX			unlink (argv[2]); 
#endif NEVER
	}
	else {	/* is WRITE */
		/*
		 * for write, we open the local file, then call BSD tftp to
		 * send the file.
		 */
		if( strcmp( argv[2], "-") == 0) { 
		   /*
		    * writing from stdin:
		    *	make sure we are dealing with a TTY before we start
		    *	believing TTY i/o controls work.
		    *	(the TTY code is wrong also, but thats another problem)
		    */
                   if (ioctl(0, TCGETA, &in) != -1) {
			   file_stdin = 1;
			   ioctl(0, TCGETA, &deftio);
			   in.c_cc[VEOF] = 1;
			   in.c_cc[VEOL] = 1;
			   in.c_lflag = 070;
			   ioctl(0, TCSETAF, &in);
		   }
		   if ((fd = dup(fileno(stdin))) < 0) {
			fprintf (stderr, 
			MSGSTR(LOCAL_FILE_ERR1, "Local file error\nCode = 1\nCan't open stdin\n") );  /*MSG*/
			return (1); 
		   } 
		} else {
		   if ((fd = open( argv[2], O_RDONLY )) < 0 ) { 
			fprintf (stderr, 
			MSGSTR(LOCAL_FILE_ERR2, "Local file error\nCode = 1\nCan't open local file %s\n"), argv[2] );  /*MSG*/
			return (1); 
		   } 
		}
		/* fd of source, target name, mode */
		rc = sendfile( fd, argv[4], mode );
	}
	 
	/* if we timeout, we won't come back here, we longjump back to main */
	return( rc ); 
} 
 
aix_usage( who )
char	*who;
{
	fprintf(stderr, MSGSTR(USAGE_7, "usage: %s {-r|-g|-o|-w|-p} <local file> <host> <foreign file> [netascii|image]\n"), who ); /*MSG*/
	exit(1); 
}
 
