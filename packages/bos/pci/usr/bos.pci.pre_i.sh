# COMPONENT_NAME: pkgpci

# ORIGINS: 13, 27

# @(#)85        1.3  src/packages/bos/pci/usr/bos.pci.pre_i.sh, pkgpci, pkg411, GOLD410 3/24/94 18:23:08

#!/bin/bsh
#
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
#########################################################################
#
# Pre-installation Script for AIX on the RS/6000
# Developed by IBM and Locus to allow installation of DOS Server on AIX
# Version 4.
#
# Usage:
#	bos.pci.pre_i
#
# Output:
#	SUCCESS/FAIL exit code
#
# This script handles the pre-installation tasks that are required.
#
#########################################################################

# The only requirement preceeding installation, is to terminate the current
# servers, if in fact they are running.

if [ -x /usr/pci/bin/pcistop ]; then

	/usr/pci/bin/pcistop

	if [ $? -ne 0 ]; then
		exit $EXIT_FAILURE
	fi
fi

exit $EXIT_SUCCESS
