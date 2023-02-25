#!/bin/ksh
# @(#)07        1.1  src/packages/bos/perf/diag_tool/usr/bos.perf.diag_tool.pre_rm.sh, pkgperf, pkg411, 9438C411a 9/22/94 14:35:11
#
#   COMPONENT_NAME: pkgperf
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# ... bos.perf.diag_tool.pre_rm.sh
#
# Remove the USR bos.perf.diag_tool.cfgfiles file so that the files
#   can be handled properly by the ROOT bos.perf.diag_tool.cfgfiles .
# The files to be removed are the ones listed in the 410 cfgfiles.S file.
#
# Remove entries from inventory database
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.collection.control" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.files" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.reporting.control" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.reporting.list" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.retention.control" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.retention.list" -o inventory >/dev/null 2>&1
odmdelete -q "loc0 = /var/perf/cfg/diag_tool/.thresholds" -o inventory >/dev/null 2>&1
#
# Remove deinstl/bos.perf.diag_tool.al file
rm -f /usr/lpp/bos.perf/deinstl/bos.perf.diag_tool.al >/dev/null 2>&1
exit 0
