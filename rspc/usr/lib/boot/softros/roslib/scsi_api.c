static char sccsid[] = "@(#)27	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_api.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:08";
#ifdef DEBUG

#ifdef LISTON
 #pragma options nosource
#endif

/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS: see scsi_api.h
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
                                                                            */
#define SCSI_API 1                                      /* identify this file to included files */
#define SCSI_DEBUG 1                                      /* identify this file to included files */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
//#include <screen.h>
#include <misc_sub.h>
#include <rtcodes.h>
#include <io_sub.h>
#define DEBUGGER 1
#include <ncrsdef.h>   // TEMPORARY !!!!!!!! don't intend to leave 'stf' active here
#include <scsi2.h>
#include <scsi_public.h>
#include <scsi_externals.h>

int sdf = 0;
int stf = 0;

//--------------------------------------------------------------------------//
// External global definitions                                              //
//--------------------------------------------------------------------------//

#include <scsi_drvr.h>   // common driver definitions including config table
#include <scsi_text.h>   // Text messages

//--------------------------------------------------------------------------//
// Declarations of external functions used by this file.                    //
//--------------------------------------------------------------------------//

#include <scsi_util.h>
#include <scsi_ctl.h>
#include <misc_asm.h>

void mem_fill( char *adr, int length,uchar fill);
void mem_dump( char *adr, int length);

//--------------------------------------------------------------------------//
// External declarations of functions defined in this file & reltd #defines //
//--------------------------------------------------------------------------//

//#include <scsi_api.h>  // included in scsi_drvr.h

//--------------------------------------------------------------------------//
// Local definitions of functions used only by functions in this file       //
//--------------------------------------------------------------------------//

#ifdef LISTON
 #pragma options source list
#endif
uint scsi_tur( ADAPTER_INFO_REC  *card_info_ptr,         // Pointer to adapter info record
               DEV_INFO_REC      *device_info_ptr,       // Pointer to device info record
               int                retry_count );         // Number of retries to perform

int scsi_start_motor(ADAPTER_INFO_REC *card_info_ptr,
                     DEV_INFO_REC     *device_info_ptr);

uint scsi_cap( ADAPTER_INFO_REC  *card_info_ptr,
               DEV_INFO_REC      *device_info_ptr);

int prep_scsi_device( ADAPTER_INFO_REC  *card_info_ptr,
                      DEV_INFO_REC      *device_info_ptr);

void maybe_open(ADAPTER_INFO_PTR adapter_info_ptr);

/*--------------------------------------------------------------------------*/
/* Definitions used only within this file                                   */
/*--------------------------------------------------------------------------*/
#if      NCR_SUPPORTED
  #include <scsi_ncr.h>
#endif

static DEV_INFO_REC *devices = 0;               // Pointer to first device information record
static SCSI_CONFIG_TABLE_PTR  cfg_tbl_ptr = 0;  // Configuration table pointer
static unsigned int adapter_info_size;          // Size of each ADAPTER_INFO_REC field
static unsigned char *inquiry_data;             // buffer for device inq data

static CMD_DESC reserve_cmd = {0, 6, 0, 0, { 0x16, 0x00, 0x00, 0x00, 0x00, 0x00 } };

#ifdef LISTON
 #pragma options source
#endif

void scsi_init(void)
{

//ADAPTER_INFO_PTR       card_info_ptr;         // Adapter info record pointer
  ADAPTER_INFO_REC      *card_info_ptr;         // Pointer to current adapter info record
  DEV_INFO_REC          *device_info_ptr;       // Pointer to current device info record
  int                    card_num;              // card index number
  int                    device_num;            // device index number
  uint                   index;                 // byte index value
  int                    i,rc;                  // misc. counters
  register uchar        *byte_ptr;              // memory (byte) location ptr
  signed char            bus_num,id_num,lun_num;// current search device address
  unsigned int           iq_data_avail;         // inquiry data bytes returned

                     // Specify system bus type search sequence
  unsigned char      bus_search[] = { PCI_Bus };  // Only looking at the PCI bus for right now

                     // Specify device driver(s) to call and order to call them in
  unsigned char      driver_search[] =
                            {
                              #if NCR_SUPPORTED
                               NCR,
                              #endif
                              #if KEY_SUPPORTED
                               Key_West,
                              #endif
                            };

#if SCSI_DEBUG
//if( FALSE != check_key_avail() )
//  {
//   sdf = 1;
//   stf = 1;
//  }
//else
//  {
//   sdf = 0;
//   stf = 0;
//  }
#endif

  hf_led_off();

  // allocate the configuration table space

  if(cfg_tbl_ptr != 0)
    {
     free(cfg_tbl_ptr);   // Free up previously allocated table
     free(inquiry_data);  // Free up space previously allocated for temporary data
    }

  cfg_tbl_ptr = (SCSI_CONFIG_TABLE_PTR) malloc(1024);   // allocate 1 KB for configuration table
  inquiry_data = (uchar *) malloc(128);                 // and workspace for manipulating inq. data
                                                        // 128 bytes allocated so buffer can be used
                                                        // for clearing msgs on the screen also
  //---------------------------------------------------------------------------------------------//
  //                             initialize the configuration table                              //
  //---------------------------------------------------------------------------------------------//

  cfg_tbl_ptr->ADAPTER_DATA_SIZE = 0;
  cfg_tbl_ptr->ADAPTER_COUNT = 0;
  cfg_tbl_ptr->DEVICE_COUNT = 0;
  cfg_tbl_ptr->int_expected = 0;
  cfg_tbl_ptr->int_flag = 0;

  card_num = 0;

  // get pointer to the first adapter entry
  card_info_ptr = (ADAPTER_INFO_PTR) cfg_tbl_ptr->ADAPTER;


  //---------------------------------------------------------------------------------------------//
  //         Determine the size of the adapter's "private" data area in the config table         //
  //---------------------------------------------------------------------------------------------//

  for(index=0;index < sizeof(driver_search);index++)
    {
     card_info_ptr->handle.type = driver_search[index];  // get (next) controller type value

     rc = scsi_cmd( card_info_ptr,     // Pointer to current adapter information record
                    0,                 // Pointer to current device information record
                                       //   (not required for this command)
                    Adapter_Dsize,     // Perform the Locate function
                    0,                 // data_buffer - unused for this command
                    0,                 // byte_count - unused for this command
                    0,                 // lba - unused for this command
                    0,                 // lba_count - unused for this command
                    0,                 // retry_count - unused for this command
                    0 );               // timout - unused for this command

     if(rc > cfg_tbl_ptr->ADAPTER_DATA_SIZE) cfg_tbl_ptr->ADAPTER_DATA_SIZE = rc;
    }

  // Initialize (clear) first adapter information record
  byte_ptr = (uchar *) card_info_ptr;     // get a pointer to the current adapter entry

  // The following calculation ensures each adapter info record begins on a dword boundary
  // (this also, eventually, ensures the first device info record is dword aligned)
  adapter_info_size = ( ( ((uint)sizeof(ADAPTER_INFO_REC)-1) +
                          (uint)cfg_tbl_ptr->ADAPTER_DATA_SIZE +
                          3
                        ) & 0xfffffffc
                      );
  for(i=0;i<adapter_info_size;i++) byte_ptr[i]=0;

  //---------------------------------------------------------------------------------------------//
  //             Search supported system bus types for attached SCSI controllers //              //
  //---------------------------------------------------------------------------------------------//

  for(index=0;
      ( (index<sizeof(bus_search)) && (card_num<SUPPORTED_ADAPTERS) );
      index++
     )
    {
     //------------------------------------------------------------------------------------------//
     //             Search all supported device drivers                                          //
     //------------------------------------------------------------------------------------------//
     for(device_num=0;
         ( (device_num<sizeof(driver_search)) && (card_num<SUPPORTED_ADAPTERS) );
         device_num++)
       {

        card_info_ptr->handle.sys_bus = bus_search[index];       // Get (next) bus and driver
        card_info_ptr->handle.type = driver_search[device_num];  // type to search for

        card_info_ptr->status.Locate = 1;     // specify Locate mode type OPEN command

        rc = scsi_cmd( card_info_ptr,     // Pointer to current adapter information record
                       0,                 // Pointer to current device information record
                                          //   (not required for this command)
                       OPEN,              // Perform the Locate function
                       0,                 // data_buffer |
                       0,                 // byte_count  |
                       0,                 // lba         |-- Unused for this command
                       0,                 // lba_count   |
                       0,                 // retry_count |
                       0 );               // timeout     |

        (void) scsi_cmd( card_info_ptr,   // Pointer to current adapter information record
                         0,               // Pointer to current device information record
                                          //   (not required for this command)
                         CLOSE,           // Close the adapter down to find the next one
                         0,               // data_buffer |
                         0,               // byte_count  |
                         0,               // lba         |-- Unused for this command
                         0,               // lba_count   |
                         0,               // retry_count |
                         0 );             // timeout     |

        if(rc != NO_SCSI_CARD )
          {
           cfg_tbl_ptr->ADAPTER_COUNT++;     // installed adapter if rc != NO_SCSI_CARD
                                             // non-zero rc other than NO_SCSI_CARD means error
           card_info_ptr->status.Locate = 0; // Next OPEN will be a normal mode command
           card_num++;

           // generate info record pointer for new entry
           card_info_ptr = (ADAPTER_INFO_PTR) ( (uint)card_info_ptr + adapter_info_size );

           // initialize new adapter information record block

           byte_ptr = (uchar *) card_info_ptr;
           for(i=0;i<rc;i++) byte_ptr[i]=0;
          }
       }  // end of supported device driver search loop
    } // end of supported system bus type search loop


  //---------------------------------------------------------------------------------------------//
  // All supported SCSI instances have been located at this point, now OPEN them (same order     //
  // in which they were found) and scan for attached devices                                     //
  //---------------------------------------------------------------------------------------------//


  device_num = 0;                                // Clear attached device counter
  devices = (DEV_INFO_REC *) card_info_ptr;      // set (global/permanent) pointer to first device
                                                 // information record address
  device_info_ptr = devices;                     // set working pointer

  // initialize first device information record block
  byte_ptr = (uchar *) device_info_ptr;     // get a byte pointer to the first device entry
  for(index=0;index<sizeof(DEV_INFO_REC);index++) byte_ptr[index]=0;

  printf("\n");  // leave blank line on screen before listing scsi devices found

  for(card_num=0;
      ( (card_num<cfg_tbl_ptr->ADAPTER_COUNT) && (device_num < SUPPORTED_DEVICES) );
      card_num++
     )
    {
#if SCSI_DEBUG
     if(sdf && (cfg_tbl_ptr->ADAPTER_COUNT > 1)) printf("Scanning card %d\n",card_num);
#endif
     // Develop pointer to current adapter information record
     card_info_ptr = (ADAPTER_INFO_PTR) ( (uint)cfg_tbl_ptr->ADAPTER +
                                          ((uint)card_num * adapter_info_size) );
     // Open the adapter if necessary
     maybe_open(card_info_ptr);

     for(bus_num=0;
         ( (bus_num <= card_info_ptr->max_bus) && (device_num < SUPPORTED_DEVICES) );
         bus_num++
        )
       {
        for(id_num=card_info_ptr->max_id;
            ( (id_num >= 0) && (device_num < SUPPORTED_DEVICES) );
            id_num--
           )
          {
           for(lun_num=0;
               ( (lun_num <= card_info_ptr->max_lun) && (device_num < SUPPORTED_DEVICES) );
               lun_num++
              )
             {
              // Initialize current device info record with current search parameters

//            printf("\nDevice entry @ %X",device_info_ptr);
              device_info_ptr->adapter = card_num;
              device_info_ptr->bus     = bus_num;
              device_info_ptr->id      = id_num;
              device_info_ptr->lun     = lun_num;
//            device_info_ptr->max_lba = 0x12345678;
              device_info_ptr->status.Negotiate = 1;  // Need to establish data rate on first access

              // Provide operator indication of progress while scanning for devices

              printf(check_msg);
              if(bus_num) printf(bus_msg,bus_num);
              printf(id_msg,id_num);
              if(lun_num) printf(lun_msg,lun_num);
              printf("\r");

              iq_data_avail = 32;
              rc = scsi_cmd(card_info_ptr,           // Go see if there's a device
                            device_info_ptr,         // at the current search
                            DEVICE_Inquiry,          // address.
                            inquiry_data,            //
                            iq_data_avail,           // Go for all 32 bytes
                            0,                       //
                            0,                       //
                            2,                       //
                            5);                      //

              // Info on devices :
              //  - IOMEGA BETA90 revision 1.2
              //      When an invalid(uninstalled ?) LUN is addressed, this device only
              //      returned 1 byte of inquiry data (0x7f).  It also Aborts a multi-byte
              //      synchronous negotiations message (even if set for asynchronous).
              //  - IBM CDRM00201 (FRU ID 'F') revision 1723
              //      Returns all requested Inquiry data when accessing 'invalid' LUN with
              //      the first byte set to 0x7f.
              //  - IBM MD3125B EH1400 (FRU ID 'B') revision 0
              //      This version of the Erimo only returns 8 bytes of inquiry data
              //      when an 'invalid' LUN is addressed with the first byte set fo 0x7f.

              if (rc & SHORT_RECORD)
                {
                 iq_data_avail -= (rc-SHORT_RECORD);
                 rc = ( iq_data_avail > 1 ) ? 0 : DEVICE_NOT_AVAILABLE;

                 // (sort of) OK if I at least got the first 2 bytes
                }

              if (!rc)
                {
                 if ( ( (inquiry_data[0] >> 5) == 0 ) ||
                      ( (inquiry_data[0] >> 5) >= 4 ) )
                    {
                     // Device 'present' section

                     // initialize the device information record with new information
                     device_info_ptr->type = inquiry_data[0];
                     device_info_ptr->status.Rem_Media = (inquiry_data[1] >> 7);
                     if(lun_num) device_info_ptr->status.Device_LUN_Sig = 1;

                     printf(SCSI_sig); printf(" ");
                     if( inquiry_data[0] <
                         ( (sizeof(type_msg)/sizeof(type_msg[0])) - 1 ) // Don't include Disk_msg in value check
                       ) // Device type value within range of table ?
                       {
                        if( (inquiry_data[0]) ||      // if NOT a DASD type device OR
                            (inquiry_data[1] & 0x80)  // DASD with removable media,
                          )
                          inquiry_data[0]++;          // Adjust value to skip Disk_msg pointer

                        printf(type_msg[inquiry_data[0]]);
                       }
                     else
                       {
                        if( inquiry_data[0] >= 0x80 )  // If vendor specific type
                          printf(vendor_msg);
                        else
                          printf(unknown_msg);
                       }

                     printf(" (");
                     if (bus_num) printf(bus_msg,bus_num);
                     printf(id_msg,id_num);
                     if (lun_num) printf(lun_msg,lun_num);
                     printf(") : ");

                     if (iq_data_avail == 32)
                       {
                        inquiry_data[32]='\0';    /* set string terminator character */
                        printf("%s\n",&inquiry_data[8]);
                       }
                     else
                       {
                        printf("%s\n",no_vpd_msg);
                       }

                     //
                     // Now indicate device's presence and create a new device record
                     //
                     device_num++;
                     device_info_ptr++;

                     // initialize new device information record block

                     byte_ptr = (uchar *) device_info_ptr;
                     for(index=0;index<sizeof(DEV_INFO_REC);index++) byte_ptr[index]=0;

                    } // End of 'device present' section
                } // End of rc == 0
              else
                {
                 if( ( (rc==DEVICE_NOT_AVAILABLE) || (rc==RESET_NEEDED) ) &&
                     (!lun_num) ) break;
                }

             } // End of for all LUN values loop
          } // End of for all ID values loop
       } // End of for all adapter's busses loop
    } // End of search on all installed 'adapters'

  // Clear last 'Checking ....' line
  i = (strlen(check_msg) + strlen(bus_msg) + strlen(id_msg) + strlen(lun_msg));  // Max msg length
  for(index=0; index<i; index++) inquiry_data[index] = ' ';
  inquiry_data[i] = '\0';  // string terminator
  printf("%s\r",inquiry_data);

  cfg_tbl_ptr->DEVICE_COUNT = device_num;  // Set attached device count in config table

#if SCSI_DEBUG
  if(sdf)
    {
     i = sizeof(SCSI_CONFIG_TABLE) - ( sizeof(ADAPTER_INFO_REC) + sizeof(DEV_INFO_REC) );
     i += (uint)cfg_tbl_ptr->ADAPTER_COUNT * adapter_info_size;
     i += (uint)cfg_tbl_ptr->DEVICE_COUNT * sizeof(DEV_INFO_REC);
     mem_dump((uchar *)cfg_tbl_ptr, i);
    }
#endif

}

void maybe_open(ADAPTER_INFO_PTR adapter_info_ptr)
{
 ADAPTER_INFO_PTR current_adapter_ptr;
 uint             adapter_num,rc;

 if(!adapter_info_ptr->status.Open)
   {
    for(adapter_num=0;
        adapter_num<cfg_tbl_ptr->ADAPTER_COUNT;
        adapter_num++
       )
      {
       current_adapter_ptr = (ADAPTER_INFO_PTR) cfg_tbl_ptr->ADAPTER;
       current_adapter_ptr += ((uint)adapter_num) *
                              (((uint)sizeof(ADAPTER_INFO_REC)-1) + (uint)cfg_tbl_ptr->ADAPTER_DATA_SIZE);

       if(current_adapter_ptr->status.Open)
         (void) scsi_cmd( current_adapter_ptr,   // Pointer to current adapter information record
                          0,                     // Pointer to current device information record
                                                 //   (not required for this command)
                          CLOSE,                 // Close the adapter down
                          0,                     // data_buffer |
                          0,                     // byte_count  |
                          0,                     // lba         |-- Unused for this command
                          0,                     // lba_count   |
                          0,                     // retry_count |
                          0 );                   // timeout     |

      }
    rc = scsi_cmd( adapter_info_ptr,   // Pointer to current adapter information record
                   0,                  // Pointer to current device information record
                                       //   (not required for this command)
                   OPEN,               // Perform the Locate function
                   0,                  // data_buffer |
                   0,                  // byte_count  |
                   0,                  // lba         |-- Unused for this command
                   0,                  // lba_count   |
                   0,                  // retry_count |
                   0 );                // timeout     |

    if(!rc) adapter_info_ptr->status.Open = 1;   // Indicate the adapter is now open
   }

 return;
}

unsigned int device_num_valid_check(command,device)
{
  register uint rc;

  rc = INVALID_DEVICE;  // initially assume invalid

  if( !device &&      // if device number 0 specified ...
      ( (command == Adapter_Query) ||  // and current command is
        (command == Device_Query) )    // a Query command ...
    )
     {
      rc = OK;   // ... Device number 0 is always valid
     }
  else
     {
      if(command == Adapter_Query)
        {
         if( device < cfg_tbl_ptr->ADAPTER_COUNT ) rc = OK;  // Adapter number is valid
        }
      else  // Device related command
        {
         if( (command == Get_Type_Count) ||
             (command == Get_LDN       ) ||
             (command == Get_LDN_SIG   ) ||
             (command == Find_LDN_SIG  )    )
           {
            rc = OK;  // Device number is actually a device type for these commands
           }
         else
           {
            if( device < cfg_tbl_ptr->DEVICE_COUNT ) rc = OK;   // Device number is valid
           }
        }
     }

  return(rc);
}

unsigned int scsi_io ( SCSI_OPS       command,
                       char          *read_buff,
                       int            device,
                       int            lba,
                       int            lba_count)
{

  ADAPTER_INFO_REC      *card_info_ptr;         // Pointer to adapter info record
  DEV_INFO_REC          *device_info_ptr;       // Pointer to device info record
  uint retry_count = 3;                         // Default read/write retry count
  uint rc,index;
  uchar bus_num;                                // Used if bus reset was issued

  rc = device_num_valid_check(command, device);  // Check validity of specified device number

  if(!rc) // Specified device number is valid !
    {
     if( (command == Adapter_Query) || (command == Adapter_Diags) )
       {
        index = device;  // Set (temporary) adapter index number
       }
     else
       {
        // Get pointer to appropriate device info record
        device_info_ptr = (DEV_INFO_REC *) ( (uint)devices +
                                             (device * sizeof(DEV_INFO_REC)) );
        index = device_info_ptr->adapter;    // Get adapter number for this device
       }

     // Get pointer to the appropriate adapter information record
     card_info_ptr = (ADAPTER_INFO_PTR) ( (uint)cfg_tbl_ptr->ADAPTER +
                                        ( index * adapter_info_size ) );

     switch(command)
       {
        case Read:
             {
#if SCSI_DEBUG
              if(stf || sdf) printf("\nRead - drive %d, %X lba's from lba %X to mem @%X",
                                device,
                                lba_count,
                                lba,
                                read_buff );
#endif
              rc = device_info_ptr->status.Ready ? 0 : prep_scsi_device(card_info_ptr,
                                                                        device_info_ptr);
              if (!rc)
                {
                 rc = device_info_ptr->status.Cap_Valid ? 0 : scsi_cap(card_info_ptr,
                                                                       device_info_ptr);
                 if (device_info_ptr->status.Cap_Valid)
                   {
                    rc=INVALID_ADDRESS;   // # blocks to read too big
                    if( (lba_count * device_info_ptr->block_size) < 0x1000000)
                      {
#if SCSI_DEBUG
//                     if(sdf) printf("Reading %X sector(s) from %X to memory @ %X\n",lba_count,lba,read_buff);
#endif
                       do
                         {
#if SCSI_DEBUG
                          if(sdf && retry_count && (retry_count < 3)) printf("\napi retry");
#endif
                          rc =scsi_cmd(card_info_ptr,
                                       device_info_ptr,
                                       READ_scsi_data,                // scsi op code for a read
                                       read_buff,
                                       0,
                                       lba,
                                       lba_count,
                                       0,                             // No retry at lower levels
                                       60);                           // Wait for large transfers

                          retry_count -= ((rc == 0) ? retry_count : 1);

                           // Do not do Retry if Unit Attention Sense.key here.  If the
                           // device was previously Not Ready and the Unit Attention is
                           // to be retried, the retry is performed at the scsi_cmd
                           // (scsi_ctl.c) level.

                          if(rc && sdf) printf("\nRetry cnt %d, sense key %X",retry_count,sense.key);
                         } while ( retry_count && ((sense.key & 0x0f) != 0x06) );
                      }
                   }
                }
#if SCSI_DEBUG
              if (rc && sdf) { printf("\nRead failed, api rc = %d\n",rc); };
#endif
              break;
             }
        case Write:
             {
#if SCSI_DEBUG
              if(stf || sdf) printf("\nWrite - drive %d, %X lba's from mem @%X to lba %X",
                                device,
                                lba_count,
                                read_buff,
                                lba );
#endif
              rc = device_info_ptr->status.Ready ? 0 : prep_scsi_device(card_info_ptr,
                                                                        device_info_ptr);
              if (!rc)
                {
                 rc = device_info_ptr->status.Cap_Valid ? 0 : scsi_cap(card_info_ptr,
                                                                       device_info_ptr);
                 if (device_info_ptr->status.Cap_Valid)
                   {
                    rc=INVALID_ADDRESS;   // # blocks to read too big
                    if( (lba_count * device_info_ptr->block_size) < 0x1000000)
                      {
#if SCSI_DEBUG
//                     if(sdf) printf("Writing %X sector(s) to %X from memory @ %X\n",lba_count,lba,read_buff);
#endif
                       do
                         {
                          rc =scsi_cmd(card_info_ptr,
                                       device_info_ptr,
                                       WRITE_scsi_data,                // scsi op code for a write
                                       read_buff,
                                       0,
                                       lba,
                                       lba_count,
                                       0,                             // No retry at lower levels
                                       60);                           // Delay for large transfers

                          retry_count -= ((rc == 0) ? retry_count : 1);
                          if(rc && sdf) printf("\nRetry cnt %d, sense key %X",retry_count,sense.key);
                         } while ( retry_count && ((sense.key & 0x0f) != 0x06) );
                      }
                   }
                }
#if SCSI_DEBUG
              if (rc && sdf) { printf("\nWrite failed, api rc = %d (0x%X)\n",rc,rc); };
#endif
              break;
             }
        case Test_Unit_Ready:
             {
              rc =  scsi_tur(card_info_ptr, device_info_ptr, 1); // allow 1 retry
              break;
             }
        case Start_Motor:
             {
              rc = scsi_cmd(card_info_ptr,           //
                            device_info_ptr,         //
                            START_motor_immed,       // Send the Start Unit command to the
                            0,                       // drive with the immediate bit set
                            0,                       //
                            0,                       //
                            0,                       //
                            1,                       // Allow 1 retry
                            10);                     // Allow up to 10 seconds for command to complete
              break;
             }
        case Adapter_Query:
             {
              ADAPTER_Q_REC *ret_data;

#if SCSI_DEBUG
              if(sdf) printf("\nAdapter %d Query",device );
#endif
              ret_data = (ADAPTER_Q_REC *) read_buff;
              ret_data->Adapter_Count =   cfg_tbl_ptr->ADAPTER_COUNT;
              rc=OK;      // Precharge return code for No Adapter installed

              if(cfg_tbl_ptr->ADAPTER_COUNT)
                {
                 ret_data->handle        = card_info_ptr->handle;
                 ret_data->Base_IO_Addr  = card_info_ptr->Base_IO_Addr;
                 ret_data->max_lun       = card_info_ptr->max_lun;
                 ret_data->max_id        = card_info_ptr->max_id;
                 ret_data->max_bus       = card_info_ptr->max_bus;
                 (void) memcpy(ret_data->bus_id, card_info_ptr->bus_id, 8);
                 ret_data->status        = card_info_ptr->status;

                 scsi_cmd( card_info_ptr,   //
                           0,               //  Get the VPD information from
                           Inquiry,         //  the adapter
                           (uchar *) &ret_data->VPD,  //
                           28,              //
                           0,               //
                           0,               //
                           0,               //
                           0);              //
                 rc = OK;
                }
              break;
             }
        case Device_Query:
             {
              DEVICE_Q_REC *ret_data;

#if SCSI_DEBUG
              if(sdf) printf("\nDevice %d Query",device );
#endif
              ret_data = (DEVICE_Q_REC *) read_buff;
              ret_data->Device_Count = cfg_tbl_ptr->DEVICE_COUNT;
              rc=OK;      // Precharge return code for No Devices installed

              if(cfg_tbl_ptr->DEVICE_COUNT)
                {
                 rc = scsi_cmd(card_info_ptr,           // Go get the device information
                               device_info_ptr,         //
                               DEVICE_Inquiry,          //
                               inquiry_data,            //
                               36,                      // Go for all 36 bytes
                               0,                       //
                               0,                       //
                               3,                       // Allow retries to get the data
                               30);                     // Up to 30 seconds (if device accesses
                                                        //  media before returning data)

                 if(rc & SHORT_RECORD)
                   {
                    mem_fill( inquiry_data, 36 - (rc - SHORT_RECORD), ' '); // Clear bytes not returned
                    rc = 0;                      // indicate data was returned
                   }

                 if(!rc) // If all (or at least some of) the data was obtained ...
                   {
                    (void) memcpy(&ret_data->VPD.vendor, &inquiry_data[8], 8);
                    ret_data->VPD.spc1 = ' ';
                    (void) memcpy(&ret_data->VPD.product_name, &inquiry_data[16], 16);
                    ret_data->VPD.spc2 = ' ';
                    (void) memcpy(&ret_data->VPD.revision, &inquiry_data[32], 4);
                    ret_data->VPD.string_term = '\0';
                   }

                 // Do a Read Capacity (in an attempt) to determine the
                 // device's block_size

                 (void) scsi_cap(card_info_ptr, device_info_ptr);

                 // Now do a Test Unit Ready to establish Ready status flag

                 (void) scsi_tur(card_info_ptr, device_info_ptr, 1); // allow 1 retry

                 ret_data->max_lba      = device_info_ptr->max_lba;
                 ret_data->block_size   = device_info_ptr->block_size;
                 ret_data->type         = device_info_ptr->type;
                 ret_data->adapter      = device_info_ptr->adapter;
                 ret_data->bus          = device_info_ptr->bus;
                 ret_data->id           = device_info_ptr->id;
                 ret_data->lun          = device_info_ptr->lun;
                 ret_data->status       = device_info_ptr->status;
                }
              break;
             }
        case Lock:
             {
              rc =scsi_cmd(card_info_ptr,
                           device_info_ptr,
                           LOCK,               // scsi op code to prevent media removal
                           read_buff,
                           0,
                           0,
                           0,
                           3,                  // Allow a few retries
                           30);                // Shouldn't take too long, but just in case
              break;
             }
        case Unlock:
             {
              rc =scsi_cmd(card_info_ptr,
                           device_info_ptr,
                           UNLOCK,             // scsi op code to allow media removal
                           read_buff,
                           0,
                           0,
                           0,
                           3,                  // Allow a few retries
                           30);                // Shouldn't take too long, but just in case
              break;
             }
        case Reserve:
             {
              reserve_cmd.cntl.read = 1;  // No data should be transfered
              reserve_cmd.cmd[0] = 0x16;  // Make sure CDB is for Reserve (not Release)

              rc = scsi_cmd(card_info_ptr,           //
                            device_info_ptr,         // Reserve the entire device
                            SEND,                    // for this controller
                            (uchar *) &reserve_cmd,  // (initiator) exclusively
                            0,                       //
                            0,                       //
                            0,                       //
                            3,                       //
                            30);                     //
              break;
             }
        case Release:
             {
              reserve_cmd.cntl.read = 1;  // No data should be transfered
              reserve_cmd.cmd[0] = 0x17;  // Make sure CDB is for Release (not Reserve)

              rc = scsi_cmd(card_info_ptr,           //
                            device_info_ptr,         // Release the exclusive access
                            SEND,                    // request for the device
                            (uchar *)&reserve_cmd,   //
                            0,                       //
                            0,                       //
                            0,                       //
                            3,                       //
                            30);                     //
              break;
             }
        case Get_Type_Count:
             {
              // Scan through attached devices and return count of 'device' types found
              rc = 0;  // Reset device counter

              // Need to change pointer to first device info record
              device_info_ptr = (DEV_INFO_REC *) (uint)devices;

              for(index=0;index<cfg_tbl_ptr->DEVICE_COUNT;index++)
                {
                 if(device == device_info_ptr->type) rc++;
                 device_info_ptr++;    // Point to next device record
                }
              return(rc);
              break;
             }
        case Get_LDN:
             {
              // Scan through attached devices and return LDN
              // when the Nth instance of 'device' type is found
              rc = 0;  // Reset device LDN value

              // Need to change pointer to first device info record
              device_info_ptr = (DEV_INFO_REC *) (uint)devices;

              for(index=0;index<cfg_tbl_ptr->DEVICE_COUNT;index++)
                {
                 if(device == device_info_ptr->type) rc++;
                 if(rc == lba) break;
                 device_info_ptr++;    // Point to next device record
                }
              index = (rc == lba) ? index : 0xffffffff;
              return(index);

              break;
             }
        case Get_LDN_SIG:
             {
              break;
             }
        case Find_LDN_SIG:
             {
              break;
             }
        case Send:
             {
              rc = scsi_cmd(card_info_ptr,           //
                            device_info_ptr,         // Forward the command on to the
                            SEND,                    // appropriate device
                            read_buff,               //
                            0,                       //
                            0,                       //
                            0,                       //
                            lba,                     //
                            lba_count);              //
              break;
             }
        case Adapter_Diags:
             {
              rc =scsi_cmd(card_info_ptr,
                           0,                  // Device pointer not required for command
                           DIAGnose,           // scsi op code for controller self test
                           0,
                           0,
                           0,
                           0,
                           0,                  // Allow a few retries
                           5);                 // Shouldn't take too long
              break;
             }
        default:
             {
              rc = INVALID_COMMAND;
              break;
             }
       }
    }
  return(rc);
}

uint scsi_tur( ADAPTER_INFO_REC  *card_info_ptr,         // Pointer to adapter info record
               DEV_INFO_REC      *device_info_ptr,       // Pointer to device info record
               int                retry_count )          // Number of retries to perform
{
  int rc;

  rc = scsi_cmd(card_info_ptr,           //
                device_info_ptr,         //
                TEST_UNIT_ready,         // Send the test unit ready
                0,                       // command to the device
                0,                       //
                0,                       //
                0,                       //
                retry_count,             //
                30);                     // Shouldn't take 30 seconds, but some devices complete
                                         // certain operations and don't respond to this cmd
                                         // as quickly as they probably should.

  if(!rc) device_info_ptr->status.Ready = 1;  // If device is ready set the flag

  return(rc);
}

int scsi_start_motor(ADAPTER_INFO_REC *card_info_ptr,
                     DEV_INFO_REC     *device_info_ptr)
{
  uint rc,retry;
  rtc_value start_delay;
  uint start_wait;        // Wait delay for device depends on removable media or not

   retry=5;
   start_wait = device_info_ptr->status.Rem_Media ? 10 : 30;
   start_delay = get_timeout_value(start_wait + 1 , SECONDS);
   do
      {
       rc = scsi_cmd(card_info_ptr,           //
                     device_info_ptr,         //
                     START_motor_immed,       // Send the Start Unit command to the
                     0,                       // drive with the immediate bit set
                     0,                       //
                     0,                       //
                     0,                       //
                     1,                       // Allow 1 retry
                     start_wait);             //
      } while ( rc && ( (rc == CMD_FAILED)   ||
                        (rc == CMD_ABORTED) )
                   && (FALSE == timeout(start_delay))
                   && retry-- );

   // If drive doesn't support immediate mode, issue Start cmd without immediate bit

   if ( rc == INVALID_COMMAND ) // if Illegal Request
     {
      start_delay = get_timeout_value(start_wait + 1 , SECONDS);  // Need to restart 11/31 second timer as the
                                                                  // command hasn't been accepted yet
      do
         {
          rc = scsi_cmd(card_info_ptr,           //
                        device_info_ptr,         //
                        START_motor,             // Send the Start Unit command to the
                        0,                       // drive with the immediate bit reset
                        0,                       //
                        0,                       //
                        0,                       //
                        0,                       // No retries
                        start_wait);             //
         } while ( rc && ( (rc == CMD_FAILED)   ||
                           (rc == CMD_ABORTED /* Aborted Cmd */) )
                      && retry-- );
     }

   // now that drive has performed (or at least accepted) the start command, see if it's ready

   if (rc != RESERVATION_CONFLICT)
     {
      do
        {
         scsi_delay(500000);   // delay 500,000 usec - 1/2 second before (next) command
         rc = scsi_tur( card_info_ptr,         // Pointer to adapter info record
                        device_info_ptr,       // Pointer to device info record
                        0);                    // No retries

        } while ( rc &&
                  (rc != NO_MEDIA) &&
                  (FALSE == timeout(start_delay))
                );
     }

   return(rc);
}


int prep_scsi_device(ADAPTER_INFO_REC  *card_info_ptr,         // Pointer to adapter info record
                     DEV_INFO_REC      *device_info_ptr)       // Pointer to device info record
{
  int ret,rc;
  int test_unit_ready_tries;

  ret=(-1);
  test_unit_ready_tries=5;
  do
    {
    ret = scsi_tur( card_info_ptr,     // Do a test unit ready with
                    device_info_ptr,   // 1 retry
                    1);                //
    if (!(ret))
      {
       test_unit_ready_tries=1;
      }
    else
      {
       if ( ( ret == NOT_READY )                            // If sense key is not ready ...
//          ( 0 == (device_info_ptr->type & 0x1f) )    &&   // if device is dasd,
//          ( ! (device_info_ptr->status.Rem_Media)  ) &&   // non-removable media, and
          )

         {
          // try to start the motor to get a unit ready condition
          ret = scsi_start_motor(card_info_ptr, device_info_ptr);
          test_unit_ready_tries=1;
         }
      }
    } while ( --test_unit_ready_tries &&
              (ret != INVALID_COMMAND) &&
              (ret != NO_MEDIA)
            );
  if (ret && (ret != INVALID_COMMAND))
    {
     device_info_ptr->status.Ready=0;
    }
  else
    {
     device_info_ptr->status.Ready=1;
     ret = scsi_cap(card_info_ptr, device_info_ptr);
     if(ret == INVALID_COMMAND) ret = 0;
    }
  return (ret);
}


uint scsi_cap( ADAPTER_INFO_REC *card_info_ptr,   // Pointer to adapter info record
               DEV_INFO_REC     *device_info_ptr) // Pointer to device info record
{

  uint rc;

  rc = scsi_cmd(card_info_ptr,           // Go get the device max lba
                device_info_ptr,         // and block size values
                READ_CAPacity,           //
                inquiry_data,            // store data in inquiry buffer
                8,                       // Get all 8 bytes
                0,                       //
                0,                       //
                3,                       // Allow retries to get the data
                30);                     //

  if (rc)
    {
#if SCSI_DEBUG
      if (sdf) printf("Read cap failed rc = %X\n",rc);
#endif
      device_info_ptr->status.Cap_Valid = 0;
    }
  else
    {
      device_info_ptr->status.Cap_Valid = 1;
      device_info_ptr->max_lba = (uint)( (inquiry_data[0] << 24) |
                                         (inquiry_data[1] << 16) |
                                         (inquiry_data[2] <<  8)  |
                                         (inquiry_data[3]      )
                                       );
      device_info_ptr->block_size = (uint)( (inquiry_data[4] << 24) |
                                            (inquiry_data[5] << 16) |
                                            (inquiry_data[6] <<  8)  |
                                            (inquiry_data[7]      )
                                          );
    }

    return(rc);
}

//// #ifdef DEBUGGER
//// uint scsi_read( char *read_buff,
////                 int drive,
////                 int lba,
////                 int lba_count)
//// {
////   return ( scsi_io ( Read,
////                      read_buff,
////                      drive,
////                      lba,
////                      lba_count) );
//// }
////
//// uint scsi_write( char *write_buff,
////                  int drive,
////                  int lba,
////                  int lba_count)
//// {
////   return ( scsi_io ( Write,
////                      write_buff,
////                      drive,
////                      lba,
////                      lba_count) );
//// }
////
////
//// uint scsi_lock( char *read_buff,
////                 int drive,
////                 int lba,
////                 int lba_count)
//// {
////   return ( scsi_io ( Lock,
////                      0,
////                      drive,
////                      0,
////                      0) );
//// }
////
////
//// uint scsi_unlock( char *read_buff,
////                   int drive,
////                   int lba,
////                   int lba_count)
//// {
////   return ( scsi_io ( Unlock,
////                      0,
////                      drive,
////                      0,
////                      0) );
//// }
////
//// #endif
#endif /*DEBUG*/
