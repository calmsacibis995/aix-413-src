$ @(#)47	1.1  src/tcpip/usr/sbin/mkhosts/mkhosts.msg, tcpip, tcpip411, GOLD410 2/15/90 14:49:24
$ 
$ COMPONENT_NAME: TCPIP mkhosts.msg
$ 
$ FUNCTIONS: 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for mkhosts
$

$set MS_MKHOSTS

$quote "

USAGE		"usage: mkhosts [ -v ] file\n"
$		Usage statement

DBM_OPEN_FAIL  	"dbm_open failed: "
$		dbm_open call failed

ALIASES		"store %s, %d aliases\n"
$		verbose information

DBM_STORE_ERR	"dbm_store host address\n"
$		dbm_store call failed

HOST_ENTRIES	"%d host entries, maximum length %d\n"
$		verbose information
