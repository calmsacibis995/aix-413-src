/* @(#)68	1.2  src/tcpip/usr/include/ocsadmin.h, tcpip, tcpip411, GOLD410 3/18/94 13:11:30 */
/*
 * COMPONENT_NAME: TCPIP ocsvhost.h
 *
 * FUNCTIONS: 
 *            
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef __SRC_OCS6000_SRC_USR_LIB_DRIVERS_OCSRTYSERVER_OCSADMIN_H_
#define __SRC_OCS6000_SRC_USR_LIB_DRIVERS_OCSRTYSERVER_OCSADMIN_H_

#if !defined(NOWHAT) && !defined(lint) && defined(ID_HEADERS)
static char kingstonid__src_ocs6000_src_usr_lib_drivers_ocsrtyserver_ocsadmin_h[]=
	"@(#)98 1.2  /orbit/vault/vc/0/4/1/0/s.98 92/06/17";
#endif /* !defined(NOWHAT) && !defined(lint) && defined(ID_HEADERS) */

/*  Origin = AIXESA; License = IBM  */

/* ocs administration header file */
 
#ifndef _H_OCSADMIN
#define _H_OCSADMIN
 
 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
 
/* ocs administrative ioctl commands */
 
#define OCSIOC	('O' << 8)
#define OCS_LOCAL_TBM_OREAD	_IOWR('O', 1, struct ocsioctl)	/* read local tbm output buffer */
#define OCS_LOCAL_TBM_IREAD	_IOWR('O', 2, struct ocsioctl)	/* read local tbm input buffer */
#define OCS_REMOTE_TBM_OREAD	_IOWR('O', 3, struct ocsioctl)	/* read remote tbm output buffer */
#define OCS_REMOTE_TBM_IREAD	_IOWR('O', 4, struct ocsioctl)	/* read remote tbm input buffer */
#define OCS_RTY_READ	_IOWR('O', 5, struct ocsioctl)	/* read rty structure */
#define OCS_TTY_READ	_IOWR('O', 8, struct ocsioctl)	/* read tty structure */
#define OCS_GENSTAT	_IOWR('O', 9, struct ocsioctl)	/* all statistics */
#define OCS_TBM_STAT	_IOWR('O', 10, struct ocsioctl)	/* stats for 1 tbm */
#define OCS_OCS_STAT	_IOWR('O', 11, struct ocsioctl)	/* stats for 1 ocs */
#define OCS_RTY_STAT	_IOWR('O', 12, struct ocsioctl)	/* stats for 1 rty */
#define OCS_RTY_CALLS_TRACE	_IOW('O', 13, struct ocsioctl)	/* trace entry points */
#define OCS_TBM_CALLS_TRACE	_IOW('O', 14, struct ocsioctl)	/* trace entry points */
#define OCS_ALL_CALLS_TRACE	_IOW('O', 15, struct ocsioctl)	/* trace entry points */
#define OCS_RTY_MSGS_TRACE	_IOW('O', 16, struct ocsioctl)	/* trace a message */
#define OCS_TBM_MSGS_TRACE	_IOW('O', 17, struct ocsioctl)
#define OCS_ALL_MSGS_TRACE	_IOW('O', 18, struct ocsioctl)
#define OCS_LOCAL_SET_VAR	_IOW('O', 19, struct ocsioctl)	/* change local parameter */
#define OCS_REMOTE_SET_VAR	_IOW('O', 20, struct ocsioctl)	/* change remote parameter */
#define OCS_LOCAL_QUERY_VAR	_IOWR('O', 21, struct ocsioctl)	/* query local parameter settings */
#define OCS_REMOTE_QUERY_VAR	_IOWR('O', 22, struct ocsioctl)	/* query remote parameter settings */
 
#define OCS_ALL_READ_STAT	_IOWR('O', 30, struct ocsioctl)
#define OCS_ALL_STOP_STAT	_IOWR('O', 40, struct ocsioctl)
#define OCS_ALL_RESET_STAT	_IOWR('O', 50, struct ocsioctl)
#define OCS_TBM_READ_STAT	_IOWR('O', 60, struct ocsioctl)	/* read statistics for given tbm */
#define OCS_TBM_STOP_STAT	_IOWR('O', 70, struct ocsioctl)	/* read statistics for given tbm; stop recording */
#define OCS_TBM_RESET_STAT	_IOWR('O', 80, struct ocsioctl)	/* read statistics for given tbm; restart recording */
#define OCS_RTY_READ_STAT	_IOWR('O', 90, struct ocsioctl)	/* read statistics for given rty drvice */
#define OCS_RTY_STOP_STAT	_IOWR('O', 32, struct ocsioctl)	/* read statistics for given rty drvice; stop recording */
#define OCS_RTY_RESET_STAT	_IOWR('O', 31, struct ocsioctl)	/* read statistics for given rty drvice; restart recording */
 
#define OCS_TBM_NDEVICES	_IOWR('O', 33, struct ocsioctl) /* return number of devices attached to tbm */
#define OCS_RTY_NDEVICES	_IOWR('O', 34, int) /* return number of rty devices stored in rty table */
#define OCS_GET_ALL_TBMID	_IOWR('O', 64, struct ocsioctl)	/* return indeces and ndevice count and ip address of all tbms */
#define OCS_GET_TBMID		_IOWR('O', 65, struct ocsioctl)	/* convert ip address into tbmid */
#define OCS_RTY_DEVNUM		_IOR('O', 66, struct ocsioctl)	/* return rty device number */

/* used also by login monitor */
#define	MAXRTYDEVNAME	40

#define OCS_READ_RTY_NAMES	_IOWR('O', 67, struct ocsioctl) /* return mapping of esa device names and ocs device names */

#define OCS_IS_RTY_ACTIVE	_IOWR('O', 68, struct ocsioctl)	/* used by terminal solicitor to determine if a device is still active */
#define OCS_GET_DEVNAME		_IOWR('O', 69, struct ocsioctl)	/* convert device number into device name */
#define OCS_IS_TBM_CONNECTED	_IOR('O', 63, struct ocsioctl)	/* used by rlogin to determine if tbm connection is up */
 
/* general structure for ocs ioctl's */
struct ocsioctl
{
	int	o_unit;	/* unit number in question */
	int	o_bufsize;	/* size of buffer space */
	caddr_t o_addr;	/* user buffer address */
};
 
 
struct ocs_setvar
{
	int	which_var;
	int	new_value;
};
 
 
#define TBM_ISIZE	1
#define TBM_OSIZE	2
#define TBM_ONMSG	3
#define TBM_MAXTIME	4
#define RTY_ANCHORS	5
#define SEQ_ANCHORS	6
#define SERVER_LIMIT	7
 
/* number of different variables that can be set/queried PLUS 1*/
#define NUM_SET_VARS	8
 
 
struct ocs_stat /* OCS system related statistics */
{
	int	ndevices;	/* # devices on this OCS */
	int	buf_sent;	/* # of TBM buffers sent */
	int	buf_recv;	/* # TBM buffers received */
	int	num_msg;	/* # messages sent */
	int	buf_full;	/* # full buffers sent */
	int	buf_timer;	/* # buffers sent because of timer */
	int	buf_sig;	/* # buffers sent because of signal */
	int	tbmid;		/* tbm connection number */
	char	traceentry;	/* TRUE if trace mode for entry points is set */
	char	tracemsg;	/* TRUE if trace mode for messages is set */
	char	record;		/* TRUE if we should record statistics */
	char	pad;		/* pad to word boundary */
};
 
 
struct rty_stat /* OCS system device statistics */
{
	dev_t	dev;		/* device maj/minor # */
	int	n_open;		/* # of open system calls */
	int	n_close;	/* # of close system calls */
	int	n_read;		/* # of read system calls */
	int	n_write;	/* # of write system calls */
	int	n_ioctl;	/* # of ioctl system calls */
	int	n_select;	/* # of select system calls */
	char	record;		/* TRUE if we should record statistics */
	char	traceentry;	/* TRUE to trace driver entry points */
	char	tracemsg;	/* TRUE to trace driver messages */
	char	pad;		/* pad to word boundary */
};
 
 
struct ocsadmin_conf {		/* configuration of rty administrative device */
	dev_t	devnum;
};
 
 
struct tbmnode {        /* info about tbm connection */
	int	tbmid;
	int	ndevices;
	struct	sockaddr sin;
};
 
 
#define MAXNODES	1000	/* maximum number of tbm connections possible */
 
 
#endif /* _H_OCSADMIN */
#endif /* __SRC_OCS6000_SRC_USR_LIB_DRIVERS_OCSRTYSERVER_OCSADMIN_H_ */
