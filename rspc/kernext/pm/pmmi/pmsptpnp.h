/* @(#)02   1.2  src/rspc/kernext/pm/pmmi/pmsptpnp.h, pwrsysx, rspc41J, 9518A_all 5/2/95 10:19:00 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file of for Power management PnP packet of residual data 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* As of 3/03/95 */
/* Structure map for Power Management PnP Vendor specific packet */
 
/* See Plug and Play ISA Specification, Version 1.0a, March 24, 1994.
   It (or later versions) is available on Compuserve in the PLUGPLAY
   area.  This code has extensions to that specification, namely new
   short and long tag types for platform dependent information */
/* Warning: LE notation used throughout this file */
 
#ifndef _PMSPTPNP_
#define _PMSPTPNP_
 
#define  PMsupport_Packet  0x84         /* tag for PMsupportPack            */
 
typedef enum _PM_attribute {
   hibernation_support = 0x0001,
   suspend_support = 0x0002,
   LID_support = 0x0004,
   battery_support = 0x0008,
   ringing_resume_from_hibernation = 0x0010,
   ringing_resume_from_suspend = 0x0020,
   resume_from_hibernation_at_specified_time = 0x0040,
   resume_from_suspend_at_specified_time = 0x0080,
   hibernation_from_suspend_at_specified_time = 0x0100,
   software_controllable_main_power_switch = 0x0200,
   general_purpose_input_pin_for_resume_trigger = 0x0400,
   fail_safe_hardware_main_power_off_switch = 0x0800,
   resume_button_support = 0x1000,
   inhibit_auto_transition = 0x2000
   } PM_attribute;
 
typedef struct _PMsupportPack {
   unsigned char  Tag;                  /* large tag = 0x84 Vendor specific */
   unsigned char  Count0;               /* = 0x05 sizeof(PMsupprtPack)-3    */
   unsigned char  Count1;               /* = 0                              */
   unsigned char  Type;                 /* = 8 (PM Controller)              */
   unsigned long  PMattribute;
   } PMsupportPack;
 
#endif  /* ndef _PMSPTPNP_ */
