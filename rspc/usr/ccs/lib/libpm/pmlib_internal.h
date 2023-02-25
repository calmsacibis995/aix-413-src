/* @(#)16       1.1  src/rspc/usr/ccs/lib/libpm/pmlib_internal.h, pwrcmd, rspc41J, 9510A 3/8/95 07:41:25 */
/*
 * COMPONENT_NAME: pwrcmd
 *
 * FUNCTIONS: Power Management Library
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _H_PMLIB_INTERNAL
#define _H_PMLIB_INTERNAL

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define	PMLIB_DAEMON_PATH	"/tmp/.PMDV1"
#define	PMLIB_CLIENT_PATH	"/tmp/pm.XXXXXX"
#define PMLIB_VERSION		1
#define PMLIB_DEVICE_NAME_LEN	16


typedef struct	_pmlib_param {
	int	value;
} pmlib_param_t;

typedef struct	_pmlib_event {
	int	event;
} pmlib_event_t;

typedef struct _pmlib_internal_device_names {
	int	number;
	char	names[1][16];
} pmlib_internal_device_names_t;


struct pmlib_client {
	int	version;	/* Version number */
	int	length;		/* length of all data */
	uid_t	cli_uid;
	uid_t	cli_euid;
	pid_t	cli_pid;
	int	ftype;		/* Type of function */
	int	ctrl;		/* 1st arg of function */
	union	pmlib_cli_union {
		pmlib_event_t pme;
		pmlib_param_t pmp;
		pmlib_state_t pms;
		pmlib_battery_t pmb;
		pmlib_device_info_t pmdi;
		pmlib_internal_device_names_t pmdn;
	} pcu;
};

struct pmlib_server {
	int	version;	/* Version number */
	int	length;		/* length of all data */
	int	ftype;		/* Type of function */
	int	ctrl;		/* 1st arg of function */
	int	rc;		/* PMLIB_SUCCESS or PMLIB_ERROR */
	union	pm_svr_union {
		pmlib_event_t pme;
		pmlib_param_t pmp;
		pmlib_state_t pms;
		pmlib_battery_t pmb;
		pmlib_device_info_t pmdi;
		pmlib_internal_device_names_t pmdn;
	} psu;
};



/* PM Control Application */
/* internal query type */
#define PMLIB_INTERNAL_GET_EVENT_NOTICE		1
#define PMLIB_INTERNAL_REQUEST_STATE		2
#define PMLIB_INTERNAL_REQUEST_BATTERY		3
#define PMLIB_INTERNAL_REQUEST_PARAMETER	4
#define PMLIB_INTERNAL_REGISTER_APPLICATION	5


#endif /* _H_PMLIB_INTERNAL */
