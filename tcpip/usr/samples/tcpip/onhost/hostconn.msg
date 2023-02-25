$ @(#)42	1.2  src/tcpip/usr/samples/tcpip/onhost/hostconn.msg, tcpip, tcpip411, GOLD410 8/14/90 16:32:19
$ static char sccsid[] = "src/tcpip/usr/samples/tcpip/onhost/hostconn.msg, tcpip, tcpip411, GOLD410 8/14/90 16:32:19";
$
$ COMPONENT_NAME:  TCPIP hostconn.msg
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
$ messages for hostconnect
$

$quote "

$ Messages used in hostcon0.c 
$set HOSTCON0_ERR

HC_BAD_VALUE	"weird value on read fd=%d n=%d max=%d\n"
HC_EMUL_FAIL	"Expected full-screen emulator negotiation has failed.\n" 
HC_UN_AUTH	"\nYour host connection to the alias named %s has been closed\nbecause of an unauthenticated attempt to use it.\n"
HC_UNEXPECTED	"hostconnect received unexpected data.\n"
HC_CONN_ONLY	"Connection only - use onhost to login to the host and execute onhostld.\n"
HC_INTER	"%s got interrupt and closes\n"


$ Messages used in hostcon1.c
$set HOSTCON1_ERR

HC_PUTIAC_FULL	"buffer full in putiacbuf\n"
HC_TERM_TYPE	"cmscon3 expects terminal type\n"
HC_DOIAC_FULL	"buffer full in DoIac\n"
HC_SEND_TERM	"We expect to send terminal type\n"
HC_PROF_KEY	"%s keywords not found in onhost.profil\n"


$ Messages used in hostcon2.c
$set HOSTCON2_ERR

HC_SYS_WRITE	"could not write %s"
HC_HOST_ACCESS	"The host with alias \"%s\" will not be accessed\n"
HC_CANT_STAT	"could not stat %s\n"
HC_CANT_MALLOC	"findprofile: could not malloc\n"
HC_CANT_READ	"could not read file %s\n"
HC_PROF_VER	"onhost.profil Version is not %s, cannot continue\n"
HC_PROF_FOUND	"onhost.profil Version not found, cannot continue\n"
HC_SPCMD_FULL	"findprofile: spcmd table full, size=%d\n"
HC_PROF_LINE	"error in onhost.profil. line beginning %s does not end in ;\n"
HC_PROF_PATH	"hostconnect cannot find onhost.profil in path %s\n"
HC_PROF_FORMAT	"onhost.profil may be in wrong format.\n"
HC_ONHOST_AUTH	"gettimeofday() can't provide onhost authentication"
HC_OPEN_DEBUG	"can not open hostconn.debug\n"
HC_ALREADY_CONN	"The host with alias %s may already be connected.\n"
HC_ENTER_CMD	"Enter onhost -a %s [command] to try to use this host.\n"
HC_HOME_DIR	"%s not in current or home directory - "
HC_DEFAULTS_SET	"Default values have been set.\n"
HC_600_SET	"%s should have 600 access, please chmod\n"
HC_CANT_OPEN	"can not open %s\n"
HC_NO_ALIAS	"alias not found in %s\n"
HC_INTERNET	"internet address not set\n"
HC_USAGE	"(%s) usage: "
HC_PROMPT1	"%s [-?d[n][a alias] [ihost [style]]\n"  
HC_PROMPT2	"where \ta \t= use alias to identify host connection\n"	
HC_PROMPT3	"\td, d0 \t= debug output to stddbg\n"	
HC_PROMPT4	"\td1, d2 \t= debug output to hostconn.debug (d2: more detail)\n"
HC_PROMPT5	"\talias \t= identify entry in onhost.alias file which describes host\n"
HC_PROMPT6	"\tihost \t= Internet address or domain name of host\n"	
HC_PROMPT7	"\tstyle \t= 0 or 2 for CMS, 4 for TSO, 6 for an unknown host type;\n\t\tadd one to bypass automatic login for connection only (then use\n\t\tonhost to login and execute onhostld)\n"
HC_PROMPT8	"will connect to the IBM host described by the alias entry in onhost.alias,\n"
HC_PROMPT9 	"or, if the alias is noalias, to the host named ihost.\n"	
HC_PROMPT10	"(A special connection to a VM peer of AIX/370 will be made if ihost is LDSF.)\n"
HC_EMPTY_ALIAS	"no alias following -a\n"
HC_BAD_OPTION	"unrecognized option: %c\n"
HC_LDSF_USED	"All /dev/ldsf are in use: last dev tried was %s\n"
HC_LOCAL_HOST	"no local host system, specify a remote hostname\n"
HC_READ_ERR	"error reading %s\n"
HC_CANT_ACCESS	"cannot access %s\n"
HC_IN_BIN	"\tDoes bin own /dev/ldsf and hostconnect?\n"
HC_SUID_SET	"\tDoes hostconnect have suid bit set?\n")
HC_CHAR_DEV	"could not open %s as characters device\n"
HC_TCP_MISSING	"telnet/tcp not in /etc/services - using port 23\n"
HC_UNKNOWN_HOST	"%s: unknown host\n"
