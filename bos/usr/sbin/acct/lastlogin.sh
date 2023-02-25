#! /usr/bin/bsh
# @(#)25 1.7  src/bos/usr/sbin/acct/lastlogin.sh, cmdacct, bos412, 9446C 11/16/94 18:03:27
#
#  COMPONENT_NAME: (CMDACCT) Command Accounting
# 
#  FUNCTIONS: none
# 
#  ORIGINS: 3, 9, 27
# 
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
#	"lastlogin - keep record of date each person last logged in"
#	"bug - the date shown is usually 1 more than it should be "
#	"       because lastlogin is run at 4am and checks the last"
#	"       24 hrs worth of process accounting info (in pacct)"
#	"cleanup loginlog - delete entries of those no longer in"
#	"/etc/passwd and add an entry for those recently added"
#	"line 1 - get file of current logins in same form as loginlog"
#	"line 2 - merge the 2 files; use uniq to delete common"
#	"lines resulting in those lines which need to be"
#	"deleted or added from loginlog"
#	"line 3 - result of sort will be a file with 2 copies"
#	"of lines to delete and 1 copy of lines that are "
#	"valid; use uniq to remove duplicate lines"

PATH=/usr/sbin/acct:/bin:/usr/bin:/etc
CMD=`/bin/basename $0`
cd ${ACCTDIR:-/var/adm}/acct
if test ! -r sum/loginlog; then
	nulladm sum/loginlog
fi
/usr/sbin/lsuser -ca ALL | grep -v "^#" | sed "s/^/00-00-00  /" |\
sort +1 - sum/loginlog | uniq -u +10 |\
sort +1 - sum/loginlog | uniq -u > sum/tmploginlog
cp sum/tmploginlog sum/loginlog
#	"update loginlog"
#       "The following if statement was added in response to"
#       "Defect 87318 to allow runacct to work properly when"
#       "previous dates are specified."
if [ $1 ]
then
        day=$1
        d="`date +%y`-`echo $1 | cut -c1-2`-`echo $1 | cut -c3-4`"
else
        d="`date +%y-%m-%d`"
        day=`date +%m%d`
fi
#       "acctcon1 and acctcon2 must be run to create the"
#       "connection accounting file.  if it does not exist"
#       "give a warning and exit."
if [ ! -f "nite/ctacct.$day" ]; then
        dspmsg acct.cat -s 1 78 '%1$s: run "acctcon1" and "acctcon2" to create ctacct file.\n' "${CMD}"
        exit 1
fi
#	"lines 1 and 2 - remove everything from the total"
#	"acctng records with connect info except login"
#	"name and adds the date"
#	"line 3 - sorts in reverse order by login name; gets"
#	"1st occurrence of each login name and resorts by date"
acctmerg -a < nite/ctacct.$day | \
 sed -e "s/^[^ 	]*[ 	]*\([^ 	]*\)[ 	].*/$d  \1/" | \
 sort -r +1 - sum/loginlog | uniq +10 | sort >sum/tmploginlog
mv sum/tmploginlog sum/loginlog
exit 0
