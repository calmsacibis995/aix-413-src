$ ENGLISH_SOURCE_VERSION= 1.6.1.1
$ @(#)20  1.1  src/msg/bos/base/libpp.msg, bosmsg, msg411, GOLD410 2/9/94 09:05:17
$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting Services
$
$ FUNCTIONS:
$
$ ORIGINS: 10,27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for libpp
$

$set MS_LIBPP
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "
$ Do not add newlines to these messages.
EARG       "%s is not valid choice."
$  refers an unknown choice.
BADCTRL    "libpp.a: 1006-002 %1$c is not a valid control character (%2$o)."
$  refers unknown control characters.
BADDEVCTRL "libpp.a: 1006-003 %1$c is not a valid device control character (%2$o)."
$  refers unknown device control characters.
BADDRAW  "libpp.a: 1006-004 %1$c is not a valid drawing function (%2$o)."
$  refers unknown drawing functions.
WARN     "Warning. %s: "
$ refers to a troff printer post-processor.
BADFONT  "libpp.a: 1006-006 Font position %1$d is not valid for font %2$s."
$ refers to the font position and font.
BADOPEN  "libpp.a: 1006-007 Cannot find or open the font file %s"
$  refers to the font file.
BADINPUT "libpp.a: 1006-008 Cannot find or open %s."
$  refers to the input file.
PUSH     "libpp.a: 1006-009 The push operator `{' is not implemented."
POP      "libpp.a: 1006-010 The pop operator `}' is not implemented"
NODESC   "libpp.a: 1006-011 Cannot find or open DESC file %s."
$ Do not translate DESC , refers to the description file DESC.out.
FONTDESC "libpp.a: 1006-012 Cannot find font descriptor"
BADALLOC "libpp.a: 1006-013 Memory allocation error"
NOFONTS  "libpp.a: 1006-014 No downloaded fonts to unload"
LOSTFONT "libpp.a: 1006-015 Could not find loaded font"
BADUNLOAD "libpp.a: 1006-016 Cannot unload font from printer memory"
$ Do not translate PK in following messages, refers to the PK bitmap
$		file
PKFILE   "libpp.a: 1006-017 Cannot open font bitmap file %s"
PKPRE    "libpp.a: 1006-018 Invalid PK preamble (%s)"
$ refers to PK bitmap file name
PKFID    "libpp.a: 1006-019 Invalid PK file identification byte (%s)"
$ refers to PK bitmap file name
PKSEEK   "libpp.a: 1006-020 PK file seek error (%s)"
$ refers to PK bitmap file name
PKFLAG   "libpp.a: 1006-021 Cannot read PK file flag byte (%s)"
$ refers to PK bitmap file name
PKCHARPRE "libpp.a: 1006-022 Cannot read PK character preamble (%s)"
$ refers to PK bitmap file name
PKLCHARPRE "libpp.a: 1006-023 Cannot read PK long character preamble (%s)"
$ refers to PK bitmap file name
PKECHARPRE "libpp.a: 1006-024 Cannot read PK extended character preamble (%s)"
$ refers to PK bitmap file name
PKNOCHAR  "libpp.a: 1006-025 Cannot find PK character(%s)"
$ refers to PK bitmap file name
PKBADCMD  "libpp.a: 1006-026 Invalid PK raster command byte -- char %1$d (%2$s)"
$ refers to character number and PK bitmapfile name
PKBADRAS  "libpp.a: 1006-027 PK raster data read error -- char %1$d (%2$s)"
$ refers to character number and PK bitmapfile name
BADCODESET	"libpp.a: 1006-028 Unrecognized codeset `$s'"
$ refers to the codeset string in troff output file
BADCSMATCH	"libpp.a: 1006-029 File codeset `$s' does not match printer codeset"
$ refers to the codeset string in troff output file
ICONVFAIL	"libpp.a: 1006-030 Cannot open converter for %1$s codeset to %2$s"
