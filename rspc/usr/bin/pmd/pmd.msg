$ @(#)01  1.8  src/rspc/usr/bin/pmd/pmd.msg, pwrcmd, rspc41J, 9520A_all  5/16/95  11:49:09
$
$   COMPONENT_NAME: PWRCMD
$
$   FUNCTIONS: Power Management Command
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$     *************************************************
$     **                                             **
$     ** Message Catalog for Power Management Daemon **
$     **                                             **
$     *************************************************
$
$     NOTE:
$       If message lies over more than 2 lines, please put space or TAB
$       at the beginning of other lines than the first line.
$
$
$
$len
$quote "
$
$	Message numbers 0381-100 - 0381-200 are reserved for this file.
$
$set MS_PMD
$
$ *********************************************
$                ERROR MESSAGE 
$ *********************************************
$
ERR_ODM_INIT	   "pmd: 0381-101 Failed to initialize ODM data.\n"	
ERR_ODM_READ	   "pmd: 0381-102 Failed to read ODM data. ( attribute: %1$s\
 )\n"	
ERR_ODM_RD_READ	   "pmd: 0381-103 Failed to read ODM data and residual data.\n"
$
$ Do not translate "Mutex"
$
ERR_MUTEX_LOCK	   "pmd: 0381-104 Failed to lock Mutex.\n"
ERR_MUTEX_UNLOCK   "pmd: 0381-105 Failed to unlock Mutex.\n"
ERR_MUTEX_INIT	   "pmd: 0381-106 Failed to initialize Mutex.\n"
$
$
$
ERR_COND_WAIT      "pmd: 0381-107 Failed to synchronize by the condition\
 variables.\n"
ERR_COND_INIT	   "pmd: 0381-108 Failed to initialize the condition\
 variables.\n"
ERR_COND_BRDCST	   "pmd: 0381-109 Failed to broadcast the condition\
 variables.\n"
$
$
$
ERR_CREATE_THREAD  "pmd: 0381-110 Failed to create the thread.\n"
$
$
$
ERR_CANCEL_THREAD  "pmd: 0381-111 Failed to cancel the thread properly.\n"
$
$
$
ERR_ATTR_INIT      "pmd: 0381-112 Failed to initialize the attribute of\
 threads.\n"
ERR_ATTR_DSTRY     "pmd: 0381-113 Failed to destroy the attribute of\
 threads.\n"
ERR_ATTR_DETACH    "pmd: 0381-114 Failed to set the attribute of threads \
 to undetached state.\n"
$
$
$
ERR_LOCKFILE1	   "pmd: 0381-115 Lockfile for phase1 (%1$s) already exists.\n"
ERR_LOCKFILE2	   "pmd: 0381-116 Lockfile (%1$s) already exists.\n"
ERR_LOCKFILES	   "pmd: 0381-117 Lockfile causes error.\n"
$
$
$
ERR_SET_PARAM	   "pmd: 0381-118 Failed to set the proper data\
 for Power Management.\n"
$
$
$
ERR_FORK	   "pmd: 0381-119 Failed to fork processes.\n"
$
$
$
ERR_HIB_VOL_SEARCH "pmd: 0381-120 Failed to search for the hibernation\
 volume. \n"
ERR_HIB_VOL_INFO   "pmd: 0381-121 Failed to set the hibernation volume\
 information.\n"
ERR_HIB_VOL_REMOVE "pmd: 0381-122 Failed to remove the hibernation volume.\n"
$
$
$
ERR_MEMORY	   "pmd: 0381-123 Failed to allocate memory.\n"
$
$
$
ERR_INVAL_PID      "pmd: 0381-124 Invalid process id for %1$s.\n"
$
$
$
ERR_PROCDATA       "pmd: 0381-125 Failed to get process data.\n"
$
$
$
ERR_RMDEV          "pmd: 0381-126 Failed to unconfigure Power Management.\n"
ERR_LSDEV          "pmd: 0381-127 Power Management Core is not configured.\n"
$
$
$
ERR_BROADCAST      "pmd: 0381-128 Failed to broadcast SIGPM to the process.\
 (PID:%1$d)\n"
$
ERR_OPEN_SOCKET    "pmd: 0381-129 IPC error. Failed to open socket.\n"
ERR_RECV           "pmd: 0381-130 IPC error. Failed to receive data.\n"
ERR_SEND           "pmd: 0381-131 IPC error. Failed to send data.\n"
ERR_BIND           "pmd: 0381-132 IPC error. Failed to bind address.\n"
ERR_VERSION        "pmd: 0381-133 IPC error. Version mismatch. \n"
ERR_INVAL_FUNC     "pmd: 0381-134 IPC error.\
 Invalid function type requested.\n"
ERR_INVAL_CTRL     "pmd: 0381-135 IPC error.\
 Invalid control requested.\n"
$
ERR_BAT_INFO        "pmd: 0381-136 Failed to query battery information.\n"
ERR_BAT_SPRT        "pmd: 0381-137 Battery is not supported.\n"
ERR_BAT_DISC        "pmd: 0381-138 Failed to discharge battery.\n"
$
ERR_XLOCK           "pmd: 0381-139 Failed to invoke lock display program.\n"
ERR_SYNCD           "pmd: 0381-140 Failed to invoke sync daemon.\n"
$
ERR_KILL_SYNCD      "pmd: 0381-141 Failed to kill sync daemon.\n"
ERR_KILL_TTY        "pmd: 0381-142 Failed to kill tty sessions.\n"
ERR_KILL_LFT        "pmd: 0381-143 Failed to kill lft session.\n"
$
$
$
ERR_RES_DATA        "pmd: 0381-144 Failed to read residual data.\n"
$ *********************************************
$            END OF ERROR MESSAGE 
$ *********************************************
$
$ *********************************************
$                WARNING
$ *********************************************
$
WARN_ODM_INIT	   "pmd: 0381-200 Warning: Default value of ODM data is set.\
 ( attribute: %1$s )\n"	
WARN_UNREGISTER    "pmd: 0381-201 Warning: Unregister the process.\
 (PID:%1$d)\n"
$
$ *********************************************
$              END OF WARNING
$ *********************************************
$
$
$ *********************************************
$               INFORMATION 
$ *********************************************
$
INFO_TERMINATE	   "pmd: 0381-251 pmd terminated.\n"	
$
$ *********************************************
$            END OF INFORMATION
$ *********************************************
$
$ EOF






