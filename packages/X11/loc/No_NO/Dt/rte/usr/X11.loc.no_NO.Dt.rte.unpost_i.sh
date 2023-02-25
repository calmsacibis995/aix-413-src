#!/bin/ksh
# @(#)16        1.1  src/packages/X11/loc/no_NO/Dt/rte/usr/X11.loc.no_NO.Dt.rte.unpost_i.sh, pkgxcde, pkg411, 9432A411a 8/4/94 20:19:00
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
# FUNCTION: Removes the translated /usr/dt/appconfig/types/${LANG}/*
#           directories so that appmanager will no longer display 
#           translated icon text. 
#           
# RETURN VALUE DESCRIPTION:
#           Nonzero return code on failure.
#

# ------------------------------------------------------------------------
#  Remove all of the subdirectories that dt_appmanager created.
# ------------------------------------------------------------------------
/usr/bin/rm -rf /usr/dt/appconfig/appmanager/no_NO/*

# ------------------------------------------------------------------------
#  Make the changes known to appmanager.
# ------------------------------------------------------------------------
/usr/dt/bin/dtappgather

# ------------------------------------------------------------------------
#  Let installp know that everything's cool.
# ------------------------------------------------------------------------

exit 0
