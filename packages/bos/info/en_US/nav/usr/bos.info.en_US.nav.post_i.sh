#!/usr/bin/ksh
#  %Z%%M%        %I%  %W% %G% %U%
#
#   COMPONENT_NAME: pkginfodb
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

export ODMDIR=/usr/lib/objrepos
INFODIR=/usr/lpp/info/lib/en_US/aix41

LIST="perfagent.info.en_US.usr_gds icraft.info.en_US.user_gd_ref bos.info.en_US.adapters_gd_ref bos.info.en_US.assemb_ref bos.info.en_US.aix_tech_ref bos.info.en_US.files_ref bos.info.en_US.info_help bos.info.en_US.keybd_tech_ref bos.info.en_US.manage_gds bos.info.en_US.probsolv_gds bos.info.en_US.prog_gds bos.info.en_US.update bos.info.en_US.user_gds bos.info.en_US.cmds_ref X11.info.en_US.prog_gds X11.info.en_US.tech_ref X11.info.en_US.x_st_mgr.manage_gd Wabi.info.en_US.usr_gd UMS.info.en_US.user_gd OpenGL.info.en_US.GL32 OpenGL.info.en_US.OpenGL_X"


### Check if there's a /tmp/infolist and nuke it if so

if [ -f /tmp/infolist ]
then
        rm /tmp/infolist
fi



#####################################################################
### Section to remove entries from ODM database.#####################
### Only Austin IDD's packages contain the "4.1.0.0"
### string, so only those are selected and subsequently
### odmdeleted.######################################################
#####################################################################




for fileset in `echo $LIST`
do
	lslpp -qlc $fileset >/dev/null 2>&1
	if [ $? -eq 0 ]
	then
		lslpp -qlc $fileset | grep -E "4.1.0.0|4.1.1.0"  >/dev/null 2>&1
		if [ $? -eq 0 ]
		then
                lslpp -f $fileset >>/tmp/infolist
                odmget -q "name = $fileset and state != 1" lpp | grep "lpp_id =" | awk '{print $3}' | while read lpp_id
   		do
      		  odmdelete -o history -q "lpp_id=$lpp_id" >/dev/null 2>&1
      		  odmdelete -o lpp -q "lpp_id=$lpp_id" >/dev/null 2>&1
      		  odmdelete -o inventory -q "lpp_id=$lpp_id" >/dev/null 2>&1
      		  odmdelete -o product -q "lpp_name=$fileset" >/dev/null 2>&1
   		done
		fi
	fi
done



grep "/usr/lpp/info/lib/en_US/aix41" /tmp/infolist >/tmp/infolist2
mv /tmp/infolist2 /tmp/infolist >/dev/null 2>&1


#####################################################################
### Section to remove databases from hard drive. #####################
### Only Austin IDD Info databases are installed in
### /usr/lpp/info/lib/en_US/aix41, so removing from
### there will not remove other libraries that are installed
### in different places (pex, PHIGs, essl, xlC, etc.).################
#####################################################################


for file in `cat /tmp/infolist`
# These will be the files contained in the installed filesets from
# running lslpp -f
do
  rm -fr $file >/dev/null 2>&1
done

rm /tmp/infolist >/dev/null 2>&1

exit 0


