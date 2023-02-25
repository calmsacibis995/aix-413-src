# COMPONENT_NAME: pkgpci

# ORIGINS: 13


#     @(#)83       1.5  src/packages/bos/pci/root/bos.pci.config.sh, pkgpci, pkg# 410 4/6/94 %U

# Copyright (c) 1989 - 1994 Locus Computing Corporation
# All Rights Reserved


#########################################################################
# Configuration Script for AIX on the RS/6000
#
# Usage:
#	bos.pci.config
#
# Output:
#	SUCCESS/FAIL exit code
#
# This script will perform the necessary configurations that PC-Interface
# for DOS requires.
#
#########################################################################

# Initialize some internal values.  These are effectively constants.
#
#	PROD_MARKER	- marker string
#
#	DIR_STD_SPOOL	- spooling directory
#
#	PTY_LIST	- PTYs that are initially set to "respawn"
#
#	EXIT_SUCCESS	- normal return, no errors occured
#	EXIT_FAILURE	- something went wrong

PROD_MARKER="PCIforDOS"

DIR_STD_SPOOL="/usr/spool/pcilog"

PTY_LIST="a"

EXIT_SUCCESS=0
EXIT_FAILURE=1

readonly PROD_MARKER
readonly DIR_STD_SPOOL
readonly PTY_LIST
readonly EXIT_SUCCESS EXIT_FAILURE

# Add the PTY entries to the inittab file.  If the entry can't be added, we
# presume that the entry already exists.  In any case, it isn't really fatal,
# so we let it go.

# Add check for ptypa, if it is not there then add it, if it is there do not
# add it.

lsitab ptypa > /dev/null 2>&1
if [ $? -ne 0 ]; then


for x in $PTY_LIST; do
        mkitab "ptyp$x:2:respawn:/usr/sbin/getty /dev/ttyp$x #$PROD_MARKER" \
                                                        > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		exit $EXIT_FAILURE
        fi
done
fi

# When adding the "start PCI" entry, we first check to see if the entry is
# there from a previous installation.  If not, we add it.  However, a
# failure to do this *will* terminate installation.

lsitab rcpci > /dev/null 2>&1
if [ $? -ne 0 ]; then
	mkitab "rcpci:2:once:/etc/rc.pci > $DIR_STD_SPOOL/start.log"
	if [ $? -ne 0 ]; then
		exit $EXIT_FAILURE
	fi
fi

exit $EXIT_SUCCESS


