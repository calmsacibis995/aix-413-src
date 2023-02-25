# @(#)86        1.4  src/rspc/kernext/pm/wakeup/hrstart.m4, pwrsysx, rspc41J, 9517A_all 4/17/95 00:34:16
#
# COMPONENT_NAME: PWRSYSX
#
# FUNCTIONS: Makefile
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

####################
### Definitions  ###
####################
#
# processor version for 601/603/604
#
	.set    PV601,	1
	.set	PV603,	3 
	.set	PV604,	4
	
#
# 603/604 specific definition 
#
        .set    H0_603,		1008	# HID0 for 603
        .set    H0_604,		1008	# HID0 for 604

        .set    H0_603_ICE,	0x8000 	# I-Cache Enable
        .set    H0_603_DCE,	0x4000 	# D-Cache Enable
        .set    H0_603_ICFI,	0x0800 	# I-Cache Flash Invalidate
        .set    H0_603_DCI,	0x0400 	# D-Cache Flash Invalidate

	.set	H0_604_ICE,	0x8000	# I-Cache Enable
	.set	H0_604_DCE,	0x4000	# D-Cache Enable
	.set	H0_604_ICL,	0x2000	# I-Cache Lock
	.set	H0_604_DCL,	0x1000	# D-Cache Lock
	.set	H0_604_ICIA,	0x0800	# I-Cache Invalidate All
	.set	H0_604_DCIA,	0x0400	# D-Cache Invalidate All
	.set	H0_604_SSE,	0x0080	# Super Scalar Enable
	.set	H0_604_BHTE,	0x0004	# Branch History Table enable

	.set    H0_604_PRESET, H0_604_ICE+H0_604_DCE+H0_604_SSE+H0_604_BHTE

#
# cache line size definition  
#
        .set    C601_CACHE_LINES, 1024  # 601 cache lines
        .set    C603_CACHE_LINES,  128  # 603 cache lines
        .set    C604_CACHE_LINES,  512  # 604 cache lines

#**************************************************************
#
#  SPR_BITON(WorkReg,SprNum,SprBit)
#       The bit to be set must be in the least significant 16 bits.
#
#**************************************************************
define( SPR_BITON,
        `mfspr  $1,$2
        ori     $1,$1,$3
        sync
        isync
        mtspr   $2,$1
        isync
')

#**************************************************************
#
#  SPR_BITOFF(WorkReg,SprNum,SprBit)
#       The bit to be turned off can be in any of the 32 bits,
#       but there can not be more than one bit set.
#
#**************************************************************
define( SPR_BITOFF,
        `mfspr  $1,$2
        rlwinm  $1,$1,0,~$3
        sync
        isync
        mtspr   $2,$1
        isync
')


#
# Hibernation header
#       see _pm_hibernation_header structure in hibernation.h of
#       pmdump directory. These should be maintained if hibernation.h 
#       is changed.
#
        .set    signature,          0   #  "AIXPM001"  (8 bytes)
        .set    pwakeup_area,       8   #  physical address of wakeup area
        .set    wakeup_area_length,12   #  length of the wakeup area
        .set    wakeup_code_offset,16   #  offset of the wakeup code
        .set    wakeup_code_length,20   #  length of the wakeup code
        .set    config_data_offset,24   #  offset of the config data
        .set    config_data_length,28   #  length of the config data
        .set    prestart,          32   #  physical restart address
        .set    image_offset,      36   #  offset of compressed memory image
        .set    image_length,      40   #  length of compressed memory image
        .set    bitmap_offset,     44   #  offset of the bitmap data
        .set    bitmap_length,     48   #  length of the bitmap data
        .set    hhreserve,         52   #  reserve area (12 bytes)

        .set    header2wakeupcode, 1024 #  wakeup code offset from header

#
# Config Data structure
#                see _pm_config_data structure in hibernation.h
#
        .set    memory_size,        0   #  system memory size (in byte)
        .set    residual_sum,       4   #  check sum of residual data

#
# Stack, etc.
#
        .set    STK_FRM_SIZ, -1024
        .set    stacksize, 8192

#
# residual data
#       The Segs[] array must be excluded from check-sum.
#
        .set    start_segs,       516   #  start of Segs[] array
        .set    end_segs,        1284   #  next of Segs[] array

##############
###  MACRO ###
##############

# Debug Port Out
#   DBOUT(OutNum, WorkReg1, WorkReg2, WorkReg3)
#
ifdef(`PM_DEBUG',`
define( DBOUT, `
        SPR_BITOFF($4, H0_603,H0_603_DCE)       # D-cache off
        addi    $3, 0, 0x3bc
        oris    $3, $3, 0x8000
        addi    $2, 0, $1
        stb     $2, 0($3)
        SPR_BITON($4, H0_603,H0_603_DCE)        # D-cache on
')
',`
define( DBOUT, `# no debug output' )
')
