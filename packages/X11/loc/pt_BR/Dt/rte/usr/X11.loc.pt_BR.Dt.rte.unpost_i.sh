#!/bin/ksh
# @(#)45        1.1  src/packages/X11/loc/pt_BR/Dt/rte/usr/X11.loc.pt_BR.Dt.rte.unpost_i.sh, pkgxcde, pkg411, 9433B411a 8/11/94 16:28:53
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
/usr/bin/rm -rf /usr/dt/appconfig/appmanager/pt_BR/*

# ------------------------------------------------------------------------
#  Make the changes known to appmanager.
# ------------------------------------------------------------------------
/usr/dt/bin/dtappgather

# ------------------------------------------------------------------------
#  Let installp know that everything's cool.
# ------------------------------------------------------------------------

exit 0
