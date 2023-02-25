$ @(#)41	1.5  src/tcpip/usr/ccs/lib/libffdb/libffdb.msg, tcpip, tcpip411, GOLD410 12/8/92 07:08:54
$ 
$ COMPONENT_NAME:  TCPIP 
$ 
$ ORIGINS: 27 
$ 
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$ 
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$ messages for libffdb 
$ 
$ The range of message numbers assigned to this file are 029-041 
$quote "         define quote character.
$len 
$set MS_LIBFFDB        Define initial set#

$ Do not translate libffdb, %1$s, %2$s, etc.. 
$ Do not move %1$s: this is the program name. 
ERROPEN              "%1$s: 0822-029 Cannot open %2$s.\n"
ERRSTAT              "%1$s: 0822-030 Cannot get status for file %2$s.\n"
ERRACC               "%1$s: 0822-031 Cannot change the access mode of file %2$s.\n"
ERRLOCK              "%1$s: 0822-032 Cannot lock file %2$s.\n"
ERRUNLOCK            "%1$s: 0822-033 Cannot unlock file %2$s.\n"
ERRTMPOPEN           "%1$s: 0822-034 Cannot open the temporary file.\n"
ERRMVTMP             "%1$s: 0822-035 Cannot move the temporary file back to %2$s.\n"
ERRENOENT            "libffdb: 0822-036 The file %s does not exist.\n"
