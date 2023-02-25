static char sccsid[] = "@(#)87  1.1  src/rspc/kernext/disp/pcifga/config/Prog_ICD.c, pcifga, rspc411, GOLD410 4/15/94 18:37:28";
/* Prog_ICD.c */
/*
 *   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
 *
 *   FUNCTIONS: fga_open
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "fga_funct.h"
#include "fga_INCLUDES.h"
#include <sys/m_param.h>

/*----------------------------------------------------------------------------*/
/*               ICD2061A CLOCK INITIALIZATION ROUTINE                        */
/*----------------------------------------------------------------------------*/
void
Prog_ICD(ld, serial_data)
        struct fga_data *ld;
        unsigned long serial_data;
{
    int i;
    unsigned char start_val;

    if (serial_data == 0)
    {                               /* reset the control chip */
        *FGA_ADCNTL = 0x00;         /* default freq. for 640 x 480 */
        return;
    }

    /* Preserve the current value of FGA_ADCNTL */
    start_val = *FGA_ADCNTL & ~(FGA_ser_clock_on | FGA_ser_data_on);
    *FGA_ADCNTL  = start_val | FGA_ser_data_on;

    /* Unlock the ICD chip */
    for( i=0; i<5; i++)  /* do 5 times */
    {
        *FGA_ADCNTL = start_val |                    FGA_ser_data_on;
        *FGA_ADCNTL = start_val | FGA_ser_clock_on | FGA_ser_data_on;
    }

    /* Send Start Bit */
    *FGA_ADCNTL   = start_val;
    *FGA_ADCNTL   = start_val | FGA_ser_clock_on;
    *FGA_ADCNTL   = start_val;
    *FGA_ADCNTL   = start_val | FGA_ser_clock_on;

    /* Send Data Bits */
    for( i=0; i<24; i++) /*  do 24 times */
    {
        *FGA_ADCNTL = start_val | FGA_ser_clock_on | (~serial_data & FGA_ser_data_on);
        *FGA_ADCNTL = start_val |                    (~serial_data & FGA_ser_data_on);
        *FGA_ADCNTL = start_val |                    ( serial_data & FGA_ser_data_on);
        *FGA_ADCNTL = start_val | FGA_ser_clock_on | ( serial_data & FGA_ser_data_on);

        serial_data >>=1;
    }

    /* Send Stop Bit */
    *FGA_ADCNTL = start_val | FGA_ser_clock_on | FGA_ser_data_on;
    *FGA_ADCNTL = start_val |                    FGA_ser_data_on;
    *FGA_ADCNTL = start_val | FGA_ser_clock_on | FGA_ser_data_on;
    *FGA_ADCNTL = start_val | FGA_ser_clock_on | FGA_ser_data_on;
    *FGA_ADCNTL = start_val;

    return;  /* end of Prog_ICD () */
}

