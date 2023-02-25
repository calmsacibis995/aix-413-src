# @(#)02	1.27  src/packages/X11/vsm/rte/X11.vsm.rte.il, pkgxvsm, pkg411, 9440A411h 10/6/94 17:29:57
#
# COMPONENT_NAME: libvsm
#
# FUNCTIONS:  inslist
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
# -------------------------------------------------------------------------
#
# LPP_NAME: X11.
#


#Install Assistant Files
F 0 0 500	/usr/lib/assist/assist.xinitrc	
F 0 0 644	/usr/lib/assist/assist.resources	

# VSM Library 
V 2 2 644	/usr/lib/libvsm.a

# Defaults file
F 0 0 444	/usr/lpp/X11/lib/X11/app-defaults/Vsm


# Executables
FT 0 0 4555	/usr/lpp/X11/bin/xdevicem
FT 0 0 4555	/usr/lpp/X11/bin/xlvm
FT 0 0 4555	/usr/lpp/X11/bin/xprintm
FT 0 0 4555	/usr/lpp/X11/bin/xuserm
FT 0 0 4555	/usr/lpp/X11/bin/xinstallm
FT 0 0 4555	/usr/lpp/X11/bin/xmaintm
FT 0 0 4555	/usr/lpp/X11/bin/xdat
F 0 0 500	/usr/lib/instl/getlicenseprod

# Dt action and type files, the action files must be executable
D 2 2 755	/usr/lpp/X11.vsm
D 2 2 755	/usr/lpp/X11.vsm/dt

F 2 2 444	/usr/lpp/X11.vsm/dt/vsm.dt
F 2 2 444	/usr/lpp/X11.vsm/dt/vsm.hf
F 2 2 444	/usr/lpp/X11.vsm/dt/iassist.hf
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmDevicesManager
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmEasyInstall
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmInstallManager
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmInstallAssist
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmMaintainSoftware
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmPrintManager
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmSetDateTime
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmShowSchedule
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmStorageManager
F 2 2 555	/usr/lpp/X11.vsm/dt/VsmUsersGroups

# Default Type and Action Files:
D 2 2 755	/usr/dt
D 2 2 755	/usr/dt/appconfig
D 2 2 755	/usr/dt/appconfig/types
D 2 2 755	/usr/dt/appconfig/types/C
D 2 2 755	/usr/dt/appconfig/help
D 2 2 755	/usr/dt/appconfig/help/C
D 2 2 755	/usr/dt/appconfig/appmanager
D 2 2 755	/usr/dt/appconfig/appmanager/C
D 2 2 755	/usr/dt/appconfig/appmanager/C/System_Admin

# Symbolic links so that the CDE desktop can find the actions and types
S 2 2 777	/usr/dt/appconfig/types/C/vsm.dt /usr/lpp/X11.vsm/dt/vsm.dt
S 2 2 777	/usr/dt/appconfig/help/C/vsm.hf /usr/lpp/X11.vsm/dt/vsm.hf
S 2 2 777	/usr/dt/appconfig/help/C/iassist.hf /usr/lpp/X11.vsm/dt/iassist.hf
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmDevicesManager /usr/lpp/X11.vsm/dt/VsmDevicesManager
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmEasyInstall /usr/lpp/X11.vsm/dt/VsmEasyInstall
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmInstallManager /usr/lpp/X11.vsm/dt/VsmInstallManager
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmInstallAssist /usr/lpp/X11.vsm/dt/VsmInstallAssist
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmMaintainSoftware /usr/lpp/X11.vsm/dt/VsmMaintainSoftware
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmPrintManager /usr/lpp/X11.vsm/dt/VsmPrintManager
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmSetDateTime /usr/lpp/X11.vsm/dt/VsmSetDateTime
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmShowSchedule /usr/lpp/X11.vsm/dt/VsmShowSchedule
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmStorageManager /usr/lpp/X11.vsm/dt/VsmStorageManager
S 2 2 777	/usr/dt/appconfig/appmanager/C/System_Admin/VsmUsersGroups /usr/lpp/X11.vsm/dt/VsmUsersGroups

# Default Help Files:
# These are now in the X11.vsm.helps package

# Icons
# These are now in the X11.vsm.icons package
