#****************************************************************************
#   COMPONENT_NAME: PWRSYSX
#
#   FUNCTIONS:  cpuoff.s
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#****************************************************************************

        .file "cpuoff.s"
        .machine "ppc"
	.set     HID0, 1008

#**********************************************************************
#
#  NAME: SleepNeverReturn
#
#  FUNCTION:  CPU enters its sleep mode with interrupt disabled.      
#	      By entering sleep mode, QREQ signal is asserted.	
#	      The assertion of the signal is used to make PM chip
#	      to perform Vcc-B off operation without any software
#	      assistance.
#
#  INPUT STATE:
#	none	
#
#  OUTPUT STATE:
#	none
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#
#**********************************************************************

	S_PROLOG(SleepNeverReturn)


	lis	r6,0x0020
	mfspr	r5,HID0			## access HID0
	ori	r5,r6,0x0000		## Set sleep bit of HID0
	mtspr	HID0,r5

	lis	r6,0x0004
	mfmsr	r4
	ori	r4,r6,0x0000		## Enable CPU sleep mode
loop:	sync
	mtmsr	r4			## actually enter sleep mode	
	isync
	b 	loop

	S_EPILOG


#**********************************************************************
#
#  NAME: dbgstep
#
#  FUNCTION: single step entery 
#
#
#  INPUT STATE:
#	none	
#
#  OUTPUT STATE:
#	none
#
#  EXECUTION ENVIRONMENT:
# 	Supervisor state  : Yes
#
#**********************************************************************

	S_PROLOG(dbgstep)

	t	0x4,r1,r1

	S_EPILOG

