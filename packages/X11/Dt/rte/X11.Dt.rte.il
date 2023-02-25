# @(#)27        1.63.1.18  src/packages/X11/Dt/rte/X11.Dt.rte.il, pkgxcde, pkg41J, 9523B_all  6/7/95  15:46:40
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# X11.Dt.rte
#
# /(root) directories
#
d 0 2 755       /etc/dt
d 0 2 755       /etc/dt/appconfig
d 0 2 755       /etc/dt/appconfig/appmanager
d 0 2 755       /etc/dt/appconfig/appmanager/C
d 0 2 755       /etc/dt/appconfig/help
d 0 2 755       /etc/dt/appconfig/help/C
d 0 2 755       /etc/dt/appconfig/icons
d 0 2 755       /etc/dt/appconfig/icons/C
d 0 2 755       /etc/dt/appconfig/types
d 0 2 755       /etc/dt/appconfig/types/C
d 0 2 755       /etc/dt/config
d 0 2 755       /etc/dt/config/Xsession.d
d 2 2 777       /var/dt
d 2 2 777       /var/dt/appconfig
d 2 2 777       /var/dt/appconfig/appmanager
d 2 2 777       /var/dt/tmp
#
# /usr directories
#
D 2 2 755       /usr/dt
D 2 2 755       /usr/dt/appconfig
D 2 2 755       /usr/dt/appconfig/appmanager
D 2 2 755       /usr/dt/appconfig/appmanager/C
D 2 2 755       /usr/dt/appconfig/appmanager/C/System_Admin
D 2 2 755       /usr/dt/appconfig/appmanager/C/Information
D 2 2 755       /usr/dt/appconfig/appmanager/C/Desktop_Apps
D 2 2 755       /usr/dt/appconfig/appmanager/C/Desktop_Tools
D 2 2 755       /usr/dt/appconfig/types
D 2 2 755       /usr/dt/appconfig/types/C
D 2 2 755       /usr/dt/app-defaults
D 2 2 755       /usr/dt/app-defaults/C
D 2 2 755       /usr/dt/bin
D 2 2 755       /usr/dt/config
D 2 2 755       /usr/dt/config/C
D 2 2 755       /usr/dt/config/Xsession.d
D 2 2 755       /usr/dt/contrib
D 2 2 755       /usr/dt/contrib/dtscript
D 2 2 755       /usr/dt/contrib/dtscript/docs
D 2 2 755       /usr/dt/contrib/dtscript/FileBrowser
D 2 2 755       /usr/dt/contrib/dtscript/TimeSheet
D 2 2 755       /usr/dt/examples
D 2 2 755       /usr/dt/examples/types
D 2 2 755       /usr/dt/examples/types/C
D 2 2 755       /usr/dt/lib
D 2 2 755       /usr/dt/lib/bindings
D 2 2 755       /usr/dt/lib/dtksh
D 2 2 755       /usr/dt/lib/nls
D 2 2 755       /usr/dt/lib/nls/msg
D 2 2 755       /usr/dt/lib/nls/msg/C
D 2 2 755       /usr/dt/scripts
D 2 2 755       /usr/dt/install
D 2 2 755       /usr/dt/install/oldrules
D 2 2 755       /usr/dt/man
D 2 2 755       /usr/dt/man/man1
D 2 2 755       /usr/dt/migrate
D 2 2 755       /usr/dt/migrate/bin
D 2 2 755       /usr/dt/examples/dtksh
D 2 2 755       /usr/dt/share
S 2 2 777       /usr/dt/share/examples /usr/dt/examples

D 2 2 755       /usr/sbin
#
# files to configure the desktop
#
F 2 2 555       /usr/dt/install/configMin
F 2 2 555       /usr/dt/install/configRun
#
# files missing from the databases
#
# WAITING FOR AMRO'S WORD ...
#F 2 2 555      /usr/dt/bin/adjust
#F 2 2 555      /usr/dt/bin/adjust16
v 0 0 774       /etc/rc.dt
F 2 2 644       /usr/dt/README
#
# AIX 410 examples
# these files normally ship in /usr/dt/bin
#
F 0 2 4555       /usr/dt/bin/dtterm
F 2 2 555        /usr/dt/bin/dtcm
F 2 2 555        /usr/dt/bin/dtcm_editor
F 2 2 555        /usr/dt/bin/dtcm_delete
F 2 2 555        /usr/dt/bin/dtcm_insert
F 2 2 555        /usr/dt/bin/dtcm_lookup
F 2 6 2555       /usr/dt/bin/dtmail
F 2 2 555       /usr/dt/bin/dtexternalbody
# dtnosup is being shipped in the X11.Dt.adt fileset now.
#F 2 2 555        /usr/dt/bin/dtnosup
V 2 2 444        /usr/dt/appconfig/types/C/ibm.dt
#
# Files for old xdt3 removal
#
F 2 2 755        /usr/dt/install/oldrules/dtrmrules.ui
F 2 2 755        /usr/dt/install/oldrules/dtrmrules.driver
F 2 2 744        /usr/dt/install/oldrules/dtrmrules.data
F 2 2 744        /usr/dt/install/oldrules/dtrmrules.ico
#
# files added for dtscript
#
F 2 2 555       /usr/dt/bin/dtscript
V 2 2 444       /usr/dt/app-defaults/C/Dtscript
S 2 2 777       /usr/lpp/X11/lib/X11/app-defaults/Dtscript /usr/dt/app-defaults/C/Dtscript
F 2 2 444       /usr/dt/appconfig/types/C/dtscript.dt
D 0 2 755       /usr/dt/dtscript
D 0 2 755       /usr/dt/dtscript/bin
F 2 2 555       /usr/dt/dtscript/bin/uxcgen
D 0 2 755       /usr/dt/dtscript/palettes
F 2 2 444       /usr/dt/dtscript/palettes/DtScript.pal
#
F 2 2 444       /usr/dt/lib/nls/msg/C/dtscript.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/uxutils.cat
#
F 2 2 444       /usr/dt/contrib/dtscript/FileBrowser/README
F 2 2 444       /usr/dt/contrib/dtscript/FileBrowser/FileBrowser.i
F 2 2 444       /usr/dt/contrib/dtscript/TimeSheet/README
F 2 2 444       /usr/dt/contrib/dtscript/TimeSheet/InitialCode
F 2 2 444       /usr/dt/contrib/dtscript/TimeSheet/TimeSheet.i
F 2 2 444       /usr/dt/contrib/dtscript/docs/Dtksh.asc
#
# files from CDE-MIN.udb
#
F 2 2 444        /usr/dt/copyright
F 2 2 555        /usr/dt/bin/dtspcd
S 2 2 777        /usr/dt/bin/dtappintegrate /usr/sbin/dtappintegrate
F 2 2 555        /usr/dt/bin/dtksh
F 0 2 555        /usr/dt/bin/suid_exec
V 2 2 444        /usr/dt/app-defaults/C/Dtksh
V 2 2 444        /usr/dt/config/dtspcdenv
F 2 2 444        /usr/dt/lib/dtksh/DtFuncs.dtsh
#
# files from CDE-RUN.udb
#
V 2 2 555        /usr/dt/config/Xfailsafe
V 2 2 444        /usr/dt/config/Xconfig
V 2 2 544        /usr/dt/config/Xreset
V 2 2 544        /usr/dt/config/Xsetup
V 2 2 444        /usr/dt/config/Xservers
V 2 2 444        /usr/dt/config/Xaccess
V 2 2 544        /usr/dt/config/Xstartup
V 2 2 444        /usr/dt/config/C/Xresources
V 2 2 444        /usr/dt/config/C/sys.dtwmrc
V 2 2 555        /usr/dt/config/sys.dtprofile
V 2 2 444        /usr/dt/config/C/sys.resources
V 2 2 444        /usr/dt/config/C/sys.session
V 2 2 444        /usr/dt/config/C/sys.font
F 2 2 555        /usr/dt/bin/dtconfig
V 2 2 444        /usr/dt/app-defaults/C/Dthello
V 2 2 444        /usr/dt/app-defaults/C/Dtsession
V 2 2 444        /usr/dt/app-defaults/C/Dtwm
V 2 2 444        /usr/dt/app-defaults/C/Dtfile
V 2 2 444        /usr/dt/app-defaults/C/Dtstyle
V 2 2 444        /usr/dt/app-defaults/C/Dtpad
V 2 2 444        /usr/dt/app-defaults/C/Dtscreen
V 2 2 444        /usr/dt/app-defaults/C/Dticon
V 2 2 444        /usr/dt/app-defaults/C/Dtcalc
V 2 2 444        /usr/dt/app-defaults/C/Dtterm
V 2 2 444        /usr/dt/app-defaults/C/Dt
V 2 2 444        /usr/dt/app-defaults/C/Dtcreate
V 2 2 444        /usr/dt/app-defaults/C/Dtprintinfo
V 2 2 555        /usr/dt/bin/Xsession
V 2 2 555        /usr/dt/config/Xsession.d/0010.dtpaths
V 2 2 555        /usr/dt/config/Xsession.d/0020.dtims
V 2 2 555        /usr/dt/config/Xsession.d/0030.dttmpdir
V 2 2 555        /usr/dt/config/Xsession.d/0040.xmbind
V 2 2 555        /usr/dt/config/Xsession.d/0050.dtxmodmap
V 2 2 444        /usr/dt/examples/sys.font.iso
V 2 2 444        /usr/dt/config/C/dtfile.config
F 2 2 555        /usr/dt/bin/dtexec
F 2 2 555        /usr/dt/bin/dtcreate
F 2 2 555        /usr/dt/examples/dtksh/CallDataTest4
F 2 2 555        /usr/dt/examples/dtksh/CallbackTest2
F 2 2 555        /usr/dt/examples/dtksh/DtCursorTest2
F 2 2 555        /usr/dt/examples/dtksh/DtWsTest1
F 2 2 555        /usr/dt/examples/dtksh/EventHandlerTest
F 2 2 555        /usr/dt/examples/dtksh/ListBounds1
F 2 2 555        /usr/dt/examples/dtksh/ListItemPos1
F 2 2 555        /usr/dt/examples/dtksh/ListPosSel1
F 2 2 555        /usr/dt/examples/dtksh/PopupTest
F 2 2 555        /usr/dt/examples/dtksh/SelBoxResTest
F 2 2 555        /usr/dt/examples/dtksh/SessionTest
F 2 2 555        /usr/dt/examples/dtksh/TextCutBuf1
F 2 2 555        /usr/dt/examples/dtksh/TextDisp1
F 2 2 555        /usr/dt/examples/dtksh/TextFXYPos1
F 2 2 555        /usr/dt/examples/dtksh/TransEventTest
F 2 2 555        /usr/dt/examples/dtksh/TransTest1
F 2 2 555        /usr/dt/examples/dtksh/WorkProcTest1
F 2 2 555        /usr/dt/examples/dtksh/XdrawTest
F 2 2 555        /usr/dt/examples/dtksh/crMovesText1
#not_installedF 2 2 444          /usr/dt/examples/dtksh/ksh93.memo
F 2 2 555        /usr/dt/bin/dtfile
F 0 2 4555       /usr/dt/bin/dtsession
F 2 2 555        /usr/dt/bin/dtwm
F 2 2 555        /usr/dt/bin/dtfplist
F 0 2 544        /usr/dt/bin/dtlogin
F 0 2 555        /usr/dt/bin/dtlslocale
F 2 2 544        /usr/dt/bin/dtgreet
#shipped_with_X11.appsF 2 2 444          /usr/bin/X11/aixterm
#shipped_with_X11.appsS 2 2 777  /usr/bin/X11/aixterm /usr/dt/bin/aixterm
F 2 2 555        /usr/dt/bin/dthello
F 0 3 6555       /usr/dt/bin/dtaction
F 2 2 555        /usr/dt/bin/dtpad
F 2 2 555        /usr/dt/bin/dtstyle
F 2 2 555        /usr/dt/bin/dtscreen
F 2 2 555        /usr/dt/bin/dticon
F 2 2 555        /usr/dt/bin/dtcalc
#ship_as_exampleFT 0 2 4555      /usr/dt/bin/dtterm
F 2 2 555        /usr/dt/bin/dtsearchpath
F 2 2 555        /usr/dt/bin/dtappgather
F 2 2 555        /usr/dt/bin/dttypes
F 2 2 555        /usr/dt/bin/dtmosaic
F 2 2 555        /usr/dt/bin/rpc.cmsd
F 2 2 555        /usr/dt/bin/dtprintinfo
#ship_as_exampleF 2 2 555        /usr/dt/bin/dtcm
#ship_as_exampleF 2 2 555        /usr/dt/bin/dtcm_editor
#ship_as_exampleF 2 2 555        /usr/dt/bin/dtcm_delete
#ship_as_exampleF 2 2 555        /usr/dt/bin/dtcm_insert
#ship_as_exampleF 2 2 555        /usr/dt/bin/dtcm_lookup
V 2 2 444        /usr/dt/appconfig/types/C/dtcm.dt
#ship_as_exampleF 2 6 555        /usr/dt/bin/dtmail
#not_installedF 2 2 555          /usr/dt/bin/dtmailconv
V 2 2 444        /usr/dt/appconfig/types/C/dtmail.dt
V 2 2 444        /usr/dt/appconfig/types/C/dtwm.fp
V 2 2 444        /usr/dt/appconfig/types/C/autoStart.dt
V 2 2 444        /usr/dt/appconfig/types/C/uxstd.dt
V 2 2 444        /usr/dt/appconfig/types/C/dt.dt
V 2 2 444        /usr/dt/appconfig/types/C/dtappman.dt
V 2 2 444        /usr/dt/appconfig/types/C/dtfile.dt
V 2 2 444        /usr/dt/appconfig/types/C/dtpad.dt
V 2 2 444        /usr/dt/appconfig/types/C/xclients.dt
V 2 2 444        /usr/dt/appconfig/types/C/print.dt
V 2 2 444        /usr/dt/appconfig/types/C/user-prefs.dt
V 2 2 444        /usr/dt/appconfig/types/C/dthelp.dt
V 2 2 444        /usr/dt/appconfig/types/C/dtscreen.dt
V 2 2 444        /usr/dt/examples/types/C/IconBrowse.dt
#
F 0 2 544       /usr/dt/bin/dtchooser
ST 2 2 777       /usr/dt/bin/aixterm /usr/lpp/X11/bin/aixterm
F 2 2 444       /usr/dt/app-defaults/C/Dtcm
F 2 2 444       /usr/dt/app-defaults/C/Dtmail
F 2 2 444       /usr/dt/app-defaults/C/UNIXbindings
F 2 2 444       /usr/dt/appconfig/types/C/datatypes.dt
F 2 2 444       /usr/dt/appconfig/types/C/develop.dt
F 2 2 444       /usr/dt/examples/types/C/miscActions.dt
F 2 2 444       /usr/dt/examples/types/C/miscImages.dt

F 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtcm /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtmail /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtmanpageview /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dttrash /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dthelpview /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtterm /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtcreate /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dticon /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtpad /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtcalc /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtfile /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Apps/Dtstyle /usr/dt/appconfig/appmanager/C/Desktop_Apps/DtPrintManager


F 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Df /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Diff /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DttermConsole /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DttermDtspcd /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DttermErrorlog /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DttermRlogin /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Dttypes /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DtwmrcEdit /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DuSort /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Env /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/File /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Grep /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Nm /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/ReloadActions /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/ReloadResources /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/RestorePanel /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Rm /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Spell /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Wc /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xclipboard /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/XclockDig /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xdpyinfo /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xfd /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xload /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xlsfonts /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xprop /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xrefresh /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/XtermDtspcd /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/XtermRlogin /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xwd /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xwininfo /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xwud /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Compress /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/ExecuteCmd /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Make /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Tar /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/TarList /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/TarUnpack /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Uncompress /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Vi /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Xterm /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/ReloadApps /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Aixterm /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/Dtscript /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap
H 2 2 555       /usr/dt/appconfig/appmanager/C/Desktop_Tools/DtEmptyTrash /usr/dt/appconfig/appmanager/C/Desktop_Tools/Bitmap

F 2 2 555       /usr/dt/bin/dsdm
#F 2 2 555      /usr/dt/bin/dtcm_admin
F 2 2 555       /usr/dt/bin/dtdspmsg
F 2 2 555       /usr/dt/bin/dterror.ds
F 2 2 555       /usr/dt/bin/dtfile_copy
F 2 2 555       /usr/dt/bin/dtfile_error
F 2 2 555       /usr/dt/bin/dtlp
F 2 2 555       /usr/dt/bin/dtsession_res
F 2 6 2555      /usr/dt/bin/dtmailpr
#
#
# files from CDE-MSG-CAT.udb
#
F 2 2 444        /usr/dt/lib/nls/msg/C/dtlogin.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dthello.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtsession.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtwm.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtfile.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtstyle.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtpad.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtscreen.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dticon.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtact.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dt.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtcreate.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtcopy.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/DtHelp.cat
#not_installedF 2 2 444          /usr/dt/lib/nls/msg/C/dtmail.cat
#not_installedF 2 2 444          /usr/dt/lib/nls/msg/C/libDtMail.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtksh.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtcalc.cat
F 2 2 444        /usr/dt/lib/nls/msg/C/dtterm.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtmosaic.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtbuilder.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtcm.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtlp.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtmailopts.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtprintinfo.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtmail.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dttypes.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/MotifApp.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/DtWidget.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtconfig.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/libDtMail.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtcodegen.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/DtMail.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/SUNW_TOOLTALK.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtaclafs.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dthelpprint.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/dtnosup.cat
F 2 2 444       /usr/dt/lib/nls/msg/C/fmt_tbl.cat
#
# files converted to links for appmanager
#
F 2 2 555 /usr/dt/appconfig/types/C/action
F 2 2 555 /usr/dt/appconfig/appmanager/C/Information/NCSA_Mosaic
#
# IBM AIX 4.1.x only
#
F 2 2 555 /usr/dt/appconfig/appmanager/C/System_Admin/Smit
F 2 2 555 /usr/dt/appconfig/appmanager/C/System_Admin/RootSmit
V 0 0 774       /usr/sbin/rc.bootx
F 0 2 544       /usr/sbin/bootxutil
#
# migration files
#
F 2 2 444       /usr/dt/lib/nls/msg/C/dtmigrate.cat
F 2 2 555       /usr/dt/migrate/bin/dtmigrate
F 2 2 555       /usr/dt/bin/dtversion


