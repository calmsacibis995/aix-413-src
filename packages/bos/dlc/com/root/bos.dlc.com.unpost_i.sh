#!/bin/bsh
# @(#)78  1.1  src/packages/bos/dlc/com/root/bos.dlc.com.unpost_i.sh, pkgdlc, pkg41J, 9517B_all 4/21/95 13:14:25
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
#   NAME: bos.dlc.com.unpost_i.sh
#
#   RETURN VALUE DESCRIPTION:
#         return 0 on successful completion
#
############################################################################

# My Variables
  BASE_DIR=/lpp/bos.dlc

# Check to see if there is an error undo file
if [ -s $BASE_DIR/lan.err.undo ]
then

	#-------------------------------------------------------------
	# Issue errupdate with -p (alert flag checking removed) and
	#                 with -q (do not create undo file).
	# Note: this is not [option].err so that the system will
	#       not attempt to issue errupdate with the wrong flags.
	#-------------------------------------------------------------

	errupdate -qpf $BASE_DIR/lan.err.undo 1>/dev/null 2>&1

fi

# Check to see if there is a trace undo file
if [ -s $BASE_DIR/dlcfmt.undo.trc ]
then

	#-------------------------------------------------------------
	# Execute the trcupdate cmd.  The trcupdate command takes
        # <fn>.trc as an input file and creates <fn>.undo.trc in the
        # same directory that the input file is located.  For this
        # reason <fn>.trc must be in a writable directory.
	#--------------------------------------------------------------

	trcupdate $BASE_DIR/dlcfmt.undo 1>/dev/null 2>&1

fi
 
exit 0
