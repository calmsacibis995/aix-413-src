static char sccsid[] = "@(#)26        1.1  src/bos/usr/sbin/trace/lock_sema.c, cmdtrace, bos41B, 412_41B_sync 11/29/94 10:43:59";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: lock_sema unlock_sema
 *
 * ORIGINS: 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  @IBM_COPYRIGHT@
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include "trace.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/errno.h>

extern sid;

#define SEMA_KEY 16893672

/*
 * NAME:     lock_sema
 * FUNCTION: lock with semaphore
 * INPUTS:   none
 * RETURNS:  1 if error 
 */

lock_sema()
{
struct sembuf sb;


        if ((sid = semget(SEMA_KEY, 1, 0666)) == -1) {
                if (errno == ENOENT) {  /* the semaphore not yet exist  */
                        if ((sid = semget(SEMA_KEY, 1, 0666|IPC_CREAT)) == -1) {
                                perror ("semget");
                                return(1);
                        }

                } else {
                        perror ("semget");
                        return(1);
                  }
        }
        else {
                /* Grab the semaphore */
                sb.sem_num = 0;
                sb.sem_op = -1;
                sb.sem_flg = 0;
                if (semop(sid, &sb, 1) == -1) {
                        perror ("semopget");
                        return(1);
                }
        }
}


/*
 * NAME:     unlock_sema
 * FUNCTION: unlock the trace semaphore
 * INPUTS:   none
 * RETURNS:  1 if error
 */

unlock_sema()
{
struct sembuf sb;

         sb.sem_num = 0;
         sb.sem_op = 1;
         sb.sem_flg = 0;
         /* Release the semaphore */
         if (semop(sid, &sb, 1) == -1) {
                 perror ("semoprel");
                 return(1);
         }
}

