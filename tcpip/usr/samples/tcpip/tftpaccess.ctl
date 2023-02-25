# @(#)05	1.2  src/tcpip/usr/samples/tcpip/tftpaccess.ctl, tcpip, tcpip411, GOLD410 10/17/91 09:25:40
#
# COMPONENT_NAME: tcpip
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
####################################################################
#
#	These are examples on allowing/denying tftp to access 
#	specific directories.  If the /etc/tftpaccess.ctl file exists
#	with permissions -rw-r--r-- (or 644) and it does not contain
#	any uncommented allow lines, then tftp access is denied for
#	the entire system.
#
####################################################################
####################################################################
# 	The following example, when uncommented, only
#	allows access to the Xstations boot files.
####################################################################
#allow:/usr/lpp/x_st_mgr/bin/bootfile
#allow:/usr/lpp/x_st_mgr/bin/bootfile1
#allow:/usr/lpp/x_st_mgr/bin/bootfile2
#
####################################################################
# 	The following example, when uncommented, only
#	allows access to the Diskless Client boot files.
####################################################################
#allow:/tftpboot
#
####################################################################
# 	The following example, when uncommented, allows access to
#	the /tmp and /usr/tmp (which is actually /var/tmp)
#	directories only.
####################################################################
#allow:/tmp
#allow:/usr/tmp
#
####################################################################
# 	The following example, when uncommented, allows access to
#	the entire system with the exemption of the /dev,
#	/etc, /sbin, /usr/bin, /usr/lib, and /usr/sbin directories.
####################################################################
#allow:/
#deny:/dev
#deny:/etc
#deny:/sbin
#deny:/usr/bin
#deny:/usr/lib
#deny:/usr/sbin
