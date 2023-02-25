#!/bin/ksh
# @(#)46        1.2  src/packages/bos/txt/ts/usr/bos.txt.ts.post_i.sh, pkgtxt, pkg411, GOLD410 7/24/93 18:29:30
#
#   COMPONENT_NAME: PKGTXT
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#####################################################################
# NAME: bos.txt.ts.post_i
#
# FUNCTION: script called from instal to do special configuration for
#           Transcript font family generation.
#
#
# RETURN VALUE DESCRIPTION:
#          return code from managefonts
##
#
##############################################################################
#	In response to Defect # 10610, make will no longer be used to install
# 	and/or update the TranScript fonts. Instead the shell script, managefonts,
#	is used to perform that function. Previously managefonts, used 
#	interactively, provided a user interface to the make environment for
#	the TranScript fonts. The same shell script can now take parameters
#	similar to those provided as make targets when using the makefile,
#	Makefile.fonts. So, everywhere make -f Makefile.fonts was used, we
#	can now replace make -f Makefile.fonts with managefonts and pass the
#	same parameters.
##############################################################################
##########
## This procedure is executed after the files have been extracted by
## installp. The following action will be performed:
##
##   - If installp, build default font familes. Currently, the default
##     font families are Times, Courier, and Helvetica. The default is
##     controlled by a target in the Makefile.fonts makefile.
##########
# Ignore user interrupts
trap "" 1 2 3 15

# Message numbers (from inuumsg.msg)
typeset INU_TS_1=93
typeset INU_TS_2=94
typeset INU_TS_3=95
typeset INU_TS_4=96
typeset INU_TS_5=97
typeset INU_TS_6=98
typeset INU_TS_7=99
typeset INU_TS_8=100
typeset INU_TS_9=101

typeset -i rc=0

typeset makeTarget=default

cd /usr/lib/ps/ditroff.font

# Make sure we start from scratch
managefonts cleanall > /dev/null 2>&1
rc=$?

# after cleaning out font families, check for fail status code
# and issue an error message if make failed. If make did not fail,
# procede to building the font familes.

if [ $rc != 0 ] ; then
	# print -ru2 "Update of font families has failed in setup phase."
	/usr/sbin/inuumsg $INU_TS_5
else
	# print -r "Installing font families."
	/usr/sbin/inuumsg $INU_TS_6
	# print -r "This will take a few minutes, please wait."
	/usr/sbin/inuumsg $INU_TS_7
	managefonts ${makeTarget} > /dev/null 2>&1
	rc=$?

	# If make exits with an error, issue an error message and try
	# to clean up.
	if [ $rc != 0 ] ; then
		# print -ru2 "Update of font families has failed during build phase."
		/usr/sbin/inuumsg $INU_TS_8

		# Try to clean up incomplete build
		managefonts cleanall > /dev/null 2>&1
	else
		# print -r "Font installation successful!"
		/usr/sbin/inuumsg $INU_TS_9
	fi
fi

# Restore interrupts
trap 1 2 3 15
exit $rc
