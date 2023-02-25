# @(#)70        1.4  src/packages/bos/rte/mp/usr/bos.rte.mp.config.sh, pkgbos, pkg411, 9439B411c 9/29/94 12:46:52
#
#   COMPONENT_NAME: pkgbos
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

# The bootinfo -z command will return 0 if UP system or 1 if MP system
# The links should only be added if this is a MP system.

RV=`bootinfo -z`
if [ "$RV" -eq 1 ]
then
	ln -fs /usr/lib/boot/unix_mp /usr/lib/boot/unix || exit 1
        exit 0
fi

if [ "$RV" -eq 0 ]
then
        # This is a valid return code and we can exit clean 
        # There is no work for this script to do.
        exit 0
fi

# if RV is not set to 1 or 0 OR bootinfo fails and doesn't return a valid entry
# then there is nothing to be done.  Exit non-zero, letting installp return an
# error to the screen and fail the install.  If this is a BOS install then the
# system will probably not build a new boot image and the customer will have to
# reinstall or follow the instructions given for a bosboot failure at this point.

exit 1
