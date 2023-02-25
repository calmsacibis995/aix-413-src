$ @(#)25	1.2  src/tcpip/usr/sbin/mkprtsv/mkprtsv.msg, tcpip, tcpip411, GOLD410 2/9/90 09:44:42
$
$ messages for mkprtsv   
$
$ ORIGINS: 27   
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989   
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$
$ The range of message numbers assigned to this file are 153-166 
$quote "         define quote character.
$len 
$set MS_MKPRTSV        Define initial set#

INCOMP_SW            "mkprtsv: 0822-153 The specified flags are not compatible.\n"
UNK_SWITCH           "mkprtsv: 0822-154 %c is not a valid flag.\n"
NOT_PARSED           "mkprtsv: 0822-155 A flag was passed with an option that is not expected.\n"
SPEC_SW              "mkprtsv: 0822-156 Specify either the -c flag or the -s flag.\n"
SPEC_SW2             "mkprtsv: 0822-157 Specify the -a, -b, -q, and -v flags.\n"
ERR_EXEC             "%1$s: 0822-158 Cannot run %2$s.\n"
$ Do not move the %1$s 
ERR_OPEN             "%1$s: 0822-159 Cannot open %2$s.\n"
$ Do not move the %1$s 
ERR_READ             "%1$s: 0822-160 Cannot read the %2$s stanza in %3$s.\n"
$ Do not move the %1$s 
$ stanza is a technical term and should not be translated  
$ unless you know the technical meaning. 
$ %2$s is the stanza name it is searching for.   
$ %3$s is the specified stanza file. 
ERR_STANZA           "%1$s: 0822-161 Stanza format %2$s in %3$s is not valid.\n"
$ stanza is a technical term and should not be translated  
$ unless you know the technical meaning. 
$ Do not move the %1$s 
USAGE1               "\n\
Usage: %s -c [-S] [-q QueueName -v DeviceName -b Attribute=Value...\n"
USAGE2               "\t-a Attribute=Value... | -A File]\n"
USAGE3               "\n\
Usage: %s -s [-S] [-q QueueName -v DeviceName -b AttributeValue...\n"
USAGE4               "\t[-h HostName... | -H FileName]\n"
