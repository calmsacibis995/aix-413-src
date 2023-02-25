; @(#)65	1.5  src/tcpip/usr/samples/tcpip/named.boot, tcpip, tcpip411, GOLD410 9/10/93 15:04:21
; 
; COMPONENT_NAME: TCPIP named.boot
; 
; FUNCTIONS: 
;
; ORIGINS: 26  27 
;
; (C) COPYRIGHT International Business Machines Corp. 1985, 1993
; All Rights Reserved
; US Government Users Restricted Rights - Use, duplication or
; disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
;
; Licensed Materials - Property of IBM
;
;
;	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
;
; INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
; EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
; WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
; LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
; PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
; OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
; IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
; DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
; DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
; CORRECTION.
;
;  RISC System/6000 is a trademark of International Business Machines
;   Corporation.
;
;  /etc/named.boot
;
;		boot file for name server
;
; Look in /usr/samples/tcpip for hosts.awk to create named.data
; from the /etc/hosts file, this can be done with the following command:
;	cd /usr/samples/tcpip
;	hosts.awk /etc/hosts > /etc/named.data
;
; Also you will find addrs.awk in the same directory which will create
; named.rev from /etc/hosts, this can be done with the command:
;	cd /usr/samples/tcpip
;	addrs.awk /etc/hosts > /etc/named.rev
;
; You don't need to use the /etc/hosts file. Any file could be used to
;  supply the input provided it follows the same format as /etc/hosts.
;
; BEFORE doing these you should edit this file first and provide your
;  domain entries. 
;
; type		domain			source file or host
;
domain		grandchild.child.parent.top
primary		grandchild.child.parent.top	/etc/named.data
primary		in-addr.arpa			/etc/named.rev
