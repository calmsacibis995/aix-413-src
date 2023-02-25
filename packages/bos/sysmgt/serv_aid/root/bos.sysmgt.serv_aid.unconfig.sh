# @(#)04        1.4  src/packages/bos/sysmgt/serv_aid/root/bos.sysmgt.serv_aid.unconfig.sh, pkgserv_aid, pkg41B, 9504A 12/13/94 14:03:22
# COMPONENT_NAME: pkgserv_aid
#
# FUNCTIONS:  unconfig
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1994,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Remove the errclear entries if any

crontab -l | grep -v /usr/bin/errclear | crontab 1>/dev/null 2>/dev/null


# Remove the logsymptom entry from inittab
rmitab logsymp
exit 0


