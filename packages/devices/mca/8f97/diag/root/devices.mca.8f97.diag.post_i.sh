#!/bin/ksh
# @(#)05 1.2  src/packages/devices/mca/8f97/diag/root/devices.mca.8f97.diag.post_i.sh, mayaixpackaging, pkg41J, 9513A_all 3/8/95 12:54:01
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.mca.8f97.diag.post_i			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    This is called after an installation has been done	#
#		    and we have the oportunity to do some final work.	#
#		    We add the run_ssa_ela and run_ssa_healthcheck	#
#		    programs to crontab.				#
#									#
#   Licensed Materials - Property of IBM				#
#									#
#   (C) Copyright International Business Machines Corp. 1995.		#
#   All rights reserved.						#
#									#
#   US Government Users Restricted Rights - Use, duplication or		#
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.	#
#									#
#########################################################################

#########################################################################
# These are the files used to update the crontab for root		#
#########################################################################

NewCronFile=./crontab.save$$
ChangedCronFile=./crontab.changed$$

#########################################################################
# Do a forced delete of any of our entries already in the crontab	#
#########################################################################

./${0%%post_i}unpost_i

#########################################################################
# First of all save the original crontab file locally			#
#########################################################################

crontab -l > $NewCronFile

if [[ $? -ne 0 ]]
then
    print -u2 "Failed to create crontab backup."
    exit -1
fi

#########################################################################
# Now add our new lines to this local crontab				#
#########################################################################

cat << EOF >> $NewCronFile
01 5 * * * /usr/lpp/diagnostics/bin/run_ssa_ela 1>/dev/null 2>/dev/null
0 * * * * /usr/lpp/diagnostics/bin/run_ssa_healthcheck 1>/dev/null 2>/dev/null
EOF

if [[ $? -ne 0 ]]
then
    print -u2 "Failed to update crontab backup."
    rm -f $NewCronFile
    exit -1
fi

#########################################################################
# Update the real crontab						#
# Don't check the return code - it gives a 1 if cron is not running.	#
# Check the updates worked by hand 					#
#########################################################################

crontab < $NewCronFile
crontab -l > $ChangedCronFile

if [[ $? -ne 0 ]]
then
    print -u2 "Failed to create crontab check file."
    rm -f $NewCronFile
    exit -1
fi

diff $NewCronFile $ChangedCronFile 1>/dev/null 2>/dev/null

if [[ $? -ne 0 ]]
then
    print -u2 "Failed to update crontab."
    rm -f $NewCronFile
    rm -f $ChangedCronFile
    exit -1
fi

#########################################################################
# Tidy up								#
#########################################################################

rm -f $NewCronFile
rm -f $ChangedCronFile

#########################################################################
# crontab has now been updated up					#
#########################################################################

exit 0
