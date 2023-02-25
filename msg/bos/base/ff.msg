$ ENGLISH_SOURCE_VERSION= 1.7
$ @(#)98	1.1  src/msg/bos/base/ff.msg, bosmsg, msg411, GOLD410 2/9/94 08:58:29
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len
$set Define        Define initial set#



$set MS_FF        Redefine set#

$ $ Message numbers have been assigned a range of 101-110
$ Do not translate %d, %s, %1$d, %1$s, etc ...
$ Do not translate ff.

BADIVAL              "ff: 0506-101 The -i value %d is not positive and is ignored.\n"

BADVALUE             "ff: 0506-102 %1$s is not valid. Values for the %2$c flag must not be zero.\n"

NOINPUT              "ff: 0506-103 Cannot get information about %s.\n"

NOTSPECIAL           "ff: 0506-104 Input file %s is not a device.\n"

STATFAIL             "ff: 0506-105 Cannot get information about %s.\n"

USAGE "Usage: %s [-3MIldsu -V Vfs -i Ilist -p Path -n File\n\
\t-a # -m # -c #] /InputDevice\n"
