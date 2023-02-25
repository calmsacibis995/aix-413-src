# @(#)67        1.6  src/packages/bos/sysmgt/serv_aid/root/bos.sysmgt.serv_aid.unpost_i.sh, pkgserv_aid, pkg411, 9438C411a 9/23/94 16:42:48
#
# COMPONENT_NAME: pkgserv_aid
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.sysmgt.serv_aid.unpost_i
#
# FUNCTION: script called from instal to do remove the error log
#           codepoint to /var/adm/ras/codepoint.cat
#
#
# RETURN VALUE DESCRIPTION:
#          return code from errinstall
##
#
##########


SAVEFILE=/lpp/bos.sysmgt/deinstl/bos.sysmgt.serv_aid.codept.undo

/usr/bin/errinstall -f -z /var/adm/ras/codepoint.cat $SAVEFILE >/dev/null 2>&1

rm -rf $SAVEFILE 

exit 0
