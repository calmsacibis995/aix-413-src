/* @(#)27	1.14  src/rspc/kernext/tty/sty.h, isatty, rspc41J, 9523A_all 6/6/95 05:20:01  */
/*
 *   COMPONENT_NAME: isatty
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _STY_H
#define _STY_H

/*
 * inline function definitions. (#gagme)
 * (these would be commented better, but the implementation of these by the
 *  compiler is so bad that it resists my efforts to comment it in a
 *  reasonable manner)
 */
#pragma mc_func disable { "7c6000a6" "5464045e" "7c800124" }
#pragma mc_func enable  { "7c600124" }	/* mtmsr	r3	*/

#pragma reg_killed_by disable	gr3-gr4
#pragma reg_killed_by enable	gr3

#define	TR_TTY			1	/* KOFF trace flag bit	*/
#define RBSIZE			512	/* receive buffer size		*/
#define STY_RDBUFSZ		64	/* streams rcv message size	*/

/*
 * this struct is used to keep track of the major device # to underlying
 * device mapping.
 */
struct ttydevs {
	struct ttydevs *nxt;		/* linked list of	*/
	char	*name;			/* name of this adapter	*/
	struct sty_tty	*(*open)();	/* open unit		*/
	void	(*print)();		/* debug print func	*/
	dev_t	dev;			/* major device		*/
	uchar	busy;			/* on ttydevsw list	*/
	uchar	isopen;			/* flag: !0 => open	*/
};

/*
 * common streams/tty device struct.
 */
struct sty_tty {
	char t_name[DEV_NAME_LN+1];	/* resource name for this tty	*/
	dev_t t_dev;			/* major/minor number		*/
	int (*t_ioctl)();		/* I/O control handler		*/
	int (*t_ctl)();			/* M_CTL message handler	*/
	int (*t_close)();		/* close unit			*/
	int (*t_service)();		/* service proc			*/
	int (*t_proc)();		/* proc proc			*/
	int (*t_rsrv)();		/* h/w read service		*/
	int (*t_wsrv)();		/* h/w write service		*/

	queue_t *t_queue;		/* queue pointer		*/
	mblk_t *t_outmsg;		/* msg block on x'sion		*/
	mblk_t *t_inmsg;		/* msg block on reception	*/
	mblk_t *t_ioctlmsg;		/* async ioctl req		*/
	void *t_hptr;			/* hardware storage		*/

	int t_event;			/* event list for e_sleep	*/
	Simple_lock  t_lock;	        /* lock per tty			*/
	int t_timeout;			/* timeout: M_DELAY		*/
	int t_btimeout;			/* timeout: M_BREAK		*/
	int t_alloctimer;		/* timeout: sty_allocmsg	*/
	int t_bufcall;			/* bufcall utility was called	*/
	uchar t_draining;		/* draining data at close time	*/
	uchar t_hpri;			/* h/w priority			*/

	uint t_stopbits		:2;	/* number of stop bits		*/
	uint t_parity		:3;	/* parity style			*/
	uint t_csize		:2;	/* character size		*/
	uint t_cread		:1;	/* termios cread flag		*/
	uint t_isopen		:1;	/* tty is open			*/
	uint t_hupcl		:1;	/* current HUPCL termios cfalg	*/
	uint t_excl		:1;	/* exclusive open mode		*/
	uint t_clocal		:1;	/* local/remote open mode	*/
	uint t_ctlx		:1;	/* got a control X		*/
	uint t_sak		:1;	/* sak is enabled		*/
	uint t_localstop	:1;	/* M_STOP generated flow ctl	*/

	struct {
		struct termios tios;
		struct termiox tiox;
	} t_dflt;			/* default parameters		*/

	int t_cflag;			/* save of termios.c_cflag	*/
	tcflag_t t_softpacing;		/* Current termios c_iflag	*/
	cc_t t_softchars[2];		/* VSTART and VSTOP chars	*/
	unsigned short t_hardpacing;	/* Current termiox x_hflag	*/

	baud_t t_ospeed;		/* output speed			*/
	baud_t t_ispeed;		/* input speed			*/

	/*
	 * the following bits must be protected against concurrent access
	 */
	uint t_carrier		:1;	/* carrier flag			*/
	uint t_busy		:1;	/* output in progress		*/
	uint t_stop		:1;	/* output stopped, remote pacing*/
	uint t_block		:1;	/* input stopped, local pacing	*/
	uint t_xon_pending	:1;	/* XON pending output	*/
	uint t_xoff_pending	:1;	/* XOFF pending output	*/
	uint t_modem_chg	:1;	/* modem change flag	*/

	/* Open disciplines	*/
	int t_open_disc;		/* Current open discipline	*/
					/* 0 == dtropen */
					/* 1 == wtopen	*/
					/* 2 == riopen	*/
	caddr_t t_openRetrieve;		/* current open discipline	*/
	uint t_wopen;			/* waiting for open		*/

	/* Flow disciplines	*/
	enum flow_type {
		flo_none   =0x0000,	/* no flow control		*/
		flo_idtr   =0x0001,	/* iput flow controlled by DTR	*/
		flo_irts   =0x0002,	/* iput flow controlled by RTS	*/
		flo_isw    =0x0080,	/* iput flow controlled by s/w	*/
		flo_imask  =0x00ff,	/* iput flow mask		*/
		flo_octs   =0x0100,	/* oput flow controlled by CTS	*/
		flo_odsr   =0x0200,	/* oput flow controlled by DSR	*/
		flo_odcd   =0x0400,	/* oput flow controlled by DCD	*/
		flo_osw    =0x8000,	/* oput flow controlled by s/w	*/
		flo_omask  =0xff00,	/* oput flow mask		*/
		flo_hwmask =0x7f7f,	/* s/w flo control bits		*/
		flo_swmask =0x8080,	/* h/w flo control bits		*/
	} t_flow_disc;

	ushort *t_rptr;			/* ^ to <status, char>s to read	*/
	ushort *t_wptr;			/* ^ to where to write		*/
	ushort t_rbuf[RBSIZE];		/* receive buffer		*/

	struct trace *t_tbuf;		/* ^ to trace buffer, for dbg	*/
};

/*
 * trace stuff.  currently, we steal the "rs" driver hook word for "sty".
 * at some other time, we must allocate a hook for "sty" that would permit
 * "rs" to be recoded to use "sty".
 */
#define TRC_STY(w)   ((HKWD_STTY_RS)|w)

/*
 * debugger stuff
 */
#ifdef TTYDBG
#  define ttyDBG	1
#else
#  define ttyDBG	0
#endif

#ifdef FOR_CRASH
#  define for_CRASH	1
#  define sty_printd	printf
#else
#  define for_CRASH	0
#endif

#ifndef DEBUG
#  define dbg_tbuf()	(0)
#endif

#if defined(KOFF)
#include <sys/koff/koff.h>
#define	printd	printf
#else
/*
 * non-KOFF environment.  Sigh.  Have to pull in some defs so that things work.
 */
#include "sys/time.h"

#define koff_addsyms(a, b)	/* no function available	*/
#define koff_delsyms(a)		/* no function available	*/

#define printd	sty_printd	/* "sty" debugger print func	*/

/*
 * krash-mode function desc
 */
struct krash_func {
	void	(*func)();	/* ^ to function	*/
	char	*name;		/* function name	*/
	char	*synopsis;	/* brief synopsis	*/
	char	*help;		/* help string		*/
};

/*
 * debug flag description
 */
struct db_flags {
	ulong	bitmask;
	char	*what;
};

#define	TRACE_SIZE	(8 * 1024)	/* # trace entries	*/

struct trace {
	struct timestruc_t ticks;	/* copy of `tod'	*/
	u_short pslot;			/* proc table slot	*/
	void *id;			/* opaque trace entry ID*/
	char *fmt;			/* printf() format	*/
	u_long	a0, a1, a2, a3;		/* args			*/
};
#ifdef DEBUG
	extern	long	db_tflags;
#	define db_trace(mask, zot)	if (db_tflags & (mask)) _db_trace zot
#else
#	define db_trace(mask, zot)
#endif	/* !DEBUG	*/
#endif	/* !KOFF	*/

/*
 * various function prototypes
 */
int styopen(queue_t *q,dev_t *devp, int mode, int sflag, cred_t *credp);
int styclose(queue_t *q, int mode, cred_t *credp);
int sty_ioctl(queue_t *q, mblk_t *mp, int fromput);
void sty_ctl(queue_t *q, mblk_t *mp);
void sty_termios_set(struct sty_tty *tp,struct termios *tios, int force);
void sty_termiox_set(struct sty_tty *tp,struct termiox *tiox, int force);
void sty_termios_get(struct sty_tty *tp,struct termios *tios);
void sty_termiox_get(struct sty_tty *tp,struct termiox *tiox);
void sty_break_set(struct sty_tty *tp, int duration, mblk_t *mp);
void sty_break_clear(struct sty_tty *tp);
int styrsrv(queue_t *q);
int stywput(queue_t *q,mblk_t *mp);
int stywsrv(queue_t *q);
int sty_allocmsg(struct sty_tty *tp, int gotlock);
int sty_recover(struct sty_tty *tp);
void sty_timeout(struct sty_tty *tp);
void sty_flush(struct sty_tty *tp, int rw, int recycle);

#endif  /* _STY_H     */
