/* @(#)74	1.4  src/nfs/usr/sbin/yp/revnetgroup/getgroup.h, cmdnfs, nfs411, GOLD410 9/21/89 13:11:59 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc. 
 */

/*      (#)getgroup.h	1.1 88/03/07 4.0NFSSRC SMI   */

struct grouplist {		
	char *gl_machine;
	char *gl_name;
	char *gl_domain;
	struct grouplist *gl_nxt;
};

struct grouplist *my_getgroup();

			
			
