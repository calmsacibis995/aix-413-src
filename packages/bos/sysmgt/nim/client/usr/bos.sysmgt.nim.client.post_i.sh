#!/bin/ksh
#
# @(#)04        1.1  bos.sysmgt.nim.client.post_i.sh, cmdnim, pkg41J  6/15/95  09:39:12
# COMPONENT_NAME: pkgnim
#
# FUNCTIONS: Removes some replaced SMIT entries.  Workaround until the
#            package tools can handle cumulative changes in base levels.
#
# ORIGIN: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#----------------------------------------------------------------#

# This is a workaround to remove some obsolete entries from the smit 
# database when the 413 base level fileset is being installed on top of
# a 411 or 412 client fileset.

ODMDIR=/usr/lib/objrepos odmdelete -o sm_cmd_opt -q"id_seq_num=050 and id=nim_installp_flags" > /dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -o sm_cmd_opt -q"id_seq_num=055 and id=nim_installp_flags" > /dev/null 2>&1

exit 0
