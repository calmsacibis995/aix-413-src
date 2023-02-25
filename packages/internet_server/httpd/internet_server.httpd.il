# @(#)48	1.5  src/packages/internet_server/httpd/internet_server.httpd.il, pkgweb, pkg41J, 9523B_all 6/5/95 18:04:22
#
# COMPONENT_NAME: pkgweb
#
# FUNCTIONS: packaging definition
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
v 0 0 664	/etc/httpd.conf
#v 0 0 644	/etc/lynx.conf
v 2 2 755	/etc/rc.httpd
#F 2 2 755	/usr/bin/lynx
D 0 0 755	/usr/lpp/internet
D 0 0 775	/usr/lpp/internet/server_root
D 0 0 755	/usr/lpp/internet/server_root/cgi-bin
F 0 0 755	/usr/lpp/internet/server_root/cgi-bin/cgiparse
F 0 0 755	/usr/lpp/internet/server_root/cgi-bin/cgiutils
F 0 0 755	/usr/lpp/internet/server_root/cgi-bin/htadm
F 0 0 755	/usr/lpp/internet/server_root/cgi-bin/htimage
D 0 0 755	/usr/lpp/internet/server_root/icons
F 2 2 755	/usr/lpp/internet/server_root/icons/back.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/binary.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/binary.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/binhex.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/blank.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/blank.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/compressed.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/dir.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/dirblue.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/directory.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/doc.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/doc.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/dummy.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/ftp.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/gzip.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/image.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/index.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/index2.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/movie.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/sound.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/tar.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/telnet.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/text.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/text.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/unknown.doc.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/unknown.old.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/unknown.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/unknown.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/uu.xbm
F 2 2 755	/usr/lpp/internet/server_root/icons/DtC++.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtassem.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/DtaudioFile.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtbshll.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtcmprs.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtcshll.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtdirup.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtgif.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtimage.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtjpegi.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtkshll.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtmagtp.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtmime.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtpcl.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtpixmp.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtps.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtshar.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtsprds.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dttif.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtuil.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtuuncd.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/Dtxwd.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/html.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/video_avi.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/video_file.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/video_jpeg.m.pm.gif
F 2 2 755	/usr/lpp/internet/server_root/icons/video_mpeg.m.pm.gif
D 0 0 775	/usr/lpp/internet/server_root/logs
D 0 0 755	/usr/lpp/internet/server_root/pub
#D 0 0 755	/usr/lpp/internet/server_root/pub/lynx
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/Lynx_users_guide.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/Lynx_users_guide237.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/about_lynx.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/bookmark_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/dired_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/history_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/keystroke_help.237.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/keystroke_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/lynx2_3.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/lynx_help_main.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/movement_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/option_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/other_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/print_help.html
#F 2 2 644	/usr/lpp/internet/server_root/pub/lynx/scrolling_help.html
D 0 0 755	/usr/lpp/internet_server
F 0 0 644	/usr/lpp/internet_server/README
D 0 0 755	/usr/samples/internet_server
D 0 0 755	/usr/samples/internet_server/config
F 0 0 644	/usr/samples/internet_server/config/all.conf
F 0 0 644	/usr/samples/internet_server/config/proxy.conf
F 0 0 644	/usr/samples/internet_server/config/caching.conf
F 0 0 644	/usr/samples/internet_server/config/prot.conf
F 0 0 4554	/usr/sbin/httpd
