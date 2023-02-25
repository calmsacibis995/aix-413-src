/* @(#)52	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_text.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:58  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: message
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

//----------------------------------------------------------------------------------//
// This file contains all the translatable text messages used by the SCSI subsystem //
//----------------------------------------------------------------------------------//

// General rule - DO NOT translate any formatting values such as "\n", "\r", "%d", etc.
//                the formatting values are changed at display time as follows
//                   \n, \r - not displayable cursor control characters
//                       %d - Decimal number
//                       %x - Lower case hexadecimal number
//                       %X - Upper case hexadecimal number
//                       %c - Single printable character
//                       %s - An entire string

#ifdef SCSI_UTIL
 unsigned char *SCSI_sig = "SCSI";  // Do NOT translate.  This text string used to locate
                                    // information in NVRAM as well as for display
#else
extern unsigned char *SCSI_sig;
#endif

#ifdef SCSI_API
//
// The following messages are displayed during the scan for devices attached to the
// SCSI controller during system initialization
//
// The screen message is constructed based on the test parameters at the time the
// message is displayed.  It may have any of the following forms
//
// Checking ID xx
// Checking ID xx LUN xx
// Checking Bus xx ID xx
// Checking Bus xx ID xx LUN xx
//

static unsigned char *check_msg = "Checking ";  // Leave a blank after the word
static unsigned char *bus_msg   = "Bus %d, ";   // Leave a blank at the end of message
static unsigned char *id_msg    = "ID %d";      // Do not leave a blank at end
static unsigned char *lun_msg   = ", LUN %d";   // Do not leave a blank at end

//
// The following messages tell the operator what type of device was located during
// system initialization
//
// The displayed line is built using 4 different pieces of information
//    - SCSI_sig message
//    - Device type message (see below)
//    - Device address (built using bus_msg, id_msg, and lun_msg from above as required)
//    - Text obtained from device itself that identifies the device's
//       vendor name and model information (for example, a model number)
//
//  The following shows an example display line
//
//  SCSI R/W Optical (Bus 1, ID 5, LUN 2) : IBM      MD3125A PML00038
//
// Note : When more than one SCSI 'adapter' is supported, another identifier
//        (maybe 'Card') will have to be added as shown in the following line
//
//  SCSI R/W Optical (Card 1, Bus 1, ID 5, LUN 2) : IBM      MD3125A PML00038
//

// Device type messages :
  // For new message structure testing ....
  unsigned char *type_msg[] = { "Disk",            // Device type 0, non-removable media
                                "DASD",            // Device type 0, removable media
                                "Tape",            // Device type 1
                                "Printer",         // Device type 2
                                "Processor",       // Device type 3
                                "WORM",            // Device type 4
                                "CD-ROM",          // Device type 5
                                "Scanner",         // Device type 6
                                "R/W Optical",     // Device type 7
                                "Changer",         // Device type 8
                                "Communications",  // Device type 9
                              };

 unsigned char *unknown_msg = "Unknown device type";
 unsigned char *vendor_msg  = "Vendor specific type";

//
// When the device cannot return its device specific information
// (for example the 'IBM      MD3125A PML00038' in the above example)
// the following is displayed instead
//

 unsigned char *no_vpd_msg  =  "(product information not available)\n";

// Possible change to fit worst case message on screen
// SCSI Vendor unique type (Bus 1, ID 5, LUN 2) : (device data not available)
// SCSI Vendor unique type (Card 1, Bus 1, ID 5, LUN 2) : (device data not available)

#endif
