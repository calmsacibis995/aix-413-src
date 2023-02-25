$ @(#)37        1.6.2.4  src/bos/usr/sbin/fuser/fuser.msg, cmdfs, bos411, 9428A410j 3/8/94 11:46:44
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
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
$quote "         define quote character.

$len 
$set MS_FUSER        Define initial set#

$ Do not translate fuser.
$ Do not translate %s, %c, %d, etc.

ENAMELIST            "fuser: 0506-070 Cannot get information from namelist.\n"

GETPROCER            "fuser: 0506-071 Cannot get process structure.\n"

ILLOPT               "%c option ignored.\n"
$ The following 2 messages are followed by an error message.  

READERR              "fuser: 0506-072 Read failed on file descriptor %d: "
$ Do not translate read.  

SEEKERR              "fuser: 0506-073 Seek failed on file descriptor %d: "
$ Do not translate seek. 

UAREALSK             "fuser: 0506-074 lseek to u area (offset %u) failed.\n"
$ Do not translate lseek, u area.  

UAREAREAD            "fuser: 0506-075 read of u area (segment %d offset %u) failed.\n"

USAGE                "Usage:  %s [-ku] File ...\n"

FINDSELFERR          "fuser: 0506-076 Cannot find fuser in process table.\n"

OPENERR              "fuser: 0506-077 Cannot find or open %s.\n"

SEGKXERR 	     "fuser: 0506-078 Cannot read kernel segment extension value\n"

READPROCERR	     "fuser: 0506-079 Cannot read proc table\n"

READUSERERR	     "fuser: 0506-080 Cannot read user structure.\n"

UBLOCK		     "fuser: 0506-081 Cannot read ublock in /unix\n"

MALLOC               "fuser: 0506-082 There is not enough memory available"
