static char sccsid[] = "@(#)92	1.44  src/bos/usr/sbin/rdump/dumptape.c, cmdarch, bos411, 9428A410j 3/14/94 10:42:42";
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
#include <sys/errno.h>
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
extern daddr_t fs_size;
int	range = 0;
int	unit;
int	min_unit;
int	max_unit;
char	unit_prefix[PATH_MAX];
ino_t	last_ino_dumped;

/*
 * Concurrent dump mods (Caltech) - disk block reading and tape writing
 * are exported to several slave processes.  While one slave writes the
 * tape, the others read disk blocks; they pass control of the tape in
 * a ring via flock().	The parent process traverses the filesystem and
 * sends spclrec()'s and lists of daddr's to the slaves via pipes.
 */
struct req {			/* instruction packets sent to slaves */
	struct req *next;
	daddr_t dblk;
	int count;
	int isspcl;
	char *tblockp;
        int origin;     /* addr of first uncompressed sector containing dblk */
        int length;     /* length compressed block in bytes */
} *req;
int reqsiz;
extern int compressed;

#define SLAVES 5		/* 1 slave writing, 1 reading, 1 for slack */
int slavefd[SLAVES];		/* pipes from master to each slave */
int slavepid[SLAVES];		/* used by killall() */
int slrdfd[SLAVES];		/* status reports come in here */

int msgid[SLAVES];		/* handle for the message queue */
struct {
  long  mtype ;
  int   mtext ;
} msg_token = { 0xAB, 0xCD } ;      /* message queue operation buffer */

struct req *anchor = NULL;	/* beginning of list to write out */
struct req *start_rec = NULL;	/* the beginning of the record */
long current_inode;		/* Current inode number we are working on */
union u_spcl lsprec;		/* Special record we have on last tape */
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


taprec(dp, flag)
	char *dp;
	int flag;
{
	struct req *t;

	t = malloc(sizeof (struct req));
	if (t == NULL)
		dumpabort();

	t->tblockp = malloc(TP_BSIZE);

	if (t->tblockp == NULL)
		dumpabort();

	t->dblk = (daddr_t)0;
	t->count = 1;
	*(union u_spcl *)(t->tblockp) = *(union u_spcl *)dp;	/* movc3 */
	t->isspcl = flag;
        t->origin = 0;
        t->length = 0;
	spcl.c_tapea++;
	output(t);
}

dmpblk(blkno, size)
	daddr_t blkno;
	int size;
{
	int avail, tpblks, dblkno, origin, len, fperpage;
	struct req *t;
	fdaddr_t blkaddr;

	/* we are working with the disk block in terms of 512 byte blocks */
	blkaddr.d = blkno;
	blkno = blkaddr.f.addr;
	dblkno = blkno * (FragSize / DEV_BSIZE);
        /* in case blkno parameter was nonsense (a frag with 0 address)
         * setting it to fs_size results in zeros (see blkread()).
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

	while ((avail = min(tpblks, 1)) > 0) {
		t = malloc(sizeof (struct req));
		if (t == NULL)
			dumpabort();

		t->dblk = dblkno;
		t->count = avail;
		t->isspcl = FALSE;
		t->tblockp = NULL;
                t->origin = origin;
                t->length = len;
		output(t);
		spcl.c_tapea += avail;
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

flusht()
{
	char buf[BUFSIZ]; /* message from slave */
	int i, nb;
	struct req *t;
	struct s_spcl *xxx;
	static int EOT_flag;

	if (atomic(read, slrdfd[rotor], buf, 10) != 10) {
		perror(MSGSTR(DUMPCOMP, "error reading command pipe"));
		dumpabort();
	}
	buf[10] = '\0'; /* Safety Sake */
	if (EOT_flag || strncmp(buf, "EOT       ", 3) == 0) {
		/* Fix up all the recs and write out headers */
		blockspervol = 0;
		EOT_flag = FALSE;
		close_rewind();
		otape();
		return;
	}
	nb = atoi(buf);
	if (nb != 0) {
		if (nb > ntrec || nb < 0) {
			perror(MSGSTR(DUMPCOMP,
				"error reading command pipe"));
			dumpabort();
		}
		if (nb != ntrec)
			EOT_flag = TRUE;
		/*
		 * We actually wrote a tape block.  Now we need to do
		 * house cleaning.  We look through the records to find
		 * an inode block.  If we find one we fix up the bit map
		 */
		for (i = 0, t = anchor; i < nb; i++) {
			if (t == NULL)
				dumpabort(); /* Cannot happen */
			if (t->isspcl) {
				xxx = &((union u_spcl *)t->tblockp)->s_spcl;
				lsprec.s_spcl = *xxx; /* Save it away for next tape */
				lsprec.s_spcl.c_tapea--;
				if (xxx->c_type == TS_INODE) {
					current_inode = xxx->c_inumber;
					BIC(current_inode, nodmap);
				}
			}
			anchor = anchor->next;
			if (t->tblockp)
				free(t->tblockp);
			free(t);
			t = anchor;
			lsprec.s_spcl.c_tapea++;
		}
	}

	/*
	 * Write out the next group of requests
	 */
	for (i = 0, t = start_rec; i < ntrec; i++) {
		if (t == NULL) {
			perror(MSGSTR(ERRCOMP, "error writing command pipe"));
			dumpabort();
		}
		if (atomic(write, slavefd[rotor], t, sizeof(*t)) != sizeof(*t)){
			perror(MSGSTR(ERRCOMP,
				"error writing command pipe"));
			dumpabort();
		}
		t = t->next;
	}
	/*
	 * Write out any data blocks that might be needed
	 */
	for (i = 0, t = start_rec; i < ntrec; i++) {
		if (t->tblockp) {
			if (atomic(write, slavefd[rotor], t->tblockp,
				TP_BSIZE) != TP_BSIZE){
				perror(MSGSTR(ERRCOMP,
					"error writing command pipe"));
				dumpabort();
			}
		}
		t = t->next;
	}
	start_rec = t;
	if (++rotor >= SLAVES) rotor = 0;
	tblock = (char (*)[TP_BSIZE]) &req[ntrec];
	trecno = 0;
	asize += tenths;
	blockswritten += ntrec;
	blockspervol += ntrec;
	timeest();
}

dump_rewind()
{
	int f;
	struct stat statbuf;

/* close the write descriptors on the pipe, so child could get an eof */
	for (f = 0; f < SLAVES; f++)
		close(slavefd[f]);
	while (wait(NULL) >= 0)      
		;		      /* wait for any signals from slaves */

	if (pipeout)		/* A12856 */
		return;		/* A12856 */

	/* give rewind message if appropriate */
	if (!diskette &&
	    strstr(tape, "rmt") &&
	    stat(tape, &statbuf) == 0 &&
	    !(minor(statbuf.st_rdev) & TAPE_NOREW_ON_CLOSE))
		msg(MSGSTR(TREWIND, "Tape rewinding\n"));

	(void) close(to);
	while ((f = open(tape, 0)) < 0)
		sleep (10);
	close(f);
	/*
	 * Get rid of the slave responses
	 */
	for (f = 0; f < SLAVES; f++)
		close(slrdfd[f]);
}

close_rewind()
{
	int newprompt;
	char *mesg;

	dump_rewind();
	newprompt = 1;
	if (range) {
		unit++;
		if (unit > max_unit)
			unit = min_unit;
		else
			newprompt = 0;
		sprintf(tape, "%s%d", unit_prefix, unit);
	}
	if (!nogripe && newprompt) {
		msg(MSGSTR(CHANGM, "Change Media: Insert volume #%d\n"), tapeno+1);
		msg(MSGSTR(CHANGM2, "CHANGE MEDIA!\7\7\n"));
	}
	if (newprompt) {
		if (range) {
			mesg = MSGSTR(NEWVOLS, "Are the new volumes ready to go?");
		} else {
			mesg = MSGSTR(NEWVOL, "Is the new volume ready to go?");
		}
		while (!query(mesg, NO))
			if (query(MSGSTR(ABORTDUM, "Do you want to abort?"), YES)) {
				dumpabort();
				/*NOTREACHED*/
			}
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
				msg(MSGSTR(BADCODE, "Bad return code from backup: %d\n"), status);
				Exit(X_ABORT);
		}
		/*NOTREACHED*/
	} else {	/* we are the child; just continue */
#ifdef TDEBUG
		sleep(4);	/* allow time for parent's message to get out */
		msg("Child on Tape %d has parent %d, my pid = %d\n",
			tapeno+1, parentpid, getpid());
#endif TDEBUG
		while ((to = (pipeout ? 1 : 
			is_first_tape ? first_tape_fd : creat(tape, (mode_t)0666))) < 0)
		  {
			msg(MSGSTR(OPENTF, "Cannot open %s.\n"),tape);
			if (!query(MSGSTR(OPRETRY,"Do you want to retry the open? "), NO))
				dumpabort();
          }
		is_first_tape=0;
		enslave();  /* Share open tape file descriptor with slaves */

		asize = 0;
		tapeno++;		/* current tape sequence */
		newtape++;		/* new tape signal */
		spcl.c_volume++;
		if (tapeno > 1) {
			lsprec.s_spcl.c_magic = NFS_MAGIC;
			lsprec.s_spcl.xix_flag = XIX_MAGIC;
			lsprec.s_spcl.c_volume++;
			lsprec.s_spcl.c_type = TS_TAPE;
			lsprec.s_spcl.c_checksum = 0;
			summit(&lsprec.s_spcl);
			start_new_tape(); /* Handle EOT */
		} else {
			spcl.c_type = TS_TAPE;
			spclrec();
		}
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
		msg(MSGSTR(ENTIRE, "The backup command has ended abnormally.\n"));
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
 * gets a message queue id for all the child processes to use.
 */
getmsgid()
{
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
 * Start the child processes. All except the first child will block in this
 * routine when they are first created. They will block until msgrcv() is
 * satisfied.
 */
enslave()
{
	register int i, j;
	int cmd[2], in[2];

	master = getpid();
	dump_signal(SIGTERM, (void (*)(int))dumpabort); /* Slave sends SIGTERM on dumpabort() */

	dump_signal(SIGPIPE, (void (*)(int))sigpipe);
	dump_signal(SIGUSR1, (void (*)(int))tperror);    /* Slave sends SIGUSR1 on tape errors */
	dump_signal(SIGUSR2, (void (*)(int))ioerror);    /* Slave sends SIGUSR2 on I/O errors */
	getmsgid();
	for (i = 0; i < SLAVES; i++) {
		if (pipe(cmd) < 0 || pipe(in) || (slavepid[i] = fork()) < 0) {
			msg(MSGSTR(SLAVES1, "too many slaves, %d (recompile smaller) "), i);
			perror("");
			dumpabort();
		}
		slavefd[i] = cmd[1];
		slrdfd[i] = in[0];
		if (slavepid[i] == 0) { 	    /* Slave starts up here */
			for (j = 0; j <= i; j++) {
				close(slavefd[j]);
				close(slrdfd[j]);
			}
			dump_signal(SIGINT, SIG_IGN);    /* Master handles this */
			doslave(cmd[0], in[1], i);
			if (close(to) < 0) {
				kill(master, SIGUSR1);
				for (;;)
					sigpause(0);
			}
			if (io_error)
				Exit(X_IOERR);
			else
				Exit(X_FINOK);
		}
		close(cmd[0]);
		close(in[1]);
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
doslave(cmd, reply, child_num)
	register int cmd, reply, child_num;
{
	register int nread;
	register int next;
	char buf[BUFSIZ];
	int wcnt;

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
	 * First we give a NOP to the parent so he can handle the write
	 */
	if (atomic(write, reply, "NOP       ", 10) != 10) {
		perror(MSGSTR(DUMPCOMP, "error writing command pipe"));
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
                                } else  /* compressed block */
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
		if ((wcnt = write(to, tblock[0], writesize)) != writesize) {
			/*
			 * Try the operation again.  If we hit EOT we will
			 * get back ENXIO
			 */
			if (wcnt < 0)
				wcnt = 0;
			if (write(to, tblock[0], writesize) < 0) {
				if (errno == ENXIO) {
					sprintf(buf, "%10d", wcnt/TP_BSIZE);
					if (wcnt) {
						if (atomic(write, reply, buf,
						    10) != 10) {
						    perror(MSGSTR(DUMPCOMP,
					"error writing command pipe"));
						    kill(master, SIGUSR1);
						    for (;;)
							sigpause(0);
						}
					} else {
						if (atomic(write, reply, "EOT          ",
						    10) != 10) {
						    perror(MSGSTR(DUMPCOMP,
					"error writing command pipe"));
						    kill(master, SIGUSR1);
						    for (;;)
							sigpause(0);
						}
					}
					goto out;
				}
			}
			kill(master, SIGUSR1);
			for (;;)
				sigpause(0);
		}
		sprintf(buf, "%10d", wcnt/TP_BSIZE);
		if (atomic(write, reply, buf, 10) != 10) {
			perror(MSGSTR(DUMPCOMP, "error writing command pipe"));
			dumpabort();
		}

		/* wake up the next guy */
		MSGsnd( next );
	}
	if (nread != 0) {
		perror(MSGSTR(DUMPCOMP, "error reading command pipe"));
		dumpabort();
	}
out:
	/* empty read on pipe, so we need to finish up, also wake up the next guy */
	MSGsnd( next );
	/*
	 * We go to sleep until everybody has had a chance to detect EOF.
	 * This is needed since the message queue will be removed
	 */
	sleep(5);
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
	char *newbuf, *sv;

	sv = newbuf = malloc (blks * TP_BSIZE);
	if (type == TS_ACL)
		memcpy (newbuf, buffer, ((struct acl *)buffer)->acl_len);
	else
		memcpy (newbuf, buffer, ((struct pcl *)buffer)->pcl_len);

	for (i = 0; i < blks; i++) {
		taprec(newbuf, FALSE);
		newbuf += TP_BSIZE;
	}
	free(sv);
}

/*
 * Grand routine to handle the beginning of a tape.  We already have everything
 * set up, just need to put the special record at the beginning of the tape
 * and put in the bitmap before the first inode.
 */
start_new_tape()
{
	struct req *t, *tsave;
	struct s_spcl *xxx;
	int save_inode;
        int eot_flag = 0;

	/*
	 * Walk the list of outstanding requests and change the spcl volume
	 * numbers
	 */
	for (t = anchor; t != NULL; t = t->next) {
		if (t->isspcl) {
			xxx = &((union u_spcl *)t->tblockp)->s_spcl;
			xxx->c_volume = spcl.c_volume;
			xxx->c_checksum = 0;
			summit(xxx);
		}
	}
	/*
	 * Remove the chain so we can add the entries
	 */
	t = anchor;
	anchor = NULL;
	start_rec = NULL;
	/*
	 * The special record is all set up now, put it at the front
	 */
	taprec(&lsprec.s_spcl, TRUE);	/* New Tape marker */

	while (t != NULL) {
		if (newtape && t->isspcl) {
			xxx = &((union u_spcl *)t->tblockp)->s_spcl;
			if (xxx->c_type == TS_INODE) {
				/*
				 * Have to put in bitmaps ourselves here
				 */
				newtape = 0; /* We are handling */
				/*
				 * Put the correct inode number on tape
				 * bit maps.
				 */
				save_inode = ino;
				ino = xxx->c_inumber;
				spcl.c_inumber = xxx->c_inumber;
				bitmap(nodmap, TS_BITS);
				spcl.c_inumber = save_inode;
				ino = save_inode;
			}
		}
		/* have to save away since NULL'ed */
		tsave = t->next;	
		output(t);
		t = tsave;
	}
}

summit(spclp)
	struct s_spcl *spclp;
{
	int s, i, *ip;

	spclp->c_checksum = 0;

	ip = (int *) spclp;
	s = 0;
	i = sizeof (union u_spcl) / (4 * sizeof(int));
	while (--i >= 0) {
		s += *ip++; s += *ip++;
		s += *ip++; s += *ip++;
	}
	spclp->c_checksum = CHECKSUM - s;
}


output(t)
	struct req *t;
{
	struct req *tmp;

	if (anchor == NULL) {
		anchor = t;
		t->next = NULL;
	} else {
		tmp = anchor;
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = t;
		t->next = NULL;
	}
	if (start_rec == NULL) {
		start_rec = t;
	}
	start_output();
}

start_output()
{
	struct req *t;
	int count;

	t = start_rec;
	count = 0;
	while (t != NULL) {
		count++;
		if (count >= ntrec) {
			flusht();
			count = 0;
			return;
		}
		t = t->next;
	}
	trecno = count;
}

check_eot()
{
        char buf[BUFSIZ]; /* message from slave */
        int i, j, nb;
        struct req *t;
        struct s_spcl *xxx;
	int hit_eot=0,wrote_eot_rec=0;

	for(i=0; i<SLAVES; i++) {
	        if (atomic(read, slrdfd[rotor], buf, 10) != 10) {
			perror(MSGSTR(DUMPCOMP, "error reading command pipe"));
			dumpabort();
		}
	        buf[10] = '\0'; /* Safety Sake */
		if (strncmp(buf, "EOT       ", 3) == 0) {
			hit_eot = 1;
			if (++rotor >= SLAVES) rotor = 0;
			continue;
		}
        	nb = atoi(buf);
        	if (nb != 0) {
                	if (nb > ntrec || nb < 0) {
                       		perror(MSGSTR(DUMPCOMP, 
					"error reading command pipe"));
                       		dumpabort();
                	}
                	/*
                 	 * We actually wrote a tape block.  Now we need to do
                 	 * house cleaning.  We look through the records to find
                 	 * an inode block.  If we find one we fix up the bit map
                 	 */
                	for (j = 0, t = anchor; j < nb; j++) {
                       		if (t == NULL)
                               		dumpabort(); /* Cannot happen */
                       		if (t->isspcl) {
                               		xxx = &((union u_spcl *)t->tblockp)->s_spcl;
					/* Save it away for next tape */
                               		lsprec.s_spcl = *xxx; 
                               		lsprec.s_spcl.c_tapea--;
                               		if (xxx->c_type == TS_INODE) {
                                       		current_inode = xxx->c_inumber;
                                       		BIC(current_inode, nodmap);
                               		}
					if (xxx->c_type == TS_END) {
						wrote_eot_rec++;
					}
                       		}
                       		anchor = anchor->next;
                       		if (t->tblockp)
                               		free(t->tblockp);
                       		free(t);
                       		t = anchor;
                       		lsprec.s_spcl.c_tapea++;
                	}
		}
		if (++rotor >= SLAVES) rotor = 0;
	}
	if (hit_eot && !wrote_eot_rec) {
		/* Fix up all the recs and write out headers */
		ino = last_ino_dumped;
		blockspervol = 0;
		close_rewind();
		otape();
	}
	return;
}
