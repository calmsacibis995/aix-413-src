$ @(#)62	1.8.2.2  src/bos/usr/sbin/diskusg/diskusg.msg, cmdfs, bos411, 9428A410j 12/16/92 17:12:15
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "         define quote character.
$len
$set Define        Define initial set#
$
$set MS_DISKUSG        Redefine set#
$
$  Do not translate %o %s  %1$s %2$s etc.   
$  These are output formats for file names, directory, etc.   
$  depending on their context within each message.    
$  Do not translate diskusg:
$  Message numbers have been assigned a range of 081-098.  
$
BADUID               "The uid is not valid.\n\
\tFile system: %1$s, inode: %2$u, uid: %3$u\n"
$ uid is user identification

BADUVALUE	"diskusg: 0506-081 The value for the -U flag must be greater than 0.\n"
$ Do not translate -U

CANTOPEN	"diskusg: 0506-082 Cannot find or open %s.\n"

IGLISTOFL	"diskusg: 0506-083 Specify no more than 10 files systems with the -i flag.\n"
$ Do not translate -i   

NOTJFS		"diskusg:  0506-084  %s is not recognized as a JFS filesystem.\n"

NOTSUPJFS	"diskusg:  506-085 %s is not a supported JFS filesystem version.\n"

CORRUPTJFS	"diskusg:  506-086  The %s JFS filesystem super block is corrupted.\n"

CANTRESB	"diskusg:  506-087 Cannot read super block on %s.\n"

NOTBLOCK	"diskusg: 0506-088 %s is not a block device.\n"

STATFAIL	"diskusg: 0506-089 Cannot get information about %s.\n"

USAGE		"Usage:\tdiskusg [ -U NumberUsers ] [  -i  FileListName ] [  -p File ]\n\t[  -u File |  -v ] { -s [ File ... ] | FileSystem ... }\n"

USERLIM		"diskusg: 0506-090 Specify a -U value greater than %1$d\n"
