# @(#)62	1.5  src/tcpip/usr/samples/tcpip/addrs.awk, tcpip, tcpip411, GOLD410 5/20/91 11:51:34
# 
# COMPONENT_NAME: TCPIP addrs.awk
# 
# FUNCTIONS: 
#
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
#	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
#
#   INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
#   EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
#   WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
#   LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#   PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
#   OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
#   IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
#   DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
#   DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
#   CORRECTION.
#   
#    RISC System/6000 is a trademark of International Business Machines
#      Corporation.
#   
#
# hosts.awk
# to convert /etc/hosts into /etc/named.rev 
# type:
#    addrs.awk /etc/hosts > /etc/named.rev
#
# 	YOU NEED TO HAND EDIT THE named.boot first
#

HOSTNAME=`hostname | cut -d. -f1`
if test -z "$HOSTNAME" 
then echo "$0: ERROR: Hostname not set. " > /dev/console  ; exit 2
fi

DOMAIN=`grep "^domain" /etc/named.boot 2>/dev/null | awk '{print $2}'`

test -z "$DOMAIN" && DOMAIN=`hostname 2>/dev/null | cut -d. -f2-10`

if [ -z "$DOMAIN"  -o "$DOMAIN" = "." -o "$DOMAIN" = "`hostname`" ]
then 
    echo "$0: WARNING !! Cannot find Domain name.\n... setting domain to \".\" ." > /dev/console
    DOMAIN=""
    echo "; setting default domain to ... \".\""
else
    echo "; setting default domain to ... $DOMAIN"
    DOMAIN="$DOMAIN."
fi

awk 'BEGIN { initial = 0 }
initial == 0 { 
	printf("@\t\t9999999 IN\tSOA\t%s.%s root.%s.%s (\n\t\t\t\t\t1.1\t\t; Serial\n\t\t\t\t\t3600\t\t; Refresh\n\t\t\t\t\t300\t\t; Retry\n\t\t\t\t\t3600000\t\t; Expire\n\t\t\t\t\t86400 )\t\t; Minimum\n\t\t9999999 IN\tNS\t%s\n",
		hostname, Domain, hostname, Domain, hostname);	
initial = 1 }

/^[ 	]*[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+/ {
	j = split( $1, adr, "." )
	outadr = sprintf( "%s.%s.%s.%s",adr[4],adr[3],adr[2],adr[1])

	xtract = substr($2,0,index($2,".")) substr(Domain,0,length(Domain) - 1)

	outname = ""

	k = split( $2, names,".")
	if(k == 1) outname = sprintf( "%s.%s", $2, Domain )
	else 
	if ( $2 == xtract ) outname = sprintf("%s.",$2)

	if ( outname != "" ) printf("%s 	IN PTR %s\n", outadr, outname);
} ' Domain=$DOMAIN hostname=$HOSTNAME $1
