#!/bin/ksh
# @(#)35        1.1  src/packages/X11/loc/zh_CN/Dt/rte/usr/X11.loc.zh_CN.Dt.rte.post_i.sh, pkgxcde, pkg41J, 9512A_all 3/15/95 21:36:53
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
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
cd /usr/dt/appconfig/types/zh_CN || exit 1

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
