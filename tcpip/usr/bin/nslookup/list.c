static char sccsid[] = "@(#)62	1.2  src/tcpip/usr/bin/nslookup/list.c, tcpras, tcpip411, GOLD410 6/3/93 11:38:18";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: Finger
 *		ListHost_close
 *		ListHosts
 *		ListHostsByType
 *		ListSubr
 *		MSGSTR
 *		PrintListInfo
 *		ViewList
 *		defined
 *		strip_domain
 *
 *   ORIGINS: 26 27 65
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 *
 * OSF/1 1.1 snapshot 4
 */ 
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "list.c,v $ $Revision: 1.7.2.4 (OSF) $Date: 1992/03/25 16:04:52";
#endif
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 * list.c	5.20 (Berkeley) 6/1/90
 */

/*
 *******************************************************************************
 *
 *  list.c --
 *
 *	Routines to obtain info from name and finger servers.
 *
 *	Adapted from 4.3BSD BIND ns_init.c and from finger.c
 *
 *******************************************************************************
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include "res.h"

#include <nl_types.h>
#include <locale.h>
#include "nslookup_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_NSLOOKUP,Num,Str)
extern  nl_catd catd;           /* message catalog file descriptor */

/*
 *  Imported from res_debug.c
 */
extern char *_res_resultcodes[];

extern int errno;

typedef union {
    HEADER qb1;
    char qb2[PACKETSZ];
} querybuf;

extern HostInfo *defaultPtr;
extern HostInfo curHostInfo;
extern int      curHostValid;
extern int      queryType;
extern int      queryClass;

static int sockFD = -1;
static int ListSubr();

/*
 *  During a listing to a file, hash marks are printed 
 *  every HASH_SIZE records.
 */

#define HASH_SIZE 50


/*
 *******************************************************************************
 *
 *  ListHosts --
 *  ListHostsByType --
 *
 *	Requests the name server to do a zone transfer so we
 *	find out what hosts it knows about.
 *
 *      For ListHosts, there are five types of output:
 *      - Internet addresses (default)
 *      - cpu type and operating system (-h option)
 *      - canonical and alias names  (-a option)
 *      - well-known service names  (-s option)
 *      - ALL records (-d option)
 *      ListHostsByType prints records of the default type or of a speicific
 *      type.
 *
 *      To see all types of information sorted by name, do the following:
 *        ls -d domain.edu > file
 *        view file
 *
 *  Results:
 *	SUCCESS		the listing was successful.
 *	ERROR		the server could not be contacted because 
 *			a socket could not be obtained or an error
 *			occured while receiving, or the output file
 *			could not be opened.
 *
 *******************************************************************************
 */

void
ListHostsByType(string, putToFile)
        char *string;
        int putToFile;
{
        int     i, qtype, result;
        char    *namePtr;
        char    name[NAME_LEN];
        char    option[NAME_LEN];

        /*
         *  Parse the command line. It maybe of the form "ls -t domain"
         *  or "ls -t type domain".
         */

        i = sscanf(string, " ls -t %s %s", option, name);
        if (putToFile && i == 2 && name[0] == '>') {
            i--;
	}
        if (i == 2) {
            qtype = StringToType(option, -1);
            if (qtype == -1)
                return;
            namePtr = name;
        } else if (i == 1) {
            namePtr = option;
            qtype = queryType;
        } else {
            fprintf(stderr, "*** ls: invalid request %s\n",string);
            return;
        }
        result = ListSubr(qtype, namePtr, putToFile ? string : NULL);
        if (result != SUCCESS)
            fprintf(stderr, "*** Can't list domain %s: %s\n",
                        namePtr, DecodeError(result));
}

void
ListHosts(string, putToFile)
    char *string;
    int  putToFile;
{
	int     i, qtype, result;
        char    *namePtr;
        char    name[NAME_LEN];
        char    option[NAME_LEN];

    /*
     *  Parse the command line. It maybe of the form "ls domain",
     *  "ls -X domain".
     */ 
	i = sscanf(string, MSGSTR( LS, " ls %s %s"), option, name);
	if (putToFile && i == 2 && name[0] == '>') {
	    i--;
	}
	if (i == 2) {
	    if (strcmp( MSGSTR( OPT_A, "-a"), option) == 0) {
		qtype = T_CNAME;
	    } else if (strcmp(MSGSTR(OPT_H, "-h"), option) == 0) {
		qtype = T_HINFO;
	    } else if (strcmp(MSGSTR(OPT_M, "-m"), option) == 0) {
		qtype = T_MX;
	    } else if (strcmp(MSGSTR(OPT_S, "-s"), option) == 0) {
		qtype = T_WKS;
	    } else if (strcmp(MSGSTR(OPT_D, "-d"), option) == 0) {
		qtype = T_ANY;
	    } else {
		qtype = T_A;
	    }
	    namePtr = name;
	} else if (i == 1) {
	    namePtr = option;
	    qtype = T_A;
	} else {
	    fprintf(stderr, MSGSTR( LIST_HOST, 
		"ListHosts: invalid request %s\n"), string);
	    return;
	}

	result = ListSubr(qtype, namePtr, putToFile ? string : NULL);
        if (result != SUCCESS)
            fprintf(stderr, "*** Can't list domain %s: %s\n",
                        namePtr, DecodeError(result));
}

static int
ListSubr(qtype, domain, cmd)
        int qtype;
        char *domain;
        char *cmd;
{
        querybuf                buf;
        struct sockaddr_in      sin;
        HEADER                  *headerPtr;
        int                     msglen;
        int                     amtToRead;
        int                     numRead;
        int                     numAnswers = 0;
        int                     result;
        int                     soacnt = 0;
        u_short                 len;
        char                    *cp, *nmp;
        char                    dname[2][NAME_LEN];
        char                    file[NAME_LEN];
        static char             *answer = NULL;
	static int              answerLen = 0;
        enum {
            NO_ERRORS,
            ERR_READING_LEN,
            ERR_READING_MSG,
            ERR_PRINTING
        } error = NO_ERRORS;

	/*
	 *  Create a query packet for the requested domain name.
	 *
	 */
	msglen = res_mkquery(QUERY, domain, queryClass, T_AXFR,
                                (char *)0, 0, (struct rrec *)0,
                                (char *) &buf, sizeof(buf));
	if (msglen < 0) {
	    if (_res.options & RES_DEBUG) {
		fprintf(stderr, MSGSTR( LH_MKQ, 
			"ListHosts: Res_mkquery failed\n"));
	    }
	    return (ERROR);
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family	= AF_INET;
	sin.sin_port	=  htons(nsport);

	/*
	 *  Check to see if we have the address of the server or the
	 *  address of a server who knows about this domain.
	 *       
	 *  For now, just use the first address in the list.
	 */

	if (defaultPtr->addrList != NULL) {
	  sin.sin_addr = *(struct in_addr *) defaultPtr->addrList[0];
	} else {
	  sin.sin_addr = *(struct in_addr *)defaultPtr->servers[0]->addrList[0];
	}

	/*
	 *  Set up a virtual circuit to the server.
	 */
	if ((sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    perror(MSGSTR( LIST_HOSTS, "ListHosts"));
	    return(ERROR);
	}	
	if (connect(sockFD, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
	    int e;
            if (errno == ECONNREFUSED) {
                e = NO_RESPONSE;
            } else {
                perror("ls: connect");
                e = ERROR;
            }
            (void) close(sockFD);
            sockFD = -1;
            return e;
	}	

	/*
	 * Send length & message for zone transfer 
	 */

        len = htons(msglen);

        if (write(sockFD, (char *)&len, sizeof(len)) != sizeof(len) ||
            write(sockFD, (char *) &buf, msglen) != msglen) {
		perror(MSGSTR( LIST_HOSTS, "ListHosts"));
		(void) close(sockFD);
		sockFD = -1;
		return(ERROR);
	}

	fprintf(stdout, MSGSTR( FORMAT4, "[%s]\n"),
		(defaultPtr->addrList != NULL) ? defaultPtr->name : 
		 defaultPtr->servers[0]->name);

	if (cmd == NULL) {
	    filePtr = stdout;
	} else {
	    filePtr = OpenFile(cmd, file);
            if (filePtr == NULL) {
                fprintf(stderr, MSGSTR( ERR_WRITE, 
			"*** Can't open %s for writing\n"), file);
		(void) close(sockFD);
		sockFD = -1;
                return(ERROR);
            }
	    fprintf(filePtr, MSGSTR( R_ARROW1, "> %s\n"), cmd);
	    fprintf(filePtr, MSGSTR( FORMAT4, "[%s]\n"),
		(defaultPtr->addrList != NULL) ? defaultPtr->name : 
		 defaultPtr->servers[0]->name);
	}

#if 0
	
	if (qtype == T_CNAME) {
            fprintf(filePtr, MSGSTR( FORMAT1, "%-30s"), 
		MSGSTR( ALIAS_NAME, "Alias"));
        } else {
		fprintf(filePtr, MSGSTR(FORMAT1, "%-30s"), MSGSTR( HOST_DOM, 
		"Host or domain name"));
	}
	switch(qtype) {
	    case T_A:
		    fprintf(filePtr, MSGSTR( FORMAT5, " %-30s\n"), 
			MSGSTR( INTER_ADDR, "Internet address"));
		    break;
	    case T_HINFO:
		    fprintf(filePtr, MSGSTR( FORMAT6, " %-10s %s\n"), 
			MSGSTR( CPU, "CPU"), MSGSTR( OS, "OS"));
		    break;
	    case T_CNAME:
		    fprintf(filePtr, MSGSTR( FORMAT5, " %-30s\n"), 
			MSGSTR( ALIAS_NAME, "Alias"));
		    break;
	    case T_MX:
		    fprintf(filePtr, MSGSTR( FORMAT7, " %3s %s\n"), 
			MSGSTR( METRIC, "Metric"), MSGSTR( HOST, "Host"));
		    break;
	    case T_WKS:
		    fprintf(filePtr, MSGSTR( FORMAT8, " %-4s %s\n"), 
			MSGSTR( PROTOCOL, "Protocol"),  
			MSGSTR( SERVICES, "Services"));
	    case T_MB:
                    fprintf(filePtr, " %-30s\n", "Mailbox");
                    break;
            case T_MG:
                    fprintf(filePtr, " %-30s\n", "Mail Group");
                    break;
            case T_MR:
                    fprintf(filePtr, " %-30s\n", "Mail Rename");
                    break;
            case T_MINFO:
                    fprintf(filePtr, " %-30s\n", "Mail List Requests & Errors");
                    break;
            case T_UINFO:
                    fprintf(filePtr, " %-30s\n", "User Information");
                    break;
	    case T_UID:
                    fprintf(filePtr, " %-30s\n", "User ID");
                    break;
            case T_GID:
                    fprintf(filePtr, " %-30s\n", "Group ID");
                    break;
            case T_TXT:
                    fprintf(filePtr, " %-30s\n", "Text");
                    break;
            case T_NS:
                    fprintf(filePtr, " %-30s\n", "Name Servers");
                    break;
            case T_PTR:
                    fprintf(filePtr, " %-30s\n", "Pointers");
                    break;
            case T_SOA:
                    fprintf(filePtr, " %-30s\n", "Start of Authority");
                    break;
	    case T_ANY:
		    fprintf(filePtr, MSGSTR( FORMAT5, " %-30s\n"), 
			MSGSTR( T_ANY_RES, "Resource record info"));
		    break;
	}

#endif


	dname[0][0] = '\0';
	while (1) {
	    unsigned short tmp;

	    /*
	     * Read the length of the response.
	     */

	    cp = (char *) &tmp;
	    amtToRead = sizeof(u_short);
	    while(amtToRead > 0 && (numRead = read(sockFD, cp, amtToRead)) > 0){
		cp 	  += numRead;
		amtToRead -= numRead;
	    }
	    if (numRead <= 0) {
		error = ERR_READING_LEN;
		break;
	    }	

	    if ((len = htons(tmp)) == 0) {
		break;	/* nothing left to read */
	    }
	
	    /*
             * The server sent too much data to fit the existing buffer --
             * allocate a new one.
             */
            if (len > answerLen) {
                if (answerLen != 0) {
                    free(answer);
                }
                answerLen = len;
     		answer = Malloc(answerLen);
            }

	    /*
	     * Read the response.
	     */

	    amtToRead = len;
	    cp = answer;
	    while(amtToRead > 0 && (numRead = read(sockFD, cp, amtToRead)) > 0){
		cp += numRead;
		amtToRead -= numRead;
	    }
	    if (numRead <= 0) {
		error = ERR_READING_MSG;
		break;
	    }

	    result = PrintListInfo(filePtr, answer, cp, qtype, dname[0]);
	    if (result != SUCCESS) {
		error = ERR_PRINTING;
		break;
	    }

	    numAnswers++;
	    if (cmd != NULL && ((numAnswers % HASH_SIZE) == 0)) {
		fprintf(stdout, MSGSTR( POUND, "#"));
		fflush(stdout);
	    }
	    cp = answer + sizeof(HEADER);
            if (ntohs(((HEADER* )answer)->qdcount) > 0)
                cp += dn_skipname(cp, answer + len) + QFIXEDSZ;
	    nmp = cp;
	    cp += dn_skipname(cp, (u_char *)answer + len);
	    if ((_getshort((uchar *)cp) == T_SOA)) {
		(void) dn_expand((uchar *)answer, (uchar *)answer + len,
			(uchar *)nmp, (uchar *)dname[soacnt], sizeof(dname[0]));
	        if (soacnt) {
		    if (strcmp(dname[0], dname[1]) == 0)
			break;
		} else
		    soacnt++;
	    }
	}

	if (cmd != NULL) {
	    fprintf(stdout, MSGSTR( RECORD_REC, "%sReceived %d record%s.\n"), 
		(numAnswers >= HASH_SIZE) ? 
			MSGSTR( NEWLINE, "\n") : MSGSTR( NULL_STRG, ""),
		numAnswers,
		(numAnswers > 1) ? MSGSTR( S, "s") : MSGSTR( NULL_STRG, ""));
	}

	(void) close(sockFD);
	sockFD = -1;
	if (cmd != NULL && filePtr != NULL) {
	    fclose(filePtr);
	    filePtr = NULL;
	}

	switch (error) {
	    case NO_ERRORS:
		return (SUCCESS);

	    case ERR_READING_LEN:
		return(ERROR);

	    case ERR_PRINTING:
		return(result);

	    case ERR_READING_MSG:
		headerPtr = (HEADER *) &buf;
		fprintf(stderr, MSGSTR( ERR_READING, 
			"ListHosts: error receiving zone transfer:\n"));
		fprintf(stderr, MSGSTR( ERR_READING1, 
			"  result: %s, answers = %d, authority = %d, additional = %d\n"), 
		    	_res_resultcodes[headerPtr->rcode], 
		    	ntohs(headerPtr->ancount), ntohs(headerPtr->nscount), 
			ntohs(headerPtr->arcount));
		return(ERROR);
	    default:
		return(ERROR);
	}
}


/*
 *******************************************************************************
 *
 *  PrintListInfo --
 *
 * 	Used by the ListInfo routine to print the answer 
 *	received from the name server. Only the desired 
 *	information is printed.
 *
 *  Results:
 *	SUCCESS		the answer was printed without a problem.
 *	NO_INFO		the answer packet did not contain an answer.
 *	ERROR		the answer was malformed.
 *      Misc. errors	returned in the packet header.
 *
 *******************************************************************************
 */

#define NAME_FORMAT " %-30s"

static Boolean
strip_domain(string, domain)
    char *string, *domain;
{
    register char *dot;

    if (*domain != '\0') {
        dot = string;
	while ((dot = strchr(dot, '.')) != NULL && strcasecmp(domain, ++dot))
                ;
        if (dot != NULL) {
            dot[-1] = '\0';
            return TRUE;
        }
    }
    return FALSE;
}

PrintListInfo(file, msg, eom, qtype, domain)
    FILE 	*file;
    char 	*msg, *eom;
    int 	qtype;
    char	*domain;
{
    register char 	*cp;
    HEADER 		*headerPtr;
    int 		type, class, dlen, nameLen;
    u_long		ttl;
    int 		n, pref;
    struct in_addr 	inaddr;
    char 		name[NAME_LEN];
    char 		name2[NAME_LEN];
    Boolean		stripped;

    /*
     * Read the header fields.
     */
    headerPtr = (HEADER *)msg;
    cp = msg + sizeof(HEADER);
    if (headerPtr->rcode != NOERROR) {
	return(headerPtr->rcode);
    }

    /*
     *  We are looking for info from answer resource records.
     *  If there aren't any, return with an error. We assume
     *  there aren't any question records.
     */

    if (ntohs(headerPtr->ancount) == 0) {
	return(NO_INFO);
    } else {
	if (ntohs(headerPtr->qdcount) > 0) {
	    nameLen = dn_skipname(cp, eom);
	    if (nameLen < 0)
		return (ERROR);
	    cp += nameLen + QFIXEDSZ;
	}
	if ((nameLen = dn_expand((uchar *)msg, (uchar *)eom, (uchar *)cp,
				 (uchar *)name, sizeof(name))) < 0) {
	    return (ERROR);
	}
	cp += nameLen;

        type = _getshort((uchar *)cp);
        cp += sizeof(u_short);

        if (!(type == qtype || qtype == T_ANY) &&
            !((type == T_NS || type == T_PTR) && qtype == T_A))
                return(SUCCESS);

	class = _getshort((uchar *)cp);
	cp += sizeof(u_short);
	ttl = _getlong((uchar *)cp);
	cp += sizeof(u_long);
	dlen = _getshort((uchar *)cp);
	cp += sizeof(u_short);

	if (name[0] == 0)
		strcpy(name, MSGSTR( ROOT, "(root)"));

	/* Strip the domain name from the data, if desired. */
        stripped = FALSE;
        if ((_res.options & RES_DEBUG) == 0) {
            if (type != T_SOA) {
                stripped = strip_domain(name, domain);
            }
        }
        if (!stripped && nameLen < sizeof(name)-1) {
            strcat(name, ".");
        }

	fprintf(file, NAME_FORMAT, name);

        if (qtype == T_ANY) {
            if (_res.options & RES_DEBUG) {
                fprintf(file,"\t%lu %-5s", ttl, p_class(queryClass));
            }
            fprintf(file," %-5s", p_type(type));
        }

        /* XXX merge this into debug.c's print routines */

	switch (type) {

	    case T_A:

		if (class == C_IN) {
                    bcopy(cp, (char *)&inaddr, sizeof(inaddr));
                    if (dlen == 4) {
			fprintf(file, MSGSTR( FORMAT10, " %s"), 
				inet_ntoa(inaddr));
                    } else if (dlen == 7) {
			fprintf(file, MSGSTR( FORMAT10, " %s"), 
				inet_ntoa(inaddr));
			fprintf(file, MSGSTR( FORMAT11, " (%d, %d)"), cp[4],(cp[5] << 8) + cp[6]);
                    } else
			fprintf(file, MSGSTR( DLEN, " (dlen = %d?)"), dlen);
                }
                break;
		
	    case T_CNAME:
		case T_MB:
            	case T_MG:
            	case T_MR:
               		if ((nameLen =
                        	dn_expand((uchar *)msg, (uchar *)eom,
					  (uchar *)cp, (uchar *)name2,
					  sizeof(name2))) < 0) {
		    		fprintf(file, MSGSTR( STARS, " ***\n"));
                    		return (ERROR);
                	}
                	fprintf(file, " %s", name2);
                	break;
	    case T_NS:
            case T_PTR:
                putc(' ', file);
                if (qtype != T_ANY)
                    fprintf(file,"%s = ", type == T_PTR ? "host" : "server");
                cp = Print_cdname2(cp, msg, eom, file);
                break;
		
	    case T_HINFO:
		if (n = *cp++) {
		    (void)sprintf(name, MSGSTR( FORMAT14, "%.*s"), n, cp);
		    fprintf(file, MSGSTR( FORMAT15, " %-10s"), name);
                    cp += n;
                } else {
		    fprintf(file, MSGSTR( FORMAT15, " %-10s"), 
			MSGSTR( SPACE, " "));
                }
                if (n = *cp++) {
		    fprintf(file, MSGSTR( FORMAT16, " %.*s"), n, cp);
                    cp += n;
                }
                break;


	    case T_SOA:
                if ((nameLen =
                        dn_expand((uchar *)msg, (uchar *)eom, (uchar *)cp,
				  (uchar *)name2, sizeof(name2))) < 0) {
                    fprintf(file, " ***\n");
                    return (ERROR);
                }
                cp += nameLen;
                fprintf(file, " %s", name2);
                if ((nameLen =
                        dn_expand((uchar *)msg, (uchar *)eom, (uchar *)cp,
				  (uchar *)name2, sizeof(name2))) < 0) {
                    fprintf(file, " ***\n");
                    return (ERROR);
                }
                cp += nameLen;
                fprintf(file, " %s. (", name2);
                for (n = 0; n < 5; n++) {
			u_long u;

                    u = _getlong((uchar *)cp);
                    cp += sizeof(u_long);
                    fprintf(file,"%s%lu", n? " " : "", u);
                }
                fprintf(file, ")");
                break;

	    case T_MX:
		pref = _getshort((uchar *)cp);
                cp += sizeof(u_short);
		fprintf(file, MSGSTR( FORMAT18, " %-3d "),pref);
                if ((nameLen =
                        dn_expand((uchar *)msg, (uchar *)eom, (uchar *)cp,
				  (uchar *)name2, sizeof(name2))) < 0) {
		    fprintf(file, MSGSTR( STARS, " ***\n"));
                    return (ERROR);
                }
		fprintf(file, MSGSTR( FORMAT10, " %s"), name2);
                break;

	    case T_TXT:
                {
                    char *cp2 = cp + dlen;
                    int c;

                    (void) fputs(" \"", file);
                    while (cp < cp2) {
                            if (n = (unsigned char) *cp++) {
                                    for (c = n; c > 0 && cp < cp2; c--)
                                            if (*cp == '\n') {
                                                (void) putc('\\', file);
                                                (void) putc(*cp++, file);
                                            } else
                                                (void) putc(*cp++, file);
                            }
                    }
                    (void) putc('"', file);
                }
                break;
	    
	    case T_MINFO:
                (void) putc(' ', file);
                cp = Print_cdname(cp, msg, eom, file);
                fprintf(file, "  ");
                cp = Print_cdname(cp, msg, eom, file);
                break;

            case T_UINFO:
                fprintf(file, " %s", cp);
                break;

            case T_UID:
            case T_GID:
                fprintf(file, " %lu", _getlong((uchar *)cp));
                break;
	    case T_WKS:
                if (class == C_IN) {
                    struct protoent *pp;
                    struct servent *ss;
                    u_short port;

                    cp += 4;    /* skip inet address */
                    dlen -= 4;

                    setprotoent(1);
                    setservent(1);
                    n = *cp & 0377;
                    pp = getprotobynumber(n);
                    if (pp == 0)
                        fprintf(file," %-3d ", n);
                    else
                        fprintf(file," %-3s ", pp->p_name);
                    cp++; dlen--;

		    port = 0;
                    while (dlen-- > 0) {
                        n = *cp++;
                        do {
			    /* if a service is specified then print it out */
                            if (n & 0200) {
                                ss = getservbyport((int)htons(port),
                                            pp?pp->p_name:(char *)pp);
                                if (ss == 0)
                                    fprintf(file," %u", port);
                                else
                                    fprintf(file," %s", ss->s_name);
                            }
                                n <<= 1;
                        } while (++port & 07); /* while port is a WKS port */
                    }
                    endprotoent();
                    endservent();
                }
                break;
        }
	fprintf(file,"\n");
    }
    return(SUCCESS);
}


/*
 *******************************************************************************
 *
 *  ViewList --
 *
 *	A hack to view the output of the ls command in sorted
 *	order using more.
 *
 *******************************************************************************
 */

ViewList(string)
    char *string;
{
    char file[NAME_LEN];
    char command[NAME_LEN];

    sscanf(string, MSGSTR( VIEW, " view %s"), file);
    (void)sprintf(command, MSGSTR( GREP_CMD, 
	"grep \"^ \" %s | sort | more"), file);
    system(command);
}

/*
 *******************************************************************************
 *
 *   Finger --
 *
 *	Connects with the finger server for the current host
 *	to request info on the specified person (long form)
 *	who is on the system (short form).
 *
 *  Results:
 *	SUCCESS		the finger server was contacted.
 *	ERROR		the server could not be contacted because 
 *			a socket could not be obtained or connected 
 *			to or the service could not be found.
 *
 *******************************************************************************
 */

Finger(string, putToFile)
    char *string;
    int  putToFile;
{
	struct servent 		*sp;
	struct sockaddr_in 	sin;
	register FILE 		*f;
	register int 		c;
	register int 		lastc;
	char 			name[NAME_LEN];
	char 			file[NAME_LEN];

	/*
	 *  We need a valid current host info to get an inet address.
	 */
	if (!curHostValid) {
	    fprintf(stderr, MSGSTR( NO_CHOST, 
		"Finger: no current host defined.\n"));
	    return (ERROR);
	}

	if (sscanf(string, MSGSTR( FINGER, " finger %s"), name) == 1) {
	    if (putToFile && (name[0] == '>')) {
		name[0] = '\0';
	    }
	} else {
	    name[0] = '\0';
	}

	sp = getservbyname( MSGSTR( FING, "finger"),  MSGSTR( TCP, "tcp"));
	if (sp == 0) {
	    fprintf(stderr, MSGSTR( FING_UNKNOWN, 
		"Finger: unknown service\n"));
	    return (ERROR);
	}

	bzero((char *)&sin, sizeof(sin));
	sin.sin_family	= curHostInfo.addrType;
	sin.sin_port	= sp->s_port;
	bcopy(curHostInfo.addrList[0], (char *)&sin.sin_addr, 
		curHostInfo.addrLen);

	/*
	 *  Set up a virtual circuit to the host.
	 */

	sockFD = socket(curHostInfo.addrType, SOCK_STREAM, 0);
	if (sockFD < 0) {
	    fflush(stdout);
	    perror(MSGSTR( FING, "Finger"));
	    return (ERROR);
	}

	if (connect(sockFD, (struct sockaddr *)&sin, sizeof (sin)) < 0) {
	    fflush(stdout);
	    perror(MSGSTR( FING, "Finger"));
	    close(sockFD);
	    sockFD = -1;
	    return (ERROR);
	}

	if (!putToFile) {
	    filePtr = stdout;
	} else {
	    filePtr = OpenFile(string, file);
	    if (filePtr == NULL) {
		fprintf(stderr, MSGSTR( ERR_WRITE,
			"*** Can't open %s for writing\n"), file);
		close(sockFD);
		sockFD = -1;
		return(ERROR);
	    }
	    fprintf(filePtr, MSGSTR( R_ARROW1, "> %s\n"), string);
	}
	fprintf(filePtr, MSGSTR(FORMAT4, "[%s]\n"), curHostInfo.name);

	if (name[0] != '\0') {
	    write(sockFD, MSGSTR( CAP_W, MSGSTR( CAP_W, "/W ")), 3);
	}
	write(sockFD, name, strlen(name));
	write(sockFD, MSGSTR( FORMAT26, "\r\n"), 2);
	f = fdopen(sockFD, MSGSTR( SMALL_R, "r"));
	while ((c = getc(f)) != EOF) {
	    switch(c) {
		case 0210:	/* check for back space */
		case 0211:	/* check for back tab */
		case 0212:	/* check for line feed */
		case 0214:	/* check for from feed */
			c -= 0200; /* strip of high order bit */
			break;
		case 0215:	/* check for carriage control */
			c = '\n'; /* convert to new line */
			break;
	    }
	    putc(lastc = c, filePtr);
	}
	if (lastc != '\n') {
	    putc('\n', filePtr);
	}
	putc('\n', filePtr);

	close(sockFD);
	sockFD = -1;

	if (putToFile) {
	    fclose(filePtr);
	    filePtr = NULL;
	}
	return (SUCCESS);
}

ListHost_close()
{
    if (sockFD != -1) {
        (void) close(sockFD);
        sockFD = -1;
    }
}
