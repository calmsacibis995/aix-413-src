/* @(#)21	1.3  src/rspc/kernext/pm/pmmd/IBM8301/pdplanar.h, pwrsysx, rspc41J, 9516A_all 4/18/95 08:46:38  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Planar Device Driver Common Control Definitions
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

/* ************************************************************************* */
/* *                                                                       * */
/* * Module Name: PDPLANAR.H                                               * */
/* * Description: Planar Device Driver Common Control Definitions          * */
/* * Restriction:                                                          * */
/* *                                                                       * */
/* ************************************************************************* */
#ifndef  _H_PDPLANAR
#define  _H_PDPLANAR


/* ************************************************************************* */
/* *                         Include Files Section                         * */
/* ************************************************************************* */

/* ************************************************************************* */
/* *                   Data/Structure Definition Section                   * */
/* ************************************************************************* */
/* ************************************* */
/* *          Data Definition          * */
/* ************************************* */
#ifndef  TRUE
#define  TRUE     (1)
#endif

#ifndef  FALSE
#define  FALSE    (0)
#endif

#ifndef  YES
#define  YES      (1)
#endif

#ifndef  NO
#define  NO       (0)
#endif

#ifndef  ON
#define  ON       (1)
#endif

#ifndef  OFF
#define  OFF      (0)
#endif

#ifndef  FAKE
#define  FAKE     (-1)
#endif

#ifndef  NULL
#define  NULL     ((void *)0)
#endif

/* ************************************* */
/* *          Type Definition          * */
/* ************************************* */
typedef unsigned int       UINT;
typedef unsigned int *     PUINT;

typedef unsigned char      BYTE;
typedef unsigned char *    PBYTE;

typedef unsigned short     WORD;
typedef unsigned short *   PWORD;

typedef unsigned long      DWORD;
typedef unsigned long *    PDWORD;

typedef void               VOID;
typedef void *             PVOID;

typedef unsigned char      UCHAR;
typedef unsigned char *    PUCHAR;

typedef unsigned long      ULONG;
typedef unsigned long *    PULONG;

/* ************************************* */
/* *     Common Message Definition     * */
/* ************************************* */
/* ---------------------- */
/*  Message member types  */
/* ---------------------- */
typedef unsigned short     CMESSAGE;                  /* Common message type */
typedef unsigned short *   PCMESSAGE;

typedef unsigned short     CRESULT;                    /* Common result type */
typedef unsigned short *   PCRESULT;

typedef unsigned short     CPARAMETER;              /* Common parameter type */
typedef unsigned short *   PCPARAMETER;

/* -------------------------- */
/*  Message packet structure  */
/* -------------------------- */
typedef struct                                      /* Common message packet */
{
   CMESSAGE    cmMessage;                          /* Message ID to function */
   CRESULT     crResult;                             /* Result from function */
   CPARAMETER  pcpParameter[1];                /* Parameter to/from function */
} MSGCOMMON;
typedef MSGCOMMON *PMSGCOMMON;

/* ************************************* */
/* *     Common Object Definition      * */
/* ************************************* */
/* -------------------- */
/*  Object member type  */
/* -------------------- */
typedef VOID (*POBJCTL)(PMSGCOMMON, PVOID);        /* Object controller type */

/* ------------------ */
/*  Object structure  */
/* ------------------ */
typedef struct                                              /* Common object */
{
   POBJCTL  pfnControl;                      /* Pointer to Object controller */
} OBJCOMMON;
typedef OBJCOMMON *POBJCOMMON;

/* ************************************* */
/* * Object/Message Control Definition * */
/* ************************************* */
/* ------------------------ */
/*  Message control macros  */
/* ------------------------ */
#define  SendMsg(pkt, obj)                                         \
            (((POBJCOMMON)&(obj))->pfnControl)((PMSGCOMMON)&(pkt), \
            (POBJCOMMON)&(obj))

#define  BuildMsg(pkt, msg)                                              \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);

#define  BuildMsgWithParm1(pkt, msg, parm1)                                 \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);    \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1);

#define  BuildMsgWithParm2(pkt, msg, parm1, parm2)                           \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);     \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 1) = (CPARAMETER)(parm2);

#define  BuildMsgWithParm3(pkt, msg, parm1, parm2, parm3)                    \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);     \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 1) = (CPARAMETER)(parm2); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 2) = (CPARAMETER)(parm3);

#define  BuildMsgWithParm4(pkt, msg, parm1, parm2, parm3, parm4)             \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);     \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 1) = (CPARAMETER)(parm2); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 2) = (CPARAMETER)(parm3); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 3) = (CPARAMETER)(parm4);

#define  BuildMsgWithParm5(pkt, msg, parm1, parm2, parm3, parm4, parm5)      \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);     \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 1) = (CPARAMETER)(parm2); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 2) = (CPARAMETER)(parm3); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 3) = (CPARAMETER)(parm4); \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 4) = (CPARAMETER)(parm5);

#define  BuildMsgWithParm6(pkt, msg, parm1, parm2, parm3, parm4, parm5, parm6) \
              ((PMSGCOMMON)&(pkt))->cmMessage         = (CMESSAGE)(msg);       \
            *(((PMSGCOMMON)&(pkt))->pcpParameter)     = (CPARAMETER)(parm1);   \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 1) = (CPARAMETER)(parm2);   \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 2) = (CPARAMETER)(parm3);   \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 3) = (CPARAMETER)(parm4);   \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 4) = (CPARAMETER)(parm5);   \
            *(((PMSGCOMMON)&(pkt))->pcpParameter + 5) = (CPARAMETER)(parm6);

#define  GetParm1(pkt, parm1)                                                 \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);

#define  GetParm2(pkt, parm1, parm2)                                           \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);

#define  GetParm3(pkt, parm1, parm2, parm3)                                    \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);\
            *((PCPARAMETER)&(parm3)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+2);

#define  GetParm4(pkt, parm1, parm2, parm3, parm4)                             \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);\
            *((PCPARAMETER)&(parm3)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+2);\
            *((PCPARAMETER)&(parm4)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+3);

#define  GetParm5(pkt, parm1, parm2, parm3, parm4, parm5)                      \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);\
            *((PCPARAMETER)&(parm3)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+2);\
            *((PCPARAMETER)&(parm4)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+3);\
            *((PCPARAMETER)&(parm5)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+4);

#define  GetParm6(pkt, parm1, parm2, parm3, parm4, parm5, parm6)               \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);\
            *((PCPARAMETER)&(parm3)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+2);\
            *((PCPARAMETER)&(parm4)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+3);\
            *((PCPARAMETER)&(parm5)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+4);\
            *((PCPARAMETER)&(parm6)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+5);

#define  GetParm7(pkt, parm1, parm2, parm3, parm4, parm5, parm6, parm7)        \
            *((PCPARAMETER)&(parm1)) = *(((PMSGCOMMON)&(pkt))->pcpParameter);  \
            *((PCPARAMETER)&(parm2)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+1);\
            *((PCPARAMETER)&(parm3)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+2);\
            *((PCPARAMETER)&(parm4)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+3);\
            *((PCPARAMETER)&(parm5)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+4);\
            *((PCPARAMETER)&(parm6)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+5);\
            *((PCPARAMETER)&(parm7)) = *(((PMSGCOMMON)&(pkt))->pcpParameter+6);

#define  SelectMessage(pkt)   ((PMSGCOMMON)&(pkt))->cmMessage

#define  SelectResult(pkt)    ((PMSGCOMMON)&(pkt))->crResult

#define  SelectParm1(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter)

#define  SelectParm2(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 1)

#define  SelectParm3(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 2)

#define  SelectParm4(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 3)

#define  SelectParm5(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 4)

#define  SelectParm6(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 5)

#define  SelectParm7(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 6)

#define  SelectParm8(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 7)

#define  SelectParm9(pkt)     *(((PMSGCOMMON)&(pkt))->pcpParameter + 8)

#define  SelectParm10(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 9)

#define  SelectParm11(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 10)

#define  SelectParm12(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 11)

#define  SelectParm13(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 12)

#define  SelectParm14(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 13)

#define  SelectParm15(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 14)

#define  SelectParm16(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 15)

#define  SelectParm17(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 16)

#define  SelectParm18(pkt)    *(((PMSGCOMMON)&(pkt))->pcpParameter + 17)


#endif /* _H_PDPLANAR */
/* *****************************  End of File  ***************************** */
