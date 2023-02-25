# @(#)05        1.3  src/packages/bos/terminfo/ibm/data/bos.terminfo.ibm.data.pre_rm.sh, pkgterm, pkg411, 9438C411a 9/23/94 09:36:34 
#
# COMPONENT_NAME: pkgterm
#
# FUNCTIONS: pre_rm
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
# NAME: bos.terminfo.ibm.data.pre_rm.sh
#
# FUNCTION:  Script is executed prior to the removal of any terminfo
#	     files and software vital product data.  It is needed for
#	     system migration purposes to remove files belonging
#	     to a fileset other than terminfo in a previous version of AIX.  
#
#	     The following script removes /usr/share/lib/terminfo/dtterm.ti
#	     and /usr/share/lib/terminfo/d/dtterm from the system inventory and
#	     from the X11.Dt.rte.al used by deinstall.  bos.terminfo.ibm.data
#	     will re-install dtterm.ti.  dtterm will be recreated by
#	     bos.terminfo.ibm.data.post_i.sh. 
#

ODMDIR=/usr/lib/objrepos odmdelete -q "loc0=/usr/share/lib/terminfo/dtterm.ti" \
	 -o inventory >/dev/null 2>&1
ODMDIR=/usr/lib/objrepos odmdelete -q "loc0=/usr/share/lib/terminfo/d/dtterm" \
	 -o inventory >/dev/null 2>&1

APPLYLIST=/usr/lpp/X11.Dt/deinstl/X11.Dt.rte.al
if [  -f "$APPLYLIST" ] ; then
        if egrep -q 'terminfo/.*dtterm' $APPLYLIST 1>/dev/null 2>&1
        then
                sed -e "/\/terminfo.*\/dtterm/d" $APPLYLIST >./apply.tmp 2>&1
                mv ./apply.tmp $APPLYLIST
        fi
fi
exit 0


