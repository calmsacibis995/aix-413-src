static char sccsid[] = "@(#)74  1.3  src/bos/diag/tu/fddi/tu023.c, tu_fddi, bos411, 9428A410j 11/4/93 11:07:49";
/*
 *   COMPONENT_NAME: TU_FDDI
 *
 *   FUNCTIONS: diag_test1
 *              tu023
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*****************************************************************************

Function(s) Test Unit 023 - Adapter Diagnostic Test 1 : ALISA Interface

Module Name :  tu023.c
SCCS ID     :  1.20

Current Date:  5/23/90, 11:17:55
Newest Delta:  3/26/90, 16:25:54

This test unit checks the control registers and shared RAM locations in
ALISA mapped to the Node Processor bits.

*****************************************************************************/
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/comio.h>
#include <sys/intr.h>
#include <sys/err_rec.h>
#include <errno.h>
#include "diagddfddiuser.h"
#include "fdditst.h"

#ifdef debugg
extern void detrace();
#endif
extern int rw_diag_tst();


/*****************************************************************************

daig_test1

*****************************************************************************/

int diag_test1(fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc;
        struct htx_data *htx_sp;

        /*
         * Set up a pointer to HTX data structure to increment
         * counters in case TU was invoked by hardware exerciser.
         */
        htx_sp = tucb_ptr->fddi_s.htx_sp;

#ifdef debugg
        detrace(0,"\n--- Test 1: ALISA Interface test begins ---\n");
#endif

        /*
         * Call Read/Write Diagnostic Test with appropiate test number.
         * The return code will indicate an error in writing the command to
         * the HCR, reading the results of the test run from Shared RAM or
         * pass/failure of the test.
         */

        rc = rw_diag_tst(fdes, 1, tucb_ptr);
        if (rc)
           {
#ifdef debugg
                detrace(0,"--- Test 1: ALISA Interface Test failed. ---\n");
#endif
                return(rc);
           }

#ifdef debugg
        detrace(0,"--- End of Test 1: ALISA Interface Test ---\n");
#endif
        return(0);
   }


/*****************************************************************************

tu023

*****************************************************************************/

int tu023 (fdes, tucb_ptr)
   int fdes;
   TUTYPE *tucb_ptr;
   {
        int             rc, result;

        /*
         * Call Adapter Diagnostic Test 1: ALISA Interface.
         */

        rc = diag_test1(fdes, tucb_ptr);

        return(rc);
   }
