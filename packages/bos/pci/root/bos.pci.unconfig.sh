# COMPONENT_NAME: pkgpci 

# ORIGINS: 13, 27

# @(#)84        1.3  src/packages/bos/pci/root/bos.pci.unconfig.sh, pkgpci, pkg411, GOLD410 3/24/94 18:22:46

#!/bin/bsh

#   Copyright (c) 1989 - 1994 Locus Computing Corporation
#   All Rights Reserved
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#
#
#
#########################################################################
#
# Un-configuration Script for AIX on the RS/6000
# Developed by IBM and Locus to allow installation/de-installation of DOS
# Server on AIX Version 4.
#
#
# Usage:
#	bos.pci.unconfig
#
# Output:
#	SUCCESS/FAIL exit code
#
#########################################################################

# Initialize some internal values.  These are effectively constants.
#
#	PROD_MARKER	- marker string
#
#	EXIT_SUCCESS	- normal return, no errors occured
#	EXIT_FAILURE	- something went wrong

PROD_MARKER="PCIforDOS"

EXIT_SUCCESS=0
EXIT_FAILURE=1

readonly PROD_MARKER
readonly EXIT_SUCCESS EXIT_FAILURE

# Remove the "start PCI" entry from the inittab file first.  If we can't
# do that, complain, but continue.

rmitab rcpci > /dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "WARNING: Unable to remove \"rcpci\" entry from /etc/inittab."
        exit $EXIT_FAILURE
fi

# Remove any extra entries that we may have created from the inittab file.
# Again, complain about failures, but don't let that stop removing any others.

initTableList=`grep "$PROD_MARKER" /etc/inittab | cut -f1 -d:`
for entryName in $initTableList; do
	rmitab $entryName
	if [ $? -ne 0 ]; then
		echo "WARNING: Unable to remove inittab entry, \"$entryName\"."

                exit $EXIT_FAILURE
	fi
done

exit $EXIT_SUCCESS
