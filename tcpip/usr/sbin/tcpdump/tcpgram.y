/* @(#)70	1.2  src/tcpip/usr/sbin/tcpdump/tcpgram.y, tcpip, tcpip411, GOLD410 9/15/93 14:53:38 */
%{
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: and
 *		aqual
 *		arth
 *		byteop
 *		dqual
 *		expr
 *		head
 *		id
 *		irelop
 *		narth
 *		ndaqual
 *		nid
 *		not
 *		null
 *		or
 *		other
 *		paren
 *		pid
 *		pname
 *		pnum
 *		pqual
 *		qid
 *		relop
 *		rterm
 *		term
 *		yyparse
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1988-1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Grammar for tcpdump.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#ifdef _AIX
#include <sys/cdli.h>
#include <net/nd_lan.h>
#endif /* _AIX */
#include <netinet/if_ether.h>

#include "interface.h"

#include <sys/time.h>
#include <net/bpf.h>

#include "gencode.h"

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror()
{
	++n_errors;
}

%}

%union {
	int i;
	u_long h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		struct block *b;
	} blk;
	struct block *rblk;
}

%type	<blk>	expr id nid pid term rterm qid
%type	<blk>	head
%type	<i>	pqual dqual aqual ndaqual
%type	<a>	arth narth
%type	<i>	byteop pname pnum relop irelop
%type	<blk>	and or paren not null prog
%type	<rblk>	other

%token  DST SRC HOST GATEWAY
%token  NET PORT LESS GREATER PROTO BYTE
%token  ARP RARP IP TCP UDP ICMP
%token  TK_BROADCAST TK_MULTICAST
%token  NUM
%token  LINK
%token	GEQ LEQ NEQ
%token	ID EID HID
%token	LSH RSH
%token  LEN

%type	<s> ID
%type	<e> EID
%type	<h> HID
%type	<i> NUM

%left OR AND
%nonassoc  '!'
%left '|'
%left '&' 
%left LSH RSH
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%%
prog:	  null expr
{ 
	finish_parse($2.b);
}
	| null
	;
null:	  /* null */		{ $$.q = qerr; }
	;
expr:	  term
	| expr and term		{ gen_and($1.b, $3.b); $$ = $3; }
	| expr and id		{ gen_and($1.b, $3.b); $$ = $3; }
	| expr or term		{ gen_or($1.b, $3.b); $$ = $3; }
	| expr or id		{ gen_or($1.b, $3.b); $$ = $3; }
	;
and:	  AND			{ $$ = $<blk>0; }
	;
or:	  OR			{ $$ = $<blk>0; }
	;
id:	  nid
	| pnum			{ $$.b = gen_ncode((u_long)$1,
						   $$.q = $<blk>0.q); }
	| paren pid ')'		{ $$ = $2; }
	;
nid:	  ID			{ $$.b = gen_scode($1, $$.q = $<blk>0.q); }
	| HID			{ $$.b = gen_ncode($1, $$.q = $<blk>0.q); }
	| EID			{ $$.b = gen_ecode($1, $$.q = $<blk>0.q); }
	| not id		{ gen_not($2.b); $$ = $2; }
	;
not:	  '!'			{ $$ = $<blk>0; }
	;
paren:	  '('			{ $$ = $<blk>0; }
	;
pid:	  nid
	| qid and id		{ gen_and($1.b, $3.b); $$ = $3; }
	| qid or id		{ gen_or($1.b, $3.b); $$ = $3; }
	;
qid:	  pnum			{ $$.b = gen_ncode((u_long)$1, 
						   $$.q = $<blk>0.q); }
	| pid
	;
term:	  rterm
	| not term		{ gen_not($2.b); $$ = $2; }
	;
head:	  pqual dqual aqual	{ QSET($$.q, $1, $2, $3); }
	| pqual dqual		{ QSET($$.q, $1, $2, Q_DEFAULT); }
	| pqual aqual		{ QSET($$.q, $1, Q_DEFAULT, $2); }
	| pqual PROTO		{ QSET($$.q, $1, Q_DEFAULT, Q_PROTO); }
	| pqual ndaqual		{ QSET($$.q, $1, Q_DEFAULT, $2); }
	;
rterm:	  head id		{ $$ = $2; }
	| paren expr ')'	{ $$.b = $2.b; $$.q = $1.q; }
	| pname			{ $$.b = gen_proto_abbrev($1); $$.q = qerr; }
	| arth relop arth	{ $$.b = gen_relation($2, $1, $3, 0); 
				  $$.q = qerr; }
	| arth irelop arth	{ $$.b = gen_relation($2, $1, $3, 1); 
				  $$.q = qerr; }
	| other			{ $$.b = $1; $$.q = qerr; }
	;
/* protocol level qualifiers */
pqual:	  pname
	|			{ $$ = Q_DEFAULT; }
	;
/* 'direction' qualifiers */
dqual:	  SRC			{ $$ = Q_SRC; }
	| DST			{ $$ = Q_DST; }
	| SRC OR DST		{ $$ = Q_OR; }
	| DST OR SRC		{ $$ = Q_OR; }
	| SRC AND DST		{ $$ = Q_AND; }
	| DST AND SRC		{ $$ = Q_AND; }
	;
/* address type qualifiers */
aqual:	  HOST			{ $$ = Q_HOST; }
	| NET			{ $$ = Q_NET; }
	| PORT			{ $$ = Q_PORT; }
	;
/* non-directional address type qualifiers */
ndaqual:  GATEWAY 		{ $$ = Q_GATEWAY; }
	;
pname:	  LINK			{ $$ = Q_LINK; }
	| IP			{ $$ = Q_IP; }
	| ARP			{ $$ = Q_ARP; }
	| RARP			{ $$ = Q_RARP; }
	| TCP			{ $$ = Q_TCP; }
	| UDP			{ $$ = Q_UDP; }
	| ICMP			{ $$ = Q_ICMP; }
	;
other:	  pqual TK_BROADCAST	{ $$ = gen_broadcast($1); }
	| pqual TK_MULTICAST	{ $$ = gen_multicast($1); }
	| LESS NUM		{ $$ = gen_less($2); }
	| GREATER NUM		{ $$ = gen_greater($2); }
	| BYTE NUM byteop NUM	{ $$ = gen_byteop($3, $2, $4); }
	;
relop:	  '>'			{ $$ = BPF_JGT; }
	| GEQ			{ $$ = BPF_JGE; }
	| '='			{ $$ = BPF_JEQ; }
	;
irelop:	  LEQ			{ $$ = BPF_JGT; }
	| '<'			{ $$ = BPF_JGE; }
	| NEQ			{ $$ = BPF_JEQ; }
	;
arth:	  pnum			{ $$ = gen_loadi($1); }
	| narth
	;
narth:	  pname '[' arth ']'		{ $$ = gen_load($1, $3, 1); }
	| pname '[' arth ':' NUM ']'	{ $$ = gen_load($1, $3, $5); }
	| arth '+' arth			{ $$ = gen_arth(BPF_ADD, $1, $3); }
	| arth '-' arth			{ $$ = gen_arth(BPF_SUB, $1, $3); }
	| arth '*' arth			{ $$ = gen_arth(BPF_MUL, $1, $3); }
	| arth '/' arth			{ $$ = gen_arth(BPF_DIV, $1, $3); }
	| arth '&' arth			{ $$ = gen_arth(BPF_AND, $1, $3); }
	| arth '|' arth			{ $$ = gen_arth(BPF_OR, $1, $3); }
	| arth LSH arth			{ $$ = gen_arth(BPF_LSH, $1, $3); }
	| arth RSH arth			{ $$ = gen_arth(BPF_RSH, $1, $3); }
	| '-' arth %prec UMINUS		{ $$ = gen_neg($2); }
	| paren narth ')'		{ $$ = $2; }
	| LEN				{ $$ = gen_loadlen(); }
	;
byteop:	  '&'			{ $$ = '&'; }
	| '|'			{ $$ = '|'; }
	| '<'			{ $$ = '<'; }
	| '>'			{ $$ = '>'; }
	| '='			{ $$ = '='; }
	;
pnum:	  NUM
	| paren pnum ')'	{ $$ = $2; }
	;
%%
