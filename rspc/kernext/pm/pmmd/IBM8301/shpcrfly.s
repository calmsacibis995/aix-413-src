# @(#)37	1.5  src/rspc/kernext/pm/pmmd/IBM8301/shpcrfly.s, pwrsysx, rspc41J, 9519A_all 5/1/95 14:42:34
#
#   COMPONENT_NAME: PWRSYSX
#
#   FUNCTIONS:  shpcrfly.s
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

	.file	"shpcrfly.s"

#  Abstract:
#
#     This module contains;
#     . wake up code from address translation disabled
#

include(shpcrfly.m4)

#  	callback routine from ASM to C after register save
       .extern .ProcessorResetFromAsmToC 


#
#  static data area used only in this module
#
       .align 3


#
#  Routine Description:
#
#     GetReturnToVirtualEntry        : Get ReturnToVirtualEntry after RFI
#     GetResumeEntry             : Get ShcResumeEntry
#     RegisterReturnToVirtualEntry   : Register ReturnToVirtualEntry after RFI
#
#  Arguments:
#
#     GetReturnToVirtualEntry        : None.
#     GetResumeEntry             : None.
#     RegisterReturnToVirtualEntry   : ReturnToVirtualEntry (logical address)
#
#  Return Value:
#
#     GetReturnToVirtualEntry        : ReturnToVirtualEntry (logical address)
#     GetResumeEntry             : ShcResumeEntry   (logical address)
#     RegisterReturnToVirtualEntry   : None.
#
#

# GetReturnToVirtualEntry
        S_PROLOG_USING_LR(GetReturnToVirtualEntry, r0 )
        LLA( r3, ReturnToVirtualEntry, GRTHE.001 )
        S_EPILOG_USING_LR(GetReturnToVirtualEntry, r0 )

# GetResumeEntry
        S_PROLOG_USING_LR(GetResumeEntry, r0 )
        LLA( r3, ShcResumeEntry.EntryPoint, GRE.001)
        S_EPILOG_USING_LR(GetResumeEntry, r0 )

# RegisterReturnToVirtualEntry
        S_PROLOG_USING_LR(RegisterReturnToVirtualEntry, r0 )
        LSTW( r3, r9, SaveReturnToVirtualEntry, RRTHE.001 )
        S_EPILOG_USING_LR(RegisterReturnToVirtualEntry, r0 )

# GetContiguousCodeSize
        S_PROLOG_USING_LR(GetContiguousCodeSize, r0 )
        LLA( r5, RelocatableCodeStart,GCCS.002 )
        stw     r5, 0(r3)                             #  entry address
        LLA( r6, RelocatableCodeEnd,GCCS.001 )
        subf    r5, r5, r6                            #  r5 = r6 - r5
        stw     r5, 0(r4)                             #  code size
        S_EPILOG_USING_LR(GetContiguousCodeSize, r0 )

# GetRelocatableCodeStart
        S_PROLOG_USING_LR(GetRelocatableCodeStart, r0 )
        LLA( r3, RelocatableCodeStart,GRCS.001 )
        S_EPILOG_USING_LR(GetRelocatableCodeStart, r0 )

# GetRelocatableCodeExit
        S_PROLOG_USING_LR(GetRelocatableCodeExit, r0 )
        LLA( r3, RelocatableCodeEnd,GRCE.001 )
        S_EPILOG_USING_LR(GetRelocatableCodeExit, r0 )


#
#  Routine Description:
#
#     ProcessorReset : Reset the processor with saving/restoring its state.
#
#  Arguments:
#    	R3 for working area as a dummy. 
#
#  Return Value:
#
#     None.
#
#  Affected Registers
#
#     Unknown
#


        S_PROLOG_NO_SECT(ProcessorReset)
        mflr r3                                             #
        stwu rsp,-(ContextFrameLengthExtended)(rsp)   	    # get stack space
        stw  r3,CxLr(rsp)  				    # save lr


#  save msr and disable ext-int

        mfmsr   r3
        stw     r3, CxMsr(rsp)
        rlwinm  r3,r3,0,~(1 < (0x1f-0x10))
        isync
        mtmsr   r3	# clear MSR EE
  	sync		# Note that printf in C implictly contains i_enable.	

#  Start to save registers
#  	save save srr0/srr1

        mfsrr0  r3
        stw     r3, CxSrr0(rsp)

        mfsrr1  r3
        stw     r3, CxSrr1(rsp)

#  save FPRs
        bl      ENTRY(SaveFPR)

#  save gprs except rtoc
        bl      ENTRY(SaveGpr)
 
#  save rtoc
        stw     rtoc, CxGpr2(rsp)

#  save rsp (in local)
        LSTW( rsp, r9, SaveStackPtr,PR.001 )

#  save SPRS
        LLA( r9, SaveBATArea,PR.002)
        bl      ENTRY(SaveSpr)           #  restored at resume in real mode


        bl      ENTRY(ProcessorResetFromAsmToC) 
#-----------------------------------------------------------------------
# 	Going to suspend or hibernation
#-----------------------------------------------------------------------
#  At the above calling, either suspend or hibernatio process is called.
#  Those are C code. Normally, the control never returns here.
#-----------------------------------------------------------------------


#-----------------------------------------------------------------------
#       Coming back from resume from suspend or hibernation
#	   Before here, some code with translation off are run.     
#	   Refer to the code entry named ShcResumeEntry.EntryPoint
#	   which is called by ROS or hibernation wakeup code. It 
#          located below in this file.
#-----------------------------------------------------------------------
#  Translation enabled again (comming from resume from suspend or hibernation)
#  Jump to this place using "rfi".
#  TLB miss exception must occur at the same time jumping here.
#-----------------------------------------------------------------------
ReturnToVirtualEntry:

#  restore rsp (in local)
        LLWZ( rsp, r9, SaveStackPtr,PR.003 )


#  restore rtoc
        lwz     rtoc, CxGpr2(rsp)             

#  restore gprs except rtoc
        bl      ENTRY(RestoreGpr)


#  restore FPRs
        bl      ENTRY(RestoreFPR)

#  restore srr0/srr1, restore msr
        lwz     r3, CxSrr0(rsp)
        mtsrr0  r3

        lwz     r3, CxSrr1(rsp)
        mtsrr1  r3

        lwz     r3, CxMsr(rsp)
        isync
        mtmsr   r3
	sync


        lwz r3,CxLr(rsp)  				    #
        mtlr r3                                             # retrieve lr
	lwz  rsp,0(rsp)   	    			    # get stack space

        S_EPILOG(ProcessorReset)


#
#  Routine Description:
#
#     SaveGpr    : Save general purpose registers
#     SaveFPRs    : Save FPRs
#
#  Arguments:
#
#     None.
#
#  Return Value:
#
#     None.
#
#  Affected Registers
#
#     GPRs
#     FPRs
#

        S_PROLOG_NO_SECT(SaveGpr)

        stw     r0  , CxGpr0(rsp)             
        stw     r4  , CxGpr4(rsp)             #  probably no need
        stw     r5  , CxGpr5(rsp)             #  probably no need
        stw     r6  , CxGpr6(rsp)             #  probably no need
        stw     r7  , CxGpr7(rsp)             #  probably no need
        stw     r8  , CxGpr8(rsp)
        stw     r9  , CxGpr9(rsp)
        stw     r10 , CxGpr10(rsp)
        stw     r11 , CxGpr11(rsp)
        stw     r12 , CxGpr12(rsp)
        stw     r13 , CxGpr13(rsp)
        stw     r14 , CxGpr14(rsp)
        stw     r15 , CxGpr15(rsp)
        stw     r16 , CxGpr16(rsp)
        stw     r17 , CxGpr17(rsp)
        stw     r18 , CxGpr18(rsp)
        stw     r19 , CxGpr19(rsp)
        stw     r20 , CxGpr20(rsp)
        stw     r21 , CxGpr21(rsp)
        stw     r22 , CxGpr22(rsp)
        stw     r23 , CxGpr23(rsp)
        stw     r24 , CxGpr24(rsp)
        stw     r25 , CxGpr25(rsp)
        stw     r26 , CxGpr26(rsp)
        stw     r27 , CxGpr27(rsp)
        stw     r28 , CxGpr28(rsp)
        stw     r29 , CxGpr29(rsp)
        stw     r30 , CxGpr30(rsp)
        stw     r31 , CxGpr31(rsp)

        S_EPILOG(SaveGpr)

#
#  Routine Description:
#
#     RestoreGpr : Restore general purpose registers
#     RestoreFPRs : Restore FPRs
#
#  Arguments:
#
#     None.
#
#  Return Value:
#
#     None.
#
#  Affected Registers
#
#     GPRs
#     FPRs
#

        S_PROLOG_NO_SECT(RestoreGpr)

        lwz     r0  , CxGpr0(rsp)             
        lwz     r4  , CxGpr4(rsp)             #  probably no need
        lwz     r5  , CxGpr5(rsp)             #  probably no need
        lwz     r6  , CxGpr6(rsp)             #  probably no need
        lwz     r7  , CxGpr7(rsp)             #  probably no need
        lwz     r8  , CxGpr8(rsp)
        lwz     r9  , CxGpr9(rsp)
        lwz     r10 , CxGpr10(rsp)
        lwz     r11 , CxGpr11(rsp)
        lwz     r12 , CxGpr12(rsp)
        lwz     r13 , CxGpr13(rsp)
        lwz     r14 , CxGpr14(rsp)
        lwz     r15 , CxGpr15(rsp)
        lwz     r16 , CxGpr16(rsp)
        lwz     r17 , CxGpr17(rsp)
        lwz     r18 , CxGpr18(rsp)
        lwz     r19 , CxGpr19(rsp)
        lwz     r20 , CxGpr20(rsp)
        lwz     r21 , CxGpr21(rsp)
        lwz     r22 , CxGpr22(rsp)
        lwz     r23 , CxGpr23(rsp)
        lwz     r24 , CxGpr24(rsp)
        lwz     r25 , CxGpr25(rsp)
        lwz     r26 , CxGpr26(rsp)
        lwz     r27 , CxGpr27(rsp)
        lwz     r28 , CxGpr28(rsp)
        lwz     r29 , CxGpr29(rsp)
        lwz     r30 , CxGpr30(rsp)
        lwz     r31 , CxGpr31(rsp)

        S_EPILOG(RestoreGpr)


#
#  Routine Description:
#
#     SaveFPRs    : Save FPRs
#     RestoreFPRs : Restore FPRs
#
#  Arguments:
#
#     None.
#
#  Return Value:
#
#     None.
#
#  Affected Registers
#
#     FPRs
#


        S_PROLOG_NO_SECT(SaveFPR)

        mfmsr   r3
        ori     r3, r3, (1 < (0x1f-0x12))     #  enable FP availability
        mtmsr   r3
        isync

        mfctr   r3
        stw     r3, CxCtr(rsp)
        mfxer   r3
        stw     r3, CxXer(rsp)
	mfcr	r3
	stw	r3, CxCr(rsp)

        stfd    f0, CxFpr0(rsp)
        stfd    f1, CxFpr1(rsp)
        stfd    f2, CxFpr2(rsp)
        stfd    f3, CxFpr3(rsp)
        stfd    f4, CxFpr4(rsp)
        stfd    f5, CxFpr5(rsp)
        stfd    f6, CxFpr6(rsp)
        stfd    f7, CxFpr7(rsp)
        stfd    f8, CxFpr8(rsp)
        stfd    f9, CxFpr9(rsp)
        stfd    f10,CxFpr10(rsp)
        stfd    f11,CxFpr11(rsp)
        stfd    f12,CxFpr12(rsp)
        stfd    f13,CxFpr13(rsp)
        stfd    f14,CxFpr14(rsp)
        stfd    f15,CxFpr15(rsp)
        stfd    f16,CxFpr16(rsp)
        stfd    f17,CxFpr17(rsp)
        stfd    f18,CxFpr18(rsp)
        stfd    f19,CxFpr19(rsp)
        stfd    f20,CxFpr20(rsp)
        stfd    f21,CxFpr21(rsp)
        stfd    f22,CxFpr22(rsp)
        stfd    f23,CxFpr23(rsp)
        stfd    f24,CxFpr24(rsp)
        stfd    f25,CxFpr25(rsp)
        stfd    f26,CxFpr26(rsp)
        stfd    f27,CxFpr27(rsp)
        stfd    f28,CxFpr28(rsp)
        stfd    f29,CxFpr29(rsp)
        stfd    f30,CxFpr30(rsp)
        stfd    f31,CxFpr31(rsp)

        mffs    f0
        stfd    f0, CxFpscr(rsp)

        S_EPILOG(SaveFPR)

        S_PROLOG_NO_SECT(RestoreFPR)

        mfmsr   r3
        ori     r3, r3, (1 < (0x1f-0x12))     #  enable FP availability
        mtmsr   r3
        isync

        lwz     r3, CxCtr(rsp)
        mtctr   r3
        lwz     r3, CxXer(rsp)
        mtxer   r3
	lwz	r3, CxCr(rsp)
	mtcrf   0x80,r3			# restore cr0 - maybe meaningless
	mtcrf	0x40,r3			# restore cr1 - maybe meaningless
	mtcrf	0x20,r3			# restore cr2
	mtcrf	0x10,r3			# restore cr3
	mtcrf	0x08,r3			# restore cr4
	mtcrf	0x04,r3			# restore cr5
	mtcrf	0x02,r3			# restore cr6
	mtcrf	0x01,r3			# restore cr7

        lfd     f1, CxFpr1(rsp)
        lfd     f2, CxFpr2(rsp)
        lfd     f3, CxFpr3(rsp)
        lfd     f4, CxFpr4(rsp)
        lfd     f5, CxFpr5(rsp)
        lfd     f6, CxFpr6(rsp)
        lfd     f7, CxFpr7(rsp)
        lfd     f8, CxFpr8(rsp)
        lfd     f9, CxFpr9(rsp)
        lfd     f10,CxFpr10(rsp)
        lfd     f11,CxFpr11(rsp)
        lfd     f12,CxFpr12(rsp)
        lfd     f13,CxFpr13(rsp)
        lfd     f14,CxFpr14(rsp)
        lfd     f15,CxFpr15(rsp)
        lfd     f16,CxFpr16(rsp)
        lfd     f17,CxFpr17(rsp)
        lfd     f18,CxFpr18(rsp)
        lfd     f19,CxFpr19(rsp)
        lfd     f20,CxFpr20(rsp)
        lfd     f21,CxFpr21(rsp)
        lfd     f22,CxFpr22(rsp)
        lfd     f23,CxFpr23(rsp)
        lfd     f24,CxFpr24(rsp)
        lfd     f25,CxFpr25(rsp)
        lfd     f26,CxFpr26(rsp)
        lfd     f27,CxFpr27(rsp)
        lfd     f28,CxFpr28(rsp)
        lfd     f29,CxFpr29(rsp)
        lfd     f30,CxFpr30(rsp)
        lfd     f31,CxFpr31(rsp)

        lfd     f0, CxFpscr(rsp)
        mtfsf   0xff, f0

        lfd     f0, CxFpr0(rsp)

        S_EPILOG(RestoreFPR)



#  ***********************************
#  ***    RelocatableCode (Begin)  ***
#  ***********************************
#      This area should be within a certain page (4K bytes).
#      But when this area is loaded, no one can load this area to put within
#      a certatin pare.  Then after loaded, it is checked and if this area is
#      not within in a page, this area is moved to the reserved area following
#      this area.
#      Then this area size should be less than 2048 bytes.
#
#
        .globl  RelocatableCodeStart
RelocatableCodeStart:                           #  start of p-code


#
#  Routine Description:
#
#     ShcResumeEntry : Entry point from the system reset entry
#     This code must be relocatable without the loader's help.
#
#  Arguments:
#
#     None.
#
#  Return Value:
#
#     None.(No exit and jump to ReturnToVirtualEntry)
#
#  Affected Registers
#
#     Unknown
#
#  Note:
#     Do not use   S_PROLOG(ShcResumeEntry)  for this routine.
#     Because S_PROLOG includes .csect and this breaks the continuity of
#     "RelocatableCode".
#

        .globl ShcResumeEntry           # instead of S_PROLOG
ShcResumeEntry:

SaveReturnToVirtualEntry:
        .long   0                               #  l-addr accessed by p-code
SaveStackPtr:
        .long   0                               #  l-addr accessed by p-code
SaveBATArea:
        .space  SaveBATStrucLength

#
#  Physical Address Entry (From wakeup)
#

ShcResumeEntry.EntryPoint:
        sync                                    #
        sync                                    #
        sync                                    #
        sync                                    #
        sync                                    #

        mfmsr   r3
        stw     r3, CxMsr(rsp)
        rlwinm  r3,r3,0,~(1 < (0x1f-0x10))
        isync
        mtmsr   r3	# clear MSR EE for fail safe
  	sync		

#
#       Following are required to invalidate all of IBATs/DBATs
#       in physical address mode
#

        li      r10, 0                         #  BATs invalidate
        mtibatu 0, r10                         #
        mtibatu 1, r10                         #
        mtibatu 2, r10                         #
        mtibatu 3, r10                         #
	mfpvr   r9                             #  check processor type
        srwi    r9,r9,16                       #  extract processor version
        cmpwi   cr6,r9,PVR_601                 #  is this a 601?
        cmpwi   cr7,r9,PVR_604                 #  is this a 604?
        beq     cr6,skip_601                   #  branch if 601
	beq	cr7, no_chk_604P
        cmpwi   cr7,r9,PVR_604_P               #  is this a 604+?
no_chk_604P:
        mtdbatu 0, r10                         #
        mtdbatu 1, r10                         #
        mtdbatu 2, r10                         #
        mtdbatu 3, r10                         #

skip_601:

#  restore SPRS
        LLA( r9, SaveBATArea,HRE.003)
        bl       ENTRY(RestoreSpr)


        LLWZ( r9, r10, SaveReturnToVirtualEntry,HRE.004 )
        cmpwi   r9, 0
        bne     ShcResumeEntry.Continue
        b       $                              #  unexpected resume

ShcResumeEntry.Continue:

TLB_invalid:
#----------------------------------------------
#         TLB flush
#----------------------------------------------
# The following C code is written down to ASM.
#       int  pno;
#       for (pno = 0; pno < 32; pno++)
#               tlbie(pno*4096);
#----------------------------------------------
        beq     cr6,for_601                   #  branch if 601
        beq     cr7,for_604                   #  branch if 604
for_601:
        cal     r3,TLB_CLASSES_601(0) 	       # All TLBs are flushed here. 
	b	around
for_604:
        cal     r3,TLB_CLASSES_604(0) 	       # All TLBs are flushed here. 
	b	around
around:
        mtctr   r3                  	       # move to count register
        cal     r10,0(0)              	       # clear r10
        rlinm   r3,r10,12,0,19                 # hit addresses within 0xNNNNN000
        tlbie   r3                 	       # invalidate TLBs
        ai      r10,r10,1                      # 
        bc      16,0,$-0xc		       # Decrement count and branch!=0
	sync



        mfmsr   r10                                  #
        rlwinm  r10,r10,0,~(1 < (0x1f-0x19))         #  Clear MSR(IP)
        mtmsr   r10                                  #  
        ori     r10, r10, (1 < (0x1f-0x1b))          #  d-relocation on
        ori     r10, r10, (1 < (0x1f-0x1a))          #  i-relocation on
        mtsrr1  r10                                  #  set next msr
        mtsrr0  r9                                   #  start of code

#----------------------------------------------
# Now going back to "ReturnToVirtualEntry" with translation on
#----------------------------------------------
        rfi

        .align 2


#
#  Routine Description:
#
#     RestoreSpr: Restore special purpose registers to be set before
#                 entering the virtual mode.
#
#  Arguments:
#
#     None
#
#  Return Value:
#
#     None
#
#  Affected Registers
#
#     r3/r4/r5
#
#  Note:
#     Do not use   S_PROLOG(RestoreSpr)  for this routine.
#     Because S_PROLOG includes .csect and this breaks the continuity of
#     "RelocatableCode".
#

        S_PROLOG_NO_SECT(RestoreSpr)    

        lwz     r3, SaveIBAT0L(r9)
        mtibatl 0, r3
        lwz     r3, SaveIBAT0U(r9)
        mtibatu 0, r3
        lwz     r3, SaveIBAT1L(r9)
        mtibatl 1, r3
        lwz     r3, SaveIBAT1U(r9)
        mtibatu 1, r3
        lwz     r3, SaveIBAT2L(r9)
        mtibatl 2, r3
        lwz     r3, SaveIBAT2U(r9)
        mtibatu 2, r3
        lwz     r3, SaveIBAT3L(r9)
        mtibatl 3, r3
        lwz     r3, SaveIBAT3U(r9)
        mtibatu 3, r3

        beq     cr6,is_601                   #  branch if 601

        lwz     r3, SaveDBAT0L(r9)
        mtdbatl 0, r3
        lwz     r3, SaveDBAT0U(r9)
        mtdbatu 0, r3
        lwz     r3, SaveDBAT1L(r9)
        mtdbatl 1, r3
        lwz     r3, SaveDBAT1U(r9)
        mtdbatu 1, r3
        lwz     r3, SaveDBAT2L(r9)
        mtdbatl 2, r3
        lwz     r3, SaveDBAT2U(r9)
        mtdbatu 2, r3
        lwz     r3, SaveDBAT3L(r9)
        mtdbatl 3, r3
        lwz     r3, SaveDBAT3U(r9)
        mtdbatu 3, r3

is_601:

#  restore SRs
        isync
        lwz     r3, SaveSr0(r9)
        mtsr    0, r3
        lwz     r3, SaveSr1(r9)
        mtsr    1, r3
        lwz     r3, SaveSr2(r9)
        mtsr    2, r3
        lwz     r3, SaveSr3(r9)
        mtsr    3, r3
        lwz     r3, SaveSr4(r9)
        mtsr    4, r3
        lwz     r3, SaveSr5(r9)
        mtsr    5, r3
        lwz     r3, SaveSr6(r9)
        mtsr    6, r3
        lwz     r3, SaveSr7(r9)
        mtsr    7, r3
        lwz     r3, SaveSr8(r9)
        mtsr    8, r3
        lwz     r3, SaveSr9(r9)
        mtsr    9, r3
        lwz     r3, SaveSr10(r9)
        mtsr    10, r3
        lwz     r3, SaveSr11(r9)
        mtsr    11, r3
        lwz     r3, SaveSr12(r9)
        mtsr    12, r3
        lwz     r3, SaveSr13(r9)
        mtsr    13, r3
        lwz     r3, SaveSr14(r9)
        mtsr    14, r3
        lwz     r3, SaveSr15(r9)
        mtsr    15, r3
        isync

        beq     cr7,is_604_1                   #  branch if 604

#  restore HID1
        lwz     r3, SaveHID1(r9)
	mtspr	HID1, r3

        beq     cr6,is_601_1                   #  branch if 601

#  restore ICMP
        lwz     r3, SaveICMP(r9)
        mtspr   icmp, r3
#  restore DCMP
        lwz     r3, SaveDCMP(r9)
        mtspr   dcmp, r3
#  restore RPA
        lwz     r3, SaveRPA(r9)
        mtspr   rpa, r3
	
	b 	common

is_604_1:

#  restore SDAR
        lwz     r3, SaveSDAR(r9)
	mtspr	SDAR, r3
#  restore SIAR
        lwz     r3, SaveSIAR(r9)
	mtspr	SIAR, r3
#  restore PMC2
        lwz     r3, SavePMC2(r9)
	mtspr	PMC2, r3
#  restore PMC1
        lwz     r3, SavePMC1(r9)
	mtspr	PMC1, r3
#  restore MMCR
        lwz     r3, SaveMMCR(r9)
	mtspr	MMCR, r3

is_601_1:

#  restore HID5
        lwz     r3, SaveHID5(r9)
	mtspr	HID5, r3

common:

#  restore EAR
        lwz     r3, SaveEAR(r9)
        mtspr   EAR, r3
#  restore SDR1
        lwz     r3, SaveSDR1(r9)
        mtspr   SDR1, r3
#  restore HID2
        lwz     r3, SaveHID2(r9)
	mtspr	HID2, r3
#  restore HID0
        lwz     r3, SaveHID0(r9)
	mtspr	HID0, r3		# do this for 601...reenable_icache
					# clobbers r3 anyway

#  restore sprg0-3 for TLB imiss/dmiss exceptions after I/D translation-on
        lwz     r3, SaveSprg0(r9)
        mtsprg  0, r3
        lwz     r3, SaveSprg1(r9)
        mtsprg  1, r3
        lwz     r3, SaveSprg2(r9)
        mtsprg  2, r3
        lwz     r3, SaveSprg3(r9)
        mtsprg  3, r3

#  restore decrement counter
        lwz     r3, SaveDec(r9)		# This value is a huge one so that dec
        mtdec   r3			# exception can't be generated for a 
					# following critical period.

        isync			# This needs for errata of early version of 603


#  restore time base counter
        lwz     r3, SaveTBU(r9)
        lwz     r4, SaveTBL(r9)


        li      r5, 0                          #  prevent TB carry-up
        beq     cr6,is_601_3                   #  branch if 601
        mttbl   r5             #  supervisor ! #  force TBL to 0


        mttbu   r3             #  supervisor ! #  set TBU
        mttbl   r4             #  supervisor ! #  set TBL
	b	out
is_601_3:
.machine "601"

	mtrtcl	r5
	mtrtcu  r3		
	mtrtcl	r4
.machine "603"

out:
        S_EPILOG(RestoreSpr)    
        .align 2

#  ***********************************
#  ***    RelocatableCode (End)    ***
#  ***********************************
        .globl  RelocatableCodeEnd
RelocatableCodeEnd:                             #  end of p-code


#
#  Reserved Area for alternative RelocatableCode
#
#     If the previous relocatable code area is on the boundary of pages and
#     those pages are not contiguous, the previous relocatable code is copied
#     to this area and it is used during resume from Suspend/Hibernation.
#     From this reason, the size of the previous relocatable code should be
#     less than 2048 bytes.
#
#  ***********************************
        .space   2048
#  ***********************************



#
#  Routine Description:
#
#     SaveSpr: Save special purpose registers to be set before
#              entering the virtual mode.
#
#  Arguments:
#
#     None
#
#  Return Value:
#
#     None
#
#  Affected Registers
#
#     r3/r4/r5
#
        S_PROLOG_NO_SECT(SaveSpr)

	mfpvr   r3                      #  check processor type
        srwi    r3,r3,16                #  extract processor version
        cmpwi   cr7,r3,PVR_604          #  is this a 604?
        cmpwi   cr6,r3,PVR_601          #  is this a 601?
	beq	cr7, skip_chk_604P
        cmpwi   cr7,r3,PVR_604_P        #  is this a 604+?

skip_chk_604P:
        mfibatl r3, 0
        stw     r3, SaveIBAT0L(r9)
        mfibatu r3, 0
        stw     r3, SaveIBAT0U(r9)
        mfibatl r3, 1
        stw     r3, SaveIBAT1L(r9)
        mfibatu r3, 1
        stw     r3, SaveIBAT1U(r9)
        mfibatl r3, 2
        stw     r3, SaveIBAT2L(r9)
        mfibatu r3, 2
        stw     r3, SaveIBAT2U(r9)
        mfibatl r3, 3
        stw     r3, SaveIBAT3L(r9)
        mfibatu r3, 3
        stw     r3, SaveIBAT3U(r9)

	beq	cr7, is_604_5

#  save HID1
        mfspr   r3, HID1
        stw     r3, SaveHID1(r9)

        beq     cr6,is_601_10              #  branch if 601

is_604_5:
        mfdbatl r3, 0
        stw     r3, SaveDBAT0L(r9)
        mfdbatu r3, 0
        stw     r3, SaveDBAT0U(r9)
        mfdbatl r3, 1
        stw     r3, SaveDBAT1L(r9)
        mfdbatu r3, 1
        stw     r3, SaveDBAT1U(r9)
        mfdbatl r3, 2
        stw     r3, SaveDBAT2L(r9)
        mfdbatu r3, 2
        stw     r3, SaveDBAT2U(r9)
        mfdbatl r3, 3
        stw     r3, SaveDBAT3L(r9)
        mfdbatu r3, 3
        stw     r3, SaveDBAT3U(r9)

	beq	cr7, is_604_10

#  save ICMP
        mfspr   r3, icmp
        stw     r3, SaveICMP(r9)
#  save DCMP
        mfspr   r3, dcmp
        stw     r3, SaveDCMP(r9)
#  save RPA
        mfspr   r3, rpa
        stw     r3, SaveRPA(r9)

	b	common_10

is_604_10:
#  save SDAR
	mfspr	r3, SDAR
        stw     r3, SaveSDAR(r9)
#  save SIAR
	mfspr	r3, SIAR
        stw     r3, SaveSIAR(r9)
#  save PMC2
	mfspr	r3, PMC2
        stw     r3, SavePMC2(r9)
#  save PMC1
	mfspr	r3, PMC1
        stw     r3, SavePMC1(r9)
#  save MMCR
	mfspr	r3, MMCR
        stw     r3, SaveMMCR(r9)

is_601_10:

#  save HID5
	mfspr	r3, HID5
        lwz     r3, SaveHID5(r9)

common_10:

#  save EAR
        mfspr   r3, EAR
        stw     r3, SaveEAR(r9)
#  save HID0
        mfspr   r3, HID0
        stw     r3, SaveHID0(r9)
#  save HID2
        mfspr   r3, HID2
        stw     r3, SaveHID2(r9)
#  save SDR1
        mfspr   r3, SDR1
        stw     r3, SaveSDR1(r9)

#  save SRs
        isync
        mfsr    r3, 0
        stw     r3, SaveSr0(r9)
        mfsr    r3, 1
        stw     r3, SaveSr1(r9)
        mfsr    r3, 2
        stw     r3, SaveSr2(r9)
        mfsr    r3, 3
        stw     r3, SaveSr3(r9)
        mfsr    r3, 4
        stw     r3, SaveSr4(r9)
        mfsr    r3, 5
        stw     r3, SaveSr5(r9)
        mfsr    r3, 6
        stw     r3, SaveSr6(r9)
        mfsr    r3, 7
        stw     r3, SaveSr7(r9)
        mfsr    r3, 8
        stw     r3, SaveSr8(r9)
        mfsr    r3, 9
        stw     r3, SaveSr9(r9)
        mfsr    r3, 10
        stw     r3, SaveSr10(r9)
        mfsr    r3, 11
        stw     r3, SaveSr11(r9)
        mfsr    r3, 12
        stw     r3, SaveSr12(r9)
        mfsr    r3, 13
        stw     r3, SaveSr13(r9)
        mfsr    r3, 14
        stw     r3, SaveSr14(r9)
        mfsr    r3, 15
        stw     r3, SaveSr15(r9)
        isync
#  save sprg0-3
        mfsprg  r3, 0
        stw     r3, SaveSprg0(r9)
        mfsprg  r3, 1
        stw     r3, SaveSprg1(r9)
        mfsprg  r3, 2
        stw     r3, SaveSprg2(r9)
        mfsprg  r3, 3
        stw     r3, SaveSprg3(r9)

#  dec
#    Get the faked dec value here so that dec interrupt can't be generated
#    until the primitive procedure finishes at resume sequence. This value
#    will be used at resume.
	lis	r3, 0x7fff
	ori	r3, r3, 0xffff	
        stw     r3, SaveDec(r9)

#  save time base counter
LoopTBGet:
	beq	cr6, is_601_11	# branch if 601
        mfspr   r3, TBU        #  Supervisor ! #  load from TBU
        mfspr   r4, TBL        #  Supervisor ! #  load from TBL

        mfspr   r4, TBL        #  Supervisor ! #  load from TBL
        mfspr   r4, TBL        #  Supervisor ! #  load from TBL
        mfspr   r4, TBL        #  Supervisor ! #  load from TBL

        mfspr   r5, TBU        #  Supervisor ! #  load from TBU once
	b	past
is_601_11:
.machine "601"
	mfrtcu	r3
	mfrtcl	r4
	mfrtcu	r5
.machine "603"

past:
        cmpw    r5, r3                         #  see if 'old' == 'new'
        bne     LoopTBGet                      #  loop if carry occurred
        stw     r3, SaveTBU(r9)
        stw     r4, SaveTBL(r9)

        S_EPILOG(SaveSpr)

#
#  Routine Description:
#
#     MakeDECavailable: Make Decrementor exception available  
#             		Because DEC was set with a huge number so that 	
#		        decrementor exception can't be generated during	
#			resume critical procedure.  
#  Arguments:
#
#     None
#
#  Return Value:
#
#     None
#
#  Affected Registers
#
#     r3
#
        S_PROLOG_NO_SECT(MakeDECavailable)

	lis	r3, 0
	ori	r3, r3, 0x1
	mtdec	r3

        S_EPILOG(MakeDECavailable)
