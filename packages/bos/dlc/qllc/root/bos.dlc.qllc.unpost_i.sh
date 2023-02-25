#!/bin/bsh
# @(#)84  1.1  src/packages/bos/dlc/qllc/root/bos.dlc.qllc.unpost_i.sh, pkgdlc, pkg41J, 9517B_all 4/21/95 13:23:20
#
#   COMPONENT_NAME: PKGDLC
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#   NAME: bos.dlc.qllc.unpost_i.sh
#
#   RETURN VALUE DESCRIPTION:
#         return 0 on successful completion
#
############################################################################

# My Variables
  BASE_DIR=/lpp/bos.dlc

# Check to see if there is an error undo file
if [ -s $BASE_DIR/qllc.err.undo ]
then

	#-------------------------------------------------------------
	# Issue errupdate with -p (alert flag checking removed) and
	#                 with -q (do not create undo file).
	# Note: this is not [option].err so that the system will
	#       not attempt to issue errupdate with the wrong flags.
	#-------------------------------------------------------------

	errupdate -qpf $BASE_DIR/qllc.err.undo 1>/dev/null 2>&1

fi
 
exit 0
