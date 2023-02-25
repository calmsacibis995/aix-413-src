# @(#)82	1.4  src/tcpip/usr/sbin/gated/version.awk, tcprouting, tcpip411, GOLD410 12/6/93 18:06:02
#
#   COMPONENT_NAME: tcprouting
#
#   FUNCTIONS: none
#
#   ORIGINS: 27,36
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#*******************************************************************************
#
#       The origin of this software will be explicitly stated as developed 
#       by Cornell University and its collaborators.
#
#       You agree to use reasonable efforts to provide Cornell University with 
#       all modifications, enhancements and documentation you make to GateD, 
#       including documentation related to your distribution of GateD.  Cornell 
#       University shall have the rights to redistribute the modifications,   
#       enhancements, and documentation without accounting to you.
#
#*******************************************************************************

#
#	$Header: /users/jch/src/gated/src/RCS/version.awk,v 2.0 90/04/16 16:54:34 jch Exp $
#
BEGIN {
	maxfields = 4;
	max = 0; strmax = ""; test =""; local="";
	for (i = 1; i <= maxfields; i++) {
		power[i] = exp(log(10)*(maxfields-i));
	}
}
{
	if (NF >= 3) {
		version = "";
		if ( substr($3,1,6) == "*rcsid" ) {
			version = $7;
                        locked = $11;
			newlock = $12;
		} 
		if ( $2 == "*" ) {
			version = $5;
			locked = $9;
			newlock = $10;
		}
		if ( version == "" ) {
			continue;
		}
                if ( locked == "Locked" || newlock == "Locker:") {
			test = ".development";
		}
		sum = 0;
		num = split(version, string, ".")
		if (num > maxfields) {
			local = ".local";
			num = maxfields;
		}
		for (i = 1; i <= num; i++) {
			sum += string[i]*power[i];
		}
		if ( sum > max ) {
			max = sum;
			strmax = version;
		}
	}
}
END {
	print "#include \"include.h\""
	print "const char *gated_version = \"" strmax local 3.0.2 "\";"
#	print "const char *gated_version = \"" strmax local "\";"
#	print "const char *gated_version = \"" strmax local test "\";"
}

