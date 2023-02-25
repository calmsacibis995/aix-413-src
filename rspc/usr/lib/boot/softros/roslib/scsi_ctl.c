static char sccsid[] = "@(#)33	1.1  src/rspc/usr/lib/boot/softros/roslib/scsi_ctl.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:19";
#ifdef DEBUG

#ifdef LISTON
 #pragma options source list flag=I:I
#endif

/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS: see scsi_ctl.h and below.
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
#ifdef LISTON
 #pragma options source
#endif

#define SCSI_CTL 1
#define SCSI_DEBUG 1
/*--------------------------------------------------------------------------*/
/* External global definitions                                              */
/*--------------------------------------------------------------------------*/

#include <sys/types.h>
#include <string.h>
#include <rtcodes.h>
#include <scsi2.h>
#include <scsi_public.h>
/*--------------------------------------------------------------------------*/
/* Declarations of external functions used by this file.                    */
/*--------------------------------------------------------------------------*/


#include <misc_asm.h>

#include <scsi_drvr.h>
#include <scsi_util.h>
#include <scsi_ncr.h>
#include <ncrsdef.h>    // Looking for 'trace' definition

#include <scsi_externals.h>

void mem_dump( char *adr, int length);

/*--------------------------------------------------------------------------*/
/* External declarations of functions defined in this file & reltd #defines */
/*--------------------------------------------------------------------------*/

#include <scsi_ctl.h>

/*--------------------------------------------------------------------------*/
/* Local definitions of functions used only by functions in this file       */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Definitions used only within this file                                   */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Command (CDB) definitions                                                */
/*--------------------------------------------------------------------------*/

static uchar INQUIRY_command[]={ 0x12,0x0,0x0,0x0,0x05,0x0 };

static uchar T_U_R_command[]={ 0x0,0x0,0x0,0x0,0x0,0x0 };

static uchar SENSE_command[]={ 0x3,0x0,0x0,0x0,0x0E,0x0 };

static uchar START_command[]={ 0x1B,0x0,0x0,0x0,0x1,0x0 };

static uchar READ_CAP_command[]={ 0x25,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0 };

static uchar LOCK_command[]={ 0x1E,0x0,0x0,0x0,0x0,0x0 };

#pragma options align=packed
typedef struct {
    unsigned char op_field;
    unsigned char lun_field;
    unsigned short lba_h_field;
    unsigned short lba_l_field;
    unsigned char reserved;  /* reserved field */
    unsigned char lba_h_cnt_field;
    unsigned char lba_l_cnt_field;
    unsigned char cntl_field;
} RW_cmds;
#pragma options align=reset

static uchar READ_command[] = { 0x28,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x1,0x0 };

static uchar WRITE_command[] = { 0x2A,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0 };

static CMD_DESC cmd_desc;
static RW_cmds *cmd_ptr = (RW_cmds *)cmd_desc.cmd;


/* Sense key translation table   */
#pragma options align=packed
static uchar keys[] = { CMD_FAILED,       /* 0x00 no sense data returned */
                        OK,               /* 0x01 recovered error        */
                        NOT_READY,        /* 0x02                        */
                        MEDIA_ERROR,      /* 0x03                        */
                        HARDWARE_ERROR,   /* 0x04                        */
                        INVALID_COMMAND,  /* 0x05 illegal request        */
                        UNIT_ATTENTION,   /* 0x06                        */
                        WRITE_PROTECT,    /* 0x07 data protect error     */
                        BLANK_CHECK,      /* 0x08                        */
                        CMD_FAILED,       /* 0x09 vendor specific code   */
                        CMD_FAILED,       /* 0x0A copy aborted           */
                        CMD_ABORTED,      /* 0x0B aborted command        */
                        CMD_FAILED,       /* 0x0C equal (error ???)      */
                        CMD_FAILED,       /* 0x0D volume overflow        */
                        CMD_FAILED,       /* 0x0E miscompare             */
                        CMD_FAILED        /* 0x0F reserved               */
                      };

typedef struct xlat_asc { uchar asc;
                          uchar rc;
                        }XLAT_ASC;

static struct xlat_asc ascs[] = {
                                  0x04 , NOT_READY,
                                  0x20 , INVALID_COMMAND,
                                  0x21 , INVALID_ADDRESS,
                                  0x27 , WRITE_PROTECT,
                                  0x28 , MEDIA_CHANGED,
                                  0x29 , DEVICE_RESET,
                                  0x31 , FORMAT_CORRUPT,
                                  0x3A , NO_MEDIA,
                                };


typedef struct stat { unsigned char error;
                      struct { int Ready : 1;
                               int Cap_Valid : 1;
                             } flag;
                    } STAT;

// Only the return codes that should clear the Ready
// and/or Cap_Valid flag appears in the following array
// In the table, a 0 value for the flag indicates the bit is to be reset
// if the command resulted in the specified return code.
//                                 return code            Ready Cap_Valid
static struct stat stat_mod[] = { NOT_READY,            {  0  ,   0     },
                                  UNIT_ATTENTION,       {  1  ,   0     },
                                  MEDIA_CHANGED,        {  1  ,   0     },
                                  DEVICE_RESET,         {  1  ,   0     },
                                  NO_MEDIA,             {  0  ,   0     },
                                  DEVICE_NOT_AVAILABLE, {  0  ,   0     },
                                  START_REQUIRED,       {  0  ,   0     },
                                  MANUAL_INTERVENTION,  {  0  ,   0     },
                                  FORMAT_IN_PROGRESS,   {  0  ,   0     },
                                  FORMAT_CORRUPT,       {  1  ,   0     },
//                                CMD_FAILED,
//                                HARDWARE_ERROR,
//                                INVALID_COMMAND,
//                                WRITE_PROTECT,
//                                BLANK_CHECK,
//                                CMD_ABORTED,
//                                BUSY,
//                                RESERVATION_CONFLICT,
//                                RESET_NEEDED,

                                };
#pragma options align=reset


#ifdef LISTON
 #pragma options source list
#endif

//---------------------------------------------------------------------------//
// Function: scsi_cmd()                                                      //
//                                                                           //
// Action:   Direct the command to the proper driver based on adapter handle //
//                                                                           //
//                                                                           //
// Return:   0 if successful                                                 //
//           See return codes in rtcodes.h if not                            //
//---------------------------------------------------------------------------//

unsigned int scsi_cmd ( ADAPTER_INFO_PTR adapter_ptr,
                        DEVICE_INFO_PTR  device_ptr,
                        CMD_OPS          op,
                        unsigned char *  data_buffer,
                        unsigned int     byte_count,
                        unsigned int     lba,
                        unsigned int     lba_count,
                        int              retry_count,
                        int              timeout)
{
  int                rc,ret;
////****  register unsigned char *sense_ptr;
////****  register unsigned int index;
          unsigned char *sense_ptr;
          unsigned int index;
  rtc_value          busy_timer;
  int                attn_retry = 0;
  int                retry_flag;
  unsigned int       driver_op;
  unsigned char     *driver_data_buffer;

  do
    {
#if SCSI_DEBUG
     if(sdf && attn_retry && (op != REQUEST_sense)) printf("\nUnit Attention retry (cmd %d)",op);
#endif

     // Prechare the driver command and buffer pointers assuming
     // the driver command will be a SEND command

     ret = 0;   // Precharge indicator for recognized command (op)
     driver_op = SEND;
     driver_data_buffer = (unsigned char *)&cmd_desc;

     // Initialize the sense Key, code, and qualifier fields of sense data
     * ( (uint *)&sense.error ) = 0;
     * ( (ushort *)&sense.code ) = 0;

     switch(op)
       {
        //----------------------------------------------------------------------//
        // The following commands do not require initialization of a SCSI CDB   //
        // (or the command is directed at the controller or device driver code) //
        //----------------------------------------------------------------------//

        case Adapter_Dsize:
        case Inquiry:
        case OPEN:
        case CLOSE:
        case BUS_RESET:
        case DIAGnose:
        case SEND:
             {
              driver_op = op;
              driver_data_buffer = data_buffer;
              break;
             }

        //----------------------------------------------------------------------//
        // The following commands require building a command descriptor and     //
        // in some cases, modification to the SCSI CDB to be sent to the device //
        //----------------------------------------------------------------------//

        case TEST_UNIT_ready:
             {
              cmd_desc.cntl.read = 1;             // Really a don't care for this command
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Another don't care - no data involved
              cmd_desc.byte_count = 0;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &T_U_R_command, 6);
              break;
             }
        case REQUEST_sense:
             {
              cmd_desc.cntl.read = 1;             // Getting sense data from device
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Return data to caller
              cmd_desc.byte_count = byte_count;   // However much data was requested

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &SENSE_command, 6);

              // Set the requested byte count in the CDB
              cmd_desc.cmd[4] = (uchar) byte_count;
              cmd_desc.cmd[3] = (uchar) (byte_count >> 8);

              break;
             }
        case DEVICE_Inquiry:
             {
              cmd_desc.cntl.read = 1;             // Getting Inquiry data from device
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Return data to caller
              cmd_desc.byte_count = byte_count;   // However much data was requested

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &INQUIRY_command, 6);

              // Set the requested byte count in the CDB
              cmd_desc.cmd[4] = (uchar) byte_count;
              break;
             }
        case START_motor_immed:
             {
//            if(device_ptr->status.Rem_Media) return (OK);

              cmd_desc.cntl.read = 1;             // Really a don't care for this command
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Another don't care - no data involved
              cmd_desc.byte_count = 0,

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &START_command, 6);
              cmd_desc.cmd[1] |= 0x01;            // Set the CDB immediate bit
              break;
             }
        case START_motor:
             {
//            if(device_ptr->status.Rem_Media) return (OK);

              cmd_desc.cntl.read = 1;             // Really a don't care for this command
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Another don't care - no data involved
              cmd_desc.byte_count = 0;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &START_command, 6);
              break;
             }
        case READ_CAPacity:
             {
              cmd_desc.cntl.read = 1;             // Getting max lba and block size from device
              cmd_desc.len = 10;                  // This is a 10 byte command
              cmd_desc.data_buffer = data_buffer; // Return data to caller
              cmd_desc.byte_count = byte_count;   // However much data was requested

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &READ_CAP_command, 10);
              break;
             }
        case READ_scsi_data:
             {
              cmd_desc.cntl.read = 1;             // Getting data from device
              cmd_desc.len = 10;                  // This is a 10 byte command
              cmd_desc.data_buffer = data_buffer; // Return data to caller

              // Calculate byte count for transfer
              cmd_desc.byte_count = (lba_count * device_ptr->block_size) & 0x00ffffff;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &READ_command, 10);

              /* patch the starting LBA and LBA count fields of the command */

              cmd_ptr->lba_l_field = (ushort)(lba & 0x0000ffff);
              cmd_ptr->lba_h_field = (ushort)(lba >> 16);
              cmd_ptr->lba_l_cnt_field = (uchar)(lba_count & 0xff);
              cmd_ptr->lba_h_cnt_field = (uchar)(lba_count >> 8);

              break;
             }
        case WRITE_scsi_data:
             {
              cmd_desc.cntl.read = 0;             // Sending data to device
              cmd_desc.len = 10;                  // This is a 10 byte command
              cmd_desc.data_buffer = data_buffer; // Get data from caller

              // Calculate byte count for transfer
              cmd_desc.byte_count = (lba_count * device_ptr->block_size) & 0x00ffffff;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &WRITE_command, 10);

              /* patch the starting LBA and LBA count fields of the command */

              cmd_ptr->lba_l_field = (ushort)(lba & 0x0000ffff);
              cmd_ptr->lba_h_field = (ushort)(lba >> 16);
              cmd_ptr->lba_l_cnt_field = (uchar)(lba_count & 0xff);
              cmd_ptr->lba_h_cnt_field = (uchar)(lba_count >> 8);

              break;
             }
        case LOCK:
             {
              cmd_desc.cntl.read = 1;             // Really a don't care for this command
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Another don't care - no data involved
              cmd_desc.byte_count = 0;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &LOCK_command, 6);
              cmd_desc.cmd[4] |= 0x01;            // Set 'Prevent' bit in CDB
              break;
             }
        case UNLOCK:
             {
              cmd_desc.cntl.read = 1;             // Really a don't care for this command
              cmd_desc.len = 6;                   // This is a 6 byte command
              cmd_desc.data_buffer = data_buffer; // Another don't care - no data involved
              cmd_desc.byte_count = 0;

              // Build the command in the descriptor
              (void) memcpy(&cmd_desc.cmd, &LOCK_command, 6);
              break;
             }
        default:
             {
              ret = INVALID_COMMAND;
             }
       }

     if(!ret)  // Current command is valid
       {
        if(driver_op == SEND) hf_led_on();
        busy_timer = get_scsi_timeout(10 * 1000);  // Start the 10 second "Busy" timer
        do
          {
           #if NCR_SUPPORTED
           if(adapter_ptr->handle.type == NCR)
             ret = ncr_cmd( adapter_ptr,
                            device_ptr,
                            driver_op,
                            driver_data_buffer,
                            byte_count,
                            retry_count,
                            timeout );
           #endif
           #if KEY_SUPPORTED
           if(adapter_ptr->handle.type == Key_West)
             ret = key_cmd( adapter_ptr,
                            device_ptr,
                            driver_op,
                            driver_data_buffer,
                            byte_count,
                            retry_count,
                            timeout );
           #endif

           if(ret == BUSY) scsi_delay(500);  // Delay 1/2 second before bothering device again

          } while ( (ret == BUSY) && (! check_scsi_timeout(busy_timer)) );

        if( (ret == RESET_NEEDED) && (op != BUS_RESET) )
            rc = scsi_cmd ( adapter_ptr,       //
                            device_ptr,        //  Direct the adapter to reset the
                            BUS_RESET,         //  Bus the current device is attached
                            0,                 //  to.
                            0,                 //
                            0,                 //
                            0,                 //  No other parameters are required
                            0,                 //  for the bus reset command
                            0 );               //

        if ( (ret == SENSE_NEEDED) &&
             (op != REQUEST_sense)
           )

          {
            rc = scsi_cmd ( adapter_ptr,       //
                            device_ptr,        //  Get the sense data from the device
                            REQUEST_sense,     //  and try to determine why the command
                            (uchar *)&sense,   //  failed
                            NUM_SENSE_BYTES,   //
                            0,                 //
                            0,                 //
                            2,                 //
                            10 );              //

            // Request sense data notes
            //  The error code is defined as the first byte of the returned data (sans bit 7).
            //  SCSI-1 defines codes of 0x00 -> 0x6F to be non-extended vendor specific errors.
            //  SCSI-2 defines the codes of 0x70 and 0x71 to be normal extended sense and
            //  deferred errors respectively.  This code will translate any error code other
            //  than 0x70 or 0x71 simply to CMD_FAILED.
            //  If a short record exception is detected, it is assumed that all residual bytes
            //  of the sense data are 0x00.

            if( rc & SHORT_RECORD )
              {
               sense_ptr = (uchar *) &sense;
               rc -= SHORT_RECORD;
               // fill the bytes NOT returned with 0x00
               for (index=NUM_SENSE_BYTES-rc;index<NUM_SENSE_BYTES ;index++ ) sense_ptr[index] = 0;
               rc=0;  // indicate Request Sense command was actually successful
              }

            sense.error &= 0x7f;   // strip off the information field valid bit
            if ( rc ||
                 ( (sense.error != 0x70) && (sense.error != 0x71) )
               )
              {
                * ( (uint *)&sense.error ) = 0;
                * ( (ushort *)&sense.code ) = 0;
              }

#if SCSI_DEBUG
            if (sdf
#ifdef trace
                && !stf
#endif
               )
              {
               printf("\nSense data : ");
               sense_ptr = (uchar *)&sense;
               for (index=0 ; index<NUM_SENSE_BYTES ;index++) printf("%02X ",sense_ptr[index]);
               printf("\n");
              }
#endif

            ret = keys[sense.key & 0x0f];

            if( ret == UNIT_ATTENTION ) // If Unit Attention condition ...
              {
               attn_retry++; // Increment counter
              }
            else
              {
               attn_retry = 0; // otherwise clear the counter
              }

//          if( (ret == UNIT_ATTENTION) &&     // If Unit Attention condition
//              (!device_ptr->status.Ready) && //    on Not Ready device
//              (attn_retry <= 2)              //    and I haven't done 2 retries yet
//            )
//
//             { attn_retry++; }
//          else
//             { attn_retry = 0; }             // otherwise, clear the retry indicator/counter

            for(index=0;index<(sizeof(ascs)/sizeof(XLAT_ASC));index++)
              if(ascs[index].asc == sense.code)
                {
                 ret = ascs[index].rc;
                 break;
                }
          }  // End of Sense needed & cmd != Req. Sense
        else
          {
           if(op != REQUEST_sense)
             // Command either passed or failed with other than Check Condition
             attn_retry = 0;  // Reset the Unit Attention retry counter
          }

#if SCSI_DEBUG
        if(ret && sdf) printf("\nscsi_cmd complete ret = %d (0x%X)",ret,ret);
#endif

       } // End of valid command section

      // Interrogate conditions to determine if a retry is appropriate
      retry_flag = 0;  // initially assume not
      if ( ret )
        {
         if( attn_retry &&                   // Check for Unit Attention on a not ready
             (op != REQUEST_sense) &&        // device and allow a couple of retries
             (!device_ptr->status.Ready) &&  // for this condition
             (attn_retry < 2)                //
           )
           retry_flag = TRUE;

         if( // (ret == CMD_FAILED) ||       // Device driver retries simple cmd failure
             (ret == DEVICE_NOT_AVAILABLE)   // Retry Selection Timeouts here
           )
           retry_flag = TRUE;
        }

     // Re-establish negotiations on next device command if a Reset occurred
     if ((ret==DEVICE_RESET) || (ret==RESET_NEEDED)) device_ptr->status.Negotiate = 1;

    } while ( retry_flag && retry_count-- );

  // Check to see if either the Ready or Cap_Valid device status flags need to be cleared

  if(ret)
    for(index = 0; index < ( sizeof(stat_mod)/sizeof(STAT) ); index++)
       if(ret == stat_mod[index].error)
         {
          device_ptr->status.Ready = stat_mod[index].flag.Ready;
          device_ptr->status.Cap_Valid = stat_mod[index].flag.Cap_Valid;
          break;
         };

  hf_led_off();
  return (ret);
}

int xlat_sense(void)
{
 int i,ret;

   ret = keys[sense.key & 0x0f];

   for(i=0;i<(sizeof(ascs)/sizeof(XLAT_ASC));i++)
     if(ascs[i].asc == sense.code)
       {
        ret = ascs[i].rc;
        break;
       }

   return (ret);
}
#endif /*DEBUG*/
