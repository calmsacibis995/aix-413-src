$ @(#)35	1.5  src/bos/usr/sbin/dfsck/dfsck.msg, cmdfs, bos411, 9428A410j 6/15/90 21:18:57
$
$  COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$  FUNCTIONS: dfsck messages
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_DFSCK        Define initial set#

$ Do not translate dfsck, fsck.

ASIN1                "For example, 1y refers to %s.\n"

CANTCHANG            "dfsck: 0506-050 Cannot change file descriptor.\n"

CANTDUP              "dfsck: 0506-051 Cannot dup tty file descriptor for ttyi\n"
$ Do not translate dup, tty. 

CANTDUP1             "dfsck: 0506-052 Cannot dup tty file descriptor\n"

CANTEXEC             "dfsck: 0506-053 Cannot exec 'fsck' on %s\n"
$ Do not translate fsck, exec.

CANTFORK             "dfsck: 0506-054 Cannot create process to fsck %s\n"

CANTOPEN             "dfsck: 0506-055 Cannot open tty\n"

CANTPIPE1            "dfsck: 0506-056 Cannot create pipe for first fsck\n"

CANTPIPE2            "dfsck: 0506-057 Cannot create pipes for first fsck\n"

CANTPIPE3            "dfsck: 0506-058 Cannot create pipe for second fsck\n"

CANTPIPE4            "dfsck: 0506-059 Cannot create pipes for second fsck\n"

CANTSET              "dfsck: 0506-060 Cannot set O_NDELAY on ttyi\n"
$ Do not translate O_NDELAY. 

CAUGHT               "signal %o caught\n"

CORED                "core dumped\n"

DDONE                ">>> DFSCK DONE <<<\n"

ONEWILL              "1 will identify "

PIPERROR             "pipe error: write to wrong pipe(fsck)\n"

QUEST                "which filesystem? answer '1' or '2'\n"

PRECED1              "\nPrecede every answer with 1 or 2.\n"

TWOWILL              "\n2 will identify "

UNKNCHD              "dfsck: 0506-061 Signal received. The child process is not known.\n"

USAGE                "Usage:  %s [-Options] Filesystem1 ... [-Options] Filesystem2 ... \n"
