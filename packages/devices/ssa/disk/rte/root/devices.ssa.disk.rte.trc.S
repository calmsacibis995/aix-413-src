#@(#)28 1.1  src/packages/devices/ssa/disk/rte/root/devices.ssa.disk.rte.trc.S, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:32:57
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    325 : ssadisk.obj.trc.S				#
#		    410 : devices.ssa.disk.rte.trc.S			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Stanza file for the trcupdate command. It installs	#
#		    the SSA DASD device driver's trace hook		#
#		    parsing data.					#
#									#
#   IBM	CONFIDENTIAL --	(IBM Confidential Restricted when combined	#
#			 with the aggregate modules for	this project)	#
#									#
#   (C)	Copyright International	Business Machines Corp.	1995		#
#   All	rights reserved.						#
#									#
#########################################################################
* /etc/trcfmt

+ 45B 1.0 "SSA DASD"                                                    \
        $HDU,                                                           \          
        0x30 { "entry_finish:" devno=$D1 },                             \
        0x31 { "exit_finish:"  devno=$D1},                              \ 
        0x0F { strategy: bp=$D2 starttimer(0x45B,$D2) },                \ 
        0x1E { `coalesce: (pBuf,pIoreq)=($D2,$D3)` },                   \ 
        0x1F { iodone: devtoname($D1) bp=$D2 buftofilename($D2)         \ 
                endtimer(0x45B,$D2) },                                  \ 
        0x17 { $D2, 0 { $SKIP }                                         \
                bstart: devtoname($D1) bp=$D2 buftofilename($D2)        \ 
                        pblock=$D4 bcount=$D5 bflags($D3)               \ 
         },                                                             \ 
        \*   { $DD1 }
