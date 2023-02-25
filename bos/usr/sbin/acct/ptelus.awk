# @(#)32 1.3  src/bos/usr/sbin/acct/ptelus.awk, cmdacct, bos411, 9428A410j 5/12/93 14:24:51
#
#  COMPONENT_NAME: (CMDACCT) Command Accounting
# 
#  FUNCTIONS: none
# 
#  ORIGINS: 3, 9, 27
# 
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1993
#  All Rights Reserved
#  Licensed Materials - Property of IBM
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 

BEGIN	{
	MAXCPU = 20.		# report if cpu usage is greater than this
	MAXKCORE = 500.		# report is Kcore usage is greater than this
	MAXCONNECT = 120.	# report if connect time is greater than this
	}
NR == 1	 {
	MAXCPU = MAXCPU + 0
	MAXKCORE = MAXKCORE + 0
	MAXCONNECT = MAXCONNECT + 0
	system("dspmsg acct.cat 74 '\t\t\t\tLogins with exceptional Prime/Non-prime Time Usage\n'")
	system("dspmsg acct.cat 75 '\t\t\t\tCPU > %s or KCORE > %s or CONNECT > %s\n\n\n' " MAXCPU " " MAXKCORE " " MAXCONNECT)
	system("dspmsg acct.cat 76 '\tLogin\t\tCPU (mins)\tKCORE-mins\tCONNECT-mins\tdisk\t# of\t# of\t# Disk\tfee\n'")
	system("dspmsg acct.cat 77 'UID\tName\t\tPrime\tNprime\tPrime\tNprime\tPrime\tNprime\tBlocks\tProcs\tSess\tSamples\n\n'")
	}
$3 > MAXCPU || $4 > MAXCPU || $5 > MAXKCORE || $6 > MAXKCORE || $7 > MAXCONNECT || $8 > MAXCONNECT {
				printf("%d\t%-8.8s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
				}
