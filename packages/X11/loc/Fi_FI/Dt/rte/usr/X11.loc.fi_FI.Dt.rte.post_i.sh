#!/bin/ksh
# @(#)22        1.2  src/packages/X11/loc/fi_FI/Dt/rte/usr/X11.loc.fi_FI.Dt.rte.post_i.sh, pkgxcde, pkg411, 9434B411a 8/24/94 13:45:40
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# FUNCTION: Creates the translated /usr/dt/appconfig/types/${LANG}/*
#           directories so that appmanager can display translated
#           icon text. 
#           
# RETURN VALUE DESCRIPTION:
#           Nonzero return code on failure.
#


# ------------------------------------------------------------------------
#  Chg dir to where action and dt_appmanager reside.  Exit if problems.
# ------------------------------------------------------------------------
cd /usr/dt/appconfig/types/fi_FI || exit 1

# ------------------------------------------------------------------------
#  Create the translated dir/file names in the locale-specific directory.
# ------------------------------------------------------------------------
./dt_appmanager . / > /dev/null 2>&1

# ------------------------------------------------------------------------
#  Make the changes known to appmanager.
# ------------------------------------------------------------------------
/usr/dt/bin/dtappgather

# ------------------------------------------------------------------------
#  Let installp know that everything's cool.
# ------------------------------------------------------------------------

exit 0
