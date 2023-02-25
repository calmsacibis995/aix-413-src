#!/bin/sh
# @(#)29	1.6  src/tenplus/e2/common/housekeep.sh, tenplus, tenplus411, GOLD410 7/18/91 12:32:05

# COMPONENT_NAME: (INED)  INed Editor
#
# FUNCTIONS:
#
# ORIGINS: 9,10,27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#
# This shell script is referenced by the "Housekeep" option of the
# New Task Menu.

rm -rf $HOME/.putdir
find $HOME -type d -exec cleandir {} \;
