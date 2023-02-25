# @(#)71	1.12  src/packages/bos/net/uucp/bos.net.uucp.il, pkguucp, pkg411, 9436D411a 9/8/94 15:07:24
#
# COMPONENT_NAME: pkguucp
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.net.uucp
#
v 5 5 400	/etc/uucp/Dialcodes uucp
S 5 5 777	/usr/lib/uucp/Dialcodes /etc/uucp/Dialcodes uucp
v 5 5 444	/etc/uucp/Dialers.samples uucp
S 5 5 777	/usr/lib/uucp/Dialers.samples /etc/uucp/Dialers.samples uucp
v 5 5 444	/etc/uucp/Maxuuscheds uucp
S 5 5 777	/usr/lib/uucp/Maxuuscheds /etc/uucp/Maxuuscheds uucp
v 5 5 444	/etc/uucp/Maxuuxqts uucp
S 5 5 777	/usr/lib/uucp/Maxuuxqts /etc/uucp/Maxuuxqts uucp
v 5 5 400	/etc/uucp/Permissions uucp
S 5 5 777	/usr/lib/uucp/Permissions /etc/uucp/Permissions uucp
v 5 5 644	/etc/uucp/Poll uucp
S 5 5 777	/usr/lib/uucp/Poll /etc/uucp/Poll uucp
v 5 5 400	/etc/uucp/Systems uucp
S 5 5 777	/usr/lib/uucp/Systems /etc/uucp/Systems uucp
v 5 5 644	/etc/uucp/Sysfiles uucp
S 5 5 777	/usr/lib/uucp/Sysfiles /etc/uucp/Sysfiles uucp
F 5 5 555 	/usr/bin/ct uucp
F 5 5 4555	/usr/bin/cu uucp
F 5 5 555	/usr/bin/tip uucp
F 5 5 4555	/usr/bin/uucp uucp
F 5 5 555	/usr/bin/uudecode uucp
F 5 5 555	/usr/bin/uuencode uucp
F 5 5 555	/usr/bin/uulog uucp
F 5 5 4555	/usr/bin/uuname uucp
F 5 5 555	/usr/bin/uupick uucp
F 5 5 555	/usr/bin/uupoll uucp
F 5 5 4555	/usr/bin/uuq uucp
F 5 5 555	/usr/bin/uusend uucp
F 5 5 4555	/usr/bin/uustat uucp
H 5 5 777	/usr/bin/uusnap /usr/bin/uustat uucp
F 5 5 555	/usr/bin/uuto uucp
F 5 5 4555	/usr/bin/uux uucp
V 0 0 644	/usr/lib/phones-file uucp
V 0 0 644	/usr/lib/remote-file uucp
D 0 3 755	/usr/sbin/uucp uucp
F 5 5 555	/usr/sbin/uucp/Nutry uucp
S 5 5 777	/usr/lib/uucp/Nutry /usr/sbin/uucp/Nutry uucp
F 5 5 555	/usr/sbin/uucp/Uutry uucp
S 5 5 777	/usr/lib/uucp/Uutry /usr/sbin/uucp/Uutry uucp
FT 0 0 4550	/usr/sbin/uucpd uucp
st 0 0 777	/etc/uucpd /usr/sbin/uucpd uucp
V 5 5 555	/usr/sbin/uucp/remote.unknown uucp
S 5 5 777	/usr/lib/uucp/remote.unknown /usr/sbin/uucp/remote.unknown uucp
F 5 5 550	/usr/sbin/uucp/uucheck uucp
S 5 5 777	/usr/lib/uucp/uucheck /usr/sbin/uucp/uucheck uucp
F 5 5 4555	/usr/sbin/uucp/uucico uucp
S 5 5 777	/usr/lib/uucp/uucico /usr/sbin/uucp/uucico uucp
F 5 5 550	/usr/sbin/uucp/uuclean uucp
S 5 5 777	/usr/lib/uucp/uuclean /usr/sbin/uucp/uuclean uucp
F 5 5 550	/usr/sbin/uucp/uucleanup uucp
S 5 5 777	/usr/lib/uucp/uucleanup /usr/sbin/uucp/uucleanup uucp
F 5 5 500	/usr/sbin/uucp/uucpadm uucp
S 5 5 777	/usr/lib/uucp/uucpadm /usr/sbin/uucp/uucpadm uucp
F 5 5 644	/usr/sbin/uucp/uucpadm.hf uucp
S 5 5 777	/usr/lib/uucp/uucpadm.hf /usr/sbin/uucp/uucpadm.hf uucp
V 5 5 555	/usr/sbin/uucp/uudemon.admin uucp
S 5 5 777	/usr/lib/uucp/uudemon.admin /usr/sbin/uucp/uudemon.admin uucp
V 5 5 555	/usr/sbin/uucp/uudemon.cleanu uucp
S 5 5 777	/usr/lib/uucp/uudemon.cleanup /usr/sbin/uucp/uudemon.cleanu uucp
S 5 5 777	/usr/lib/uucp/uudemon.cleanu /usr/sbin/uucp/uudemon.cleanu uucp
V 5 5 555	/usr/sbin/uucp/uudemon.hour uucp
S 5 5 777	/usr/lib/uucp/uudemon.hour /usr/sbin/uucp/uudemon.hour uucp
V 5 5 555	/usr/sbin/uucp/uudemon.poll uucp
S 5 5 777	/usr/lib/uucp/uudemon.poll /usr/sbin/uucp/uudemon.poll uucp
F 5 5 555	/usr/sbin/uucp/uukick uucp
S 5 5 777	/usr/lib/uucp/uukick /usr/sbin/uucp/uukick uucp
F 5 5 4555	/usr/sbin/uucp/uusched uucp
S 5 5 777	/usr/lib/uucp/uusched /usr/sbin/uucp/uusched uucp
F 5 5 555	/usr/sbin/uucp/uutry uucp
S 5 5 777	/usr/lib/uucp/uutry /usr/sbin/uucp/uutry uucp
F 5 5 4555	/usr/sbin/uucp/uuxqt uucp
S 5 5 777	/usr/lib/uucp/uuxqt /usr/sbin/uucp/uuxqt uucp
f 0 8 644	/var/spool/cron/crontabs/uucp uucp
d 5 5 755	/var/spool/uucp uucp
d 5 5 755	/var/spool/uucp/.Admin uucp
d 5 5 755	/var/spool/uucp/.Corrupt uucp
d 5 5 755	/var/spool/uucp/.Log uucp
d 5 5 755	/var/spool/uucp/.Log/uucico uucp
d 5 5 755	/var/spool/uucp/.Log/uucp uucp
d 5 5 755	/var/spool/uucp/.Log/uux uucp
d 5 5 755	/var/spool/uucp/.Log/uuxqt uucp
d 5 5 755	/var/spool/uucp/.Old uucp
d 5 5 755	/var/spool/uucp/.Sequence uucp
d 5 5 755	/var/spool/uucp/.Status uucp
d 5 5 755	/var/spool/uucp/.Workspace uucp
d 5 5 755	/var/spool/uucp/.Xqtdir uucp
d 5 5 777	/var/spool/uucppublic uucp
