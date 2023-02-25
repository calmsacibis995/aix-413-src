# @(#)64	1.5  src/tcpip/usr/samples/tcpip/hosts.awk, tcpip, tcpip411, GOLD410 8/14/90 15:50:16
# 
# COMPONENT_NAME: TCPIP hosts.awk
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
#	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES
#
# INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
# EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
# WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
# LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
# OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
# IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
# DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
# DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
# CORRECTION.
#
#  RISC System/6000 is a trademark of International Business Machines
#   Corporation.
#
#
# hosts.awk
# to convert /etc/hosts into /etc/named.data 
# type:
#    hosts.awk /etc/hosts > /etc/named.data
#
# 	YOU NEED TO HAND EDIT THE named.boot 
#

HOSTNAME=`hostname | cut -d. -f1`
if test -z "$HOSTNAME" 
then echo "$0: ERROR: Hostname not set. " > /dev/console  ; exit 2
fi

cat <<- EOF
	; name server data file 
	; (also see /etc/named.boot)
	;
	; NAME		TTL	CLASS	TYPE	RDATA
	;
	EOF

DOMAIN=`grep "^domain" /etc/named.boot 2>/dev/null | awk '{print $2}'`

test -z "$DOMAIN" && DOMAIN=`hostname 2>/dev/null | cut -d. -f2-10`

if [ -z "$DOMAIN"  -o "$DOMAIN" = "." -o "$DOMAIN" = "`hostname`" ]
then 
    echo "$0: WARNING !! Cannot find Domain name.\n... setting domain to \".\" ." > /dev/console
    DOMAIN=""
    echo "; setting default domain to \".\"\n;"
else
    echo "; setting default domain to \"$DOMAIN\"\n;"
fi



awk 'BEGIN {
	FS="#";
	printf("@\t\t9999999 IN\tSOA\t%s.%s. root.%s.%s. (\n\t\t\t\t\t1.1\t\t; Serial\n\t\t\t\t\t3600\t\t; Refresh\n\t\t\t\t\t300\t\t; Retry\n\t\t\t\t\t3600000\t\t; Expire\n\t\t\t\t\t86400 )\t\t; Minimum\n\t\t9999999 IN\tNS\t%s\n",
		hostname, Domain, hostname, Domain, hostname);
}

/^[ 	]*[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+\./ 
{
	x = split($1,ARRAY," ")

	FOUND = "FALSE"

	for (i = 2; i <= x ; i++ ) {

   	  xtract = substr(ARRAY[i],index(ARRAY[i],".")+1)

  	  j = split(ARRAY[i],subARR,".")

  	  if ( j != 1 && xtract != Domain )
	   continue  ;

	   if ( FOUND == "FALSE" ) {
	     split(ARRAY[i],savARR,".")
	     FOUND = "TRUE" }

	   if (length(subARR[1]) < 8)
	     tab = "	";  # add extra tab for alignment
	   else
	     tab = "";
	   if ( FOUND == "TRUE" && NF == 2 ) 
	     printf("%s%s\t9999999\tIN\tA\t%s\t;%s\n",subARR[1],tab,ARRAY[1],$2)
	   else if ( FOUND == "TRUE" )
	     printf("%s%s\t9999999\tIN\tA\t%s\n",subARR[1],tab,ARRAY[1])
	   else if ( subARR[1] != savARR[1] ) 
	     printf("%s%s\t9999999\tIN\tCNAME\t%s\n",subARR[1],tab,savARR[1]) 

	     FOUND = "PAST"}

}' Domain=$DOMAIN hostname=$HOSTNAME $1
