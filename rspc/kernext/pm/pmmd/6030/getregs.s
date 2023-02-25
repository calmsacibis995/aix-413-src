# @(#)55        1.1  src/rspc/kernext/pm/pmmd/6030/getregs.s, pwrsysx, rspc41J, 9517A_all 4/25/95 16:33:56
#
#   COMPONENT_NAME: PWRSYSX
#
#   FUNCTIONS:  getregs.s
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

	.file	"getregs.s"

#  Abstract:
#
#     This module contains;
#    	CPU register read routine to dump the CPU context for debug purpose 
#


        .extern pIoControlBase


        .set    tbl, 284
        .set    tbu, 285

	S_PROLOG(KiGetPcr)  

        mfsprg  r3, 1

        blr  
	S_EPILOG(KiGetPcr)

	S_PROLOG(KiGetXer)  

        mfxer   r3
        blr  

        S_EPILOG(KiGetXer)


	S_PROLOG(KiGetDsisr)  

        mfdsisr r0
        stw     r0, 0(r3)
        blr  

        S_EPILOG(KiGetDsisr)


	S_PROLOG(KiGetDar)  

        mfdar   r3
        blr  

        S_EPILOG(KiGetDar)


	S_PROLOG(KiGetDec)  

        mfdec   r3
        blr  

        S_EPILOG(KiGetDec)


	S_PROLOG(KiGetSdr1)  

        mfsdr1  r3
        blr  
        S_EPILOG(KiGetSdr1)



	S_PROLOG(KiGetSrr0)  

        mfsrr0  r3
        blr  
        S_EPILOG(KiGetSrr0)


	S_PROLOG(KiGetSrr1)  

        mfsrr1  r3
        blr  

        S_EPILOG(KiGetSrr1)


	S_PROLOG(KiGetSprg)  

        cmpwi   cr0, r3, 1
        cmpwi   cr1, r3, 2
        cmpwi   cr2, r3, 3
        beq     cr0, KiGetSprg1
        beq     cr1, KiGetSprg2
        beq     cr2, KiGetSprg3

KiGetSprg0:
        mfsprg  r3, 0
        blr

KiGetSprg1:
        mfsprg  r3, 1
        blr

KiGetSprg2:
        mfsprg  r3, 2
        blr

KiGetSprg3:
        mfsprg  r3, 3
        blr  
        S_EPILOG(KiGetSprg)


	S_PROLOG(KiGetEar)  

        mfear   r3
        blr  
        S_EPILOG(KiGetEar)


	S_PROLOG(KiGetTbu)  

waitLoopTB:
        mfspr   r3,tbu
        mfspr   r4,tbl

        mfspr   r4,tbl
        mfspr   r4,tbl
        mfspr   r4,tbl

        mfspr   r5,tbu
        cmpw    r5, r3
        bne     waitLoopTB

        blr  
        S_EPILOG(KiGetTbu)


	S_PROLOG(KiGetTbl)  

        mfspr   r3,tbl
        blr  
        S_EPILOG(KiGetTbl)


	S_PROLOG(KiGetPvr)  

        mfpvr   r3
        blr  
        S_EPILOG(KiGetPvr)


	S_PROLOG(KiGetIbat0)  
        mfibatu r4, 0
        mfibatl r5, 0
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetIbat0)

	S_PROLOG(KiGetIbat1)  
        mfibatu r4, 1
        mfibatl r5, 1
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetIbat1)

	S_PROLOG(KiGetIbat2)  
        mfibatu r4, 2
        mfibatl r5, 2
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetIbat2)

	S_PROLOG(KiGetIbat3)  
        mfibatu r4, 3
        mfibatl r5, 3
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr  
        S_EPILOG(KiGetIbat3)



	S_PROLOG(KiGetDbat0)  
        mfdbatu r4, 0
        mfdbatl r5, 0
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetDbat0)

	S_PROLOG(KiGetDbat1)  
        mfdbatu r4, 1
        mfdbatl r5, 1
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetDbat1)

	S_PROLOG(KiGetDbat2)  
        mfdbatu r4, 2
        mfdbatl r5, 2
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetDbat2)

	S_PROLOG(KiGetDbat3)  
        mfdbatu r4, 3
        mfdbatl r5, 3
        stw     r4, 4(r3)
        stw     r5, 0(r3)
        blr
        S_EPILOG(KiGetDbat3)



	S_PROLOG(KiGetSr)  

        slwi    r3, r3, 28
        mfsrin  r3, r3

        blr  
        S_EPILOG(KiGetSr)


	S_PROLOG(KiGetMsr)  

        mfmsr   r3
        blr  

        S_EPILOG(KiGetMsr)

	S_PROLOG(KiSetMsr)  

        mtmsr   r3
        blr  
        S_EPILOG(KiSetMsr)

	S_PROLOG(KiGetRsp)  

        mr      r3, r1
        blr  

        S_EPILOG(KiGetRsp)

	S_PROLOG(KiGetRtoc)  

        mr      r3, r2
        blr  

        S_EPILOG(KiGetRtoc)

	S_PROLOG(KiGetHID)  

        cmpwi   cr0, r3, 1
        cmpwi   cr1, r3, 2
        cmpwi   cr2, r3, 5
        beq     cr0, KiGetHID1
        beq     cr1, KiGetHID2
        beq     cr2, KiGetHID5

KiGetHID0:
        mfspr   r3, 0x3f0
        blr

KiGetHID1:
        mfspr   r3, 0x3f1
        blr

KiGetHID2:
        mfspr   r3, 0x3f2
        blr

KiGetHID5:
        mfspr   r3, 0x3f5
        blr  
        S_EPILOG(KiGetHID)

	S_PROLOG(KiSetHID)  

        cmpwi   cr0, r3, 1
        cmpwi   cr1, r3, 2
        cmpwi   cr2, r3, 5
        beq     cr0, KiSetHID1
        beq     cr1, KiSetHID2
        beq     cr2, KiSetHID5

KiSetHID0:
        mtspr   0x3f0, r4
        blr

KiSetHID1:
        mtspr   0x3f1, r4
        blr

KiSetHID2:
        mtspr   0x3f2, r4
        blr

KiSetHID5:
        mtspr   0x3f5, r4
        blr  
        S_EPILOG(KiSetHID)

	S_PROLOG(KiGetRPA)  

        mfspr   r3, 982
        blr  
        S_EPILOG(KiGetRPA)

	S_PROLOG(KiSetRPA)  

        mtspr   982, r3

        blr  
        S_EPILOG(KiSetRPA)

	S_PROLOG(KiGetICMP)  

        mfspr   r3, 981

        blr  
        S_EPILOG(KiGetICMP)

	S_PROLOG(KiSetICMP)  

        mtspr   981, r3
        blr  
        S_EPILOG(KiSetICMP)

	S_PROLOG(KiGetDCMP)  

        mfspr   r3, 977
        blr  
        S_EPILOG(KiGetDCMP)

	S_PROLOG(KiSetDCMP)  

        mtspr   977, r3
        blr  
        S_EPILOG(KiSetDCMP)

	S_PROLOG(KiGetIMISS)  

        mfspr   r3, 980
        blr  
        S_EPILOG(KiGetIMISS)


	S_PROLOG(KiGetDMISS)  

        mfspr   r3, 976
        blr  

        S_EPILOG(KiGetDMISS)

