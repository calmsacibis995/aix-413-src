/* @(#)93	1.2  src/bos/usr/sbin/monitord/monitord.h, netls, bos411, 9428A410j 4/4/94 08:12:11  */
/*
 *   COMPONENT_NAME: netls
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
#ifndef _MONITORD_H
#define _MONITORD_H

/*
#define NETLS_DEBUG
*/

#include <sys/types.h>
#include <sys/file.h>
#include <sys/license.h>
#include <stdio.h>
#include <unistd.h>
#include <values.h>
#include <nl_types.h>
#include "fault.h"
#include "uuid.h"
#include "libnetls.h"
#include "monitord_admin_msg.h"
#include "monitord_funcs.h"

/*
#define SOFTSTOP
*/

#define LOCK_FILE	      "/etc/security/monitord_lock"
#define MAX_PIDS 	      4000      /* Max. logins trackable by monitord */
#define DEF_H_INTERVAL        (15 * 60) /* Def. heartbeat interval (seconds) */
#define MAX_H_INTERVAL        MAXINT    /* Max. heartbeat interval (seconds) */
#define LICENSE_NOT_GRANTED   0
#define LICENSE_GRANTED	      1
#define KEYWORD "HEARTBEAT="
#define KEYWORDLEN (sizeof(KEYWORD) - 1)
#define MAX_KEYWORD "INFINITE"
#define MAXCHARS 2000
#define MAX_TOK_SZ 100
#define CONFIG_FILE "/etc/security/monitord.cfg"


/* 
 *  struct used to keep info for licensed logins 
 */
struct pid_lic
  {
  pid_t pid;				/* login process id                 */
  int count;				/* counter in case of recycled pids */
  nls_lic_trans_id_t transid;		/* NetLS license transaction id     */
  };

#endif  /* _MONITORD_H */
