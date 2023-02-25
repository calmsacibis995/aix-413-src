/* @(#)85	1.2  src/rspc/kernext/pcmcia/cs/Evh.h, pcmciaker, rspc41J, 9509A_all 2/20/95 17:33:06  */
/*
 *   COMPONENT_NAME:  PCMCIAKER
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#if !defined( Evh_h )
#define Evh_h

#include <sys/pcmciacs.h>

#if !defined( EvhP_h )
typedef int Client;
#endif

#define MAX_EVENTS     40
#define MAX_USR_EVENTS 20
#define MAX_SLOTS      16

#define NO_SLOT (-1)

/* client types */
#define CL_TYPE1           CSCliAttrIOCDD
#define CL_TYPE2           CSCliAttrMTD
#define CL_TYPE3           CSCliAttrMCDD
#define CL_TYPE_LAST       (0x10000)
#define CL_USR             (0x20000)
#define CL_SHR             CSCliAttrSHR
#define CL_EXCL            CSCliAttrEXCL

/* return values */
#define EVR_SUCCESS        CSR_SUCCESS
#define EVR_BAD_CLIENT     CSR_BAD_HANDLE
#define EVR_BAD_SLOT       CSR_BAD_SOCKET
#define EVR_IN_USE         CSR_IN_USE
#define EVR_NO_CARD        CSR_NO_CARD
#define EVR_BUSY           CSR_BUSY

/* event masks */
#define EVM_CDChg          CSEvMaskCDChg
#define EVM_ReadyChg       CSEvMaskReadyChg

/* event sources - an event source may cause multiple events */
#define EVH_KILL           (-1)
#define EVH_HWNOTIFY       (-2)
#define EVH_REGISTRATION   CSE_REGISTRATION_COMPLETE
#define EVH_INSERTION      CSE_CARD_INSERTION
#define EVH_REMOVAL        CSE_CARD_REMOVAL
#define EVH_READY          CSE_CARD_READY
#define EVH_CLIENT_INFO    CSE_CLIENT_INFO
#define EVH_EXCLUSIVE      CSE_EXCLUSIVE_REQUEST
#define EVH_PM_SUSPEND     (0xc)
#define EVH_PM_RESUME      (0xb)
#define EVH_SHARE          (-3)
#define EVH_CFGDEV         (-4)


/* events send to clients */
#define EV_REGISTRATION_COMPLETE  CSE_REGISTRATION_COMPLETE
#define EV_CARD_INSERTION         CSE_CARD_INSERTION
#define EV_CARD_REMOVAL           CSE_CARD_REMOVAL
#define EV_CARD_READY             CSE_CARD_READY
#define EV_CLIENT_INFO            CSE_CLIENT_INFO
#define EV_EXCLUSIVE_REQUEST      CSE_EXCLUSIVE_REQUEST
#define EV_EXCLUSIVE_COMPLETE     CSE_EXCLUSIVE_COMPLETE

/* broadcast type */
#define ALL       1
#define SLOT      2
#define REQUESTER 3
#define TARGET    4

/* return value of getstatus */
#define SSTAT_CARD_EXIST     1  /* used on registration */
#define SSTAT_CARD_INSERTION 2
#define SSTAT_CARD_REMOVAL   4
#define SSTAT_CARD_READY     8


/* flag in evhcoreadd() */
#define ACTION_DEFAULT    0
#define ACTION_APPEND     1
#define ACTION_OVERWRITE  2

/* environment category */
#define ENV_STOP         (-1)
#define ENV_CFG          0
#define ENV_INTR         1
#define ENV_EVH          2
#define ENV_CLI          3

/* flags for a slot */
#define SLOT_SUSPENDED   1

/* flags for Usr_enqueue() */
#define KEEP_PREVIOUS_EVENTS       0
#define FLUSH_EVENTS_ON_THE_SLOT   1
#define FLUSH_ALL_EVENTS           2

extern int svccfehRegisterHandler( void );
extern int svccfehDeregisterHandler( int cl );
extern int svccfehGetEvent( int cl, char* devn, int devsize,
			   int* slot, int* eventSrc );

#endif /* Evh_h */




