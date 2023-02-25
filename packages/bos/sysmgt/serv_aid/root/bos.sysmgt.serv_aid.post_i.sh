# @(#)66        1.5  src/packages/bos/sysmgt/serv_aid/root/bos.sysmgt.serv_aid.post_i.sh, pkgserv_aid, pkg411, 9436D411a 9/8/94 14:24:47
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
# NAME: bos.sysmgt.serv_aid.post_i
#
# FUNCTION: script called from instal to do add the error log
#           codepoint to /var/adm/ras/codepoint.cat
#
#
# RETURN VALUE DESCRIPTION:
#          return code from errinstall
##
#
##########

# Create the directory if it does not exist.
SAVEDIR=/lpp/bos.sysmgt/deinstl
mkdir -p $SAVEDIR 2>/dev/null

# The bos.sysmgt.serv_aid.copept.undo  will be created in the SAVEDIR directory.
cd $SAVEDIR
/usr/bin/errinstall -f -z /var/adm/ras/codepoint.cat /usr/lpp/bos.sysmgt/serv_aid/bos.sysmgt.serv_aid.codept

exit $?
