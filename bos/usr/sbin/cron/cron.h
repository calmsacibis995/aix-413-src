/* @(#)03	1.12  src/bos/usr/sbin/cron/cron.h, cmdcntl, bos411, 9428A410j 11/30/92 16:27:49 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 3, 27 ,18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * cron.h   1.9  com/cmd/oper/cron,3.1,9021 4/18/90 09:01:52
 *
 * (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.0
 */
  
#define MINUTE          60L
#define HOUR            60L*60L
#define DAY             24L*60L*60L
#define NQUEUE          26              /* number of queues available */
#define ATEVENT         0               /* queue 'a' has regular(sh) at jobs */
#define BATCHEVENT      1               /* queue 'b' has batch jobs */
#define CRONEVENT       2               /* queue 'c' has cron jobs  */
#define SYNCEVENT       3               /* queue 'd' is a sync     */
#define KSHEVENT        4               /* queue 'e' has ksh at jobs  */
#define CSHEVENT        5               /* queue 'f' has csh at jobs  */

#define ADD             'a'
#define DELETE          'd'
#define AT              'a'
#define CRON            'c'

#define ROOT            0               /* Root userid */

#define FLEN    30
#define LLEN    9
/*****
#define FLEN    15
#define LLEN    9
*******/

/* structure used for passing messages from the
   at and crontab commands to the cron                  */

struct  message {
        char    etype;
        char    action;
        char    fname[FLEN];
        char    logname[LLEN];
} msgbuf;

/* anything below here can be changed */

#define CRONDIR         "/var/spool/cron/crontabs"
#define ATDIR           "/var/spool/cron/atjobs"
#define ACCTFILE        "/var/adm/cron/log"
#define CRONALLOW       "/var/adm/cron/cron.allow"
#define CRONDENY        "/var/adm/cron/cron.deny"
#define ATALLOW         "/var/adm/cron/at.allow"
#define ATDENY          "/var/adm/cron/at.deny"
#define PROTO           "/var/adm/cron/.proto"
#define QUEDEFS         "/var/adm/cron/queuedefs"
#define FIFO            "/var/adm/cron/FIFO"

#define SHELL           "/usr/bin/bsh"   /* shell to execute */
#define CSH             "/usr/bin/csh"
#define KSH             "/usr/bin/ksh"
#define BSH             "/usr/bin/bsh"


#define CTLINESIZE      1000    /* max chars in a crontab line */
#define UNAMESIZE       20      /* max chars in a user name */

#define CRON_NOTHING    0x00000000      /* indicate there is no specific flags*/
#define CRON_SORT_M     0x00000001      /* sorted by submission time */
#define CRON_COUNT_JOBS 0x00000002      /* count of number of jobs */
#define CRON_USER       0x00000004      /* all at jobs for user removed */
#define CRON_PROMPT     0x00000008      /* prompt to validate removal */
#define CRON_QUIET      0x00000010      /* no output is printed */
#define CRON_VERBOSE    0x00000020      /* display all of crontab file */
#define CRON_SORT_E     0x00000040      /* sort in order of execution */
#define CRON_NON_VERBOSE 0x00000080     /* no extra information */
#define CRON_QUE_A      0x00000100      /* queue a */
#define CRON_QUE_B      0x00000200      /* queue b */
#define CRON_QUE_C      0x00000400      /* queue c */
#define CRON_QUE_D      0x00000800      /* queue d */
#define CRON_QUE_E      0x00001000      /* queue e */
#define CRON_QUE_F      0x00002000      /* queue f */

#define REMOVE_CRON     0
#define ADD_CRON        1
#define REMOVE_AT       2
#define ADD_AT          3
