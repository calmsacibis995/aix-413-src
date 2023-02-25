$ ENGLISH_SOURCE_VERSION= 1.1
$ @(#)74       1.1  src/msg/bos/base/chvmode.msg, bosmsg, msg411, 9435B411a 8/30/94 12:44:29
$
$   COMPONENT_NAME: xonwf
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote	"	define quote character.

$set	MS_CHVMODE
MSG_CHVMODE_1	"Can't connect to the X server [%s]\n"
MSG_CHVMODE_2	"Please check DISPLAY environment variable\n"
MSG_CHVMODE_3	"The X server doesn't support video mode extension\n"
MSG_CHVMODE_4	"Can't get the current video mode from the X server\n"
MSG_CHVMODE_5	"Invalid size specify format\n"
MSG_CHVMODE_6	"Monitor [%s] doesn't support the video mode [%s]\n"
MSG_CHVMODE_7	"Specified video mode is rejected by the X server\n"
MSG_CHVMODE_8	"Current video mode information\n"
MSG_CHVMODE_9	"logical screen size  [%dx%d]\n"
MSG_CHVMODE_10	"viewport size        [%dx%d]\n"
MSG_CHVMODE_11	"Active output device "
MSG_CHVMODE_12	"[LCD] "
MSG_CHVMODE_13	"[CRT]"
MSG_CHVMODE_14	"[none]"
MSG_CHVMODE_15	"Invalid option %s is specified\n"
MSG_CHVMODE_16	"Usage: %s\n"
MSG_CHVMODE_17	"Usage: %s {+|-c} {+|-l} {-vsize WIDTHxHEIGHT}\n"
MSG_CHVMODE_18	"No external CRT is presented. Ingore -l option.\n"
MSG_CHVMODE_19	"Enable LCD before desable CRT. Ingore -c option.\n"
