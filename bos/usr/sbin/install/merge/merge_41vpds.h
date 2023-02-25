/* @(#)75  1.1  src/bos/usr/sbin/install/merge/merge_41vpds.h, cmdinstl, bos411, 9428A410j 1/28/94 14:21:42 */
/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_MERGE_41VPDS
#define _H_MERGE_41VPDS

#include <stdio.h>
#include <locale.h>             /* for call setlocale */
#include <signal.h>             /* for call setlocale */
#include <ulimit.h>             /* for ulimit call */
#include <sys/resource.h>       /* for ulimit call */
#include <sys/types.h>
#include <sys/statfs.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/limits.h>
#include <unistd.h>
#include <errno.h>
#include <inuerr.h>
#include <instlmsg.h>
#include <commonmsg.h>
#include <installpmsg.h>
#include <inulib.h>


   /* definitions */
#define    CMD_LINE_LT   1024
#define    MAX_PATH_LT   1024
#define    CHFS              "/usr/sbin/chfs"
#define    CHFS_FLAGS        "-a size=+" 
#define    PASS        0
#define    FAIL        -1
#define    NULL_C      '\0'

  /* names of the vpds */

#define LPP "lpp"
#define PRODUCT "product"
#define HISTORY "history"
#define INVENTORY "inventory"

#define USR_VPD_PATH "/usr/lib/objrepos"
#define SHARE_VPD_PATH "/usr/share/lib/objrepos"
#define ROOT_VPD_PATH "/etc/objrepos" 
#define LAST_ELEMENT -1 


  /* prototypes */

 boolean get_N_put_table (int tbl_id, char * vpd_src, char * vpd_target);
 long get_size_of_file (long);
 void validate_command_line (int , char **);
 boolean enough_space_for_vpd   (char *src_path, char * target_path, 
                                                 boolean auto_expand);
 long get_free_blocks_in_filesystem (char *path );
 long  get_size_of_vpd   (char *path);
 boolean expand_my_lv (char *fs, long num_of_512_blocks);

#endif /* _H_MERGE_41VPDS */
