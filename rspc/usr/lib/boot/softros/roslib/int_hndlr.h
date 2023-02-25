/* @(#)37	1.1  src/rspc/usr/lib/boot/softros/roslib/int_hndlr.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:26  */
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
//************************** OCO SOURCE MATERIALS *****************************
//******* IBM CONFIDENTIAL (IBM CONFIDENTIAL-RESTRICTED when combined *********
//************** with the aggregated modules for this product) ****************
//*****************************************************************************
// FILE NAME:          int_hndlr.h                                            
// FILE CREATED:       05/28/93 by D. Cronk                                  
// FILE OWNER:         D. Cronk                                               
//                                                                             
// COPYRIGHT:          (C) Copyright IBM Corp. 1993, 1993                      
//                                                                             
// DESCRIPTION: This file contains public routine prototypes and equates for   
//              the functions in int_hndlr.c                              
//                                                                             
// REVISION HISTORY:                                                           
// -----------------------Sandalfoot-----------------------------------------  
// Date      Track#     Who  Description                                          
// --------  ---------- ---  ------------------------------------------------
//                                                                             
//*****************************************************************************

#ifndef _H_INT_HNDLR
#define _H_INT_HNDLR
/*---------------------------------------------------------------------------*/
/*               Public function prototypes....                              */


void init_interrupts(void);
unsigned int proc_int_registrar(unsigned int service_type, 
                                unsigned int interrupt_vector, 
                                unsigned int *handler_address);
unsigned int ext_int_registrar(unsigned int service_type, 
                               unsigned char interrupt_number, 
                               unsigned int *device_handler_address);



/*---------------------------------------------------------------------------*/
/*               Common equate definitions....                               */

   /* Some common equates used to specify interrupt vectors */

#define MIN_PROCESSOR_VECTOR    0x00000100      /* Minimum and maximum processor vector */
#define MAX_PROCESSOR_VECTOR    0x00002000      /* values.                              */

#define MIN_SUPERVISOR_VECTOR   0x00001000      /* Minimum and maximum supervisor vector */ 
#define MAX_SUPERVISOR_VECTOR   0x00001FE0      /* values.                               */ 
#define NUM_SUPERVISOR_VECTOR   (MAX_SUPERVISOR_VECTOR - MIN_SUPERVISOR_VECTOR) / 0x20

#define EXT_INT_VECTOR          0x00000500      /* External interrupt and decrementer    */
#define DEC_INT_VECTOR          0x00000900      /* interrupt vector values.              */

   /* Request types for external interrupt registrar routine */

#define GET_INTERRUPT_HANDLER_ADDRESS      1
#define ASSIGN_INTERRUPT_HANDLER_ADDRESS   2
#define RELEASE_INTERRUPT_HANDLER_ADDRESS  3

   /* Return codes for external interrupt registrar routine */

#define GOOD_RC                         0
#define INT_OUT_OF_RANGE                1
#define INT_NOT_TAKEN                   2
#define INT_TAKEN                       3
#define BAD_SERVICE                     4
#define BAD_ADDRESS                     5
#define INTS_NOT_INITIALIZED            6

#endif  /* _H_INT_HNDLR */
