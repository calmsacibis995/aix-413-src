/* @(#)29	1.4  src/rspc/kernext/pm/pmmd/IBM8301/pm_sig750.h, pwrsysx, rspc41J, 9517A_all 4/25/95 17:04:53  */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for PM kernel externsion machine dependent code 
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

#ifndef _H_PM_SIG750
#define _H_PM_SIG750

#ifndef _KERNEL
#define _KERNEL
#endif /* _KERNEL */

#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/pm.h>
#include <pmmi.h>

#include "pmdi.h"

/*
 *  Signetics control bit definitions
 */
#define ENABLE_FAILSAFE_BYTE	0x20
#define 	ENABLE_FAILSAFE_BIT	0x01
#define CLR_FAILSAFE_BYTE	0x20
#define 	CLR_FAILSAFE_BIT	0x02
#define ACTIVATE_ALARM_BYTE	0x20
#define 	ACTIVATE_ALARM_BIT	0x04
#define PUSHBUTTON_PMI_BYTE	0x21
#define 	PUSHBUTTON_PMI_BIT	0x04
#define PM_EXEC_OFF_BYTE	0x21
#define 	PM_EXEC_OFF_BIT		0x05
#define PMI_OFF_BYTE		0x21
#define 	PMI_OFF_BIT		0x06
#define INTERRUPT_ACK_BYTE	0x22
#define 	INTERRUPT_ACK_BIT	0x01
#define EXT_WAKEUP_ENABLE_BYTE	0x22
#define 	EXT_WAKEUP_ENABLE_BIT	0x05
#define SYSTEM_ID0_BYTE		0x25
#define 	SYSTEM_ID0_BIT		0x01
#define SWITCH_TURNEDON_BYTE	0x26
#define 	SWITCH_TURNEDON_BIT	0x01
#define ALARM_TURNEDON_BYTE	0x26
#define 	ALARM_TURNEDON_BIT	0x02
#define MODEM_TURNEDON_BYTE	0x26
#define 	MODEM_TURNEDON_BIT	0x03
#define GATE_PWR_FAILSAFE_BYTE	0x26
#define 	GATE_PWR_FAILSAFE_BIT	0x05
#define FAST_FLASH_LED_BYTE	0x26
#define 	FAST_FLASH_LED_BIT	0x07

/*
 * Signetics host pin definitions
 */
#define AUDIO_PWR_DOWN		0x0c	/* P3.4 */
#define ETHER_SLEEP		0x0d	/* P3.5 */
#define LED_CONTROL		0x0e	/* P3.6 */

/*
 * Miscellaneous Signetics definitions
 */
#define FAILSAFE_HOLD		0x02
#define ALARM0			0x03
#define ALARM1			0x04
#define ALARM2			0x05

#define	FAILSAFE_HOLD_VAL	0x91

/*
 * Miscellaneious other definitions
 */
#define PMI_REQ			0x04	/* set/cleared in 0x838 */
#define PMI_ENABLE		0x20	/* written to 0x82B */
#define SPEAKER_ENABLE_PORT	0x61
#define SPEAKER_TONE_PORT	0x42
#define CONTROL_WORD_PORT	0x43
#define MODE3			0xB6	/* MODE 3 for square wave */
#define SPEAKER_OFF		0x00	/* bits 0&1: 00 = off */
#define SPEAKER_ON		0x03	/* bits 0&1: 03 = on  */
#define COUNTER_CLOCK_FREQ	1193000	/* 1.193 Mhz */
#define FREQ_LSB		(COUNTER_CLOCK_FREQ / 1000)	/* 1000 Hz */
#define FREQ_MSB		((COUNTER_CLOCK_FREQ / 1000) >> 8)



typedef struct _pmmd_control_data {
	int	resume_trigger;
} pmmd_control_data_t;

/*
 ***********************************
 *  Slave I/F register IO mapping  *
 ***********************************
 */
typedef struct _pm_ISA_data {
        int             base;
        struct io_map   map;
} ISA_DATA;

typedef struct _pm_md_data {
	struct intr			intr;
	struct _pm_ODM_basic_data	pmdi;
	callback_entry_block_t		callback;
	Simple_lock			lock;
	struct io_map			io_map;
	void 				*io_addr;
	struct pm_planar_control_handle	ent_planar_ctl;
	struct pm_planar_control_handle	audio_planar_ctl;
	struct _pm_ISA_data		pm_RTC_data;
} pm_md_data_t;


/*----------------------------------------------------*/
/* Definitions for Basic device save/restore function */
/*----------------------------------------------------*/
/*
 * int planardevice_control(int); 
 */
#define	Initialize_planar_device 0
#define	Save_planar_device	1
#define	Restore_planar_device	2

#endif	/* _H_PM_SIG750 */
