#!/bin/ksh
# @(#)34        1.2  src/packages/bos/perf/diag_tool/root/bos.perf.diag_tool.unconfig.sh, pkgperf, pkg411, GOLD410 2/7/94 14:15:26
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
# ... bos.perf.diag_tool.unconfig.sh
#
# shell script to unconfigure PDT
#
#    ASSUMES THIS IS RUN AS ROOT
#
# get a copy of user adm's cron table, 
#   remove all entries in the cron table that refer to the pdt directory
#
cronadm cron -l adm | \
   sed "/ \/usr\/sbin\/perf\/diag_tool\/Driver_ /d" > /tmp/.diag_tool.cron \
      || exit 1
#
# store the new cron table
#
su - adm "-c crontab /tmp/.diag_tool.cron" || exit 1
#
# clean up
#
rm /tmp/.diag_tool.cron || exit 1
exit 0
