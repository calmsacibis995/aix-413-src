static char sccsid[] = "@(#)34	1.3  src/rspc/usr/lib/boot/softros/roslib/scsi_ncr.c, rspc_softros, rspc41J, 9519A_all 5/3/95 11:02:43";
#ifdef DEBUG

#ifdef LISTON
 #pragma options source flag=I:I
#endif

/*
 *   COMPONENT_NAME: rspc_softros
 *
 * FUNCTIONS: procname1, procname2, ...
 *
 *   ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
                                                                            */
//#pragma options nosource
//--------------------------------------------------------------------------//
// External global definitions                                              //
//--------------------------------------------------------------------------//

#define SCSI_DEBUG 1
#define PCI_BASE_ADDR   0x80000000
#define PCI_CONFIG_ADDR 0x80881000
#define PCI2_CONFIG_DEV	0x0c
#define PCI2_CONFIG_BUS	0x00

#define CHECKPOINT(y)      if(sdf)printf("<<%02X>>",y);
//#define CHECKPOINT(y)

#include <sys/types.h>
#include <string.h>
#include <io_sub.h>
#if USE_POLLING
#else
#include <int_hndlr.h>
#endif
#include <rtcodes.h>
#include <scsi2.h>

//--------------------------//
// include the NCR scripts  //
//--------------------------//

// include the NCR specific script information
#include <ncrsdef.h>
#include <ncrscr.h>
   int SCRIPT_SIG = 0x01; // becomes a bigger number when byte reversed

//--------------------------------------------------------------------------//
// Declarations of external functions & reltd #defines used by this file    //
//--------------------------------------------------------------------------//

#include <scsi_drvr.h>

#if !SCSI_DEBUG
 #ifdef trace
   #error Script data contains TRACE instructions
   #error   Code in scsi_ncr.c not being generated to handle them
   #error   Corrective action :
   #error     Change SCSI_DEBUG flag in scsi_drvr.h
   #error     or use version of Scripts without trace instructions (ncr810.ss)
 #endif
#endif

#include <misc_sub.h>
#include <scsi_util.h>
#include <scsi_externals.h>
#include <scsi_public.h>

#include <misc_asm.h>
#include <model.h>


//--------------------------------------------------------------------------//
// External declarations of functions defined in this file & reltd #defines //
//--------------------------------------------------------------------------//

#include <scsi_ncr.h>

#ifdef LISTON
 #pragma options source
#endif

#define IO_READ_8(x)     in8(io_address + x)
#define IO_READ_32(x)    in32(io_address + x)
#define IO_WRITE_8(x,y)  out8(io_address + x, y)
#define IO_WRITE_32(x,y) out32(io_address + x, y)


typedef union _sregs { unsigned char reg[0x60];
                       unsigned int  dreg[0x60/4];
                     } sregs;

static struct ncr_info { unsigned int *scsi_script_ptr;
                         unsigned int *scsi_script_data_ptr;
                         unsigned char scsi_script_type;
                                  int int_flag;
                         struct { int modified : 1;
                                  int cmd_buf : 1;
                                  int data_buf : 1;
                                  int identify_buf : 1;
                                  int msg_in_buf : 1;
                                  int status_byte : 1;
                                  int sync_msg_buf : 1;
                                } scrmods;
                         volatile sregs  ncr_regs;
                       } NCR_INFO;

#define NCR_REG(x) (NCR_INFO.ncr_regs.reg[(x)])
#define NCR_DREG(x) (NCR_INFO.ncr_regs.dreg[(x/4)])
#define INT_FLAG (NCR_INFO.int_flag)

static unsigned int data_expected;
static unsigned int data_xfered;        // accumulator for all data transferred
static unsigned int address_conversion; // Bus master address conversion factor
static unsigned int io_address;         // Base I/O address established on OPEN commands
unsigned char xfer_length;              // SCSI Burst length 2/4/8/16 transfers

  // Definitions of Saved and Current Data Pointer fields for support of disconnect messages

static uint current_data_ptr;      // current pointer to data area
static uint current_data_byte_cnt; // current byte count
static uint saved_data_ptr;      // Saved pointer to data area
static uint saved_data_byte_cnt; // Saved byte count
#if SCSI_DEBUG
static uchar *original_data_ptr;
#endif

//---------------------------------------------------------------------------//
// Miscellaneous device notes :                                              //
//   * 'IBM     WDS-3200'                                                    //
//        This hardfile responds with only the required 5 bytes of inquiry   //
//        data when addressing invalid (unsupported) LUN addresses.          //
//   * 'IOMEGA  BETA90'                                                      //
//        This Bernoulli box only returns 1 byte of inquiry when addressing  //
//        invalid (unsupported) LUN addresses.  It also msg rejects any      //
//        multiple-byte extended messages (like the attempt to establish     //
//        synchronous negotiations.  The drive was borrowed from SunSoft     //
//        Inc. Contact ; William P. Taylor (Chris Dotson ?) (310)-348-8605   //
//        or Michelle A. Diaz (443-9938) at IBM (was on fax).  Also Bill     //
//        Taylor (310)-348-6053 at SunSoft.                                  //
//---------------------------------------------------------------------------//
static unsigned char last_reenter_phase;

//--------------------------//
// Data area for scripts    //
//--------------------------//

#define TRANSFER_PERIOD  0x32
#define SYNC_OFFSET 0x00

static uchar             identify_msg_buf[] = { 0xc0,0x01,0x03,0x01,TRANSFER_PERIOD,SYNC_OFFSET };
static uchar             status_byte;
static uchar             msg_in_buf;
static uchar             sync_msg_in_buf;
static uchar             sync_msg_resp[4];
//static ADAPTER_INFO_REC *current_adapter_ptr;
//static DEV_INFO_REC     *current_device_ptr;
static rtc_value         bus_reset_delay;     // 3 second delay counter following bus reset
static uint              bus_reset = 0;       // Flag to indicate bus reset occurred
static DEV_INFO_REC      device;              // Dummy device information record for testing


//--------------------------------------------------------------------------//
// Local Declarations for functions used only by functions in this file     //
//--------------------------------------------------------------------------//

#if SCSI_DEBUG
//unsigned char PCI_read_8(unsigned int addr);
//void PCI_write_8(unsigned int addr,unsigned char data);
//unsigned int PCI_read_32(unsigned int addr);
//void PCI_write_32(unsigned int addr,unsigned int data);
//void full_cache_flush(void);
// The following are for debugging purposes
void print_regs( void );
void print_cmd(void);
void print_script(void);
void print_stat(void);
#endif

void Reset_SCSI_Bus(void);

int ncr_adapter_open(ADAPTER_INFO_PTR adapter_ptr);

int ncr_execute( ADAPTER_INFO_PTR adapter_ptr,
                 DEVICE_INFO_PTR  device_ptr,
                 unsigned int     retry_count,     /* retry_count                */
                 unsigned int     int_timeout_s);  /* timeout in seconds         */

void ncr_fun_setup( ADAPTER_INFO_PTR adapter_ptr,
                    DEVICE_INFO_PTR  device_ptr,
                    CMD_OPS          op,           // op                         */
                    CMD_DESC         *cmd_ptr);    // command descriptor pointer */

int controller_diags(ADAPTER_INFO_PTR adapter_ptr);
#if USE_POLLING
#else
void SCSI_int_hndlr(void);
#endif
void ncr_default_setup(ADAPTER_INFO_PTR adapter_ptr);
void ncr_soft_reset(void);
int get_script_and_patch(void);
unsigned int ncr_cleanup_script(void);

//--------------------------------------------------------------------------//
// SCSI I/O Registers                                                       //
//                                                                          //
// All of these registers are implemented in the 53C700                     //
//                                                                          //
//--------------------------------------------------------------------------//

#define  SIOP_REGS        0x00 // 0x80
#define  TOLERANT_ENABLE  0x80 // Enable active negation of Req, Ack, Data, and Parity lines

#if    ( (NCR_CHIP == 720) || (NCR_CHIP == 810) )

//--------------------------------------------------------------------------//
// Register implementation for the 53C720 and 53C810                        //
//--------------------------------------------------------------------------//
//                              comments                 NCR 700 comp       //

  #define  SCNTL0              0x00           // ok U SCNTL0 0x00
  #define  SCNTL1              0x01           // ESR U SCNTL1 0x01
  #define  SCNTL2              0x02           // U SDID 0x02
  #define  SCNTL3              0x03           // - SIEN 0x03
  #define  SCID                0x04           // NOT COMPAT U SCID 0x04
  #define  SXFER               0x05           // Probably ok U SXFER 0x05
  #define  SDID                0x06           // NOT COMPAT - SODL 0x06
  #define  GPREG               0x07           // - SOCL 0x07
  #define  SFBR                0x08           // - SFBR 0x08
  #define  SOCL                0x09           // - SIDL 0x09
  #define  SSID                0x0a           // - SBDL 0x0a
  #define  SBCL                0x0b           // - SBCL 0x0b
  #define  DSTAT               0x0c           // read only U DSTAT 0x0c
  #define  SSTAT0              0x0d           // U SSTAT0 0x0d
  #define  SSTAT1              0x0e           // - SSTAT1 0x0e
  #define  SSTAT2              0x0f           // - SSTAT2 0x0f
  #define  DSA                 0x10
  #define  ISTAT               0x14           // - CTEST0 0x14

//                                               -   CTEST1       0x15  */
//                                               -   CTEST2       0x16  */
//                                               -   CTEST3       0x17  */

  #define  CTEST0              0x18           // - CTEST4 0x18
  #define  CTEST1              0x19           // - CTEST5 0x19
  #define  CTEST2              0x1a           // - CTEST6 0x1a
  #define  CTEST3              0x1b           // - CTEST7 0x1b
  #define  TEMP                0x1c           // - TEMP 0x1c
  #define  DFIFO               0x20           // U DFIFO 0x20
  #define  CTEST4              0x21           // U ISTAT 0x21
  #define  CTEST5              0x22
  #define  CTEST6              0x23
  #define  DBC                 0x24           // U DBC 0x24
  #define  DCMD                0x27           // - DCMD 0x27
  #define  DNAD                0x28           // - DNAD 0x28
  #define  DSP                 0x2c           // U DSP 0x2c
  #define  DSPS                0x30           // U DSPS 0x30
  #define  SCRATCHA            0x34           // U DMODE 0x34
  #define  DMODE               0x38
  #define  DIEN                0x39           // U DIEN 0x39
  #define  DWT                 0x3a           // - DWT 0x3a
  #define  DCNTL               0x3b           // U DCNTL 0x3b
  #define  ADDER               0x3c           // check name U SCSI_RSR 0x3c
  #define  SIEN0               0x40
  #define  SIEN1               0x41
  #define  SIST0               0x42
  #define  SIST1               0x43
  #define  SLPAR               0x44
#if    (NCR_CHIP == 720)
  #define  SWIDE               0x45
#endif
  #define  MACNTL              0x46
  #define  GPCNTL              0x47
  #define  STIME0              0x48
  #define  STIME1              0x49
  #define  RESPID0             0x4a
#if    (NCR_CHIP == 720)
  #define  RESPID1             0x4b
#endif
  #define  STEST0              0x4c
  #define  STEST1              0x4d
  #define  STEST2              0x4e
  #define  STEST3              0x4f
  #define  SIDL                0x50
  #define  SODL                0x54
  #define  SBDL                0x58
  #define  SCRATCHB            0x5c
  #define  DIEN_MASK           0x15
#endif

//--------------------------------------------------------------------------//

#define  S_SCNTL0            (SIOP_REGS + SCNTL0 )
#define  S_SCNTL1            (SIOP_REGS + SCNTL1 )
#define  S_SDID              (SIOP_REGS + SDID )
#define  S_SIEN_MASK         (SIOP_REGS + SIEN_MASK )
#define  S_SCID              (SIOP_REGS + SCID )
#define  S_SXFER             (SIOP_REGS + SXFER )
#define  S_SODL              (SIOP_REGS + SODL )
#define  S_SOCL              (SIOP_REGS + SOCL )
#define  S_SSID              (SIOP_REGS + SSID )
#define  S_SFBR              (SIOP_REGS + SFBR )
#define  S_SIDL              (SIOP_REGS + SIDL )
#define  S_SBDL              (SIOP_REGS + SBDL )
#define  S_SBCL              (SIOP_REGS + SBCL )
#define  S_DSTAT             (SIOP_REGS + DSTAT )
#define  S_SSTAT0            (SIOP_REGS + SSTAT0 )
#define  S_SSTAT1            (SIOP_REGS + SSTAT1 )
#define  S_SSTAT2            (SIOP_REGS + SSTAT2 )
#define  S_CTEST0            (SIOP_REGS + CTEST0 )
#define  S_CTEST1            (SIOP_REGS + CTEST1 )
#define  S_CTEST2            (SIOP_REGS + CTEST2 )
#define  S_CTEST3            (SIOP_REGS + CTEST3 )
#define  S_CTEST4            (SIOP_REGS + CTEST4 )
#define  S_CTEST5            (SIOP_REGS + CTEST5 )
#define  S_CTEST6            (SIOP_REGS + CTEST6 )
#define  S_CTEST7            (SIOP_REGS + CTEST7 )
#define  S_TEMP              (SIOP_REGS + TEMP )
#define  S_DFIFO             (SIOP_REGS + DFIFO )
#define  S_ISTAT             (SIOP_REGS + ISTAT )
#define  S_DBC               (SIOP_REGS + DBC )
#define  S_DCMD              (SIOP_REGS + DCMD )
#define  S_DNAD              (SIOP_REGS + DNAD )
#define  S_DSP               (SIOP_REGS + DSP )
#define  S_DSPS              (SIOP_REGS + DSPS )
#define  S_DMODE             (SIOP_REGS + DMODE )
#define  S_DIEN              (SIOP_REGS + DIEN )
#define  S_DWT               (SIOP_REGS + DWT )
#define  S_DCNTL             (SIOP_REGS + DCNTL )
#define  S_SCSI_RSR          (SIOP_REGS + SCSI_RSR )


#if    ( (NCR_CHIP==720) || (NCR_CHIP==810) )
#define  S_SCNTL2            (SIOP_REGS + SCNTL2 )
#define  S_SCNTL3            (SIOP_REGS + SCNTL3 )
  #define  S_SIEN0             (SIOP_REGS + SIEN0 )
  #define  S_SIEN1             (SIOP_REGS + SIEN1 )
  #define  S_SIST0             (SIOP_REGS + SIST0 )
  #define  S_SIST1             (SIOP_REGS + SIST1 )
  #define  S_STEST3            (SIOP_REGS + STEST3)
  #define  S_MACNTL            (SIOP_REGS + MACNTL)
  #define  S_GPCNTL            (SIOP_REGS + GPCNTL)
  #define  S_RESPID0           (SIOP_REGS + RESPID0)
  #define  S_SCRATCHA          (SIOP_REGS + SCRATCHA)
#endif

//--------------------------------------------------------------------------//

#define  ISTAT_DIP           0x01
#define  ISTAT_SIP           0x02
#define  ISTAT_INTF          0x04             // interrupt on the fly bit
#define  SCNTL1_RST          0x08
//---- DSTAT bit definitions ------//
#define  DMA_FIFO_EMPTY      0x80            // FIFO is empty
#define  MASTER_DATA_PARITY  0x40            // Target signalled parity error - PCI bus
#define  BUS_FAULT           0x20            // PCI fault - Target Abort ?
#define  CMD_ABORT           0x10            // S.W. initiated cmd abort
#define  SINGLE_STEP         0x08            // Scripts single step interrupt
#define  DSTAT_SIR           0x04            // Script interrupt instruction
#define  ILLEGAL_OP          0x01            // Illegal SCRIPTS instruction

#define  SLIH_STOP           0x01
#define  DRIVE_STOP          0x02
#define  DATA_STOP           0x03
#define  PATCH_SCRIPT        0x04
#define  SCSI_RST            0x02
#define  SCSI_UNX_DISC       0x04
#define  SCSI_SEL_FAIL       0x20
#define  PHASE_MIS           0x80
#define  SCSI_RSC_FAIL       (0x0100)
#define  NCR_FAIL            (0x0200)
#define  SCSI_FUSE           (0x0001)
#define  SCSI_PTC            (0x0002)

//--------------------------------------------------------------------------//
// The following are scsi_post return codes generated by scsi_post code.    //
// Other codes (from rtcodes.h) may also be returned.                       //
//--------------------------------------------------------------------------//
//- fuse and thermal device failures ---------------------------------------//
//--------------------------------------------------------------------------//

#define  SCSI_RSC_FAIL_FUSE  (SCSI_RSC_FAIL + SCSI_FUSE)
#define  SCSI_RSC_FAIL_PTC   (SCSI_RSC_FAIL + SCSI_PTC )

//--------------------------------------------------------------------------//
//- Failures due to NCR chip register errors -------------------------------//
//--------------------------------------------------------------------------//

#define  NCR_FAIL_SCNTL0     (NCR_FAIL + SCNTL0 )
#define  NCR_FAIL_SCNTL1     (NCR_FAIL + SCNTL1 )
#define  NCR_FAIL_SDID       (NCR_FAIL + SDID )
#define  NCR_FAIL_SIEN       (NCR_FAIL + SIEN )
#define  NCR_FAIL_SCID       (NCR_FAIL + SCID )
#define  NCR_FAIL_SXFER      (NCR_FAIL + SXFER )
#define  NCR_FAIL_SODL       (NCR_FAIL + SODL )
#define  NCR_FAIL_SOCL       (NCR_FAIL + SOCL )
#define  NCR_FAIL_SFBR       (NCR_FAIL + SFBR )
#define  NCR_FAIL_SIDL       (NCR_FAIL + SIDL )
#define  NCR_FAIL_SBDL       (NCR_FAIL + SBDL )
#define  NCR_FAIL_SBCL       (NCR_FAIL + SBCL )
#define  NCR_FAIL_DSTAT      (NCR_FAIL + DSTAT )
#define  NCR_FAIL_SSTAT0     (NCR_FAIL + SSTAT0 )
#define  NCR_FAIL_SSTAT1     (NCR_FAIL + SSTAT1 )
#define  NCR_FAIL_SSTAT2     (NCR_FAIL + SSTAT2 )
#define  NCR_FAIL_DFIFO      (NCR_FAIL + DFIFO )
#define  NCR_FAIL_ISTAT      (NCR_FAIL + ISTAT )
#define  NCR_FAIL_DBC        (NCR_FAIL + DBC )
#define  NCR_FAIL_DCMD       (NCR_FAIL + DCMD )
#define  NCR_FAIL_DNAD       (NCR_FAIL + DNAD )
#define  NCR_FAIL_DSP        (NCR_FAIL + DSP )
#define  NCR_FAIL_DSPS       (NCR_FAIL + DSPS )
#define  NCR_FAIL_DIEN       (NCR_FAIL + DIEN )
#define  NCR_FAIL_DWT        (NCR_FAIL + DWT )
#define MAX_DEV_FUNC 36

#ifdef LISTON
 #pragma options source list
#endif
//--------------------------------------------------------------------------//
// Function: ncr_cmd()                                                      //
//                                                                          //
// Action:   Receive all commands for this device driver.                   //
//--------------------------------------------------------------------------//

int ncr_cmd(ADAPTER_INFO_PTR adapter_ptr,     // Adapter vitals pointer
            DEVICE_INFO_PTR  device_ptr,      // Device vitals pointer
            CMD_OPS          op,              // command to be performed
            unsigned char   *data_ptr,        // Data buffer or command descriptor
            unsigned int     byte_cnt,        // Data transfer count, ignored if
                                              // data_ptr is a command descriptor
            unsigned int     retry_count,     // To do, or not to do
            unsigned int     int_timeout_s)   // and how long to do it
{
  int                ret;
  int                adapter_id;
  uint               retries;
#if USE_POLLING
#else
  void      (* int_hndlr_ptr)();
#endif

  retries = 0;

//current_adapter_ptr = adapter_ptr;   // Initialize global pointers to data structures
//current_device_ptr  = device_ptr;    // for use by the interrupt handler function

  switch (op)
   {
      case Adapter_Dsize:
           {

            ret = 0;  // No 'private' data area requirements at this time
            break;
           }
      case Inquiry:
           {
            unsigned char *ncr_tag = "NCR";
            unsigned char *ncr_prod = "53C810";
            unsigned char *temp;

            // Blank out the return field, then set Vendor, Product and revision fields
            (void) memset(data_ptr, ' ', 24);
            (void) memcpy(data_ptr, ncr_tag, 3);
            (void) memcpy(data_ptr+8, ncr_prod, 6);
	    temp = (uchar *)itoa(pci_cfg_read_8(adapter_ptr->handle,0x08));
            (void) memcpy(data_ptr+24, temp, 4);
            ret = OK;
            break;
           }
      case OPEN:
           {
            last_reenter_phase = 0xff;
#if USE_POLLING
#else
            int_hndlr_ptr = SCSI_int_hndlr;
            ext_int_registrar (ASSIGN_INTERRUPT_HANDLER_ADDRESS, 13, (uint *)int_hndlr_ptr);
#endif
            ret = ncr_adapter_open(adapter_ptr);
            break;
           }
      case CLOSE:
           {
            // Disable all interrupts at the controller
            #if ( (NCR_CHIP==720) || (NCR_CHIP==810) )
            IO_WRITE_8(S_SIEN0,(unsigned char)0);
            IO_WRITE_8(S_SIEN1,(unsigned char)0);
            #endif
            IO_WRITE_8(S_DIEN,(unsigned char)0);
#if USE_POLLING
#else
            // ... and release the interrupt vector
            ext_int_registrar (RELEASE_INTERRUPT_HANDLER_ADDRESS, 13, int_hndlr_ptr);
#endif
            ret = OK;
            break;
           }
      case BUS_RESET:
           {
            Reset_SCSI_Bus();
            ret = OK;
            break;
           }
      case DIAGnose:    // Adapter POST (self test) request
           {
            ret = controller_diags(adapter_ptr);
            break;
           }
      case SEND:
           {
            // note: 6/29/93 - noticed that if a command was re-started in ncr_execute
            // the byte count and target address are modified in the script.  If the
            // command then fails (CMD_FAILED code) and the retry count had not expired
            // the script would be restarted again from the top.  If the retry then
            // works, only partial data has been transferred (probably) but the
            // external indications are that the command completed successfully.
            // Because of this exposure, ncr_execute is now called with a retry count
            // of 0 so that ncr_fun_setup can re-establish the original transfer
            // count and address information.

            IO_WRITE_8(S_DMODE, (IO_READ_8(S_DMODE) & 0x3f) | (xfer_length & 0xC0));
//          flush_it = (op==MEM_TO_MEM) ? 0 : 1;
            do
             {
               ncr_fun_setup(
                   adapter_ptr,
                   device_ptr,
                   op,                              // op
                   (CMD_DESC *)data_ptr);           // data ptr

               ret=ncr_execute(
                       adapter_ptr,
                       device_ptr,
                       0,                               // retry_count
                       int_timeout_s);                  // timeout in ms

             } while ( (ret==CMD_FAILED) &&
                       ( (++retries) < retry_count) );
            break;
           }
      default:
           {
            ret = INVALID_COMMAND;
            break;
           }
   }
#if SCSI_DEBUG
  if(ret && sdf) printf("\nncr_cmd ret = %d",ret);
#endif
  return (ret);
}


//--------------------------------------------------------------------------//
// Function: ncr_adapter_open()                                             //
//                                                                          //
// Action:   Initializes the adapter                                        //
//                                                                          //
//                                                                          //
// Returns:                                                                 //
//                                                                          //
//--------------------------------------------------------------------------//

int ncr_adapter_open(ADAPTER_INFO_PTR adapter_ptr)

{
  int                ret;
  int            retries;
  ADAPTER_HANDLE handle;
  DEVICE_INFO_PTR device_ptr = &device; // Pointer to dummy device info record for testing
  uchar dmy;
  int scsi_dev_id;
  int base_save;
  int input;
  uint invalue;
  int i;

  // The following command descriptor specifies a 6 byte Inquiry command
  // with no data to be transferred to address 0
  CMD_DESC self_sel = { 0, 6, 0, 0, { 0x12,0x0,0x0,0x0,0x00,0x0 } };

  // There is one unique command (SCRIPT_test) to the NCR controller test
  // that is used during the adapter open commnad.  This is a local command only.

#define SCRIPT_test 0xffffffff

  if( adapter_ptr->status.Locate )               // If this is a 'Locate' function call
   {
    if(adapter_ptr->handle.sys_bus != PCI_Bus)   //  for anything other than the PCI Bus
      {
       return (NO_SCSI_CARD);                    //  indicate nothing found
      }
//  if( request != the FIRST instance )  // If not looking for the FIRST instance
//    {
//     return (NO_SCSI_CARD);            // indicate nothing found
//    }
    // initialize my temporary adapter handle
    handle.type    = NCR;
    handle.sys_bus = PCI_Bus;
    if (pci_type == 0) {
	/*
	 * PCI 1.0 configuration
	 */
	handle.sys_id.PCI_Addr.Device = (uchar) (PCI_CONFIG_ADDR >> 8);
	handle.sys_id.PCI_Addr.Bus    = (uchar) (PCI_CONFIG_ADDR >> 16);
    } else {
	/*
	 *  PCI 2.0 configuration
	 */
extern struct ipldevs {
                unsigned char dev_func;
                unsigned char bus_number;
                } IPLDEVS[MAX_DEV_FUNC];

extern int numipldevs;
extern uchar scsi_dev_func;
extern uchar scsi_bus_number;
        if (numipldevs != 1)
	{ 
retry:
	    printf("Are the scsi_bus_num and scsi_dev_func values OK? [y/n]");
	    if (getchar() == 'n')
	    {
		printf("\nEnter the ID from the following list:\n");
		if (numipldevs < 10)
		{
		    for (i=0; i < numipldevs; i++)
		    {
			printf("%d-bus = %2X, dev_func = %2X\n", i, 
				IPLDEVS[i].bus_number, IPLDEVS[i].dev_func);
		    }
		}
		else
		{
		    for (i=0; i < 10; i++)
		    {
			printf("%d-bus = %2X, dev_func = %2X\n", i, 
				IPLDEVS[i].bus_number, IPLDEVS[i].dev_func);
		    }
		    for (i=10; i < numipldevs; i++)
		    {
			printf("%c-bus = %2X, dev_func = %2X\n", i+0x57, 
				IPLDEVS[i].bus_number, IPLDEVS[i].dev_func);
		    }
		}
		
		input = getchar();
		if ((input >= 0x30) && (input <= 0x39))
			input = input & 0xf;
		else if ((input >= 0x41) && (input <= 0x5a))
			input = input - 0x37;
		else if ((input >= 0x61) && (input <= 0x7a))
			input = input - 0x57;
		else
		{
			printf("\nInvalid input.  Retry.\n");
			goto retry;
		}	
		if (input >= numipldevs)
		{
			printf("\nInvalid input.  Retry.\n");
			goto retry;
		}	
		scsi_dev_func = IPLDEVS[input].dev_func;
		scsi_bus_number = IPLDEVS[input].bus_number;
	    }
	}

	handle.sys_id.PCI_Addr.Device =  scsi_dev_func >> 3;
	handle.sys_id.PCI_Addr.Bus    =  scsi_bus_number;
    }
printf("\nThe SCSI DEV_FUNC = 0x%x. The SCSI BUS = 0x%x\n",scsi_dev_func, scsi_bus_number);
    scsi_dev_id = pci_cfg_read_32(handle,0x00);  // Check for the NCR 53C810 IDs
    if ((0x00011000 != scsi_dev_id) && (0x00031000 != scsi_dev_id)) // the NCR chips
       {
        return (NO_SCSI_CARD);    // Indicate controller not found
       }

    printf("\nNCR 53C8XX revision %02X",pci_cfg_read_8(handle,8));

    base_save = (pci_cfg_read_32(handle, 0x10) & 0xffffff00);         // Set I/O address
    printf("\nPCI Base Address = %x\n", base_save);
    pci_cfg_write_16(handle, 0x0c, (LATENCY_TIMER << 8));  //     Latency Timer
    pci_cfg_write_16(handle, 0x04, 0x05);                  // and Command fields (Bus Master
                                                           // (operation and I/O space enabled)

//  printf("\nProgrammed to respond at 601 address 0xA00000xx\n");

    adapter_ptr->Base_IO_Addr = base_save;   // See scsi_ncr.h
    adapter_ptr->handle       = handle;
    adapter_ptr->max_bus      = NCR_MAX_BUS_ID;  // See scsi_ncr.h
    adapter_ptr->max_id       = NCR_MAX_PUN_ID;  // See scsi_ncr.h
    adapter_ptr->max_lun      = NCR_MAX_LUN_ID;  // See scsi_ncr.h

    controller_id(handle, (uchar *)&adapter_ptr->bus_id, 1);  // Go get the ID I'm supposed to be using

    address_conversion = sys_mem_address(handle);    // Get the Master to memory address conversion factor

   }

  io_address = adapter_ptr->Base_IO_Addr;    // Initialize the base I/O address value

  #if (NCR_CHIP==720)
  IO_WRITE_8(DCNTL,0x21);
  #endif

  // Perform a reset on the chip
  ncr_soft_reset();

#if SCSI_DEBUG
  NCR_REG(SCNTL0) = IO_READ_8(SCNTL0);
  #if (NCR_CHIP==810)
  NCR_REG(SCNTL0) &= 0xFB;
  #endif
  if (NCR_REG(SCNTL0) != 0xC0) printf("\nInit error : SCNTL0 = %02X",NCR_REG(SCNTL0));
#endif

  // Initialize the operating registers
  ncr_default_setup(adapter_ptr);

  //------------------------------------------------------------------------//
  // Set up for usage                                                       //
  //------------------------------------------------------------------------//

  if( adapter_ptr->status.Locate )    // If this is a 'Locate' function call
    {
     get_script_and_patch();

     // Execute the test script to ensure script data swapped correctly

     device.id = (adapter_ptr->bus_id[0] & 0x0f);  // Set target device id for cmd */
     device.lun = 0;                               // to be the same as controller */

     retries=3;
     do { ncr_fun_setup(
              adapter_ptr,
              device_ptr,
              SCRIPT_test,     // op
              (CMD_DESC *)0);     // data ptr

          ret = ncr_execute(
                   adapter_ptr,
                   device_ptr,
                   0,           // retry_count
                   1);          // timeout in seconds

//        if(ret)
//          {
//           printf("\nTest script rc %02X",ret);
//           dmy= getchar();
//          }
        } while (ret && retries--);

#if SCSI_DEBUG
     if(ret == SCRIPT_ERROR)    // Non-Zero return code indicates problem
       {
        if(sdf) printf("\nNCR Script data error");
       }
     else
#else
     if(ret != SCRIPT_ERROR)    // If Script data appears to be converted for little endian operation
#endif
       {
        // Attempt a selection at the card's ID on supported bus(ses) in an
        // attempt to ensure that there is not a device hung on the bus.
        // Command processing will eventually indicate a bus reset is in
        // order if other attempts to gain access to the bus fail.

        retries = 3;
        self_sel.cntl.read = 1;  // indicate DATA IN phase (although one should not occur)
        do {
            ncr_fun_setup(
               adapter_ptr,
               device_ptr,
               SEND,                            /* op                         */
               (CMD_DESC *)&self_sel);          /* command descriptor         */
            ret=ncr_execute(
               adapter_ptr,
               device_ptr,
               0,                               /* retry_count                */
               4);                              /* timeout in seconds         */
           } while ( ( ret && (ret!=RESET_NEEDED) && (ret!=DEVICE_NOT_AVAILABLE) ) &&
                     retries-- );

        if(!ret) ret = DEVICE_ID_CONFLICT;      // Device at same address as controller
        if(ret == RESET_NEEDED) Reset_SCSI_Bus();
        ret = OK;
       }
    }
  else
    {
     ret = OK;  // Return code for normal OPEN command
    }
  return (ret);
}


//--------------------------------------------------------------------------//
// Function: ncr_fun_setup()                                                //
//                                                                          //
// Action:   Sets up the parameters for the command to be issued            //
//                                                                          //
// Notes:                                                                   //
//                                                                          //
//--------------------------------------------------------------------------//

void ncr_fun_setup( ADAPTER_INFO_PTR adapter_ptr,
                    DEVICE_INFO_PTR  device_ptr,
                    CMD_OPS          op,           // op  
                    CMD_DESC         *cmd_ptr)     // command descriptor pointer 
{
  uint               i;
  uchar * mem_ptr;

  switch(op)
    {
     case SCRIPT_test:
          {
           NCR_INFO.scsi_script_ptr= (uint *)Ent_script_test;
           NCR_INFO.scsi_script_type= 'T';            // identify script as the test script
           break;
          };
     case SEND:
          {
           if(device_ptr->status.Negotiate)
             {
              device_ptr->status.Negotiate = 0;       // Reset the negotiate flag
              NCR_INFO.scsi_script_ptr= (uint *)Ent_negotiate_script;
              NCR_INFO.scsi_script_type= 'N';         // identify script as a 'negotiate' script

//            // if/when we go to synchronous transfers, clean up the period and offset values
//            // in case they got changed in negotiations with a previous device
//
//            Actually, the values will have to be set based on information (probably) stored
//            in the device information record.  There will also have to be a new status flag that
//            indicates when negotiations have been completed and the two fields are valid
//
//            identify_msg_buf[4] = TRANSFER_PERIOD;
//            identify_msg_buf[5] = SYNC_OFFSET;
              identify_msg_buf[0] = 0x80;   // Don't allow disconnect during negotiations
             }
           else
             {
              NCR_INFO.scsi_script_ptr= Ent_common_script;
              NCR_INFO.scsi_script_type= 'C';         // identify script as the 'common' portion
              identify_msg_buf[0] = 0xc0;             // Allow disonnect for cmds other than Inquiry
             }

           if(cmd_ptr->cntl.read)     // Check for (and set) proper data phase detection in the Scripts
             {
              if ( (SCRIPT[Ent_data_phase / 4] & 0x07) != DATA_IN_PHASE)
                { SCRIPT[Ent_data_phase / 4]     = (SCRIPT[Ent_data_phase / 4]     & 0xFFFFFFF8) | DATA_IN_PHASE;
                  SCRIPT[Ent_data_xfer_inst / 4] = (SCRIPT[Ent_data_xfer_inst / 4] & 0xFFFFFFF8) | DATA_IN_PHASE;
                  SCRIPT[Ent_data_xfer / 4]      = (SCRIPT[Ent_data_xfer / 4]      & 0xFFFFFFF8) | DATA_IN_PHASE;
                }
             }
           else
             {
              if ( (SCRIPT[Ent_data_phase / 4] & 0x07) != DATA_OUT_PHASE)
                { SCRIPT[Ent_data_phase / 4]     = (SCRIPT[Ent_data_phase / 4]     & 0xFFFFFFF8) | DATA_OUT_PHASE;
                  SCRIPT[Ent_data_xfer_inst / 4] = (SCRIPT[Ent_data_xfer_inst / 4] & 0xFFFFFFF8) | DATA_OUT_PHASE;
                  SCRIPT[Ent_data_xfer / 4]      = (SCRIPT[Ent_data_xfer / 4]      & 0xFFFFFFF8) | DATA_OUT_PHASE;
                }
             }

           data_expected = cmd_ptr->byte_count;   // Set flag for data transfer expected

           // Initialize Current and Saved Data Pointer variables
#if SCSI_DEBUG
           original_data_ptr   = cmd_ptr->data_buffer;
#endif
           saved_data_ptr      = current_data_ptr      = (uint)(address_conversion + cmd_ptr->data_buffer);
           saved_data_byte_cnt = current_data_byte_cnt = data_expected;

           // Patch in the address and byte count of the command to be sent

           for (i=0; i< (sizeof E_cmd_buf_Used) / 4; i++)
             {
              SCRIPT[E_cmd_buf_Used[i]] = swap_word((uint)(address_conversion + cmd_ptr->cmd));
              SCRIPT[E_cmd_buf_Used[i]-1] = (SCRIPT[E_cmd_buf_Used[i]-1]&0xff) | swap_word(cmd_ptr->len);
             }

           // Patch in the data buffer address and byte count

           for (i=0; i< (sizeof E_data_buffer_Used) / 4; i++)
             {
              SCRIPT[E_data_buffer_Used[i]] = swap_word((uint) (address_conversion + cmd_ptr->data_buffer));
              SCRIPT[E_data_buffer_Used[i]-1] = (SCRIPT[E_data_buffer_Used[i]-1] & 0xff) | swap_word(cmd_ptr->byte_count);
             }

           // Patch in the current target ID

           for (i=0; i< (sizeof E_target_id_Used) / 4; i++)
             {
              SCRIPT[E_target_id_Used[i]]= (SCRIPT[E_target_id_Used[i]]&0xfffff8ff) | (device_ptr->id <<8);
             }

           // Patch in the current target ID for possible reselection

           for (i=0; i< (sizeof E_resel_id_Used) / 4; i++)
             {
              SCRIPT[E_resel_id_Used[i]]= (SCRIPT[E_resel_id_Used[i]]&0x00ffffff) | ((device_ptr->id+0x80) << 24);
             }

           // Patch the target's LUN value into the command CDB and identify msg

           cmd_ptr->cmd[1] = (cmd_ptr->cmd[1] & 0x1f) | (uchar) ( (device_ptr->lun & 0x07) << 5 );
           identify_msg_buf[0] = (identify_msg_buf[0] &0xf8) | (device_ptr->lun & 0x07);
           identify_msg_buf[1] = 0x01;

//         print_script();
           break;
          }
    }

}


//--------------------------------------------------------------------------//
// Function: ncr_execute()                                                  //
//                                                                          //
// Action:                                                                  //
//  This function causes previously setup scsi commands to be executed.     //
//  The command completion status will be interrogated and retries will     //
//  be attempted under control of the retry flag that is passed as a parm   //
//  to this routine.                                                        //
//                                                                          //
// RETURNS: see rtcodes.h                                                   //
//                                                                          //
//--------------------------------------------------------------------------//

int ncr_execute( ADAPTER_INFO_PTR adapter_ptr,
                 DEVICE_INFO_PTR device_ptr,
                 unsigned int retry_count,     /* retry_count                */
                 unsigned int int_timeout_s)   /* timeout in seconds         */
{

  int                i,rc=0;
  int                loop_count=0;
  unsigned int       timeout_ms;
  unsigned int       orig_count;
  unsigned int      *script_entry_point;
  rtc_value          cmd_timer,abort_timer;
#if USE_POLLING
  unsigned int       waiting_for_int;
#endif
#if SCSI_DEBUG
 #ifdef trace
  register uchar *buf_ptr;
  register int j;
 #endif
#endif
  int  data_ptr_modifier;  // Device specified value for Modify Data Pointers message

  script_entry_point = NCR_INFO.scsi_script_ptr;
  do
    {
     if(bus_reset)
       { do { } while ( 0 == check_scsi_timeout(bus_reset_delay) );
         bus_reset = 0;  // Reset the flag, we've waited the 3 seconds
       }

#if SCSI_DEBUG
 #ifdef trace
     cmd_timer = get_scsi_timeout(int_timeout_s * (stf ? 2000 : 1000));
 #else
     cmd_timer = get_scsi_timeout(int_timeout_s * 1000);
 #endif
#else
     cmd_timer = get_scsi_timeout(int_timeout_s * 1000);
#endif
     data_xfered = 0;
     status_byte = 0;
     msg_in_buf = 0;

     //-----------------------------//
     //  setup the target scsi id   //
     //-----------------------------//

 #if    ( (NCR_CHIP == 720) || (NCR_CHIP == 810) )
     IO_WRITE_8(S_SDID,(unsigned char) device_ptr->id );
 #endif

     rc=RETURN_TO_SCRIPT;

     do    // while rc = return_to_script and haven't timed out yet
       {
 //     if(flush_it) full_cache_flush();
        INT_FLAG=~GET_RESULTS;  // initialize interrupt occurred flag

        // To execute a script, the address of the first scsi script instuction
        // should be written to register DSP. (DMA Scripts Pointer)
        // Once this is done, the scripts are fetched and executed until an
        // interrupt condition occurs.

        CHECKPOINT(0x97)

 #if USE_POLLING

        // Starts the script and watch ISTAT for an interrupt

        IO_WRITE_32(S_DSP,swap_word(address_conversion + (uint)&SCRIPT + (uint)script_entry_point));

        CHECKPOINT(0x98)

        waiting_for_int=1;
 //     timeout_ms=int_timeout_s *1000;

        while ( (waiting_for_int) && (0 == check_scsi_timeout(cmd_timer)) )
          {
           CHECKPOINT(0x99)
           scsi_delay(500);
           NCR_REG(ISTAT)=IO_READ_8(S_ISTAT);
           #if ( (NCR_CHIP == 720) || (NCR_CHIP == 810) )
           if (NCR_REG(ISTAT)&(ISTAT_INTF))      // interrupt on the fly
             {                                   // must be manually cleared
              IO_WRITE_8(S_ISTAT,(unsigned char) 0x04);

              // NOTE !  when disconnect was introduced, interrupts were being used.
              //         data_xfered needs to be adjusted at this point to reflect
              //         the total number of bytes of data transferred so far to
              //         support multiple disconnects in a transfer

              data_xfered += ( swap_word(SCRIPT[Ent_data_xfer_inst / 4]) & 0x00ffffff );

#if SCSI_DEBUG
              #ifdef trace
              if(stf)
                {
                 printf("\n=> INTFLY (0x%X bytes)",
                 ( swap_word(SCRIPT[Ent_data_xfer_inst / 4]) & 0x00ffffff) );

                 if( *(uchar *) (swap_word(SCRIPT[(Ent_data_xfer_inst / 4)+1]) - address_conversion) == 0x03)
                   {
                    printf("\n=>  (");
                    for (i=0 ; i<NUM_SENSE_BYTES ;i++) printf("%02X ",(uchar *)(((uint)&sense)+i));
                    printf(")\n");
                   }
                }
              #endif
#endif
             }
           #endif
           if (NCR_REG(ISTAT)&(ISTAT_DIP|ISTAT_SIP))
             {
             waiting_for_int=0;
             INT_FLAG=GET_RESULTS;
             }
          }
 #else
        // Start the script and wait for the hardware interrupt to occur

        NCR_REG(ISTAT) = 0;
        IO_WRITE_32(S_DSP,swap_word(address_conversion + (uint)&SCRIPT + (uint)script_entry_point));

        while( (!(NCR_REG(ISTAT))) && (0 == check_scsi_timeout(cmd_timer)) ) { };

        if(NCR_REG(ISTAT)) INT_FLAG=GET_RESULTS;
 #endif

        if (INT_FLAG != GET_RESULTS)  // command timed out */
          {
           // try to stop the script processor with an ISTAT Abort

#if SCSI_DEBUG
           if(sdf)
             {
              printf("\nTIMED OUT  ");
              print_stat();
             }

#endif

 #if USE_POLLING
           abort_timer= get_scsi_timeout(int_timeout_s*1000);
           IO_WRITE_8(S_ISTAT,NCR_REG(ISTAT) | 0x80);  // Abort cmd
           waiting_for_int = 1;
           while ( (waiting_for_int) && (0 == check_scsi_timeout(abort_timer)) )
             {
              // wait for the script aborted interrupt
              scsi_delay(500);
              NCR_REG(ISTAT) = IO_READ_8(S_ISTAT);
              if (NCR_REG(ISTAT) & ISTAT_SIP)
                { NCR_REG(SIST0) = IO_READ_8(S_SIST0);
                  scsi_delay(10);
                  NCR_REG(SIST1) = IO_READ_8(S_SIST1);
                }
              else
                {
                 if (NCR_REG(ISTAT) & ISTAT_DIP)
                   {
                     IO_WRITE_8(S_ISTAT,0x00); // reset the abort bit
                     waiting_for_int=0;  // abort complete
                     INT_FLAG=GET_RESULTS;
#if SCSI_DEBUG
                     if(sdf) printf("Script Aborted");
#endif
                   };
                }
             }
           if(INT_FLAG != GET_RESULTS)
 #else
           NCR_REG(ISTAT) = 0;
           NCR_REG(DSTAT) = 0;
           abort_timer= get_scsi_timeout(int_timeout_s*1000);
           IO_WRITE_8(S_ISTAT,NCR_REG(ISTAT) | 0x80);  // Abort cmd

           // wait for the script aborted interrupt
           do{ } while ( (! (NCR_REG(DSTAT) & 0x10) ) &&
                         (0 == check_scsi_timeout(abort_timer))
                       );

           if(NCR_REG(DSTAT) & 0x10)  // was script processing terminated ?
             {
              INT_FLAG = GET_RESULTS;
             }
           else
 #endif
             {
              // Script Abort timed out, Controller needs to be reset
 //           IO_WRITE_8(S_ISTAT,0x00); // reset the abort bit
              ncr_soft_reset();
              ncr_default_setup(adapter_ptr);
             };
          }

        rc=RETURN_TO_SCRIPT;
        if (INT_FLAG==GET_RESULTS)
         {

          CHECKPOINT(0x10)
 #if ( (NCR_CHIP==720) || (NCR_CHIP==810) )
 #if USE_POLLING
          NCR_REG(SIST0)=IO_READ_8(S_SIST0);  // SCSI int status
          scsi_delay(5);
          NCR_REG(SIST1)=IO_READ_8(S_SIST1);  // SCSI int status
 #endif
          NCR_REG(SSTAT0) = IO_READ_8(S_SSTAT0);
          NCR_REG(SSTAT1) = IO_READ_8(S_SSTAT1);

          // Read DMA Scripts Pointer Save Register
          NCR_DREG(DSP)=swap_word(IO_READ_32(S_DSP));
          NCR_DREG(DSPS)=swap_word(IO_READ_32(S_DSPS));

 #if USE_POLLING
          NCR_REG(DSTAT)=IO_READ_8(S_DSTAT);
 #endif
 #endif

          switch ( NCR_REG(ISTAT) & (ISTAT_DIP | ISTAT_SIP) )
           { case ISTAT_DIP:
                  {
                   CHECKPOINT(0x11)
                   // this is DMA interrupt and NOT a SCSI interrupt
                   //
                   // The following conditions will cause a DMA interrupt:
                   //   host parity error | Script executed in single step mode
                   //   bus fault         | Script interrupt instruction executed
                   //   an abort condition| Watchdog timer decrements to zero
                   //   illegal instuction|

                   // Look for a Script Interrupt Instruction Interrupt.
                   // This is the normal exit for a SCSI operation.

                   if ((NCR_REG(DSTAT)&DIEN_MASK)==DSTAT_SIR)
                    {

                     if (A_Complete == NCR_DREG(DSPS))  // if command was successful ...
                      {
#if SCSI_DEBUG
 #ifdef trace
                       if(stf)
                         {
                          buf_ptr = original_data_ptr;
                          // If Request Sense command ...
                          if( *(uchar *)(swap_word(SCRIPT[ E_cmd_buf_Used[0] ]) - address_conversion) == 0x03)
                            {
                             printf("\n=> sense (");
                             for(j=0; j<data_xfered; j++) printf("%02X ",buf_ptr[j]);
                             printf(")\n");
                            }
                          else
                            {
                             // If Inquiry command
                             if( *(uchar *)(swap_word(SCRIPT[ E_cmd_buf_Used[0] ]) - address_conversion) == 0x12)
                               {
                                printf("\n=> inq data");
                                (void) mem_dump(original_data_ptr,
                                                data_xfered);
                               }
                             else
                               {
                                if( *(uchar *)(swap_word(SCRIPT[ E_cmd_buf_Used[0] ]) - address_conversion) == 0x25)
                                  {
                                   printf("\n=> cap (");
                                   for(j=0; j<data_xfered; j++) printf("%02X ",buf_ptr[j]);
                                   printf(")\n");
                                  }
                               }
                            }
                         }
 #endif
#endif
                       switch (status_byte)
                         {
                          case SCSI_STATUS_GOOD:
                                 {
                                  last_reenter_phase = 0xff; // reset since currently in Bus Free state

                                  // if no error recovery was necessary for command, results are good
                                  // otherwise, go through a retry to get a clean run

                                  rc = ( NCR_INFO.scrmods.modified  ) ? CMD_FAILED : 0;
#if SCSI_DEBUG
                                  if( sdf && (status_byte || msg_in_buf) )
                                     printf("\nStatus byte = %02X, Cmd complete msg in = %02X\n",status_byte,msg_in_buf);
#endif
                                  if(!rc)
                                    {
                                     rc = data_expected - data_xfered;
                                     if(rc) rc += SHORT_RECORD;
                                    }
#if SCSI_DEBUG
                                  if(rc && sdf) printf("\nScript complete OK - Status byte = Good, rc = 0x%X",rc);
#endif
                                  break;
                                 };
                          case SCSI_STATUS_DEVICE_BUSY:
                                 {
#if SCSI_DEBUG
                                  if(sdf
                                 #ifdef trace
                                     && !stf
                                 #endif
                                    ) printf("  status = 0x%X (busy)",status_byte);
#endif
                                  rc=BUSY;
                                  break;
                                 };
                          case SCSI_STATUS_CHECK_CONDITION:
                                 {
                                  rc=SENSE_NEEDED;
                                  break;
                                 };
                          case SCSI_STATUS_RESERVATION_CONFLICT:
                                 {
#if SCSI_DEBUG
                                  if(sdf
                                 #ifdef trace
                                     && !stf
                                 #endif
                                    )printf("  status = 0x%X (res. conf.)",status_byte);
#endif
                                  rc=RESERVATION_CONFLICT;
                                  break;
                                 }
                          default:
                                 {
#if SCSI_DEBUG
                                  if(sdf
                                 #ifdef trace
                                     && !stf
                                 #endif
                                    )printf("  status = 0x%X ",status_byte);
                                  CHECKPOINT(0x14)
                                  print_stat();
#endif
                                  rc=CMD_FAILED;
                                 }
                         }; // end of switch (status_byte)
                      }
                     else  // DSPS != Command complete
                      {
#if SCSI_DEBUG
 #ifdef trace
                       if ( trace == (NCR_DREG(DSPS)&0xFFFF0000) )
                         {
                          if(stf) printf("\n=> ");
                          switch( NCR_DREG(DSPS) )
                           {
                            case A_trace_select_1 : { script_entry_point = (uint *)Ent_trace_select_1_ret;
                                                      if(stf)
                                                        printf("Select (%d,%d)",device_ptr->id,device_ptr->lun);
                                                      break;
                                                    }
                            case A_trace_msgout_1 : { script_entry_point = (uint *)Ent_trace_msgout_1_ret;
                                                      if(stf) printf("Msg Out (%02X)",identify_msg_buf[0]);
                                                      break;
                                                    }
                                 case A_trace_cmd : { script_entry_point = (uint *)Ent_trace_cmd_ret;
                                                      if(stf)
                                                        {
                                                         printf("Cmd  ");
                                                         buf_ptr = (uchar *) (swap_word(SCRIPT[E_cmd_buf_Used[0]])
                                                                             - address_conversion);
                                                         i = swap_word(SCRIPT[E_cmd_buf_Used[0]-1]) & 0x00ffffff;

                                                         printf("%02X %02X %02X %02X %02X ",buf_ptr[0],buf_ptr[1],
                                                                                 buf_ptr[2],buf_ptr[3],buf_ptr[4]);
                                                         for(j=5 ;j<i ;j++) printf("%02X ",buf_ptr[j]);

                                                        }
                                                      break;
                                                    }
                                case A_trace_data : { script_entry_point = (uint *)Ent_trace_data_ret;
                                                      if(stf)
                                                        {
                                                         j = (swap_word( SCRIPT[E_data_buffer_Used[0]-1] ) & 0x00ffffff);
                                                         i = swap_word(SCRIPT[E_data_buffer_Used[0]]);
                                                         if ( DATA_IN_PHASE == (NCR_REG(SSTAT1) & 0x07)  )
                                                           printf("Data In (%d bytes @ 0x%X)",j,i);
                                                         else
                                                           if ( DATA_OUT_PHASE == (NCR_REG(SSTAT1) & 0x07)  )
                                                             printf("Data Out (%d bytes @ 0x%X)",j,i);
                                                        }

                                                      break;
                                                    }
                              case A_trace_status : { script_entry_point = (uint *)Ent_trace_status_ret;
                                                      if(stf) printf("Status (%02X)",status_byte);
                                                      break;
                                                    }
                             case A_trace_msgin_1 : { script_entry_point = (uint *)Ent_trace_msgin_1_ret;
                                                      if(stf)
                                                        {
                                                         printf("Msg In (%02X) ",msg_in_buf);
//@****                                                  switch(msg_in_buf)
//@****                                                     {
//@****                                                      case 0x00: printf("Cmd Complete");
//@****                                                                 break;
//@****                                                      case 0x01: printf("Extended Msg");
//@****                                                                 break;
//@****                                                      case 0x02: printf("Save pointer");
//@****                                                                 break;
//@****                                                      case 0x03: printf("Restore pointer");
//@****                                                                 break;
//@****                                                      case 0x04: printf("Disconnect");
//@****                                                                 break;
//@****                                                      case 0x07: printf("Reject");
//@****                                                                 break;
//@****                                                      default:   printf("Unhandled Msg");
//@****                                                                 break;
//@****                                                     }
                                                        }
                                                      break;
                                                    }
                            case A_trace_select_2 : { script_entry_point = (uint *)Ent_trace_select_2_ret;
                                                      if(stf)
                                                        printf("Neg. Select (%d,%d)",device_ptr->id,device_ptr->lun);
                                                      break;
                                                    }
                            case A_trace_msgout_2 : { script_entry_point = (uint *)Ent_trace_msgout_2_ret;
                                                      if(stf)
                                                         printf("Msg Out (%02X %02X %02X %02X %02X %02X)",
                                                                                     identify_msg_buf[0],
                                                                                     identify_msg_buf[1],
                                                                                     identify_msg_buf[2],
                                                                                     identify_msg_buf[3],
                                                                                     identify_msg_buf[4],
                                                                                     identify_msg_buf[5] );
                                                      break;
                                                    }
                             case A_trace_msgin_2 : { script_entry_point = (uint *)Ent_trace_msgin_2_ret;
                                                      if(stf) printf("Msg In (neg resp)");
                                                      break;
                                                    }
                             case A_trace_msgin_3 : { script_entry_point = (uint *)Ent_trace_msgin_3_ret;
                                                      if(stf) printf("Abort (neg resp)");
                                                      break;
                                                    }
                             case A_trace_msgin_4 : { script_entry_point = (uint *)Ent_trace_msgin_4_ret;
                                                      if(stf) printf("Msg In (finish neg resp)");
                                                      break;
                                                    }
                          case A_trace_check_xfer : { script_entry_point = (uint *)Ent_trace_check_xfer_ret;
                                                      if(stf)
                                                         printf("   neg resp : %02X %02X %02X %02X %02X",
                                                                                        sync_msg_in_buf,
                                                                                        sync_msg_resp[0],
                                                                                        sync_msg_resp[1],
                                                                                        sync_msg_resp[2],
                                                                                        sync_msg_resp[3]);
                                                      break;
                                                    }
                               case A_trace_resel : { script_entry_point = (uint *)Ent_trace_resel_ret;
                                                      if(stf) printf("Reselected");
                                                      break;
                                                    }
                                          default : { if(stf) printf("0x%X",NCR_DREG(DSPS)); }
                           }
                         }
                       else
                         {
 #endif
#endif
                          //----------------------------------------------------------------//
                          //        Need to re-enter script at this point to cleanup        //
                          //----------------------------------------------------------------//


                          switch (NCR_DREG(DSPS))
                           {
                             case 0x00000000: // SCRIPT data not swapped for little endian operation
                                              rc = SCRIPT_ERROR;
                                              break;
                             case A_Disconnecting:  // Device disconnect request
                                              last_reenter_phase = 0xff;  // Clear phase tracker so I
                                                                          // don't reset the bus later
                                              script_entry_point = (uint *)Ent_dev_disconnect;

//                                            // If format command with data transfer, return with disconnect
//                                            if(
//                                               ( *(uchar *)(swap_word(SCRIPT[ E_cmd_buf_Used[0] ]) - address_conversion)
//                                                   == 0x04) &&
//                                               ( *(uchar *)( (swap_word(SCRIPT[ E_cmd_buf_Used[0] ])
//                                                              - address_conversion) + 1) & 0x10) &&
//                                               (data_xfered == data_expected)  // all data transferred
//                                              )
//
//                                              rc = DISCONNECT;
                                              break;
                             case A_Save_PTR:    // Save current data pointer request
                                              saved_data_ptr = current_data_ptr;
                                              saved_data_byte_cnt = current_data_byte_cnt;
                                              script_entry_point = (uint *)Ent_common_restart;
                                              last_reenter_phase = 0xff;  // Clear since this isn't
                                                                          // an error condition
                                              break;  // Return to script
                             case A_Restore_PTR:  // Restore Saved data pointer request

                                              // Set SCRIPT pointer and byte count to saved values
                                              for (i=0; i< ((sizeof E_data_buffer_Used) / 4); i++)
                                                {
                                                 // Set byte count to original count
                                                 SCRIPT[E_data_buffer_Used[i] - 1] =
                                                         (SCRIPT[E_data_buffer_Used[i]-1] & 0x000000ff) |
                                                         swap_word(saved_data_byte_cnt);

                                                 // Adjust the target address to saved value
                                                 SCRIPT[E_data_buffer_Used[i]] = swap_word(saved_data_ptr);
                                                }
                                              // Update current values
                                              current_data_ptr = saved_data_ptr;
                                              current_data_byte_cnt = saved_data_byte_cnt;

                                              script_entry_point = (uint *)Ent_common_restart;
                                              last_reenter_phase = 0xff;  // Clear since this isn't
                                                                          // an error condition
                                              break;
                             case A_Modify_PTR:   // Modify data pointer request
                                              // Modify the current data pointer info (and SCRIPT data)
                                              // based on specified modifier !
#if SCSI_DEBUG
                                              if(stf) printf("\n Modify pointer message modifier (0x%X)",data_ptr_modifier);
#endif
                                              current_data_ptr += data_ptr_modifier;
                                              current_data_byte_cnt -= data_ptr_modifier;

                                              // Update SCRIPT pointer and byte count
                                              for (i=0; i< ((sizeof E_data_buffer_Used) / 4); i++)
                                                {
                                                 // Set byte count to original count
                                                 SCRIPT[E_data_buffer_Used[i] - 1] =
                                                         (SCRIPT[E_data_buffer_Used[i]-1] & 0x000000ff) |
                                                         swap_word(current_data_byte_cnt);

                                                 // Adjust the target address to saved value
                                                 SCRIPT[E_data_buffer_Used[i]] = swap_word(current_data_ptr);
                                                }

                                              script_entry_point = (uint *)Ent_common_restart;
                                              last_reenter_phase = 0xff;  // Clear since this isn't
                                                                          // an error condition
                                              break;
                             case A_not_Status:  // not Status  phase
                                              // If still in data phase, got out of sync with target
                                              if(sdf) printf("\nSSTAT1 %X ",NCR_REG(SSTAT1));
                                              if ( (DATA_OUT_PHASE == (NCR_REG(SSTAT1) & 0x07) ) ||
                                                   (DATA_IN_PHASE  == (NCR_REG(SSTAT1) & 0x07) )
                                                 )
                                                {
                                                 script_entry_point = NCR_INFO.scsi_script_ptr ;
                                                 rc=RESET_NEEDED;
                                                 // Indicate negotiations need to be (re)established
//@@                                             device_ptr->status.Negotiate = 1;
                                                }
                                              else
                                                {
                                                 script_entry_point = (uint *)ncr_cleanup_script();
                                                 if ( script_entry_point )
                                                   {
#if SCSI_DEBUG
                                                    if(sdf) printf("\nScript restart @ 0x%X\n",script_entry_point);
#endif
                                                   }
                                                 else
                                                   {
                                                    script_entry_point = NCR_INFO.scsi_script_ptr ;
                                                    rc=RESET_NEEDED;
                                                    // Indicate negotiations need to be (re)established
//@@                                                device_ptr->status.Negotiate = 1;
                                                   }
                                                }
                                              break;
                             case A_not_Msg_Out: // not Msg Out phase
                             case A_not_Cmd:     // not Cmd     phase
                             case A_not_Data:    // not Data In/Out phase
                             case A_not_Msg_In:  // not Msg In  phase
                                     default:
                                              {
#if SCSI_DEBUG
                                                if(sdf) printf("\nSIR but not OK (Check SSTAT1 - unexp phase)\n");
                                                CHECKPOINT(0x15)
                                                print_stat();
#endif
                                                script_entry_point = (uint *)ncr_cleanup_script();
                                                if ( script_entry_point )
                                                  {
#if SCSI_DEBUG
                                                   if(sdf) printf("\nScript restart @ 0x%X\n",script_entry_point);
#endif
                                                  }
                                                else
                                                  {
                                                   script_entry_point = NCR_INFO.scsi_script_ptr ;
                                                   rc=RESET_NEEDED;
                                                   // Indicate negotiations need to be (re)established
//@@                                               device_ptr->status.Negotiate = 1;
                                                  }
                                              }
                           }
#if SCSI_DEBUG
 #ifdef trace
                         }
 #endif
#endif
                      }
                    }
                   else // DSTAT SIR bit not set
                    {
                     rc=CMD_FAILED;
                     if (NCR_REG(DSTAT)&BUS_FAULT)
                       { orig_count = pci_cfg_read_32(adapter_ptr->handle,0x04);  // for rev level 1 part
#if SCSI_DEBUG
                         NCR_REG(SCRATCHB) = (unsigned char) (orig_count >> 24);
                         if(sdf)
                           {
                            printf("\nPCI Bus fault, Status = %04X ",NCR_REG(SCRATCHB)<<8);
                            if ( NCR_REG(SCRATCHB) & 0x80 ) printf("(Parity Error) ");
                            if ( NCR_REG(SCRATCHB) & 0x20 ) printf("(Master Abort) ");
                            if ( NCR_REG(SCRATCHB) & 0x10 ) printf("(Target Abort) ");
                           }
#endif
                         pci_cfg_write_32(adapter_ptr->handle, 0x04, orig_count & 0xf7000147);
                       }
#if SCSI_DEBUG
                     else
                       {
                        if(! (NCR_REG(DSTAT) & 0x10) )  // If command not aborted
                          {
                           if(sdf) printf("\nDMA : Check DSTAT\n");
                           CHECKPOINT(0x16)
                           print_stat();
                          }
                       }
#endif
                     script_entry_point = (uint *)ncr_cleanup_script();
                     if ( script_entry_point )
                       {
                        // values returned by ncr_cleanup_script are relative to the beginning
                        // of SCRIPT, not the initial entry point (NCR_INFO.scsi_script_ptr)

#if SCSI_DEBUG
                        if(sdf) printf("\nScript restart @ 0x%X\n",script_entry_point);
#endif
                       }
                     else
                       {
                        script_entry_point = NCR_INFO.scsi_script_ptr ;
                        CHECKPOINT(0x17)
#if SCSI_DEBUG
                        print_stat();
#endif
                        rc=RESET_NEEDED;
                        // Indicate negotiations need to be (re)established
//@@                    device_ptr->status.Negotiate = 1;
                       }
                    }
                   break;
                  }
             case ISTAT_SIP:
                  {
                   //----------------------------------------------------------------//
                   // this is a SCSI interrupt and NOT a DMA interrupt               //
                   // The following conditions may cause this type of interrupt:     //
                   // phase mismatch                                                 //
                   // arbitration sequence complete                                  //
                   // selection or reselection timeout                               //
                   // chip selected or reselected                                    //
                   // SCSI gross error                                               //
                   // unexpected disconnect             handshake timer expired      //
                   // SCSI reset                        gen purpose timer expierd    //
                   // parity error                                                   //
                   //----------------------------------------------------------------//

 #if ( (NCR_CHIP==720) || (NCR_CHIP==810) )
                   if ((NCR_REG(SIST0)&PHASE_MIS)==PHASE_MIS)
 #endif
                     {
                      if( ( Ent_data_xfer_inst == ( ((NCR_DREG(DSP) - address_conversion) - (uint)SCRIPT ) - 8 ) ) &&
                          !( (DATA_OUT_PHASE == NCR_REG(SSTAT1)) || (DATA_IN_PHASE == NCR_REG(SSTAT1)) )
                        )

                        {
                         // Phase change during data transfer, temporarily save the current pointers
                         NCR_DREG(DBC)  = swap_word(IO_READ_32(S_DBC) & 0xffffff00);  // Get byte count
                         NCR_DREG(DNAD) = swap_word(IO_READ_32(S_DNAD));              // and current pointer
#if SCSI_DEBUG
                         if(stf)
                             printf("\n    DBC (%X) DFIFO (%X) SSTAT0 (%X) DNAD (%x) DSTAT (%X)",
                                       NCR_DREG(DBC),
                                       IO_READ_8(S_DFIFO) & 0x7f,
                                       IO_READ_8(S_SSTAT0),
                                       NCR_DREG(DNAD),
                                       IO_READ_8(S_DSTAT)
                                   );
#endif


                         if ( (SCRIPT[Ent_data_phase / 4] & 0x07) == DATA_OUT_PHASE)  // If writing data ...
                           {
                            // See if there's data in the DMA FIFO (use NCR_REG(DFIFO) as work variable)
                            NCR_REG(DFIFO) = (((uint)(IO_READ_8(S_DFIFO) & 0x7f)) - (NCR_DREG(DBC) & 0x7f))&0x7f;

                            // Now check for data in the SCSI output latch
                            NCR_REG(DFIFO) += ( ( IO_READ_8(S_SSTAT0) & 0x20 ) ? 1 : 0 );

                            if(NCR_REG(DFIFO))
                              {
                               IO_WRITE_8(S_CTEST3, 0x04);                      // Clear DMA FIFO
                               IO_WRITE_8(S_STEST3, (TOLERANT_ENABLE | 0x02) ); //   and SCSI Output data register
                               NCR_DREG(DBC)  += NCR_REG(DFIFO);                // Update byte count
                               NCR_DREG(DNAD) -= NCR_REG(DFIFO);                //    and pointer
                              }
                           }
#if SCSI_DEBUG
                         if(stf)
                             printf("\n    After DBC (%X) DFIFO (%X) SSTAT0 (%X) DNAD (%X) DSTAT (%X)",
                                        NCR_DREG(DBC),
                                        IO_READ_8(S_DFIFO) & 0x7f,
                                        IO_READ_8(S_SSTAT0),
                                        NCR_DREG(DNAD),
                                        IO_READ_8(S_DSTAT)
                                   );
#endif

                         // Modify SCRIPT data with current pointer and count values
                         for (i=0; i< ((sizeof E_data_buffer_Used) / 4); i++)
                           {
                            // Adjust byte count to residual count (reflected in DBC)
                            SCRIPT[E_data_buffer_Used[i] - 1] =
                                    (SCRIPT[E_data_buffer_Used[i]-1] & 0x000000ff) | swap_word(NCR_DREG(DBC));

                            // Adjust the target address to saved value
                            SCRIPT[E_data_buffer_Used[i]] = swap_word(NCR_DREG(DNAD));
                           }

                         // Tally data transferred during the last DATA phase
                         data_xfered += (current_data_byte_cnt - NCR_DREG(DBC));

#if SCSI_DEBUG
 #ifdef trace
                         if(stf)
                             printf("\n    orig (%d) - accum (%d) = (%d) left",
                                    data_expected,
                                    data_xfered,
                                    NCR_DREG(DBC)
                                   );
 #endif
#endif
                         current_data_ptr      = NCR_DREG(DNAD);  // update current values
                         current_data_byte_cnt = NCR_DREG(DBC);   //

                         script_entry_point = (uint *)Ent_common_restart;  // Return to handle phase change
#if SCSI_DEBUG
 #ifdef trace
                         if(stf)
                           {
                            printf("\n=>  (0x%X of 0x%X byte(s) transferred)",
                              (saved_data_byte_cnt - current_data_byte_cnt), saved_data_byte_cnt );
                            if((saved_data_byte_cnt - current_data_byte_cnt) & 0x0f)
                              {
                               printf("\n  bad xfer count\n");
//@@                           do{} while (shf && (FALSE == check_key_avail()) );  // Halt
                              }
                           }
 #endif
#endif
                        }
                      else
                        {
#if SCSI_DEBUG
                         if(sdf) printf("\nSCSI phase mismatch");
 #ifdef trace
                         NCR_DREG(SCRATCHA) = swap_word(SCRIPT[Ent_data_xfer_inst /4]) & 0x00ffffff;
 #endif
#endif
                         script_entry_point = (uint *)ncr_cleanup_script();
                         if ( script_entry_point )
                           {
                            // values returned by ncr_cleanup_script are relative to the beginning
                            // of SCRIPT, not the initial entry point (NCR_INFO.scsi_script_ptr)
#if SCSI_DEBUG
                            if(stf) printf("\nScript restart @ 0x%X\n",script_entry_point);
 //                         print_stat();
#endif
                           }
                         else
                           {
                            script_entry_point = NCR_INFO.scsi_script_ptr ;
                            rc=RESET_NEEDED;
                            // Indicate negotiations need to be (re)established
//@@                        device_ptr->status.Negotiate = 1;
                           }
                        }
                     }
                   else  // phase mismatch bit not on
                     {
 #if ( (NCR_CHIP==720) || (NCR_CHIP==810) )
                      if ( (NCR_REG(SIST0) & 0x04) || (NCR_REG(SIST1)&0x04) ) // unexpected disconnect or selection timeout
 #endif
                       {
                        rc=DEVICE_NOT_AVAILABLE;
                        device_ptr->status.Negotiate = 1;       // Set the negotiate flag for next cmd
                       }
                      else
                       {
                        NCR_REG(DSTAT)=IO_READ_8(S_DSTAT);
                        CHECKPOINT(0x18)
#if SCSI_DEBUG
                        print_stat();
#endif
                        rc=CMD_FAILED;
                       }
                     }
                   break;
                  }
             case (ISTAT_DIP | ISTAT_SIP):
                  {
                    //--------------------------------------------------------------//
                    // there seems to be both a scsi and dma interrupt try to       //
                    // clear all interrupts present there should only be one        //
                    // present at a time. Dont allow hang in the loop.              //
                    //--------------------------------------------------------------//

                   CHECKPOINT(0x12)
                   timeout_ms=200;
#if SCSI_DEBUG
                   printf("\nDevice address %d, %d",device_ptr->id,device_ptr->lun);
                   printf("\nBoth DIP & SIP (ISTAT = %02X) : DSPS = %X, DSTAT = %X",
                                                       NCR_REG(ISTAT),NCR_DREG(DSPS),NCR_REG(DSTAT));
                   printf("\n                              SSTAT0 = %X",NCR_REG(SSTAT0));
                   printf("\n                              SIST0 = %X, SIST1 = %X",NCR_REG(SIST0),NCR_REG(SIST1));
                 if (NCR_REG(DSTAT)&BUS_FAULT)
                   { orig_count = pci_cfg_read_32(adapter_ptr->handle,0x04);  // for rev level 1 part
                     NCR_REG(SCRATCHB) = (unsigned char) (orig_count >> 24);
                       printf("\nPCI Bus fault, Status = %04X ",NCR_REG(SCRATCHB)<<8);
                     if ( (NCR_REG(SCRATCHB) & 0x80) )
                       { printf("(Parity Error) "); }
                     if ( (NCR_REG(SCRATCHB) & 0x20) )
                       { printf("(Master Abort) "); }
                     if ( (NCR_REG(SCRATCHB) & 0x10) )
                       { printf("(Target Abort) "); }
                     pci_cfg_write_32(adapter_ptr->handle, 0x04, orig_count & 0xf7000147);
                   }
#endif
                   while((NCR_REG(ISTAT)&(ISTAT_DIP|ISTAT_SIP)) && timeout_ms  )
                    {
                     NCR_REG(DSTAT)=IO_READ_8(S_DSTAT);
                     scsi_delay(10);
                     NCR_REG(SIST0)=IO_READ_8(S_SIST0);
                     scsi_delay(10);
                     NCR_REG(SIST1)=IO_READ_8(S_SIST1);
                     scsi_delay(10);
                     NCR_REG(SSTAT0)=IO_READ_8(S_SSTAT0);
                     NCR_REG(ISTAT)=IO_READ_8(S_ISTAT);
                     timeout_ms--;
                    }
#if SCSI_DEBUG
 //                printf("Script timeout\n");
#endif
                   rc=CMD_FAILED;
                   break;
                  }
           }; // end of switch
         }
        else // INT_FLAG != GET_RESULTS
         {
          // a timeout has ocurred, couldn't abort Script processing

          // reset the scsi bus to try to clear cause of timeout (700&720 ok)
             CHECKPOINT(0x13)
//           Reset_SCSI_Bus();
             rc=RESET_NEEDED;
             // Indicate negotiations need to be (re)established
//@@         device_ptr->status.Negotiate = 1;
         }

       // end do loop while RC == RETURN_TO_SCRIPT and not timed out yet
       } while ( (rc==RETURN_TO_SCRIPT) && (0 == check_scsi_timeout(cmd_timer)) );

     if(rc==RETURN_TO_SCRIPT) rc=CMD_FAILED;  // Set to failed if Timed Out

     NCR_REG(SCRATCHA) = IO_READ_8(S_SCRATCHA);  // Get SCSI Bus phase tracker register
     if(NCR_REG(SCRATCHA))                       // If at least one phase was completed,
                                                 // check to see if the command completed
                                                 // or if it needs to be aborted
       if( (NCR_REG(SCRATCHA) & (A_status_flag | A_cmd_cmp_flag))
           != (A_status_flag | A_cmd_cmp_flag) )  // Must have had Status and Cmd Complete msg
         {
          rc = RESET_NEEDED;            // Reset the bus 'til I figure out Abort sequence
          if(sdf) printf("\nPhase flags %X",NCR_REG(SCRATCHA));
          last_reenter_phase = 0xff;    // Reset phase tracker for next command
         }

         //------------------------------------------------------------------//
         // anticipate a retry being neccessary                              //
         //------------------------------------------------------------------//

     NCR_REG(ISTAT) = IO_READ_8(S_ISTAT);
     while (NCR_REG(ISTAT)&0x03)   // check for a stacked interrupt
       {
#if SCSI_DEBUG
        if(sdf) printf("*** STACKED INTERRUPT : %02X ",NCR_REG(ISTAT));
#endif
        if (NCR_REG(ISTAT) & 0x02) // if SCSI interrupt pending
         {
          NCR_REG(SIST0) = IO_READ_8(S_SIST0);
          scsi_delay(5);
          NCR_REG(SIST1) = IO_READ_8(S_SIST1);
          scsi_delay(5);
#if SCSI_DEBUG
          if(sdf) printf("SIST0 = %X, SIST1 = %X ",NCR_REG(SIST0),NCR_REG(SIST1));
#endif
         };
        if (NCR_REG(ISTAT) & 0x01) // if DMA interrupt pending
         {
#if SCSI_DEBUG
          if(sdf) printf("DSTAT = %X",IO_READ_8(S_DSTAT));
#endif
          scsi_delay(5);
         };
        NCR_REG(ISTAT) = IO_READ_8(S_ISTAT);
        rc=CMD_FAILED;
       };

 //  rc==SENSE_NEEDED removed from the following line - only set with Check Condition
 //  and it'd be nice to get the sense data from the device

    }  while (((rc==CMD_FAILED)||(rc==BUSY)  /* ||(rc==SENSE_NEEDED)  */  )
        &&((loop_count++)<retry_count));

   //  if script entry point changed, script may need to be cleaned back up -
   //  transfer addresses and/or sizes may have been adjusted
   if ( script_entry_point != NCR_INFO.scsi_script_ptr )
     {
      if (NCR_INFO.scrmods.modified)
        {
         NCR_INFO.scrmods.modified = 0;  // clear the scripts modified flag
         if (NCR_INFO.scrmods.cmd_buf)
           {
            NCR_INFO.scrmods.cmd_buf = 0;
            // command is systematically built on each command request, so no
            // exit cleanup is required
            // currently, this is a placeholder in case I find a condition
            // where there is something I need to do
           }
         if (NCR_INFO.scrmods.identify_buf)
           {
            NCR_INFO.scrmods.identify_buf = 0;
            for (i=0;i<(sizeof E_identify_msg_buf_Used) / 4; i++)
              {
               SCRIPT[E_identify_msg_buf_Used[i]-1] = (SCRIPT[E_identify_msg_buf_Used[i]-1] & 0xff) | swap_word(1);
               SCRIPT[E_identify_msg_buf_Used[i]] = swap_word(address_conversion + (uint)&identify_msg_buf);
              }
            // hard code the synchronous negotiations byte count value directly
            SCRIPT[ (Ent_sync_neg/4) ] = (SCRIPT[(Ent_sync_neg/4) ] & 0xff) | swap_word(6);
           }
         if (NCR_INFO.scrmods.msg_in_buf)
           {
            NCR_INFO.scrmods.msg_in_buf = 0;
            for (i=0;i<(sizeof E_msg_in_buf_Used) / 4; i++)
              {
               SCRIPT[E_msg_in_buf_Used[i]-1] = (SCRIPT[E_msg_in_buf_Used[i]-1] & 0xff) | swap_word(1);
               SCRIPT[E_msg_in_buf_Used[i]] = swap_word(address_conversion + (uint)&msg_in_buf);
              }
           }
         if (NCR_INFO.scrmods.status_byte)
           {
            NCR_INFO.scrmods.status_byte = 0;
            for (i=0;i<(sizeof E_status_byte_Used) / 4; i++)
              {
               SCRIPT[E_status_byte_Used[i]-1] = (SCRIPT[E_status_byte_Used[i]-1] & 0xff) | swap_word(1);
               SCRIPT[E_status_byte_Used[i]] = swap_word(address_conversion + (uint)&status_byte);
              }
           }
         if (NCR_INFO.scrmods.sync_msg_buf)
           {
            NCR_INFO.scrmods.sync_msg_buf = 0;
            for (i=0;i<(sizeof E_sync_msg_resp_Used) / 4; i++)
              {
               SCRIPT[E_status_byte_Used[i]-1] = (SCRIPT[E_status_byte_Used[i]-1] & 0xff) | swap_word(1);
               SCRIPT[E_sync_msg_resp_Used[i]] = swap_word(address_conversion + (uint)&sync_msg_resp);
              }
            SCRIPT[ (Ent_sync_neg_in / 4) ] = (SCRIPT[ (Ent_sync_neg_in / 4) ] & 0xff) | swap_word(4);
           }
        }
     }
   return (rc);
}

//--------------------------------------------------------------------------//
// Function: ncr_cleanup_script();                                          //
// Returns : 0 - if return to script condition not feasible                 //
//           1 - if return to script should be done                         //
//--------------------------------------------------------------------------//
unsigned int ncr_cleanup_script(void)
{
 unsigned int i,c_script_flag;
 unsigned int reenter_modifier;
 unsigned int maybe_modify;
 rtc_value reset_timer;

 // the following 2 definitions are to point to the scripts arrays that identify
 // the scripts words that need buffer address adjustments before the script can
 // be restarted.  The corresponding byte count values are ALWAYS in the word
 // preceding the buffer address word.

   /* iwp = instruction word pointer */
 unsigned int *buffer_addr_iwp;
 unsigned int buffer_addr_word_cnt;

    if('T' == NCR_INFO.scsi_script_type) return(Ent_script_test);

#if SCSI_DEBUG
    if(sdf) printf("\nDSTAT %X  SIST0 %X  SIST1 %X",NCR_REG(DSTAT),NCR_REG(SIST0),NCR_REG(SIST1));
#endif

    // Now we'll use the current bus phase to make the decision on what buffer address
    // and count field (if any) needs to be modified, and where to reenter the script

    c_script_flag = ('C' == NCR_INFO.scsi_script_type) ? TRUE : 0;  // (maybe) set common script type flag

#if ( (NCR_CHIP==720) || (NCR_CHIP==810) )
    if (NCR_REG(DSTAT) & BUS_FAULT)   // If a PCI Bus fault detected ...
        maybe_modify = 1;             //  modify Script pointers for restart
    else
        maybe_modify = 0;             // otherwise, don't modify any script data
#endif

#if SCSI_DEBUG
    if(sdf) printf("\nCur phase = %02X, last = %02X ",NCR_REG(SSTAT1) & 0x07,last_reenter_phase);
#endif
    if( last_reenter_phase == (NCR_REG(SSTAT1) & 0x07) )
      {
       reenter_modifier = 0;
      }
    else
      {
       last_reenter_phase = (NCR_REG(SSTAT1) & 0x07);
#if SCSI_DEBUG
       if(sdf) printf(" Bus phase : ");
#endif
       switch ( NCR_REG(SSTAT1) & 0x07 )
         {
           case DATA_OUT_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("Data Out");
#endif
                                   if (c_script_flag) // invalid phase for inquiry script
                                     {
                                      reenter_modifier = Ent_common_restart;

                                      // set pointers for any data_buffer script references
                                      buffer_addr_iwp = (uint *)E_data_buffer_Used;
                                      buffer_addr_word_cnt = (sizeof E_data_buffer_Used) / 4;

                                      // scripts modified flag does not have to be set since
                                      // a retry will completely rebuild the address & byte cnt
                                      // for data buffer

                                      // Make sure data buffer addresses get updated
//@@@@                                maybe_modify = 1;
#if SCSI_DEBUG
                                      if(sdf) printf(" cleanup 1 mm = %d",maybe_modify);
#endif
                                     }
                                   else
                                     {
                                      reenter_modifier=0;
#if SCSI_DEBUG
                                      if(sdf) printf(" cleanup 2 mm = %d",maybe_modify);
#endif
                                     }
                                   break;
                                 }
            case DATA_IN_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("Data In");
#endif

                                   // set pointers for any data_buffer script references
                                   buffer_addr_iwp = (uint *)E_data_buffer_Used;
                                   buffer_addr_word_cnt = (sizeof E_data_buffer_Used) / 4;

                                   // scripts modified flag does not have to be set since
                                   // a retry will completely rebuild the address & byte cnt
                                   // for data buffer

                                   reenter_modifier = 1;
                                   maybe_modify = 1;
#if SCSI_DEBUG
                                   if(sdf) printf(" cleanup 3 mm = %d",maybe_modify);
#endif
                                   break;
                                 }
                case CMD_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("CMD");
#endif
                                   // set pointers for any cmd_buffer script references
                                   buffer_addr_iwp = (uint *)E_cmd_buf_Used;
                                   buffer_addr_word_cnt = (sizeof E_cmd_buf_Used) / 4;
                                   NCR_INFO.scrmods.modified = maybe_modify;  // set script modified flag
                                   NCR_INFO.scrmods.cmd_buf = maybe_modify;   //  &  cmd buffer modified
                                   reenter_modifier = 1;
#if SCSI_DEBUG
                                   if(sdf) printf(" cleanup 4 mm = %d",maybe_modify);
#endif
                                   break;
                                 }
             case STATUS_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("Status");
#endif
                                   // set pointers for any status byte references
                                   buffer_addr_iwp = (uint *)E_status_byte_Used;
                                   buffer_addr_word_cnt = (sizeof E_status_byte_Used) / 4;
                                   NCR_INFO.scrmods.modified = maybe_modify;      // set script modified flag &
                                   NCR_INFO.scrmods.status_byte = maybe_modify;   // status byte buffer modified
                                   reenter_modifier = 1;
#if SCSI_DEBUG
                                   if(sdf) printf(" cleanup 5 mm = %d",maybe_modify);
#endif
                                   break;
                                 }
            case MSG_OUT_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("Msg Out");
#endif
                                   // set pointers for any identify script references
                                   buffer_addr_iwp = (uint *)E_identify_msg_buf_Used;
                                   buffer_addr_word_cnt = (sizeof E_identify_msg_buf_Used) / 4;
                                   NCR_INFO.scrmods.modified = maybe_modify;      // set script modified flag &
                                   NCR_INFO.scrmods.identify_buf = maybe_modify;  // identify buffer modified
                                   reenter_modifier = 1;
#if SCSI_DEBUG
                                   if(sdf) printf(" cleanup 6 mm = %d",maybe_modify);
#endif
                                   break;
                                 }
             case MSG_IN_PHASE : {
#if SCSI_DEBUG
                                   if(sdf) printf("Msg In");
#endif
                                   if (c_script_flag)
                                     {
                                      reenter_modifier = Ent_common_restart;

                                      // set pointers for any msg_in_buf references
                                      buffer_addr_iwp = (uint *)E_msg_in_buf_Used;
                                      buffer_addr_word_cnt = (sizeof E_msg_in_buf_Used) / 4;
                                      NCR_INFO.scrmods.modified = maybe_modify;      // set script modified flag
                                      NCR_INFO.scrmods.msg_in_buf = maybe_modify;    // set msg_in buffer modified flag
#if SCSI_DEBUG
                                      if(sdf) printf(" cleanup 7 mm = %d",maybe_modify);
#endif
                                     }
                                   else
                                     {
                                      NCR_INFO.scrmods.modified = maybe_modify;      // set script modified flag
                                      if ( ( ((NCR_DREG(DSP) - address_conversion) - (uint)SCRIPT ) > Ent_sdtr_request) &&
                                           ( ((NCR_DREG(DSP) - address_conversion) - (uint)SCRIPT ) < Ent_negotiate_restart) )
                                        {
                                         // set pointers for any sync_msg_in_buf references
                                         buffer_addr_iwp = (uint *)E_sync_msg_resp_Used;
                                         buffer_addr_word_cnt = (sizeof E_sync_msg_resp_Used) / 4;
                                         NCR_INFO.scrmods.sync_msg_buf = maybe_modify;    // set sync_msg_resp buffer modified flag
                                         reenter_modifier = Ent_negotiate_restart;
#if SCSI_DEBUG
                                         if(sdf) printf(" cleanup 8 mm = %d",maybe_modify);
#endif
                                        }
                                      else
                                        {
                                         // set pointers for any msg_in_buf references
                                         buffer_addr_iwp = (uint *)E_msg_in_buf_Used;
                                         buffer_addr_word_cnt = (sizeof E_msg_in_buf_Used) / 4;
                                         NCR_INFO.scrmods.msg_in_buf = maybe_modify;    // set msg_in buffer modified flag
                                         reenter_modifier = Ent_negotiate_restart_not_sdtr;
#if SCSI_DEBUG
                                         if(sdf) printf(" cleanup 9 mm = %d",maybe_modify);
#endif
                                        }
                                     }
                                   break;
                                 }
                              default : {
#if SCSI_DEBUG
                                          if(sdf) printf("(%02X)",NCR_REG(SSTAT1));
#endif
                                          reenter_modifier=0;
#if SCSI_DEBUG
                                          if(sdf) printf(" cleanup 10 mm = %d",maybe_modify);
#endif
                                        }
         }
      }

    if (reenter_modifier)
      {
       if (reenter_modifier == 1)
         {
          if (c_script_flag)
            {
             reenter_modifier = Ent_common_restart;
            }
          else
            {
             reenter_modifier = Ent_negotiate_restart_not_sdtr;
            }
         }

       if (buffer_addr_word_cnt && maybe_modify)
         {
#if SCSI_DEBUG
          if(sdf) printf(" scr modified ");
#endif

          // get the remaining byte count value (if any)
          NCR_DREG(DBC) = swap_word(IO_READ_32(S_DBC)) & 0x00ffffff;

          // If DMA direction is inbound, chip cleans up FIFO and data latches,
          // need to do that work here if data is outbound
          if( !(IO_READ_8(S_CTEST2) & 0x80) );  // Check the direction bit
            {
             // Data is outbound
            }

          for (i=0; i<buffer_addr_word_cnt; i++)
            {
               // Get original byte count value for transfer
             NCR_DREG(SCRATCHB) = swap_word(SCRIPT[buffer_addr_iwp[i] - 1]);

             // Adjust byte count to residual count (reflected in DBC)
             SCRIPT[buffer_addr_iwp[i] - 1] = swap_word( (NCR_DREG(SCRATCHB) & 0xff000000) | NCR_DREG(DBC) );

             // Adjust the target address (add (original count - residual count) to original address)
             SCRIPT[buffer_addr_iwp[i]] = swap_word( swap_word(SCRIPT[buffer_addr_iwp[i]]) +
                                                     ( (NCR_DREG(SCRATCHB) & 0x00ffffff) - NCR_DREG(DBC) ) );
            }
         }

       //-------------------------------------------------//
       // clear scsi latches and both dma and scsi fifo's //
       //-------------------------------------------------//

       IO_WRITE_8(S_CTEST3,(unsigned char)0x04);
       IO_WRITE_8(S_STEST3,(unsigned char)(TOLERANT_ENABLE | 0x02) );
      }

    return(reenter_modifier);
}

#if USE_POLLING
#else
void SCSI_int_hndlr(void)
{
   unsigned char istat;
// int i;

   istat = IO_READ_8(S_ISTAT);
   if(istat & (ISTAT_DIP | ISTAT_SIP | ISTAT_INTF ) )
     {
      NCR_REG(DSTAT) = NCR_REG(SIST0) = NCR_REG(SIST1) = 0;
      if(istat & ISTAT_INTF)
        {
         data_xfered += current_data_byte_cnt; // signal data transfer occurred
         IO_WRITE_8(S_ISTAT,ISTAT_INTF); // clear the flag
#if SCSI_DEBUG
#ifdef trace
//@****  if(stf) printf("\n=> INTFLY (0x%X bytes transferred)", current_data_byte_cnt);
#endif
#endif
        }
      while ( istat & (ISTAT_DIP | ISTAT_SIP) )
        {
         NCR_REG(ISTAT) |= istat;
         if(istat & ISTAT_DIP)
           {
            NCR_REG(DSTAT) = IO_READ_8(S_DSTAT);
            // check for script aborted and reset abort bit if true
            if(NCR_REG(DSTAT) & 0x10) IO_WRITE_8(S_ISTAT,0x00);
           }
         if(istat & ISTAT_SIP)
           {
            if(istat & ISTAT_DIP)
              {
               scsi_delay(2); /* delay before reading SIST0 */
              }
            NCR_REG(SIST0) = IO_READ_8(S_SIST0);
            scsi_delay(2);  /* delay before reading SIST1 */
            NCR_REG(SIST1) = IO_READ_8(S_SIST1);

            // If a SCSI Bus Reset is detected, set the flag to indicate that it occurred
            // and start a 3 second timer to delay any command to any device on that bus

            if( NCR_REG(SIST0) & 0x02 )
              { bus_reset = 1;
                bus_reset_delay = get_scsi_timeout(3 * 1000);
              }
           }
         scsi_delay(2);  // delay before reading ISTAT again
         istat = IO_READ_8(S_ISTAT);
        }
     }
}
#endif

void Reset_SCSI_Bus(void)
{
 rtc_value reset_timer;

#if SCSI_DEBUG
  if(sdf) printf("\n Resetting SCSI Bus ");
#endif

  NCR_REG(SCNTL1) = IO_READ_8(S_SCNTL1);

  IO_WRITE_8(S_SCNTL1,NCR_REG(SCNTL1) | 0x08);
  scsi_delay(32);

  IO_WRITE_8(S_SCNTL1,NCR_REG(SCNTL1));
  scsi_delay(5);

}

//--------------------------------------------------------------------------//
// Function: ncr_default_setup()                                            //
//--------------------------------------------------------------------------//

void ncr_default_setup(ADAPTER_INFO_PTR adapter_ptr)
{
  register int       i;

#if    ( (NCR_CHIP==720) || (NCR_CHIP==810) )
  #if (NCR_CHIP==810)
  IO_WRITE_8(S_DCNTL,0x09);
  #endif
  IO_WRITE_8(S_SCNTL0,(unsigned char)0xC8);
  IO_WRITE_8(S_SCNTL1,(unsigned char)0x00);
  IO_WRITE_8(S_SCNTL3,(unsigned char)0x33); // 5MB/sec
#if   USE_POLLING
  IO_WRITE_8(S_SIEN0,(unsigned char)0x00);  // Interrupts
  IO_WRITE_8(S_SIEN1,(unsigned char)0x00);  //    all
  IO_WRITE_8(S_DIEN,(unsigned char)0x00);   //  disabled
#else
  IO_WRITE_8(S_SIEN0,(unsigned char)0x8f);  // Enable all
  IO_WRITE_8(S_SIEN1,(unsigned char)0x04);  // SCSI interrupts
  #if (NCR_CHIP==720)
  IO_WRITE_8(S_DIEN,(unsigned char)0x7F);   //
  #endif
  #if (NCR_CHIP==810)
  IO_WRITE_8(S_DIEN,(unsigned char)0x7D);   // Enable all DMA interrupts
  #endif
#endif
  IO_WRITE_8(GPREG,0x0);
  IO_WRITE_8(CTEST0,0x0);
  IO_WRITE_8(CTEST4,0x0);
  IO_WRITE_8(S_SXFER,(unsigned char)0x00);
  IO_WRITE_8(S_DMODE,xfer_length & 0xC0);   // Set bursting size
  IO_WRITE_8(STIME0,0x0B);                  // Selection Timeout = 102ms
  IO_WRITE_8(S_STEST3,(unsigned char)TOLERANT_ENABLE); // TolerANT enabled (?)
  IO_WRITE_8(S_CTEST3,(unsigned char)0x04); // clear fifo's
  IO_WRITE_8(S_SCID, (adapter_ptr->bus_id[0] & 0x07)); // Set adapter's ID
  IO_WRITE_8(S_RESPID0, 1 << (adapter_ptr->bus_id[0] & 0x07)); // Adapter ID for reselection
#endif

  //------------------------------------------------------------------------//
  // clear the siop register save area                                      //
  //------------------------------------------------------------------------//

  for (i=0; i<sizeof(NCR_INFO.ncr_regs); ++i)
    NCR_REG(i)=0;
}


//--------------------------------------------------------------------------//
// Function: ncr_soft_reset()                                               //
// Action:   enable, pause, then disable software reset                     //
//                                                                          //
//                                                                          //
//--------------------------------------------------------------------------//

void ncr_soft_reset(void)
{

  #if ( (NCR_CHIP == 720) || (NCR_CHIP == 810) )
  IO_WRITE_8(S_SIEN0,(unsigned char)0x00);
  IO_WRITE_8(S_SIEN1,(unsigned char)0x00);
  IO_WRITE_8(S_DIEN,(unsigned char)0x00);
  IO_WRITE_8(S_ISTAT,0x40);
  scsi_delay(125);
  IO_WRITE_8(S_ISTAT,0x00);
  scsi_delay(125);
  #endif
}


//--------------------------------------------------------------------------//
// Function: get_script_and_patch()                                         //
//                                                                          //
// Action:   Makes necessary patches to the NCR Script instructions and     //
//           swaps the instructions in memory for little endian processing  //
//           by the NCR Scripts processor                                   //
//                                                                          //
// Returns:  0 on success                                                   //
//          -1 if failure                                                   //
//--------------------------------------------------------------------------//

int get_script_and_patch()
{
  int                ret;
  uint               i;
  uint               size;
  int                offset;

  // If Scripts have already been patched and swapped in memory, just return

  if (SCRIPT_SIG > 1) return(0);

   //--------------------------------------------------------------------//
   // first patch any labels                                             //
   //--------------------------------------------------------------------//

    for (i=0; i< sizeof(LABELPATCHES)/4; i++)
     {
      SCRIPT[LABELPATCHES[i]]+= (address_conversion + (uint)&SCRIPT);
     }

   //--------------------------------------------------------------------//
   // now patch external data buffer addresses                           //
   //--------------------------------------------------------------------//

    //  command buffer info built at run time, no fixup for E_cmd_buf_Used[]
    //  array required here

    //  data buffer info is also constructed at run time, no E_data_buffer_Used[]
    //  array fixups required here

    for (i=0; i< (sizeof E_identify_msg_buf_Used) / 4; i++)
      {
       SCRIPT[E_identify_msg_buf_Used[i]] = address_conversion + (uint)&identify_msg_buf;
      }

    for (i=0; i< (sizeof E_msg_in_buf_Used) / 4; i++)
      {
       SCRIPT[E_msg_in_buf_Used[i]] = address_conversion + (uint)&msg_in_buf;
      }

    for (i=0; i< (sizeof E_status_byte_Used) / 4; i++)
      {
       SCRIPT[E_status_byte_Used[i]] = address_conversion + (uint)&status_byte;
      }

    for (i=0; i< (sizeof E_sync_msg_in_buf_Used) / 4; i++)
      {
       SCRIPT[E_sync_msg_in_buf_Used[i]] = address_conversion + (uint)&sync_msg_in_buf;
      }

    for (i=0; i< (sizeof E_sync_msg_resp_Used) / 4; i++)
      {
       SCRIPT[E_sync_msg_resp_Used[i]] = address_conversion + (uint)&sync_msg_resp;
      }

   //--------------------------------------------------------------------//
   // now swap all script words (NCR needs them little endian)           //
   //--------------------------------------------------------------------//

    for (i=0; i< (sizeof SCRIPT)/4; i++)
     {
      SCRIPT[i]=swap_word(SCRIPT[i]);
     }


    SCRIPT_SIG=swap_word(SCRIPT_SIG);  // flag scripts as being reversed

  return (0);
}

//--------------------------------------------------------------------------//
// Function: controller_diags()                                             //
//                                                                          //
// Action:   Performs a series of self tests on the NCR controller.         //
//                                                                          //
// Returns:  0 on success                                                   //
//           Return code defined in rtcodes.h on failure                    //
//--------------------------------------------------------------------------//

int controller_diags(ADAPTER_INFO_PTR adapter_ptr)
{
 int rc;    // function return code

  rc = mem_test(adapter_ptr);    // Perform memory to memory copy test

 return(rc);
}

//--------------------------------------------------------------------------//
// Function: mem_test()                                                     //
//                                                                          //
// Action:   Perform a series of memory to memory move operations using     //
//           the controller memory to memory move SCRIPT instruction.       //
//                                                                          //
// Returns:  0 on success                                                   //
//           Return code defined in rtcodes.h on failure                    //
//--------------------------------------------------------------------------//
int mem_test(ADAPTER_INFO_PTR adapter_ptr)
{
 // the following moved from scsi_diag.c
 uchar * src;
 uchar * targ;
 int i,rc;
 uchar pass_count,ok_cycles;
 register uint mem_index;

 src = (uchar *)malloc(4 * 1024);  // Allocate 2 4KB buffers for the test
 targ = (uchar *)malloc(4 * 1024);

 // initialize dummy device information record for testing

 device.id  = adapter_ptr->bus_id[0] & 0x0f;    // Set target ID to adapter's ID
 device.lun = 0;                                // Lun value not really req'd

 if( (targ == (uchar *)0xffffffff) || (targ == (uchar *)0x00000000) ||
     ( src == (uchar *)0xffffffff) || ( src == (uchar *)0x00000000) )
   {
    // Buffer(s) not allocated, cannot perform test !
#if SCSI_DEBUG
    if(sdf) printf("\nTarget buffer not allocated\n");
#endif
    rc = NOT_ENOUGH_MEMORY;      // Flag allocation error
   }
 else
   {
    pass_count=1;
    ok_cycles =1;
    rc = 0;

#if SCSI_DEBUG
    for( i=(4 * 1024); i; i/=2)  // divide count by 2 for each pass
#else
    for( i=(4 * 1024); (i && !rc); i/=2)  // divide count by 2 for each pass
#endif
      {
       for(mem_index=0;mem_index<i;mem_index++) src[mem_index]=pass_count;

       // copy i bytes from src to targ
#if SCSI_DEBUG
       if(sdf) printf("\nCopying %d bytes from memory address 0x%X to 0x%X",i,(uint)src,(uint)targ);
#endif

       NCR_INFO.scsi_script_ptr=(uint *)Ent_mem_test;
       NCR_INFO.scsi_script_type='T';

       // Modify memory to memory move Script instruction with current test parameters

       for(mem_index=0;mem_index<(sizeof E_mem_bytes_Used) / 4;mem_index++)
         {
          SCRIPT[E_mem_bytes_Used[mem_index]] &= 0xff;     // strip off current byte count value
          SCRIPT[E_mem_bytes_Used[mem_index]] |= swap_word(i & 0x00ffffff);
         }

       for(mem_index=0;mem_index<(sizeof E_mem_source_Used) / 4;mem_index++)
         {
          SCRIPT[E_mem_source_Used[mem_index]] = swap_word( ((uint)src) + address_conversion);
         }

       for(mem_index=0;mem_index<(sizeof E_mem_target_Used) / 4;mem_index++)
         {
          SCRIPT[E_mem_target_Used[mem_index]] = swap_word( ((uint)targ) + address_conversion);
         }

       for(mem_index=0; mem_index<i;mem_index++) targ[mem_index]=0;

       rc = ncr_execute( adapter_ptr,
                         &device,       // Point to dummy device info record
                         0,             // No retries required
                         5);            // Allow 5 seconds (shouldn't take nearly that long)
#if SCSI_DEBUG
       if(rc)
         {
          if(sdf) printf("\nMem Test cmd failed rc %d",rc);
         }
       else
#else
       if(!rc)
#endif
         {
          for(mem_index=0;mem_index<i;mem_index++)
            {
             if(targ[mem_index]!=pass_count)
               {
#if SCSI_DEBUG
                 if(sdf)
                   {
                    printf("\nData miscompare");
                    printf("\n  Source\n");
                    mem_dump(src,i);
                    printf("\n  Destination\n");
                    mem_dump(targ,i);
                   }
#endif
                 rc=1;
                 break;
               }
            }
          if(!rc) ok_cycles++;
         }
       pass_count++;
      }
    free(src);
    free(targ);
#if SCSI_DEBUG
    if(sdf && (pass_count==ok_cycles)) printf("\nData Verified");
#endif
   }
 return(rc);
}

#if SCSI_DEBUG
void print_cmd(void)
{
uchar *cmd_buf_ptr;
uint i,j,x;

     if(sdf)
       for (i=0; i< (sizeof E_cmd_buf_Used)/4; i++)
         {
          cmd_buf_ptr = (uchar *) (swap_word(SCRIPT[E_cmd_buf_Used[i]]) - address_conversion);
          j = swap_word(SCRIPT[E_cmd_buf_Used[i]-1]) & 0x00ffffff;
          for(x=0 ;x<j ;x++)
            {
              printf("%02X ",cmd_buf_ptr[x]);
            }
         }
}

void print_script(void)
{
uint   x;
ushort offset;

       printf("\n\n");
       offset = 0;
       for (x=0;x<(sizeof SCRIPT)/4;x+=2)
         { if ( (x & 3) == 0)
             { printf("  %03X  ",offset); }
           else
             { printf("       "); }
           printf("%08X    %08X\n",swap_word(SCRIPT[x]),swap_word(SCRIPT[x+1]));
           offset+=8;
         };
//     print_cmd();
//     offset = getchar();
       printf("\n");
}

void print_regs(void)
{
 unsigned int i;

 for (i=0;i<0x60 ;i++ )
    {
     printf(" %X",IO_READ_8(i+0x80));
    }
 wait_prompt();
}

void print_stat(void)
{
  unsigned int x,y;   /*DAKOTA*/

  if(sdf)
    {
     x=(uint)NCR_DREG(DSP) - address_conversion -(uint)&SCRIPT;
     y=(uint)NCR_INFO.scsi_script_ptr;
//   x-=y;

     printf("\nScr @ %08X ",( (uint)NCR_INFO.scsi_script_ptr + &SCRIPT) );
     printf("DSP = %08X (%02X), DSPS = %08X\n",NCR_DREG(DSP),x,NCR_DREG(DSPS));

     printf("DSTAT = %02X, SSTAT0 = %02X, SSTAT1 = %02X, ",NCR_REG(DSTAT),NCR_REG(SSTAT0),NCR_REG(SSTAT1));
     printf("DBC = %06X, DNAD = %08X\n",(swap_word(IO_READ_32(S_DBC)) & 0x00ffffff),
                                         swap_word(IO_READ_32(S_DNAD)) );

     printf("SIST0 = %02X, SIST1 = %02X, ",NCR_REG(SIST0),NCR_REG(SIST1));
     NCR_REG(SBCL)=IO_READ_8(S_SBCL);
     printf("SBCL = %02X, DFIFO %02X",NCR_REG(SBCL),IO_READ_8(S_DFIFO));

//   print_script();
     printf("\n");
    }
}
#endif
#endif /*DEBUG*/
