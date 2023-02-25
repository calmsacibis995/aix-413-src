# @(#)59  1.2 src/packages/devices/mca/8fd4/rte/root/devices.mca.8fd4.rte.err.S, pkgdevbase, pkg411, GOLD410  1/25/94  17:26:57
#
# COMPONENT_NAME: (SYSXLZ1) Compression Adapter - Makefile
#
# FUNCTIONS: N/A
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

*
*  Compression card detected a temporary PIO error
*
+ HLZ1_PIO_TEMP:
    Comment = "Data Compression Card recovered from a PIO error"
    Class = H
    Log = TRUE
    Report = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E874
    Prob_Causes = 3330, 0005
    Fail_Causes = E938, 0005
    Fail_Actions = E856, 0400
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  PIO error reset operation failed
*
+ HLZ1_RES_FAIL:
    Comment = "Data Compression Card PIO error reset failed"
    Class = H
    Log = TRUE
    Report = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E874
    Prob_Causes = 3330, 0005
    Fail_Causes = E938, 0005
    Fail_Actions = 0000, 0400
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Compression card detected a permanent PIO error
*
+ HLZ1_PIO_PERM:
    Comment = "Data Compression Card sustained unrecoverable PIO error"
    Class = H
    Log = TRUE
    Report = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = E874
    Prob_Causes = 3330, 0005
    Fail_Causes = E938, 0005
    Fail_Actions = 0000, 0400
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  DMA error
*
+ HLZ1_DMA_SYS:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card DMA transfer failed"
    Err_Desc = 1010
    Prob_Causes = 0005, 3300
    Fail_Causes = 0005, 3300
    Fail_Actions = F011, E803
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Sync/Async command timeout
*
+ HLZ1_COM_TO:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card request timeout"
    Err_Desc = 1002
    Prob_Causes = 3330, 3331
    Fail_Causes = 3300, E923
    Fail_Actions= 0000, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Adapter Reset Timeout
*
+ HLZ1_RESET_TO:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card initialization timeout"
    Err_Desc = 3210
    Prob_Causes = 3330, 3331
    Fail_Causes = 3300, E923
    Fail_Actions = 0000, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Microcode-detected device error
*
+ HLZ1_COM_TSB:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card request failed"
    Err_Desc = 1300
    Prob_Causes = 3330, 3331
    Fail_Causes = 3300, E923
    Fail_Actions = 0000, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  VPD read error
*
+ HLZ1_NO_VPD:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card VPD read error"
    Err_Desc = 1002
    Prob_Causes = 3330, 3331
    Fail_Causes = 3300, E923
    Fail_Actions = 0000, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Critical Initialization error
*
+ HLZ1_CRIT_INIT:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card critical initialization error"
    Err_Desc = 3210
    Prob_Causes = 3330
    Fail_Causes = 3300
    Fail_Actions= 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*  Non-critical Initialization error
*
+ HLZ1_NON_CRIT:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Data Compression Card non-critical initialization error"
    Err_Desc = 3210
    Prob_Causes = 3330
    Fail_Causes = 3300
    Fail_Actions= 0000, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

*
*   Driver and/or adapter in invalid state
*
+ HLZ1_STATE_INVAL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Driver and microcode in invalid state"
    Err_Desc = EC34
    Prob_Causes = 3331
    Fail_Causes = E983
    Fail_Actions = EB8E, 3000
    Detail_Data = 4, 003D, DEC
    Detail_Data = 8, 8190, HEX
    Detail_Data = 2, EB28, HEX
    Detail_Data = 6, 8121, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 4, EB29, HEX
    Detail_Data = 32, EB2A, HEX

