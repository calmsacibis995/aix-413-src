$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)33 1.1 src/msg/bos/base/devinstall.msg, bosmsg, msg411, GOLD410 2/9/94 08:55:09
$ 
$  COMPONENT_NAME: (CMDCFG) 
$ 
$  FUNCTIONS: msg file for devinstall
$ 
$  ORIGINS: 27
$ 
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989,1994
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set 1

1	" running installp command with args %s.\n"
2	" updating diagnostic bundle file %s\n"
3	" updating failed device packages file %s\n"

$set 2

$ in the following, the word "devinstall" is not to be translated
1	"devinstall: 0514-700 Usage error : %s\nUsage: devinstall -f file -d device [-s]\n"
2	"devinstall: 0514-710 There is not enough memory available now.\n"
3	"devinstall: 0514-720 Unable to open the file %s\n"
4	"devinstall: 0514-730 Unable to write the file %s\n"
5	"devinstall: 0514-740 Unable to read the file %s\n"
6	"devinstall: 0514-750 Cannot initialize the ODM data base.\n"
$ ODM is not to be translated

7	"devinstall: 0514-760 Cannot access the PdAt object class in the\n\
\tdevice configuration database.\n"
$ PdAt is not to be translated

