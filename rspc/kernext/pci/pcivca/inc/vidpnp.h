/* @(#)34  1.1 src/rspc/kernext/pci/pcivca/inc/vidpnp.h, pcivca, rspc41J, 9515B_all 4/13/95 03:51:09 */
/******************************************************************************
 * COMPONENT_NAME: (PCIVCA) PCI Video Capture Device Driver
 *
 * FUNCTIONS: vidpnp.h - general header file
 *
 * ORIGINS:     27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************************/

/* +--------------------------------------------------------------------------+
   Structure map for Video Channels in PnP Vendor specific packet             
 
   See Plug and Play ISA Specification, Version 1.0a, March 24, 1994.        
   It (or later versions) is available on Compuserve in the PLUGPLAY        
   area.  This code has extensions to that specification, namely new       
   short and long tag types for platform dependent information             
   Warning: LE notation used throughout this file                          
 
   If this packet is not available on a system with a G10 video capture    
   card installed, assume WOODFILED (ThinkPad Power Series 850 - M/T 6020 
   developer's kit).  If this packet is not available on a system without a 
   G10 video capture card, assume no video channels are available except for 
   +-----------------------------------------------------------------------+ */
#ifndef _VIDCHNPNP_
#define _VIDCHNPNP_
 
/* +--------------------------------------------------------------------------+
   Physical Video Channel ID definitions                                      
   Note that a physical channel may have more than one logical channel.  For 
   example, a video input channel can be used for NTSC and PAL.  In another 
   example, a video channel can be used for composite video and S-video.     
                                                                            
   At this moment, only Brooktree Bt812 and ASCII V7310A are the available   
   video chips.  Originator's intention is that the upper 4 bits represent   
   controller types and the lower 4 bits represent controller-dependent video 
   channel names.  But, for flexibility, the above guideline is not          
   registered at this revision.  It may be applied in the future.  Note that 
   Extension = 0xFF is reserved for future uses.                             
   +-----------------------------------------------------------------------+ */
 
#define	NTSCComposite	0x00            /* NTSC Composite Video  */
#define NTSCLume        0x01            /* NTSC S-Video Luma     */
#define NTSCChroma      0x02            /* NTSC S-Video Chroma   */
#define NTSCY           0x03            /* NTSC Component Y      */
#define NTSCYR          0x04            /* NTSC Component Y-R    */
#define NTSCYB          0x05            /* NTSC Component Y-B    */
#define NTSCTimingOnly  0x06            /* NTSC Timing Only      */
#define NTSCDiagnostics 0x07            /* NTSC Wrap-back        */
#define PALComposite    0x10            /* PAL Composite Video   */
#define PALLume         0x11            /* PAL S-Video Luma      */
#define PALChroma       0x12            /* PAL S-Video Chroma    */
#define PALY            0x13            /* PAL Component Y       */
#define PALYR           0x14            /* PAL Component Y-R     */
#define PALYB           0x15            /* PAL Component Y-B     */
#define PALTimingOnly   0x16            /* PAL Timing Only       */
#define PALDiagnostics  0x17            /* PAL Wrap-back         */
#define SECAMComposite  0x20            /* SECAM Composite Video */
#define SECAMLume       0x21            /* SECAM S-Video Luma    */
#define SECAMChroma     0x22            /* SECAM S-Video Chroma  */
#define SECAMY          0x23            /* SECAM Component Y     */
#define SECAMYR         0x24            /* SECAM Component Y-R   */
#define SECAMYB         0x25            /* SECAM Component Y-B   */
#define SECAMTimingOnly 0x26            /* SECAM Timing Only     */
#define SECAMDiagnostic 0x27            /* SECAM Wrap-back       */
#define VGARed          0x30            /* VGA Red               */
#define VGAGreen        0x31            /* VGA Green             */
#define VGABlue         0x32            /* VGA Blue              */
#define NoSignal        0xFF            /* No Signal             */
 
typedef struct _VidChanMap {           /* Video Channel Map                   */
   unsigned char  LogicalChannelID;    /* Video Channel = 0, 1, ... n         */
   unsigned char  VideoIO;             /* 0 : In, 1 : Out                     */
                                       /* Note that a single physical channel */
                                       /* may support both video-in and       */
                                       /* video-out future implementations.   */
                                       /* Under such circumstances, separate  */
                                       /* logical channel IDs must be         */
                                       /* assigned.                           */
   unsigned char  PhysicalChannelID;   /* Controller dependent port ID        */
   unsigned char  ConnectorType;       /* Mechanical Connector Type           */
   unsigned char  SignalType;          /* Electrical Signal Type              */
   } VidChanMap;
 
typedef struct _VidChanInfoPack {
   unsigned char  Tag;                 /* large tag = 0x84 Vendor specific    */
   unsigned char  Count0;              /* lo byte of count                    */
   unsigned char  Count1;              /* hi byte of count                    */
  /* count = number of logical video channels * sizeof(VidChanMap) + 1        */
   unsigned char  Type;                /* = B                                 */
   VidChanMap     VCMap[1];            /* Video channel map                   */
                                       /*                                     */
   } VidChanInfoPack;
 
#endif  /* ndef _VIDCHNPNP_ */
