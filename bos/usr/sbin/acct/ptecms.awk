# @(#)31 1.3  src/bos/usr/sbin/acct/ptecms.awk, cmdacct, bos411, 9428A410j 5/12/93 14:24:48
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

BEGIN {
	MAXCPU = 20.0		# report if cpu usage greater than this
	MAXKCORE = 1000.0	# report if KCORE usage is greater than this
	}
NF == 4		{
			printf("\t\t\t\t%s", $1)
			system("dspmsg acct.cat 71 'Time Exception Command Usage Summary'")
		}

NF == 3		{
			system("dspmsg acct.cat 72 '\t\t\t\tCommand Exception Usage Summary'")
		}

NR == 1		{
			MAXCPU = MAXCPU + 0.0
			MAXKCORE = MAXKCORE + 0.0
			system("dspmsg acct.cat 73 '\t\t\t\tTotal CPU > %s or Total KCORE > %s' " MAXCPU " " MAXKCORE)
		}

NF <= 4 && length != 0	{
				next
			}

$1 == "COMMAND" || $1 == "NAME"		{
						print
						next
					}

NF == 10 && ( $4 > MAXCPU || $3 > MAXKCORE ) && $1 != "TOTALS"

NF == 13 && ( $5 + $6 > MAXCPU || $4 > MAXKCORE ) && $1 != "TOTALS"

length == 0
