# @(#)86        1.14  src/packages/bos/acct/bos.acct.il, pkgacct, pkg411, GOLD410 4/26/94 16:42:25
#
# COMPONENT_NAME: pkgacct
#
# FUNCTIONS: none
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
# LPP_NAME: bos.acct
#
d 0 0 755	/etc/acct acct
v 0 4 660	/etc/acct/holidays acct
D 2 2 755	/usr/lib/acct acct
S 0 4 777	/usr/lib/acct/holidays /etc/acct/holidays acct
FT 0 2 4555     /usr/bin/iostat acct
F 2 2 555	/usr/bin/sysline acct
F 2 4 2555	/usr/bin/timex acct
FT 0 2 4555     /usr/bin/vmstat acct
D 4 4 755       /usr/lib/sa acct
F 2 4 550       /usr/lib/sa/sa1 acct
F 2 4 550       /usr/lib/sa/sa2 acct
FT 0 4 4550     /usr/lib/sa/sadc acct
D 2 2 755	/usr/sbin/acct acct
F 0 4 555	/usr/sbin/acct/ac acct
S 0 4 777	/usr/lib/acct/ac /usr/sbin/acct/ac acct
F 0 4 550	/usr/sbin/acct/acctcms acct
S 0 4 777	/usr/lib/acct/acctcms /usr/sbin/acct/acctcms acct
F 0 4 555	/usr/sbin/acct/acctcom acct
S 0 4 777	/usr/bin/acctcom /usr/sbin/acct/acctcom acct
F 0 4 550	/usr/sbin/acct/acctcon1 acct
S 0 4 777	/usr/lib/acct/acctcon1 /usr/sbin/acct/acctcon1 acct
F 0 4 550	/usr/sbin/acct/acctcon2 acct
S 0 4 777	/usr/lib/acct/acctcon2 /usr/sbin/acct/acctcon2 acct
F 0 4 550	/usr/sbin/acct/acctdisk acct
S 0 4 777	/usr/lib/acct/acctdisk /usr/sbin/acct/acctdisk acct
F 0 4 550	/usr/sbin/acct/acctdusg acct
S 0 4 777	/usr/lib/acct/acctdusg /usr/sbin/acct/acctdusg acct
F 0 4 550	/usr/sbin/acct/acctmerg acct
S 0 4 777	/usr/lib/acct/acctmerg /usr/sbin/acct/acctmerg acct
FT 0 4 4550	/usr/sbin/acct/accton acct
ST 0 4 777	/usr/lib/acct/accton /usr/sbin/acct/accton acct
F 0 4 550	/usr/sbin/acct/acctprc1 acct
S 0 4 777	/usr/lib/acct/acctprc1 /usr/sbin/acct/acctprc1 acct
F 0 4 550	/usr/sbin/acct/acctprc2 acct
S 0 4 777	/usr/lib/acct/acctprc2 /usr/sbin/acct/acctprc2 acct
F 0 4 550	/usr/sbin/acct/acctwtmp acct
S 0 4 777	/usr/lib/acct/acctwtmp /usr/sbin/acct/acctwtmp acct
F 0 4 550	/usr/sbin/acct/chargefee acct
S 0 4 777	/usr/lib/acct/chargefee /usr/sbin/acct/chargefee acct
F 0 4 550	/usr/sbin/acct/ckpacct acct
S 0 4 777	/usr/lib/acct/ckpacct /usr/sbin/acct/ckpacct acct
F 0 4 550	/usr/sbin/acct/dodisk acct
S 0 4 777	/usr/lib/acct/dodisk /usr/sbin/acct/dodisk acct
F 0 4 550	/usr/sbin/acct/fwtmp acct
S 0 4 777	/usr/lib/acct/fwtmp /usr/sbin/acct/fwtmp acct
F 0 4 550	/usr/sbin/acct/lastlogin acct
S 0 4 777	/usr/lib/acct/lastlogin /usr/sbin/acct/lastlogin acct
F 0 4 550	/usr/sbin/acct/monacct acct
S 0 4 777	/usr/lib/acct/monacct /usr/sbin/acct/monacct acct
F 0 4 550	/usr/sbin/acct/nulladm acct
S 0 4 777	/usr/lib/acct/nulladm /usr/sbin/acct/nulladm acct
F 0 4 550	/usr/sbin/acct/prctmp acct
S 0 4 777	/usr/lib/acct/prctmp /usr/sbin/acct/prctmp acct
F 0 4 550	/usr/sbin/acct/prdaily acct
S 0 4 777	/usr/lib/acct/prdaily /usr/sbin/acct/prdaily acct
F 0 4 550	/usr/sbin/acct/prtacct acct
S 0 4 777	/usr/lib/acct/prtacct /usr/sbin/acct/prtacct acct
F 0 4 440	/usr/sbin/acct/ptecms.awk acct
S 0 4 777	/usr/lib/acct/ptecms.awk /usr/sbin/acct/ptecms.awk acct
F 0 4 440	/usr/sbin/acct/ptelus.awk acct
S 0 4 777	/usr/lib/acct/ptelus.awk /usr/sbin/acct/ptelus.awk acct
F 0 4 550	/usr/sbin/acct/remove acct
S 0 4 777	/usr/lib/acct/remove /usr/sbin/acct/remove acct
F 0 4 550	/usr/sbin/acct/runacct acct
S 0 4 777	/usr/lib/acct/runacct /usr/sbin/acct/runacct acct
F 0 4 550	/usr/sbin/acct/shutacct acct
S 0 4 777	/usr/lib/acct/shutacct /usr/sbin/acct/shutacct acct
F 0 4 550	/usr/sbin/acct/startup acct
S 0 4 777	/usr/lib/acct/startup /usr/sbin/acct/startup acct
F 0 4 550	/usr/sbin/acct/turnacct acct
S 0 4 777	/usr/lib/acct/turnacct /usr/sbin/acct/turnacct acct
F 0 4 550	/usr/sbin/acct/wtmpfix acct
S 0 4 777	/usr/lib/acct/wtmpfix /usr/sbin/acct/wtmpfix acct
FT 0 4 4550	/usr/sbin/diskusg acct
ST 0 4 777	/usr/sbin/acct/diskusg /usr/sbin/diskusg acct
ST 0 4 777	/usr/lib/acct/diskusg /usr/sbin/diskusg acct
F 2 2 555       /usr/sbin/sa acct
F 2 2 555       /usr/sbin/sar acct
d 4 4 775       /var/adm/acct acct
d 4 4 775       /var/adm/sa acct
