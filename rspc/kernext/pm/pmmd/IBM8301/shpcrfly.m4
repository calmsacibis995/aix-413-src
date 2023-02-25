# @(#)36	1.4  src/rspc/kernext/pm/pmmd/IBM8301/shpcrfly.m4, pwrsysx, rspc41J, 9516A_all 4/18/95 08:03:22
#
#   COMPONENT_NAME: PWRSYSX
#
#   FUNCTIONS:  shpcrfly.m4
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
#
#
#
#  special purpose register names
#

         .set    rsp, r1		# Stack Pointer
         .set    rtoc, r2		# TOC Pointer


         .set   dmiss,       976	# Data TLB Miss (603)
         .set   dcmp,        977	# Data TLB Compare (603)
         .set   imiss,       980	# Instruction TLB Miss (603)
         .set   icmp,        981	# Instruction TLB Compare (603)
         .set   rpa,         982	# Replacement Address (603)
         .set   TBL,         284	# Time Base Lower (603/4)
         .set   TBU,         285	# Time Base Upper (603/4)
         .set   MFRTCL,	     5		# Reading RTC Lower (601)
         .set   MFRTCU,      4		# Reading RTC Upper (601)
         .set   MTRTCL,      21		# Storing RTC Lower (601)
         .set   MTRTCU,      20		# Storing RTC Upper (601)
         .set   EAR,         282	# External Access Reg (601/3/4)
         .set   SDAR,        959	# Sample Data Address Reg (604) 
         .set   SIAR,        955	# Sample Instruction Address Reg (604) 
         .set   PMC2,        954	# Perf Monitor Reg 2 (604)
         .set   PMC1,        953	# Perf Monitor Reg 1 (604)
         .set   MMCR,        952	# Monitor Mode Control Reg (604)

         .set   SDR1,        25        #  SDR1 Storage Description Register #
         .set   HID0,        0x3f0     #  HID0 Special Purpose Register #
         .set   HID1,        0x3f1     #  HID1 Special Purpose Register #
         .set   HID2,        0x3f2     #  HID2 Special Purpose Register #
         .set   HID5,        0x3f5     #  HID5 Special Purpose Register #
         .set   H0_ICFI,     0x0800    #  HID0 I-Cache Flash Invalidate (603/4)
         .set   H0_DCFI,     0x0400    #  HID0 D-Cache Flash Invalidate (603/4)
         .set   H0_ICE,      0x8000    #  HID0 I-Cache Enable (603/4)
         .set   H0_DCE,      0x4000    #  HID0 D-Cache Enable (603/4)

         .set   TLB_CLASSES_601, 128   #  601 tlb has 128 congruence classes
         .set   TLB_CLASSES_604, 64    #  604 tlb has  64 congruence classes
         .set   TLB_CLASSES_603, 32    #  603 tlb has  32 congruence classes

	.set 	PVR_601,	1	# 601 Processor Version
	.set 	PVR_603,	3	# 603 Processor Version
	.set 	PVR_603_P,	6	# 603+ Processor Version
	.set 	PVR_604,	4	# 604 Processor Version
	.set 	PVR_604_P,	9	# 604+ Processor Version

#
#  Context Frame Offset and Flag Definitions
#
#	 The first 0x10 bytes are used for link register saving 
#        and reserved bytes. 
#
         .set   CxFpr0,  0x10
         .set   CxFpr1,  0x18
         .set   CxFpr2,  0x20
         .set   CxFpr3,  0x28
         .set   CxFpr4,  0x30
         .set   CxFpr5,  0x38
         .set   CxFpr6,  0x40
         .set   CxFpr7,  0x48
         .set   CxFpr8,  0x50
         .set   CxFpr9,  0x58
         .set   CxFpr10,  0x60
         .set   CxFpr11,  0x68
         .set   CxFpr12,  0x70
         .set   CxFpr13,  0x78
         .set   CxFpr14,  0x80
         .set   CxFpr15,  0x88
         .set   CxFpr16,  0x90
         .set   CxFpr17,  0x98
         .set   CxFpr18,  0xa0
         .set   CxFpr19,  0xa8
         .set   CxFpr20,  0xb0
         .set   CxFpr21,  0xb8
         .set   CxFpr22,  0xc0
         .set   CxFpr23,  0xc8
         .set   CxFpr24,  0xd0
         .set   CxFpr25,  0xd8
         .set   CxFpr26,  0xe0
         .set   CxFpr27,  0xe8
         .set   CxFpr28,  0xf0
         .set   CxFpr29,  0xf8
         .set   CxFpr30,  0x100
         .set   CxFpr31,  0x108
         .set   CxFpscr,  0x110
         .set   CxGpr0,  0x118
         .set   CxGpr1,  0x11c
         .set   CxGpr2,  0x120
         .set   CxGpr3,  0x124
         .set   CxGpr4,  0x128
         .set   CxGpr5,  0x12c
         .set   CxGpr6,  0x130
         .set   CxGpr7,  0x134
         .set   CxGpr8,  0x138
         .set   CxGpr9,  0x13c
         .set   CxGpr10,  0x140
         .set   CxGpr11,  0x144
         .set   CxGpr12,  0x148
         .set   CxGpr13,  0x14c
         .set   CxGpr14,  0x150
         .set   CxGpr15,  0x154
         .set   CxGpr16,  0x158
         .set   CxGpr17,  0x15c
         .set   CxGpr18,  0x160
         .set   CxGpr19,  0x164
         .set   CxGpr20,  0x168
         .set   CxGpr21,  0x16c
         .set   CxGpr22,  0x170
         .set   CxGpr23,  0x174
         .set   CxGpr24,  0x178
         .set   CxGpr25,  0x17c
         .set   CxGpr26,  0x180
         .set   CxGpr27,  0x184
         .set   CxGpr28,  0x188
         .set   CxGpr29,  0x18c
         .set   CxGpr30,  0x190
         .set   CxGpr31,  0x194
         .set   CxCr,  0x198
         .set   CxXer,  0x19c
         .set   CxMsr,  0x1a0
         .set   CxIar,  0x1a4
         .set   CxLr,  0x1a8
         .set   CxCtr,  0x1ac
         .set   CxContextFlags,  0x1b0
         .set   CxDr0,  0x1c0
         .set   CxDr1,  0x1c4
         .set   CxDr2,  0x1c8
         .set   CxDr3,  0x1cc
         .set   CxDr4,  0x1d0
         .set   CxDr5,  0x1d4
         .set   CxDr6,  0x1d8
         .set   CxDr7,  0x1dc
         .set   CxSrr0,  0x1e0		
         .set   CxSrr1,  0x1e4	
         .set   ContextFrameLengthExtended,  0x1e8


###
###  Extended LA/STW/LWZ macros for label names
###
###  WARNING : These macros use LR.
###

#**************************************************************
#
#  LLA(PointingReg,PointingLabel,LocalLabel)
#
#**************************************************************
define( LLA,
        `bl    $3
$3:
         mflr  $1
         la    $1, $2-$3($1)
')


#**************************************************************
#
#  LSTW(SaveReg,PointingReg,PointingLabel,LocalLabel)
#
#**************************************************************
define( LSTW,
        `LLA($2,$3,$4)
         stw   $1, 0($2)
')


#**************************************************************
#
#  LLWZ(RestoreReg,PointingReg,PointingLabel,LocalLabel)
#
#**************************************************************
define( LLWZ,
        `LLA($2,$3,$4)
         lwz   $1, 0($2)
')

#**************************************************************
#
#  S_PROLOG_NO_SECT(Name)
#
#**************************************************************
define( S_PROLOG_NO_SECT,
        `.globl ENTRY($1)   
         ENTRY($1):')

#**************************************************************
#
#  S_PROLOG_USING_LR(Name,SaveReg)
#
#**************************************************************
define( S_PROLOG_USING_LR,
        `S_PROLOG_NO_SECT($1)
         mflr  $2
')

#**************************************************************
#
#  S_EPILOC_USING_LR(Name,SaveReg)
#
#**************************************************************
define( S_EPILOG_USING_LR,
        `mtlr  $2
         S_EPILOG($1)
')



#
#  IBAT/DBAT save structure
#

       .set    SaveIBAT0L       , 0x0000
       .set    SaveIBAT0U       , 0x0004
       .set    SaveIBAT1L       , 0x0008
       .set    SaveIBAT1U       , 0x000C
       .set    SaveIBAT2L       , 0x0010
       .set    SaveIBAT2U       , 0x0014
       .set    SaveIBAT3L       , 0x0018
       .set    SaveIBAT3U       , 0x001C
       .set    SaveDBAT0L       , 0x0020
       .set    SaveDBAT0U       , 0x0024
       .set    SaveDBAT1L       , 0x0028
       .set    SaveDBAT1U       , 0x002C
       .set    SaveDBAT2L       , 0x0030
       .set    SaveDBAT2U       , 0x0034
       .set    SaveDBAT3L       , 0x0038
       .set    SaveDBAT3U       , 0x003C
       .set    SaveICMP         , 0x0040
       .set    SaveDCMP         , 0x0044
       .set    SaveSDR1         , 0x0048
       .set    SaveRPA          , 0x004C
       .set    SaveHID0         , 0x0050
       .set    SaveSr0          , 0x0054
       .set    SaveSr1          , 0x0058
       .set    SaveSr2          , 0x005C
       .set    SaveSr3          , 0x0060
       .set    SaveSr4          , 0x0064
       .set    SaveSr5          , 0x0068
       .set    SaveSr6          , 0x006C
       .set    SaveSr7          , 0x0070
       .set    SaveSr8          , 0x0074
       .set    SaveSr9          , 0x0078
       .set    SaveSr10         , 0x007C
       .set    SaveSr11         , 0x0080
       .set    SaveSr12         , 0x0084
       .set    SaveSr13         , 0x0088
       .set    SaveSr14         , 0x008C
       .set    SaveSr15         , 0x0090
       .set    SaveSprg0        , 0x0094
       .set    SaveSprg1        , 0x0098
       .set    SaveSprg2        , 0x009C
       .set    SaveSprg3        , 0x00A0
       .set    SaveDec          , 0x00A4
       .set    SaveTBL          , 0x00A8   # 603/4  RTCL for 601
       .set    SaveTBU          , 0x00AC   # 603/4  RTCU for 601
       .set    SaveHID1         , 0x00B0   # HID1 (601/3)
       .set    SaveHID2         , 0x00B4   # HID2 (601/3/4)
       .set    SaveHID5         , 0x00B8   # HID5 (601/4)
       .set    SaveMMCR         , 0x00BC   # Monitor Mode Control (604)
       .set    SavePMC1         , 0x00C0   # Performance Monitor Counter 1(604)
       .set    SavePMC2         , 0x00C4   # Performance Monitor Counter 2(604)
       .set    SaveSIAR         , 0x00C8   # Sample Instruction Address (604)  
       .set    SaveSDAR         , 0x00CC   # Sample Data Address (604)  
       .set    SaveEAR          , 0x00D0   # External Access Reg (601/3/4)

       .set    SaveBATStrucLength, 0x00D4



###
###  I/D-cache re-enable macros
###

#**************************************************************
#
#  REENABLE_ICACHE(WorkReg1,WorkReg2)
#
#**************************************************************
define(REENABLE_ICACHE,
        `mfspr   $1, HID0
	 rlwinm  $1, $1, 0, ~H0_DCE
         rlwinm  $2, $1, 0, ~H0_ICE
         ori     $2, $2, H0_ICFI
         ori     $1, $1, H0_ICE
         sync
         isync
         mtspr   HID0, $2
         isync
         mtspr   HID0, $1
         isync
')


#**************************************************************
#
#  REENABLE_BOTHCACHE(WorkReg)
#
#**************************************************************
define( REENABLE_BOTHCACHE,
        `mfspr   $1, HID0
	 ori     $1, $1, (H0_ICFI+H0_DCFI)
	 sync
	 isync
         mtspr   HID0,$1
	 rlwinm  $1, $1, 0, ~(H0_ICFI+H0_DCFI)
	 sync
	 isync
         mtspr   HID0,$1
         ori     $1, $1, (H0_ICE+H0_DCE)
 	 sync
 	 isync
         mtspr   HID0,$1
         isync
')

