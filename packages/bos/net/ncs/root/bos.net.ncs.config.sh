#!/bin/ksh
# @(#)24	1.3  src/packages/bos/net/ncs/root/bos.net.ncs.config.sh, pkgncs, pkg411, GOLD410 4/14/94 09:42:25
# 
#
# COMPONENT_NAME: pkgncs
#
# FUNCTIONS:
#
# ORIGINS: 27, 84
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

Mkssys(){
	mkssys -s $1 -p$2 -G $3 -u 0 -S -n15 -f9
	if [ $? -ne 0 ]
	then
		inuumsg 115 mkssys $1 
                rc=1
        fi
}

Rmssys(){
	rmssys -s $1 2>&1 > /dev/null
}


rc=0

Rmssys llbd
Mkssys llbd /usr/lib/ncs/bin/llbd ncs

Rmssys glbd
Mkssys glbd /usr/lib/ncs/bin/glbd ncs

Rmssys nrglbd
Mkssys nrglbd /usr/lib/ncs/bin/nrglbd ncs

# copy the NCS 1.1 glb_sites file to the NCS 1.5.1 glb_site.txt file,
# but only if glb_sites exists and glb_site.txt doesn't.
if [[ -a /etc/ncs/glb_sites && ! -a /etc/ncs/glb_site.txt ]]; then
  cp /etc/ncs/glb_sites /etc/ncs/glb_site.txt
  if [[ $? -ne 0 ]]; then
    rc=1
  else
    chmod 644 /etc/ncs/glb_site.txt
    if [[ $? -ne 0 ]]; then
      rc=1
    fi
  fi
fi

exit $rc
