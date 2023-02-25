/* @(#)37       1.6  src/bos/usr/sbin/install/bffcreate/bffcreate.h, cmdinstl, bos411, 9428A410j 2/26/94 16:53:34 */

/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _BFFCREATE_H
#define _BFFCREATE_H

#include <inulib.h>
#include <sys/stat.h>
#include <inu_dev_ctl.h>

#define  DFLT_DESTDIR      "/usr/sys/inst.images"
#define  DFLT_WORKDIR      "/tmp"
#define  DFLT_DEVICE       "/dev/rfd0"
#define  MARKED            1
#define  UNMARKED          0

typedef struct
{
   char      prodname[MAX_LPP_FULLNAME_LEN];
   Level_t   level;
   char      content;
}prod_lev;


/* -----------------------------------------------------------------
                     P R O T O T Y P E S
   -----------------------------------------------------------------*/

void  mark_toc_as_notfnd          (TOC_t *);
int   mark_lpps                   (TOC_t *, int, char *[], int, char *);
int   bff                         (TOC_t *, char *,  char *);
int   get_service_num             (BffRef_t *, char *, int *, int);
int   space_message               ();
void  system_message              (char *, char *);
char  *inu_get_bff_name           (Option_t *, char *);
int   inu_do_file                 (BffRef_t *, char *);
int   inu_do_stacked_diskette     (BffRef_t *, char *);
int   inu_do_tape                 (BffRef_t *, char *);
int   inu_do_non_stacked_diskette (BffRef_t *, char *, char *);
int   ensure_dir_exists           (char *);
int   comp_func                   (const void * first_arg,
                                   const void * second_arg);
void  list_media_contents         (TOC_t * toc);
int   mark_lpps_driver            (TOC_t *toc_ptr, int argc, char *argv[],
                                   char *device,   int cur_index);
int   check_for_all_keyword       (int * all_keyword, char * argv[], int argc,
                                   int cur_index);
#endif    /* of _BFFCREATE_H */
