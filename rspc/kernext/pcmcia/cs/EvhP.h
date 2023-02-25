/* @(#)25	1.3  src/rspc/kernext/pcmcia/cs/EvhP.h, pcmciaker, rspc41J, 9509A_all 2/20/95 17:33:08  */
/*
 *   COMPONENT_NAME: PCMCIAKER
 *
 *   FUNCTIONS: Event Handler private data structures definition
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
#if !defined( EvhP_h )
#define EvhP_h

#include <sys/sleep.h>
#include <sys/m_param.h>
#include <sys/lock_def.h>
#include <sys/malloc.h>
#include <sys/types.h>
#include "Evh.h"


typedef struct _Client{
  int valid;
  int calling;
  unsigned int attributes;
  unsigned int eventMask;
  int slot[4];  /* slot which the client gets resources from */
  int slot_refcount[4];
  char* clientData;
  int (*callback)();
  int (*destruct)();
  Simple_lock lock; /* lock for this client members */
  struct _Client* next;
  char* ext;               /* extension for future use */
  char* child_link;        /* extension for future use */
} Client;

typedef struct{
  Client* clients;        /* client list */
  char* ext;               /* extension for future use */
  char* child_link;        /* extension for future use */
} ClientClass;

typedef struct _SlotMask{
  Client* client;
  unsigned int eventMask;
  int slot;
  int valid;
  struct _SlotMask* next;
} SlotMask;

typedef struct _EventAction{
  int eventSrc;
  unsigned int eventMask;  /* event mask associated with this eventsrc */
  void (*action)();        /* Action function where actual events are sent */
  struct _EventAction* next;
} EventAction;

typedef struct _EventActionList{
  int ref_count;
  EventAction* eventActions;
  struct _EventActionList* next;
} EventActionList;

typedef struct{
  EventActionList* all;
} EventActionListClass;

typedef struct _EventQueue{
  int eventSrc;
  int slot;
  Client* requester;
  Client* target;
  struct _EventQueue* next;
} EventQueue;

typedef char* IEventQueue;

typedef struct _Usr_Client{
  struct _Client client;
  tid_t tid;
  struct _EventQueue* read;
  struct _Usr_Client* next;
/***/
#if 0
  static Usr_Client* const_Usr_Client( Usr_Client* cl, tid_t tid );
  static int register_Usr_Client( Usr_Client* cl );
  static void dest_Usr_Client( clientID );
  static int get_event_for_Usr_Client( clientID, &ev );
#endif
} Usr_Client;

typedef struct _Usr_ClientClass{
  struct _Usr_Client* usr_clients;
  Simple_lock queuelock;
  int    newevent;
  struct _EventQueue* free_queues;
  struct _EventQueue* used_queues;
  struct _EventQueue usr_eventQueues[ MAX_USR_EVENTS ];
/***/
#if 0
  static void init_Usr_ClientClass();
  static int usr_enqueue( EventQueue* q, int flag );
  /* flag == FLUSH_EVENTS_ON_THE_SLOT or KEEP_PREVIOUS_EVENTS */
#endif
} Usr_ClientClass;

typedef struct _EvhCore{
  Simple_lock lock;
  int slot;          /* logical socket number in card services */
  dev_t devno;       /* device number of the detecter */
  char devname[16];  /* logical device name in ODM */
  EventActionList* eventActionLists;
  EventActionList* eventActionLists2;
  SlotMask*  slotMasks;
  int (*getstatus)();
  Client* RequestingExclusiveUser;
  Client* ExclusiveUser;
  struct _EvhCore* next;
  int flag;              /* used for SLOT_SUSPENDED */
  char* ext;             /* extension for future use */
  char* child_link;      /* extension for future use */
} EvhCore;

typedef struct{
  EvhCore* evhCores;     /* list of instances */
  int kpid;              /* kernel process ID for event handler */
  int stopping_flag;     /**/
  int configproc_word;   /* event word for config process */
  int evhproc_word;      /* event word for event handler process */
  EventQueue* read;
  EventQueue* write;
  IEventQueue* iread;
  IEventQueue* iwrite;
  IEventQueue  ieventQueues[ MAX_SLOTS ];
  Simple_lock queuelock;
  Simple_lock iqueuelock;
  Complex_lock configlock;
  int wakeup_word;       /* event word for blocked requesters of PM suspend/resume */
  char* ext;            /* extension for future use */
  char* child_link;     /* extension for future use */
} EvhCoreClass;


#endif /* EvhP_h */


