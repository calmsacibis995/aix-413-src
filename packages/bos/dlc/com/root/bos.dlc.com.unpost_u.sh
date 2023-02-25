#!/bin/bsh
# @(#)80  1.1  src/packages/bos/dlc/com/root/bos.dlc.com.unpost_u.sh, pkgdlc, pkg41J, 9517B_all 4/21/95 13:14:55
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
#   NAME: bos.dlc.com.unpost_u.sh
#
#   RETURN VALUE DESCRIPTION:
#         return 0 on successful completion
#
############################################################################

# System Variables
  # SAVEDIR is expected to be /lpp/bos.dlc/bos.dlc.com/<v.r.m.f>

# Check to see if there is an error undo file
if [ -s $SAVEDIR/lan.err.undo ]
then

	#--------------------------------------------------------------
	# Issue errupdate with -p (alert flag checking removed) and
	#                 with -q (do not create undo file).
	# Note: this is not [option].err so that the system will
	#       not attempt to issue errupdate with the wrong flags.
	#--------------------------------------------------------------

	errupdate -qpf $SAVEDIR/lan.err.undo 1>/dev/null 2>&1

fi
 
# Check to see if there is a trace undo file
if [ -s $SAVEDIR/dlcfmt.undo.trc ]
then

        #-------------------------------------------------------------
        # Execute the trcupdate cmd.  The trcupdate command takes
        # <fn>.trc as an input file and creates <fn>.undo.trc in the
        # same directory that the input file is located.  For this
        # reason <fn>.trc must be in a writable directory.
        #--------------------------------------------------------------

        trcupdate -o $SAVEDIR/dlcfmt.undo 1>/dev/null 2>&1

fi

exit 0
