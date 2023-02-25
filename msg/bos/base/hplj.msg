$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)69	1.1  src/msg/bos/base/hplj.msg, bosmsg, msg411, GOLD410 2/9/94 09:02:28
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
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ messages for hplj
$

$set MS_HPLJ
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

$quote "
$ Do not add newlines to these messages.
BADDLFONT "1006-301 Font %s not initialized for downloading"
$ refers to font name
BADDATA   "1006-302 Multibyte data is invalid - data ignored"
$
TOO_MANY_FONTS "1006-303 Too many fonts on current page - data ignored"
$
TOO_MANY_DL_FONTS "1006-304 Too many downloaded fonts (maximum %d)"
$
BAD_FLAG   "1006-305 hplj: The %s is not a valid argument."
$ This message is issued when an unknown choice is passed on the
$ command line.
MALLOC_ERR "1006-306 Unable to allocate memory for downloaded font table."
$
