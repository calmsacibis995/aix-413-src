/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Planar Device Control External Interface
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/* INCLUDES */
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>

#include "pm_6030.h"
#include "pdplanar.h"
#include "pdcommon.h"
#include "pdsystem.h"


/* FUNCTION PROTOTYPE DEFINITIONS */
int    planardevice_initialize(void);
int    planardevice_save_context(void);
int    planardevice_restore_context(void);
int    pm_register_restart_information(void * restart_address);
int    pm_clear_restart_information(void);
void * pm_map_iospace(void);
void   pm_unmap_iospace(void);
int    md_restart_block_read(MACH_DD_IO * md);
int    md_restart_block_upd(MACH_DD_IO * md, unsigned char shutdown_mode);


/******************************************************************************
 * NAME:
 *    planardevice_control
 *
 * FUNCTION:
 *    Dispatch the planar device controls
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    planar_job_id - ID number of the planar device control
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
int
planardevice_control(
   int   planar_job_id
   )
{
   int   rc;

   switch(planar_job_id)
   {
      case Initialize_planar_device:
         rc = planardevice_initialize();
         break;

      case Save_planar_device:
         rc = planardevice_save_context();
         break;

      case Restore_planar_device:
         rc = planardevice_restore_context();
         break;

      default:
         rc = -1;
   } /* endswitch */

   return rc;
}

/******************************************************************************
 * NAME:
 *    planardevice_initialize
 *
 * FUNCTION:
 *    Initialize planar devices control
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
int
planardevice_initialize(
   void
   )
{
   pm_map_iospace();

   PmSystemInitialize();

   pm_unmap_iospace();

   return 0;
}

/******************************************************************************
 * NAME:
 *    planardevice_save_context
 *
 * FUNCTION:
 *    Save context for all planar devices
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
int
planardevice_save_context(
   void
   )
{
   pm_map_iospace();

   PmSystemSuspend(NULL);

   pm_unmap_iospace();

   return 0;
}

/******************************************************************************
 * NAME:
 *    planardevice_restore_context
 *
 * FUNCTION:
 *    Restore context for all planar devices
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
int
planardevice_restore_context(
   void
   )
{
   pm_map_iospace();

   PmSystemResume();

   pm_unmap_iospace();

   return 0;
}

/******************************************************************************
 * NAME:
 *    pm_register_restart_information
 *
 * FUNCTION:
 *    Register information to restart from appropriate system state
 *    (suspend/hibernate/etc.). That information must be stored into
 *    the non-valatile area before completing any shutdown.
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    restart_address - real (physical) address value to restart the system
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
typedef struct
{
   unsigned short          Version;
   unsigned short          Revision;
   unsigned long           BootMasterId;
   unsigned long           ProcessorId;
   volatile unsigned long  BootStatus;
   unsigned long           CheckSum;
   void                    *RestartAddress;
   void                    *SaveAreaAddr;
   unsigned long           SaveAreaLength;
} RESTART_BLOCK;

typedef enum
{
   Suspend   = 0x80,                           /* part of state is in memory */
   Hibernate = 0                                        /* Nothing in memory */
} PMMode;

int
pm_register_restart_information(
   void *   restart_address
   )
{
   MACH_DD_IO     mdd;
   RESTART_BLOCK  rblk;

   /* Set resume entry point          */
   /*   & set shutdown mode to NV-RAM */
   /*                                 */
   mdd.md_data = (char *)&rblk;
   mdd.md_size = sizeof(rblk);
   md_restart_block_read(&mdd);

   rblk.RestartAddress = restart_address;
   md_restart_block_upd(&mdd, (unsigned char)Suspend);

   return 0;
}

/******************************************************************************
 * NAME:
 *    pm_clear_restart_information
 *
 * FUNCTION:
 *    Clear the restart information on the non-valatile area
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    zero    - success
 *    no zero - error
 */
int
pm_clear_restart_information(
   void
   )
{
   MACH_DD_IO     mdd;
   RESTART_BLOCK  rblk;

   /* Reset resume entry point          */
   /*   & clear shutdown mode to NV-RAM */
   /*                                   */
   mdd.md_data = (char *)&rblk;
   mdd.md_size = sizeof(rblk);
   md_restart_block_read(&mdd);

   rblk.RestartAddress = NULL;
   md_restart_block_upd(&mdd, (unsigned char)Hibernate);

   return 0;
}

/******************************************************************************
 * NAME:
 *    pm_get_memorymapiospace
 *
 * FUNCTION:
 *    Get I/O address on a memory mapped space
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    address - real (physical) address value to access the device I/O
 *    length  - length of the access area
 *
 * RETURN VALUE:
 *    null    - error
 *    no null - address
 */
struct
{
   unsigned long  address;
   unsigned long  length;
   void *         map_address;
}     pm_iomap_list[10];
int   pm_iomap_ptr = 0;

void *
pm_get_memorymapiospace(
   unsigned long  address,
   unsigned long  length
   )
{
   void *         addr;
   struct io_map  iomap;
   int            i;

   for(i = 0, addr = NULL; (i < pm_iomap_ptr) && (addr == NULL); i++)
   {
      if((pm_iomap_list + i)->address == address)
      {
         addr = (pm_iomap_list + i)->map_address;
      } /* endif */
   } /* endfor */

   if(addr == NULL)
   {
      iomap.key     = IO_MEM_MAP;
      iomap.flags   = 0;
      iomap.size    = SEGSIZE;
      iomap.bid     = BID_VAL(IO_PCI, PCI_IOMEM, BID_NUM(0));
      iomap.busaddr = (long long)address;

      addr = iomem_att(&iomap);

      (pm_iomap_list + pm_iomap_ptr)->address     = address;
      (pm_iomap_list + pm_iomap_ptr)->length      = length;
      (pm_iomap_list + pm_iomap_ptr)->map_address = addr;

      pm_iomap_ptr++;
   } /* endif */

   return(addr);
}

/******************************************************************************
 * NAME:
 *    pm_free_memorymapiospace
 *
 * FUNCTION:
 *    Free I/O address on a memory mapped space
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    none
 */
void
pm_free_memorymapiospace(
   void
   )
{
   for(; pm_iomap_ptr; )
   {
      iomem_det((pm_iomap_list + --pm_iomap_ptr)->map_address);
   } /* endfor */

   return;
}

/******************************************************************************
 * NAME:
 *    pm_map_iospace
 *
 * FUNCTION:
 *    Map I/O address on a memory mapped space
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    null    - error
 *    no null - address
 */
void *   io_control_base = NULL;
void *   io_memory_base  = NULL;
void *   pci_config_base = (void *)0x80000000;

void *
pm_map_iospace(
   void
   )
{
   struct io_map  iomap;

   iomap.key     = IO_MEM_MAP;
   iomap.flags   = 0;
   iomap.size    = SEGSIZE;
   iomap.bid     = BID_VAL(IO_ISA, ISA_IOMEM, BID_NUM(0));
   iomap.busaddr = 0;

   return(io_control_base = iomem_att(&iomap));
}

/******************************************************************************
 * NAME:
 *    pm_unmap_iospace
 *
 * FUNCTION:
 *    Unmap I/O address on a memory mapped space
 *
 * EXECUTION ENVIRONMENT:
 *    This routine can be called from either the process or interrupt
 *    environment.
 *
 * NOTES:
 *
 * INPUT:
 *    none
 *
 * RETURN VALUE:
 *    none
 */
void
pm_unmap_iospace(
   void
   )
{
   iomem_det(io_control_base);
   io_control_base = NULL;

   pm_free_memorymapiospace();

   return;
}
