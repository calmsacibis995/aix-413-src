/* @(#)45	1.2  src/rspc/usr/lib/boot/softros/roslib/scsi_api.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:52:17  */
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

#ifdef SCSI_API
#define api_extern
#else
#define api_extern extern
#endif

#define SUPPORTED_ADAPTERS 1    // only 1 adapter currently supported
#define SUPPORTED_DEVICES  56   // Max. of 7 is more likely

//-----------------------------------------//
// Configuration table definitions         //
//-----------------------------------------//

// List of supported "adapter" types
#define NCR_SUPPORTED TRUE
#define KEY_SUPPORTED FALSE

#pragma options align=packed enum=small

typedef enum adapter_types { No_Adapter,
#if NCR_SUPPORTED
                             NCR,
#endif
#if KEY_SUPPORTED
                             Key_West
#endif
                   } ADAPTER_TYPES;

typedef struct { unsigned char Device;
                 unsigned char Bus;
               } PCI_INFO;

typedef union { PCI_INFO PCI_Addr;
                unsigned short ISA_IO_Addr;
                unsigned short ID;
                unsigned short Slot;
              } SYS_BUS_ID;

typedef enum busses { Local_Bus,
                      PCI_Bus,
                      ISA_Bus,
                      MicroChannel_Bus,
                      PCMCIA_Bus
                    } BUSSES;

typedef struct { ADAPTER_TYPES type;
                 BUSSES        sys_bus;
                 SYS_BUS_ID    sys_id;
               } ADAPTER_HANDLE;

typedef struct { int Open : 1;
                 int Locate : 1;
               } ADAPTER_STATUS;

typedef struct { ADAPTER_HANDLE handle;           // offset 0
                 unsigned long  Base_IO_Addr;     //        4
                 unsigned char  max_lun;          //        8
                 unsigned char  max_id;           //        9
                 unsigned char  max_bus;          //        A
                 unsigned char  bus_id[8];        //    B->12
                 ADAPTER_STATUS status;           //       13

                 // To allow structures within the private area to be naturally
                 // aligned, need to guarantee 'private' is dword aligned.  It
                 // is assumed that the handle is dword aligned.

                 unsigned char  private[1];       // offset 14

                 // To guarantee additional adapter records are properly aligned
                 // code that builds the configuration table in memory calculates
                 // the location of the second (and subsequent) adapter entries
                 // to ensure each begins on a dword aligned boundary.

               } ADAPTER_INFO_REC, *ADAPTER_INFO_PTR;


typedef struct device_status { int Ready : 1;
                               int Rem_Media : 1;
                               int Cap_Valid : 1;
                               int Negotiate : 1;
                               int Adapter_Addr_Sig : 1;
                               int Adapter_Bus_Sig : 1;
                               int Device_LUN_Sig : 1;
                             } DEVICE_STATUS;

typedef struct { unsigned long max_lba;        // offset 0
                 unsigned long block_size;     //        4
                 unsigned char type;           //        8
                 unsigned char adapter;        //        9
                 unsigned char bus;            //        A
                 unsigned char id;             //        B
                 unsigned char lun;            //        C
                 DEVICE_STATUS status;         //        D
                 unsigned char pad[2];         //        E->F

                 // Make sure structure size is dword multiple to ensure
                 // max_lba always dword aligned for all devices

               } DEV_INFO_REC, *DEVICE_INFO_PTR;

#pragma options align=reset

typedef struct { unsigned short ADAPTER_DATA_SIZE;
                 unsigned char  ADAPTER_COUNT;
                 unsigned short DEVICE_COUNT;
                 unsigned int   int_expected;
                 unsigned int   int_flag;
                 ADAPTER_INFO_REC ADAPTER[1 /* ADAPTER_COUNT */ ];
                 DEV_INFO_REC  DEVICE[1 /* DEVICE_COUNT */ ];
               } SCSI_CONFIG_TABLE, *SCSI_CONFIG_TABLE_PTR;


// Function prototypes and returned data structure definitions

extern void scsi_init(void);

typedef enum scsi_ops { Read,              // Read from specified LBA
                        Write,             // Write to specified LBA
                        Test_Unit_Ready,   // Check if device ready for media accesses
                        Start_Motor,       // Start Disk spindle motor - no wait
                        Adapter_Query,     // Return specific adapter information (and adapter count)
                        Device_Query,      // Return specific device information (and device count)
                        Lock,              // Prevent media removal
                        Unlock,            // Allow media removal
                        Reserve,           // Request exclusive device read/write access
                        Release,           // Allow other initiator (system) device read/write access
                        Get_Type_Count,    // Get count of device type (DEVICE_TYPES) attached
                        Get_LDN,           // Get device number for Nth instance of device type
                        Get_LDN_SIG,       // Get unique device signature for boot device identifier
                        Find_LDN_SIG,      // Get device number for specified (boot device) signature
                        Send,              // Passthrough interface for Diagnostics
                        Adapter_Diags      // Execute self test on specified controller
                      } SCSI_OPS;

typedef enum device_types { DASD,          // Both Disk and removable media Dasd
                            TAPE,          // Any Sequential access type device
                            PRINTER,       // 'nuf said ?
                            PROCESSOR,     // Typically another initiator (twin-tailed Spock/Tribble/etc...)
                            WORM,          // Write-Once-Read-Multiple type devices
                            CDROM,         // CD Rom devices, no distinction between data types supported
                            SCANNER,       // Optical scanner devices
                            OPTICAL,       // Read/Write Optical devices
                            CHANGER,       // Media changer devices, typically services other devices like Opticals
                            COMM,          // Communications devices (ie. LAN adapters)
                          } DEVICE_TYPES;


                                                            // (n) - see Notes below for non-intuitive usage of field
extern unsigned int scsi_io ( SCSI_OPS       command,       // Command to be executed
                              char          *read_buff,     // Buffer for data transferred or Command Descriptor
                                                            //  for Send command
                              int            device,        // (1) Device number for device commands, Adapter number
                                                            //  for adapter commands
                              int            lba,           // (2) Target block address and
                              int            lba_count);    // (3)   count of blocks affected by command
//
// Notes on non-intuitive use of certain scsi_io parameters
//
//  1 - device number is actually the adapter number for Adapter_Query and Adapter_Diags commands.  For
//      Get_Type_Count and Get_LDN functions, device number is interpreted as a device type (DEVICE_TYPES).
//      The field is ignored for Find_LDN_SIG.
//  2 - For device Read and Write, this field adheres to its intuitive meaning, for a Send command, lba
//      is used as a retry count value.  For the Get_LDN command, the device number is returned for the
//      instance (1 based) of the device type specified in the lba field.
//  3 - For the Send command, lba_count is the command timeout value

#pragma options align=packed
typedef struct adapter_q_rec { ADAPTER_HANDLE handle;
                               unsigned long  Base_IO_Addr;
                               unsigned char  Adapter_Count;
                               unsigned char  max_lun;
                               unsigned char  max_id;
                               unsigned char  max_bus;
                               unsigned char  bus_id[8];
                               ADAPTER_STATUS status;
                               unsigned char *VPD;
                             } ADAPTER_Q_REC;

typedef struct device_q_rec { unsigned long max_lba;
                              unsigned long  block_size;
                              unsigned short Device_Count;
                              unsigned char  type;
                              unsigned char  adapter;
                              unsigned char  bus;
                              unsigned char  id;
                              unsigned char  lun;
                              DEVICE_STATUS  status;
                              struct { unsigned char vendor[8];
                                       unsigned char spc1; // Space separator
                                       unsigned char product_name[16];
                                       unsigned char spc2; // Space separator
                                       unsigned char revision[4];
                                       unsigned char string_term;  // terminator for VPD string
                                     } VPD;
                            } DEVICE_Q_REC;
#pragma options align=reset

//// extern unsigned int scsi_read( char *read_buff,
////                                  int drive,
////                                  int lba,
////                                  int lba_count);
////
//// extern unsigned int scsi_write( char *write_buff,
////                                   int drive,
////                                   int lba,
////                                   int lba_count);
////
//// extern unsigned int scsi_lock( char *read_buff,
////                                  int drive,
////                                  int lba,
////                                  int lba_count);
////
//// extern unsigned int scsi_unlock( char *read_buff,
////                                    int drive,
////                                    int lba,
////                                    int lba_count);
