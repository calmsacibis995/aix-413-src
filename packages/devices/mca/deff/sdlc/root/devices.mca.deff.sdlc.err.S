# @(#)16	1.3  src/packages/devices/mca/deff/sdlc/root/devices.mca.deff.sdlc.err.S, pkgdevcommo, pkg411, GOLD410 2/17/94 13:13:58
#
#   COMPONENT_NAME: (MPAINST) MP/A SDLC DEVICE DRIVER INSTALLATION
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993,1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

*
*      Single Port Multiprotocol Adapter/A (MP/A) 
*      SDLC Device Driver Error Template
*

+ MPA_ADPERR:
        Comment         = " MP/A Adapter Error "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = EB66
        Prob_Causes     = EB02
        Inst_Causes     = EB32, EB31
        Inst_Actions    = EB63
        Fail_Causes     = EB41
        Fail_Actions    = EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_XFTO:
        Comment         = "Failsafe Transmit Timer"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERM
        Err_Desc        = EB67
        Prob_Causes     = EB02, EB01
        Fail_Causes     = EB41, EB40
        Fail_Actions    = EB61, EB62, EB66
        Detail_Data = 32, EB00, ALPHA

+ MPA_DSRON:
        Comment         = " DSR Already On for Switched Line "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = EB60
        Prob_Causes     = EB01, EB02 
        User_Causes     = EB20
        User_Actions    = EB60
        Inst_Causes     = EB30, EB31, EB32
        Inst_Actions    = EB60
        Fail_Causes     = EB40, EB41
        Fail_Actions    = EB62, EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_DSRDRP:
        Comment         = " DSR Dropped "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = EB68
        Prob_Causes     = EB01, EB02, EB04
        User_Causes     = EB20
        User_Actions    = EB60, EB63
        Inst_Causes     = EB30, EB31, EB32
        Inst_Actions    = EB60
        Fail_Causes     = EB40, EB41
        Fail_Actions    = EB62, EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_CTSON:
        Comment         = " CTS Already On at Modem "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = 3605
        Prob_Causes     = EB01, EB02 
        User_Causes     = EB20
        User_Actions    = EB60
        Inst_Causes     = EB30, EB31, EB32
        Inst_Actions    = EB60
        Fail_Causes     = EB40, EB41
        Fail_Actions    = EB62, EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_CTSDRP:
        Comment         = " CTS Dropped During Transmit "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = EB61
        Prob_Causes     = EB01, EB02, EB04
        User_Causes     = EB20
        User_Actions    = EB60, EB63
        Inst_Causes     = EB30, EB31, EB32
        Inst_Actions    = EB60
        Fail_Causes     = EB40, EB41
        Fail_Actions    = EB62, EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_RCVERR:
        Comment         = " MPA Error On Receive Data"
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = TEMP
        Err_Desc        = EB64
        Prob_Causes     = EB05, EB01
        Inst_Causes     = EB31, EB32, EB30
        Inst_Actions    = EB60, EB63
        Fail_Causes     = EB40, EB43
        Fail_Actions    = EB62, EB63
        Detail_Data = 32, EB00, ALPHA
        Detail_Data = 4, EB01, HEX

+ MPA_RCVOVR:
        Comment         = " Receive Overrun   "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = EB63
        Prob_Causes     = EB02
        Fail_Causes     = EB41, EB42
        Fail_Actions    = EB61
        Detail_Data = 32, EB00, ALPHA

+ MPA_BFR:
        Comment         = " Out of Resources "
        Class           = S
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = EB65
        Prob_Causes     = EB03
        Fail_Causes     = EB44
        Fail_Actions    = EB64, EB65
        Detail_Data = 32, EB00, ALPHA

+ MPA_XMTUND:
        Comment         = " Transmit Underrun "
        Class           = H
        Report          = TRUE
        Log             = TRUE
        Alert           = FALSE
        Err_Type        = PERF
        Err_Desc        = EB62
        Prob_Causes     = EB02
        Fail_Causes     = EB41, EB42
        Fail_Actions    = EB61
        Detail_Data = 32, EB00, ALPHA
