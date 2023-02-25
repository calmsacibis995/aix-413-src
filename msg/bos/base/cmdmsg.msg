$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)65	1.2  src/msg/bos/base/cmdmsg.msg, bosmsg, msg411, 9430C411a 7/23/94 12:20:57
$ ****************************************************************************\
$ COMPONENT_NAME: cmdmle
$
$ FUNCTIONS: none
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ ****************************************************************************/
$
$
$quote "
$set 44
1  "SOURCE message catalog"
2  "LOCALE of source message catalog"
3  "TARGET message catalog"
4  "LOCALE of target message catalog"
10 "Convert System Messages"

240 "The message catalog %s was not converted.\n"
$ the %s on the above line will be replaced with a file name
250 "Error : The source locale %s is not installed.\n"
260 "Error : The target locale %s is not installed.\n"
270 "Error : The codeset conversion from %1$s to %2$s failed.\n"
