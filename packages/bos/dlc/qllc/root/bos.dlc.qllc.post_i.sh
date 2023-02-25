#!/bin/bsh
# static char sccsid[] = "@(#)43  1.2  src/packages/bos/dlc/qllc/root/bos.dlc.qllc.post_i.sh, pkgdlc, pkg41J, 9517B_all 4/21/95 13:24:03";
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
#   NAME: bos.dlc.qllc.post_i.sh
#
#   RETURN VALUE DESCRIPTION:
#         return 0 on successful completion
#
#########################################################################

# BASE_DIR must be writable
  BASE_DIR=/lpp/bos.dlc

#------------------------------------------------------------------------
# Issue errupdate with -p (alert flag checking removed) and
#                 with -q (do not create undo file).
# Note: this is not [option].err so that the system will
#       not attempt to issue errupdate with the wrong flags.
#------------------------------------------------------------------------

errupdate -fpq /usr/lpp/bos.dlc/qllc/qllc.err 1>/dev/null 2>&1


#------------------------------------------------------------------------
#  re-issue errupdate with alert flag checking removed to generate the  #
#  proper undo file.  This is a work-around for errupdate since the -f  #
#  flag does not allow the undo file to build properly.  You must use   #
#  the -y flag and specify an alternate template file.                  #
#------------------------------------------------------------------------

cd $BASE_DIR

errupdate -fpy altmpltfile /usr/lpp/bos.dlc/qllc/qllc.err 1>/dev/null
 2>&1

rm -f altmpltfile

exit 0

