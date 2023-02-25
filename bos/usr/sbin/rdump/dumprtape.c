static char sccsid[] = "@(#)60	1.20  src/bos/usr/sbin/rdump/dumprtape.c, cmdarch, bos411, 9428A410j 12/14/93 16:32:39";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)dumptape.c	5.5 (Berkeley) 5/23/86";
#endif not lint
*/

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/id.h>
#include "dump.h"
#include <string.h>
#include <sys/watchdog.h>
#include <sys/scsi.h>
#include <sys/tapedd.h>

int     dumpabort(void);
char	(*tblock)[TP_BSIZE];	/* Pointer to malloc()ed buffer for tape */
int	writesize;		/* Size of malloc()ed buffer for tape */
int	trecno = 0;
int	blockspervol = 0;	/* blocks written per disk or tape volume */
extern int ntrec;		/* blocking factor on tape */
extern int cartridge;
extern int read(), write();
extern void (*dump_signal()) ();
extern int first_tape_fd;
extern int is_first_tape;
extern int io_error;
extern char *host;
extern daddr_t fs_size;

/*
 * Concurrent dump mods (Caltech) - disk block reading and tape writing
 * are exported to several slave processes.  While one slave writes the
 * tape, the others read disk blocks; they pass control of the tape in
 * a ring via flock().	The parent process traverses the filesystem and
 * sends spclrec()'s and lists of daddr's to the slaves via pipes.
 */
struct req {			/* instruction packets sent to slaves */
	daddr_t dblk;
	int count;
        int origin;     /* addr of first uncompressed sector containing dblk */
        int length;     /* length compressed block in bytes */
} *req;
int reqsiz;
extern int compressed;

#define SLAVES 5		/* 1 slave writing, 1 reading, 1 for slack */
int slavefd[SLAVES];		/* pipes from master to each slave */
int slavepid[SLAVES];		/* used by killall() */

int msgid[SLAVES];		/* handle for the message queue */
struct {
  long  mtype ;
  int   mtext ;
} msg_token = { 0xAB, 0xCD } ;      /* message queue operation buffer */

key_t key, ftok();
int rotor;			/* next slave to be instructed */
int master;			/* pid of master, for sending error signals */
int tenths;			/* length of tape used per block written */

alloctape()
{
	int pgoff = getpagesize() - 1;

	writesize = ntrec * TP_BSIZE;
	reqsiz = ntrec * sizeof(struct req);
	/*
	 * CDC 92181's and 92185's make 0.8" gaps in 1600-bpi start/stop mode
	 * (see DEC TU80 User's Guide).  The shorter gaps of 6250-bpi require
	 * repositioning after stopping, i.e, streaming mode, where the gap is
	 * variable, 0.30" to 0.45".  The gap is maximal when the tape stops.
	 */
	tenths = writesize/density + (cartridge ? 16 : density == 625 ? 5 : 8);
	/*
	 * Allocate tape buffer contiguous with the array of instruction
	 * packets, so flusht() can write them together with one write().
	 * Align tape buffer on page boundary to speed up tape write().
	 */
	req = (struct req *)malloc(reqsiz + writesize + pgoff);
	if (req == NULL)
		return(0);
	tblock = (char (*)[TP_BSIZE]) (((long)&req[ntrec] + pgoff) &~ pgoff);
	req = (struct req *)tblock - ntrec;
	return(1);
}


taprec(dp,flag)
	char *dp;
	int flag;
{
	req[trecno].dblk = (daddr_t)0;
	req[trecno].count = 1;
        req[trecno].origin = 0;
        req[trecno].length = 0;
	*(union u_spcl *)(*tblock++) = *(union u_spcl *)dp;	/* movc3 */
	trecno++;
	spcl.c_tapea++;
	if(trecno >= ntrec)
		flusht();
}

dmpblk(blkno, size)
	daddr_t blkno;
	int size;
{
	int avail, tpblks, dblkno, origin, len, fperpage;
	fdaddr_t blkaddr;

	/* we are working with the disk block in terms of 512 byte blocks */
	blkaddr.d = blkno;
	blkno = blkaddr.f.addr;
	dblkno = blkno * (FragSize / DEV_BSIZE);
        /* in case blkno parameter was nonsense (a frag with 0 address)
         * setting it to fs_size will result in zeros (see blkread()).
         */
        if (dblkno == 0)
                dblkno = fs_size;
	tpblks = size / TP_BSIZE;

        /* determine origin in 512 byte units, length in bytes */
        if (compressed == 0 || blkaddr.f.nfrags == 0)
        {
                origin = len = 0;
        }
        else
        {
                origin = dblkno;
                fperpage = PAGESIZE/FragSize;
                len = (fperpage - blkaddr.f.nfrags)*(FragSize);
                /* in case blkno parameter was nonsense */
                if (len < 0)
                        len = 0;
        }

	while ((avail = min(tpblks, ntrec - trecno)) > 0) {
		req[trecno].dblk = dblkno;
		req[trecno].count = avail;
                req[trecno].origin = origin;
                req[trecno].length = len;
		trecno += avail;
		spcl.c_tapea += avail;
		if (trecno >= ntrec)
			flusht();
		dblkno += avail * (TP_BSIZE / DEV_BSIZE);
		tpblks -= avail;
	}
}

int	nogripe = 0;

tperror(void) {
	if (pipeout) {
		msg(MSGSTR(MEDIAWRE, "Media write error on %s\n"), tape);
		msg(MSGSTR(CANTREC, "Cannot recover\n"));
		dumpabort();
		/* NOTREACHED */
	}
	msg(MSGSTR(MEDIAWRE2, "Media write error %d blocks into volume %d\n"),
		blockspervol, tapeno);
	msg(MSGSTR(MEDIAEE, "MEDIA ERROR!\n"));
	if (!query(MSGSTR(RESTART, "Do you want to restart?"), NO))
		dumpabort();
	msg(MSGSTR(REPLACE1,
		"Replace the faulty media with a new one.\n"));
	msg(MSGSTR(REPLACE2,
		"If the faulty media is a tape, then please wait\n"));
	msg(MSGSTR(REPLACE3,
		"for the tape to rewind before replacing it.\n"));

	msg(MSGSTR(REWRITE, "this backup volume will be rewritten.\n"));
	killall();
	nogripe = 1;
	close_rewind();
	Exit(X_REWRITE);
}

sigpipe(void)
{

	msg(MSGSTR(PIPEB, "Broken pipe\n"));
	dumpabort();
}

ioerror(void)
{
	dump_signal(SIGUSR2, (void (*)(int))ioerror);
	io_error++;
}

/*
 * compatibility routine
 */
tflush(i)
	int i;
{

	for (i = 0; i < ntrec; i++)
		spclrec();
}

flusht()
{
	int siz = (char *)tblock - (char *)req;

	if (atomic(write, slavefd[rotor], req, siz) != siz) {
		perror(MSGSTR(ERRCOMP, "error writing command pipe"));
		dumpabort();
	}
	if (++rotor >= SLAVES) rotor = 0;
	tblock = (char (*)[TP_BSIZE]) &req[ntrec];
	trecno = 0;
	asize += tenths;
	blockswritten += ntrec;
	blockspervol += ntrec;
	if ((lflag && blockspervol >= maxblock) || (!pipeout && asize > tsize)) {
		blockspervol = 0;
		close_rewind();
		otape();
	}
	timeest();
}

dump_rewind()
{
	int f;
	char *cp;
	int minor;

	if (pipeout)
		return;
/* close the write descriptors on the pipe, so child could get an eof */
	for (f = 0; f < SLAVES; f++)
		close(slavefd[f]);
	while (wait(NULL) >= 0)      
		;		      /* wait for any signals from slaves */

	/* give rewind message if appropriate; since we can't stat the remote
	   device we have to assume the minor devno from the filename */

	if (!diskette && strstr(tape, "rmt")) {
	    if (cp = strrchr(tape, '.'))
		minor = atoi(cp + 1);
	    else
		minor = 0;
	    if (!(minor & TAPE_NOREW_ON_CLOSE))
		    msg(MSGSTR(TREWIND, "Tape rewinding\n"));
	}

	if (host) {
		if (rmtclose() < 0)
			tperror();
		while (rmtopen(tape, 0) < 0)
			sleep(10);
		(void) rmtclose();
		return;
	}
	(void) close(to);
	while ((f = open(tape, 0)) < 0)
		sleep (10);
	close(f);
}

close_rewind()
{
	dump_rewind();
	if (!nogripe) {
		msg(MSGSTR(CHANGM, "Change Media: Insert volume #%d\n"), tapeno+1);
		msg(MSGSTR(CHANGM2, "CHANGE MEDIA!\7\7\n"));
	}
	while (!query(MSGSTR(NEWVOL, "Is the new volume ready to go?"), NO))
		if (query(MSGSTR(ABORTDUM, "Do you want to abort?"), YES)) {
			dumpabort();
			/*NOTREACHED*/
		}
}

/*
 *	We implement taking and restoring checkpoints on the tape level.
 *	When each tape is opened, a new process is created by forking; this
 *	saves all of the necessary context in the parent.  The child
 *	continues the dump; the parent waits around, saving the context.
 *	If the child returns X_REWRITE, then it had problems writing that tape;
 *	this causes the parent to fork again, duplicating the context, and
 *	everything continues as if nothing had happened.
 */

otape()
{
	int	parentpid;
	int	childpid;
	int	status;
	int	waitpid;
	int	(*interrupt)(void) = (int (*)(void))dump_signal(SIGINT, SIG_IGN);

	parentpid = getpid();

    restore_check_point:
	dump_signal(SIGINT, (void (*)(int))interrupt);
	/*
	 *	All signals are inherited...
	 */
	childpid = fork();
	if (childpid < 0) {
		msg(MSGSTR(FORKF, "Context save fork fails in parent %d\n"), parentpid);
		Exit(X_ABORT);
	}
	if (childpid != 0) {
		/*
		 *	PARENT:
		 *	save the context by waiting
		 *	until the child doing all of the work returns.
		 *	don't catch the interrupt
		 */
		dump_signal(SIGINT, SIG_IGN);
#ifdef TDEBUG
		msg("Tape: %d; parent process: %d child process %d\n",
			tapeno+1, parentpid, childpid);
#endif TDEBUG
                if (is_first_tape) close(first_tape_fd);
		while ((waitpid = wait(&status)) != childpid)
			msg(MSGSTR(DUMPPWAIT, "Parent %d waiting for child %d has another child %d return\n"),
				parentpid, childpid, waitpid);
		if (status & 0xFF) {
			msg(MSGSTR(CHILDR, "Child %d returns LOB status %o\n"),
				childpid, status&0xFF);
		}
		status = (status >> 8) & 0xFF;
#ifdef TDEBUG
		switch(status) {
			case X_IOERR:
				msg("Child %d finishes X_IOERR\n", childpid);
				break;
			case X_FINOK:
				msg("Child %d finishes X_FINOK\n", childpid);
				break;
			case X_ABORT:
				msg("Child %d finishes X_ABORT\n", childpid);
				break;
			case X_REWRITE:
				msg("Child %d finishes X_REWRITE\n", childpid);
				break;
			default:
				msg("Child %d finishes unknown %d\n",
					childpid, status);
				break;
		}
#endif TDEBUG
		switch(status) {
			case X_IOERR:
				Exit(X_IOERR);
			case X_FINOK:
				Exit(X_FINOK);
			case X_ABORT:
				Exit(X_ABORT);
			case X_REWRITE:
                                is_first_tape=0;
				goto restore_check_point;
			default:
				msg(MSGSTR(BADCODE, "Bad return code from rdump: %d\n"), status);
				Exit(X_ABORT);
		}
		/*NOTREACHED*/
	} else {	/* we are the child; just continue */
#ifdef TDEBUG
		sleep(4);	/* allow time for parent's message to get out */
		msg("Child on Tape %d has parent %d, my pid = %d\n",
			tapeno+1, parentpid, getpid());
#endif TDEBUG
		while ((to = (host ? rmtopen(tape, 2) :
                        pipeout ? 1 :
                        is_first_tape ? first_tape_fd : creat(tape, (mode_t)0666))) < 0)
		 {
		    msg(MSGSTR(OPENTF, "Cannot open %s.\n"), tape);
			if (!query(MSGSTR(OPRETRY,"\tDo you want to retry the open? "), NO))
		      dumpabort();
 		 }
                is_first_tape=0;
		enslave();  /* Share open tape file descriptor with slaves */

		asize = 0;
		tapeno++;		/* current tape sequence */
		newtape++;		/* new tape signal */
		spcl.c_volume++;
		spcl.c_type = TS_TAPE;
		spclrec();
		if (tapeno > 1)
			msg(MSGSTR(VOLBEG,
				"Volume %d begins with blocks from ino %d\n"),
				tapeno, ino);
	}
}

dumpabort(void)
{
	if (master != 0 && master != getpid())
		kill(master, SIGTERM);	/* Signals master to call dumpabort */
	else {
	  	killall();
		msg(MSGSTR(RENTIRE, "The rdump command has ended abnormally.\n"));
	}
	Exit(X_ABORT);
}

Exit(status)
{
#ifdef TDEBUG
	msg("pid = %d exits with status %d\n", getpid(), status);
#endif TDEBUG
	exit(status);
}

/*
 * The dump is completed, so we can now remove the message queue id.
 */
rmmsgid(i)
int i;
{
	if (msgctl(msgid[i], IPC_RMID, 0) < 0) {
		msg(MSGSTR(MSQCTL2, "Could not remove message queue\n"));
		perror("");
		dumpabort();
	}
}


/*
 * gets a messag-queue id for all the child processes to use.
 */
getmsgid()
{
	int initval;
	register int i;

	for (i = 0; i < SLAVES; i++) {
		if ((msgid[i] = msgget(IPC_PRIVATE, 0777|IPC_CREAT)) < 0) {
			msg(MSGSTR(MSQGETE, "Could not get a message queue\n"));
			perror("");
			dumpabort();
		}
	}
	/* initialize first message queue */
	MSGsnd( 0 );
}


/*
 * Start the child processes.  All except the first child will block in this
 * routine when they are first created.  They will block until msgrcv()
 * is satified.
 */
enslave()
{
	register int i, j;
	int cmd[2];

	master = getpid();
	dump_signal(SIGTERM, (void (*)(int))dumpabort); /* Slave sends SIGTERM on dumpabort() */

	dump_signal(SIGPIPE, (void (*)(int))sigpipe);
	dump_signal(SIGUSR1, (void (*)(int))tperror);    /* Slave sends SIGUSR1 on tape errors */
	dump_signal(SIGUSR2, (void (*)(int))ioerror);    /* Slave sends SIGUSR2 on I/O errors */
	getmsgid();
	for (i = 0; i < SLAVES; i++) {
		if (pipe(cmd) < 0 || (slavepid[i] = fork()) < 0) {
			msg(MSGSTR(SLAVES1, "too many slaves, %d (recompile smaller) "), i);
			perror("");
			dumpabort();
		}
		slavefd[i] = cmd[1];
		if (slavepid[i] == 0) { 	    /* Slave starts up here */
			for (j = 0; j <= i; j++)
				close(slavefd[j]);
			dump_signal(SIGINT, SIG_IGN);    /* Master handles this */
			doslave(cmd[0], i);
			if (io_error)
				Exit(X_IOERR);
			else
				Exit(X_FINOK);
		}
		close(cmd[0]);
	}
	master = 0; rotor = 0;
}

killall()
{
	register int i;

	for (i = 0; i < SLAVES; i++)
		if (slavepid[i] > 0)
			kill(slavepid[i], SIGKILL);
}

/*
 * Synchronization - each process has an associated message queue.  When one
 * process completes its write, it will send a message to the next process,
 * so that the next process can be awaken.  The current
 * process then does a msgrcv(), the process will block waiting for some
 * process to send a message.  This is essentially a scheme that
 * passes a token from one process to the next.
 */
doslave(cmd, child_num)
	register int cmd, child_num;
{
	register int nread;
	register int neg, next;

	neg = -child_num - 1;
	/* check to see if we need to circulate back to the first
	 * child process
	 */
	if (child_num == SLAVES - 1)
		next = 0;
	else
		next = child_num + 1;
	close(fi);
	if ((fi = open(disk, 0)) < 0) { 	/* Need our own seek pointer */
		perror(MSGSTR(SLAVEOP, "slave couldn't reopen disk"));
		dumpabort();
	}
	/*
	 * Get list of blocks to dump, read the blocks into tape buffer
	 */
	while ((nread = atomic(read, cmd, req, reqsiz)) == reqsiz) {
		register struct req *p = req, *q;

		/* Gather adjacent read requests into one read */
		for (trecno = p->count; trecno < ntrec;) {
			q = p + p->count;
			if (p->dblk + p->count * (TP_BSIZE / DEV_BSIZE) == 
				q->dblk && p->origin == q->origin) {
				p->count += q->count;
				trecno += q->count;
			} else {
				trecno += p->count;
				p = q;
			}
		}

		p = req;

		for (trecno = 0; trecno < ntrec; trecno += p->count, p += p->count) {
			if (p->dblk) {
				if (p->origin == 0) {
					if (blkread(p->dblk, tblock[trecno],
				    	    p->count * TP_BSIZE) < 0) {
						kill(master, SIGUSR2);
					}
				} else	/* compressed block */
                                        cblkread(p->dblk, tblock[trecno],
                                                p->count*TP_BSIZE, p->origin,
                                                p->length);
			} else {
				if (p->count != 1 || atomic(read, cmd,
				    tblock[trecno], TP_BSIZE) != TP_BSIZE) {
					msg(MSGSTR(MASTER, "Master/slave protocol botched.\n"));
					dumpabort();
				}
			}
		}

		/* time to go to sleep now and wait for our turn */
		MSGrcv( child_num );
		if ((host ? rmtwrite(tblock[0], writesize)
			: write(to, tblock[0], writesize)) != writesize) {
			kill(master, SIGUSR1);
			for (;;)
				sigpause(0);
		}
		/* wake up the next guy */
		MSGsnd( next );
	}
	if (nread != 0) {
		perror(MSGSTR(DUMPCOMP, "error reading command pipe"));
		dumpabort();
	}
	/* empty read on pipe, so we need to finish up, also wake up the next guy */
	MSGsnd( next );
	/*
	 * We go to sleep until everybody has had a chance to detect EOF.
	 * This is needed since the message queue will be removed
	 */
	sleep(2);
	rmmsgid(next);

}


MSGrcv( msg_id )
int msg_id ;
{
	if (msgrcv(msgid[ msg_id ], &msg_token, sizeof(int), 0, 0 ) < 0) {
		perror("msgrcv");
		dumpabort();
	}
}


MSGsnd( msg_id )
int msg_id ;
{
	if (msgsnd(msgid[ msg_id ], &msg_token, sizeof(int), 0 ) < 0) {
		perror("msgsnd");
		dumpabort();
	}
}


/*
 * Since a read from a pipe may not return all we asked for,
 * or a write may not write all we ask if we get a signal,
 * loop until the count is satisfied (or error).
 */
atomic(func, fd, buf, count)
	int (*func)(), fd, count;
	char *buf;
{
	int got, need = count;

	while ((got = (*func)(fd, buf, need)) > 0 && (need -= got) > 0)
		buf += got;
	return (got < 0 ? got : count - need);
}

dmpsec(buffer, blks, type)
char *buffer;
int blks;
int type;
{
	int i;
	char *newbuf,
	     *temp;

	newbuf = malloc (blks * TP_BSIZE);
	temp=newbuf;		/* remember original malloced address*/

	if (type == TS_ACL)
		memcpy (newbuf, buffer, ((struct acl *)buffer)->acl_len);
	else
		memcpy (newbuf, buffer, ((struct pcl *)buffer)->pcl_len);

	for (i=0; i< blks; i++) {
		req[trecno].dblk = (daddr_t)0;
		req[trecno].count = 1;
                req[trecno].origin = 0;
                req[trecno].length = 0;
	        *(union u_spcl *)(*tblock++) = *(union u_spcl *)newbuf;	
		trecno++;
		spcl.c_tapea++;
		if (trecno >= ntrec)
			flusht();
		newbuf+=TP_BSIZE;
	}
	free(temp);		/* free entire original chunk */
}
