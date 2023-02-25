static char sccsid[] = "@(#)60	1.2  src/tcpip/usr/bin/nslookup/debug.c, tcpras, tcpip411, GOLD410 6/3/93 11:36:34";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: Fprint_query
 *		MSGSTR
 *		Print_cdname
 *		Print_cdname2
 *		Print_cdname_sub
 *		Print_query
 *		Print_rr
 *		defined
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
static char rcsid[] = "debug.c,v Revision: 1.7 (OSF) $Date: 1991/01/07 13:19:14 ";
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
 * is provided ``as is'' without express or implied warranty. 6/29/90";
 */

/*
 *******************************************************************************
 *
 *  debug.c --
 *
 *	Routines to print out packets received from a name server query.
 *
 *      Modified version of 4.3BSD BIND res_debug.c 5.30 6/27/90
 *
 *******************************************************************************
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <netdb.h>
#include <time.h>
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
extern char *_res_opcodes[];

/*
 *  Used to highlight the start of a record when printing it.
 */
#define INDENT "    ->  "



/*
 * Print the contents of a query.
 * This is intended to be primarily a debugging routine.
 */

Print_query(msg, eom, printHeader)
	char *msg, *eom;
	int printHeader;
{
	Fprint_query(msg, eom, printHeader,stdout);
}

Fprint_query(msg, eom, printHeader,file)
	char *msg, *eom;
	int printHeader;
	FILE *file;
{
	register char *cp;
	register HEADER *hp;
	register int n;
	short class;
	short type;

	/*
	 * Print header fields.
	 */
	hp = (HEADER *)msg;
	cp = msg + sizeof(HEADER);
	if (printHeader || (_res.options & RES_DEBUG2)) {
	    fprintf(file, MSGSTR(HEADER1, "    HEADER:\n"));
	    fprintf(file, MSGSTR(HEADER2, "\topcode = %s"), _res_opcodes[hp->opcode]);
	    fprintf(file, MSGSTR(HEADER3, ", id = %d"), ntohs(hp->id));
	    fprintf(file, MSGSTR(HEADER4, ", rcode = %s\n"), _res_resultcodes[hp->rcode]);
	    fprintf(file, MSGSTR(HEADER5, "\theader flags: "));
	    if (hp->qr) {
		    fprintf(file, MSGSTR(HEADER6, " response"));
	    } else {
		    fprintf(file, MSGSTR(HEADER7, " query"));
	    }
	    if (hp->aa)
		    fprintf(file, MSGSTR(HEADER8, ", auth. answer"));
	    if (hp->tc)
		    fprintf(file, MSGSTR(HEADER9, ", truncation"));
	    if (hp->rd)
		    fprintf(file, MSGSTR(HEADER10, ", want recursion"));
	    if (hp->ra)
		    fprintf(file, MSGSTR(HEADER11, ", recursion avail."));
	    if (hp->pr)
		    fprintf(file, MSGSTR(HEADER12, ", primary"));
	    fprintf(file, MSGSTR(HEADER13, "\n\tquestions = %d"), ntohs(hp->qdcount));
	    fprintf(file, MSGSTR(HEADER14, ",  answers = %d"), ntohs(hp->ancount));
	    fprintf(file, MSGSTR(HEADER15, ",  auth. records = %d"), ntohs(hp->nscount));
	    fprintf(file, MSGSTR(HEADER16, ",  additional = %d\n\n"), ntohs(hp->arcount));
	}

	/*
	 * Print question records.
	 */
	if (n = ntohs(hp->qdcount)) {
		fprintf(file, MSGSTR(QUEST1, "    QUESTIONS:\n"));
		while (--n >= 0) {
			fprintf(file, MSGSTR( TAB, "\t"));
			cp = Print_cdname(cp, msg, eom, file);
			if (cp == (char *)NULL)
				return;
			type = _getshort((uchar *)cp);
			cp += sizeof(u_short);
			class = _getshort((uchar *)cp);
			cp += sizeof(u_short);
			fprintf(file, MSGSTR(QUEST3, 
				", type = %s"), p_type(type));
			fprintf(file, MSGSTR(QUEST4, 
				", class = %s\n"), p_class(class));
		}
	}
	/*
	 * Print authoritative answer records
	 */
	if (n = ntohs(hp->ancount)) {
		fprintf(file, MSGSTR( ANSWERS, "    ANSWERS:\n"));
		while (--n >= 0) {
			fprintf(file, INDENT);
			cp = Print_rr(cp, msg, eom, file);
			if (cp == (char *)NULL)
				return;
		}
	}
	/*
	 * print name server records
	 */
	if (n = ntohs(hp->nscount)) {
		fprintf(file, MSGSTR(AUTH, "    AUTHORITY RECORDS:\n"));
		while (--n >= 0) {
			fprintf(file, INDENT);
			cp = Print_rr(cp, msg, eom, file);
			if (cp == (char *)NULL)
				return;
		}
	}
	/*
	 * print additional records
	 */
	if (n = ntohs(hp->arcount)) {
		fprintf(file, MSGSTR( ADD_REC, "    ADDITIONAL RECORDS:\n"));
		while (--n >= 0) {
			fprintf(file, INDENT);
			cp = Print_rr(cp, msg, eom, file);
			if (cp == (char *)NULL)
				return;
		}
	}
	fprintf(file, MSGSTR( LINE, "\n------------\n"));

}


char *
Print_cdname_sub(cp, msg, eom, file, format)
	char *cp, *msg, *eom;
	FILE *file;
	int format;
{
	int n;
	char name[MAXDNAME];
	extern char *strcpy();

	if ((n = dn_expand((uchar *)msg, (uchar *)eom, (uchar *)cp,
			   (uchar *)name, sizeof(name))) < 0)
		return ((char *)NULL);
	if (name[0] == '\0') {
	    (void) strcpy(name, "(root)");
	}
	if (format) {
	    fprintf(file, MSGSTR(FORMAT1, "%-30s"), name);
	} else {
	    fputs(name, file);
	}
	return (cp + n);
}

char *
Print_cdname(cp, msg, eom, file)
	char *cp, *msg, *eom;
	FILE *file;
{
    return(Print_cdname_sub(cp, msg, eom, file, 0));
}

char *
Print_cdname2(cp, msg, eom, file)
	char *cp, *msg, *eom;
	FILE *file;
{
    return(Print_cdname_sub(cp, msg, eom, file, 1));
}

/*
 * Print resource record fields in human readable form.
 */
char *
Print_rr(cp, msg, eom, file)
	char *cp, *msg, *eom;
	FILE *file;
{
	int type, class, dlen, n, c;
	time_t rrttl, ttl;
	struct in_addr inaddr;
	char *cp1, *cp2;
        int debug;

	if ((cp = (char *)Print_cdname(cp, msg, eom, file)) == NULL) {
		fprintf(file, MSGSTR( NAME_TRUNC, "(name truncated?)\n"));
		return ((char *)NULL);			/* compression error */
	}

	type = _getshort((uchar *)cp);
	cp += sizeof(u_short);
	class = _getshort((uchar *)cp);
	cp += sizeof(u_short);
	rrttl = (time_t) _getlong((uchar *)cp);
	cp += sizeof(u_long);
	dlen = _getshort((uchar *)cp);
	cp += sizeof(u_short);

	debug = _res.options & (RES_DEBUG|RES_DEBUG2);
        if (debug) {
            if (_res.options & RES_DEBUG2) {
	    	fprintf(file, MSGSTR( RES_DEBUG2_OPT, 
                "\n\ttype = %s, class = %s, dlen = %d"),
                            p_type(type), p_class(class), dlen);
            }
            if (type == T_SOA) {
                fprintf(file,"\n\tttl = %lu", rrttl);
            }
	    fprintf(file, MSGSTR( NEWLINE, "\n"));
        }

	cp1 = cp;

	/*
	 * Print type specific data, if appropriate
	 */
	switch (type) {
	case T_A:
		switch (class) {
		case C_IN:
		case C_HS:
			bcopy(cp, (char *)&inaddr, sizeof(inaddr));
			if (dlen == 4) {
				fprintf(file, MSGSTR( INET_ADDR, 
					"\tinet address = %s\n"),
					inet_ntoa(inaddr));
				cp += dlen;
			} else if (dlen == 7) {
				fprintf(file, MSGSTR( INET_ADDR,
					"\tinet address = %s"),
					inet_ntoa(inaddr));
				fprintf(file, MSGSTR( PROTO, 
					", protocol = %d"), cp[4]);
				fprintf(file, MSGSTR( PORT, 
					", port = %d\n"),
					(cp[5] << 8) + cp[6]);
				cp += dlen;
			}
			break;
		default:
			fprintf(file, MSGSTR( T_DEFAULT, 
				"\taddress, class = %d, len = %d\n"),
			    class, dlen);
			cp += dlen;
		}
		break;

	case T_CNAME:
		fprintf(file, MSGSTR( T_CANON_NME, "\tcanonical name = "));
		goto doname;

	case T_MG:
		fprintf(file, MSGSTR( T_MG_OPT, "\tmail group member = "));
		goto doname;

	case T_MB:
		fprintf(file, MSGSTR( T_MB_OPT, "\tmail box = "));
		goto doname;

	case T_MR:
		fprintf(file, MSGSTR( T_MR_OPT, "\tmailbox rename = "));
		goto doname;

	case T_MX:
		fprintf(file, MSGSTR( T_MX_OPT, 
			"\tpreference = %d"),_getshort((uchar *)cp));
		cp += sizeof(u_short);
		fprintf(file,MSGSTR( T_MX_OPT1, ", mail exchanger = "));
		goto doname;

	case T_NS:
		fprintf(file, MSGSTR( T_NS_OPT, "\tnameserver = "));
		goto doname;

	case T_PTR:
		fprintf(file, MSGSTR( T_PTR_OPT, "\thost name = "));
doname:
		cp = Print_cdname(cp, msg, eom, file);
		fprintf(file, MSGSTR(NEWLINE, "\n"));
		break;

	case T_HINFO:
		if (n = *cp++) {
			fprintf(file, MSGSTR(T_HINFO_OPT, "\tCPU=%.*s"), n, cp);
			cp += n;
		}
		if (n = *cp++) {
			fprintf(file, MSGSTR(T_HINFO_OPT1, "\tOS=%.*s\n"), n, cp);
			cp += n;
		}
		break;

	case T_SOA:
		fprintf(file, MSGSTR( T_SOA_OPT1, "\torigin = "));
		cp = Print_cdname(cp, msg, eom, file);
		fprintf(file, MSGSTR( T_SOA_OPT2, "\n\tmail addr = "));
		cp = Print_cdname(cp, msg, eom, file);
		fprintf(file, MSGSTR( T_SOA_OPT3, "\n\tserial=%ld"), _getlong((uchar *)cp));
		cp += sizeof(u_long);
		fprintf(file, MSGSTR( T_SOA_OPT4, ", refresh=%ld"), _getlong((uchar *)cp));
		cp += sizeof(u_long);
		fprintf(file, MSGSTR( T_SOA_OPT5, ", retry=%ld"), _getlong((uchar *)cp));
		cp += sizeof(u_long);
		fprintf(file, MSGSTR( T_SOA_OPT6, ", expire=%ld"), _getlong((uchar *)cp));
		cp += sizeof(u_long);
		fprintf(file, MSGSTR( T_SOA_OPT7, ", min=%ld\n"), _getlong((uchar *)cp));
		cp += sizeof(u_long);
		break;

	case T_MINFO:
		fprintf(file, MSGSTR( T_MINFO_OPT, "\trequests = "));
		cp = Print_cdname(cp, msg, eom, file);
		fprintf(file, MSGSTR( T_MINFO_OPT1, "\n\terrors = "));
		cp = Print_cdname(cp, msg, eom, file);
		break;

	case T_TXT:
                (void) fputs("\ttext = \"", file);
                cp2 = cp1 + dlen;
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
                (void) fputs("\"\n", file);
                break;

	case T_UINFO:
		fprintf(file, MSGSTR(T_UINFO_OPT, "\t%s\n"), cp);
		cp += dlen;
		break;

	case T_UID:
	case T_GID:
		if (dlen == 4) {
			fprintf(file, MSGSTR(T_GID_OPT, "\t%cid %ld\n"), 
				type == T_UID ? 'u' : 'g',
			    _getlong((uchar *)cp));
			cp += sizeof(int);
		} else {
			fprintf(file, MSGSTR( T_GID_OPT1, 
				"\t%cid of length %ld?\n"),
			    type == T_UID ? 'u' : 'g', dlen);
			cp += dlen;
		}
		break;

	case T_WKS: {
		struct protoent *protoPtr;

		if (dlen < sizeof(u_long) + 1)
			break;
		bcopy(cp, (char *)&inaddr, sizeof(inaddr));
		cp += sizeof(u_long);
		 if ((protoPtr = getprotobynumber(*cp)) != NULL) {
                    fprintf(file, MSGSTR( T_WKS_OPT,
			"\tinet address = %s, protocol = %s\n\t"),
                        inet_ntoa(inaddr), protoPtr->p_name);
                } else {
		fprintf(file, MSGSTR( T_WKS_OPT, 
				"\tinet address = %s, protocol = %d\n\t"),
				inet_ntoa(inaddr), *cp);
                }
                cp++;
		n = 0;
		while (cp < cp1 + dlen) {
			c = *cp++;
			do {
				struct servent *s;

 				if (c & 0200) {
					 s = getservbyport(n, NULL);
					if (s != NULL) {
                                            fprintf(file,MSGSTR( T_WKS_OPT1,
						"  %s"), s->s_name);
                                        } else {
                                            fprintf(file, MSGSTR( T_WKS_OPT2,
						" #%d"), n);
                                        }
                		}
 				c <<= 1;
			} while (++n & 07);
		}
		putc('\n',file);
	   }
	   break;

	case T_NULL:
		fprintf(file, MSGSTR( T_NULL_OPT, "(type NULL, dlen %d)\n"), dlen);
		cp += dlen;
		break;

	default:
		fprintf(file, MSGSTR( T_DEFAULT_OPT, "\t???\n"));
		cp += dlen;
	}
	if (_res.options & RES_DEBUG && type != T_SOA) {
            fprintf(file,"\tttl = %lu\n", rrttl);
        }
        if (cp != cp1 + dlen) {
                fprintf(file,
                        "\n*** Error: record size incorrect (%d != %d)\n\n",
                        cp - cp1, dlen);
                cp = NULL;
        }
	return (cp);
}
