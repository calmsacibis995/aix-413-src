$ @(#)44	1.2  src/tcpip/usr/samples/tcpip/onhost/onhost.msg, tcpip, tcpip411, GOLD410 8/14/90 16:32:36
$ static char sccsid[] = "src/tcpip/usr/samples/tcpip/onhost/onhost.msg, tcpip, tcpip411, GOLD410 8/14/90 16:32:36";
$
$ COMPONENT_NAME: TCPIP onhost.msg
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
$
$ INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
$ EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
$ WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
$ LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
$ PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
$ OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
$ IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
$ DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
$ DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
$ CORRECTION.
$
$  RISC System/6000 is a trademark of International Business Machines
$   Corporation.
$
$
$ messages for onhost
$

$quote "

$ Messages used in onhost 
$set ONHOST_ERR

OH_EMUL_WAIT	"hostconnect is waiting for a full-screen emulator\n to call host %s port %s as an IBM-3278-2 emulator\n"
OH_EXECLP_FAIL	"execlp failed for full-screen emulator "
OH_FULL_SCREEN	"(Press enter for full-screen or enter a host line.)\n"
OH_CURR_ALIAS	"current alias is %s\n"
OH_NO_ALIAS		"no current alias\n"
OH_BAD_OPTION	"unrecognized option: %c\n"
OH_CANT_DEBUG	"can not open onhost.debug\n"
OH_NOT_CONN		"The host with alias %s is probably not connected.\n"
OH_ENTER_CMD	"Enter hostconnect -a %s%s to make a connection\n or retry onhost with the alias of an active connection.\n"
OH_ACTIVE_ALIAS	"(ls hostcon.* in your home directory to find an active alias.)\n"
OH_BACKGROUND	"\nHostconnect should be running as a background process at %s.\n\tIf it is not, remove hostcon.%s from your home directory and try again.\n\tOtherwise you can kill hostconnect (do not use kill -9) and try again.\n"
OH_HELP1		"(%s) usage: "	
OH_HELP2		"%s [-?abcd[n]lnpqst] [alias] [command]\n"	
OH_HELP3		"where \ta \t= use alias if present or display current alias\n"
OH_HELP4		"\tb \t= bury all host output\n"	
OH_HELP5		"\tc \t= continue after one onhost command until input end of file\n\t\t\tor interrupt (see note 1) ends onhost\n"	
OH_HELP6		"\td, d0 \t= debug output to stddbg\n"	
OH_HELP7		"\td1 \t= debug output to onhost.debug\n"	
OH_HELP8		"\tl \t= loud display of all host output\n"	
OH_HELP9		"\tn \t= normal line for host, no special treatment (see note 2)\n"
OH_HELP10		"\tp \t= prompt for host input - prompt will show\n\t\t>> for onhost command, > for normal line\n\t\tif no prompt appears, a line can be forced (see note 1)\n"
OH_HELP11		"\tq \t= quiet host input prompt\n"	
OH_HELP12		"\ts \t= show host return code\n"	
OH_HELP13		"\tt \t= test mode for full-screen emulator\n"	
OH_HELP14		"will send command to the host identified by alias or to the\n\tlast used host.  If no command is present, then a full-screen\n\tsession is started using the IBM-3278-2 emulator named %s\n"
OH_HELP15		"note 1: an interrupt (ctrl-c) will terminate onhost if onhost is waiting\n\tfor your input, otherwise onhost will force a line to the host\n"
OH_HELP16		"note 2: a normal line of hat(^) followed by a character produces a\n\tpf1..pf12(1234567890-=), pa1..pa2(ab), enter(e), or clear(z) at the host\n"
OH_CP_READ		"CP read without expert option - onhost requesting ipl\n"
OH_OUT_PHASE	"scanmsg: out of phase nr=%d c=0x%x\n"
