# @(#)12 1.1  src/packages/devices/mca/8f97/com/root/devices.mca.8f97.com.trc.S, mayaixpackaging, pkg41J, 9509A_all 2/20/95 21:31:03
#########################################################################
#									#
#   COMPONENT NAME: MAYAIXPACKAGING					#
#									#
#   FILE NAME:	    325 : ssa.rte.obj.trc.S				#
#		    410 : devices.mca.8f97.com.trc.S			#
#									#
#   ORIGINS:	    27							#
#									#
#   DESCRIPTION:    Stanza file for the trcupdate command. It installs	#
#		    the mayflower adapter device driver's trace hook	#
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

+ 45A 1.0 "SSA Adapter" 				\
        {{ $loc = X1 }} R1 				\
        $DD1  						\
        $loc, 						\
	0x01  { starttimer(0x01,$D1) }, 		\
	0x02  { endtimer(0x01,$D1) }, 			\
	0x03  { starttimer(0x03,$D1) }, 		\
	0x04  { endtimer(0x03,$D1) }, 			\
	0x09  { starttimer(0x09,$D1) }, 		\
	0x0A  { endtimer(0x09,$D1) }, 			\
	0x0D  { starttimer(0x0D,$D1) }, 		\
	0x0E  { endtimer(0x0D,$D1) }, 			\
	0x15  { starttimer(0x15,$D1) }, 		\
	0x16  { endtimer(0x15,$D1) }, 			\
	0xC0  { "IPN entry" starttimer(0xC0,0x557) }, 	\
	0xC1  { "IPN exit" endtimer(0xC0,0x557) }, 	\
	0xC2  { "Start IO entry" devtoname($D1) 	\
		starttimer(0xC2,$D1) }, 		\
	0xC3  { "Start IO exit" devtoname($D1) 		\
		endtimer(0xC2,$D1) }, 			\
	0xC4  { "Start GTCB" devtoname($D1) 		\
		"ID:" $D2 starttimer(0xC4,$D2) }, 	\
	0xC5  { "Complete GTCB" devtoname($D1) 		\
		"ID:" $D2 endtimer(0xC4,$D2) }, 	\
	0xC6  { "Start slave op" devtoname($D1) 	\
		"ID:" $D2				\
		 "Type:" $D3 starttimer(0xC6,$D2) }, 	\
	0xC7  { "Complete slave op" devtoname($D1) 	\
		"ID:" $D2 endtimer(0xC6,$D2) }, 	\
	0xC8  { "Start directive" 			\
		"ID:" $D1 starttimer(0xC8,$D1) },	\
	0xC9  { "End directive"				\
		"ID:" $D1 endtimer(0xC8,$D1) }, 	\
	0xCA  { "Start TCB" devtoname($D1)		\
		"ID:" $D2 starttimer(0xCA,$D2) },	\
	0xCB  { "End TCB" devtoname($D1)		\
		"ID:" $D2 endtimer(0xCA,$D2) }

