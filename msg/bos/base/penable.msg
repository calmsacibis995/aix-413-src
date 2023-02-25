$ ENGLISH_SOURCE_VERSION= 1.6
$ @(#)30	1.1  src/msg/bos/base/penable.msg, bosmsg, msg411, GOLD410 2/9/94 09:10:50
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ comment  
$quote "         define quote character.

$len 
$set MS_PENABLE        Define initial set#
$ Message numbers have been assigned a range of 048-060
$ Do not translate penable, %d, %s, %1$d, %1$s, etc .... 

M_CMDNAME            "0481-048 The %s command name is not recognized.\n"

M_OPEN               "0481-049 Cannot open %s.\n"

M_INVALARG           "0481-050 A parameter is not valid.\n"

M_BADREQ             "0481-051 The request for %s is not valid.\n"

M_UPDINIT            "0481-052 Cannot update inittab for %s.\n"
$ Do not translate inittab. 

M_NOACT              "0481-053 No action was taken.\n"

M_NOMEM              "0481-054 There is not enough memory available now.\n"

M_STAT               "0481-055 The stat system call failed on %s.\n"
$ Do not translate stat 

M_NENABLE            "0481-056 Port %s is not enabled. It cannot be locked.\n"
$ Do not translate port. 

M_NLOCK              "Warning: %s is not locked.\n"

M_TMPFILE            "0481-058 Cannot create a temporary file.\n"

M_UPDATE             "0481-059 Cannot update the temporary file.\n"

M_RENAME             "0481-060 Cannot change the name of the temporary file.\n"

M_ENABLE             " ENABLE\n"
$ Leave a space at the beginning of this message. 

M_SHARE              " SHARE\n"
$ Leave a space at the beginning of this message. 

M_DELAY              " DELAY\n"
$ Leave a space at the beginning of this message. 

M_HOLD               " HOLD\n"
$ Leave a space at the beginning of this message. 

M_DISABLE            " DISABLE\n"
$ Leave a space at the beginning of this message. 

M_UNKNOWN            " UNKNOWN\n"
$ Leave a space at the beginning of this message. 

M_UNAVAIL            " UNAVAIL\n"
$ Leave a space at the beginning of this message. 

M_UNDEF              " UNDEF\n"
$ Leave a space at the beginning of this message. 
