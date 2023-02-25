/* @(#)20	1.2  src/rspc/usr/lib/boot/softros/include/boca_compat.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:20  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: scsi_read
 *		scsi_write
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** boca_compat.h -- Defines to map AIXMON function names to ROS **/

/*** Defines ******************************************************/

				/* Map scsi read/write to scsi_io */
#define scsi_read(b,d,l,c)  scsi_io(Read,b,d,l,c)
#define scsi_write(b,d,l,c) scsi_io(Write,b,d,l,c)



/*** Declare Variables ********************************************/

#ifdef IN_MAIN
int in_debug=0;		/* Declared in ROS in debsubs.c		  */
#endif
