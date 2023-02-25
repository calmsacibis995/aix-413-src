/* @(#)01	1.3  src/tcpip/usr/sbin/snmpd/fddi.h, snmp, tcpip411, GOLD410 3/25/94 10:59:31 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/fddi.h
 */

#ifndef	_H_FDDI
#define	_H_FDDI

#include "interfaces.h"
#include <sys/err_rec.h>
#include <sys/cdli_fddiuser.h>

/*
 * This is the define for a special type for fddi.  The variables of this
 * type need to be converted.  It is fdditimenano.  This value should be 
 * larger than the syntax types in pepsy.h.  Also, fdditimemilli.
 */
#define FDDITIMENANO	100
#define FDDITIMEMILLI	101

struct fddi_smt_hdr
{
    uchar   flag;                   /* flag to hold the TRF flags */
    uchar   res[2];                 /* 3 reserved bytes */
    uchar   fc;                     /* Frame control field */
    uchar   dest[6];                /* Destination address */
    uchar   src[6];                 /* Source address */
    uchar   frame_class;            /* SMT Frame Class */
    uchar   frame_type;             /* SMT Frame Type */
    ushort  vid;                    /* SMT version ID */
    uint    tid;                    /* Transaction ID */
    uchar   station_id[8];          /* SMT station ID */
    ushort  pad;                    /* padding */
    ushort  l_info;                 /* length of SMT information field */
};
typedef struct fddi_smt_hdr fddi_smt_hdr_t;

struct fddi_smt_info
{
    ushort          type;
    ushort          len;
    uchar           data[200];
};
typedef struct fddi_smt_info fddi_smt_info_t;

typedef struct {
    fddi_smt_info_t *variableinfo;
    int SNMPvarsyntax;
    int reso;
    struct SMT_var_type *next;
} SMT_var_type;

typedef struct frames {
    int rc;
    int numvariable;
    SMT_var_type *list;
    SMT_var_type *endlist;
    int buflen;
    union 
    {
        char buffer[CFDDI_MAX_SMT_PACKET];
        fddi_smt_hdr_t header;
    } un; 
} SMT_frame;

typedef struct Token_Path {
    u_short path_id;
    u_short token_num;
    u_int resource_type;
    u_short resource_id;
    u_short current_path;
    struct Token_Path *next;
} token_path;


typedef struct {
    unsigned int snmpFddiSMTIndex;         /* This equal devunit + 1 */

    int nummacs;
    unsigned short *macidnumbers;  /* contains resource ids for macs */
				   /* Should be 1-n, but not necessarily */

    int numports;
    unsigned short *portidnumbers; /* contains resource ids for ports */
				   /* Should be 1-n, but not necessarily */

    int numpaths;
    unsigned int *pathidnumbers; /* contains resource ids for paths */
    token_path *token_paths;       /* contains paths for each path */

    int s; 			   /* socket descriptor */

    struct interface *ifentry;

    SMT_frame *msgtosend;
} SMT_impl;

#define FDDI_TRF_FLAG		(0x00)	/* TRF Flag */

#define FDDI_FC_SMT             (0x41)  /* frame control = SMT */
#define FDDI_SMT_VID            (0x0002)/* SMT version ID for 7.2 */
#define FDDI_TID_SNMP		(0x673f)/* FDDI trans. id for SNMP */

#define FDDI_SMT_FT_REQUEST     (0x02)  /* frame type = request */
#define FDDI_SMT_FT_RESPONSE    (0x03)  /* frame type = response */

#define FDDI_SMT_FC_PMF_GET     (0x08)  /* PMF = Parameter Mgt. frame: Get */
#define FDDI_SMT_FC_PMF_CHG     (0x09)  /* PMF = Parameter Mgt. frame: Change */

#define FDDI_SMT_RC_FC          (0x01)
#define FDDI_SMT_RC_FRM_VER     (0x02)
#define FDDI_SMT_RC_SUCC        (0x03)
#define FDDI_SMT_RC_SC          (0x04)
#define FDDI_SMT_RC_ILLEGAL     (0x05)
#define FDDI_SMT_RC_NO_PARAM    (0x06)
#define FDDI_SMT_RC_RANGE       (0x08)
#define FDDI_SMT_RC_AUTH        (0x09)
#define FDDI_SMT_RC_LEN         (0x0a)
#define FDDI_SMT_RC_TOO_LONG    (0x0b)
#define FDDI_SMT_RC_SBADENIED   (0x0d)

#endif /* _H_FDDI_ */
