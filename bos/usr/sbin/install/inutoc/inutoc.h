/* @(#)15       1.5  src/bos/usr/sbin/install/inutoc/inutoc.h, cmdinstl, bos411, 9428A410j 6/10/93 18:31:16 */
/*
 * COMPONENT_NAME: (CMDINSTL) High Level Install Command
 *
 * FUNCTIONS: inutoc.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef  _H_INUTOC
#define  _H_INUTOC

#include <inulib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define         DFLT_TOC_DIR        "/usr/sys/inst.images"
#define         NOTFOUND            0
#define         FOUND               1
#define         UNMARKED            0
#define         MARKED              1
#define         TOC_FILE            ".toc"
#define         Q_FLAG_ON           1
#define         NUM_BYTES           10


#ifndef _NO_PROTO               /* if compiler supports full prototyping */
void    dump_toc                 (void);
void    process_existing_bffs    (char *);
void    remove_toc_entry         (BffRef_t *, BffRef_t *);
int     isa_bff                  (char *);

#else

void    dump_toc                 ();
void    process_existing_bffs    ();
void    remove_toc_entry         ();
int     isa_bff                  ();

#endif                  /* to _NO_PROTO */

#endif                  /* to _H_INUTOC */
