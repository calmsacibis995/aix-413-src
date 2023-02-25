$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)64        1.1  src/msg/bos/base/makedev.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:23
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 10,27 
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

$
$ messages for makedev
$

$set MS_MAKEDEV
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "

MK_CANT_OPEN    "makedev: 1006-101 Cannot find or open %s.\n"
$  refers to a file.
MK_KJIV         "makedev: 1006-102 This the KJI version.\n"
$ Do not translate KJI, refers to Japanese character support.
MK_NLSV         "makedev: 1006-103 This the NLS version.\n"
$ Do not translate NLS, refers to National language character support.
MK_BAD_CMD      "makedev: 1006-104 %s is not a valid command.\n"
$  refers to an unknown command.
MK_NO_FONT      "makedev: 1006-105 Cannot find or open font %s.\n"
$  refers to the font.
MK_NO_CHAR      "makedev: 1006-106 %2$s in font %1$s is not in the character set.\n"
$  refers to a font and character.
MK_TOO_BIG      "makedev: 1006-107 The font %s too big.\n"
$  refers to a font.
MK_STAT1  "makedev: 1006-108 %1$3s: %2$3d characters, width %3$3d, size %4$3d"
MK_STAT2  "makedev: 1006-109 ; with %1$d multi-byte %2$s, in %3$d %4$s\n"
MK_STAT3  "makedev: 1006-110 %1$3s: %2$3d characters, width %3$3d, size %4$3d\n"
MK_BAD_LIG      "makedev: 1006-111 The %s ligature is not allowed.\n"
MK_NLSQ_OFLO    "makedev: 1006-112 The NLesc sequence is too long.\n"
$ Do not translate NLesc.
MK_NLSQ_BAD     "makedev: 1006-113 The %s NLesc sequence is ignored.\n"
$ Do not translate NLesc.
$ refers to an unknown NLesc sequence.
MK_BAD_HEX      "makedev: 1006-114 The hexadecimal escape sequence \\x%c%c is not valid.\n"
$ refers to the bad hexadecimal escape.
MK_BAD_OCT      "makedev: 1006-115 The octal escape sequence \\%c%c%c is not valid.\n"
$ refers to the bad octal escape.
MK_BAD_CS       "makedev: 1006-116 The code set type %s is not valid.\n"
$ refers to a bad codeset type.
MK_USAGE	"Usage:  1006-117 %1$s DESC\n                 %2$s font_file\n"
$ Used when makedev is called with no arguments.
