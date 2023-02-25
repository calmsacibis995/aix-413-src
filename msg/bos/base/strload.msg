$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)62	1.1  src/msg/bos/base/strload.msg, bosmsg, msg411, GOLD410 2/9/94 09:21:06
$
$ COMPONENT_NAME: (CMDPSE) STREAMS commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for strload.
$ Do not translate strload.
$ Message numbers have been assigned a range of XXX - YYY
$
$quote "		define quote character.
$set MS_STRLOAD		Define initial set#

$ note:
$	Do not translate the following words:
$		open, PSE, unlink, major, odmerr
$
USAGE	"usage: %s [-u|-q] [-h <size>] [-f <file>] [-d <list>] [-m <list>]\n"
MSG1	"cannot open %s"
MSG2	"unknown PSE type '%s'\n"
MSG3	"cannot load %s"
MSG4	"cannot initialize %s"
MSG5	"cannot terminate %s"
MSG6	"cannot unload %s"
MSG7	"cannot initialize %s"
MSG8	"cannot initialize %s"
MSG9	"cannot unload: %s not present"
MSG10	"cannot terminate %s"
MSG11	"cannot unload %s"
MSG12	"cannot unload: %s not present"
MSG13	"cannot terminate %s"
MSG14	"cannot unload %s"
MSG15	"yes"
MSG16	"no"
MSG17	"cannot query %s"
MSG18	"'%s' already loaded\n"
MSG19	"cannot load %s"
MSG20	"cannot get major for %s"
MSG21	"cannot unlink %s"
MSG22	"cannot release major for %s - odmerr %d"
MSG23	"cannot generate major for %s - odmerr %d"
MSG24	"cannot get major for /dev/clone"
MSG25	"cannot generate major for %s - odmerr %d"
MSG26	"cannot unlink %s"
MSG27	"cannot create %s"
MSG28	"cannot create %s"
MSG30	"line %d: only first %d minors used"
MSG31	"line %d: unknown attribute '%c'"
MSG32	"line %d: missing extension type"
MSG33	"line %d: no filename present"
