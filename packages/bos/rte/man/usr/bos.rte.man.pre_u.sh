#!/usr/bin/ksh
# SCCS_ID = "@(#)38	1.1  src/packages/bos/rte/man/usr/bos.rte.man.pre_u.sh, pkgbossub, pkg41J, 9521B_all  5/25/95  08:10:11"
#---------------------------------------------------------------------
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#---------------------------------------------------------------------

#**************************************************************
# NAME    : symlink fix 
# PURPOSE : to correct the problem of symlinks appearing
#           as regular files in ODM inventory SWVPD database.
# OUTPUTS : 
#      - ODM inventory database entries are modified.
# NOTES   :
# RETURNS : nothing
#***************************************************************

#
# Be explicit in case user has alias
#
RM="/usr/bin/rm"
ODMCHANGE="/usr/bin/odmchange"
ODMGET="/usr/bin/odmget"
#-------------------------------------------------------------
#  Check to see that all of the files in the list below
#  have been fixed
#-------------------------------------------------------------
needsFix=$(${ODMGET} -q "file_type=0 and loc0=/usr/lib/makewhatis" inventory 2>/dev/null)

if [[ -z "${needsFix}" ]]
then
    exit 0
fi

#-----------------------------------------------------------
# Have work to do for this fileset.
# So, generate a file containing the fields that need
# need to change and change the DB.
#-----------------------------------------------------------
wkFile=/tmp/work.$$
while read link realFile
do
    ${RM} -f ${wkFile}

    print "inventory:\n\tfile_type = 6\n\tloc2 = ${realFile}\n\n" >${wkFile}
    ${ODMCHANGE} -q "loc0=${link}" -o inventory ${wkFile} >/dev/null 2>&1
done <<EOD
/usr/lib/mkwhatis /usr/lbin/mkwhatis
/usr/lib/makewhatis /usr/lbin/mkwhatis
EOD

${RM} -f ${wkFile}
exit 0

