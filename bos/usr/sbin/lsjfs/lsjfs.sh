#!/usr/bin/ksh
# @(#)85	1.1.1.3  src/bos/usr/sbin/lsjfs/lsjfs.sh, cmdfs, bos411, 9428A410j 1/4/94 11:31:38
#
# COMPONENT_NAME: (CMDFS) File System Commands
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
####
#
#	This awk script is used to parse the output of lsfs -qc and 
#	put it in a format acceptable to SMIT.
#
#	The lsfs -qc out put is:
#	#MountPoint:Device:Vfs:Nodename:Type:Size:Options:AutoMount:Acct
#	/:/dev/hd4:jfs::bootfs:8192:rw:yes:no
#  	  (lv size 8192:fs size 8192:frag size 4096:nbpi 4096:compress no)
#
#	It needs to be translated into:
#	#MountPoint:Device:Vfs:Nodename:Type:Size:Options:AutoMount:Acct:\
#		OtherOptions:LvSize:FsSize:FragSize:Nbpi:Compress:
# 	/:/dev/hd4:jfs::bootfs:8192:rw:yes:no::8192:8192:4096:4096:no:
#
####

/usr/sbin/lsfs -cq $* 2> /dev/null | awk -F: '

BEGIN {
		#
		# If the query from lsfs -q fails then then there
		# will be no parenthetical list. The query_good
		# flag will be set any time the parenthetical list
		# is encountered.
		#
	query_good = 0
}

	#
	# Match the first line; it begins w/ "#"
	#
NR == 1 && $1 ~ /\#.+/ {
		#
		# Print out the lsjfs header
		#
	printf ("%s%s\n", $0, \
		":OtherOptions:LvSize:FsSize:FragSize:Nbpi:Compress:");
	continue;
}
	#
	# Look at JFS only
	#	
NR >  1 && $3 == "jfs" {

	if (NR > 2)
	{
		if (query_good)
		{
			printf ("\n")
			query_good = 0 
		}
		else
			printf (":::::\n")
	} 

		#
		# Print first 6 fields
		# MountPoint:Device:Vfs:Nodename:Type:Size
		#
	for (i = 1; i < 7; ++i)
		printf ("%s:", $i);
		#
		# Pull nodev and nosuid out of 7th field if present;
		# these will be printed under OtherOptions; this
		# is a smit requirement.
		#
	sep = ""
	other["nodev"] = ""
	other["nosuid"] = ""

	n = split($7, options, ",");
	for (i = 1; i <= n; ++i)
	{
		if (options[i] == "nodev" || options[i] == "nosuid")
			    other[options[i]] = 1;
		else 
		{
			printf ("%s%s", sep, options[i]);
			sep = ","
		}
	}
	
	for (i = 8; i <= NF; ++i)
		printf (":%s", $i);
		
	if (other["nodev"] && other["nosuid"])
		printf (":nodev,nosuid:");
	else if (other["nodev"])
		printf (":nodev:");
	else if (other["nosuid"])
		printf (":nosuid:");
	else 
		printf ("::");

	continue

} 
	#
	# Process the filesystem geometry numbers (size, frag, nbpi, compress)
	#
NR > 1 && $1 ~ /[" "\t]*\(.+[0-9]+/ {

	gsub (/[\(\)]/, "");
	#
	# pick out each value out of the parenthetical list. 
	#
	num_fields = split ($0, fields, ":")
	for (i = 1; i <= num_fields; i++)
	{
		num_values = split (fields[i], values, " ");
		printf ("%s:", values[num_values]);
	}
	query_good = 1
}
	
END {
	if (query_good)
		printf ("\n")
	else
		printf (":::::\n")
}'
