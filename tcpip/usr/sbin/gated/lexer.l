/* @(#)58	1.5  src/tcpip/usr/sbin/gated/lexer.l, tcprouting, tcpip411, GOLD410 12/6/93 17:52:26 */
%{
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		defined
 *		parse_eof
 *		parse_open
 *		yylex
 *		yyrestart
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  lexer.l,v 1.19.2.1 1993/09/28 18:38:59 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#define	GATED_LEXER
#define	INCLUDE_STAT
#define	INCLUDE_TIME

#include "include.h"
#include "ctype.h"
#include "parse.h"
#include "parser.h"

#ifdef	vax11c
extern YYSTYPE yylval;
#endif	/* vax11c */
    
#define	printf(fmt, arg)	TRACE_PROTO(TR_PARSE) { trace(TR_PARSE, 0, fmt, arg); }
#undef	yywrap
#define	yywrap	parse_eof

#define	parse_newname(fip)	{ \
    register char *new_name; \
    parse_filename = (new_name = (char *) rindex((fip)->fi_name, '/')) ? ++new_name : (fip)->fi_name; \
    }

#define	YBEGIN(state)	TRACE_PROTO(TR_PARSE) { trace(TR_PARSE, 0, "yylex: %s State %d", parse_where(), state); } BEGIN state

static fi_info parse_fi[FI_MAX+1];		/* Table of input files */
static int fi_file = 0;				/* Index into file table */
char *parse_directory = (char *) 0;

PROTOTYPE(parse_include,
	  static int,
	  (char *));
PROTOTYPE(parse_eof,
	  static int,
	  (void));

#ifdef	FLEX_SCANNER

int yylineno = 0;

#undef	YY_INPUT
#define	YY_INPUT(buf, result, max_size)	result = fgets(buf, max_size, yyin) ? strlen(buf) : YY_NULL;

#define	YY_NEWLINE	yylineno++

#ifdef	YY_FATAL_ERROR
#undef	YY_FATAL_ERROR
#define	YY_FATAL_ERROR(msg)	{ trace(TR_ALL, LOG_ERR, msg); task_quit(EINVAL); }
#endif	/* YY_FATAL_ERROR */

#else	/* FLEX_SCANNER */

#define	YY_NEWLINE

#endif	/* FLEX_SCANNER */

%}

%s CONFIG PP
%a 3000

nl				\n
ws				[ \t]+
digit				[0-9]
letter				[A-Za-z]
hexdigit			[0-9A-Fa-f]
octet				(([01]?{digit}?{digit})|((2([0-4]{digit}))|(25[0-5])))
dot				\.
hyphen				\-
special				[{}()\.\-+:\|\?\*]
percent				\%
semi				\;

comment				\#.*
ccomment			"/*"
bstring				\<[^>]*\>
qstring				\"[^"]*\"
number				{digit}+
hexpair				{hexdigit}{hexdigit}
hexstring			0[xX]{hexpair}+
hhexstring			{hexpair}+({hyphen}{hexpair}+)+
octetstring			{octet}({dot}{octet})+
isoaddr				{hexdigit}{hexdigit}{dot}{hexdigit}{4}({dot}{hexdigit}+)*
keyword				{letter}{letter}+
name				{letter}(({letter}|{digit}|{hyphen})*({letter}|{digit}))*
hname				{name}({dot}{name})*{dot}?

%%
{comment}	{
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s COMMENT",
				  parse_where());
			}
		}
{ccomment}	{
			char ttchar;
			static const char *search = "*/";
			const char *sp = search;
			char buf[BUFSIZ];

			strcpy(buf, parse_where());

			while (ttchar = input()) {
			    TRACE_PROTO(TR_PARSE) {
				trace(TR_PARSE, 0, "lex: %s comment: %c(%x)",
				      parse_where(),
				      ttchar,
				      ttchar);
			    }
			    if (ttchar == (char) EOF) {
				(void) sprintf(parse_error, "lex: unterminated C constant at %s",
					       buf);
				return L_ERROR;
			    } else if (ttchar == *sp) {
				if (!*++sp) {
				    break;
				}
			    } else {
				if (ttchar == '\n') {
				    YY_NEWLINE;
				}
				sp = search;
			    }
			}
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s COMMENT",
				  parse_where());
			}
		}
{ws}		{ ; }
{bstring}	{
			yylval.str.ptr = (char *) yytext;
			yylval.str.len = yyleng;
			yylval.str.type = L_STRING;
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s L_STRING: %s length %d",
				  parse_where(),
				  yylval.str.ptr,
				  yylval.str.len);
			}
			return yylval.str.type;
		}
{qstring}	{
			yylval.str.ptr = (char *) yytext;
			yylval.str.len = yyleng;
			yylval.str.type = L_STRING;
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s L_STRING: %s length %d",
				  parse_where(),
				  yylval.str.ptr,
				  yylval.str.len);
			}
			return yylval.str.type;
		}

{name}	{
			bits *key;

			yylval.str.ptr = (char *) yytext;
			yylval.str.len = yyleng;
			key = parse_keyword((char *) yytext);
			if (key) {
			    yylval.str.type = key->t_bits;
			    TRACE_PROTO(TR_PARSE) {
				trace(TR_PARSE, 0, "lex: %s KEYWORD: %s",
				      parse_where(),
				      (char *) yytext);
			    }
			} else {
			    yylval.str.type = L_KEYWORD;
			    TRACE_PROTO(TR_PARSE) {
				trace(TR_PARSE, 0, "lex: %s L_KEYWORD: %s length: %d",
				      parse_where(),
				      yylval.str.ptr,
				      yylval.str.len);
			    }
			}
			return yylval.str.type;
		}
<PP>{nl}	{
			YY_NEWLINE;
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s L_EOS",
				  parse_where());
			}
			YBEGIN(CONFIG);
			return L_EOS;
		}

<CONFIG>{nl}	{
			YY_NEWLINE;
		}
<CONFIG>{semi}	{
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s L_EOS",
				  parse_where());
			}
			return L_EOS;
		}
<CONFIG>^{percent}{ws}?[iI][nN][cC][lL][uU][dD][eE]{ws}?{qstring}{ws}?{nl} {
    			register char *cp = (char *) yytext;
			char *name;

			while (*cp++ != '"') ;
			name = cp;
			while (*cp != '"') {
			    cp++;
			}
			*cp = (char) 0;
			cp = name;

#ifdef	FLAT_FS
			name = task_mem_strdup((task *) 0, cp);
#else	/* FLAT_FS */
			switch (*cp) {
			case '/':
			    name = task_mem_strdup((task *) 0,
						   cp);
			    break;

			default:
			    name = task_mem_malloc((task *) 0,
						   (size_t) (strlen(cp) + strlen(parse_directory) + 2));

			    (void) strcpy(name, parse_directory);
			    (void) strcat(name, "/");
			    (void) strcat(name, cp);
			}
#endif	/* FLAT_FS */
			
    			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s INCLUDE '%s'",
				  parse_where(),
				  name);
			}

			/* Count the newline in this file */
    			YY_NEWLINE;

			if (parse_include(name)) {
			    return L_ERROR;
			}
			/* String is freed by parse_eof() */
}
<CONFIG>^{percent}{ws}?[dD][iI][rR][eE][cC][tT][oO][rR][yY]{ws}?{qstring}{ws}?{nl} {
    			register char *cp = (char *) yytext;
			char *name;
#ifndef	NO_STAT
			struct stat stbuf;
#endif	/* NO_STAT */

			while (*cp++ != '"') ;
			name = cp;
			while (*cp != '"') {
			    cp++;
			}
			*cp-- = (char) 0;
			if (*cp == '/') {
			    *cp-- = (char) 0;
			}

#ifndef	NO_STAT
			if (stat(name, &stbuf) < 0) {
			    (void) sprintf(parse_error, "stat(%s): %m",
					   name);
			    return L_ERROR;
			}

			switch (stbuf.st_mode & S_IFMT) {
			case S_IFDIR:
			    break;

			default:
			    (void) sprintf(parse_error, "%s is not a directory",
					   name);
			    return L_ERROR;
			}
#endif	/* NO_STAT */

			if (parse_directory) {
			    task_mem_free((task *) 0,
					  parse_directory);
			}
			parse_directory = task_mem_strdup((task *) 0,
							  name);
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s included file prefeix now %s",
				  parse_where(),
				  parse_directory);
			}
}
<CONFIG>^{percent}	{
			YBEGIN(PP);
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s '%s'",
				  parse_where(),
				  (char *) yytext);
			}
			return (char) *yytext;
		}
<CONFIG>{hname}	{
			yylval.str.ptr = (char *) yytext;
			yylval.str.len = yyleng;
			yylval.str.type = L_HNAME;
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s HNAME: %s length %d",
				  parse_where(),
				  yylval.str.ptr,
				  yylval.str.len);
			}
			return yylval.str.type;
		}
<CONFIG>{special}	{
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s '%s'",
				  parse_where(),
				  (char *) yytext);
			}
			return (char) *yytext;
		}
<CONFIG>{number}	{
			yylval.num = atoi((char *) yytext);
			TRACE_PROTO(TR_PARSE) {
			    trace(TR_PARSE, 0, "lex: %s L_NUMBER: %d",
				  parse_where(),
				  yylval.num);
			}
			return L_NUMBER;
		}
<CONFIG>{isoaddr}	{
#define	atox(c)	(isdigit(c) ? (c - '0') : ( isupper(c) ? (c - 'A' + 10) : (c - 'a' + 10) ))
#ifdef	PROTO_ISO
			register int i = 0;
			register u_char acc = 0;
			register char *cp;
			register byte *dp;

			for (cp = (char *) yytext; *cp; cp++) {
			    if (*cp != '.') {
				i++;
			    }
			}
			yylval.hex.len = (i + 1)/2;
			yylval.hex.ptr = (byte *) task_mem_malloc((task *) 0, yylval.hex.len);
			yylval.hex.type = L_ISOADDR;
			dp = yylval.hex.ptr;
			cp = (char *) yytext;

			/* Pick up string ignoring trailing dots */
			i = 0;
			do {
			    switch (*cp) {
			    case '.':
				/* Ignore dots */
				break;

			    case 0:
				if (i) {
				    /* Stuff the trailing nibble */

				    *dp = acc;
				}
				/* This will cause the loop to break */
				cp = (char *) 0;
				break;

			    default:
				if (i = !i) {
				    /* First nibble */

				    acc = atox(*cp) << 4;
				} else {
				    /* Second nibble */

				    *dp++ = acc | atox(*cp);
				}
				break;
			    }			    
			} while (cp++);

			TRACE_PROTO(TR_PARSE) {
			    tracef("lex: %s L_ISOADDR: length %d ",
				   parse_where(),
				   yylval.hex.len);
			    for (i = 0; i < yylval.hex.len; i++) {
				tracef("%02x",
				       yylval.hex.ptr[i]);
			    }
			    trace(TR_PARSE, 0, NULL);
			}
			return yylval.hex.type;
#else	/* PROTO_ISO */
			return (char) *yytext;
#endif	/* PROTO_ISO */
		    }
<CONFIG>{octetstring}  {
    			int i;
    			char *cp;

			/* First count the number of octets */
			yylval.hex.len = 1;
			for (cp = (char *) yytext; cp = (char *) index(cp, '.'); *cp++ = (char) 0) {
			    yylval.hex.len++;
			}

			yylval.hex.ptr = (byte *) task_mem_malloc((task *) 0, yylval.hex.len);
			yylval.hex.type = L_OCTETSTRING;

			cp = (char *) yytext;
			for (i = 0; i < yylval.hex.len; i++) {
			    yylval.hex.ptr[i] = atoi(cp);
			    cp += strlen(cp) + 1;
			}

			TRACE_PROTO(TR_PARSE) {
			    tracef("lex: %s L_OCTETSTRING: length %d ",
				   parse_where(),
				   yylval.hex.len);
			    for (i = 0; i < yylval.hex.len; i++) {
				tracef("%s%u",
				       i ? "." : "",
				       yylval.hex.ptr[i]);
			    }
			    trace(TR_PARSE, 0, NULL);
			}

			return L_OCTETSTRING;
		    }
<CONFIG>{hhexstring}	{
			int i = 0;
			char *cp;
			byte *dp;

			for (cp = (char *) yytext; *cp; cp++) {
			    if (*cp != '-') {
				i++;
			    }
			}
			yylval.hex.len = (i + 1)/2;
			yylval.hex.ptr = (byte *) task_mem_malloc((task *) 0, yylval.hex.len);
			dp = yylval.hex.ptr;
			cp = (char *) yytext;
#ifdef	notdef
			if (i % 2) {
			    *(dp++) = atox(*cp);
			    cp++;
			    yylval.hex.type = L_HEXSTRING_ODD;
			} else
#endif	/* notdef */
			    yylval.hex.type = L_HEXSTRING;
			for (; *cp; cp++, dp++) {
			    if (*cp == '-') {
				cp++;
			    }
			    *dp = atox(*cp) << 4;
			    cp++;
			    if (*cp == '-') {
				cp++;
			    }
			    *dp |= atox(*cp);
			}
			TRACE_PROTO(TR_PARSE) {
			    tracef("lex: %s L_HEXSTRING: length %d ",
				   parse_where(),
				   yylval.hex.len);
			    for (i = 0; i < yylval.hex.len; i++) {
				tracef("%02x",
				       yylval.hex.ptr[i]);
			    }
			    trace(TR_PARSE, 0, NULL);
			}
			return yylval.hex.type;
		    }
<CONFIG>{hexstring}	{
			int i = yyleng - 2;
    			char *cp = (char *) yytext + 2;
			byte *dp;

			yylval.hex.len = (i + 1)/2;
			yylval.hex.ptr = (byte *) task_mem_malloc((task *) 0, yylval.hex.len);
			dp = yylval.hex.ptr;
#ifdef	notdef
			if (i % 2) {
			    *(dp++) = atox(*cp);
			    cp++;
			    yylval.hex.type = L_HEXSTRING_ODD;
			} else
#endif	/* notdef */
			    yylval.hex.type = L_HEXSTRING;
			for (; *cp; cp += 2, dp++) {
			    *dp = (atox(cp[0]) << 4) | atox(cp[1]);
			}
			TRACE_PROTO(TR_PARSE) {
			    tracef("lex: %s L_HEXSTRING: length %d 0x",
				   parse_where(),
				   yylval.hex.len);
			    for (i = 0; i < yylval.hex.len; i++) {
				tracef("%02x",
				       yylval.hex.ptr[i]);
			    }
			    trace(TR_PARSE, 0, NULL);
			}
			return yylval.hex.type;
		    }
.		{
    			(void) sprintf(parse_error, "unrecognized character: `%s'",
				       (char *) yytext);
			return L_ERROR;
		}
%%

#if	!defined(FLEX_SCANNER) || !defined(YY_USER_ACTION)

/* For parsers that do not define yyrestart() */
/* Define it as a function so we get an error if it is already defined */
void
yyrestart(fp)
FILE *fp;
{
    yyin = fp;
}
#else	/* !defined(FLEX_SCANNER) || !defined(YY_USER_ACTION) */

/* Assign yyin the first time and call yyrestart() other times */
#define	YYRESTART_MAGIC

#endif	/* !defined(FLEX_SCANNER) || !defined(YY_USER_ACTION) */


/*
 *	Open the first file (called from parse_parse).  If no file specified,
 *	assume stdin (for testing ).
 */
int
parse_open(name)
char *name;
{
#ifdef	YYRESTART_MAGIC
    static int first;
#endif	/* YYRESTART_MAGIC */
    fi_info *fip;

    fip = &parse_fi[fi_file = 0];

    if (name) {
	fip->fi_FILE = fopen(name, "r");
	if (!fip->fi_FILE) {
	    trace(TR_ALL, LOG_ERR, "parse_open: error opening %s: %m",
		  name);
	    return TRUE;
	}
    } else {
#if	YYDEBUG != 0
	name = "stdin";
#else	/* YYDEBUG */
	trace(TR_ALL, LOG_ERR, "parse_open: no file specified");
	return TRUE;
#endif	/* YYDEBUG */
    }

    yylineno = 1;
#ifdef	YYRESTART_MAGIC
    if (first++) {
	yyrestart(fip->fi_FILE);
    } else {
	yyin = fip->fi_FILE;
    }
#else	/* YYRESTART_MAGIC */
    yyrestart(fip->fi_FILE);
#endif	/* YYRESTART_MAGIC */

    fip->fi_name = name;
    parse_newname(fip);
    TRACE_PROTO(TR_PARSE) {
	trace(TR_PARSE, 0, "parse_open: reading %s (%d)",
	      fip->fi_name,
	      fileno(fip->fi_FILE));
    }
	
    YBEGIN(CONFIG);
    return FALSE;
}


/*
 *	Process an include directive, read another input file
 */
static int
parse_include __PF1(name, char *)
{
    fi_info *fip;
    FILE *file;

    /* Verify file name is unique (this is easy to fool) */
    for (fip = parse_fi; fip <= &parse_fi[fi_file]; fip++) {
	if (fip->fi_name && !strcmp(name, fip->fi_name)) {
	    (void) sprintf(parse_error, "recursive includes: %s",
			   name);
	    return TRUE;
	}
    }

    /* Try to open new file */
    if (fi_file > FI_MAX) {
	(void) sprintf(parse_error, "too many levels of includes");
	return TRUE;
    }

    file = fopen(name, "r");
    if (!file) {
	(void) sprintf(parse_error, "error opening %s: %m",
		       name);
	return TRUE;
    }

    /* Update line number of current file and make sure we have correct file pointer */
    fip = &parse_fi[fi_file++];
    fip->fi_lineno = yylineno;
    fip->fi_FILE = yyin;

    fip++;
    fip->fi_name = name;
    fip->fi_FILE = file;
    yylineno = 1;
    yyin = fip->fi_FILE;
    parse_newname(fip);
    TRACE_PROTO(TR_PARSE) {
	trace(TR_PARSE, 0, "parse_include: %s now reading %s (%d)",
	      parse_where(),
	      fip->fi_name,
	      fileno(fip->fi_FILE));
    }

    return FALSE;
}


/* parse_eof - process end of file on current input file */
static int
parse_eof()
{
    fi_info *fip;

    if (fi_file < 0) {
	return TRUE;
    }
    
    fip = &parse_fi[fi_file];
    TRACE_PROTO(TR_PARSE) {
	trace(TR_PARSE, 0, "parse_eof: %s EOF on %s (%d)",
	      parse_where(),
	      fip->fi_name,
	      fileno(fip->fi_FILE));
    }
    if (fclose(fip->fi_FILE)) {
	trace(TR_ALL, LOG_ERR, "parse_eof: error closing %s: %m",
	      fip->fi_name);
    }
    fip->fi_FILE = NULL;
    task_mem_free((task *) 0, fip->fi_name);
    fip->fi_name = NULL;
    parse_filename = NULL;

    if (fi_file-- > 0) {
	fip = &parse_fi[fi_file];
	parse_newname(fip);
	yylineno = fip->fi_lineno;
	yyin = fip->fi_FILE;
	TRACE_PROTO(TR_PARSE) {
	    trace(TR_PARSE, 0, "parse_eof: %s resuming %s",
		  parse_where(),
		  fip->fi_name);
	}
	return FALSE;
    }
    yyin = (FILE *) 0;
    TRACE_PROTO(TR_PARSE) {
	trace(TR_PARSE, 0, "parse_eof: %s EOF",
	      parse_where());
    }
    return TRUE;
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
