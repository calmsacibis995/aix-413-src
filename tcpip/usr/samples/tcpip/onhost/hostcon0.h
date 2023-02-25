/*static char sccsid[] = "src/tcpip/usr/samples/tcpip/onhost/hostcon0.h, tcpip, tcpip411, GOLD410 8/14/90 16:28:00";
 *
 * COMPONENT_NAME: TCPIP hostcon0.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 * RISC System/6000 is a trademark of International Business Machines
   Corporation.
*/
/* Include file for hostconnect.
 */
/*
static char AIXwhatH[] = "@(#)	hostcon0.h	1.7 PASC 1.7";
 */
 
/* debugvar turns debug print on and off or	*/
#define debug(x)  if (debugvar) fprintf x
#define debug2(x)  if (debugvar >= 3) fprintf x
 
/* host system styles and tests */
#define STYLEdefault	0

#define STYLElogfield	1
#define STYLEnologin	(1 == (style & STYLElogfield))

#define STYLEsysfield	6
#define STYLEdef	(0 == (style & STYLEsysfield))
#define STYLEcms	((2 == (style & STYLEsysfield)) || STYLEdef)
#define STYLEtso	(4 == (style & STYLEsysfield))
#define STYLEunk	(6 == (style & STYLEsysfield))
 
typedef enum {
	hostunk, hostcms, hosttso
	} hosttype;
 
typedef enum {
	oncmsMake, oncmsOpen, oncmsClose, oncmsUnmake
	} oncmsConnect;
 
typedef enum {
	CPread, VMread, VMRunning, UndefinedScreen
	} ScreenStatusType;
 
EXTERN int	blockmode;
EXTERN int	havecursor;
EXTERN int	debugvar;
EXTERN FILE	*stddbg;
EXTERN int	errno;
EXTERN int	forkid;
EXTERN int	GotNucxRetCode; /* host sent onhost end of command string */
EXTERN int	listensock; /* socket to listen for oncms process */
EXTERN char	localhost[16];	/*  my hostname */
EXTERN char	*unixhost;	/*  hostname  used by ftp */
EXTERN int	localread, localwrite; /* socket to oncms process */
EXTERN char	*pendofmsg;
EXTERN int	remotesock;	/* socket to VM machine */
EXTERN int 	reclen;
EXTERN int	Running;  /* =1 after we are connected and forked */
EXTERN int	quitanyio;
EXTERN ScreenStatusType ScreenStatus, screenwas;
EXTERN char	sockfile[80];
EXTERN char	wwdir[80];  /* working directory for $FTP substitution */
EXTERN int	SpecialRc;	/* return code from special op */
EXTERN int	ConnectOnly;	/* don't do login */
 
#define	byte(x)	((x)&0377)
 
#define BFSIZE 4096
EXTERN int	automaticlogin; /* 1 during automatic login */
EXTERN int	cursor;
EXTERN char	fromcmsbuf[BFSIZE+1];
EXTERN char	tocmsbuf[BFSIZE];
EXTERN char	fromuserbuf[BFSIZE];
EXTERN char	TOuserbuf[BFSIZE];
 
EXTERN char	c3270[64];
EXTERN int	LastUserField, UserEnterField, UserEnterLen;
EXTERN int	ntocms;
EXTERN int	ntouser, nnltouser;
EXTERN char	userid[9];  /* 8 characters plus \0 */
EXTERN char	password[13];  /* 12 characters plus \0 */
EXTERN char	*phost;
EXTERN int	telluser; /*tell user there has been a cmsstate change*/
EXTERN char	*vmid, *vmpwd;
 
EXTERN int	tn3270;	/* 1 passes data between host and 3278 emulator */
EXTERN int	tn3270n;	/* 0 for standard negotiation with 3278 emulator */
EXTERN int iplrequested;  /* onhost requested ipl of host */
EXTERN char *iplcmd;  /* ipl string from alias */
 
#ifdef	LDSF
EXTERN int	isldsf;		/* using an ldsf connection */
#endif	LDSF
 
typedef enum {  /* host login stages, MUST match StageIs in hostcon1.c */
	GotNothing, GotReady, GotRestart, SentLogin,
	SentPassword, SentIpl, SentExec, WantToQuit,
	GotNucx, StartupFailed, LostHost
	} UserStageType;
 
EXTERN	int	connected;
EXTERN int	keywordi;
EXTERN int	keywordbad;
EXTERN char	*keywords[32];
EXTERN int	keywordindex; /* see findkeyword function */
EXTERN	int	loggingoff;
EXTERN UserStageType UserStage;
 
#define AsciiEsc 0x1b	/* ascii escape character */
 
/* definitions for telnet */
#define IAC		255	/* Interpret as command */
#define DONT		254
#define DO		253
#define WONT		252
#define WILL		251
#define SB		250	/* sub-negotation begin */
#define Go_ahead	249
#define Erase_Line	248
#define Erase_char	247
#define Are_you_there	246
#define Abort_output	245
#define Interrupt_Proc	244
#define Break		243
#define Mark		242
#define NOP		241
#define SE		240	/*  End subnegotiation */
#define EOR		239	/*  End of record */
#define Tablow		239	
 
#define Transmit_binary	  0
#define Echo		  1
#define Terminal_type	 24
#define End_of_record	 25
 
#ifdef DOINIT
char *iactab[] = { "EOR ",
	"SE ", "Nop ","Mark ", "Break", "Interrupt ", "Abort output ",
	"Are you there? ","Erase char ", "Erase line",
	"Go ahead ", "SB ",
	"Will ", "Won't ", "Do ", "Don't ", "IAC "};
char *iac0[] = { "Transmit binary ", "Echo "};
char *iac24[] = { "Terminal type ", "End of record "};
 
int AtoE[256];	/* Compute this as inverse of EtoA */
/* Ebcdic to Ascii Conversion
 * for example EtoA[64] = 32 which an Ascii blank
 */
int EtoA[256] = {
         0,   1,   2,   3, 207,   9, 211, 127,
       212, 213, 195,  11,  12,  13,  14,  15,
        16,  17,  18,  19, 199, 180,   8, 201,
        24,  25, 204, 205, 131,  29, 210,  31,
       129, 130,  28, 132, 134,  10,  23,  27,
       137, 145, 146, 149, 162,   5,   6,   7,
       224, 238,  22, 229, 208,  30, 234,   4,
       138, 246, 198, 194,  20,  21, 193,  26,
        32, 166, 225, 128, 235, 144, 159, 226,
       171, 139, 155,  46,  60,  40,  43, 124,
        38, 169, 170, 156, 219, 165, 153, 227,
       168, 158,  33,  36,  42,  41,  59,  94,
        45,  47, 223, 220, 154, 221, 222, 152,
       157, 172, 186,  44,  37,  95,  62,  63,
       215, 136, 148, 176, 177, 178, 252, 214,
       251,  96,  58,  35,  64,  39,  61,  34,
       248,  97,  98,  99, 100, 101, 102, 103,
       104, 105, 150, 164, 243, 175, 174, 197,
       140, 106, 107, 108, 109, 110, 111, 112,
       113, 114, 151, 135, 206, 147, 241, 254,
       200, 126, 115, 116, 117, 118, 119, 120,
       121, 122, 239, 192, 218,  91, 242, 249,
       181, 182, 253, 183, 184, 185, 230, 187,
       188, 189, 141, 217, 191,  93, 216, 196,
       123,  65,  66,  67,  68,  69,  70,  71,
        72,  73, 203, 202, 190, 232, 236, 237,
       125,  74,  75,  76,  77,  78,  79,  80,
        81,  82, 161, 173, 245, 244, 163, 143,
        92, 231,  83,  84,  85,  86,  87,  88,
        89,  90, 160, 133, 142, 233, 228, 209,
        48,  49,  50,  51,  52,  53,  54,  55,
        56,  57, 179, 247, 240, 250, 167, 255   };
#else ~DOINIT
EXTERN char *iactab[];
EXTERN char *iac0[];
EXTERN char *iac24[];
EXTERN int AtoE[256];
EXTERN int EtoA[256];
#endif ~DOINIT
 
/* these can appear in input to VM */
 
#define EnterKey	0x7d
#define ClearKey	0x6d
#define PA1Key		0x6c
#define PA2Key		0x6d
#define PA3Key		0x6e
#define NoAttnKey	0x60
#define PF1Key		0xf1 /* pf1..9 = f1..f9 */
#define PF10Key		0x7a /* pf10..12 = 7a..7b */
 
/* first character of a new screen */
 
#define	Read3270	0x01
#define	ReadBuffer	0x02
#define	EraseScreen	0x05
#define	AltEraseScreen	0x0d
 
/* characters in output screen */
#define InsCursor	0x13	/* Insert Cursor */
#define RepChar		0x3c	/* repeat to address */
#define SbaChar		0x11	/* Set Buffer address */
#define StartChar	0x1d	/* Start field */
/* see use of RepChar */
#define GescChar	0x08	/* Graphics Escape */
 
#define NDmask		0x0c	/* follows StartChar */
 
/* screen fields */
/* unknown - cms or tso */
#define	unkLastUserField		(24*80 -22)
#define	unkUserEnterField		(24*80 -21)
#define	cmsLastUserField		(22*80 -2)
#define	cmsUserEnterField		(22*80)
 
#define	StatusAreaField		(24*80 -21)
#define	LastScreenField		(24*80 -1)
#define	unkUserEnterLen		StatusAreaField - unkUserEnterField
#define	cmsUserEnterLen		StatusAreaField - cmsUserEnterField
#define	StatusAreaLen		LastScreenField+1 - StatusAreaField
 
#define	Esc	0x25	/* use % as an escape character */
 
#define mRead3270	-1
#define mEraseScreen	-2
#define mAltEraseScreen	-3
#define mInsCursor	-4
#define mRepChar	-5
#define mSbaChar	-6
#define mStartChar	-7
#define	mIAC		-8
#define	mEsc		-9
#define	mReadBuffer	-10
 
#define	mod64(x)	((x)&0x3f)
