%{

static char sccsid[] = "@(#)05	1.4  src/bos/usr/sbin/adfutil/parser.y, cmdadf, bos411, 9428A410j 6/15/90 16:58:24";

%}
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (parser) yyparse()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/*
 * NAME: parser (yyparse)
 *                                                                    
 * FUNCTION: use tokens to decode sentences of the grammar as defined below
 *                                                                    
 * NOTES: compile with "yacc -d parser.y" to make y.tab.c
 *
 * RETURN: sets global "proot" as pointer to adft
 *	
 */  

%{

#include "symbol.h"
#include "externs.h"

int block = 0;

%}

%union	{
	int w;
	ulong ul;
	char *sb;
	struct cl *pcl;
	struct fl *pfl;
	struct fr *pfr;
	struct he *phe;
	struct ni *pni;
	struct nml *pnm;
	struct pl *ppl;
	struct rgl *prg;
	struct rl *prl;
	struct sl *psl;
	};

%token BBEGIN END DEVICE SCSI SYSMEM INITPROG LOCK ADDRESS FUNCTION
%token MICROCODE INPUT SHAREDARB VCHECK RS422 DYNARB NODMA

%token LBRACK RBRACK EQ DASH
%token ARB ID NAME
%token CHOICE
%token FIXED
%token HELP 
%token INT IO
%token MEM
%token ITEM BYTES
%token POS PROMPT

%token <w>   DECIMAL
%token <ul>  HEX
%token <sb>  POSBYTE
%token <sb>  STRING
%token <phe> IDENT

%type <prl> arb_list
%type <pni> b_end
%type <pni> b_item
%type <pni> b_item_list
%type <ul>  card_id
%type <sb>  card_name
%type <pcl> choice
%type <pcl> choice_list
%type <pfr> fixed_resources
%type <sb>  help
%type <pni> input_item 
%type <prl> interrupt_list
%type <prl> ioblock_list
%type <pni> item
%type <pni> item_list
%type <prl> memaddr_list
%type <pfl> micro_list
%type <pni> named_item
%type <w>   nbytes
%type <ul>  number
%type <pnm> number_list
%type <ppl> pos_setting
%type <ppl> pos_setting_list
%type <sb>  prompt
%type <prg> range
%type <prg> range_list
%type <prl> resource_setting
%type <prl> res_list
%type <pfl> string_list
%type <psl> system
%type <psl> system_list
%type <w>   unsup

%start adf_file

%%

adf_file : card_id card_name nbytes system_list item_list
	{
	    proot = makeadft($1, $2, $3, $4, $5);
	}
	;

card_id : ID number
	{
	    $$ = (0xff00 & $2 << 8) | (0xff & $2 >> 8);
	}
	;

card_name: NAME STRING
	{
	    $$ = $2;
	}
	;

nbytes	: BYTES number
	{
	    $$ = (int) $2;
	}
	;

system_list: /* empty */
	{
	    $$ = NULL;
	}
	| system_list system
	{
	    $$ = attachsl($2, $1);
	}
	;

system	: fixed_resources
	{
	    $$ = makesl(stFixed, $1);
	}
	| micro_list
	{
	    $$ = makesl(stMicro, $1);
	}
	| unsup
	{
	    $$ = makesl(stUnsup, $1);
	}
	;

fixed_resources: FIXED pos_setting_list res_list
	{
	    $$ = makefr($2, $3);
	}
	;

micro_list: MICROCODE string_list
	{
	    $$ = $2;
	    if (adf_ucode) {
		struct fl *pfl;
		for (pfl = adf_ucode; pfl->pflNext; pfl = pfl->pflNext)
		    /* NO-OP */;
		pfl->pflNext = $2;
	    } else
		adf_ucode = $2;
	}
	;

string_list: string_list STRING
	{
	    $$ = attachfl($2, $1);
	}
	| STRING
	{
	    $$ = makefl($1);
	}
	;

item_list: /* empty */
	{
	    $$ = NULL;
	}
	| item_list item
	{
	    $$ = attachni($2, $1);
	}
	;

item	: b_end
	| b_item
	;

b_end	: BBEGIN { block++; } device rs422 dynamic dma b_item_list END
	{
	    $$ = $7;
	}
	;

b_item_list: /* empty */
	{
	    $$ = NULL;
	}
	| b_item_list b_item
	{
	    $2->blocknum = block;
	    $$ = attachni($2, $1);
	}
	;

b_item	: named_item
	| input_item 	/* UNSUPPORTED */
	;

named_item: 
	{ 
	    cChoice = 0; 
	}
	ITEM prompt 
	{
	    if (fPrtDebug)
		printf("Item %d %s\n", cItem++, $3);
	} 
	choice_list help 
	{
	    $$ = makeni($3, $5, $6);
	}
	;

input_item: INPUT prompt pos_setting range help
	{
	    $$ = makeni($2, NULL, $5);
	}
	;

prompt	: PROMPT STRING {
	    $$ = $2;
	}
	;

choice_list: choice_list choice
	{
	    $$ = attachcl($2, $1);
	}
	| choice
	;

choice: { cPos = 0; cRes = 0; }
	CHOICE valid STRING pos_setting_list res_list 
	{
	    if (fPrtDebug)
	    printf("Choice %d %s, [%d] pos, [%d] resource\n", ++cChoice, $4,
		cPos, cRes);
	    $$ = makecl($5, $6, $4);
	}
	| { cPos = 0; cRes = 0; } FUNCTION STRING
	{
	    if (fPrtDebug)
	    printf("Choice %d %s, [%d] pos, [%d] resource\n", ++cChoice, $3,
		cPos, cRes);
	    $$ = makecl(NULL, NULL, $3);
	    $$->ffunc = 1;
	}
	;

valid	: VCHECK
	| /* empty */
	;

help	: HELP STRING
	{
	    $$ = $2;
	}
	;

pos_setting_list: pos_setting_list pos_setting
	{
	    $$ = attachpl($2, $1);
	}
	| pos_setting
	;

pos_setting: POS LBRACK number RBRACK EQ POSBYTE 
	{
	    cPos++;
	    $$ = makepl(2+$3, $6);
	}
	;

res_list: /* empty */
	{
	    $$ = NULL;
	}
	| res_list resource_setting
	{
	    $$ = attachrl($2, $1);
	}
	;

resource_setting: ioblock_list 
	| interrupt_list 
	| arb_list 
	| memaddr_list
	;

ioblock_list: IO range_list {
	    cRes++;
	    $$ = makerl(rsIo, $2);
	}
	;

interrupt_list: INT number_list {
	    cRes++;
	    $$ = makerl(rsInt, $2);
	}
	;

arb_list: ARB sharedarb number_list {
	    cRes++;
	    $$ = makerl(rsArb, $3);
	}
	;

sharedarb: SHAREDARB
	| 
	;
memaddr_list: MEM range_list 
	{
	    cRes++;
	    $$ = makerl(rsMem, $2);
	}
	;

range_list: range_list range
	{
	    $$ = attachrgl($2, $1);
	}
	| range
	;

range	: number DASH number
	{
	    $$ = makergl($1, $3);
	}
	;

number_list: number_list number
	{
	    $$ = makenml($2, $1);
	}
	| number
	{
	    $$ = makenml($1, NULL);
	}
	;

number	: DECIMAL
	{
	    $$ = (ulong) $1;
	}
	| HEX
	;

unsup	: sys_mem	{ $$ = SYSMEM; }
	| init_prog	{ $$ = INITPROG; }
	| lock		{ $$ = LOCK; }
	| address	{ $$ = ADDRESS; }
	| error		{ $$ = UNKNOWN; }
	;

device	: DEVICE SCSI
	| /* empty */
	;

rs422	: RS422
	| /* empty */
	;

dynamic : DYNARB
	| /* empty */
	;

dma	: NODMA
	| /* empty */
	;

sys_mem	: SYSMEM number 
	| SYSMEM number number
	;

init_prog: INITPROG number
	;

lock	: LOCK
	;

address	: ADDRESS number
	;

