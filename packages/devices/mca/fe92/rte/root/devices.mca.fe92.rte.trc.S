# @(#)33        1.2  src/packages/devices/mca/fe92/rte/root/devices.mca.fe92.rte.trc.S, pkgdevcommo, pkg411, GOLD410 1/26/94 08:58:48
#
# COMPONENT_NAME: pkgdevcommo
#
# FUNCTIONS:  devices.mca.fe92.rte.trc
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt

+ 350 1.1 "CATDD" \
   {{ $loc = X1 }} R1 $DD1 \
        $loc, \
        0x05 { \n "chan:" X4 "base_addr:" X4 "length:" D4 }, \
        0x07 { \n "chan:" X4 "base_addr:" X4 "length:" D4 }, \
        0x15 { "interrupt entry type:" X4 }, \
        0x16 { "interrupt exit type:" X4 }, \
        0x70 { "entry_halt:" $DD2 \n "chan:" X4 "netid:" D4 }, \
        0x71 { "exit_halt:" $DD2 }, \
        0x72 { "entry_get_stat:" $DD2 "chan:" X4 }, \
        0x73 { "exit_get_stat:" $DD2 }, \
        0x8B { "entry_start:" $DD2 \
                \n "chan:" X4 "groupnum" D4 "netid:" D4 \
                "subset_size:" D4 }, \
        0x8C { "exit_start:" $DD2 }, \
        0x8D { "entry_query:" $DD2 "chan:" X4 }, \
        0x8E { "exit_query:" $DD2 }, \
        0xC0 { "PSCA acknowledge:" $DD2 \
                \n "command:" X1 "cmdmod:" X1 "correl:" D1 "subchan:" D1 \
                \n "chanstat:" X1 "ccw:" X1 "origcmd:" X1 "retcode:" X1 \
                \n "data[0]:" X1 "data[1]:" D1 "length:" X2 "buffer:" X4 \
                }, \
        0xC1 { "send_elem:" $DD2 \
                \n "command:" X1 "cmdmod:" X1 "correl:" D1 "subchan:" D1 \
                \n "chanstat:" X1 "ccw:" X1 "origcmd:" X1 "retcode:" X1 \
                \n "data[0]:" X1 "data[1]:" D1 "length:" X2 "buffer:" X4 \
                },\
        0xC2 { "PSCA command:" $DD2 \
                \n "command:" X1 "cmdmod:" X1 "correl:" D1 "subchan:" D1 \
                \n "chanstat:" X1 "ccw:" X1 "origcmd:" X1 "retcode:" X1 \
                \n "data[0]:" X1 "data[1]:" D1 "length:" X2 "buffer:" X4 \
             },\
    	   0xC3 { "PSCA link:" $DD2 \
	     	\n "subchannel id:"  X1 "link actual id:" X1 "application link id:" X1 "link state:" X1 \
	         }, \
 	   0xC4 { "PSCA subchan:" $DD2 \
		     \n "subchannel:" X1 "state:" X1 "group:" X1 "subset:" X1 \
		     \n "specmode:" X1 "shrtbusy:" X1 "startde:" X1 "misc:" X1 \
		     \n "num links:" X4   \
	     }, \
	   0xC5 { "PSCA Gen:"  $DD2 \
			\n A4  X4  X4  X4 \
		   }
