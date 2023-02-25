static char sccsid[] = "@(#)35	1.2  src/rspc/usr/lib/boot/softros/roslib/scsi_util.c, rspc_softros, rspc411, 9436E411a 9/8/94 15:19:19";
#ifdef DEBUG

#ifdef LISTON
 #pragma options source flag=I:I
#endif

/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS: see scsi_util.h
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * note for rs2  old            new
 *           dma_init           init_ext_int
 *           enable             enable_ext_int
 *           clear_int_mask     disable_ext_int
 *           clear_int_status   reset_ext_int
 */

#define SCSI_UTIL   // identify parent file to included files
#pragma options nosource
//--------------------------------------------------------------------------//
// External global definitions                                              //
//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//
// Declarations of external functions & reltd #defines used by this file    //
//--------------------------------------------------------------------------//

#include <sys/types.h>
#include <misc_asm.h>
#include <io_sub.h>
#include <misc_sub.h>
#include <scsi_public.h>
#include <scsi_externals.h>
#include <model.h>

//--------------------------------------------------------------------------//
// External declarations of functions defined in this file & reltd #defines //
//--------------------------------------------------------------------------//

#include <scsi_drvr.h>
#include <scsi_text.h>  // Translatable text file
#include <scsi_util.h>

//--------------------------------------------------------------------------//
// Local definitions of functions used only by functions in this file       //
//--------------------------------------------------------------------------//

unsigned int pci_cfg_addr(ADAPTER_HANDLE handle,
                          unsigned int   addr);

//--------------------------------------------------------------------------//
// Definitions used only within this file                                   //
//--------------------------------------------------------------------------//

#ifdef LISTON
 #pragma options source list
#endif

//--------------------------------------------------------------------------//
// Function: swap()               exchange 2 bytes                          //
//--------------------------------------------------------------------------//

ushort swap(ushort x)
{
  return ((x<<8)+(x>>8));
}


//--------------------------------------------------------------------------//
// Function: swap_word()          exchange 4 bytes                          //
//--------------------------------------------------------------------------//

uint swap_word(uint x)
{
  return ((swap((unsigned short int)x)<<16)+(swap((unsigned short int)(x>>16
     ))));
}


//--------------------------------------------------------------------------//
// Function: scsi_delay()                                                   //
// Action:   Interface routine to a common method of delay. (microseconds)  //
//--------------------------------------------------------------------------//

void scsi_delay(unsigned int duration)
{
rtc_value delay;

  delay = get_timeout_value(duration,MICROSECONDS);
  while ( FALSE == timeout(delay) ) { };
  return ;
}

//--------------------------------------------------------------------------//
// Function: get_scsi_timeout()                                             //
// Action:   Get the value from the system's time base that can be passed   //
//           to the scsi_timeout function to determine if the requested     //
//           amount of time has elapsed or not.                             //
//  Input : duration in millisecond increments                              //
//--------------------------------------------------------------------------//

rtc_value get_scsi_timeout(unsigned int duration)
{
  return (get_timeout_value(duration,MILLISECONDS));
}


//--------------------------------------------------------------------------//
// Function: check_scsi_timeout()                                           //
// Action:   Check to see if the requested time has elapsed or not yet.     //
//           The value passed to this routine must be one obtained from     //
//           the get_scsi_timeout function to work properly.                //
//  Input : rtc_value argument returned by the get_scsi_timeout function    //
// Output : returns the value 0 if time has not expired                     //
//                  non-zero if time has expired                            //
//--------------------------------------------------------------------------//

int check_scsi_timeout(rtc_value timeout_value)
{
  return ( (FALSE == timeout(timeout_value)) ? 0 : 1 );
}

//--------------------------------------------------------------------------//
// Function : wait_prompt();                                                //
//   Action : Displays '=== wait ===' on next line then waits for a         //
//            keystroke.  When a key is hit the '=== wait ===' is cleared   //
//            and the cursor is left in column 1 of the line.  The screen   //
//            will scroll if necessary.                                     //
//--------------------------------------------------------------------------//
void wait_prompt(void)
{
  uchar key;

  printf("\n=== wait ===");
  key=getchar();
  printf("\r            \r");

}

//--------------------------------------------------------------------------//
// Function: scsi_logic_error()                                             //
// Action:   Flashes the LEDs with 2 codes forever.                         //
//           To be called when a condition occurs that should never occur.  //
//           For DEVELOPMENT_MODE debug use only.                           //
//--------------------------------------------------------------------------//

#if SCSI_DEBUG
#if      DEVELOPMENT_MODE    == 1
void scsi_logic_error(unsigned int ledcode1,unsigned int ledcode2)
{
  printf("\nInternal SCSI logic error\n");
  while (1)
    {
    }
}

#endif
#endif

//--------------------------------------------------------------------------//
// Function: hf_led_on()                                                    //
// Action:   Turns on the front panel hardfile led.                         //
//--------------------------------------------------------------------------//
void hf_led_on(void)
{
  out8( hf_led_port, 1 );
}

//--------------------------------------------------------------------------//
// Function: hf_led_off()                                                   //
// Action:   Turns off the front panel hardfile led.                        //
//--------------------------------------------------------------------------//
void hf_led_off(void)
{
  out8( hf_led_port, 0 );
}


//--------------------------------------------------------------------------//
// Function: controller_id()                                                //
// Action:   Return the SCSI Bus ID(s) stored in NVRAM for the caller.      //
//           If the handle or count values don't match, default ID(s) of    //
//           DEFAULT_BUS_ID (defined in scsi_drvr.h) are returned.          //
//--------------------------------------------------------------------------//
void controller_id(ADAPTER_HANDLE handle,
                   unsigned char *id_buffer,
                   unsigned char  id_count)
{
 unsigned int i;
 unsigned int NVRAM_handle;

 NVRAM_handle = (((uint)handle.type)    << 24) +
                (((uint)handle.sys_bus) << 16) +
                (uint)handle.sys_id.ID;  // Reference sys_id as an unsigned short value

 if( (NVRAM_read_word(SCSI_CFG_DATA) == *((uint *)SCSI_sig)) &&
     (NVRAM_read_word(SCSI_CFG_DATA + 5) == NVRAM_handle) &&
     (NVRAM_read_byte(SCSI_CFG_DATA + 9) == id_count)
   )

  // NOTE !!!! - Since only 1 adapter is supported at the moment, I'm not checking the
  //             the adapter count field of the data.  This will have to change when
  //             more than 1 adapter is supported.

    {
     // found the data OK, read in the id(s)
     for(i=0;i<( (id_count+1)/2);i++) id_buffer[i] = NVRAM_read_byte(SCSI_CFG_DATA + 10 + i);
    }
 else
    {
     // Nothing stored in NVRAM at the moment so fill with default data
     i = 0;           // index into id_buffer
     while(id_count)
       {
        id_buffer[i] = DEFAULT_BUS_ID;     // Set even numbered bus ID to default
        id_count--;
        if(id_count)
          {
           id_buffer[i] |= DEFAULT_BUS_ID << 4; // Set ID for odd numbered bus
           id_count--;
          }
       }
    }
 return;
}

//--------------------------------------------------------------------------//
// Function: sys_mem_address()                                              //
// Action:   Returns the address modifier value needed for a bus master     //
//           to address physical memory location 0x00000000.  The handle    //
//           supplied is used to determine which system bus the instance    //
//           is attached to and returns the appropriate conversion factor.  //
//                                                                          //
//           As the PCI NCR controller is the only one currently supported, //
//           the function currently just returns the value 0x80000000.      //
//--------------------------------------------------------------------------//

unsigned int sys_mem_address   (ADAPTER_HANDLE handle)
{
 return (PCI_TO_SYS_MEM);  // Defined in scsi_public.h
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_addr()                                                 //
// Action:   Return the actual PCI configuration register address based on  //
//           the PCI Bus and Device numbers specified in the adapter's      //
//           handle plus the requested register offset.                     //
//                                                                          //
//           The returned address is truncated as necessary to the previous //
//           4 byte aligned address (eg. address 0x16 would be 0x14)        //
//--------------------------------------------------------------------------//
unsigned int pci_cfg_addr(ADAPTER_HANDLE handle,
                          unsigned int   addr)
{
 if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
 return
 ( PCI_IO_ADDRESS_SPACE +   // System implementation dependant mapping modifier

   // The next term could be read as a halfword then swapped, but the way it
   // is coded will ensure it works regardless of the order the Bus and Device
   // number is stored in memory.

   ( ( ((uint)handle.sys_id.PCI_Addr.Bus) << 16) |
     ( ((uint)handle.sys_id.PCI_Addr.Device) << 8) ) +

   (addr & 0xfffffffc)  // Convert register offset to 4 byte aligned address
 );
 } else {
	/*
	 *  PCI 2.0 configuration
	 */
 return
 ( PCI_IO_ADDRESS_SPACE +   // System implementation dependant mapping modifier

   // The next term could be read as a halfword then swapped, but the way it
   // is coded will ensure it works regardless of the order the Bus and Device
   // number is stored in memory.

   ( ( ((uint)handle.sys_id.PCI_Addr.Bus) << 16) |
     ( ((uint)handle.sys_id.PCI_Addr.Device) << 11) ) +

   (addr & 0xfffffffc)  // Convert register offset to 4 byte aligned address
 );
 }
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_read_8()                                               //
// Action:   Read a byte from the specified PCI configuration register      //
//--------------------------------------------------------------------------//
unsigned char pci_cfg_read_8 (ADAPTER_HANDLE handle,
                              unsigned int   addr)
{
 unsigned int data;

 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
        data = in32le( pci_cfg_addr(handle,addr) ) >> (8 * (addr & 0x03));
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	data = in32le(0x80000cfc) >> (8 * (addr & 0x03));
    }
   }
 else
   {
    data = 0;
   }
 return(data);
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_read_16()                                              //
// Action:   Read a word (halfword ?) from the specified PCI                //
//           configuration register                                         //
//--------------------------------------------------------------------------//
unsigned short pci_cfg_read_16 (ADAPTER_HANDLE handle,
                                unsigned int   addr)
{
 unsigned int data,data2;

 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
        data = in32le( pci_cfg_addr(handle, addr) );
        if( (addr & 0x03) < 3 )
          {
           // data contains all 16 bits of data
           data = data >> (8 * (addr & 0x03));
          }
        else
          {
           data2 = in32le( pci_cfg_addr(handle, addr + 1) );
           data = (data2 << 8) | (data >> 24);
          }
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	data = in32le(0x80000cfc);
	if( (addr & 0x03) < 3 )
	  {
           // data contains all 16 bits of data
           data = data >> (8 * (addr & 0x03));
          }
	else
	  {
	   out32le(0x80000cf8, pci_cfg_addr(handle, addr+1)); /* set addr reg */
	   data2 = in32le(0x80000cfc);
           data = (data2 << 8) | (data >> 24);
	  }
    }
   }
 else
   {
     data = 0;
   }
 return (data);
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_read_32()                                              //
// Action:   Read a double-word (word ?) from the specified PCI             //
//           configuration register                                         //
//--------------------------------------------------------------------------//
unsigned int pci_cfg_read_32 (ADAPTER_HANDLE handle,
                              unsigned int   addr)
{
 unsigned int data,data2;


 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
        data = in32le( pci_cfg_addr(handle, addr) );
        if( addr & 0x03 )
          {
           // need to read more data from the next 32 bit address
           data2 = in32le( pci_cfg_addr(handle, addr + 4) );
           // now combine the data from both 32 bit addresses
           data = (data >> ((addr & 0x03) * 8)) |
		(data2 << ((4-(addr & 0x03)) * 8));
          }
    } else {
	/*
	 *  PCI 2.0 configuration
	 */

	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	data = in32le(0x80000cfc);
	if( addr & 0x03 )
	  {
           // need to read more data from the next 32 bit address
	   out32le(0x80000cf8, pci_cfg_addr(handle, addr+1)); /* set addr reg */
	   data2 = in32le(0x80000cfc);
           // now combine the data from both 32 bit addresses
           data = (data >> ((addr & 0x03) * 8)) |
		(data2 << ((4-(addr & 0x03)) * 8));
          }
    }
   }
 else
   {
    data = 0;
   }
 return(data);
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_write_8()                                              //
// Action:   Write a byte to the specified PCI configuration register       //
//--------------------------------------------------------------------------//
void pci_cfg_write_8 (ADAPTER_HANDLE handle,
                      unsigned int   addr,
                      unsigned char  data)
{
 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
	out8( pci_cfg_addr(handle, addr), data );
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	out8(0x80000cfc, data);                          /* write data      */
    }
   }
 return;
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_write_16()                                             //
// Action:   Write a word (halfword ?) to the specified PCI                 //
//           configuration register                                         //
//--------------------------------------------------------------------------//
void pci_cfg_write_16 (ADAPTER_HANDLE handle,
                       unsigned int   addr,
                       unsigned short data)
{
 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
	out16le( pci_cfg_addr(handle, addr), data );
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	out16le(0x80000cfc, data);                       /* write data      */
    }
   }
 return;
}

//--------------------------------------------------------------------------//
// Function: pci_cfg_write_32()                                             //
// Action:   Write a double-word (word ?) to the specified PCI              //
//           configuration register                                         //
//--------------------------------------------------------------------------//
void pci_cfg_write_32 (ADAPTER_HANDLE handle,
                       unsigned int   addr,
                       unsigned int   data)
{
 if(handle.sys_bus == PCI_Bus)
   {
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
	out32le( pci_cfg_addr(handle, addr), data );
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
	out32le(0x80000cf8, pci_cfg_addr(handle, addr)); /* set up addr reg */
	out32le(0x80000cfc, data);                       /* write data      */
    }
   }
 return;
}
#endif /*DEBUG*/
