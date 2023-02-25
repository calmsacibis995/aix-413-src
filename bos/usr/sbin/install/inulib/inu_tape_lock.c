static char sccsid[] = "@(#)11	1.1  src/bos/usr/sbin/install/inulib/inu_tape_lock.c, cmdinstl, bos411, 9428A410j 3/23/94 14:48:50";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: active_tape_restore
 *		do_lock_tape_restore
 *		lock_tape_restore
 *		unlock_tape_restore
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

#define ACTIVE_RESTORE_TAPE "/usr/lib/instl"

/* wait until there is no active tape action */
void active_tape_restore()
{
	int SemID;
	int rc;
	key_t Key;
	struct sembuf sembuf;

	Key = ftok(ACTIVE_RESTORE_TAPE, 'b');

	do { 
		SemID = semget(Key, 1, IPC_EXCL);
		if(SemID == -1 || semctl(SemID,0,GETVAL,0) != 1)
			return;
	
		sembuf.sem_num=0;
		sembuf.sem_op=0;
		sembuf.sem_flg=0;
	
		/* loop just incase we get spanked by some old signal ... */
	} while (semop(SemID,&sembuf,(unsigned int) 1) == -1);
}

/* lock that tape */
static int do_lock_tape_restore()
{
	key_t Key;
	int SemID;
	struct sembuf sembuf[2];

	sembuf[0].sem_num=0;
	sembuf[0].sem_op=0;
	sembuf[0].sem_flg=0;
	sembuf[1].sem_num=0;
	sembuf[1].sem_op=1;
	sembuf[1].sem_flg=SEM_UNDO;

	Key = ftok(ACTIVE_RESTORE_TAPE, 'b');
	SemID = semget(Key, 1, IPC_EXCL|IPC_CREAT|S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (SemID == -1) 
	{
		SemID = semget(Key, 1, IPC_EXCL);
		if (SemID == -1 || (semop(SemID,sembuf,(unsigned int) 2) == -1))
			return(-1);
	}
	else if(semop(SemID,sembuf,(unsigned int) 2)== -1)
		return(-1);

	return(0);
}

void delete_tape_lock()
{
	key_t Key;
	int SemID;

	Key = ftok(ACTIVE_RESTORE_TAPE, 'b');
	SemID = semget(Key, 1, 0);
	if (SemID == -1)
		return;
	semctl(SemID,0,IPC_RMID,0);
}

void lock_tape_restore()
{
	/* just incase we get spanked by some signal ... */
	while(do_lock_tape_restore() != 0);
}
