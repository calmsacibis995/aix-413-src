$ ENGLISH_SOURCE_VERSION= 1.3
$  @(#)96  1.1  src/msg/bos/base/rmusrprof.msg, bosmsg, msg411, GOLD410 2/9/94 09:15:05
$ 
$    COMPONENT_NAME: RCS
$ 
$    FUNCTIONS: none
$ 
$    ORIGINS: 27
$ 
$    IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$    combined with the aggregated modules for this product)
$                     SOURCE MATERIALS
$ 
$    (C) COPYRIGHT International Business Machines Corp. 1993
$    All Rights Reserved
$    US Government Users Restricted Rights - Use, duplication or
$    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$   

$ scmc msg file created Sun May  2 14:39:55 1993
$ user:  elaird

$quote "         define quote character.

$len 
$set MS_rmusrprof        Define initial set#

M_MSG_1          "Usage: rmusrprof -m l -t { TCP/IP | SLIP | SNA } -n Name |\n\
\trmusrprof -m s -t { TCP/IP | SLIP } -n Name |\n\
\trmusrprof -a -m { l | s }\n"
$ 
$  Don't translate the word rmusrprof (the command)
$ 
M_MSG_2              "Unable to obtain a lock on odm the database.\n"
M_MSG_3              "Retrieval of error message failed.\n"
