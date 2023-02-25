static char sccsid[] = "@(#)27        1.11  src/tcpip/usr/bin/tftp/tftp.c, tcpfilexfr, tcpip411, GOLD410 3/8/94 18:02:53";
/* 
 * COMPONENT_NAME: TCPIP tftp.c
 * 
 * FUNCTIONS: MSGSTR, err_load, makerequest, nak, printstats, 
 *            recvfile, sendfile, startclock, stopclock, timer, 
 *            tpacket 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
static char sccsid[] = "tftp.c	5.5 (Berkeley) 2/7/86";
#endif not lint
*/
/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Protocol Machines
 */

/*
 * The following changes have been made according to RFC 1123:
 *
 * Packets that have a block number for a previous transmission
 * are assumed to have been delayed and are silently ignored.
 * This solves the Sorcerer's Apprentice Syndrome which resulted
 * from transmitting a duplicate packet when a duplicate ACK
 * was received.
 *
 * Adaptive timeout has been implemented with a technique similar
 * to that used by TCP.  When a packet that was not retransmitted
 * is acknowledged, the round trip time is computed and used to 
 * update a smoothed round trip time (srtt = rtt/8 + srtt * 7/8) 
 * and a smoothed round trip time difference (srttd = srttd * 3/4 + 
 * |delta|/4, where delta = rtt - srtt).  When a packet that was
 * retransmitted is acknowledged, the smoothed times are not
 * updated because it is not known which packet is being 
 * acknowledged and thus the round trip time is uncertain.
 * When a packet is transmitted for the first time, the timeout
 * is calculated from the smoothed round trip time and the smoothed
 * round trip time difference (timeout = srtt + 2 * srttd).  When a
 * packet is retransmitted, the timeout is doubled (limited by the
 * maximum timeout).  When a retransmitted packet is acknowledged,
 * the round trip time is used as the best guess for the timeout
 * for the next packet.
 */

#include <sys/types.h>
#include <sys/socket.h>
#ifndef aiws
#include <sys/time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <termio.h>
#include <errno.h>
#include <setjmp.h>

#include <nl_types.h>
#include "tftp_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTP,n,s) 


extern	int errno;

extern  struct sockaddr_in sin;         /* filled in by main */
extern  int     f;                      /* the opened socket */
extern  int     trace;
extern  int     verbose;
extern  int     rexmtval;
#define TFTP_MIN_TIMEOUT 1
extern  int     maxtimeout;
extern 	int	file_stdin;
extern 	struct	termio	deftio;

#define PKTSIZE    SEGSIZE+4
char    ackbuf[PKTSIZE];
int	timeout;
jmp_buf	toplevel;
jmp_buf	timeoutbuf;
int	eof_seen;

#ifdef FAULT_INSERTION
#include <stdlib.h>
int debug_delay;
#endif /* FAULT_INSERTION */

/*
 * Packet control block
 */
static struct packet_cb {
        short   pk_rtt;                 /* round trip time */
        short   pk_srtt;                /* smoothed round-trip time */
                                        /* stored fixed pt. 13.3 */
        short   pk_srttd;               /* difference in round-trip time */
                                        /* stored fixed pt. 14.2 */
        struct timeval pk_xtstart;      /* transmit start time */
        struct timeval pk_xtend;        /* transmit end time */
        short   pk_rxtcnt;              /* retransmit count */
        short   pk_rxtcur;              /* current retransmit timeout value */
        short   pk_rxtnxt;              /* next retransmit timeout value */
} pkcb;
struct timezone zone;

/*
 * Force a time value to be in a certain range.
 */
#define RANGESET(tv, value, tvmin, tvmax) { \
        (tv) = (value); \
        if ((tv) < (tvmin)) \
                (tv) = (tvmin); \
        else if ((tv) > (tvmax)) \
                (tv) = (tvmax); \
}

timer()
{

	signal(SIGALRM, timer);
        pkcb.pk_rxtcnt++;
	timeout += rexmtval;
	if (timeout >= maxtimeout) {
                if (file_stdin)
                        ioctl(0, TCSETAF, &deftio);
		printf(MSGSTR(TRANS_TIMED_OUT, "Transfer timed out.\n")); /*MSG*/
		longjmp(toplevel, -1);
	}
	longjmp(timeoutbuf, 1);
}

/*
 * Set packet start time.
 */
start_pk_time()
{
        /*
         * If this packet has timed out and is being retransmitted,
         * then backoff to a longer retransmit timeout value.
         */
        if (pkcb.pk_rxtcnt > 0) {
                RANGESET(pkcb.pk_rxtcur, pkcb.pk_rxtcur * 2,
                        TFTP_MIN_TIMEOUT, maxtimeout);
                rexmtval = pkcb.pk_rxtcur;
#ifdef TFTP_DEBUG
		if (trace)
        		printf("   backoff rexmtval to:  %d\n", rexmtval);
#endif /* TFTP_DEBUG */
                return;
        }
        /*
         * Otherwise, this packet is being transmitted for the first time.
         */

        /*
         * Set the start time for this packet.
         */
        gettimeofday(&pkcb.pk_xtstart, &zone);

        /*
         * If the previous packet was retransmitted, its final retransmit
         * timeout value is more appropriate to use as a starting point
         * for this packet.
         */
        if (pkcb.pk_rxtnxt > 0) {
                rexmtval = pkcb.pk_rxtcur = pkcb.pk_rxtnxt;
                pkcb.pk_rxtnxt = 0;
#ifdef TFTP_DEBUG
		if (trace)
       	 	printf("   using prev rexmtval:  %d\n", rexmtval);
#endif /* TFTP_DEBUG */
                return;
        }

        /*
         * Otherwise, determine the retransmission timeout value =
         * srtt + 2 * srttd.
         */
        RANGESET(pkcb.pk_rxtcur, ((pkcb.pk_srtt >> 2) + pkcb.pk_srttd) >> 1,
                TFTP_MIN_TIMEOUT, maxtimeout);
        rexmtval = pkcb.pk_rxtcur;
#ifdef TFTP_DEBUG
	if (trace)
	        printf("    calculate rexmtval:  %d\n", rexmtval);
#endif /* TFTP_DEBUG */
}

/*
 * Set packet end time.
 */
end_pk_time()
{
        register short delta;

        /*
         * If the first transmission of the packet has been acknowledged,
         * then determine the smoothed round-trip time and the
         * smoothed round-trip time difference.
         *      srtt = rtt/8 + srtt * 7/8
         *      srttd = srttd * 3/4 + |delta|/4
         *              where delta = rtt - srtt
         */
        if (pkcb.pk_rxtcnt == 0) {
                /*
                 * Round down the round trip time to the nearest second.
                 */
                gettimeofday(&pkcb.pk_xtend, &zone);
                pkcb.pk_rtt = pkcb.pk_xtend.tv_sec -
                                pkcb.pk_xtstart.tv_sec;

                /*
                 * Determine the difference between the round trip time
                 * and the smoothed round trip time.
                 */
                delta = pkcb.pk_rtt - (pkcb.pk_srtt >> 3);

                /*
                 * Update srtt
                 */
                if ((pkcb.pk_srtt += delta) <= 0)
                        pkcb.pk_srtt = 1 << 3;
#ifdef TFTP_DEBUG
		if (trace)
       			printf("       calculate delta:  %d\n", delta);
#endif /* TFTP_DEBUG */

                /*
                 * Update srttd
                 */
                if (delta < 0)
                        delta = -delta;
                delta -= pkcb.pk_srttd >> 2;
                if ((pkcb.pk_srttd += delta) <= 0)
                        pkcb.pk_srttd = 1 << 2;

                /*
                 * Nullify the next packet transmission timeout value.
                 */
                pkcb.pk_rxtnxt = 0;
#ifdef TFTP_DEBUG
		if (trace) {
        		printf("         calculate rtt:  %d\n", pkcb.pk_rtt);
        		printf("        calculate srtt:  %d+%d/8\n",
               	        	pkcb.pk_srtt >> 3, pkcb.pk_srtt & 7);
        		printf("       calculate srttd:  %d+%d/4\n",
                        	pkcb.pk_srttd >> 2, pkcb.pk_srttd & 3);
		}
#endif /* TFTP_DEBUG */
        }
        /*
         * If a retransmitted packet is being acknowledged, use
         * the current retransmission time for the next packet.
         */
        else {
                pkcb.pk_rxtnxt = pkcb.pk_rxtcur;
        }

}

/*
 * Send the requested file.
 */
sendfile(fd, name, mode)
	int fd;
	char *name;
	char *mode;
{
	register struct tftphdr *ap;       /* data and ack packets */
	struct tftphdr *r_init(), *dp;
	register int block = 0, size, n;
	register unsigned long amount = 0;
	struct sockaddr_in from;
	int fromlen, rc=0;
	int convert;                    /* true if converting lf -> crlf */
	FILE *file;


        /* Initialize packet control block */
        pkcb.pk_rtt = 0;
        pkcb.pk_srtt = TFTP_MIN_TIMEOUT << 3;
        pkcb.pk_srttd = 2 << 2;
        pkcb.pk_rxtcnt = 0;
        pkcb.pk_rxtcur = 0;
        pkcb.pk_rxtnxt = 0;

	startclock();           /* start stat's clock */
	dp = r_init();          /* reset fillbuf/read-ahead code */
	ap = (struct tftphdr *)ackbuf;
	convert = !strcmp(mode, "netascii");
	if (!file_stdin)
		file = fdopen(fd, "r");

	signal(SIGALRM, timer);

	/*
	 * we want to send the first packet to the connected address; but 
	 * subsequent packets go to the address that we pick up from the
	 * recvfrom() call.  this handles the case of multi-homed hosts
	 * that may respond to us from an address other than the one we
	 * sent to.
	 */

	from = sin;
	do {
		if (block == 0)
			size = makerequest(WRQ, name, dp, mode) - 4;
		else {
		/*      size = read(fd, dp->th_data, SEGSIZE);   */
			size = readit(file, &dp, convert);
			if (size < 0) {
				nak(errno + 100, &from);
				break;
			}
			dp->th_opcode = htons((u_short)DATA);
			dp->th_block = htons((u_short)block);
		}
		timeout = 0;
                pkcb.pk_rxtcnt = 0;
		(void) setjmp(timeoutbuf);
send_data:
#ifdef TFTP_DEBUG
		if (trace)
        		printf("\n                 block:  %d\n", block);
#endif /* TFTP_DEBUG */
		if (trace)
			tpacket(MSGSTR(SENT, "sent"), dp, size + 4); /*MSG*/
		n = sendto(f, dp, size + 4, 0, (caddr_t)&from, sizeof (from));
		if (n != size + 4) {
			perror(MSGSTR(TFTPSENTO, "tftp: sendto")); /*MSG*/
			rc = 1;
			goto abort;
		}
#ifdef FAULT_INSERTION
		debug_delay = (rand() & 15) + 1;
		if (trace)
			printf("pause %d\n", debug_delay);
		sleep(debug_delay);
		if (block & 2) {
			if (trace)
				tpacket(MSGSTR(SENT, "sent"), dp, 
					size + 4); /*MSG*/
			n = sendto(f, dp, size + 4, 0, 
				(caddr_t)&from, sizeof (from));
			if (n != size + 4) {
				perror(MSGSTR(TFTPSENTO, 
					"tftp: sendto")); /*MSG*/
				goto abort;
			}
		}
#endif /* FAULT_INSERTION */
		read_ahead(file, convert);
                start_pk_time();        /* set packet start time */
		alarm(rexmtval);
 		for ( ; ; ) {
			do {
				fromlen = sizeof (from);
				n = recvfrom(f, ackbuf, sizeof (ackbuf), 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			if (n < 0) {
				alarm(0);
				perror(MSGSTR(TFTPREC, "tftp: recvfrom")); /*MSG*/
				rc = 1;
				goto abort;
			}
			if (trace)
				tpacket(MSGSTR(RECEIVED, "received"), ap, n); /*MSG*/
			/* should verify packet came from server */
			ap->th_opcode = ntohs(ap->th_opcode);
			ap->th_block = ntohs(ap->th_block);
			if (ap->th_opcode == ERROR) {
				printf(MSGSTR(ERROR_CODE, "Error code %d: %s\n"), ap->th_code, /*MSG*/
					ap->th_msg);
				alarm(0);
				rc = 1;
				goto abort;
			}
			if (ap->th_opcode == ACK) {
                                /* An ack for the current block is expected */
				if (ap->th_block == block) {
#ifdef FAULT_INSERTION
					debug_delay = (rand() & 15) + 1;
					if (trace)
						printf("pause %d\n", 
							debug_delay);
					sleep(debug_delay);
#endif /* FAULT_INSERTION */
					alarm(0);
                                        end_pk_time(); /* set packet end time */
					break;
				}
                                /* An ack for a future block is illegal */
				if (ap->th_block > block) {
					alarm(0);
					nak(4, &from);
					goto abort;
				}
                                /* An ack for a previous block is ignored */
			}
		}

		if (block > 0)
			amount += size;
		block++;
	} while (size == SEGSIZE || block == 1);
abort:

	if (!file_stdin)
		fclose(file);
	stopclock();
	if (amount > 0)
		printstats(MSGSTR(CSENT, "Sent"), amount); /*MSG*/
        if (file_stdin)
                ioctl(0, TCSETAF, &deftio);

	return(rc); 
}
char *asdf;

/*
 * Receive a file.
 */
recvfile(fd, name, mode, localfile)
	int fd;
	char *name;
	char *mode;
	char *localfile;
{
	register struct tftphdr *ap;
	struct tftphdr *dp, *w_init();
	register int block = 1, n, size;
	unsigned long amount = 0;
	struct sockaddr_in from;
	int fromlen, firsttrip = 1;
	FILE *file;
	int convert;                    /* true if converting crlf -> lf */
	char *tmp;			/* holds the filename w/o the path */
	int ABORT = 0, rc=0;			


        /* Initialize packet control block */
        pkcb.pk_rtt = 0;
        pkcb.pk_srtt = TFTP_MIN_TIMEOUT << 3;
        pkcb.pk_srttd = 2 << 2;
        pkcb.pk_rxtcnt = 0;
        pkcb.pk_rxtcur = 0;
        pkcb.pk_rxtnxt = 0;

	startclock();
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	file = fdopen(fd, "w");
	convert = !strcmp(mode, "netascii");

	signal(SIGALRM, timer);

	/*
	 * we want to send the first packet to the connected address; but 
	 * subsequent packets go to the address that we pick up from the
	 * recvfrom() call.  this handles the case of multi-homed hosts
	 * that may respond to us from an address other than the one we
	 * sent to.
	 */

	from = sin;
	do {
		if (firsttrip) {
			size = makerequest(RRQ, name, ap, mode);
			firsttrip = 0;
		} else {
			ap->th_opcode = htons((u_short)ACK);
			ap->th_block = htons((u_short)(block));
			size = 4;
			block++;
		}
		timeout = 0;
                pkcb.pk_rxtcnt = 0;
		(void) setjmp(timeoutbuf);
send_ack:
#ifdef TFTP_DEBUG
		if (trace)
        		printf("\n                 block:  %d\n", block);
#endif /* TFTP_DEBUG */
		if (trace)
			tpacket(MSGSTR(SENT, "sent"), ap, size); /*MSG*/
		if (sendto(f, ackbuf, size, 0, (caddr_t)&from,
		    sizeof (from)) != size) {
			alarm(0);
			perror(MSGSTR(TFTPSENTO, "tftp: sendto")); /*MSG*/
                        ABORT = 1;
			rc = 1;
			goto abort;
		}
#ifdef FAULT_INSERTION
		debug_delay = (rand() & 15) + 1;
		if (trace)
			printf("pause %d\n", debug_delay);
		sleep(debug_delay);
		if (block & 2) {
			if (trace)
				tpacket(MSGSTR(SENT, "sent"), ap, size); /*MSG*/
			if (sendto(f, ackbuf, size, 0, (caddr_t)&from,
	    			sizeof (from)) != size) {
				alarm(0);
				perror(MSGSTR(TFTPSENTO, 
					"tftp: sendto")); /*MSG*/
                       		ABORT = 1;
				rc = 1;
				goto abort;
			}
		}
#endif /* FAULT_INSERTION */
		write_behind(file, convert);
                start_pk_time();        /* set packet start time */
		alarm(rexmtval);
		for ( ; ; ) {
			do  {
				fromlen = sizeof (from);
				n = recvfrom(f, dp, PKTSIZE, 0,
				    (caddr_t)&from, &fromlen);
			} while (n <= 0);
			if (n < 0) {
				alarm(0);
				perror(MSGSTR(TFTPREC, "tftp: recvfrom")); /*MSG*/
                                ABORT = 1;
				rc = 1;
				goto abort;
			}
			if (trace)
				tpacket(MSGSTR(RECEIVED, "received"), dp, n); /*MSG*/
			/* should verify client address */
			dp->th_opcode = ntohs(dp->th_opcode);
			dp->th_block = ntohs(dp->th_block);
			if (dp->th_opcode == ERROR) {
				alarm(0);
				printf(MSGSTR(ERROR_CODE, "Error code %d: %s\n"), dp->th_code, /*MSG*/
					dp->th_msg);
                        	ABORT = 1;
				rc = 1;
				goto abort;
			}
			if (dp->th_opcode == DATA) {
                                /* The next block is expected */
				if (dp->th_block == block) {
#ifdef FAULT_INSERTION
					debug_delay = (rand() & 15) + 1;
					if (trace)
						printf("pause %d\n", 
							debug_delay);
					sleep(debug_delay);
#endif /* FAULT_INSERTION */
					alarm(0);
                                        end_pk_time(); /* set packet end time */
					break;
				}
                                /* A future block is illegal */
				if (dp->th_block > block) {
					alarm(0);
					ABORT = -1;
					nak(4, &from);
					goto abort;
				}
                                /* A previous block is ignored */
			}
		}

	/*      size = write(fd, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, convert);
		if (size < 0 || errno == ENOSPC) {
			nak(errno + 100, &from);
			if (errno == ENOSPC) {
				printf(MSGSTR(DISKFULL,
					"Disk full or allocation exceeded"));
				putchar('\n');
                                /* We do not set ABORT=1 and remove the    */
                                /* file in this case, because the user may */
                                /* be able to use the portion of the file  */
                                /* that was transferred.                   */
                                rc = 1;
			}
			break;
		}
		amount += size;
	} while (size == SEGSIZE);
abort:                             /* ok to ack, since user has seen err msg */
	ap->th_opcode = htons((u_short)ACK);
	ap->th_block = htons((u_short)block);
	(void) sendto(f, ackbuf, 4, 0, &from, sizeof (from));
	write_behind(file, convert);            /* flush last buffer */
	fclose(file);
        if (ABORT){
	    tmp = strrchr(localfile, '/');	/* if transfer aborted       */
            if (tmp == NULL)			/* then get name without the */
                tmp = localfile;		/* path and remove it from   */
            else				/* the local directory.      */
                tmp++;
 	    unlink(tmp);
	}
	stopclock();
	if (amount > 0)
		printstats(MSGSTR(CRECEIVED, "Received"), amount); /*MSG*/

	return(rc);
}

makerequest(request, name, tp, mode)
	int request;
	char *name, *mode;
	struct tftphdr *tp;
{
	register char *cp;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	strcpy(cp, name);
	cp += strlen(name);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	return (cp - (char *)tp);
}

struct errmsg {
	int	e_code;
	char	*e_msg;
} errmsgs[9];

char err_msg[10][82];

err_load()
{
	int i;

	for(i=0; i<8; i++) 
		errmsgs[i].e_msg = &err_msg[i][0];

	errmsgs[0].e_code = EUNDEF;
	NCstrncpy(&err_msg[0][0], MSGSTR(UNDEFERR, "Undefined error code"), 40); /*MSG*/
	errmsgs[1].e_code = ENOTFOUND;
	NCstrncpy(&err_msg[1][0], MSGSTR(BADFILE, "File not found"), 40); /*MSG*/
	errmsgs[2].e_code = EACCESS;
	NCstrncpy(&err_msg[2][0], MSGSTR(ILLACESS, "Access violation"), 40); /*MSG*/
	errmsgs[3].e_code = ENOSPACE;
	NCstrncpy(&err_msg[3][0], MSGSTR(DISKFULL, "Disk full or allocation exceeded"), 40); /*MSG*/
	errmsgs[4].e_code = EBADOP;
	NCstrncpy(&err_msg[4][0], MSGSTR(ILLOPE, "Illegal TFTP operation"), 40); /*MSG*/
	errmsgs[5].e_code = EBADID;
	NCstrncpy(&err_msg[5][0], MSGSTR(BADID, "Unknown transfer ID"), 40); /*MSG*/
	errmsgs[6].e_code = EEXISTS;
	NCstrncpy(&err_msg[6][0], MSGSTR(FILEXIST, "File already exists"), 40); /*MSG*/
	errmsgs[7].e_code = ENOUSER;
	NCstrncpy(&err_msg[7][0], MSGSTR(BADUSER, "No such user"), 40); /*MSG*/
	errmsgs[8].e_code = -1;
	errmsgs[8].e_msg = 0;
}
/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error, sinp)
	int error;
	struct sockaddr_in *sinp;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;
	extern char *sys_errlist[];


	tp = (struct tftphdr *)ackbuf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		pe->e_msg = sys_errlist[error - 100];
		tp->th_code = EUNDEF;
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg) + 4;
	if (trace)
		tpacket(MSGSTR(SENT, "sent"), tp, length); /*MSG*/
	if (sendto(f, ackbuf, length, 0, sinp, sizeof (*sinp)) != length)
		perror("nak");

}

tpacket(s, tp, n)
	char *s;
	struct tftphdr *tp;
	int n;
{
	static char *opcodes[] =
	   { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };
	register char *cp, *file;
	u_short op = ntohs(tp->th_opcode);
	char *index();
	char tmp[80];


	strcpy(tmp, s);
	if (op < RRQ || op > ERROR)
		printf(MSGSTR(OPCODE, "%s opcode=%x "), tmp, op);/*MSG*/
	else
		printf(MSGSTR(OPCODES, "%s %s "), tmp, opcodes[op]); /*MSG*/
	switch (op) {

	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
		cp = index(cp, '\0');
		printf(MSGSTR(FILE_MODE, "<file=%s, mode=%s>\n"), file, cp + 1); /*MSG*/
		break;

	case DATA:
		printf(MSGSTR(BLOCK_BYTES, "<block=%d, %d bytes>\n"), ntohs(tp->th_block), n - 4); /*MSG*/
		break;

	case ACK:
		printf(MSGSTR(BLOCK, "<block=%d>\n"), ntohs(tp->th_block)); /*MSG*/
		break;

	case ERROR:
		printf(MSGSTR(CODE_MSG, "<code=%d, msg=%s>\n"), ntohs(tp->th_code), tp->th_msg); /*MSG*/
		break;
	}

}

struct timeval tstart;
struct timeval tstop;

startclock() {
	gettimeofday(&tstart, &zone);
}

stopclock() {
	gettimeofday(&tstop, &zone);
}

printstats(direction, amount)
char *direction;
unsigned long amount;
{
	double delta;
	char tmp[40];
			/* compute delta in 1/10's second units */
	delta = ((tstop.tv_sec*10.)+(tstop.tv_usec/100000)) -
		((tstart.tv_sec*10.)+(tstart.tv_usec/100000));
	delta = delta/10.;      /* back to seconds */
	strcpy(tmp, direction);
        printf(MSGSTR(STATS, "%s %d Bytes in %.1f Seconds"), tmp, amount, delta);
	if (verbose)
		printf(MSGSTR(BITS_PER_SEC, " [%.0f bits/sec]"), (amount*8.)/delta); /*MSG*/
	putchar('\n');
}

