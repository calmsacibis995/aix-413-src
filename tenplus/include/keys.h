/*NOTUSED*/
/* @(#)92	1.5  src/tenplus/include/keys.h, tenplus, tenplus411, GOLD410 7/18/91 12:45:27 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/***** argument types (passed to command handlers as first arg) *****/

struct key {
    POINTER *keyname;
    POINTER *editorname;
    POINTER *ifdefs;
    int fnindex;
    char *functionname;
    int noargs:1,
	emptyarg:1,
	stringarg:1,
	numberarg:1,
	regionarg:1,
	lineregion:1,
	boxregion:1,
	textregion:1,
	modifier:1,
	cursordef:1,
	delstack:1,
	execflag:1,
	canflag:1,
	invtxtflg:1,
	regionhandle:1;
};
#define NOARGS          (0) /* command must be exec'ed w/o argument (COMMAND) */
#define EMPTYARG        (1) /* command accepts empty argument (ENTER COMMAND) */
#define STRINGARG       (2)   /* command accepts string argument              */
#define NUMBERARG       (4)   /* command accepts numeric argument             */
#define LINEREGION    (010)   /* command accepts line region argument         */
#define BOXREGION     (020)   /* command accepts box region argument          */
#define TEXTREGION    (040)   /* command accepts text region argument         */
#define MODIFIER     (0100)   /* command will modify the file                 */
#define CURSORDEF    (0200)   /* ENTER command goes into box region mode      */
#define REGIONARG    (0400)   /* command can be used in regions               */
#define DELSTACK    (01000)   /* command uses the DELETE stack                */
#define EXECFLAG    (02000)   /* command EXECUTE valid in current context     */
#define CANFLAG     (04000)   /* command CANCEL valid in current context      */
#define INVTXTFLG  (010000)   /* command context is invariant text	      */
#define REGIONHANDLE (020000) /* command handled by region input loop         */

/* prototypes of all functions defined in keys.c */

void show_endifs (FILE *, POINTER *);
void show_ifdefs (FILE *, POINTER *);
void construct_sfiles (void);
void construct_ufiles (void);
void sort (void);
void key_free (struct key *);
void get_keys (SFILE *);
POINTER *to_pointer (char *);
char *to_char (char *);
int has_value (char *);


