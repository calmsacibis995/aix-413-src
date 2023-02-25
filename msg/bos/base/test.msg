$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)02	1.1  src/msg/bos/base/test.msg, bosmsg, msg411, GOLD410 2/9/94 09:24:37
$
$ COMPONENT_NAME: (CMDSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_TEST
$ Do not translate test.

MISSING_BRACKET         "test: 0402-150 There is a missing ] character.\n"
TOO_MANY_ARGS           "test: 0402-151 Too many parameters specified.\n"
SYNTAX                  "test: 0402-152 Syntax error: a ) character was expected.\n"
PAREN_EXPECTED          "test: 0402-153 Syntax error: a ) character was expected.\n"
UNKNOWN_OP              "test: 0402-154 An operator is missing or is not valid.\n"
ARG_EXPECTED            "test: 0402-155 A parameter is missing.\n"
