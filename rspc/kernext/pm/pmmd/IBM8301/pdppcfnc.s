# @(#)23	1.2  src/rspc/kernext/pm/pmmd/IBM8301/pdppcfnc.s, pwrsysx, rspc41J, 9515B_all 4/14/95 10:55:44
#
#    COMPONENT_NAME: PWRSYSX
#
#    FUNCTIONS: Miscellaneous Assembly Routines
#
#    ORIGINS: 27
#
#    IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#    combined with the aggregated modules for this product)
#                     SOURCE MATERIALS
#    (C) COPYRIGHT International Business Machines Corp. 1995
#    All Rights Reserved
#
#    US Government Users Restricted Rights - Use, duplication or
#    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#  *************************************************************************
#  *                                                                       *
#  * Module Name: PDPPCFNC.S                                               *
#  * Description: Miscellaneous Assembly Routines    (for PowerPC 601/603) *
#  * Restriction:                                                          *
#  *                                                                       *
#  *************************************************************************

#  *************************************************************************
#  *                             Memo Section                              *
#  *************************************************************************

#  *************************************************************************
#  *                         Include Files Section                         *
#  *************************************************************************

#  *************************************************************************
#  *                   Data/Structure Definition Section                   *
#  *************************************************************************
#  *************************************
#  *      PowerPC Data Definition      *
#  *************************************
# -----------
#  Registers
# -----------
            .set     RTCU, 4
            .set     RTCL, 5
            .set     RTBU, 269
            .set     RTBL, 268
            .set     HID0, 1008

#  *************************************************************************
#  *                 Function Prototype Definition Section                 *
#  *************************************************************************

#  *************************************************************************
#  *                             Data Section                              *
#  *************************************************************************

#  *************************************************************************
#  *                             Code Section                              *
#  *************************************************************************
#  *************************************************
#  *   Read External Interrupt Enable Bit in MSR   *
#  *************************************************
#  *                                               *
#  * [ Function    ] UINT ReadMsrEe(VOID)          *
#  * [ Description ] Obtain the current  state  of *
#  *                 the external interrupt enable *
#  *                 (EE) bit in the machine state *
#  *                 register (MSR)                *
#  * [ Input       ] none                          *
#  * [ Output      ] R3 = Current state            *
#  *                      (1 - interrupt enable)   *
#  *                      (0 - interrupt disable)  *
#  *                                               *
#  *************************************************
            S_PROLOG(ReadMsrEe)

            mfmsr    r3                   #  Get MSR value
            rlwinm   r3, r3, 17, 31, 31   #  Rotate bit 16 to bit 31
                                          #  and isolate.

            S_EPILOG(ReadMsrEe)

#  **********************************************
#  *  Set External Interrupt Enable Bit in MSR  *
#  **********************************************
#  *                                            *
#  * [ Function    ] VOID SetMsrEe(VOID)        *
#  * [ Description ] Set the EE bit of MSR to 1 *
#  * [ Input       ] none                       *
#  * [ Output      ] none                       *
#  *                                            *
#  **********************************************
            S_PROLOG(SetMsrEe)

            mfmsr    r3                   #  Get MSR value
            ori      r3, r3, 0x8000       #  Set EE bit
            isync
            mtmsr    r3                   #  Update MSR
            isync

            S_EPILOG(SetMsrEe)

#  ************************************************
#  *  Reset External Interrupt Enable Bit in MSR  *
#  ************************************************
#  *                                              *
#  * [ Function    ] VOID ResetMsrEe(VOID)        *
#  * [ Description ] Clear the EE bit of MSR to 0 *
#  * [ Input       ] none                         *
#  * [ Output      ] none                         *
#  *                                              *
#  ************************************************
            S_PROLOG(ResetMsrEe)

            mfmsr    r3                   #  Get MSR value
            rlwinm   r3, r3, 17, 0, 30    #  Clear EE bit
            rlwinm   r3, r3, 15, 0, 31
            isync
            mtmsr    r3                   #  Update MSR
            isync

            S_EPILOG(ResetMsrEe)

#  *******************************************************
#  *       Read Power Management Enable Bit in MSR       *
#  *******************************************************
#  *                                                     *
#  * [ Function    ] UINT ReadMsrPow(VOID)               *
#  * [ Description ] Obtain the current  state  of  the  *
#  *                 power management enable (POW)  bit  *
#  *                 in the machine state register (MSR) *
#  * [ Input       ] none                                *
#  * [ Output      ] R3 = Current state                  *
#  *                      (1 - power management enable)  *
#  *                      (0 - power management disable) *
#  *                                                     *
#  *******************************************************
            S_PROLOG(ReadMsrPow)

            mfmsr    r3                   #  Get MSR value
            rlwinm   r3, r3, 14, 31, 31   #  Rotate bit 13 to bit 31
                                          #  and isolate.

            S_EPILOG(ReadMsrPow)

#  ***********************************************
#  *   Set Power Management Enable Bit in MSR    *
#  ***********************************************
#  *                                             *
#  * [ Function    ] VOID SetMsrPow(VOID)        *
#  * [ Description ] Set the POW bit of MSR to 1 *
#  * [ Input       ] none                        *
#  * [ Output      ] none                        *
#  *                                             *
#  ***********************************************
            S_PROLOG(SetMsrPow)

            mfmsr    r3                   #  Get MSR value
            oris     r3, r3, 0x0004       #  Set POW bit
            sync
            mtmsr    r3                   #  Update MSR
            isync

            S_EPILOG(SetMsrPow)

#  *************************************************
#  *   Reset Power Management Enable Bit in MSR    *
#  *************************************************
#  *                                               *
#  * [ Function    ] VOID ResetMsrPow(VOID)        *
#  * [ Description ] Clear the POW bit of MSR to 0 *
#  * [ Input       ] none                          *
#  * [ Output      ] none                          *
#  *                                               *
#  *************************************************
            S_PROLOG(ResetMsrPow)

            mfmsr    r3                   #  Get MSR value
            rlwinm   r3, r3, 14, 0, 30    #  Clear POW bit
            rlwinm   r3, r3, 18, 0, 31
            sync
            mtmsr    r3                   #  Update MSR
            isync

            S_EPILOG(ResetMsrPow)

#  *******************************************************
#  *         Read Power Saving Mode Bits in HID0         *
#  *******************************************************
#  *                                                     *
#  * [ Function    ] UINT ReadHid0Power(VOID)            *
#  * [ Description ] Obtain the  current  state  of  the *
#  *                 power saving mode  (DOZE/NAP/SLEEP) *
#  *                 bits in the hardware implementation *
#  *                 register 0 (HID0)                   *
#  * [ Input       ] none                                *
#  * [ Output      ] R3 = Current state                  *
#  *                      (Bit-29: doze bit )            *
#  *                      (Bit-30: nap bit  )            *
#  *                      (Bit-31: sleep bit)            *
#  *                                                     *
#  *******************************************************
            S_PROLOG(ReadHid0Power)

            mfspr    r3, HID0             #  Load R3 from HID0
            rlwinm   r3, r3, 11, 29, 31   #  Rotate bit 8:10 to bit 29:31
                                          #  and isolate.

            S_EPILOG(ReadHid0Power)

#  **************************************************
#  *       Set Power Saving Mode Bits in HID0       *
#  **************************************************
#  *                                                *
#  * [ Function    ] VOID SetHid0Power(UINT Mode)   *
#  * [ Description ] Set the power saving mode bits *
#  *                 of HID0 to mode value          *
#  * [ Input       ] R3 = power mode                *
#  *                      (Bit-29: doze bit )       *
#  *                      (Bit-30: nap bit  )       *
#  *                      (Bit-31: sleep bit)       *
#  * [ Output      ] none                           *
#  *                                                *
#  **************************************************
            S_PROLOG(SetHid0Power)

            mfspr    r4, HID0             #  Load R4 from HID0
            rlwimi   r4, r3, 21, 8, 10    #  Rotate bit 29:31 to bit 8:10
                                          #  and isolate.
            # isync
            mtspr    HID0, r4             #  Update HID0
            # isync

            S_EPILOG(SetHid0Power)

#  ***************************************************
#  *    Read Dynamic Power Management Bit in HID0    *
#  ***************************************************
#  *                                                 *
#  * [ Function    ] UINT ReadHid0Dpm(VOID)          *
#  * [ Description ] Obtain the current state of the *
#  *                 dynamic power management bit in *
#  *                 the   hardware   implementation *
#  *                 register 0 (HID0)               *
#  * [ Input       ] none                            *
#  * [ Output      ] R3 = Current state              *
#  *                      (1 - DPM enable)           *
#  *                      (0 - DPM disable)          *
#  *                                                 *
#  ***************************************************
            S_PROLOG(ReadHid0Dpm)

            mfspr    r3, HID0             #  Load R3 from HID0
            rlwinm   r3, r3, 12, 31, 31   #  Rotate bit 11 to bit 31
                                          #  and isolate.

            S_EPILOG(ReadHid0Dpm)

#  ****************************************************
#  *     Set Dynamic Power Management Bit in HID0     *
#  ****************************************************
#  *                                                  *
#  * [ Function    ] VOID SetHid0Dpm(VOID)            *
#  * [ Description ] Set the dynamic power management *
#  *                 bit of HID0 to 1                 *
#  * [ Input       ] none                             *
#  * [ Output      ] none                             *
#  *                                                  *
#  ****************************************************
            S_PROLOG(SetHid0Dpm)

            mfspr    r3, HID0             #  Load R3 from HID0
            oris     r3, r3, 0x0010       #  Set DPM bit
            isync
            mtspr    HID0, r3             #  Update HID0
            isync

            S_EPILOG(SetHid0Dpm)

#  ******************************************************
#  *     Reset Dynamic Power Management Bit in HID0     *
#  ******************************************************
#  *                                                    *
#  * [ Function    ] VOID ResetHid0Dpm(VOID)            *
#  * [ Description ] Clear the dynamic power management *
#  *                 bit of HID0 to 0                   *
#  * [ Input       ] none                               *
#  * [ Output      ] none                               *
#  *                                                    *
#  ******************************************************
            S_PROLOG(ResetHid0Dpm)

            mfspr    r3, HID0             #  Load R3 from HID0
            rlwinm   r3, r3, 12, 0, 30    #  Clear DPM bit
            rlwinm   r3, r3, 20, 0, 31
            isync
            mtspr    HID0, r3             #  Update HID0
            isync

            S_EPILOG(ResetHid0Dpm)

#  *************************************************
#  *      Read Real-Time Clock Upper Register      *
#  *************************************************
#  *                                               *
#  * [ Function    ] UINT ReadRtcu(VOID)           *
#  * [ Description ] Obtain the current content of *
#  *                 the  real-time  clock   upper *
#  *                 register (RTCU)               *
#  * [ Input       ] none                          *
#  * [ Output      ] R3 = Current content of RTCU  *
#  *                                               *
#  *************************************************
            S_PROLOG(ReadRtcu)

            mfpvr    r3                   #  Get processor version
            rlwinm   r3, r3, 16, 16, 31
            cmpi     cr0, r3, 1
            beq      ReadRtcu601

            mfspr    r3, RTBU             #  Load R3 from RTBU (for 603)
            blr

ReadRtcu601:
            mfspr    r3, RTCU             #  Load R3 from RTCU (for 601)

            S_EPILOG(ReadRtcu)

#  *************************************************
#  *      Read Real-Time Clock Lower Register      *
#  *************************************************
#  *                                               *
#  * [ Function    ] UINT ReadRtcl(VOID)           *
#  * [ Description ] Obtain the current content of *
#  *                 the  real-time  clock   lower *
#  *                 register (RTCL)               *
#  * [ Input       ] none                          *
#  * [ Output      ] R3 = Current content of RTCL  *
#  *                                               *
#  *************************************************
            S_PROLOG(ReadRtcl)

            mfpvr    r3                   #  Get processor version
            rlwinm   r3, r3, 16, 16, 31
            cmpi     cr0, r3, 1
            beq      ReadRtcl601

            mfspr    r3, RTBL             #  Load R3 from RTBL (for 603)
            blr

ReadRtcl601:
            mfspr    r3, RTCL             #  Load R3 from RTCL (for 601)

            S_EPILOG(ReadRtcl)

#  ************************************************
#  *            Read Processor Version            *
#  ************************************************
#  *                                              *
#  * [ Function    ] UINT ReadCpuVersion(VOID)    *
#  * [ Description ] Obtain the processor version *
#  *                 of  the  processor   version *
#  *                 register (PVR)               *
#  * [ Input       ] none                         *
#  * [ Output      ] R3 = version                 *
#  *                                              *
#  ************************************************
            S_PROLOG(ReadCpuVersion)

            mfpvr    r3                   #  Get processor version
            rlwinm   r3, r3, 16, 16, 31

            S_EPILOG(ReadCpuVersion)

#  **********************************************
#  *      Read from I/O Port (Byte order)       *
#  **********************************************
#  *                                            *
#  * [ Function ] BYTE inp(UINT Port)           *
#  * [ Input    ] R3 = I/O port address         *
#  * [ Output   ] R3 = value read from I/O port *
#  *                                            *
#  **********************************************
            S_PROLOG(inp)

            eieio
            lbz      r3, 0(r3)

            S_EPILOG(inp)

#  *************************************************
#  *        Write to I/O Port (Byte order)         *
#  *************************************************
#  *                                               *
#  * [ Function ] VOID outp(UINT Port, BYTE Value) *
#  * [ Input    ] R3 = I/O port address            *
#  *              R4 = value to write to I/O port  *
#  * [ Output   ] none                             *
#  *                                               *
#  *************************************************
            S_PROLOG(outp)

            eieio
            stb      r4, 0(r3)

            S_EPILOG(outp)

#  **********************************************
#  *      Read from I/O Port (Word order)       *
#  **********************************************
#  *                                            *
#  * [ Function ] WORD inpw(UINT Port)          *
#  * [ Input    ] R3 = I/O port address         *
#  * [ Output   ] R3 = value read from I/O port *
#  *                                            *
#  **********************************************
            S_PROLOG(inpw)

            eieio
            lhz      r3, 0(r3)

            S_EPILOG(inpw)

#  **************************************************
#  *         Write to I/O Port (Word order)         *
#  **************************************************
#  *                                                *
#  * [ Function ] VOID outpw(UINT Port, WORD Value) *
#  * [ Input    ] R3 = I/O port address             *
#  *              R4 = value to write to I/O port   *
#  * [ Output   ] none                              *
#  *                                                *
#  **************************************************
            S_PROLOG(outpw)

            eieio
            sth      r4, 0(r3)

            S_EPILOG(outpw)

#  **********************************************
#  *      Read from I/O Port (DWord order)      *
#  **********************************************
#  *                                            *
#  * [ Function ] DWORD inpdw(UINT Port)        *
#  * [ Input    ] R3 = I/O port address         *
#  * [ Output   ] R3 = value read from I/O port *
#  *                                            *
#  **********************************************
            S_PROLOG(inpdw)

            eieio
            lwz      r3, 0(r3)

            S_EPILOG(inpdw)

#  ****************************************************
#  *         Write to I/O Port (DWord order)          *
#  ****************************************************
#  *                                                  *
#  * [ Function ] VOID outpdw(UINT Port, DWORD Value) *
#  * [ Input    ] R3 = I/O port address               *
#  *              R4 = value to write to I/O port     *
#  * [ Output   ] none                                *
#  *                                                  *
#  ****************************************************
            S_PROLOG(outpdw)

            eieio
            stw      r4, 0(r3)

            S_EPILOG(outpdw)

#  ***********************************
#  *           Synchronize           *
#  ***********************************
#  *                                 *
#  * [ Function    ] VOID Sync(VOID) *
#  * [ Description ] Synchronize     *
#  * [ Input       ] none            *
#  * [ Output      ] none            *
#  *                                 *
#  ***********************************
            S_PROLOG(Sync)

            sync
            sync
            sync
            isync

            S_EPILOG(Sync)

#  *********************************************************
#  This function was added for the defect fix. 
#  The defect # is 165079.
#  *********************************************************
#  *                    unlock_enable()                    *
#  *********************************************************
#  *                                                       *
#  * [ Function    ] unlock_enable()                       *
#  * [ Description ] temporary replacement for  the  real  *
#  *                 unlock_enable()  enable   interrupts  *
#  *                 caller must be in supervisor (kernel) *
#  *                 state.                                *
#  *                 Input parameters of priority and lock *
#  *                 word are ignored                      *
#  *                                                       *
#  *********************************************************
            S_PROLOG(unlock_enable)

            mfmsr    r3                   # Get current msr
            ori      r3, r3, 0x8000       # Set EE bit
            mtmsr    r3                   # Set machine state

            S_EPILOG(unlock_enable)

#  *********************************************************
#  This function was added for the defect fix. 
#  The defect # is 165079.
#  *********************************************************
#  *                    disable_lock()                     *
#  *********************************************************
#  *                                                       *
#  * [ Function    ] disable_lock()                        *
#  * [ Description ] temporary replacement for  the  real  *
#  *                 disable_lock()  disable   interrupts  *
#  *                 caller must be in supervisor (kernel) *
#  *                 state.                                *
#  *                 Input parameters of priority and lock *
#  *                 word are ignored                      *
#  *                                                       *
#  *********************************************************
            S_PROLOG(disable_lock)

            mfmsr    r3                   # Get current msr
            CLRBIT(r4, r3, 16)            # Clear EE bit
            mtmsr    r4                   # Set machine state

            S_EPILOG(disable_lock)


#  *****************************  End of File  *****************************
