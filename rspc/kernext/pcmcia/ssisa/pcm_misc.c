static char sccsid[] = "@(#)76	1.5  src/rspc/kernext/pcmcia/ssisa/pcm_misc.c, pcmciaker, rspc41J, 9520A_all 5/14/95 16:02:36";
/*
 * COMPONENT_NAME: PCMCIAKER
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "pcm_inc.h"

#ifdef DEBUG
extern int pcmcia_debug;
#endif

int pcmcia_init_registers( struct adapter_def *ap, int socket );
int pcmcia_store_registers( struct adapter_def *ap, int socket );
int pcmcia_restore_registers( struct adapter_def *ap, int socket );
int pcmcia_put_speed( SPEED speed, uchar * bits );
int pcmcia_get_speed( uchar byte, SPEED * speed );

short init_value[] = { 
    /* Value,    Register */
    -1,   /* Identification and Revision  (0x00) */
    -1,   /* Interface Status (0x01) */
    0xD5, /* Power and Resetdrv Control  (0x02) */
    0x80, /* Interrupt and General Control  (0x03) */ 
    -1,   /* Card Status Change  (0x04) */
    0x05, /* Card Status Chang Interrupt Configuration  (0x05) */
    0x20, /* Address Window Enable  (0x06) */
    0x00, /* I/O Control  (0x07) */
    0x00, /* I/O Address 0 Start Low  Byte  (0x08) */
    0x00, /* I/O Address 0 Start High Byte  (0x09) */
    0x00, /* I/O Address 0 Stop  Low  Byte  (0x0A) */
    0x00, /* I/O Address 0 Stop  High Byte  (0x0B) */
    0x00, /* I/O Address 1 Start Low  Byte  (0x0C) */
    0x00, /* I/O Address 1 Start High Byte  (0x0D) */
    0x00, /* I/O Address 1 Stop  Low  Byte  (0x0E) */
    0x00, /* I/O Address 1 Stop  High Byte  (0x0F) */
    0x00, /* System Memory Address 0 Mapping Start Low  Byte  (0x10) */
    0x00, /* System Memory Address 0 Mapping Start High Byte  (0x11) */
    0x00, /* System Memory Address 0 Mapping Start Low  Byte  (0x12) */
    0x00, /* System Memory Address 0 Mapping Start High Byte  (0x13) */
    0x00, /* Card Memory Offset Address 0 Low  Byte  (0x14) */
    0x00, /* Card Memory Offset Address 0 High Byte  (0x15) */
    0x01, /* Card Detect and General Control  (0x16) */
    -1,   /* Reserved (0x17) */
    0x00, /* System Memory Address 1 Mapping Start Low  Byte  (0x18) */
    0x00, /* System Memory Address 1 Mapping Start High Byte  (0x19) */
    0x00, /* System Memory Address 1 Mapping Start Low  Byte  (0x1A) */
    0x00, /* System Memory Address 1 Mapping Start High Byte  (0x1B) */
    0x00, /* Card Memory Offset Address 1 Low  Byte  (0x1C) */
    0x00, /* Card Memory Offset Address 1 High Byte  (0x1D) */
    0x00, /* Global Control  (0x1E) */
    -1,   /* Reserved (0x1F) */
    0x00, /* System Memory Address 2 Mapping Start Low  Byte  (0x20) */
    0x00, /* System Memory Address 2 Mapping Start High Byte  (0x21) */
    0x00, /* System Memory Address 2 Mapping Start Low  Byte  (0x22) */
    0x00, /* System Memory Address 2 Mapping Start High Byte  (0x23) */
    0x00, /* Card Memory Offset Address 2 Low  Byte  (0x24) */
    0x00, /* Card Memory Offset Address 2 High Byte  (0x25) */
    -1,   /* Reserved (0x26) */
    -1,   /* Reserved (0x27) */
    0x00, /* System Memory Address 3 Mapping Start Low  Byte  (0x28) */
    0x00, /* System Memory Address 3 Mapping Start High Byte  (0x29) */
    0x00, /* System Memory Address 3 Mapping Start Low  Byte  (0x2A) */
    0x00, /* System Memory Address 3 Mapping Start High Byte  (0x2B) */
    0x00, /* Card Memory Offset Address 3 Low  Byte  (0x2C) */
    0x00, /* Card Memory Offset Address 3 High Byte  (0x2D) */
    -1,   /* Reserved (0x2E) */
    -1,   /* Reserved (0x2F) */
    0x00, /* System Memory Address 3 Mapping Start Low  Byte  (0x30) */
    0x00, /* System Memory Address 3 Mapping Start High Byte  (0x31) */
    0x00, /* System Memory Address 3 Mapping Start Low  Byte  (0x32) */
    0x00, /* System Memory Address 3 Mapping Start High Byte  (0x33) */
    0x00, /* Card Memory Offset Address 3 Low  Byte  (0x34) */
    0x00, /* Card Memory Offset Address 3 High Byte  (0x35) */
    -1,   /* Reserved (0x36) */
    -1,   /* Reserved (0x37) */
    -1,   /* Reserved (0x38) */
    -1,   /* Reserved (0x39) */
    -1,   /* Reserved (0x3A) */
    -1,   /* Reserved (0x3B) */
    -1,   /* Reserved (0x3C) */
    -1,   /* Reserved (0x3D) */
    -1,   /* Reserved (0x3E) */
    -1    /* PC Card Access Lamp (0x3F) */ /*0x00*/
};

/****************************************************************************
 *
 * NAME: pcmcia_init_registers
 *
 * FUNCTION: Initialize PCIC register
 *
 * EXECUTION ENVIRONMENT: config thread
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *    -1        The speed is too slow or invalid format.
 *
 ***************************************************************************/
int pcmcia_init_registers( struct adapter_def *ap, int socket )
{
	int index, rc, ret_code = 0;
	struct pcmcia_ioreg ioreg;
	int index_max = sizeof init_value / sizeof init_value[0];

	for (index = 0; index < index_max; index++)
	{
		if ( init_value[index] == -1 ){
			continue;
		}
		ioreg.index = ioreg_index( index, socket );
		ioreg.data = init_value[index];
		if( index == 0x1E && ap->ddi.int_mode == 'I' ){
			/* Global ctl reg. interrupt level mode */
			ioreg.data |= 2;
		}
		rc = pcmcia_ioreg( ap, PCM_IOWRITE, & ioreg );
		if ( rc ){
			ret_code = -1;
		}
	}
	return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_store_registers
 *
 * FUNCTION: store PCIC register
 *
 * EXECUTION ENVIRONMENT: PM thread
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *    -1        The speed is too slow or invalid format.
 *
 ***************************************************************************/
int pcmcia_store_registers( struct adapter_def *ap, int socket )
{
	int index, rc, ret_code = 0;
	struct pcmcia_ioreg ioreg;
	int index_max = sizeof init_value / sizeof init_value[0];

	for (index = 0; index < index_max; index++)
	{
		if ( init_value[index] == -1 ){
		        continue;
		}
		ioreg.index = ioreg_index( index, socket );
		rc = pcmcia_ioreg( ap, PCM_IOREAD, &ioreg );
		ap->socket[socket].old_value[index] = ioreg.data;
		if ( rc ){
			ret_code = -1;
		}
	}
	return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_restore_registers
 *
 * FUNCTION: restore PCIC register
 *
 * EXECUTION ENVIRONMENT: PM thread
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *    -1        The speed is too slow or invalid format.
 *
 ***************************************************************************/
int pcmcia_restore_registers( struct adapter_def *ap, int socket )
{
	int index, rc, ret_code = 0;
	struct pcmcia_ioreg ioreg;
	int index_max = sizeof init_value / sizeof init_value[0];

	for (index = index_max-1; index >= 0; index--)
	{
		if ( init_value[index] == -1 ){
		        continue;
		}
		ioreg.index = ioreg_index( index, socket );
		ioreg.data = ap->socket[socket].old_value[index];
		rc = pcmcia_ioreg( ap, PCM_IOWRITE, & ioreg );
		if ( rc ){
			ret_code = -1;
		}
	}
	return ret_code;
}

/****************************************************************************
 *
 * NAME: pcmcia_put_speed
 *
 * FUNCTION: PCMCIA Socket Service misc function
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *    -1        The speed is too slow or invalid format.
 *
 ***************************************************************************/
int pcmcia_put_speed( SPEED speed, uchar * bits )
{
	uchar mantissa =  speed >> 3 & 0x0F ;
	uchar exponent =  speed & 0x07 ;
	uchar b;

	if ( 0 <= speed && speed <= 4 ){
		*bits = 0x00;
		return 0;
	}
	switch ( exponent )
	{
	case 0x00: /* 1 ns */
	case 0x01: /* 10 ns */
		*bits = 0x00;
		return 0;
	case 0x02:                          /* 100 ns */
		if ( mantissa <= 6 ){                      /* x 2.5 */
			*bits = 0x00;
			return 0;
		} else if ( mantissa <= 8 ){               /* x 3.5 */
			*bits = BIT_6;
			return 0;
		} else if ( mantissa <= 0x0B ){            /* x 5.0 */
			*bits = BIT_7;
			return 0;
		} else if ( mantissa <= 0x0E ){            /* x 7.0 */
			*bits = BIT_7 | BIT_6 ;
			return 0;
		}
	}
	/* no match */
	return -1;
}

/****************************************************************************
 *
 * NAME: pcmcia_get_speed
 *
 * FUNCTION: PCMCIA Socket Service misc function
 *
 * EXECUTION ENVIRONMENT:
 *     This routine can be called by a process.
 *
 * RETURNS:
 *     0        The operation is successfully completed.
 *
 ***************************************************************************/
int pcmcia_get_speed( uchar byte, SPEED * speed )
{
	switch( byte & (BIT_6 | BIT_7) )
	{
	case 0x00:
		*speed = 0x01;
		break;
	case BIT_6:
		*speed = 8 << 3 | 2 ;
		break;
	case BIT_7:
		*speed = 0x0B << 3 | 2;
		break;
	case BIT_6 | BIT_7:
		*speed = 0x0E << 3 | 2;
		break;
	}
	return 0;
}
