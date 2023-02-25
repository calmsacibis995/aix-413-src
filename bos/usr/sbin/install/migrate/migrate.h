/* @(#)97  1.7  src/bos/usr/sbin/install/migrate/migrate.h, cmdinstl, bos411, 9428A410j 6/30/94 11:11:28 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_MIGRATE
#define _H_MIGRATE

#include <stdio.h>
#include <fcntl.h>
#include <locale.h>             /* for call setlocale */
#include <signal.h>             /* for call setlocale */
#include <ulimit.h>             /* for ulimit call */
#include <sys/resource.h>       /* for ulimit call */
#include <sys/types.h>
#include <sys/statfs.h>
#include <inulib.h> 
#include <stdlib.h>
#include <ctype.h>
#include <sys/limits.h>
#include <unistd.h>
#include <errno.h>
#include <inuerr.h>
#include <instlmsg.h>
#include <commonmsg.h>
#include <installpmsg.h>


   /* definitions */
#define    CMD_LINE_LT   1024
#define    MAX_PATH_LT   1024
#define    CHFS              "/usr/sbin/chfs"
#define    CHFS_FLAGS        "-a size=+" 
#define    PASS        0
#define    FAIL        -1
#define    NULL_C      '\0'
#define    QUOTE       "\\\""

  /* names of the vpds */

#define LPP "lpp"
#define PRODUCT "product"
#define HISTORY "history"
#define INVENTORY "inventory"

#define USR_VPD_PATH "/usr/lib/objrepos"
#define SHARE_VPD_PATH "/usr/share/lib/objrepos"
#define ROOT_VPD_PATH "/etc/objrepos"
#define LAST_ELEMENT ""

#define TARGET_SUFFIX ".32.add"

  /* prototypes */

 void print_lpp_stanza (lpp_t, FILE *, boolean);
 void print_product_stanza (prod_t, FILE *);
 void print_inventory_stanza (inv_t, FILE *);
 void print_history_stanza (hist_t, FILE *);

 boolean get_lpp_database (char *, char *);
 boolean get_inventory_database (char *, char *);
 boolean get_product_database (char *, char *);
 boolean get_history_database (char *, char *);
 long get_size_of_file (long);
 void validate_command_line (int , char **);
 boolean enough_space_for_vpd   (char *path, int auto_expand);
 boolean enough_space_for_41_vpd (char *path, boolean auto_expand,
                                  long num_of_512_blocks_for_41_vpd);
 long get_free_blocks_in_filesystem (char *path);
 boolean expand_my_lv (char *fs, long num_of_512_blocks);
 char * replace_quotes (char *src, char *rep, FILE *fp);

 int create_exceptions_table (FILE *);
 unsigned int hashname (char *);
 boolean omit_from_vpd (char *);

#endif /* _H_MIGRATE */
