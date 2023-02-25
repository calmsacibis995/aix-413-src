static char sccsid[] = "@(#)25	1.24.1.1  src/rspc/kernext/tty/sty.c, isatty, rspc41B, 412_41B_sync 12/8/94 14:32:54";
/*
 *   COMPONENT_NAME: isatty
 *
 *   FUNCTIONS: sty_allocmsg, sty_break_clear, sty_break_set, sty_ctl, sty_err
 *		sty_flush, sty_ioctl, sty_pcmds, sty_recover
 *		sty_register, sty_scmds, sty_termios_get, sty_termios_set
 *		sty_termiox_get, sty_termiox_set, sty_timeout, sty_unregister
 *		styclose, styopen, styrsrv, stywput, stywsrv
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
#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks		*/
#include <sys/sys_resource.h>		/* key possition check		*/
#include <sys/limits.h>			/* for MAX_CANON value		*/
#include <sys/sysmacros.h>		/* minor and major macros	*/
#include <sys/intr.h>			/* interrupt handler stuff	*/
#include <sys/pin.h>			/* pinning stuff		*/
#include <sys/adspace.h>		/* io_att and io_det		*/
#include <sys/iocc.h>			/* iocc space values		*/
#include <sys/ioacc.h>			/* bus access macros		*/
#include <sys/dma.h>			/* dma stuff			*/
#include <sys/xmem.h>			/* x-memory stuff for dma	*/
#include <sys/malloc.h>			/* malloc services		*/
#include <sys/errids.h>			/* get the error id's		*/
#include <sys/errno.h>			/* error codes			*/
#include <sys/uio.h>			/* struct uio			*/
#include "ttydrv.h"			/* instead of the old sys/tty.h	*/
#include <sys/termio.h>			/* struct termios		*/
#include <sys/termiox.h>		/* struct termiox		*/
#include <unistd.h>			/* need _POSIX_VDISABLE def	*/
#include <sys/ttydefaults.h>		/* TTY defaults			*/
#include <sys/ioctl.h>			/* Ioctl declerations		*/
#include <sys/device.h>			/* for config stuff		*/
#include <sys/dbg_codes.h>		/* temp for debugger thing	*/
#include <sys/mstsave.h>		/* temp for debugger thing	*/
#include <sys/low.h>			/* temp for debugger thing	*/
#include <sys/lockl.h>			/* for lockl and unlockl	*/
#include <sys/atomic_op.h>		/* for atomic operations	*/
#include <sys/trchkid.h>		/* trace hook id's		*/
#include <sys/sysinfo.h>		/* sysinfo stuff		*/
#include <sys/signal.h>			/* SIGSAK			*/
#include <sys/pri.h>
#include <sys/sleep.h>			/* EVENT_NULL			*/
#include <sys/except.h>			/* PIO stuff			*/
#include <sys/dump.h>			/* dump structures		*/
#include <sys/str_tty.h>		 /* Streams defines		*/
#include <sys/stream.h>			/* Streams defines		*/
#include <sys/stropts.h>		/* Streams defines		*/
#include <sys/strlog.h>			/* Streams defines		*/
#include <sys/strconf.h>		/* Streams defines		*/
#include <string.h>			/* string handling function	*/
#include "sty.h"			/* dds for config		*/
#include "common_open.h"		/* Common open discipline fns	*/
#include <sys/lpio.h>			/* LPWRITE_REQ and LPWRITE_ACK	*/

static int koff_once;			/* and tell me where it hurts	*/

struct ttydevs *ttydevsw;
int tty_mutex = LOCK_AVAIL;

/*
 * streams stuff
 */
struct module_info modinfo = {
	DRIVER_ID, "sty", 0, INFPSZ, 2048, 256
};

struct qinit sty_rinit = {
	NULL, styrsrv, styopen, styclose, NULL, &modinfo, NULL
};

struct qinit sty_winit = {
	stywput, stywsrv, NULL, NULL, NULL, &modinfo, NULL
};

struct streamtab styinfo = { &sty_rinit, &sty_winit, NULL, NULL };

strconf_t styconf = {
	0,				/* sc_name	*/
	&styinfo,			/* sc_str	*/
	STR_NEW_OPEN|STR_MPSAFE,	/* sc_flags	*/
	0,				/* sc_major	*/
	SQLVL_QUEUEPAIR,		/* sc_sqlevel	*/
	0				/* sc_sqinfo	*/
};

/*
 * styopen
 *    Checks if the minor number corresponds to a configured port.
 *    Links the queues to the sty_tty structure.
 *    Initializes the line.
 *    Calls the open entrypont of the open discipline.
 *
 * Input:
 *      q       - address of STREAMS read queue
 *      devp    - pointer to serial line device number
 *      flag    - open(2) flags (Read|Write)
 *      sflag   - type of STREAMS open (MODOPEN, 0 [normal], CLONEOPEN)
 *      credp   - pointer to cred structure
 *
 * Returns:
 *      ENODEV  - no such serial line
 */
styopen(queue_t *q, dev_t *devp, int mode, int sflag, cred_t *credp) {
	register int port, err, x;
	register struct ttydevs *v;
	register struct sty_tty *tp;
	register unsigned unit;
	dev_t dev = *devp;

	Enter(TRC_STY(TTY_OPEN), *devp, q->q_ptr, mode, sflag, 0);
	db_trace(TR_TTY, (q, "sty: open(%x, %x, %d, %x)"
			  , *devp, q->q_ptr, mode, sflag));

	unit = minor(dev);
	dev  = makedev(major(dev), 0);
	err  = 0;
	tp   = 0;

	lockl(&tty_mutex, LOCK_SHORT);

	/*
	 * find ttys associated with this device
	 */
	for (v = ttydevsw; v && v->dev != dev; v = v->nxt)
		;

	/*
	 * if we found the device, open it.
	 */
	if (!v || !(tp = (*v->open)(makedev(dev, unit))))
		err = ENODEV;

	unlockl(&tty_mutex);

	if (!err) {
		x = disable_lock(tp->t_hpri, &tp->t_lock);

		if (!tp->t_isopen) {
			/*
			 * first open
			 */
			WR(q)->q_ptr = (caddr_t)tp;
			   q ->q_ptr = (caddr_t)tp;
			tp->t_queue  = q;
			tp->t_tbuf   = (void *) dbg_tbuf();

			tp->t_localstop    = 0;
			tp->t_busy         = 0;
			tp->t_stop         = 0;
			tp->t_block        = 0;
			tp->t_xon_pending  = 0;
			tp->t_xoff_pending = 0;
			tp->t_modem_chg    = 0;

			sty_allocmsg(tp, 1);
		} else if (tp->t_excl && credp->cr_uid)
			err = EBUSY;

		unlock_enable(x, &tp->t_lock);
	}

	if (err)
		goto out;

	++tp->t_wopen;
	if (err = openDisc_open(&tp->t_openRetrieve, tp, tp->t_open_disc
				, tp->t_service
				, tp->t_clocal ? OPEN_LOCAL : OPEN_REMOTE
				, mode, !tp->t_isopen, &tp->t_event)) {
		db_trace(TR_TTY, (tp, "sty:*error during disc-open"));

		--tp->t_wopen;
		if (!tp->t_wopen && !tp->t_isopen) {
			register save = tp->t_hupcl;

			tp->t_hupcl = 1;  /* To ensure dropping DTR and RTS */
			styclose(q, mode, credp);
			tp->t_hupcl = save;
		}
		goto out;
	}

	--tp->t_wopen;
	tp->t_isopen = 1;

    out:
	Return(err);
}

/*
 * styclose
 *      Closes current open discipline.
 *      Flushs queues and disconnects them from private structures.
 *      Cancels the pending bufcall and timeout requests.
 *
 * Input:
 *      q       - address of STREAMS read queue
 *      mode    - type of STREAMS open (MODOPEN, 0 [normal], CLONEOPEN)
 *      credp   - pointer to cred structure
 *
 * Returns:
 *      ENODEV  - no such serial line
 */
styclose(queue_t *q, int mode, cred_t  *credp) {
	register struct sty_tty *tp = (void*) q->q_ptr;
        register x, err = 0;
	dev_t dev = tp->t_dev;

	Enter(TRC_STY(TTY_CLOSE), dev, tp, mode, 0, 0);
	db_trace(TR_TTY, (q, "sty: close(, %x, %x) tp %x", mode, credp, tp));

	if (tp->t_wopen) {
		/*
		 * At least one process is opening this tty, if it finishes
		 * its open successfully, it won't reinitialize everything.
		 * The last process leaving unseccessfully the openDisc_open()
		 * will call styclose()
		 */
		goto out;
	}

	/*
	 * drain data in write queue before leaving
	 */
	x = disable_lock(tp->t_hpri, &tp->t_lock);

	tp->t_draining = tp->t_outmsg || tp->t_busy || WR(q)->q_first;
	tp->t_cread    = 0;	/* not interested in input either */

	while (tp->t_draining) {
		db_trace(TR_TTY, (tp, "sty: draining %s"
				  , tp->t_localstop ? "localstop" : ""));

		if (tp->t_localstop) {
			/*
			 * output flow controlled due to M_STOP?
			 */
			 tp->t_xoff_pending = 0;
			 tp->t_localstop    = 0;

			 (*tp->t_proc)(tp, T_RESUME);
		 }

		unlock_enable(x, &tp->t_lock);

		err = TTY_SLEEP(tp) ? EINTR : 0;

		x = disable_lock(tp->t_hpri, &tp->t_lock);

		if (err == EINTR) {
			db_trace(TR_TTY, (tp, "sty:*EINTR on close"));
			flushq(WR(tp->t_queue), FLUSHDATA);
			tp->t_draining = 0;
			(*tp->t_proc)(tp, T_WFLUSH);
			ASSERT(!tp->t_outmsg);
		} else
			tp->t_draining = (tp->t_outmsg || tp->t_busy
					  || WR(q)->q_first);
	}

	unlock_enable(x, &tp->t_lock);

	simple_lock(&tp->t_lock);	/* really get the lock	*/
	tp->t_isopen    = 0;    
	tp->t_localstop = 0;

	/*
	 * close h/w port when done.
	 */
	(*tp->t_close)(tp);

	/*
	 * Call open discipline close
	 */
	if (openDisc_close(tp->t_openRetrieve, tp->t_hupcl))
		db_trace(TR_TTY, (tp, "sty:*error closing open-disc"));

	tp->t_openRetrieve = 0;

	if (tp->t_bufcall) {
		unbufcall(tp->t_bufcall);
		tp->t_bufcall = 0;
	}
	if (tp->t_timeout) {
		untimeout(tp->t_timeout);
		tp->t_timeout = 0;
	}
	if (tp->t_btimeout) {
		untimeout(tp->t_btimeout);
		tp->t_btimeout = 0;
	}
	if (tp->t_alloctimer) {
		untimeout(tp->t_alloctimer);
		tp->t_alloctimer = 0;
	}

	/*
	 * XXX: Do I believe this???
	 *	No need for locking now since neither an interrupt nor
	 *	bufcall answer is expected
	 */
	if (tp->t_inmsg) {
		freemsg(tp->t_inmsg);
		tp->t_inmsg = 0;
	}
	if (tp->t_ioctlmsg) {
		freemsg(tp->t_ioctlmsg);
		tp->t_ioctlmsg = 0;
	}

	simple_unlock(&tp->t_lock);

    out:
	Return(err);
}

/*
 * sty_flush
 *      Flushs read  and write queues, according to rw parameter.
 *      if message is set MFLUSH it is sent upstream.
 *
 * Input:
 *      tp      - address of sty_tty structure.
 *      rw      - FLUSHW or FLUSHR flags.
 *      recycle - if set, sent an M_FLUSH with FLUSHR.
 */
void
sty_flush(struct sty_tty *tp, int rw, int recycle) {
	register old_intr, zot;
	queue_t pie;	/* !! */

	db_trace(TR_TTY, (tp, "sty: flush rw %d recycle %d", rw, recycle));

	if (rw & FLUSHW) {
		flushq(WR(tp->t_queue), FLUSHDATA);
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
		(*tp->t_proc)(tp, T_WFLUSH);
		unlock_enable(old_intr, &tp->t_lock);
	}

	if (rw & FLUSHR) {
		flushq(tp->t_queue, FLUSHDATA);
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
		(*tp->t_proc)(tp, T_RFLUSH);
		unlock_enable(old_intr, &tp->t_lock);
		if (recycle)
			putctl1(tp->t_queue->q_next, M_FLUSH, FLUSHR);
	}
}

/*
 * sty_ioctl
 *      Processes the M_IOCTL messages.
 *      Commands to be perfomed by service procedure are put
 *      back in the queue.
 *      An M_IOCAK is sent upstreams if successfull processing,
 *      otherwise an M_IONACK with error code.
 *
 * Input:
 *      q       - address of STREAMS write queue
 *      mp      - M_IOCTL to process.
 *      fromput - 1 if called by put procedure.
 *
 * Returns:
 *      0 if ioctl message is successfully processed or delayed to
 *      the service procedure. -1 otherwise.
 *      Note that this value is not used by the caller , but it is usefull to
 *      record a failure in an ioctl processing.
 */
sty_ioctl(queue_t *q, mblk_t *mp,int fromput) {
	struct iocblk *iocp;
	register struct sty_tty *tp = (struct sty_tty *)q->q_ptr;
	register cnt;
	unsigned char outtype;
	unsigned long outcnt;
	struct termios *tios;
	struct termiox *tiox;
	int signal_temp;
	int rc = 0;

	Enter(TRC_STY(TTY_IOCTL), tp->t_dev, tp
	      , ((struct iocblk *)mp->b_rptr)->ioc_cmd, 0, 0);

	iocp = (struct iocblk *)mp->b_rptr;
	cnt  = iocp->ioc_count;

	db_trace(TR_TTY, (tp, "sty: ioctl cmd=%x mp=%x cnt=%x %sfromput"
			  , iocp->ioc_cmd
			  , mp
			  , iocp->ioc_count
			  , fromput ? "" : "!"));

	if (cnt == TRANSPARENT) {
		db_trace(TR_TTY, (tp, "sty:*ioctl NAK: transparent"));

		mp->b_datap->db_type = M_IOCNAK; /*Not Okay */
		iocp->ioc_error = EINVAL;
		qreply(q,mp);

		rc = -1;
		goto out;
	}
	iocp->ioc_error =0;
	outtype = M_IOCACK;
	outcnt = 0;
	
	if (!fromput) {	 /* from service routine */
		switch (iocp->ioc_cmd) {
		    case TIOCSETAW:
		    case TIOCSETAF:
			if (cnt != sizeof (struct termios) || !mp->b_cont) {
			    enospc:
				db_trace(TR_TTY
					 , (tp, "sty:*ioctl NAK: ENOSPC"));

				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}

			/*
			 * Wait for the o/p to drain and flush i/p
			 * and set params
			 */
			if (iocp->ioc_cmd == TIOCSETAF) {
				flushq(RD(q),FLUSHDATA);
				putctl1(tp->t_queue->q_next,M_FLUSH,FLUSHR);
			}
			sty_termios_set(tp, (void*) mp->b_cont->b_rptr, 0);
			break;

		    case TCSBRK:
		    case TCSBREAK: {
			    int arg;

			    if (cnt != sizeof (int) || !mp->b_cont)
				    goto enospc;

			    arg = *(int *)mp->b_cont->b_rptr;
			    if (iocp->ioc_cmd == TCSBRK && arg) {
				    if (tp->t_stop) {
					    /*
					     * wsrv will be sched by M_START
					     * or remote xon
					     */
					    rc = 1;
					    goto out;
				    }
				    break;
			    }

			    if (arg == 0)
				    arg = (250 * HZ) / 1000; /* 0.25 sec */
			    else
				    arg = (arg < 10 ? 10 : arg)*HZ/1000;

			    sty_break_set(tp, arg, mp);	/* send a break */
		        }
			return 0;

		    case TCSETXF:
		    case TCSETXW:
			if (cnt != sizeof (struct termiox) || !mp->b_cont)
				goto enospc;

			/*
			 * wait for the o/p to drain and flush i/p
			 * and set params
			 */
			if (iocp->ioc_cmd == TCSETXF) {
				flushq(RD(q),FLUSHDATA);
				putctl1(tp->t_queue->q_next,M_FLUSH,FLUSHR);
			}
			sty_termiox_set(tp, (void *) mp->b_cont->b_rptr, 0);
			break;

		    default:
			rc =  (*tp->t_ioctl)(q, mp, fromput);
			goto out;
		}
	} else {	/* From Put routine */
		switch (iocp->ioc_cmd) {
		    case TIOCSETAW:
		    case TIOCSETAF:
		    case TCSBRK:
		    case TCSBREAK:
		    case TCSETXF:
		    case TCSETXW:
			putq(q,mp);
			goto out;

		    case TIOCGETA:
			outcnt = sizeof (*tios);
			if (cnt < outcnt || !mp->b_cont)
				goto enospc;

			tios = (void *) mp->b_cont->b_rptr;
			bzero(tios, outcnt);
			sty_termios_get(tp, tios);    
			break;

		    case TIOCSETA:
			if (cnt < sizeof (*tios) || !mp->b_cont)
				goto enospc;

			tios = (void *)mp->b_cont->b_rptr;
			sty_termios_set(tp, tios, 0);
			break;

		    case TXTTYNAME:
			if (cnt < TTNAMEMAX || !mp->b_cont)
				goto enospc;

			bcopy(tp->t_name, mp->b_cont->b_rptr, TTNAMEMAX);
			outcnt = TTNAMEMAX;
			break;

		    case TIOCEXCL:
			tp->t_excl = 1;
			break;

		    case TIOCNXCL:
			tp->t_excl = 0;
			break;

		    case TIOCSWINSZ:
			break;

		    case TCSAK:
			if (cnt < sizeof(int) || !mp->b_cont)
				goto enospc;

			tp->t_sak = (*(int *) (mp->b_cont->b_rptr)) ? 1 : 0;
			break;

		    case TIOCSDTR:     
			(*tp->t_service)(tp, TS_GCONTROL, &signal_temp);
			if (!(signal_temp & TSDTR))
				(*tp->t_service)(tp, TS_SCONTROL
						 , signal_temp|TSDTR);
			break;

		    case TIOCCDTR:
			(*tp->t_service)(tp, TS_GCONTROL, &signal_temp);
			if (signal_temp & TSDTR)
				(*tp->t_service)(tp, TS_SCONTROL
						 , signal_temp & ~TSDTR);
			break;

		    case TIOCMBIS:
			if (cnt != sizeof (int) || !mp->b_cont)
				goto enospc;

			(*tp->t_service)(tp, TS_GCONTROL, &signal_temp);
			if (*(int *) mp->b_cont->b_rptr & TIOCM_DTR)
				signal_temp |= TSDTR;

			if (*(int *) mp->b_cont->b_rptr & TIOCM_RTS)
				signal_temp |= TSRTS;

			(*tp->t_service)(tp, TS_SCONTROL, signal_temp);
			outcnt = 0;
			break;

		    case TIOCMBIC:
			if (cnt != sizeof (int) || !mp->b_cont)
				goto enospc;

			(*tp->t_service)(tp, TS_GCONTROL, &signal_temp);
			if (*(int *) mp->b_cont->b_rptr & TIOCM_DTR)
				signal_temp &= ~TSDTR;

			if (*(int *) mp->b_cont->b_rptr & TIOCM_RTS)
				signal_temp &= ~TSRTS;

			(*tp->t_service)(tp, TS_SCONTROL, signal_temp);
			outcnt = 0;
			break;

		    case TIOCMSET:
			if (cnt != sizeof (int) || !mp->b_cont)
				goto enospc;

			signal_temp = 0;
			if (*(int *) mp->b_cont->b_rptr & TIOCM_DTR)
				signal_temp |= TSDTR;

			if (*(int *) mp->b_cont->b_rptr & TIOCM_RTS)
				signal_temp |= TSRTS;

			(*tp->t_service)(tp, TS_SCONTROL, signal_temp);
			outcnt = 0;
			break;

		    case TCGETX:
			outcnt = sizeof (*tiox);
			if (cnt < outcnt || !mp->b_cont)
				goto enospc;

			tiox = (void*) mp->b_cont->b_rptr;
			bzero(tiox, outcnt);

			sty_termiox_get(tp, tiox);    
			break;

		    case TCSETX:
			if (cnt < sizeof (*tiox) || !mp->b_cont)
				goto enospc;

			tiox = (void *) mp->b_cont->b_rptr;
			sty_termiox_set(tp, tiox, 0);
			break;

		    case TIOCOUTQ: {
			    mblk_t *mp1;
			    int outsize = 0;

			    if (cnt < sizeof(int) || !mp->b_cont)
				    goto enospc;

			    for (mp1 = q->q_first; mp1; mp1 = mp1->b_next) {
				    if (mp1->b_datap->db_type != M_DATA)
					    continue;

				    outsize += mp1->b_wptr - mp1->b_rptr;
			    }
			    *(int *) mp->b_cont->b_rptr = outsize;
			    outcnt = sizeof(int);
		        }
			break;

		    case TCLOOP:
			if (cnt < sizeof(int) || !mp->b_cont)
				goto enospc;

			/*
			 * Enter/leave the UART diagnostic loop mode
			 */
			(*tp->t_service)(tp, TS_LOOP
					 , (*(int *) mp->b_cont->b_rptr == 1
					    ? LOOP_ENTER : LOOP_EXIT));

			outcnt = sizeof(int);
			break;

		    default:
			rc =  (*tp->t_ioctl)(q, mp, fromput);
			goto out;
		}
	}
	mp->b_datap->db_type = outtype;
	iocp->ioc_count = outcnt;
	if (mp->b_cont)
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + outcnt;
	qreply(q,mp);

	rc = (outtype == M_IOCACK) ? 0 : -1;
    out:
	Return(rc);
}

/*
 * sty_ctl
 *      Processes the M_CTL messages.
 *      If successfully processed, the same  M_CTL is
 *      possibly sent upstream, otherwise it is freed.
 *
 * Input:
 *      q       - address of STREAMS write queue
 *      mp      - M_CTL to process.
 */
void
sty_ctl(queue_t *q, mblk_t *mp) {
	register struct sty_tty *tp = (struct sty_tty *)q->q_ptr;
	struct iocblk *iocp;
	struct termios *tios;
	struct termiox *tiox;
	mblk_t *mp1;

	iocp = (struct iocblk *)mp->b_rptr;

	db_trace(TR_TTY, (tp, "sty: ctl mp %x cmd %x", mp, iocp->ioc_cmd));

	switch (iocp->ioc_cmd) {
	    case TIOCGETA:
		if ((mp1 = mp->b_cont)
		    && (mp1->b_datap->db_lim - mp1->b_wptr >= sizeof *tios)) {
			/*
			 * this control message is used to obtain the
			 * "default" termios settings which have been
			 * saved from config.
			 */
			 tios = (struct termios *) mp1->b_wptr;
			*tios = tp->t_dflt.tios;	/* safe!	*/
			mp1->b_wptr += sizeof *tios;
		} else {
		    freeret:
			freemsg(mp);
			return;
		}
		break;

	    case TCGETX:
		if ((mp1 = mp->b_cont)
		    && (mp1->b_datap->db_lim - mp1->b_wptr >= sizeof *tiox)) {
			/*
			 * this control message is used to obtain the
			 * "default" termiox settings which have been
			 * saved from config.
			 */
			 tiox = (struct termiox *) mp1->b_wptr;
			*tiox = tp->t_dflt.tiox;	/* safe!	*/
			mp1->b_wptr += sizeof *tiox;
		} else
			goto freeret;
		break;

            case TXTTYNAME:
        	if (!(mp1 = mp->b_cont) ||
            	(mp1->b_datap->db_lim - mp1->b_rptr < TTNAMEMAX)) {
            	freemsg(mp);
            	return;
        	}
        	bcopy(tp->t_name,(char *)mp1->b_rptr,sizeof(tp->t_name));
        	mp1->b_wptr = mp1->b_rptr + sizeof(tp->t_name);
        	break;

	    case MC_CANONQUERY:
	    case TIOC_REQUEST:
	    case TIOC_REPLY:
	    case TIOCGETMODEM:
	    case TIOCMGET:
		/*
		 * these control messages are handled by the h/w
		 */
		(*tp->t_ctl)(q, mp);
		return;

	    default:
		goto freeret;
	}

	qreply(q, mp);
}

/*
 * sty_termios_set
 *      Sets the c_cflag, the IXON, IXOFF, IXANY bits
 *      of the c_iflag and VSTART and VSTOP charcters
 *      of the c_cc[] from the given tios.
 *      Calls tp->t_service to perform harware operations.
 *
 * Input:
 *      tp      - sty_tty struct pointer.
 *      tios    - pointer to termios structure to be set.
 *	force	- !0 => force all changes
 */
void
sty_termios_set(struct sty_tty *tp, struct termios * tios, int force) {
	parity_t newparity;
	csize_t newcsize;

	db_trace(TR_TTY, (tp, "sty: termios_set"));

	if (force || tp->t_ospeed != compatspcodes[cfgetospeed(tios)]) {
		tp->t_ospeed = compatspcodes[cfgetospeed(tios)];
		(*tp->t_service)(tp,TS_SBAUD,tp->t_ospeed);
	}

	switch (tios->c_cflag & CSIZE) {
	    case CS5:
		newcsize = bits5; break;

	    case CS6:
		newcsize = bits6; break;

	    case CS7:
		newcsize = bits7; break;

	    case CS8:
	    default:
		newcsize = bits8; break;
	}

	if (force || tp->t_csize != newcsize)
		(*tp->t_service)(tp,  TS_SBPC, newcsize);

	if (force || (tp->t_stopbits
		      != ((tios->c_cflag & CSTOPB) ? stop2 : stop1))) {
		tp->t_stopbits = (tios->c_cflag & CSTOPB) ? stop2 : stop1;
		(*tp->t_service)(tp,  TS_SSTOPS, tp->t_stopbits);
	}

	tp->t_cflag = tios->c_cflag;	/* save for posterity	*/

	switch (tios->c_cflag & (PARENB|PARODD|PAREXT)) {
	    default:
		newparity = nopar;	break;
	    case PARENB:
		newparity = evenpar;	break;
	    case PARENB|PARODD:
		newparity = oddpar;	break;
	    case PARENB|PAREXT:
		newparity = spacepar;	break;
	    case PARENB|PAREXT|PARODD:
		newparity = markpar;	break;
	}

	if (force || tp->t_parity != newparity)
		(*tp->t_service)(tp,  TS_SPARITY, newparity);

	tp->t_clocal = (tios->c_cflag & CLOCAL) ? 1 : 0;
	tp->t_cread  = (tios->c_cflag & CREAD)  ? 1 : 0;
	tp->t_hupcl  = (tios->c_cflag & HUPCL)  ? 1 : 0;

	tp->t_softchars[0] = tios->c_cc[VSTART];
	tp->t_softchars[1] = tios->c_cc[VSTOP];

	tp->t_softpacing = tios->c_iflag & (IXON|IXOFF|IXANY);

	/*
	 * there are unfortunately two ways to specify the t_flow_disc.
	 */
	tp->t_flow_disc &= ~flo_swmask;
	if (tp->t_softpacing & IXON)
		tp->t_flow_disc |= flo_osw;	/* oput flo using s/w	*/
	if (tp->t_softpacing & IXOFF)
		tp->t_flow_disc |= flo_isw;	/* iput flo using s/w	*/

	/*
	 * if s/w flow control enabled, disable and h/w modes
	 */
	if (tp->t_flow_disc & flo_swmask)
		tp->t_flow_disc &= flo_swmask;

	db_trace(TR_TTY, (tp, "sty: termios_set %x -> %x"
			  , tp->t_softpacing, tp->t_flow_disc));
}

/*
 * sty_termios_get
 *      Gets the c_cflag, the IXON, IXOFF, IXANY bits 
 *      of the c_iflag and VSTART and VSTOP characters
 *      of the c_cc[] from the local sty_tty structure.
 *
 * Input:
 *      tp      - sty_tty struct pointer.
 *      tios    - pointer to termios structure to be returned.
 */
void
sty_termios_get(struct sty_tty *tp, struct termios * tios) {
	tcflag_t speed = 0;
	int i;

	db_trace(TR_TTY, (tp, "sty: termios_get"));

	tios->c_cflag = 0;

	for (i = 0; (i <= 15 && compatspeeds[i].sp_speed != tp->t_ospeed); ++i)
		;

	if (i <= 15)
		speed = compatspeeds[i].sp_code;

	cfsetospeed(tios, speed);
	cfsetispeed(tios, speed);

	switch (tp->t_csize) {
	    case bits5:
		tios->c_cflag |= CS5;
		break;
	    case bits6:
		tios->c_cflag |= CS6;
		break;
	    case bits7:
		tios->c_cflag |= CS7;
		break;
	    case bits8:
		tios->c_cflag |= CS8;
		break;
	    default:
		break;
	}

	switch(tp->t_stopbits) {
	    case stop2:
		tios->c_cflag |= CSTOPB;
		break;
	    case stop1:
		tios->c_cflag &= ~CSTOPB;
		break;
	    default:
		break;
	}

	/*
	 * XXX: all this dancing about with c_cflags might be cleaned up.
	 */
	tios->c_cflag |= tp->t_cflag & (PAREXT|PARENB|PARODD);
	
	if (tp->t_clocal)
		tios->c_cflag |= CLOCAL;

	if (tp->t_cread)
		tios->c_cflag |= CREAD;

	if (tp->t_hupcl)
		tios->c_cflag |= HUPCL;
    
	tios->c_iflag = (tp->t_flow_disc & flo_swmask) ? tp->t_softpacing : 0;

	tios->c_cc[VSTART] = tp->t_softchars[0];
	tios->c_cc[VSTOP]  = tp->t_softchars[1];
}

/*
 * sty_termiox_set
 *      Sets the x_hflag from the given tiox.
 *
 * Input:
 *      tp      - sty_tty struct pointer.
 *      tiox    - pointer to termiox structure to be set.
 *	force	- use the force, puke!
 */
void
sty_termiox_set(struct sty_tty *tp, struct termiox * tiox, int force) {

	/*
	 * XXX: make sure that the flow control change should not have
	 * caused us to unblock, due to modem state.
	 */
	tp->t_hardpacing = tiox->x_hflag;

	if ((tp->t_hardpacing & (RTSXOFF|CTSXON)) == (RTSXOFF|CTSXON))
		tp->t_flow_disc = flo_irts | flo_octs;
	else if ((tp->t_hardpacing & (DTRXOFF|CDXON)) == (DTRXOFF|CDXON))
		tp->t_flow_disc = flo_idtr | flo_odcd;
	else
		tp->t_flow_disc &= ~flo_hwmask;

	/*
	 * if we just set a h/w flo control mode, clear any s/w modes.
	 */
	if (tp->t_flow_disc & flo_hwmask)
		tp->t_flow_disc &= flo_hwmask;

	db_trace(TR_TTY, (tp, "sty: termiox_set %x -> %x"
			  , tiox->x_hflag, tp->t_flow_disc));

}

/*
 * sty_termiox_get
 *      Gets the x_hflag from the local sty_tty structure.
 *
 * Input:
 *      tp      - sty_tty struct pointer.
 *      tiox    - pointer to termiox structure to be returned.
 */
void
sty_termiox_get(struct sty_tty *tp, struct termiox * tiox) {

	db_trace(TR_TTY, (tp, "sty: termiox_get"));

	tiox->x_hflag = (tp->t_flow_disc & flo_hwmask) ? tp->t_hardpacing : 0;
}

/*
 * sty_break_set
 *      Sets break condition for the requested duration.
 *
 * Input:
 *      tp       - sty_tty struct pointer.
 *      duration - time for which the break condition is set.
 *	mp	 - ^ to mblk which will be used to ACK the ioctl req
 */
void
sty_break_set(struct sty_tty *tp, int duration, mblk_t *mp) {
	register x;

	db_trace(TR_TTY, (tp, "sty: break_set"));

	x = disable_lock(tp->t_hpri, &tp->t_lock);

	(*tp->t_service)(tp,TS_SBREAK,0);

	if (duration) {
		ASSERT(!tp->t_ioctlmsg && !tp->t_btimeout);

		if (tp->t_ioctlmsg)
			freemsg(tp->t_ioctlmsg);
		if (tp->t_btimeout)
			untimeout(tp->t_btimeout);

		tp->t_ioctlmsg = mp;
		tp->t_btimeout = timeout(sty_break_clear
					 , (caddr_t)tp, duration);
	}
	unlock_enable(x, &tp->t_lock);
}

/*
 * sty_break_clear
 *      Clears break condition.
 *
 * Input:
 *      tp       - sty_tty struct pointer.
 */
void
sty_break_clear(struct sty_tty *tp) {
	register mblk_t *mp;

	db_trace(TR_TTY, (tp, "sty: break_clear: mp = %x", tp->t_ioctlmsg));

	tp->t_btimeout = 0;
	(*tp->t_service)(tp,TS_CBREAK,0);

	if (mp = tp->t_ioctlmsg) {
		tp->t_ioctlmsg = 0;
		mp->b_datap->db_type = M_IOCACK;
		((struct iocblk *)(mp->b_rptr))->ioc_count = 0;
		qreply(WR(tp->t_queue), mp);
	}
}

/*
 * sty_register -	register new serial devices with the STY
 *
 * Input:
 *	devi	-	^ to device info
 *
 * Returns:
 *	 0	-	 ok
 *	!0	-	!ok
 */
sty_register(struct ttydevs *devi) {
	register struct ttydevs **vp, *v;
	register rc = 0;

	/*
	 * the config entry point is not always called, so hook into the
	 * debugger here as well.
	 */
	if (!koff_once)
		dbg_register();

	db_trace(TR_TTY, (devi, "sty: register"));

	assert(!devi->busy);

	lockl(&tty_mutex, LOCK_SHORT);

	/*
	 * find ttys associated with this device
	 */
	for (vp = &ttydevsw; (v = *vp) && v->dev != devi->dev; vp = &v->nxt)
		;

	if (*vp) {
		/*
		 * already registered!
		 */
		rc = EBUSY;
	} else {
		styconf.sc_major = major(devi->dev);
		styconf.sc_name  = devi->name;

		if (rc = str_install(STR_LOAD_DEV, &styconf))
			sty_err("sty", ERRID_COM_CFG_DEVA, rc);
		else {
			if (ttyDBG)
				lldb_register(1, devi->name, devi->print);

			devi->busy   = 1;	/* on list	*/
			devi->isopen = 0;
			*vp = devi;
		}
	}

	unlockl(&tty_mutex);

	return rc;
}

/*
 * sty_unregister -	un-register serial devices with the STY
 *
 * Input:
 *	devi	-	^ to device info
 *
 * Note:
 *	it is ok to unregister while there exist `tp's that were discovered
 *	by using this registration.  It just means that no more serial
 *	devices can be opened using the old <dev, *devi> mapping.
 *	Thus, we don't bother to use count the *devi.
 */
sty_unregister(struct ttydevs *devi) {
	register struct ttydevs **vp, *v;
	register rc = 0;

	db_trace(TR_TTY, (devi, "sty: unregister"));

	if (!devi->busy)
		return EINVAL;

	lockl(&tty_mutex, LOCK_SHORT);

	/*
	 * find this registration in the list
	 */
	for (vp = &ttydevsw; (v = *vp) != devi; vp = &v->nxt)
		;

	if (*vp != devi) {
		/*
		 * not registered!
		 */
		rc = EINVAL;
	} else {
		styconf.sc_major = major(devi->dev);
		styconf.sc_name  = devi->name;

		if (rc = str_install(STR_UNLOAD_DEV, &styconf))
			sty_err("sty", ERRID_COM_CFG_DEVA, rc);
		else {
			devi->busy   = 0;	/* !on list	*/
			*vp = devi->nxt;

			if (ttyDBG)
				lldb_register(0, devi->name, devi->print);
		}
	}

	unlockl(&tty_mutex);

	return rc;
}

/*
 * styrsrv -	Read servce procedure.
 *
 * Input:
 *      q       - Driver's Streams read queue.
 *
 * Returns: 1
 *
 *	We handle "off level" processing here, rather than scheduling
 *	off-level handler.  This should be the moral equivalent of
 *	off-level processing.
 *      Priority messages are forwarded upstream immediately.
 *      Ordinary messages are sent when the module abive the driver can
 *      accept them. If not, they are left for back-enabling.
 */
styrsrv(queue_t *q) {
	register struct sty_tty *tp = (struct sty_tty *) q->q_ptr;
	mblk_t *mp;
	char *find_mtype();

	Enter(TRC_STY(TTY_RSRV), tp->t_dev, tp, q->q_count, 0, 0);

	db_trace(TR_TTY, (tp, "sty: rsrv q_count = %x", q->q_count));

	/*
	 * process all pending stuff first
	 */
	while (mp = getq(q)) {
		db_trace(TR_TTY, (tp, "sty: rsrv mp %x type %s"
				  , mp, find_mtype(mp->b_datap->db_type)));

		switch (mp->b_datap->db_type) {
		    case M_DATA:
			if (!tp->t_cread) {
				freemsg(mp);
				break;
			}
			/*
			 * fall thru...
			 */
		    default:
			if (QPCTL <= mp->b_datap->db_type
			    || canput(q->q_next)) {
				putnext(q, mp);
			} else {
				putbq(q, mp);
				/*
				 * note: the upstream queue is full.
				 * we know nothing about the device, which
				 * is assumed to have sufficient buffering,
				 * in addition to being smart enough to be
				 * able to look at (q->q_flags & QFULL).
				 * Thus, we leave it upto the device
				 * to decide when to flow control itself.
				 */
				goto out;
			}
		}
	}

    out:
	/*
	 * now call down to the h/w again
	 */
	(*tp->t_rsrv)(tp, 0);

	Return(1);
}

/*
 * stywput -	Driver's write put procedure.
 *
 * Input:
 *      q       - Driver's Streams write queue.
 *      mp      - message to process.
 *
 * Returns: 0.
 */
stywput(queue_t *q, mblk_t *mp) {
	register struct sty_tty *tp = (struct sty_tty *) q->q_ptr;
	register mblk_t *nmp;
	register op, old_intr;
	char *find_mtype();

	Enter(TRC_STY(TTY_WPUT), tp->t_dev, tp, mp, mp->b_datap->db_type, 0);

	db_trace(TR_TTY
		 , (tp, "sty: wput mp %x %s"
		    , mp, find_mtype(mp->b_datap->db_type)));

	switch (mp->b_datap->db_type) {
	    case M_BREAK:
	    case M_DELAY:
		putq(q,mp);
		break;

	    case M_DATA:
		/*
		 * If not currently busy, we start output directly.
		 * Otherwise, we put on queue without enabling the wsrv.
		 */
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);

		if (!q->q_count && !tp->t_busy && !tp->t_stop) {
			tp->t_outmsg = mp;
			(*tp->t_proc)(tp, T_OUTPUT);
		} else {
			noenable(q);
			putq(q, mp);
			enableok(q);
		}

		unlock_enable(old_intr, &tp->t_lock);
		break;
        
	    case M_IOCTL:
		sty_ioctl(q,mp,1);
		break;

	    case M_CTL:
		sty_ctl(q,mp);
		break;

	    case M_FLUSH:{
		int flag = 0;

		if (*mp->b_rptr & FLUSHW) {
			flag |= FLUSHW;
			*mp->b_rptr &= ~FLUSHW;
		}
		if ( *mp->b_rptr & FLUSHR) {
			flag |= FLUSHR;
			qreply(q,mp);
		} else
			freemsg(mp);

		sty_flush(tp,flag,0);    
		break;
	    }
		
	/*
	 * These messages are generated by ldterm to control the output flow
	 */
	    case M_STOP:
		tp->t_localstop = 1;	/* remember the M_STOP	*/
		op = T_SUSPEND;
	    t_proc:
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
		(*tp->t_proc)(tp, op);
	    unlock_free:
		unlock_enable(old_intr, &tp->t_lock);
		freemsg(mp);
		break;

	    case M_START:
		tp->t_localstop = 0;	/* forget the M_STOP	*/
		op = T_RESUME;
		goto t_proc;

	/*
	 * These messages are generated by ldterm to control the input flow
	 */
	    case M_STOPI:
		op = T_BLOCK;
		goto t_proc;

	    case M_STARTI:
		op = T_UNBLOCK;
		goto t_proc;

	/*
	 * M_PROTO messages: sent by sptr module
	 */
	    case M_PROTO: {
		mblk_t *mp1;

		if (*(int *)mp->b_rptr != LPWRITE_REQ) {
			freemsg(mp);
			break;
		}
		mp1 = unlinkb(mp);    /* save the M_PROTO block to be
					 enqueued at the end */
		while (mp1) {
			nmp = unlinkb(mp1);
			putq(q,mp1);
			mp1 = nmp;
		}
		putq(q,mp);
		break;
	    }

	    default:
		freemsg(mp);
		break;
	}

	Return(0);
}

/*
 * stywsrv -	Write servce procedure.
 *      Processes as many messages as hardware allows.
 *      If exits before completion than will be called
 *      again (qenable) by off-level routine.
 *
 * Input:
 *      q       - Driver's Streams write queue.
 *
 * Returns: 0.
 */
stywsrv(queue_t *q) {
	struct sty_tty *tp = (struct sty_tty *)q->q_ptr;
	int (*f)();
	mblk_t *mp;
	int old_intr;

	Enter(TRC_STY(TTY_WSRV), tp->t_dev, tp, q->q_count, 0, 0);

	db_trace(TR_TTY, (tp, "sty: wsrv q_count %x", q->q_count));

	old_intr = disable_lock(tp->t_hpri, &tp->t_lock);

	/*
	 * h/w requires write processing
	 *	this is supposed to be the most frequent path thru the
	 *	code, usually returning.
	 */
	if (f = tp->t_wsrv) {		/* atomic load		*/
		tp->t_wsrv = 0;		/* .. and clear!	*/
		(*f)(tp, 1);

		if (tp->t_busy) {
		    unlock_ret:
			unlock_enable(old_intr, &tp->t_lock);
			Return(0);
		}
	}

	while (mp=getq(q)) {
		db_trace(TR_TTY, (tp, "sty: wsrv mp %x type %s"
				  , mp, find_mtype(mp->b_datap->db_type)));

		switch(mp->b_datap->db_type) {
		    case M_DATA:
			/*
			 * this is expected to be the second-most frequent
			 * path thru the code.
			 */
			if (tp->t_busy || tp->t_stop) {
			    putb_return:
				putbq(q,mp);
				goto unlock_ret;
			}
			if (mp->b_wptr >= mp->b_rptr || mp->b_cont) {
				tp->t_outmsg = mp;

				(*tp->t_proc)(tp, T_OUTPUT);
				continue;
			}
		    unlock_free:
			unlock_enable(old_intr, &tp->t_lock);
		    free_relock:
			freemsg(mp);
			old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
			continue;

		    case M_DELAY:
			if (!tp->t_stop) {
				if (tp->t_timeout)
					untimeout(tp->t_timeout);

				tp->t_timeout
					= timeout(sty_timeout
						  , (caddr_t) tp
						  , * ((int *) mp->b_rptr));

				(*tp->t_proc)(tp, T_SUSPEND);
			}
			goto unlock_free;

		    case M_BREAK:
			if (tp->t_busy)
				goto putb_return;

			unlock_enable(old_intr, &tp->t_lock);

			if (*(int *)mp->b_rptr)
				sty_break_set(tp, 0, 0);/* send a break */
			else
				sty_break_clear(tp);	/* clear the break */

			goto free_relock;

		    case M_IOCTL:
			if (tp->t_busy)
				goto putb_return;

			unlock_enable(old_intr, &tp->t_lock);
			sty_ioctl(q,mp,0);
			break;

		/*
		 * Inform the serial printer module that its last output
		 * request was terminated (what?)
		 */
		    case M_PROTO:
			/* last M_DATA not completely transmitted yet */
			if (tp->t_outmsg)
				goto putb_return;

			unlock_enable(old_intr, &tp->t_lock);
			mp->b_datap->db_type = M_PCPROTO;
			*(int *)mp->b_rptr = LPWRITE_ACK;
			qreply(q, mp);
			break;

		    default:
			goto unlock_free;
		}
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
	}

	unlock_enable(old_intr, &tp->t_lock);

	/* All pending data at close time was drained down:
	 * wake up closing thread.
	 * This is needed here because the last can other than M_DATA.
	 */
	if (!mp && tp->t_draining) {
		db_trace(TR_TTY, (tp, "sty: wsrv drain-o"));
		/* simple_lock(&tp->t_lock);    168803 */
		old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
		tp->t_draining = 0;
		TTY_WAKE(tp);
		/* simple_unlock(&tp->t_lock);  168803 */
	        unlock_enable(old_intr, &tp->t_lock);
	}

	Return(0);
}

/*
 * sty_allocmsg
 *      Allocate a new message for received input.  Arrange a callback
 *      if we can't get one now.
 *
 * Input:
 *	tp	- pointer to sty_tty struct
 *	gotlock	- !0 => already locked/disabled
 *
 * Returns:
 *	!0	-	 ok: was able to allocate
 *	 0	-	!ok: bufcall or timeout running
 */
sty_allocmsg(struct sty_tty *tp, int gotlock) {
	register x;

	if (!gotlock)
		x = disable_lock(tp->t_hpri, &tp->t_lock);

	if (tp->t_inmsg = allocb(STY_RDBUFSZ, BPRI_MED)) {
		db_trace(TR_TTY, (tp, "sty: allocmsg %x", tp->t_inmsg));
		ASSERT(tp->t_inmsg->b_datap);	/* not free	*/
	} else if (!tp->t_bufcall) {
    	        if (!(tp->t_bufcall = bufcall(STY_RDBUFSZ, BPRI_MED
			                      , sty_recover, (caddr_t)tp))) {
			if (!tp->t_alloctimer)
				tp->t_alloctimer = timeout(sty_recover
							   , (caddr_t)tp, hz);
		} else {    /* bufcall request recorded: give up calling it */
			if (tp->t_alloctimer) {
				untimeout(tp->t_alloctimer);
				tp->t_alloctimer = 0;
			}
                }
	}

	db_trace(TR_TTY, (tp, "sty:!allocmsg bufcall %x alloct %x"
			  , tp->t_bufcall, tp->t_alloctimer));

	if (!gotlock)
		unlock_enable(x, &tp->t_lock);

	return (tp->t_inmsg != 0);
}

/*
 * sty_recover	-	recover from allocb() failures
 *
 * Input:
 *	tp    - pointer to sty_tty struct
 *
 * Returns: 0.
 *
 * SIDE EFFECT:
 *	qenable read queue
 */
sty_recover(struct sty_tty *tp) {
	int old_intr;

	db_trace(TR_TTY, (tp, "sty: recover"));

	old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
	
	tp->t_bufcall = 0;

	if (sty_allocmsg(tp, 1)) {
		/*
		 * enable the read service proc, now that we have another
		 * input buffer
		 */
		qenable(tp->t_queue);
	}

	unlock_enable(old_intr, &tp->t_lock);

	return 0;
}

/* 
 * sty_timeout
 *      Function argument to timeout utility.
 *      resets the flag t_timeout int the  sty_tty struct.
 *
 * Input:
 *      tp    - pointer to sty_tty struct
 */
void
sty_timeout(struct sty_tty *tp) {
	int old_intr;

	db_trace(TR_TTY, (tp, "sty:!timeout"));

	old_intr = disable_lock(tp->t_hpri, &tp->t_lock);
	tp->t_timeout = 0;
                      
	/* timeout call is due to M_DELAY processing */
	if (tp->t_stop)
		(*tp->t_proc)(tp,T_RESUME);

	unlock_enable(old_intr, &tp->t_lock);
}

/*
 * sty_err -	handle STY error logging
 */
sty_err(char *name, int code, int err) {
	ERR_REC(sizeof(int)) cfg_err;

	db_trace(TR_TTY, (0, "sty:*err(%s, %x, %x)", name, code, err));

	cfg_err.error_id = code;
	bcopy(name, cfg_err.resource_name, sizeof(cfg_err.resource_name));
	*(int *)cfg_err.detail_data = err;
	errsave(&cfg_err, sizeof(cfg_err));
}

/*
 * dbg_register -	register with the currently in-favour debugger
 */
static
dbg_register() {
#ifdef PSE_DBTRACE_HOOKS
	/*
	 * Occasionally, I need db_trace from PSE.  I do this by
	 * exporting pse_trace_func from PSE, and defining db_trace()
	 * in PSE in terms of the pse_trace_func.  Thus, the
	 * PSE trace hooks will appear in my buffer.  This is
	 * particularly useful for things like alloc/freeb/freemsg
	 * traces, including caller and caller's caller.
	 * If I'm lucky, one of these days I will get to drop the code into
	 * PSE, so that I don't have to keep recreating it....
	 */
	extern void *(*pse_trace_func)();

	pse_trace_func = _db_trace;
#endif

	koff_addsyms(db_syms, db_nsyms);
	k_str(0,0);
	++koff_once;
}
