/* @(#)78	1.3  src/nfs/usr/sbin/yp/revnetgroup/table.h, cmdnfs, nfs411, GOLD410 9/21/89 13:13:13 */
/* src/nfs/usr/sbin/yp/revnetgroup/table.h, cmdnfs, nfs411, GOLD410 9/21/89 13:13:13 */
/* 
 * COMPONENT_NAME: (CMDNFS) Remote Procedure Call Library
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 Sun Microsystems, Inc.
 */

/*      (#)table.h	1.1 88/03/07 4.0NFSSRC SMI   */

#define NUMLETTERS 27 /* 26 letters  + 1 for anything else */
#define TABLESIZE (NUMLETTERS*NUMLETTERS)

typedef struct tablenode *tablelist;
struct tablenode {
	char *key;
	char *datum;
	tablelist next;
};
typedef struct tablenode tablenode;

typedef tablelist stringtable[TABLESIZE];

int tablekey();
char *lookup();
void store();
