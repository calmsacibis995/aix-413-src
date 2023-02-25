#!/bin/sh
# @(#)88	1.1  src/tenplus/e2/common/cleandir.sh, tenplus, tenplus411, GOLD410 3/24/93 11:24:10

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
# This shell script is used by the "Housekeep" option of the
# New Task Menu.  It is used to "clean" a single directory.

cd $1

if [ -f $1/cleandir ]
then
    echo $1 cleaned using local cleandir
    $1/cleandir
else
    rm -rf ...* *.bak .*.bak
    rmhist -d -f * .*
fi
