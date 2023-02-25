/* @(#)80	1.3  src/nfs/usr/sbin/yp/revnetgroup/util.h, cmdnfs, nfs411, GOLD410 9/21/89 13:13:58 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: FREE, MALLOC, STRCPY, STRNCPY 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc. 
 * 
 */

/*      (#)util.h	1.1 88/03/07 4.0NFSSRC SMI   */

#define EOS '\0'

#ifndef NULL 
#	define NULL ((char *) 0)
#endif


#define MALLOC(object_type) ((object_type *) malloc(sizeof(object_type)))

#define FREE(ptr)	free((char *) ptr) 

#define STRCPY(dst,src) \
	(dst = malloc((unsigned)strlen(src)+1), (void) strcpy(dst,src))

#define STRNCPY(dst,src,num) \
	(dst = malloc((unsigned)(num) + 1),\
	(void)strncpy(dst,src,num),(dst)[num] = EOS) 

extern char *malloc();
extern char *alloca();

char *getline();
void fatal();


