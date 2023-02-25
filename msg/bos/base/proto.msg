$ ENGLISH_SOURCE_VERSION= 1.5
$  @(#)44 1.1  src/msg/bos/base/proto.msg, bosmsg, msg411, GOLD410 2/9/94 09:11:49
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
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
$ proto's messages

$quote "
$
$ $ Message numbers 565 - 581 reserved for this file.
$
$set MS_PROTO_ERR
$ Do not translate proto.

Msg_USAGE       "Usage: proto Directory [Prefix]\n"
Msg_CDFAIL      "proto: 0653-565 Cannot change to directory %s.\n"
Msg_CDFAILF     "proto: 0653-566 Cannot change to directory %s/%s.\n"
Msg_STATFAIL    "proto: 0653-567 Cannot get status on %s.\n"
Msg_STATFAILF   "proto: 0653-568 Cannot get status on %s/%s.\n"
Msg_READLNKFAIL "proto: 0653-569 Cannot read contents of symbolic link %s.\n"
Msg_NOMEM       "proto: 0653-570 There is not enough memory. %s/%s is ignored.\n"
$ Obsolete message .. Do not translate
Msg_OPNDIRFAIL  "proto: 0653-571 Cannot open the directory.\n"
Msg_INVALFILTYP "proto: 0653-572 The file type %o is not valid.\n"


$set MS_PROTO_MISC

Msg_BYLINE      "%c Prototype file for %s\n"

