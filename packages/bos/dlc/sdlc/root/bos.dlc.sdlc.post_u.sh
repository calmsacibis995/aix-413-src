#!/bin/bsh
# @(#)82  1.1  src/packages/bos/dlc/sdlc/root/bos.dlc.sdlc.post_u.sh, pkgdlc, pkg41J, 9517B_all 4/21/95 13:21:23
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
#   NAME: bos.dlc.sdlc.post_u.sh
#
#   RETURN VALUE DESCRIPTION:
#         return 0 on successful completion
#
############################################################################

# System Variables
  # BASEDIR is expected to be /lpp/bos.dlc
  # SAVEDIR is expected to be /lpp/bos.dlc/bos.dlc.sdlc/<v.r.m.f>
  # UPDATE is expected to be <v.r.m.f>

# My Variables
  DEINSTL_USR_DIR=/usr/lpp/bos.dlc/deinstl
  DEINSTL_ROOT_DIR=$BASEDIR/deinstl
  APPLY_LIST_DIR=$DEINSTL_USR_DIR/bos.dlc.sdlc/$UPDATE

# Check to see if the error format file is being updated
egrep "sdl.err" $APPLY_LIST_DIR/bos.dlc.sdlc.al 1>/dev/null 2>&1

if [ $? -eq 0 ]
then
        # use $SAVEDIR to guarantee that we can write the undo file.
        cd $SAVEDIR

	#----------------------------------------------------------------
	# Issue errupdate with -p (alert flag checking removed)
	# Note1: This builds an undo file in the current directory.        
	# Note2: This is not [option].err so that the system will
	#        not attempt to issue errupdate with the wrong flags.
	#----------------------------------------------------------------

	errupdate -pf /usr/lpp/bos.dlc/sdlc/sdl.err 1>/dev/null 2>&1

fi
 
exit 0
