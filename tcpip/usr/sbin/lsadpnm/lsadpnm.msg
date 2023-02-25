$ @(#)50	1.3  src/tcpip/usr/sbin/lsadpnm/lsadpnm.msg, tcpip, tcpip411, GOLD410 5/1/90 14:10:51
$
$ Messages for lsadpmn.c 
$
$ COMPONENT_NAME:  CMDNET   Network Configuration  
$
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when 
$ combined with the aggregated modules for this product) 
$                  SOURCE MATERIALS 
$ (C) COPYRIGHT International Business Machines Corp. 1989. 
$ All Rights Reserved 
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
$
$len 
$quote "         define quote character.
$
$ Do not translate defif.
$
$ Message numbers have been assigned a range of 014-037.
$
$set MS_LSADPNM        Define initial set#
GETPDAT "0822-050 lsadpnm: Cannot get records from PdAt.\n"
NOIFKEY "0822-051 lsadpnm: Cannot get \"if_keyword\" attribute from PdAt.\n"
GETPDDV "0822-052 lsadpnm: Cannot get records from PdDv.\n"
NOPDIF  "0822-053 lsadpnm: Cannot find %s/%s/%s in PdDv.\n"
GETCUDV "0822-054 lsadpnm: Cannot get records from CuDv.\n"
NOMEM   "0822-055 lsadpnm: Cannot allocate memory\n"
NOADPAV "0822-056 lsadpnm: No available adapter.\n"
USAGE   "\nusage:\tlsadpnm  -c class -s subclass -t type\n"
BADPATH "0822-057 lsadpnm: Cannot set the default path to %s.\n"
INITFAIL "0822-058 lsadpnm: The database initialization failed.\n"
NOCUDV	 "0822-059 lsadpnm: Cannot find adapter in CuDv with prefix %s.\n"
GETADAPT "0822-060 lsadpnm: Cannot get records from PdCn.\n"
GETPDADAP "0822-061 lsadpnm: Cannot get predefined adapter.\n"
PDADAPNUM "0822-062 lsadpnm: Expected one predefined adapter, retreived %d.\n"
