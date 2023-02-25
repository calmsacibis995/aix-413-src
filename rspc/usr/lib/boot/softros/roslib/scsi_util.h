/* @(#)54	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_util.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:01  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
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

/*--------------------------------------------------------------------------*/
/* The following section enables Development mode test code that should be  */
/* disabled for customer level product builds.                              */
/*--------------------------------------------------------------------------*/
#define  NOT_DEVELOPMENT_MODE    1
#ifdef   DEVELOPMENT_MODE
#define  SCSI_PARAMETER_CHECK_ENABLED  1
#endif
/*--------------------------------------------------------------------------*/
typedef unsigned int HND_SCSI;

#ifndef rtc_value
#include <misc_sub.h>
#endif

#define  RSC                  600
#define  SCSI_STRATEGY        RSC
#define  PCI_IO_ADDRESS_SPACE 0x80000000
#define  NVRAM_CFG_COUNT      0x00000814
#define  NVRAM_CFG_DATA       0x00000818
#define  SCSI_CFG_DATA        0x00000818
#define hf_led_port 0x808

extern unsigned short swap            (unsigned short x);
extern unsigned int swap_word         (unsigned int x);

extern void scsi_delay                (unsigned int duration);
extern rtc_value get_scsi_timeout     (unsigned int duration);
extern int check_scsi_timeout         (rtc_value timeout_value);
extern void controller_id             (ADAPTER_HANDLE handle,
                                       unsigned char *id_buffer,
                                       unsigned char  id_count);
extern unsigned int sys_mem_address   (ADAPTER_HANDLE handle);
extern unsigned char pci_cfg_read_8   (ADAPTER_HANDLE handle,
                                       unsigned int   addr);
extern unsigned short pci_cfg_read_16 (ADAPTER_HANDLE handle,
                                       unsigned int   addr);
extern unsigned int pci_cfg_read_32   (ADAPTER_HANDLE handle,
                                       unsigned int   addr);
extern void pci_cfg_write_8           (ADAPTER_HANDLE handle,
                                       unsigned int   addr,
                                       unsigned char  data);
extern void pci_cfg_write_16          (ADAPTER_HANDLE handle,
                                       unsigned int   addr,
                                       unsigned short data);
extern void pci_cfg_write_32          (ADAPTER_HANDLE handle,
                                       unsigned int   addr,
                                       unsigned int   data);
extern void wait_prompt               (void);

extern void hf_led_on                 (void);
extern void hf_led_off                (void);


typedef struct { ADAPTER_HANDLE handle;
                 unsigned char  id_count;
                 unsigned char  id_buffer[1]; // Actually defined as id_buffer[(id_count+1)/2]
               } CARD_NVRAM_IDS;

typedef struct { unsigned char  sig[4];
                 unsigned char  S_CARD_CNT;
                 CARD_NVRAM_IDS CARD_IDS[1];  // Actually defined as CARD_IDS[S_CARD_CNT]
               } NVRAM_SCSI_CFG;
