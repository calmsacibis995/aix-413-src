$ ENGLISH_SOURCE_VERSION= 1.3.1.1
$ @(#)93	1.1  src/msg/bos/base/ipcrm.msg, bosmsg, msg411, GOLD410 2/9/94 09:04:03
$
$ COMPONENT_NAME: (CMDIPC) ipc commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_IPCRM

USAGE1 "Usage: ipcrm [-q msqid] [-m shmid] [-s semid]\n%s\n"

USAGE2 "       [-Q msgkey] [-M shmkey] [-S semkey]."

NOTFOUND "ipcrm: 0515-020 %1$s(%2$s) was not found.\n"
$ %1$s is a keyword for the ipc commands(ipcs and ipcrm).
$ %2$s is a numberic value associated with %1$s.

NOPERM "ipcrm: 0515-021 %1$s(%2$s): The user does not have the appropriate \
privileges to perform this operation.\n"
$ %1$s is a keyword for the ipc commands(ipcs and ipcrm).
$ %2$s is a numberic value associated with %1$s.

UNKNOWN "ipcrm: 0515-022 %1$s(%2$s): %3$s\n"
$ This message does NOT require any translations.
$ %1$s is a keyword for the ipc commands(ipcs and ipcrm).
$ %2$s is a numberic value associated with %1$s.
$ %3$s is an error message from the strerror() subroutine.
