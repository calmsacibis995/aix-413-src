/* @(#)92	1.1  src/rspc/usr/include/pmlib.h, pwrcmd, rspc41J, 9510A 3/8/95 07:08:47 */
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
#ifndef	_H_PMLIB
#define	_H_PMLIB
#include <sys/types.h>

/*
 * PM EVENT
 */
#define	PMLIB_EVENT_NONE				0
#define	PMLIB_EVENT_AC					(1<<1)
#define	PMLIB_EVENT_DC					(1<<2)
#define	PMLIB_EVENT_NOTICE_TO_FULL_ON			(1<<3)
#define	PMLIB_EVENT_NOTICE_TO_ENABLE			(1<<4)
#define	PMLIB_EVENT_NOTICE_TO_STANDBY			(1<<5)
#define	PMLIB_EVENT_NOTICE_TO_SUSPEND			(1<<6)
#define	PMLIB_EVENT_NOTICE_TO_HIBERNATION		(1<<7)
#define	PMLIB_EVENT_NOTICE_TO_SHUTDOWN			(1<<8)
#define	PMLIB_EVENT_NOTICE_TO_TERMINATE			(1<<9)
#define	PMLIB_EVENT_NOTICE_OF_REJECTION			(1<<10)
#define	PMLIB_EVENT_NOTICE_COMPLETION			(1<<11)
#define	PMLIB_EVENT_RESUME_FROM_STANDBY			(1<<12)
#define	PMLIB_EVENT_RESUME_FROM_SUSPEND			(1<<13)
#define	PMLIB_EVENT_RESUME_FROM_HIBERNATION		(1<<14)
#define	PMLIB_EVENT_START_TO_CHANGE_STATE		(1<<15)
#define	PMLIB_EVENT_FORCE_TO_CHANGE_STATE		(1<<16)
#define	PMLIB_EVENT_FAIL_TO_CHANGE_STATE		(1<<17)

/*
 * PM EVENT MASK
 */
#define PMLIB_EVENTMASK_POWER_SUPPLY	\
	( PMLIB_EVENT_AC | \
	  PMLIB_EVENT_DC   \
	)
#define PMLIB_EVENTMASK_STATE_CHANGE		  \
	( PMLIB_EVENT_NOTICE_TO_FULL_ON		| \
	  PMLIB_EVENT_NOTICE_TO_ENABLE		| \
	  PMLIB_EVENT_NOTICE_TO_STANDBY		| \
	  PMLIB_EVENT_NOTICE_TO_SUSPEND		| \
	  PMLIB_EVENT_NOTICE_TO_HIBERNATION	| \
	  PMLIB_EVENT_NOTICE_TO_SHUTDOWN	| \
	  PMLIB_EVENT_NOTICE_TO_TERMINATE	| \
	  PMLIB_EVENT_NOTICE_OF_REJECTION	| \
	  PMLIB_EVENT_NOTICE_COMPLETION		| \
	  PMLIB_EVENT_RESUME_FROM_STANDBY	| \
	  PMLIB_EVENT_RESUME_FROM_SUSPEND	| \
	  PMLIB_EVENT_RESUME_FROM_HIBERNATION	| \
	  PMLIB_EVENT_START_TO_CHANGE_STATE	| \
	  PMLIB_EVENT_FORCE_TO_CHANGE_STATE	| \
	  PMLIB_EVENT_FAIL_TO_CHANGE_STATE	  \
	)
/*
 * SYSTEM STATE CHANGE REQUEST
 */
/* ctrl	*/
#define	PMLIB_REQUEST_STATE		1
#define	PMLIB_QUERY_STATE		2
#define	PMLIB_QUERY_ERROR		3
#define	PMLIB_CONFIRM			4

/* PMLIB state structure */
typedef	struct _pmlib_state {
	int	state;		/* system state	for set/query */
	int	error;		/* error value for query error */
	pid_t	pid;		/* process id of application prevented
					system state change */
	char	name[16];	/* name	of apllication prevented
					system state change */
} pmlib_state_t;

/* PMLIB System	states */
#define	PMLIB_NONE			0x00000000
#define	PMLIB_SYSTEM_FULL_ON		0x00000100
#define	PMLIB_SYSTEM_ENABLE		0x00000200
#define	PMLIB_SYSTEM_STANDBY		0x00000400
#define	PMLIB_SYSTEM_SUSPEND		0x00000800
#define	PMLIB_SYSTEM_HIBERNATION	0x00001000
#define	PMLIB_SYSTEM_SHUTDOWN		0x00002000
#define	PMLIB_SYSTEM_CHANGE_OK		0x00000001
#define	PMLIB_SYSTEM_CHANGE_NO		0x00000002

/* PMLIB erorrs	*/
#define	PMLIB_NO_ERROR			0
#define	PMLIB_ERROR_REJECT_BY_DEVICE	1
#define	PMLIB_ERROR_REJECT_BY_APPL	2
#define	PMLIB_ERROR_REJECT_BY_SYSTEM	3
#define	PMLIB_ERROR_REJECT_BY_HIB_VOL	4
#define	PMLIB_ERROR_REJECT_BY_EVENT	5
#define	PMLIB_ERROR_INVALID_PRIVILEGE	6
#define	PMLIB_ERROR_DEVICE_ERROR	7
#define	PMLIB_ERROR_OTHERS		8



/*
 * BATTERY REQUEST
 */
/* cmd */
#define	PMLIB_QUERY_BATTERY		1
#define	PMLIB_DISCHARGE_BATTERY		2

/* PMLIB Battery Structure */
typedef	struct _pmlib_battery {
	int attribute;		/* battery attributes */
	int capacity;		/* battery capacity */
	int remain;		/* current remained capacity */
	int refresh_discharge_capacity;	/* remain while	discharging */
	int refresh_discharge_time;	/* discharge time */
	int full_charge_count;
} pmlib_battery_t;

/* values for attribute	*/
#define	PMLIB_BATTERY_SUPPORTED		0x00000100
#define	PMLIB_BATTERY_EXIST		   0x00000200
#define	PMLIB_BATTERY_NICD		   0x00000400
#define	PMLIB_BATTERY_CHARGING		0x00000800
#define	PMLIB_BATTERY_DISCHARGING	0x00001000
#define	PMLIB_BATTERY_AC		      0x00002000
#define	PMLIB_BATTERY_DC   		   0x00004000
#define	PMLIB_BATTERY_REFRESH_REQ	0x00008000



/*
 * PARAMETER REQUEST
 */
/* ctrl	*/
#define	PMLIB_QUERY_SYSTEM_IDLE_TIME		1
#define	PMLIB_SET_SYSTEM_IDLE_TIME		2
#define	PMLIB_QUERY_DEVICE_INFO			3
#define	PMLIB_SET_DEVICE_INFO			4
#define	PMLIB_QUERY_LID_CLOSE_ACTION		5
#define	PMLIB_SET_LID_CLOSE_ACTION		6
#define	PMLIB_QUERY_SYSTEM_IDLE_ACTION		7
#define	PMLIB_SET_SYSTEM_IDLE_ACTION		8
#define	PMLIB_QUERY_MAIN_SWITCH_ACTION		9
#define	PMLIB_SET_MAIN_SWITCH_ACTION		10
#define	PMLIB_QUERY_LOW_BATTERY_ACTION		11
#define	PMLIB_SET_LOW_BATTERY_ACTION		12
#define	PMLIB_QUERY_PERMISSION			13
#define	PMLIB_SET_PERMISSION			14
#define	PMLIB_QUERY_BEEP			15
#define	PMLIB_SET_BEEP				16
#define	PMLIB_QUERY_RINGING_RESUME		17
#define	PMLIB_SET_RINGING_RESUME		18
#define	PMLIB_QUERY_RESUME_TIME			19
#define	PMLIB_SET_RESUME_TIME			20
#define	PMLIB_QUERY_DURATION_TO_HIBERNATION	21
#define	PMLIB_SET_DURATION_TO_HIBERNATION	22
#define	PMLIB_QUERY_SPECIFIED_TIME		23
#define	PMLIB_SET_SPECIFIED_TIME		24
#define	PMLIB_QUERY_SPECIFIED_TIME_ACTION	25
#define	PMLIB_SET_SPECIFIED_TIME_ACTION		26
#define	PMLIB_QUERY_DEVICE_NUMBER		27
#define	PMLIB_QUERY_DEVICE_NAMES		28
#define	PMLIB_QUERY_SUPPORTED_STATES		29
#define	PMLIB_QUERY_KILL_LFT_SESSION		30
#define	PMLIB_SET_KILL_LFT_SESSION		31
#define	PMLIB_QUERY_KILL_TTY_SESSION		32
#define	PMLIB_SET_KILL_TTY_SESSION		33
#define	PMLIB_QUERY_PASSWD_ON_RESUME		34
#define	PMLIB_SET_PASSWD_ON_RESUME		35
#define	PMLIB_QUERY_KILL_SYNCD			36
#define	PMLIB_SET_KILL_SYNCD			37

/* For On/Off */
#define	PMLIB_OFF	0
#define	PMLIB_ON  	1

/* PMLIB Device	Info Structure */
typedef	struct _pmlib_device_info {
	char	name[16];	/* name	of device */
	int	mode;		/* device mode */
	int	attribute;	/* device attribute */
	int	idle_time;	/* idle	timer value during PM system enable */
	int	standby_time;	/* idle	timer value during standby enable */
	int	idle_time1;	/* idle	timer value for	DPMS suspend */
	int	idle_time2;	/* idle	timer value for	DPSM off */
	char	reserved[24];	/* reserved area for future use	*/
} pmlib_device_info_t;

/* PMLIB Device	Modes */
#define	PMLIB_DEVICE_FULL_ON		0x00000000
#define	PMLIB_DEVICE_ENABLE		0x00000100
#define	PMLIB_DEVICE_IDLE		0x00000200
#define	PMLIB_DEVICE_SUSPEND		0x00000400
#define	PMLIB_DEVICE_HIBERNATION	0x00000800

/* PMLIB Device	attribute */
#define	PMLIB_GRAPHICAL_INPUT		0x00010000
#define	PMLIB_GRAPHICAL_OUTPUT		0x00020000
#define	PMLIB_AUDIO_INPUT		0x00040000
#define	PMLIB_AUDIO_OUTPUT		0x00080000
#define  PMLIB_RING_RESUME_SUPPORT	0x00100000
/* PMLIB Device	Names Structure	*/
typedef	struct _pmlib_device_names {
	int number;	/* number of PM	Aware DDs */
	char *names;	/* names of PM Aware DDs */
} pmlib_device_names_t;

/*
 * REGISTER APPLICATION
 */
/* cmd */
#define	PMLIB_REGISTER		1
#define	PMLIB_UNREGISTER	2


/* function prototypes */
int pmlib_get_event_notice(int *);
int pmlib_request_state(int, pmlib_state_t *);
int pmlib_request_battery(int, pmlib_battery_t *);
int pmlib_request_parameter(int, caddr_t);
int pmlib_register_application(int);

#define	PMLIB_SUCCESS		0
#define	PMLIB_ERROR		-1

#define	SIGPM	SIGPWR

#endif	/* _H_PMLIB */
