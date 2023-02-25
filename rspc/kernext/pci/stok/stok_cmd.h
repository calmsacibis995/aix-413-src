/* "@(#)42  1.1  src/rspc/kernext/pci/stok/stok_cmd.h, pcitok, rspc41J, 9513A_all 3/21/95 17:27:38" */
/*
 *   COMPONENT_NAME: pci_tok
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_STOK_CMD
#define _H_STOK_CMD
/*****************************************************************************/
/*                  Config HP Channel Command Structure                      */
/*****************************************************************************/
typedef  struct {
        uchar        cmd;
        uchar        reserved6[6];
        uchar        option2;
        ushort       option1;
        uchar        reserved2[2];
        uchar        node_address[6];
        uchar        group_address[6];
        uchar        func_address[4];
        ushort       trb_buffer_length;
        uchar        rcv_channel_options;
        uchar        number_local_addr;
        uchar        product_id[18];
  } open_op;

/*****************************************************************************/
/*                  Config HP Channel Command Structure                      */
/*****************************************************************************/
typedef struct {
        uchar   cmd;            /*  "0x13"                                  */
        uchar   rsv_1;          /* Reserved                                 */
        uchar   retcode;        /* Set by adapter on return                 */
        uchar   rsv_2;          /* Reserved                                 */
	uchar   access_priority;
        uchar   rsv_3;          /* Reserved                                 */
} cfg_hp_op;

/*****************************************************************************/
/*                  Read Adapter Log Command Structure                      */
/*****************************************************************************/
typedef struct {
        uchar   cmd;            /*  "0x08"                                  */
        uchar   rsv_1;          /* Reserved                                 */
        uchar   retcode;        /* Set by adapter on return                 */
        uchar   rsv_2;          /* Reserved                                 */
} read_log_op;

/*****************************************************************************/
/*                  Modify Receive Options Command Structure                 */
/*****************************************************************************/
typedef struct {
	uchar	cmd;		/* MODIFY.RECEIVE.OPTIONS "0x17"            */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3;          /* Reserved                                 */
	ushort	recv_op;        /* New setting for the receive options      */
	uchar	passwd[8];      /* Access password                          */
} mod_recv_op;


/*****************************************************************************/
/*                  Set Group Command Structure                              */
/*****************************************************************************/
typedef struct {
	uchar	g_addr[6];      /* Group address                            */
	ushort	num;            /* Number of sequential address             */
	uchar	type;           /* Selects the group address type           */
	uchar	location;
} g_addr;

/*****************************************************************************/
/*                  Set Function Command Structure                           */
/*****************************************************************************/
typedef struct {
	uchar	cmd;		/* SET.GROUP.ADDRESS or RESET.GROUP.ADDRESS */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3[3];       /* Reserved                                 */
	uchar	g_addr[6];      /* Group address                            */
	ushort	num;            /* Number of sequential address             */
	uchar	type;           /* Selects the group address type           */
	uchar	rsv_16;         /* Reserved                                 */
} g_address;

/*****************************************************************************/
/*                  Set Function Command Structure                           */
/*****************************************************************************/
typedef	struct {
	uchar	cmd;		/* SET.FUNCTIONAL.ADDRESS                   */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3[3];       /* Reserved                                 */
	uchar	f_addr[4];      /* functional address                       */
} f_address;

#endif /* _H_STOK_CMD */

