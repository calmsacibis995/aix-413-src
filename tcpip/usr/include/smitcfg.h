/* @(#)11	1.2  src/tcpip/usr/include/smitcfg.h, tcpip, tcpip411, GOLD410 5/28/91 15:01:17 */
/*
 * smitcfg.h -  Header file for TCP/IP SMIT routines.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1988
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */                                                                   

#ifndef _H_CFG04
#define _H_CFG04


/*      Return codes for common routines */
#define CFG_SUCC  0            /* successful termination */
#define CFG_EOF   4004         /* end of file encountered */
#define CFG_EXBF -4013         /* input exceeds buffer size */
#define CFG_SZNF -4015         /* stanza not found */
#define CFG_SZBF -4016         /* stanza size exceeds buffer size */
#define CFG_UNIO -4017         /* unrecoverable I/O error */

#define CFG_SPCE -4028         /* malloc failed -- out of space */
#define CFG_FRMT -4029         /* attribute file format corrupted */
#define CFG_EOPN -4030         /* error opening file */
#define CFG_ECLS -4032         /* error closing file */
#define CFG_NSCSI -4040        /* attempt to add duplicate SCSI device   */

/* declarations for standard lengths */

#define MAXVAL   128    /* maximum size for keyword value */
#define MAXKWD   75     /* maximum keyword length */
#define MAXDDISZ 4096   /* maximum size for DDI stanza */
#define MAXATRSZ 2048   /* maximum size for attribute file stanza */
#define MAXDIR   100    /* maximum directory path length */

/*
 *  Structure for character level access to attribute files
 */

typedef struct {

    FILE *sfile;       /* file pointer returned from fopen */
    char *defbuf;      /* pointer to buffer containing default stanza */
    short *defmap;     /* pointer to array of line indexes in default */
    char spath[MAXDIR];/* filename for re-opens */

} CFG__SFT;


CFG__SFT *cfgcopsf();  /* open routine returns structure pointer */


/*      Macro for rewinding attribute file (handles default info)     */
#define cfgcrwnd(sfptr) { \
	if (sfptr->defbuf != NULL) *sfptr->defbuf = '\0'; \
	rewind(sfptr->sfile); \
	}


#endif /* _H_CFG04 */
/*

 *  Structure for display text specification
 */

typedef struct {

    int type;          /* specification type: message number, pointer
			* to text, or null (terminator)
			*/
    union {
	int msgno;     /* message number */
	char *txptr;   /* text pointer   */
    } u;

    int purpose;       /* use to which this text or message is to be put.
			* values are: display only, selection list,
			*   parameter list, prompt, help
			*/
    int selval;        /* value to be returned to the calling routine
			* if this entry is selected from list
			*/
} cfg__dsp;


/*      Type values for cfg__dsp         */
#define DSPTERM 0      /* end of array   */
#define DSPMESG 1      /* message number */
#define DSPTEXT 2      /* text pointer   */

/*      Purpose values for cfg__dsp      */
#define DSPONLY 0      /* display only   */
#define DSPSELL 1      /* selection list: shortest unambiguous match */
#define DSPPARM 2      /* parameter list */
#define DSPROMT 3      /* prompt         */
#define DSPHELP 4      /* help           */
#define DSPXSLL 5      /* selection list: exact match */
#define DSPHEDR 6      /* header text    */

/*      Special message number           */
#define MSGNULL 0      /* empty message  */


/*      Return codes for common routines */
#define CFG_HELP  4001         /* successful - HELP key hit */
#define CFG_QUIT  4002         /* successful - QUIT key hit */
#define CFG_DO    4003         /* successful - DO key hit */
#define CFG_SHOW  4005         /* successful - SHOW key hit */
#define CFG_PRNT  4006         /* successful - PRINT key hit */
#define CFG_RANG -4010         /* message number out of range */
#define CFG_DSPT -4011         /* invalid dspec->type field */
#define CFG_PURP -4012         /* invalid dspec->purpose field */
#define CFG_MAXT -4014         /* maximum text entities exceeded */

#define CFG_VRMC -4027         /* vrmconfig failure */
#define CFG_MNFD -1            /* match not found */
#define CFG_KWNF -4031         /* keyword not found */
#define CFG_BADFLG -4033       /* bad flag value */
#define CFG_VLBF -4034         /* not enough room in value-list buffer */
#define CFG_VNF  -4035         /* value to delete not found in value list */
#define CFG_MDSN -4036         /* minidisk name could not be generated */


/*      Input error generic message number for cfgcldsp()    */
extern  int ldsperr;

/*      Global variable for activating optional keys         */
extern  unsigned short cfgkeys;
/*      Key definitions - each key is defined as a single    */
/*          bit in the cfgkeys variable                      */
#define SHOWKEY 0x0100
#define PRNTKEY 0x0010
/*      Macro for activating a key                           */
#define key_on(key) cfgkeys |= key
/*      Macro for de-activating a key                        */
#define key_off(key) cfgkeys &= (0xffff - key)

/*      Global filename variables for error message inserts  */
char msgc1[MAXDIR];
char msgc2[MAXDIR];
char msgc3[MAXDIR];
int  msgi1;
int  msgi2;


/* structure for interface to checking routines */
struct chkstr {
       char adapter[10];     /* adapter field */
       CFG__SFT *KAFptr;     /* ptr to open KAF file */
};

/* structure for ddi descriptions table */
struct kel {
    char key[10];
    char desc[30];
};

#define KELSIZE sizeof(struct kel)

/* structure for ddi possible options table */
struct oel {
    char key[20];
    char opt[30];
};

#define OELSIZE sizeof(struct oel)

#define DDIDESC "/etc/ddi/descriptions"
#define DDIOPTS "/etc/ddi/options"

#define NONATIVE 6		/* max number of native (internal) disks */

