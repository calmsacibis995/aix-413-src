#!/bin/ksh
# @(#)95        1.2  src/packages/printers/hpJetDirect/attach/usr/printers.hpJetDirect.attach.unconfig.sh, pkgdevprint, pkg411, GOLD410 1/26/94 12:08:36
#
# COMPONENT_NAME: pkgdevprtrte
#
# FUNCTIONS: none
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
 
#
# NAME:  usr/printers.hpJetDirect.attach.unconfig
#
# FUNCTION: script called from cleanup
#
# RETURN VALUE DESCRIPTION:
#       nonzero return code if it fails
#   

set -o nounset
typeset -r		tmpf=/usr/hpJetDirect-$$
typeset -r		hpjdafl=/usr/lib/lpd/pio/etc/hpJetDirect.attach

# Change the SMIT selector name for 'Add a Print Queue' menu option.
sed -e 's#^[ 	]*add_queue[ 	]*=[ 	]*ps_makhp_prt#	add_queue	=	ps_not_installed_error_ghostName#' ${hpjdafl} >|${tmpf} &&
   mv -f $tmpf $hpjdafl &&
   chmod 444 $hpjdafl &&
   chown root.printq $hpjdafl

exit $?
