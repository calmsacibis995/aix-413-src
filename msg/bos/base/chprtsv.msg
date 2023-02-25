$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)39	1.1  src/msg/bos/base/chprtsv.msg, bosmsg, msg411, GOLD410 2/9/94 08:49:14
$ 
$ messages for chprtsv  
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
$ The range of message numbers assigned to this file are 012-028 
$len 
$quote "         define quote character.
$set MS_CHPRTSV        Define initial set#

INCOMP_SW            "chprtsv: 0822-012 The specified flags are not compatible.\n"
UNK_SWITCH           "chprtsv: 0822-013 %c is not a valid flag.\n"
NOT_PARSED           "chprtsv: 0822-014 A flag was passed with an option that is not expected.\n"
SPEC_SW              "chprtsv: 0822-015 Specify either the -c flag or the -s flag.\n"
SPEC_SW2             "chprtsv: 0822-016 Specify the -a, -b, -q, and -v flags.\n"
SPEC_SW3             "chprtsv: 0822-017 Specify the -a and -q flags.\n"
SPEC_SW4             "chprtsv: 0822-018 Specify the -b, -q, and -v flags.\n"
ERR_EXEC             "%1$s: 0822-019 Cannot run %2$s.\n"
$ Do not move the %1$s 
ERR_OPEN             "%1$s: 0822-020 Cannot open %2$s.\n"
$ Do not move the %1$s 
ERR_READ             "%1$s: 0822-021 Cannot read the %2$s stanza in %3$s.\n"
$ Do not move the %1$s 
$ stanza is a technical term and should not be translated  
$ unless you know the technical meaning. 
$ %2$s is the stanza name it is searching for.   
$ %3$s is the specified stanza file. 
ERR_STANZA           "%1$s: 0822-022 %2$s is not a valid stanza format in %3$s.\n"
$ stanza is a technical term and should not be translated  
$ unless you know the technical meaning. 
$ Do not move the %1$s 
USAGE1               "\n\
Usage: %s {-c|-s} [-d|-i] [-h Host|-H File]\n"
USAGE2               "\t[-x Host|-X File] [-q qentry -v DeviceName]\n"
$ Do not translate qentry. 
USAGE3               "\t-a Attribute=Value... -b Attribute=Value...|-A File]\n"
