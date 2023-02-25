/* @(#)68	1.4  src/tcpip/usr/bin/telnet/parse.h, tcptelnet, tcpip411, GOLD410 8/23/90 18:15:52 */
/* 
 * COMPONENT_NAME: TCPIP parse.h
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27  38 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	Copyright 1985 by Paul Milazzo - all rights reserved
 */

/*
 *  token classes for the IBM 3278 emulator keyboard profile file
 */

#define C_IDENT 0
#define C_STRING	1
#define C_BIND	2
#define C_ELSE	3
#define C_EXIT	4
#define C_IF	5
#define C_LOAD	6
#define C_OR	7
#define C_PRINT 8
#define C_QUIT	9
#define C_FUNC	10
#define C_EOF	11

/* parser states */

#define S_STRT	0
#define S_BIND	1
#define S_LOAD	2
#define S_IF	3
#define S_PRNT	4
#define S_EXIT	5
#define S_BSTR	6
#define S_THEN	7
#define S_ELSE	8
#define	NUMSTATES	9

/* parser action special flags */

#define F_EXIT		0x100	/* set the EXIT flag */
#define F_QUIT		0x200	/* set the QUIT flag */
#define F_S2I		0x400	/* convert string to ident */
#define F_FLAGS		0x700	/* general mask for flag bits */

#define A_____		0	/* do nothing */
#define A_ELSE		1	/* invert last condition */
#define A_IF		2	/* clear last condition */
/* #define A_OR		3	 * logical OR into last condition */
#define A_FUNC		4	/* store function pointer */
#define A_LOAD		5	/* load file */
#define A_LDST		A_LOAD|F_S2I
#define A_TERM		6	/* match terminal type */
#define A_TRMS		A_TERM|F_S2I
#define A_PRNT		7	/* print on terminal */
#define A_PRTS		A_PRNT|F_S2I
#define A_PRTN		8	/* print newline */
#define A_PRTE		A_PRTN|F_EXIT
#define A_PRTQ		A_PRTN|F_QUIT
#define A_BIND		9	/* bind string to saved function pointer */
#define A_BNDS		A_BIND|F_S2I
#define A_COND		10	/* set conditional execution flag */
#define A_CNDE		A_COND|F_EXIT
#define A_CNDQ		A_COND|F_QUIT

#define A_EXIT		F_EXIT	/* set EXIT flag */
#define A_QUIT		F_QUIT	/* set QUIT flag */
#define A_E001		11	/* ERROR - misplaced identifier ignored */
#define A_E002		12	/* ERROR - misplaced string ignored */
#define A_E003		13	/* ERROR - misplaced OR ignored */
#define A_E004		14	/* ERROR - misplaced function key reference */
#define A_E005		15	/* ERROR - incomplete BIND command */
#define A_E105		A_E005|F_EXIT
#define A_E205		A_E005|F_QUIT
#define A_E006		16	/* ERROR - missing file name */
#define A_E106		A_E006|F_EXIT
#define A_E206		A_E006|F_QUIT
#define A_E007		17	/* ERROR - incomplete IF statement */
#define A_E107		A_E007|F_EXIT
#define A_E207		A_E007|F_QUIT
#define A_E008		18	/* ERROR - EOF in ELSE statment */
#define A_E108		18|F_EXIT

/* actions maps (TOKENCLASS x STATE) -> ACTION */

static int	actions[][NUMSTATES] = {

/* class      S_STRT,S_BIND,S_LOAD,S_IF	 ,S_PRNT,S_EXIT,S_BSTR,S_THEN,S_ELSE*/

/*C_IDENT */  A_E001,A_E001,A_LOAD,A_TERM,A_PRNT,A_PRNT,A_BIND,A_E001,A_E001,
/*C_STRING*/  A_E002,A_E002,A_LDST,A_TRMS,A_PRTS,A_PRTS,A_BNDS,A_E002,A_E002,
/*C_BIND  */  A_____,A_E005,A_E006,A_E007,A_PRTN,A_____,A_E005,A_COND,A_____,
/*C_ELSE  */  A_ELSE,A_E005,A_E006,A_E007,A_PRTN,A_____,A_E005,A_COND,A_E008,
/*C_EXIT  */  A_EXIT,A_E105,A_E106,A_E107,A_PRTE,A_____,A_E105,A_CNDE,A_EXIT,
/*C_IF	  */  A_IF  ,A_E005,A_E006,A_E007,A_PRTN,A_____,A_E005,A_COND,A_IF  ,
/*C_LOAD  */  A_____,A_E005,A_E006,A_E007,A_PRTN,A_____,A_E005,A_COND,A_____,
/*C_OR	  */  A_E003,A_E003,A_E003,A_E003,A_E003,A_____,A_E003,A_____,A_E003,
/*C_PRINT */  A_____,A_E005,A_E006,A_E007,A_PRTN,A_____,A_E005,A_COND,A_____,
/*C_QUIT  */  A_QUIT,A_E205,A_E206,A_E207,A_PRTQ,A_____,A_E205,A_CNDQ,A_QUIT,
/*C_FUNC  */  A_E004,A_FUNC,A_E004,A_E004,A_E004,A_____,A_E004,A_E004,A_E004,
/*C_EOF	  */  A_EXIT,A_E105,A_E106,A_E107,A_PRTE,A_____,A_E105,A_E107,A_E108,
};

/* states maps (TOKENCLASS x STATE) -> STATE */

static int	states[][NUMSTATES] = {

/* class      S_STRT,S_BIND,S_LOAD,S_IF	 ,S_PRNT,S_EXIT,S_BSTR,S_THEN,S_ELSE*/

/*C_IDENT */  S_STRT,S_STRT,S_STRT,S_THEN,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,
/*C_STRING*/  S_STRT,S_STRT,S_STRT,S_THEN,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,
/*C_BIND  */  S_BIND,S_BIND,S_BIND,S_BIND,S_BIND,S_STRT,S_BIND,S_BIND,S_BIND,
/*C_ELSE  */  S_ELSE,S_ELSE,S_ELSE,S_ELSE,S_ELSE,S_STRT,S_ELSE,S_ELSE,S_ELSE,
/*C_EXIT  */  S_EXIT,S_EXIT,S_EXIT,S_EXIT,S_EXIT,S_STRT,S_EXIT,S_EXIT,S_EXIT,
/*C_IF	  */  S_IF  ,S_IF  ,S_IF  ,S_IF	 ,S_IF	,S_STRT,S_IF  ,S_IF  ,S_IF  ,
/*C_LOAD  */  S_LOAD,S_LOAD,S_LOAD,S_LOAD,S_LOAD,S_STRT,S_LOAD,S_LOAD,S_LOAD,
/*C_OR	  */  S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_IF  ,S_STRT,
/*C_PRINT */  S_PRNT,S_PRNT,S_PRNT,S_PRNT,S_PRNT,S_STRT,S_PRNT,S_PRNT,S_PRNT,
/*C_QUIT  */  S_EXIT,S_EXIT,S_EXIT,S_EXIT,S_EXIT,S_STRT,S_EXIT,S_EXIT,S_EXIT,
/*C_FUNC  */  S_STRT,S_BSTR,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,
/*C_EOF	  */  S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,S_STRT,
};
