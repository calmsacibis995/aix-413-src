#!/usr/bin/ksh
# @(#)87	1.2  src/packages/bos/info/rte/usr/bos.info.rte.unpost_i.sh, pkginfo, pkg411, 9437A411a  9/12/94  11:07:52
#
#   COMPONENT_NAME: pkginfo
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

LPP_INFO_DATA_DIR=/usr/lpp/info/data
SHARE_INFO_DATA_DIR=/usr/share/info/data

for file in `ls ${LPP_INFO_DATA_DIR}`
do
   if [ -L ${LPP_INFO_DATA_DIR}/${file} ]
   then
      long_listing=`ls -l ${LPP_INFO_DATA_DIR}/${file}`
      for link in ${long_listing}
      do
        :
      done
      if [[ $link = $SHARE_INFO_DATA_DIR* ]]
      then
         rm -f ${LPP_INFO_DATA_DIR}/${file}
      fi
   fi
done

rm -rf /usr/lpp/info/32
rm -rf /usr/lpp/info/lib/32
rm -rf /usr/lpp/info/data/32

exit 0
