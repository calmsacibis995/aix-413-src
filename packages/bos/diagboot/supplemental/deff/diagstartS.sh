#  DIAG S
# @(#)63	1.1  src/packages/bos/diagboot/supplemental/deff/diagstartS.sh, mpada, pkg411, 9439B411a 9/28/94 17:15:34
#   Do not erase top line. Chkdskt searches for the string DIAG S
#
# COMPONENT_NAME: MPADA - DIAGNOSTIC SUPPLEMENTAL DISKETTE
#
# FUNCTIONS: Diagnostic Diskette Supplemental Script File 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
configure=0

# See if there is a need to add stanzas to database and run cfgmgr.
# This is done by searching /etc/addfile for your stanza file name.
cd /etc/stanzas
set `echo *`
ADD=`echo $1`

for i in `/bin/cat /etc/addfile`
do
     if [ $i = $ADD ] 
     then
         configure=1
         break
     fi

done

if [ $configure = 0 ]
then
	for i in *.add
 	do
   		odmadd $i     >>$F1 2>&1
     		echo $i >> /etc/addfile    >>$F1 2>&1 
	       	rm $i      >>$F1 2>&1
	done
	/etc/cfgmgr -t -d     >>$F1 2>&1
else
        for i in *.add
	do
	     rm $i      >>$F1 2>&1
	done
fi
exit 0
