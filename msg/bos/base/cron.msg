$ ENGLISH_SOURCE_VERSION= 1.19.1.6
$ @(#)02        1.2  src/msg/bos/base/cron.msg, bosmsg, msg411, 9431A411a 8/1/94 06:34:21
$
$ COMPONENT_NAME: (CMDCNTL) commands needed for basic system needs 
$
$ FUNCTIONS:  
$
$ ORIGINS:  18, 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$ (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.0
$
$quote "         define quote character.

$len 
$set MS_CRON        Define initial set#
$ Message numbers have been assigned a range of 067-141, 151-159 and 166-179
$ Do not translate "at" at the beginning of line.
$ Do not translate %s, %d, %1$s, %1$d, etc ...
$ Do not translate "at","atrm","atq","cron" "cronadm"or "crontab" 
$		(these are command names) 
$ Do not translate any of the specific directory names: 
$ Things like:  /usr/spool/cron/crontabs 

MS_BADCD             "0481-067 Cannot change to the %s directory.\n"
$ %s is repleced by a directory name.

MS_NOREADDIR         "0481-068 Cannot read the %s directory.\n"
$ %s is repleced by a directory name.

MS_BADJOBOPEN        "0481-069 Cannot open the at file.\n"
$ "at" is a job type, it is a unix command (do not translate) 

MS_BADSTAT           "0481-071 Cannot access the %s file.\n"
$ %s is replaced by a file name. 

MS_CANTCDHOME        "0481-072 Cannot change to your home directory.\n"

MS_EOLN              "0481-074 Reached an end-of-line that is not expected.\n"

MS_NOREAD            "0481-075 Cannot read the crontab file.\n"

MS_NOSTDIN           "0481-076 Cannot read from standard input.\n"

MS_OUTOFBOUND        "0481-077 The date or time is either too large or too small.\n"

MS_STDERRMSG         "\n\n\
*****************************************************************\n\
\tcron: The previous message is the standard output\n\
\tand standard error of one of the cron commands.\n"
$ this is informational --  Do not translate cron. 

MS_STDOUTERR         "0481-078 A cron command generated standard output\n\
\tor standard error, but cron could not mail you this output.\n\
\tRemember to redirect the standard output and standard error\n\
\tfor each of your commands.\n"
$ Do not translate cron. 

MS_UNEXPECT          "0481-079 Reached a symbol that is not expected.\n"

MS_NOQUEDEFS         "0481-080 Cannot find or access the /usr/adm/cron/queuedefs file.\n"

MS_NOQDEFS2          "Using the default queue definitions.\n"
$ This one follows the MS_NOQUEDEFS message 

MS_CRSTART           "The cron program has started. The process number is %d.\n"
$ Do not translate cron -- %d will be a number. 

MS_NOFIFO1           "0481-081 Cannot access the named pipe %s.\n"
$ %s is replaced by a file name. 

$ MS_BANGOPEN          "open"
$ Do not translate 

MS_NOFIFO2           "0481-082 Cannot open the named pipe /usr/adm/cron/FIFO.\n"

MS_NOWRITE           "0481-083 Cannot write to %s.\n"
$ %s is replaced by a file or directory name. 

MS_BADTAB            "0481-086 The crontab file contains an error.\n"

MS_IGNORED           "This entry has been ignored.\n"
$ get this message after MS_BADTAB 

MS_QLIMAX            "cron: 0481-087 The %c queue maximum run limit has been reached.\n"
$ do not translate cron
$ %c is replaced by a character 

MS_MAXRUN            "cron: 0481-088 The MAXRUN limit of %d processes has been reached.\n"
$ do not translate cron
$ %d is replaced by a number 

MS_NOFORK            "0481-091 The fork system call failed.\n"
$ Do not translate fork 

MS_BADPID            "0481-092 The %d PID is ignored because it is not expected.\n"
$ %d is replaced by a number 

$ MS_READ              "read"
$ Do not translate 

MS_BADFMT            "0481-094 A message to cron is in the wrong format.\n"
$ do not translate cron

MS_ABORT             "cron failed: "
$ Do not add a newline.  Leave the space at the end. 
$ One of the previous messages will be printed after. 
$ Do not translate cron

MS_ABORTHDR          "** CRON FAILED **"
$ Do not translate cron 

MS_CRRESCHED         "0481-095 The cron job is being rescheduled.\n"
$ Do not translate cron. 

MS_ATRESCHED         "0481-096 The at job is being rescheduled.\n"
$ Do not translate at. 

MS_BADDATE           "0481-098 The specified date is not in the correct format.\n"

MS_CANTCHOWN         "0481-102 Cannot make you the owner of the at job.\n"
$ Do not translate at

MS_CANTOPEN          "0481-103 Cannot open a file in the %s directory.\n"
$ %s is replaced by a directory name. 

MS_CANTLINK          "0481-104 Cannot link the command file.\n"

MS_CANTCWD           "0481-105 Cannot find the current working directory.\n"
$ obsolete message

MS_INVALIDUSER       "0481-106 Your user identification is not valid.\n"

MS_NOTALLOWED        "0481-108 You are not authorized to use the at command.\n"
$ Do not translate at

MS_CRNOTALLOWED      "0481-109 You are not authorized to use the cron command.\n"
$ Do not translate "cron" 

MS_NOTHING           "0481-110 Command usage error; a parameter is missing.\n"

MS_NOTOWNER          "0481-112 You must be the owner of job %s.\n"
$ %s is replaced by the job name.  

MS_NONEXIST          "0481-113 Job name %s does not exist.\n"
$ %s is replaced the job name.  

MS_NOACCESS          "at: 0481-114 Cannot access %s.\n"
$ %s is replaced by a file name.
$ do not translate "at: ".

MS_TOOLATE           "0481-116 The specified time has already passed.\n"

MS_JOBAT             "Job %1$s will be run at %2$s.\n"
$ %1$s is the job number   %2$s is the time  
$ at is NOT the command name, it's just the normal preposition "at" 

MS_QFULL             "0481-118 Cannot accept more jobs into the queue.\n"

MS_NOPROTO           "0481-119 There is no prototype file.\n"

MS_NOCRON            "0481-120 The cron program may need to be started.\n"
$ Do not translate cron. 

MS_MSGQERROR         "0481-121 Cannot open the message queue.\n"

MS_MSGSERROR         "0481-122 Cannot write to the message queue.\n"

MS_BADCREATE         "0481-124 Cannot create the cron file in the /usr/spool/cron/crontabs directory.\n"
$ Do not translate cron 

MS_LINERR            "A line of the crontab file:\n\
\t%1$s\n\
contains the following error:\n\
\t%2$s\n"
$ %1$s is replaced by a line from a file. 
$ %2$s is replaced by the message MS_EOLN or MS_OUTOFBOUND or MS_UNEXPECT 
$ Do not translate cron. 

MS_BTIME             "0481-125 There is an error in the minute or hour specification.\n"

MS_BHOUR             "0481-126 Specify an hour in the range 1 through 12.\n"

MS_BDATE             "0481-127 There is an error in the month or the day specification.\n"

MS_BYEAR             "0481-128 The specified year must be in the range 1970 through 2037.\n"

MS_WORD              "0481-129 %s is not a recognized date or time specification.\n"
$ %s is replaced by a specification of date.

MS_CONFOPTS          "0481-131 The specified flags cannot be used together.\n"

MS_MALLOC            "0481-133 There is not enough memory available now.\n"

MS_CRBADUSAGE        "Usage: crontab [-l|-r|-v|-e|File]\n"
$ Do not translate crontab and flags.

MS_NOKSH             "0481-151 Cannot find or access /usr/bin/ksh.\n"
$ Do not translate "/usr/bin/ksh" 

MS_BADUNLINK         "0481-152 The unlink function failed on the %s file.\n"
$ do not translate unlink 
$ %s is replaced by a file name.

MS_BADSCANDIR        "0481-154 The scandir function failed on /usr/spool/cron/atjobs.\n"
$ Do not translate scandir

MS_ATDELTD           "The %s at file is deleted.\n"
$ do not translate at 
$ %s is replaced by a file name.

MS_DELETE            "Delete job %s?  Enter %s to delete: "
$ first %s is replaced by a job name.
$ second %s is replaced by MS_YESLOW

MS_NONFILE           "0481-159 Specify a job name or a user name.\n"

MS_CRONCNT           "There are %d files in the queue.\n"
$ %d is replaced by a file name.

MS_FILENM            "Crontab file: %s\n"
$ %s is replaced by a message.

MS_SUBTIME           "Crontab file: %s\t Submission time: %s\n"
$ first %s is replaced by a file name.
$ second %s is replaced by a date.

MS_CRDELETD          "Crontab file: %s deleted\n"
$ %s is replaced by a file name.

MS_AUDITER           "%1$s: 0481-166 The auditproc function failed with errno %2$d.\n"
$ %1$s is replaced by the command name.
$ %2$d is replaced by the error no.

MS_ATRMUSAGE         "Usage: atrm [-f|-i] [-] [job #|Username]\n"
$ Do not translate the command "atrm" or the flags

MS_ATQUSAGE          "Usage: atq [-c|-n] [Username...]\n"
$ Do not translate the command "atq" or the flags

MS_CRADMUSAGE1       "Usage: cronadm cron [-l|-r|-v] [Username...]\n"
$ obsolete message

MS_CRADMUSAGE2       "               at [-l Username...|-r Job Number...]\n"
$ obsolete message

MS_YESLOW            "yes\n"

MS_ATRMDEL           "0481-168 Only jobs belonging to user %s may be deleted.\n"
$ %s is replaced by the user name.

MS_NOBLANK           "0481-500 Blank lines are not allowed.\n"

MS_NOCSH             "0481-501 Cannot find or access /usr/bin/csh.\n"
$ Do not translate "/usr/bin/csh"

MS_MSGALLOC          "Cannot allocate message buffer.\n"

MS_CANTCHMOD         "Cannot set the mode of the job file.\n"

MS_ATPRIVS           "Insufficient privileges\n"

MS_CANTPEEK          "You may only look at your jobs.\n"

MS_INSUFFPRIV        "%s: insufficient privileges\n"
$ %s is replaced by the command name.

MS_BADQUE1           "0481-130 Specify the a, b, e or f queue.\n"
$ Do not translate "at", "a", "b", "e", and "f"
$ "a", "b", "e", and "f" are the names of particular queues

MS_USAGE4 "Usage: at [-c|-k|-s|-q {a|b|e|f}] [-m] [-f File] Time [Day] [Increment]\n"
$ Do not translate the command "at" or the flags

MS_USAGE5            "Usage: at -l [-o] [ JobNumber... | -q {a|b|e|f} ]\n"
$ Do not translate the command "at" or the flags

MS_USAGE6            "Usage: at -r [-Fi] JobNumber ... [-u User]\n"
$ Do not translate the command "at" or the flags

MS_USAGE7            "Usage: at -n [User]\n"
$ Do not translate the command "at" or the flags

MS_MAILJOB         "cron: Job %s was run."
$ Do not translate the name "cron"
$ This message is mailed to the user executed at command with -m flag
$ %s is replaced by the job name.
$ Do not add a "\n" to this message, it causes errors with csh.

MS_CRADMUSAGE3       "Usage: cronadm cron {{-l|-v}[Username...]|-r Username...}\n"
$ Do not translate the command "cronadm", cron and the flags

MS_CRADMUSAGE4       "               at {-l Username...|-r {Job Number...|Username...}}\n"
$ Do not translate the "at" and the flags

MS_SETPCRED          "0481-134 Cannot set the process credentials for user %s\n"
$ This is a new messages.
$ %s is replaced with the user name of the crontab file owner.
$ This message is mailed to the "root" user.

MS_USAGE8 "Usage: at [-c|-k|-s|-q {a|b|e|f}] [-m] [-f File] -t [[CC]YY]MMDDhhmm[.SS]\n"
$ Do not translate the command "at" or the flags
$ Do not translate the string, "[[CC]YY]MMDDhhmm[.SS]" 

