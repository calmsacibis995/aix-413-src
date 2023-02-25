# @(#)28	1.3  src/packages/devices/mca/8fc8/com/usr/devices.mca.8fc8.com.mig.odmdel.sh, pkgdevcommo, pkg411, GOLD410 4/6/94 16:51:44
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS:  migration odm delete file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# The entries to be removed are not saved due to being a migration item.
# These entries are previous release level entries and do not need to be
# restored upon error.  They should be removed from the system because
# a new release level is present.

odmdelete -o sm_cmd_opt -q " id = 'tok_chg' AND id_seq_num = '010' " >/dev/null
odmdelete -o sm_cmd_opt -q " id = 'tok_chg' AND id_seq_num = '030' " >/dev/null
odmdelete -o sm_cmd_opt -q " id = 'tok_chg' AND id_seq_num = '040' " >/dev/null
