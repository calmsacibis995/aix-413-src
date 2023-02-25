$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)72	1.1  src/msg/bos/base/ibm3816.msg, bosmsg, msg411, GOLD410 2/9/94 09:02:40
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
$ messages for ibm3816
$

$set MS_IBM3816
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "
$ Do not put newlines on any of these messages.
BAD_FLAG   "ibm3816: 1005-901 The %s is not valid."
$ This message is issued when an unknown choice is passed on the
$ command line.
CANT_LOAD_FONT  "ibm3816: 1005-902 Cannot load the font."
$ This message is issued when the font cannot be loaded.
CANT_OPEN_FILE  "ibm3816: 1005-903 Cannot find or open %s."
$ This message is issued when an fopen fails on the fonts
$ file in /usr/lib/font/devibm3816--this file initializes
$ the font_size structure in ibm3816.
TOO_MANY_FONTS  "ibm3816: 1005-904 Specify no more than %2$d fonts in font file %1$s."
$ This message is issued when there are more than NFONT fonts. 
$ where NFONT equals 10.
BAD_FONT_STYLE  "ibm3816: 1005-905 The %s font style is not allowed."
$ This message is issued when the named font style is not
$ one of the fonts allowed.
BAD_SPEC_SIZE   "ibm3816: 1005-906 The point size for the special font %s is not available. \nPoint size of 10 is being used."
$ This message is issued when an unavailable point size is
$ requested for one of the special fonts--give it 10 point.
BAD_FONT_SIZE   "ibm3816: 1005-907 The point size %1$d for the %2$s font style is not available.\nFont style %3$s is being used."
$ This message is issued when the named font size is not available
$ in the named font, and an alternate font will be used.
BADDATA   "1005-908 Multibyte data is invalid - data ignored"
