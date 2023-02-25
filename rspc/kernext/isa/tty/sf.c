static char sccsid[] = "@(#)58	1.32.2.1  src/rspc/kernext/isa/tty/sf.c, isatty, rspc41J, 9524D_all 6/14/95 03:16:41";
/*
 *   COMPONENT_NAME: isatty
 *
 *   FUNCTIONS:	sf_close, sf_config, sf_config_gorp, sf_ctl, sf_ioctl
 *		sf_offl, sfc_open, sf_open_port, sf_proc, sf_service
 *		sf_slih
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sf.c -	sandalfoot 16550-based serial port driver
 *
 * Note:
 *	I have split as much of the common STREAMS/tty function into another
 *	module ("sty").  This driver is tightly coupled to the "sty" driver.
 */
#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks		*/
#include <sys/sysmacros.h>		/* minor and major macros	*/
#include <sys/intr.h>			/* interrupt handler stuff	*/
#include <sys/pin.h>			/* pinning stuff		*/
#include <sys/adspace.h>		/* address space junk		*/
#include <sys/ioacc.h>			/* bus access macros		*/
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
#include "common_open.h"		/* Common open discipline fns	*/
#include <sys/rs.h>			/* rs's ioctl's			*/
#include <sys/inline.h>
#include "sty.h"
#include "sf.h"
#ifdef PCMCIA
#include <sys/atomic_op.h>
#include "pcmciaDD.h"
#endif

/*
 * additional sf_proc()s
 */
#define T_BLOCKx	((1 << 31) | T_BLOCK)
#define T_UNBLOCKx	((1 << 31) | T_UNBLOCK)

/*
 * use in-line version of brkpoint() so that optimizer can have a chance
 */
#pragma mc_func brkpoint { "7c810808" }	/* t	0x4, r1, r1	*/
#pragma reg_killed_by	brkpoint

/*
 * This is the default configuration for SandalFoot
 */
struct sf_config sf_conf[NSFCOM] = {
	{ SFIO_BID(0), 115200, 0x3f8, 4, 0, TRIG_14, SF_FIFOCNT },
	{ SFIO_BID(0), 115200, 0x2f8, 3, 0, TRIG_14, SF_FIFOCNT },
};

/*
 * tioc_reply structures array. It is given as an answer to TIOC_REQUEST
 * M_CTLs.
 */
struct tioc_reply sf_tioc_reply[] = {
	{ RS_SETA, sizeof(struct rs_info), TTYPE_COPYIN },
	{ RS_GETA, sizeof(struct rs_info), TTYPE_COPYOUT },
};

/*
 * watchdog timer turb
 */
static struct trb *puppy = NULL;
static struct trb *trb = NULL;

/*
 * sf_config -	configuration entry point for SF serial devices
 *
 * Input:
 *	dev	-	device encoding
 *	cmd	-	configuration command
 *	uiop	-	^ to user space params
 */
sf_config(dev_t dev, int cmd, struct uio *uiop) {
	register rc = 0;
	static int lock = LOCK_AVAIL;
	static int once;

	Enter(TRC_SF(TTY_CONFIG), 0, 0, cmd, 0, 0);

	(void) lockl(&lock, LOCK_SHORT);

	switch (cmd) {

	    case CFG_INIT:
		if (rc = sf_config_gorp(dev, CFG_INIT, uiop))
			break;

		if (!once++) {
			/*
			 * perform once-only initialization
			 */
			koff_addsyms(db_syms, db_nsyms);
			k_rs(0);
		}
		break;

	    case CFG_TERM:
		/*
		 * we are called for each line that had been configured.
		 */
		if (rc = sfc_close(dev))
			break;

		if (!--once) {
			koff_delsyms(db_syms);
			k_rs(-1);
		}
		break;

	    default:
		rc = EINVAL;
	}

    out:
	unlockl(&lock);

	Return(rc);
}

/*
 * sf_config_gorp -
 *
 * Input:
 *	dev	-	maj/min for unit
 *	cmd	-	CFG_INIT or CFG_TERM
 *	uiop	-	^ to user supplied data area
 *
 *    In the adapter config method, a minor number is generated
 *    by setting the high order bit on the value set for the 
 *    port number. This is not important for the configuration, beacuse
 *    the config routine uses the SFDDS_DEVICE and SFDDS_LINE
 *    to configure the adapter and port.
 *    adap = unit & 0x8000; This becomes important when the adapter
 *    gets unconfigured.
 */
sf_config_gorp(dev_t dev, int cmd, struct uio *uiop) {
	register unit, rc;
	register struct sfcom *sc;
	struct sf_dds cf;

	if (rc = uiomove((caddr_t)&cf, sizeof (cf), UIO_WRITE, uiop)) {
		sty_err("sf", ERRID_COM_CFG_UIO, rc);
		return rc;
	}

	unit = minor(dev);
        unit = unit & 0x7fff;

	if (cf.rc_type)			/* only type 0 supported	*/
		return EINVAL;

	if ((ulong) unit >= NSFCOM)	/* must be in range		*/
		return ENXIO;

	if (cmd != CFG_INIT)
		return 0;

	sc = &sfcom[unit];

	/*
	 * config comes in two flavours: controller and unit.
	 */
	if (cf.rc_flags & SFDDS_DEVICE) {	/* device spec portion valid */
		if (sc->sf_cconf)		/* ctrl configured?	     */
			return EBUSY;		/* can't reconfig on the fly */

		/*
		 * convert rtrig to chip-useable form, and
		 * pre-compute amount to read on every FIFO full interrupt,
		 * based on what we will be programming the rtrig.
		 */
		switch (cf.rc_conf.rtrig) {
		    case 0:
			sc->sf_rcnt      = 1;
			cf.rc_conf.rtrig = TRIG_01;
			break;

		    case 1:
			sc->sf_rcnt      = 4;
			cf.rc_conf.rtrig = TRIG_04;
			break;

		    case 2:
			sc->sf_rcnt      = 8;
			cf.rc_conf.rtrig = TRIG_08;
			break;

		    case 3:
			sc->sf_rcnt      = 14;
			cf.rc_conf.rtrig = TRIG_14;
			break;

		    default:
			return EINVAL;
		}

		/*
		 * copy in the config info, and set the "debugger available"
		 * flag for the tty, based on the (XXX undocumented XXX)
		 * !debug bit.  This bit will normally be zero, which will
		 * enable the debugger.
		 */
		sc->sf_conf = cf.rc_conf;
		sc->sf_dbg  = (unit == 0 && !(sc->sf_flags&1));

#ifdef PCMCIA
	    {
		int ipri; /* previous interrupt priority */

		/* copy logical socket number to sfcom */
		sc->pcmcia.logical_socket = cf.pcmcia_dds.logical_socket;

		/* check CardServices's revision and level */
		rc = CheckCS();
		if ( rc != 0 ){
			return EIO;
		}
		/* card insertion event will make no_card FALSE */
		sc->pcmcia.no_card = 1;

		sc->pcmcia.event_word = EVENT_NULL;
		sc->pcmcia.reg_comp = 0;
		sc->pcmcia.cb_flag = 0;

     		/* call CardServices(RegisterClient) */
		rc = RegisterClient(sc);
		if ( rc != 0 ){
			return EIO;
		}

		ipri = i_disable( INTMAX );
		if ( sc->pcmcia.reg_comp == 0 ){
		    /* wait until CardInsertion callback */
		    rc = e_sleep(& sc->pcmcia.event_word, EVENT_SIGRET );
		}
		i_enable( ipri );

		if ( rc == EVENT_SIG || sc->pcmcia.return_callback
			|| sc->pcmcia.no_card == 1 )
		{
			(void)DeregisterClient(sc);
			return ENXIO;

		}

		/* here, rc == EVENT_SUCC, so reset is needed */
		rc = 0;
	    }
#endif /* PCMCIA */

#ifdef PM_SUPPORT
	    {
		int i;
		int sf_pm_handler(caddr_t, int);

		/* for pm_planar_control */
#ifdef PCMCIA
		sc->pm_device_id = PMDEV_PCMCIA00
			+ ((cf.pcmcia_dds.bus_number & 0xFF) << 4)
			+ (cf.pcmcia_dds.physical_socket & 0xFF) ;
#else
		if (sc == &sfcom[0]){
			sc->pm_device_id = PMDEV_SERIAL1;
		} else {
			sc->pm_device_id = PMDEV_SERIAL2;
		}
#endif

		/* setup for pm_handler struct */
		bzero( & sc->pm_handle, sizeof(sc->pm_handle) );
		sc->pm_handle.activity = PM_NO_ACTIVITY;
		sc->pm_handle.mode = PM_DEVICE_ENABLE;           
		sc->pm_handle.device_idle_time = cf.pm_dds.pm_dev_itime;
		sc->pm_handle.device_standby_time = cf.pm_dds.pm_dev_stime;
		sc->pm_handle.idle_counter = 0;
		sc->pm_handle.handler = (int (*)()) sf_pm_handler;
		sc->pm_handle.private = (caddr_t) sc;
		sc->pm_handle.devno = dev;
		sc->pm_handle.attribute = cf.pm_dds.pm_dev_att;
		sc->pm_handle.next1 = NULL;
		sc->pm_handle.next2 = NULL;
		sc->pm_handle.device_idle_time1 = 0;
		sc->pm_handle.device_idle_time2 = 0;

		i = 0;
		while ((sc->device_name[i] = cf.rc_name[i]) != '\0'
			&& i < sizeof(sc->device_name)
			&& i < sizeof(cf.rc_name))
		{
			i++;
		}

		sc->pm_handle.device_logical_name = sc->device_name;
		sc->pm_handle.reserve[0] = '\0';
		sc->pm_handle.pm_version = PM_PHASE2_SUPPORT;
		sc->pm_handle.extension = NULL;

		sc->pm_event_word = EVENT_NULL;

		rc = pm_register_handle( &(sc->pm_handle), PM_REGISTER );
		if ( rc != PM_SUCCESS ){
#ifdef PCMCIA
			(void)CardRemoval(sc);
			(void)DeregisterClient(sc);
#endif /* PCMCIA */
			return ENXIO;
		}

		/* power off the socket */
		sf_power(sc, PM_PLANAR_OFF);
	    }
#endif /* PM_SUPPORT */
		/*
		 * setup the spiffy new pio stuff
		 */
                sc->sf_iom.key     = IO_MEM_MAP;
                sc->sf_iom.flags   = 0;
                sc->sf_iom.size    = SEGSIZE;
                sc->sf_iom.bid     = sc->sf_conf.bid;
                sc->sf_iom.busaddr = 0;

		sc->sf_cconf       = 1;		/* ctlr configured	*/
	}

	if (cf.rc_flags & SFDDS_LINE) {	/* line specific portion valid	*/
		register struct sty_tty *tp;
		tp = &sc->sf_tty;
		if (!sc->sf_cconf)	/* ctrl must be configured	*/
			return ENXIO;

		/*
		 * convert rtrig to chip-useable form, and
		 * pre-compute amount to read on every FIFO full interrupt,
		 * based on what we will be programming the rtrig.
		 */
		switch (cf.rc_conf.rtrig) {		/* 171863 */
		    case 0:
			sc->sf_rcnt      = 1;
			sc->sf_rtrig = TRIG_01;
			cf.rc_conf.rtrig = TRIG_01;
			break;

		    case 1:
			sc->sf_rcnt      = 4;
			sc->sf_rtrig = TRIG_04;
			cf.rc_conf.rtrig = TRIG_04;
			break;

		    case 2:
			sc->sf_rcnt      = 8;
			sc->sf_rtrig = TRIG_08;
			cf.rc_conf.rtrig = TRIG_08;
			break;

		    case 3: 
			sc->sf_rcnt      = 14;
			sc->sf_rtrig = TRIG_14;
			cf.rc_conf.rtrig = TRIG_14;
			break;

		    default:
			return EINVAL;
		}
		sc->sf_tbc = cf.rc_conf.tbc;		/* 171863 */
		/*
		 * sfc_open needs the name for the lldb_register
		 */
		bcopy(cf.rc_name, sc->sf_tty.t_name
		      , sizeof (sc->sf_tty.t_name));

		/*
		 * open port controller
		 */
		if (rc = sfc_open(dev))
			return rc;

		/*
		 * save defaults
		 */
		tp->t_dflt.tios = cf.rc_tios;
		tp->t_dflt.tiox = cf.rc_tiox;

		sc->sf_uconf = 1;	/* unit now configured		*/

	}
	if (!trb) {	/* 168801 */
		trb = talloc();
		if (!trb) rc = ENOMEM;
	}

	return rc;
}

/*
 * sf_open_port	-	open port on controller
 */
struct sty_tty *
sf_open_port(register dev_t dev) {
	int sf_slih(), sf_offl();
	register struct sty_tty *tp;
	register struct sfcom *sc;
	register struct intr *ip;
	register x, unit;
	register volatile uchar *port;
	static lock_t mutex = LOCK_AVAIL;

	tp = &(sc = &sfcom[unit = minor(dev) & 0x7fff])->sf_tty;

#ifdef PM_SUPPORT
	pm_block( sc );
#endif

	if ((unsigned) unit >= NSFCOM || !sc->sf_uconf)
		return 0;

	db_trace(TR_TTY, (tp, "sf: open_port(%d)", unit));

	/*
	 * Handle first open initialization
	 */
	if (!sc->sf_open) {
#ifdef PM_SUPPORT
		simple_lock( &sc->pm_lock );
#endif
		if (pincode(sf_slih)) {		/* pin intr handler	*/
#ifdef PM_SUPPORT
			simple_unlock( &sc->pm_lock );
#endif
			return 0;
		}

		if (pin(sc, sizeof(*sc))) {	/* pin data		*/
		    err0:
			unpincode(sf_slih);
#ifdef PM_SUPPORT
			simple_unlock( &sc->pm_lock );
#endif
			return 0;
		}

		tp->t_queue = 0;
		tp->t_dev   = dev;
		/*
		 * register intr handler
		 */
		ip = &sc->sf_slih;
		tp = &sc->sf_tty;

		tp->t_wptr   = tp->t_rptr = &tp->t_rbuf[0];
		tp->t_hpri   = INTCLASS0;	/* h/w priority	*/
		ip->handler  = sf_slih;
		ip->bus_type = BUS_BID;
#ifndef PCMCIA
		ip->flags    = INTR_NOT_SHARED;
#endif
		ip->level    = sc->sf_irq;
		ip->priority = INTCLASS0;
		ip->bid      = sc->sf_bid;

#ifdef PM_SUPPORT
		if (sf_power(sc, PM_PLANAR_ON) != PM_SUCCESS){
			goto err1;
		}
#endif
#ifdef PCMCIA
		if (enable_pccard( sc ) != 0){
			goto err1;
		}
#endif

		if (i_init(ip) != INTR_SUCC) {
		    err1:
			unpin(sc, sizeof (*sc));
			goto err0;
		}

		/*
		 * XXX: clean up all the openDisc gorp.
		 * YYY: well, I was going to do this, but there is too much
		 *      weird stuff in the openDisc stuff.  For instance, I
		 *      notice the openDisc handling flow control.  Why?
		 * ZZZ: best to leave it alone, for now....
		 */
		if (tp->t_dflt.tiox.x_sflag & DTR_OPEN)
			tp->t_open_disc = DTRO_OPEN;
		else if (tp->t_dflt.tiox.x_sflag & WT_OPEN)
			tp->t_open_disc = WTO_OPEN;

		tp->t_hptr = (void *) sc;

		sc->sf_ethrei= 0;
		sc->sf_open  = 1;

#ifdef PM_SUPPORT
		simple_unlock( &sc->pm_lock );
#endif
		/*
		 * Push default values down into the chip
		 */
		sty_termiox_set(tp, &tp->t_dflt.tiox, 1);
		sty_termios_set(tp, &tp->t_dflt.tios, 1);

		port = io_att(sc);
		x    = disable();
		*FCR(port) = 0;	eieio();
		*FCR(port) = sc->sf_rtrig | FIFO_ENABLE;
		sc->sf_msr = sc->sf_delta = *MSR(port) & 0xf0;
		*IER(port) = ERBDAI|ELSI|EMSSI;
		*MCR(port) = (*MCR(port) & ~LOOP) | OUT2;

		enable(x);
		io_det(port);

		/*
		 * make sure watchdog timer is running
		 */
		x = i_disable(tp->t_hpri);
		if (!puppy) {
			/* static struct trb trb; 	168801 */
			void sf_mastif();

			trb->ipri      = tp->t_hpri;
			trb->flags     = 0;
			trb->func_data = (ulong) 0;
			trb->func      = sf_mastif;

			trb->timeout.it_value.tv_sec  = 4;
			trb->timeout.it_value.tv_nsec = 0;

			tstart(puppy = trb);
		}
		i_enable(x);
	}

	return tp;
}

/*
 * sf_close_port -	close port
 */
sf_close_port(struct sty_tty *tp) {
	register struct sfcom *sc = tp->t_hptr;
	register volatile uchar *port;

	db_trace(TR_TTY, (tp, "sf: close_port"));

#ifdef PM_SUPPORT
	pm_block( sc );
	simple_lock( &sc->pm_lock );
#endif

	port = io_att(sc);
	*IER(port) = 0;			/* disable port	*/
	io_det(port);

	sc->sf_ethrei= 0;
	sc->sf_watch = 0;
	sc->sf_open  = 0;

	i_clear(&sc->sf_slih);

#ifdef PCMCIA
	(void) disable_pccard(sc);
#endif

#ifdef PM_SUPPORT
	(void) sf_power(sc, PM_PLANAR_OFF);
#endif

	assert(!unpincode(sf_slih));
	assert(!unpin(sc, sizeof(*sc)));

#ifdef PM_SUPPORT
	simple_unlock( &sc->pm_lock );
#endif
}

/*
 * sf_ioctl -	handle device specific I/O controls
 */
sf_ioctl(queue_t *q, mblk_t *mp, int fromput) {
	struct iocblk *iocp;
	register volatile uchar *port;
	register struct sty_tty *tp = (struct sty_tty *)q->q_ptr;
	register struct sfcom *sc = tp->t_hptr;
	struct rs_info rs_info, *rinfo;
	register cnt;
	int err = 0, er2;
	int old_intr, signal_temp, temp;
	unsigned char outtype;
	unsigned long outcnt;

	iocp    = (struct iocblk *)mp->b_rptr;
	cnt     = iocp->ioc_count;
	outtype = M_IOCACK;
	outcnt  = 0;
	
	db_trace(TR_TTY, (q, "sf: ioctl cmd=%x %sfromput"
			  , iocp->ioc_cmd
			  , fromput ? "" : "!"));

#ifdef PM_SUPPORT
	pm_block( sc );
#endif

	if (!fromput) {	 /* from service routine */
		switch (iocp->ioc_cmd) {
		    case RS_SETA:
			if (cnt != sizeof (struct rs_info) || !mp->b_cont) {
			    enospc:
				outtype = M_IOCNAK;
				iocp->ioc_error = ENOSPC;
				break;
			}

			rs_info = *(struct rs_info *)mp->b_cont->b_rptr;

			if (rs_info.rs_rtrig > 3 || rs_info.rs_rtrig < 0
			    || rs_info.rs_tbc < 0 || rs_info.rs_tbc > 16) {
			    einval:
				outtype = M_IOCNAK;
				iocp->ioc_error = EINVAL;
				break;
			}

			rs_info.rs_rtrig <<= 6;
			if (rs_info.rs_tbc == 0)
				rs_info.rs_tbc = 16;
			sc->sf_tbc = rs_info.rs_tbc;

			if (rs_info.rs_rtrig == sc->sf_rtrig)
				break;

			port = io_att(sc);
			old_intr = disable_lock(tp->t_hpri, &tp->t_lock);

			*FCR(port) = rs_info.rs_rtrig | FIFO_ENABLE;

			unlock_enable(old_intr, &tp->t_lock);
			io_det(port);

			sc->sf_rtrig = rs_info.rs_rtrig;

			switch (sc->sf_rtrig&TRIG_MASK) {
			    case TRIG_01: sc->sf_rcnt = 1; break;
			    case TRIG_04: sc->sf_rcnt = 4; break;
			    case TRIG_08: sc->sf_rcnt = 8; break;
			    case TRIG_14: sc->sf_rcnt = 14; break;
			}

			db_trace(TR_TTY, (tp, "sf: ioctl: set fifo to %d"
					  , rs_info.rs_rtrig));
			break;

		    default:
			goto einval;
		}
	} else {
		switch (iocp->ioc_cmd) {
		    case TIOCMGET:
			if (cnt != sizeof (int) || !mp->b_cont)
				goto enospc;

			sc   = tp->t_hptr;
			temp = signal_temp = 0;

			sf_service(tp, TS_GCONTROL, &temp);

			if (temp & TSDTR) signal_temp |= TIOCM_DTR;
			if (temp & TSRTS) signal_temp |= TIOCM_RTS;

			if (sc->sf_msr & DCD) signal_temp |= TIOCM_CAR;
			if (sc->sf_msr &  RI) signal_temp |= TIOCM_RNG;
			if (sc->sf_msr & DSR) signal_temp |= TIOCM_DSR;
			if (sc->sf_msr & CTS) signal_temp |= TIOCM_CTS;

			mp->b_cont->b_rptr = mp->b_cont->b_wptr;	/* 166798 */
			*(int *) mp->b_cont->b_wptr = signal_temp;
			outcnt = sizeof(int);
			break;

		    case RS_SETA:
			putq(q,mp);
			return 0;

		    case RS_GETA:
			outcnt = sizeof (*rinfo);
			if (cnt < outcnt || !mp->b_cont)
				goto enospc;

			rinfo = (void *) mp->b_cont->b_wptr;
			mp->b_cont->b_rptr = mp->b_cont->b_wptr;   /* 171864 */
			bzero(rinfo, sizeof (*rinfo));

			rinfo->rs_dma   = 0;
			rinfo->rs_rtrig = sc->sf_rtrig >> 6;
			rinfo->rs_tbc   = sc->sf_tbc;
			rinfo->rs_mil   = 0;
			break;

		    default:
			outtype = M_IOCNAK;
			break;
		}
	}

	mp->b_datap->db_type = outtype;
	iocp->ioc_count = outcnt;
	if (mp->b_cont)
		mp->b_cont->b_wptr += outcnt;
	qreply(q,mp);

	return (outtype == M_IOCACK) ? 0 : -1;
}

/*
 * sf_ctl - Processes the M_CTL messages.
 *
 * Input:
 *      q       - address of STREAMS write queue
 *      mp      - M_CTL to process.
 */
sf_ctl(queue_t *q, mblk_t *mp) {
	register struct sty_tty *tp = (struct sty_tty *)q->q_ptr;
	register struct sfcom *sc = tp->t_hptr;
	struct termios *tios;
	struct iocblk *iocp;
	mblk_t *mp1;

#ifdef PM_SUPPORT
	pm_block( sc );
#endif

	iocp = (struct iocblk *)mp->b_rptr;

	db_trace(TR_TTY, (tp, "sf: ctl mp %x cmd %x", mp, iocp->ioc_cmd));

	switch (iocp->ioc_cmd) {
	    case TIOC_REQUEST:
	    case TIOC_REPLY: {
		register int reply_size = 2 * sizeof(struct tioc_reply);

		iocp->ioc_cmd = TIOC_REPLY;
		if (!(mp1 = allocb(reply_size, BPRI_MED))) {
			/*
			 * just reply with the same message, next RS_SETA
			 * and RS_GETA will arrive transparent and will
			 * fail
			 */
			break;
		}

		iocp->ioc_count = reply_size;
		bcopy(sf_tioc_reply, mp1->b_wptr, reply_size);

		mp1->b_wptr += reply_size;
		mp->b_cont   = mp1;

		break;
	    }
		
	    case TIOCGETMODEM:
		if ((mp1 = mp->b_cont)
		    && (mp1->b_datap->db_lim - mp1->b_wptr >= sizeof(char))) {
			*((char *) mp1->b_wptr) = sc->sf_msr & DCD;
			mp1->b_wptr += sizeof(char);
		} else
			goto freeret;
		break;

	    case TIOCMGET:
		if ((mp1 = mp->b_cont)
		    && (mp1->b_datap->db_lim - mp1->b_wptr >= sizeof(int))) {
			int signal_temp = 0;
			int cntl_tmp = 0;

			sf_service(tp, TS_GCONTROL, &cntl_tmp);

			if (cntl_tmp & TSDTR) signal_temp |= TIOCM_DTR;
			if (cntl_tmp & TSRTS) signal_temp |= TIOCM_RTS;

			if (sc->sf_msr & DCD) signal_temp |= TIOCM_CAR;
			if (sc->sf_msr & RI ) signal_temp |= TIOCM_RNG;
			if (sc->sf_msr & DSR) signal_temp |= TIOCM_DSR;
			if (sc->sf_msr & CTS) signal_temp |= TIOCM_CTS;

			mp1->b_rptr = mp1->b_wptr;		/* 166798 */
			*(int *)mp1->b_wptr = signal_temp;
			mp1->b_wptr += sizeof(int);
		} else
			goto freeret;
		break;

	    case MC_CANONQUERY:
		if ((mp1 = mp->b_cont)
		    && (mp1->b_datap->db_lim - mp1->b_wptr >= sizeof *tios)) {
			tios = (struct termios *)mp1->b_wptr;
			bzero(tios, sizeof *tios);
			tios->c_iflag |= IXON | IXOFF | IXANY;
			iocp->ioc_cmd = MC_PART_CANON;
			mp1->b_wptr += sizeof *tios;
		} else
			goto freeret;
		break;

	    default:
	    freeret:
		freemsg(mp);
		return;
	}

	qreply(q, mp);
}

/*
 * sf_proc
 *      Called by sty_service routine when requested service is
 *      TS_PROC and sub-command is T_SUSPEND, T_RESUME, T_WFLUSH
 *      or T_RFFLUSH.
 *      Called by stywsrv and off-level routine to output data to
 *      uart reggisters, when possible.
 *      In both cases, (*tp->t_proc) is called at tp->t_hpri level with the
 *      per-line lock (tp->t_lock field).
 *
 * Input:
 *      tp      - sty_tty structure pointer.
 *      cmd     - enum proc_commands to be perfomed.
 */
sf_proc(register struct sty_tty *tp, enum proc_commands cmd) {
	register struct sfcom *sc = tp->t_hptr;
	register volatile uchar *port;
	register x, unit, err, cnt, xcnt;
	register char *ptr;

	Enter(TRC_SF(TTY_PROC), tp->t_dev, (int)tp, cmd, 0, 0);

#ifdef PM_SUPPORT
	pm_block( sc );
#endif

	port = io_att(sc);

	ASSERT(sc);

	db_trace(TR_TTY, (tp, "sf: proc %s xcnt %x%s%s"
			  , sty_pcmds(cmd)
			  , sc->sf_xcnt
			  , tp->t_stop ? " stop" : ""
			  , tp->t_busy ? " busy" : ""));

	switch (cmd) {
	    case T_WFLUSH:
		sc->sf_xcnt = 0;
		if (tp->t_outmsg) {
			freemsg(tp->t_outmsg);
			tp->t_outmsg = 0;
		}
		/*
		 * fall thru...
		 */

	    case T_RESUME:
		tp->t_stop = 0;
		qenable(WR(tp->t_queue)); 
		/*
		 * fall thru...
		 */

	    case T_OUTPUT:	   /* start output	*/
	    output:
		if ((tp->t_xon_pending || tp->t_xoff_pending)
		    && !tp->t_busy) {
			if (tp->t_xoff_pending) {
				db_trace(TR_TTY, (tp, "sf:!flow: off"));
				*THR(port) = tp->t_softchars[1];
				tp->t_xoff_pending = 0;
				eieio();
			} else {
				db_trace(TR_TTY, (tp, "sf:!flow: on"));
				*THR(port) = tp->t_softchars[0];
				tp->t_xon_pending = 0;
				eieio();
			}
			tp->t_busy = 1;
			if (!sc->sf_ethrei && tp->t_busy) {
				*IER(port) |= ETHREI;
				sc->sf_ethrei = 1;
			}
		}
		if (tp->t_stop||tp->t_busy)
			break;

		sc->sf_watch = 0;
		tp->t_busy   = (sc->sf_xcnt != 0);

		cnt  = sc->sf_tbc ? sc->sf_tbc : SF_FIFOCNT;
		xcnt = sc->sf_xcnt;
		ptr  = sc->sf_xdata;

		while (cnt > 0) {
			register mblk_t *m;

			if (xcnt) {
				*THR(port) = *ptr++;
				--xcnt;
				--cnt;
				eieio();
				continue;
			}

			/*
			 * see if there is another buffer
			 */
		    retry:
			if (!(m = tp->t_outmsg)) {
				if (m = getq(WR(tp->t_queue))) {
					if (m->b_datap->db_type != M_DATA) {
						putbq(WR(tp->t_queue), m);
						qenable(WR(tp->t_queue));
						break;
					}
				} else
					break;
			}

			if (m->b_rptr >= m->b_wptr) {
				register mblk_t *n;

				n = tp->t_outmsg = m->b_cont;
				m->b_cont = 0;
				freemsg(m);
				assert(!n || n->b_datap->db_type == M_DATA);
				goto retry;
			}

			tp->t_outmsg = m;
			tp->t_busy   = 1;
			sc->sf_xcnt  = xcnt = m->b_wptr - m->b_rptr;
			sc->sf_xdata = ptr  = m->b_rptr;
			m->b_rptr    = m->b_wptr;
		}

		/*
		 * save updated values of output ptr and cnt.
		 */
		sc->sf_xdata = ptr;
		sc->sf_xcnt  = xcnt;

		/*
		 * enable xmit done interrupt
		 */
		if (!sc->sf_ethrei && tp->t_busy) {
			*IER(port) |= ETHREI;
			sc->sf_ethrei = 1;
		}
		break;

	    case T_SUSPEND:	/* suspend output	*/
		tp->t_stop = 1;
		break;

	    case T_BLOCKx:	/* block input, low level	*/
	    case T_BLOCK:	/*  block input		*/
		/*
		 * we must retain the distinction between the two types of
		 * blockage.  "normal" implies that we can assume there will
		 * eventually come a T_UNBLOCK --- we don't have to keep
		 * track of things.  "abnormal" implies we must take
		 * responsibility of making sure we will unblock.
		 */
		if (cmd == T_BLOCK) {
			/*
			 * "normal" blockage.
			 */
			sc->sf_flox = 0;	/* normalize	*/
			tp->t_block = 1;
		} else {
			if (!tp->t_block)
				sc->sf_flox = 1;
		}

		switch (tp->t_flow_disc & flo_imask) {
		    default:
		    case flo_none:
			break;		/* XXX: should be error ?	*/

		    case flo_isw:
			tp->t_xoff_pending = 1;
			goto output;

		    case flo_idtr:
			*MCR(port) &= ~DTR;
			break;

		    case flo_irts:
			*MCR(port) &= ~RTS;
			break;
		}

		break;

	    case T_RFLUSH:	/* flush input side	*/
		tp->t_rptr = tp->t_wptr;	/* flush input buffer	*/
		if (!sc->sf_flox)		/* make sure not floxed	*/
			break;
		/*
		 * fall thru to unflox it...
		 */

	    case T_UNBLOCKx:	/* unflox		*/
		/*
		 * since we "normalize" above, we should never come here
		 * with sf_flox == 1 and t_block == 1, but just in case,
		 * we back-end normalize here as well, by checking for t_block.
		 */
		sc->sf_flox = 0;
		if (tp->t_block)
			break;

	    case T_UNBLOCK:	/* resume input		*/
		switch (tp->t_flow_disc & flo_imask) {
		    default:
		    case flo_none:
			break;		/* XXX: should be error ?	*/

		    case flo_isw:
			tp->t_xoff_pending = 0;
			tp->t_xon_pending  = 1;
			goto output;

		    case flo_idtr:
			*MCR(port) |= DTR;
			break;

		    case flo_irts:
			*MCR(port) |= RTS;
			break;
		}

		sc->sf_flox = tp->t_block = 0;
		break;

	    default:
		break;  
	}

	io_det(port);
	Return(0);
}

/*
 * sf_service
 *      This function pushes values down into and gets them from
 *      the chip 
 *      Is is called by other parts of the driver on some M_IOCTL
 *      or M_CTL, or M_BREAK processing.
 *      It is also called by the the open and flow controle
 *      disciplines.
 *
 * Input:
 *      tp      - sty_tty structure pointer.
 *      cmd     - enum service_commands to be perfomed.
 *      varg    = Command argument or sub-command for TS_S* commands.
 *              = address of a storage to contain requested information for
 *                TS_G* commands.
 * Returns:
 *      0 on success, otherwise error code.
 */
sf_service(struct sty_tty *tp,  enum service_commands cmd, void *varg) {
	register struct sfcom *sc = tp->t_hptr;
	register volatile uchar *port;
	register x, reg, baud_temp;
	int arg = (int) varg;

	Enter(TRC_SF(TTY_SERVICE), tp->t_dev, (int)tp, cmd, varg, 0);

	db_trace(TR_TTY, (tp, "sf: service (%s, %x)", sty_scmds(cmd), varg));

#ifdef PM_SUPPORT
	pm_block( sc );
#endif

	port = io_att(sc);

	switch (cmd) {
	    case TS_SCONTROL:
		reg = *MCR(port);

		if (arg & TSDTR)
			reg |= DTR;
		else
			reg &= ~DTR;

		if (arg & TSRTS)
			reg |= RTS;
		else
			reg &= ~RTS;

		*MCR(port) = reg;
		break;

	    case TS_GCONTROL:
		reg = *MCR(port);

		*(int *)varg = (reg & DTR) ? TSDTR : 0;
		if (reg & RTS)
			*(int *)varg |= TSRTS;
		break;

	    case TS_GSTATUS:
		*(int *)varg = 0;
		if (sc->sf_msr & DCD) *(int *)varg |= TSCD;
		if (sc->sf_msr & RI ) *(int *)varg |= TSRI;
		if (sc->sf_msr & DSR) *(int *)varg |= TSDSR;
		if (sc->sf_msr & CTS) *(int *)varg |= TSCTS;
		break;

	/* Not used internally by the driver but needed by wtopen */
	    case TS_GBAUD:
		*((int *) varg) = tp->t_ospeed;
		break;

	    case TS_SBAUD:
		if (arg) {
			register new;

			new = arg * 2;
			if (arg == 134)
				++new;		/* new == 134.5 * 2 */
			baud_temp = (2 * sc->sf_xtal) / new;
		} else
			baud_temp = 0;

		x = disable_lock(tp->t_hpri, &tp->t_lock);

		if (arg == 0)
			*MCR(port) &= ~(DTR|RTS);
		else {
			set_dlab(port);
			*DLL(port) = baud_temp;	eieio();
			*DLM(port) = (baud_temp >> 8) & 0xff;
			clr_dlab(port);
		}

		unlock_enable(x, &tp->t_lock);

		tp->t_ospeed = tp->t_ispeed = (baud_t)arg;
		break;

	    case TS_SBPC:
		reg = *LCR(port) & ~(WLS1|WLS0);
		switch (arg) {
		    case bits5:
			break;
		    case bits6:
			reg |= WLS0;
			break;
		    case bits7:
			reg |= WLS1;
			break;
		    case bits8:
			reg |= (WLS1|WLS0);
			break;
		}
		*LCR(port) = reg;
		tp->t_csize = (csize_t)arg;
		break;

	    case TS_SPARITY:
		reg = *LCR(port) & ~(STICK|EPS|PEN);
		switch (arg) {
		    case nopar:
			break;
		    case oddpar:
			reg |= PEN;
			break;
		    case markpar:
			reg |= (STICK|PEN);
			break;
		    case evenpar:
			reg |= (EPS|PEN);
			break;
		    case spacepar:
			reg |= (STICK|EPS|PEN);
			break;
		}
		*LCR(port) = reg;
		tp->t_parity = (parity_t)arg;
		break;

	    case TS_SSTOPS:
		reg = *LCR(port);
		if ((stop_t)arg==stop1)
			reg &= ~STB;
		else
			reg |= STB;
		*LCR(port) = reg;
		tp->t_stopbits = (stop_t)arg;
		break;

	    case TS_SBREAK:
		*LCR(port) |= BREAK;
		break;

	    case TS_CBREAK:
		*LCR(port) &= ~BREAK;
		break;

	    case TS_LOOP:
		x = disable_lock(tp->t_hpri, &tp->t_lock);
		if (arg == LOOP_ENTER)
			*MCR(port) |= LOOP;
		else if (arg == LOOP_EXIT)
			*MCR(port) &= ~LOOP;

		unlock_enable(x, &tp->t_lock);
		break;

	    case TS_PROC:
		x = disable_lock(tp->t_hpri, &tp->t_lock);
		(*tp->t_proc)(tp, arg);
		unlock_enable(x, &tp->t_lock);
		break;

	    default:
		break;
	}

	io_det(port);
	Return(0);
}

#undef CTL
#define CTL(c)		('c' - '@')	/* only works with upper case	*/

/*
 * sf_offl	-	off level processing
 *
 * Input:
 *	tp	-	^ to sty_tty struct
 *	write	-	!0 => called from write side, this parameter is ignored here
 *
 * Note:
 *	This function used to be called on an off-level interrupt.
 *	Instead, I run it in the service procs for the q.
 */
sf_offl(struct sty_tty *tp, int write) {
	register struct sfcom *sc = tp->t_hptr;
	register status, x, msg_cnt = 0;
	register uchar data, msr_delta;
	register mblk_t *mp = 0;
	register char *msg_ptr;
	register queue_t *q;
	register cond_bits;
	enum {
		normal	= 0x0001,
		sak	= 0x0002,
		ctlx	= 0x0004,
		xmit	= 0x0008,
		empty	= 0x0010,
		pass_up = 0x0020,
		qfull   = 0x0040,
		flush   = 0x0080,
		busy    = 0x0100,
	};

	q = tp->t_queue;

	/*
	 * XXX: really should precompute (portions of) cond-bits elsewhere
	 */
	cond_bits = (tp->t_sak	? sak   : 0)
		  | (tp->t_cread? 0     : flush)
		  | (tp->t_ctlx	? ctlx  : 0)
		  | (q->q_count	? qfull : 0);

	x = disable();
	if (sc->sf_xmit) {
		cond_bits |= xmit;	/* save xmit flag		*/
		sc->sf_xmit = 0;	/* clear xmit for next time	*/
	}
	enable(x);

	/*
	 * "normal" path thru code is !sak and !flush
	 */
	if (!(cond_bits & (sak | flush)))
		cond_bits |= normal;

	db_trace(TR_TTY
		 , (tp, "sf: offl q %x cond %x", q, cond_bits));

	/*
	 * Note: it is OK to read tp->t_inmsg without holding the lock.
	 *	 if we read it as NULL here, and it gets set in the next
	 *	 instant, we are still guaranteed to come back here due
	 *	 to the queue being enabled, at the cost of slight inefficiency
	 *	 for those 1 in a zillion times it might happen.
	 */
	while (tp->t_inmsg) {
		if (!mp) {
			mp = tp->t_inmsg;
			msg_ptr = mp->b_wptr;
			msg_cnt = 0;
		}

		if (tp->t_modem_chg) {
			x = disable();
			tp->t_modem_chg = 0;
			msr_delta = sc->sf_msr ^ sc->sf_delta;
			status    = 1 | (sc->sf_msr = sc->sf_delta);
			data      = 0;
			enable(x);
		} else if (tp->t_rptr == tp->t_wptr) {
			cond_bits |= empty;
			break;
		} else if (cond_bits & qfull)
			break;
		else {
			status = *tp->t_rptr++;
			data   = status & 0xff;
			status >>= 8;

			ASSERT(!(status & 1));

			if (tp->t_rptr >= &tp->t_rbuf[RBSIZE])
				tp->t_rptr = &tp->t_rbuf[0];
		}

		if (!(status&(BI|FE|PE|OE|1))) { /* good data 99% of the time*/
			if (cond_bits & normal) {
			    normal:
				/*
				 * normal path thru code.  keep upfront to
				 * minimise icache footprint
				 */
				*msg_ptr++ = data;
				++msg_cnt;
				if (msg_cnt < STY_RDBUFSZ)
					continue;

				/*
				 * message full:
				 *	enqueue it and allocate another
				 */
				db_trace(TR_TTY, (tp, "sf: sendup 0 %x", mp));

				mp->b_wptr = msg_ptr;
				mp->b_datap->db_type = M_DATA;
				if (canput(q->q_next))
					putnext(q,mp);
				else {
					putq(q, mp);
					cond_bits |= qfull;
				}
				mp = 0;
				sty_allocmsg(tp, 0);
				continue;
			} else if (cond_bits & flush)
				continue;
			else {
				ASSERT(tp->t_sak);

				if ((cond_bits & ctlx) && data == CTL(R)) {
					mp->b_datap->db_type = M_PCSIG;
					*mp->b_wptr++ = SIGSAK;

					db_trace(TR_TTY
						 , (tp, "sf: sendup 1 %x", mp));

					/*
					 * M_PCSIG is a high priority message
					 */
					putnext(q, mp);

					mp = 0;
					sty_allocmsg(tp, 0);
					cond_bits ^= ctlx;
					continue;
				}
				if (cond_bits & ctlx) {
					*msg_ptr++ = CTL(X);
					++msg_cnt;
					/*
					 * XXX: check for msg full
					 */
				}
				if (data == CTL(X)) {
					cond_bits |= ctlx;
					continue;
				}

				/*
				 * not a control-X, pretend to be normal
				 */
				cond_bits &= ~ctlx;
				goto normal;
			}
			continue;
		}
		/*
		 * send up already collected data before passing up CTL msgs.
		 */
		if (msg_cnt) {
			mp->b_datap->db_type = M_DATA;
			mp->b_wptr += msg_cnt;

			db_trace(TR_TTY, (tp, "sf: sendup 2 %x", mp));

			if (canput(q->q_next))
				putnext(q,mp);
			else
				putq(q, mp);

			msg_cnt = 0;
			sty_allocmsg(tp, 0);

			if (!(mp = tp->t_inmsg))
				break;		/* XXX: this loses!	*/
			msg_ptr = mp->b_wptr;
		}

		while (status & 0x01) {	/* modem status change */
			register enum status status_delta;

			/*
			 * XXX: this is a mess.  modem status changes
			 * should be redone in toto.  Also, the bizarre
			 * openDisc stuff should be trashed.
			 */

			cond_bits |= pass_up;

			if (msr_delta&CTS) {
				msr_delta ^= CTS;
				status_delta = status&CTS?cts_on:cts_off;
			} else if (msr_delta & DSR) {
				msr_delta ^= DSR;
				status_delta = status&DSR? dsr_on: dsr_off;
			} else if (msr_delta&RI) {
				msr_delta ^= RI;
				status_delta = status&RI? ri_on: ri_off;
			} else if (msr_delta&DCD) {
				msr_delta ^= DCD;
				status_delta = status&DCD? cd_on: cd_off;
			} else {
				status = 0;
				break;
			}

			if (tp->t_openRetrieve) {
				if (openDisc_input(tp->t_openRetrieve
						   , &tp->t_event, 0
						   , status_delta)) {
					/*
					 * XXX: this is part of the openDisc
					 * gorp that should be made to go away
					 */
					ASSERT(0);
				}
			} else {
				/*
				 * if no openRetrieve gorp, we always
				 * pass the indication up.
				 */
				cond_bits |= pass_up;
			}

			/*
			 * DCD can be used for flow control.
			 * If it is being used as such, we will not
			 * pass it up.
			 */
			if ((tp->t_flow_disc & flo_odcd)
			    && (status_delta==cd_on || status_delta==cd_off))
				continue;

			if (status_delta == cd_off && tp->t_draining) {
				/*
				 * CD loss while draining
				 */
				db_trace(TR_TTY, (tp, "sf:*HANGUP: drain-o"));

				flushq(WR(tp->t_queue), FLUSHDATA);
				sf_proc(tp, T_WFLUSH);

				TTY_WAKE(tp);
			}

			if (cond_bits & pass_up) {
				mp->b_datap->db_type = M_CTL;
				*(enum status *)mp->b_wptr = status_delta;
				mp->b_wptr += sizeof (enum status);

				db_trace(TR_TTY, (tp, "sf: sendup 3 %x", mp));

				if (canput(q->q_next))
					putnext(q,mp);
				else {
					putq(q, mp);
					cond_bits |= qfull;
				}

				msg_cnt = 0;
				sty_allocmsg(tp, 0);
				if (!(mp = tp->t_inmsg))
					break; /* XXX */
				msg_ptr = mp->b_wptr;
			}
		}
		if (!mp)
			break;	/* XXX	*/

		if (status) {	/* Some trouble on the line: erronous data. */
			register enum status status_delta;

			mp->b_datap->db_type = M_BREAK;
			if (status & BI) {
				db_trace(TR_TTY, (tp, "sf:*BREAK"));
				status_delta = break_interrupt;
			} else if (status & FE) {
				db_trace(TR_TTY, (tp, "sf:*FRAMING ERROR"));
				status_delta = framing_error;
			} else if (status & PE) {
				db_trace(TR_TTY, (tp, "sf:*PARITY ERROR"));
				status_delta = parity_error;
			} else {
				db_trace(TR_TTY, (tp, "sf:*OVERRUN"));
				status_delta = overrun;
			}
			*(enum status *)mp->b_wptr = status_delta;
			mp->b_wptr += sizeof (enum status);
			*mp->b_wptr++ = data;

			db_trace(TR_TTY, (tp, "sf: sendup 4 %x", mp));

			if (canput(q->q_next))
				putnext(q,mp);
			else {
				putq(q, mp);
				cond_bits |= qfull;
			}

			msg_cnt = 0;
			sty_allocmsg(tp, 0);

			if (!(mp = tp->t_inmsg))
				break;	/* XXX */

			msg_ptr = mp->b_wptr;
		}
	}

	if (cond_bits & empty) {
		/*
		 * we exited loop because slih fubber empty.
		 */
		if (mp && (msg_cnt)) {   /* Message not full and not empty */
			mp->b_wptr = msg_ptr;

			db_trace(TR_TTY, (tp, "sf: sendup 5 %x", mp));

			if (canput(q->q_next))
				putnext(q,mp);
			else
				putq(q, mp);

			sty_allocmsg(tp, 0);
		}
	} else if (cond_bits & qfull) {
		/*
		 * streams flow control will enable the rsrv eventually...
		 */
		;
	} else {
		/*
		 * we exited the loop because of lack of messages
		 * thus, we are assured that when sty_recover() is called,
		 * we will run the sty_rsrv again, and call sf_offl.
		 */
		ASSERT(!tp->t_inmsg);
	}
    
	if (sc->sf_flox) {
		register depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE-1);
		/*
		 * we blocked input, and we need to take responsibility
		 * for unblocking.
		 */

		db_trace(TR_TTY, (tp, "sf: floxed, depth %x", depth));

		if (depth <= (2*RBSIZE)/8) {
			db_trace(TR_TTY, (tp, "sf:!floxon"));

			x = disable_lock(tp->t_hpri, &tp->t_lock);
			sf_proc(tp, T_UNBLOCKx);
			unlock_enable(x, &tp->t_lock);
		}
	}

	if (cond_bits & xmit) {
		/*
		 * fake xmit done handled here.
		 */
		x = disable_lock(tp->t_hpri, &tp->t_lock);
		sf_proc(tp, T_OUTPUT);
		if (tp->t_busy)
			cond_bits |= busy;
		unlock_enable(x, &tp->t_lock);

		/*
		 * Wakeup the sleeping process at close time,
		 * for draining
		 */
		if (tp->t_draining && !(cond_bits & busy) && !WR(q)->q_first) {
			db_trace(TR_TTY, (tp, "sf: offl drain-o"));
			TTY_WAKE(tp);
		}
        }

	tp->t_ctlx = (cond_bits & ctlx) ? 1 : 0;

	return 0;
}

/*
 * sf_slih	-	sandalfoot native serial I/O slih
 */
sf_slih(struct intr *rintr) {
    register struct sfcom *sc;
    register depth, count, iir;
    register ulong cond_bits;
    register ushort data;
    register struct queue *q;
    register volatile uchar *port;
#ifdef PCMCIA
    register rc = INTR_FAIL;
    int intr_cnt = 0, rcv_cnt;
#endif

    /*
     * the following makes parsimonious use of registers.
     * Prior to organizing the code in this manner, there was a lot of
     * registers dedicated to wasteful things like one bit flags or 8-bit
     * characters.
     * This caused register spillage, resulting in load delays.  The fast
     * shift-and-and instructions on the rs6k should make better use of
     * these precious resources.
     *
     * I also OR-in the LSR status into the "data" register instead of keeping
     * an 8-bit LSR status and an 8-bit "data".  This means that some
     * comparisons are encumbered by field extractions, but this is where the
     * optimiser is supposed to win for us.
     */
    enum bits {
	    calculated	= 0x00010000,	/* cond_bits calculated		*/
	    normal	= 0x00020000,	/* normal, fast path thru code	*/
	    dbg		= 0x00040000,	/* look for debugger entry	*/
	    swflow	= 0x00080000,	/* use s/w flow control		*/
	    overflow    = 0x00100000,	/* input buffer overflow	*/
	    blocked	= 0x00200000,	/* attempted to block input	*/
	    rcv_int	= 0x00400000,	/* interrupt type		*/
	    xmt_int	= 0x00800000,	/* interrupt type		*/
	    mdm_int	= 0x01000000,	/* interrupt type		*/
	    flush       = 0x02000000,	/* flush input			*/
	    vdis_xon	= 0x04000000,	/* START == _POSIX_VDISABLE	*/
	    vdis_xof	= 0x08000000,	/* STOP  == _POSIX_VDISABLE	*/
	    ixany	= 0x10000000,
	    xon_mask	= 0x040000ff,	/* note: vdis_xon test as well	*/
	    xof_mask	= 0x0800ff00,	/* note: vdis_xof test as well	*/
	    xon_shift	= (0*8),
	    xof_shift	= (1*8),
    };
#define xon_test(c)	((((c) & 0xff) << xon_shift) == (cond_bits & xon_mask))
#define xof_test(c)	((((c) & 0xff) << xof_shift) == (cond_bits & xof_mask))
#define tp		(&sc->sf_tty)

    Enter(TRC_SF(TTY_SLIH), 0, (int)rintr, 0, 0, 0);

    sc   = (void*) rintr;
    port = io_att(sc);

    /*     166800 
    if (!tp->t_queue)			/ * protect against activity	
	    goto out;			/ *  while not fully open	*/

    cond_bits = 0;
 
    db_trace(TR_TTY, (&sc->sf_tty, "sf:>slih"));

#ifdef PM_SUPPORT
    sc->sf_status.intr = 1;
    sc->pm_handle.activity = 1;
#endif
#ifdef PCMCIA
    if (sc->pcmcia.no_card == 1){
	goto out;
    }
#endif

    while (!((iir = *IIR(port))&1)) {
#ifdef PCMCIA
	    rc = INTR_SUCC;
	    if( ++intr_cnt > 1024 ) break;
#endif
	    db_trace(TR_TTY, (tp, "sf: slih iir %x(%s)%s"
			      , iir, i_show[(iir&0x0e)/2]
			      , tp->t_busy ? " busy" : ""));

	    switch (iir & 0x0e) {
		case 2:			/* xmit intr			*/
		    if (!tp->t_queue) goto out;		/* 166800 */
		    cond_bits |= xmt_int;
		    continue;

		case 6:			/* lsr bad news			*/
		    if (!tp->t_queue) goto out;		/* 166800 */
		    cond_bits |= rcv_int;
		    count = 0;
		    goto slurp_em;

		case 4:			/* data only -- fifo full	*/
		    if (!tp->t_queue) goto out;		/* 166800 */
		    cond_bits |= rcv_int;
		    count = sc->sf_rcnt;

		slurp_em:
		    if (!(cond_bits & calculated)) {
			    cond_bits |= calculated;
			    /*
			     * calculate loop invariant.  Hope the compiler
			     * sees the big hint here!
			     */
			    if (sc->sf_dbg && dbg_pinned && d_offset)
				    cond_bits |= dbg;
			    if (tp->t_flow_disc & flo_osw) {
				    cond_bits |= swflow;
				    /*
				     * XXX: this should be precomputed for us
				     */
				    if (tp->t_softchars[0] == _POSIX_VDISABLE)
					cond_bits |= vdis_xon;
				    if (tp->t_softchars[1] == _POSIX_VDISABLE)
					cond_bits |= vdis_xof;

				    cond_bits |= tp->t_softchars[0]<<xon_shift;
				    cond_bits |= tp->t_softchars[1]<<xof_shift;

				    if (tp->t_softpacing & IXANY)
					cond_bits |= ixany;
			    }

			    if (!tp->t_cread)
				    cond_bits |= flush;
			    else if (!(cond_bits & (dbg|swflow|flush)))
				    cond_bits |= normal;
			    /*
			     * current depth of receive buffer
			     */
			    depth = (tp->t_wptr - tp->t_rptr) & (RBSIZE-1);
		    }

		    /*
		     * while "count" positive, we don't need to read the LSR.
		     * once "count" exhausted, we read LSR for each subsequent
		     * byte.
		     */
#ifdef PCMCIA
		    rcv_cnt = 0;
#endif
		    while ((data = ((--count >= 0) ? DR : *LSR(port))) & DR) {
#ifdef PCMCIA
		      if( count < 0 && ++rcv_cnt > 1024 ) break;
#endif
			data  = (data & LSR_MASK) << 8;
			data |= *RBR(port) & 0xff;

			if (cond_bits & normal) {
			    normal:
				*tp->t_wptr = data;

				if (depth++ < (3*RBSIZE/4)) {
				    incr_continue:
					if(tp->t_wptr >= &tp->t_rbuf[RBSIZE-1])
						tp->t_wptr = &tp->t_rbuf[0];
					else
						++tp->t_wptr;

					continue;
				}

				/*
				 * buffer filling up.
				 * use flow control, if available.
				 */
				if (!(cond_bits & blocked)) {
					db_trace(TR_TTY
						 , (tp
						    , "sf:!floxoff depth %x"
						    , depth));

					sf_proc(tp, T_BLOCKx);
					cond_bits |= blocked;
				}

				/*
				 * make sure we are not about to overflow
				 * the buffer.
				 */
				if (depth < RBSIZE-1)
					goto incr_continue;

				/*
				 * overflowed receive buffer.
				 */
				cond_bits |= overflow;
				--depth;
				continue;
			}
			if (cond_bits & swflow) {
				/*
				 * handle s/w flow control
				 */
				if (xof_test(data)) {
					db_trace(TR_TTY
						 , (tp, "sf:!recv xoff"));
					/*
					 * stop output.
					 */
					tp->t_stop = 1;
					continue;		/* discard */
				} else if (((cond_bits & ixany) && tp->t_stop)
					   || xon_test(data)) {
					/*
					 * start output:
					 *	fake xmit done
					 */
					db_trace(TR_TTY
						 , (tp, "sf:!recv xon: %x"
						    , data & 0xff));

					tp->t_stop = 0;
					if (!tp->t_busy)
						cond_bits |= xmt_int;

					if (xon_test(data))
						continue;	/* discard */
				}
			}

#ifndef PCMCIA
			if ((cond_bits & dbg) && ((0xff & data) == '\\'-'@')) {
				/*
				 * we call brkpoint() rather than debugger().
				 * this is defined as in-line code, so that
				 * the optimiser will know that no registers
				 * get clobbered.
				 */
				register int intlev;
				intlev = i_disable(INTMAX);
				debugger(0, DBG_KBD, 0);
				i_enable(intlev);
				continue;
			} else
#endif
				if (cond_bits & flush)
				continue;
			else
				goto normal;
		    }
		    break;

		case 12:		/* data only */
		    if (!tp->t_queue) goto out;		/* 166800 */
		    cond_bits |= rcv_int;
		    count = 1;		/* ok first time through */
		    goto slurp_em;

		case 0:			/* modem status */
		    cond_bits |= mdm_int;
		    /*
		     * I make the assumption here that modem status changes
		     * are out-of-band messages, and that all I have to do is
		     * record the fact that a change was noticed, and either
		     * save the current MSR, or read the MSR when processing
		     * the indication.
		     * However, we do process any h/w flow control now.
		     */
		    data = (sc->sf_delta = *MSR(port)) ^ sc->sf_msr;

		    db_trace(TR_TTY, (tp, "sf: modem change %x -> %x"
				      , sc->sf_msr, sc->sf_msr ^ data));

		    if (!(cond_bits & swflow)) {
			    register bit;

			    /*
			     * see if any output flow control lines
			     * have wiggled.
			     */
			    switch (tp->t_flow_disc & flo_omask) {
				case flo_octs:
				    bit = CTS;	break;

				case flo_odcd:
				    bit = DCD;	break;

				case flo_odsr:
				    bit = DSR;	break;

				default:
				    bit = 0;	break;
			    }

			    if (data & bit) {
				    /*
				     * `bit' changed, and `bit' is being used
				     * to flow control our output.
				     */
				    db_trace(TR_TTY
					     , (tp, "sf:! oflo %sstop->%sstop"
						, tp->t_stop ? "" : "!"
						, (sc->sf_msr&bit) ? "":"!"));

				    tp->t_stop = (sc->sf_msr & bit) ? 1 : 0;

				    if (!tp->t_stop && !tp->t_busy)
					    cond_bits |= xmt_int;
			    }
		    }

		    tp->t_modem_chg = 1;
		    break;		/* data is don't care */
	    }

	    if (0 && (cond_bits & overflow)) {
		    /*
		     * we no longer do this.  I need to determine if I need
		     * to reinstate this.
		     * So far, it has not been needed.
		     */
		    *IER(port) &= ~(ERBDAI|ELSI|EMSSI);
		    sc->sf_ioff = 1;
		    break;
	    }
    }

    if (cond_bits & overflow)
	    sty_err(tp->t_name,ERRID_TTY_BADINPUT, -1);

    if (!tp->t_queue) goto out;		/* 166800 */
    q = tp->t_queue;

    /*
     * run service procs as necessary
     */
    if ((cond_bits & (rcv_int | mdm_int)) && !(QENAB & q->q_flag))
	    qenable(q);
    if (cond_bits & xmt_int) {
	    tp->t_busy = 0;
	    sf_proc(&sc->sf_tty, T_OUTPUT);

	    if (!tp->t_busy && tp->t_draining) {
		    /*
		     * why bother?
		    *IER(port) &= ~ETHREI;
		     */
		    sc->sf_xmit = 1;
	    }
	    if (sc->sf_xmit) {
		    /*
		     * sf_xmit is now used to indicate that we need to enable
		     * the write-side.
		     */
		    tp->t_wsrv = sf_offl;
		    if (!(QENAB & WR(q)->q_flag))
			    qenable(WR(q));
	    }
    }
    if (cond_bits & rcv_int)
	    ++sysinfo.rcvint;
    if (cond_bits & xmt_int)
	    ++sysinfo.xmtint;
    if (cond_bits & mdm_int)
	    ++sysinfo.mdmint;

 out:
    io_det(port);

#ifdef PM_SUPPORT
    sc->sf_status.intr = 0;
#endif

#ifdef PCMCIA
    Return( rc );
#endif
    Return(INTR_SUCC);

#undef xon_test
#undef xof_test
#undef tp
}

/*
 * sf_mastif -	a likely watchdog
 */
void
sf_mastif() {
	register struct sfcom *sc;
	register struct sty_tty *tp;
	register unit, open = 0;

	for (tp = &(sc = &sfcom[unit = 0])->sf_tty; unit < NSFCOM
	     ; ++unit, tp = &(++sc)->sf_tty) {
		if (!sc->sf_open)		/* skip closed ports	*/
			continue;

		++open;

		if (!tp->t_busy)		/* skip idle ports	*/
			continue;

		if (!sc->sf_watch) {		/* not seen before?	*/
			sc->sf_watch = 1;	/* bite 'im!!		*/

			db_trace(TR_TTY, (tp, "sf:!watch"));
			continue;
		}

		/*
		 * if we make it to here:
		 *	sc->sf_open && tp->t_busy && sc->sf_watch
		 *	must be hosed
		 */
		db_trace(TR_TTY, (tp, "sf:*watch: wedged"));
		tp->t_busy  = 0;		/* force unbusy		*/
		sc->sf_xmit = 1;		/* fake xmit done	*/
		tp->t_wsrv  = sf_offl;		/* run "off level"	*/
		qenable(WR(tp->t_queue));	/*   from wsrv		*/
	}

	if (open) {
		/*
		 * at least one port still open.  resched
		 */
		puppy->timeout.it_value.tv_sec  = 4;
		puppy->timeout.it_value.tv_nsec = 0;

		tstart(puppy);
	} else {
		db_trace(TR_TTY, (tp, "sf: watch: dead doggy"));

		puppy = 0;
	}
}

void sf_print();

/*
 * registration ticket for "sty"
 */
static struct ttydevs sf_ttydev = {
	0,		/* link			*/
#ifdef PCMCIA
	"pc",		/* adapter name		*/
#else
	"sf",		/* adapter name		*/
#endif
	sf_open_port,	/* open			*/
	sf_print,	/* debug print		*/
	0,		/* dev: filled in	*/
	0,		/* busy			*/
	0,		/* open count		*/
};

/*
 * sfc_open -	let controller know port is to be active
 *
 * Input:
 *	dev	-	maj/min of device using this controller
 *
 * Returns:
 *	 0	-	 ok: controller marked configured
 *	!0	-	!ok: errno, port configuration unchanged.
 *
 * Description:
 *	Each port on the controller must be "sfc_open"ed in order to
 *	mark the port as configured.
 */
sfc_open(dev_t dev) {
	register struct sty_tty *tp;
	register struct sfcom *sc;
	register unit, rc = 0;

	sc  = &sfcom[unit = minor(dev)];
	dev = makedev(major(dev), 0);

	/*
	 * at most one controller is supported today
	 */
#ifndef PCMCIA
	if (sf_ttydev.busy && sf_ttydev.dev != dev)
		return EINVAL;
#endif
	if ((ulong) unit >= NSFCOM)
		return ENXIO;

	if (sc->sf_uconf)
		return EBUSY;

	tp = &sc->sf_tty;

	tp->t_dev     = makedev(major(dev), unit);	/* ttyDBG	*/
	tp->t_close   = sf_close_port;
	tp->t_ioctl   = sf_ioctl;
	tp->t_ctl     = sf_ctl;
	tp->t_service = sf_service;
	tp->t_proc    = sf_proc;
	tp->t_rsrv    = sf_offl;

	simple_lock_init(&tp->t_lock);

#ifdef PM_SUPPORT
	simple_lock_init(&sc->sf_status.f_lock);
	simple_lock_init(&sc->pm_lock);
#endif
	tp->t_event = EVENT_NULL;

	if (!sf_ttydev.busy) {
		/*
		 * register this serial device with the streams TTY gorp layer
		 */
		sf_ttydev.dev = dev;

		rc = sty_register(&sf_ttydev);
	}

	if (ttyDBG)
		lldb_register(1, sc);

	if (!rc)
		sc->sf_uconf = 1;	/* fully configured	*/

	return rc;
}

#ifndef PCMCIA
/*
 * sfc_close -	let controller know port is to be inactivated
 *
 * Input:
 *	dev	-	major/minor device to use for this device.
 *
 * Returns:
 *	 0	-	 ok: port marked as unconfigured
 *	!0	-	!ok: errno.  port, if valid and configured, remains so
 *
 * Note:
 *	This is called for each port on the controller in response to
 *	CFG_TERM requests.  If we end up with no active (configured)
 *	ports, we will unregister with "sty".
 *
 *  168043 - changed to allow the major number to be unregistered
 *           from the dev switch table.
 */
sfc_close(dev_t dev) {
	register struct sfcom *sc;
	register unit, adap, rc = 0;

	unit  = minor(dev) & 0x7fff;
	adap  = minor(dev) & 0x8000;

	sc  = &sfcom[unit];
	dev = makedev(major(dev), 0);

	if (sc->sf_open || (adap && sc->sf_uconf)){
		return EBUSY;}
#ifdef PM_SUPPORT
	if ( sc->sf_status.freeze )
		return EBUSY;
#endif 

	if (adap) {
		/*
		 * the adapter is being closed.  In this driver, there are
		 * two adapters, sa0 and sa1 in ODMese.  Thus, as long as
	         * sc->sf_uconf == 0, we known all units for the "adapter"
		 * are closed, so its ok to close the adapter.
		 */
         	 sfcom[unit].sf_cconf = 0;

#ifdef PM_SUPPORT
		rc = pm_register_handle( &(sc->pm_handle), PM_UNREGISTER );
		if ( rc != PM_SUCCESS ){
			return EBUSY;
		}
#endif
	}
	else{

		if (ttyDBG)
			lldb_register(0, sc);
		
		if (puppy) {
			tstop(puppy);
			puppy = 0;
			if (trb) tfree(trb);	/* 168801 */
			trb = NULL;		/* 168801 */
		}

		sc->sf_uconf = 0;	/* needs to be re-configured	*/

		lock_free(&sc->sf_tty.t_lock);
#ifdef PM_SUPPORT
		lock_free(&sc->sf_status.f_lock);
		lock_free(&sc->pm_lock);
#endif
		/*
	 	* see if we still have units configured.  
	 	*/
		for (unit = 0; unit < NSFCOM; ++unit)
			if (sfcom[unit].sf_uconf)
				return 0;

		if ((rc = sty_unregister(&sf_ttydev)) == 0) 

		return rc;

	}

	return 0;
}

#else /* PCMCIA */
/*
 * sfc_close - let PCMCIA Fax/Modem card know port is to be inactivated
 *
 * Input:
 *	dev	-	major/minor device to use for this device.
 *
 * Returns:
 *	 0	-	 ok: port marked as unconfigured
 *	!0	-	!ok: errno.  port, if valid and configured, remains so
 *
 * Note:
 *	This is called for each port on the controller in response to
 *	CFG_TERM requests.  If we end up with no active (configured)
 *	ports, we will unregister with "sty".
 */
sfc_close(dev_t dev){
	register struct sfcom *sc;
	register unit, adap, rc = 0;

	unit  = minor(dev);
	sc  = &sfcom[unit];

	if ((unsigned) unit >= NSFCOM )
		return ENXIO;
	if ( sc->sf_open )
		return EBUSY;
#ifdef PM_SUPPORT
	if ( sc->sf_status.freeze )
		return EBUSY;
#endif 

	/* call CardServices function for PCMCIA relese */
	if ( sc->sf_uconf == 0 )
	{
		int zero = 0;

		for(;;){
			compare_and_swap(&sc->pcmcia.cb_flag, &zero, -1);
			if (sc->pcmcia.cb_flag == -1){
				break;
			}
			delay(1);
		}

		if (CardRemoval(sc)){
			return EBUSY;
		}

		if (DeregisterClient(sc)){
			return EBUSY;
		}

#ifdef PM_SUPPORT
		rc = pm_register_handle( &(sc->pm_handle), PM_UNREGISTER );
		if ( rc != PM_SUCCESS ){
			return EBUSY;
		}

		lock_free(&sc->sf_status.f_lock);
		lock_free(&sc->pm_lock);
#endif
		lock_free(&sc->sf_tty.t_lock);

		sc->sf_cconf = 0;
	}
	else
	{
		sc->sf_uconf = 0;	/* needs to be re-configured	*/

		for (unit = 0; unit < NSFCOM; ++unit){
	    		if (sfcom[unit].sf_uconf){
				return 0;
			}
		}

		if (ttyDBG)
			lldb_register(0, sc);

		if (puppy) {
			tstop(puppy);
			puppy = 0;
		}
		if (trb){
			tfree(trb);		/* 168801 */
			trb = NULL;		/* 168801 */
		}

		rc = sty_unregister(&sf_ttydev);
		if ( rc == 0) {
			sf_ttydev.busy = 0;
		}

	}

	return rc;
}
#endif /* #ifndef PCMCIA */

#if ttyDBG
/*
 * lldb_register -	reg/dereg with LLDB
 *
 * Input:
 *	add	-	flag: !0 ==> add, otherwise, delete
 *	sc	-	^ to device
 */
static
lldb_register(add, sc)
register struct sfcom *sc; {
	struct tty_to_reg regis;

	regis.dev = sc->sf_tty.t_dev;
	bcopy(sc->sf_tty.t_name, regis.ttyname, sizeof (regis.ttyname));
#ifdef PCMCIA
	bcopy("pc", regis.name, sizeof (regis.name));
#else
	bcopy("sf", regis.name, sizeof (regis.name));
#endif
	regis.private_data = sc;

	if (add)
		tty_db_open(&regis);
	else
		tty_db_close(&regis);
}
#endif	/* ttyDBG	*/
