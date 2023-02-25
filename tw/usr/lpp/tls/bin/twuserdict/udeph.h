/* @(#)74	1.1  src/tw/usr/lpp/tls/bin/twuserdict/udeph.h, tw, tw411, GOLD410 7/10/92 14:01:41 */
/*
 * COMPONENT_NAME :     (CMDTW) -  Traditional Chinese Dictionary Utility
 *
 * FUNCTIONS :          udeph.h
 *
 * ORIGINS :            27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Include error message */
# include "udepherr.h"

/************************/
/* constant definitions */
/************************/

/* default phonetic system dictionary file path and name */
/* this file is as same as the file that input method access */
#define SYSDICT "/usr/lpp/tls/dict/phsysdict"

/* The maximum number of phonetic code for every word */
#define MAX_PHONETIC_NUM 14

/* The maximum number of phrase code for every word */
#define MAX_PHRASE_NUM 17

/* The maximumr space for phonetic code */
#define PHONETIC_LEN  MAX_PHONETIC_NUM*2+1

/* The maximum space for phrase code */
#define PHRASE_LEN MAX_PHRASE_NUM*4+1

#define Add_Phrase      201
#define Delete_Phrase   202
#define Update_Phrase   203
#define Delete_Phonetic 204
#define Update_Phonetic 205
#define Phonetic_Search 206
#define Merge_Dict      207
#define Open_Dict       208

#define WaitTime        3
#define ERROR           (-1)

#define OK                   2
/* The number of error message */
#define ERR_WR               4
#define ERR_PHRASE           5
#define ERR_PHONETIC         6
#define EX_IN_SYS            7
#define EX_IN_USR            8
#define ERR_GF               9
#define EX_IN_NON           10
#define SAME_US             11
#define SAME_MS             12
#define SAME_UM             13
#define ERR_IF              14
#define ERR_AP              15
#define ERR_OP              16
#define ERR_IV              17
#define ERR_SO              18
#define ERR_MF              19
#define ERR_IC              20
#define ERR_LOCK            21
#define ERR_UNLOCK          22
#define ERR_PHRASE_EMPTY    23
#define ERR_PHONETIC_EMPTY  24
#define ERR_EMPTY           25
#define PHONETIC_LONG       26
#define PHRASE_LONG         27
#define PHRASE_SYS_DUP      28
#define FILE_EMPTY          29

/* The end mark of Master Index */
#define PH_MI_END_MARK      0xffff

/* The End Mark of Dictionary Record */
#define PH_DR_END_MARK      0xffff

/* The end mark of Group */
#define PH_GP_END_MARK      0xff

/* The number of bytes of the end mark of Master Index */
#define miend_len           2

/* The number of bytes of the end mark of Dictionary Record */
#define drend_len           2

/* The number of bytes of the end mark of Group */
#define gpend_len           1

/* The number of bytes of every RRN in MI */
#define RRN_LENGTH          4


/* The maximum length of Dictionary Record */
#define DR_MAX_LENGTH       2048

/* The maximum length of Master Index */
#define MI_MAX_LENGTH       2560

/* The maximum length of file name */
#define PATHLEN             256

/* for while loop condition to be forever */
#define LOOP                1

/* Indicator of user dictionary */
#define USR                 100

/* Indicator of merge dictionary */
#define MERGE               101

/* indicate larger than */
#define LARGE               110

/* indicate equal to */
#define EQUAL               111

/* indicate less than */
#define LESS                112

/* Flag of exist for system call access() */
#define EXIST               0

/* Flag of write permission for system call access(). C language default value*/
#define WRITE               2

/* Flag of read permission for system call access(). C language default value */
#define READ                4


/* Data structure definition */

typedef struct {
   char  *last_modified_time;
   char  *name;                        /* dictionary file name                */
   int   flag;                         /* type of access required for dict    */
   int   fd;                           /* file description                    */
   char  *mi;                          /* master index                        */
   int   mi_freespace;                 /* free space of mi                    */
   char  *indexkey;                    /* current index key                   */
   int   indexkey_ptr;                 /* point of index key                  */
   int   indexkey_len;                 /* length of index key                 */
   long  int indexkey_rrn;             /* rrn relative to current index key   */
   char  *dr;                          /* dictionary record                   */
   int   dr_freespace;                 /* free space of DR                    */
   char  *groupkey;                    /* current group key                   */
   int   groupkey_ptr;                 /* point of group key                  */
   int   groupkey_len;                 /* length of group key                 */
   long  int rrn;                      /* rrn of current DR                   */
   int   word_ptr;                     /* point of current word               */
} DictInfo;

typedef struct {
   int candnum;                        /* number of candidates                */
   int highlight;                      /* the highlight candidate             */
   char **candidate;                   /* pointer to all the candidates string*/
}  SelectInfo;

typedef struct {
   char *phonetic_entry;               /* phonetic entry field                */
   char *phrase_entry;                 /* phrase entry field                  */
   char *open_entry;                   /* open entry field                    */
   char *merge_entry;                  /* merge entry field                   */
   char *phonetic_view;                /* phonetic viewing area               */
   SelectInfo *phrase_select;          /* phrase select window                */
   DictInfo *sysdict;                  /* system dictionary control block     */
   DictInfo *usrdict;                  /* user dictionary control block       */
   DictInfo *mergedict;                /* merge dictionary control block      */
   DictInfo *tempdict;                 /* temp dictionary control block       */
}  UDEobject;

typedef struct {
   int errno ;                         /* # of error message                  */
   char *errmsg;                       /* error message                       */
}  ErrorCode;

UDEobject *obj;                        /* User Dict Editor control block      */
