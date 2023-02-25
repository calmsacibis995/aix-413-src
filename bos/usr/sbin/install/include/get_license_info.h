/* @(#)13	1.2  src/bos/usr/sbin/install/include/get_license_info.h, cmdinstl, bos411, 9428A410j 1/10/94 13:35:09  */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#include<signal.h>
#include<locale.h>
#include<inudef.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define EXECUTING_PROGRAM	0
#define NO_STATUS_FILE_NAME	1
#define LS_TV_NOT_FOUND		2
#define SERVER_FILE_OPEN_ERROR	3
#define NO_SERVERS_FOUND	4
#define VENDOR_FILE_OPEN_ERROR	5
#define LS_ADMIN_NOT_FOUND	6
#define NO_VENDORS_FOUND	7
#define OUTPUT_FILE_OPEN_ERROR	8
#define NETLS_FILE_OPEN_ERROR	9
#define STAT_CALL_ERROR		10
#define SORT_CMD_ERROR		11
#define STATUS_FILE_OPEN_ERROR	12
#define STATUS_FILE_REMOVE_ERROR	20
#define VENDOR_NAME_LENGTH	32
#define VENDOR_ID_LENGTH	41
#define PRODUCT_NAME_LENGTH	32
#define PRODUCT_VERSION_LENGTH	12
#define BUFFER_LENGTH		81
#define SLEEP_TIME		180

typedef enum {LOOKING_FOR_VENDOR, LOOKING_FOR_PRODUCT, LOOKING_FOR_VEND_PROD,
              FOUND_VENDOR, FOUND_PRODUCT }  states;

static void get_vendor_products (FILE * );
static void get_netls_info (void);
static void process_state (states , char *, FILE *, FILE *);
static void cleanup (void);
static void write_status (int );
static void usage (void);


  /***** Global Variables *****/

int     long_output = FALSE;                /* flag to be set if long output
                                               is needed */
int     dummy, status_flag;
FILE  * statusfile;                         /* File pointer to statusfile. This
                                               file is removed at exit    */
char     progname[17];

char   netls_file_name[L_tmpnam + 1];           /* temporary file */
char   server_file_name[L_tmpnam + 1];          /* temporary file */
char   vendor_file_name[L_tmpnam + 1];          /* temporary file */
char   unsorted_product_file[L_tmpnam + 1];     /* temporary file */


