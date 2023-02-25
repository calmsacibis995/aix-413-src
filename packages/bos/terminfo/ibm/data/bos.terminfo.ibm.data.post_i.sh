#!/bin/ksh
# @(#)53        1.4  src/packages/bos/terminfo/ibm/data/bos.terminfo.ibm.data.post_i.sh, pkgterm, pkg411, 9435B411a 8/31/94 11:05:53
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: config
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# NAME: bos.terminfo.ibm.data.config.sh
#
# FUNCTION:  Script called to run tic on the .ti files
#
# NOTES: o Currently, dtterm.ti has two new capabilities that tic
#	   doesn't know about.  tic produces warning messages for
#	   each of them but still succeeds (returns a 0).  These
#	   warnings can be ignored, but they may seem confusing.  
#	   To avoid confusion, stderr is redirected to get rid of
#	   the warnings.  The problem is that if there is a real 
#	   error with dtterm.ti, it will not be displayed.  The
#	   solution then is to check the return code; if an error
#	   occurred, then it is run again without redirecting stderr.
#	 o If "$?" is non-zero, then the return code for the script
#	   is whatever the second tic dtterm.ti returns.
#	 o If "$?" is zero, then the colon (a shell no-op command)
#	   sets the return code to 0 (sucess).
#

export TERMINFO=/usr/share/lib/terminfo
tic /usr/share/lib/terminfo/ibm.ti
tic /usr/share/lib/terminfo/dtterm.ti 2>/dev/null
[ "$?" != 0 ] && tic /usr/share/lib/terminfo/dtterm.ti || :
exit $?
