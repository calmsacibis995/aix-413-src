#!/bin/ksh
# @(#)04 1.2  src/packages/devices/mca/8f97/diag/root/devices.mca.8f97.diag.unpost_i.sh, mayaixpackaging, pkg41J, 9513A_all 3/8/95 12:53:39
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    devices.mca.8f97.diag.unpost_i			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    This is called after an de-installation is done	#
#		    and we have the oportunity to do some final work.	#
#		    We remove the run_ssa_ela and run_ssa_healthcheck	#
#		    programs from the crontab.				#
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
# First of all save the original crontab file locally			#
#########################################################################

crontab -l | sed '
/\/usr\/lpp\/diagnostics\/bin\/run_ssa_ela/d
/\/usr\/lpp\/diagnostics\/bin\/run_ssa_healthcheck/d' > $NewCronFile

if [[ $? -ne 0 ]]
then
    print -u2 "Failed to create local crontab."
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
# crontab has now been cleaned up					#
#########################################################################

exit 0
