static char sccsid[] = "@(#)17  1.17  src/bos/usr/sbin/auditbin/auditbin.c, cmdsaud, bos411, 9428A410j 11/12/93 14:36:55";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditbin daemon
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"audutil.h"
#include	"fcntl.h"
#include	"signal.h"
#include	"stdio.h"
#include	"sys/audit.h"
#include	"sys/errno.h"
#include	"sys/stat.h"
#include 	"locale.h"
#include	"IN/AFdefs.h"
#include	"sys/priv.h"
#include	"stdarg.h"

#define		NULLDEV			"/dev/null"
#define		AUDIT_LOCK_FILE		"/audit/auditb"
#define		NEXTBIN(p) (((p) == &(bins[0])) ? &(bins[1]) : &(bins[0]))

/*
 * Forward declarations
 */

static int fail(int, ...);
static int unlock(int);
static int getbin(struct bin_t *);
static int fixbin(struct bin_t *);
static int recovery();
static int runcmds(struct bin_t *, int);
static int lock(char *);
static int binreset(struct bin_t *);

char	*cmd_file;
char	*trail_file;
char	*orig_tty;		/* name of starting tty, or console if none */
int	binsize;
int	background;
int	lockfd;

struct bin_t {
	char	*name;
	int	fd;
	int	size;
	struct	aud_bin	binhead;
	struct	aud_bin	bintail;
};

struct bin_t bins[] = {
	{ "", -1 },
	{ "", -1 }
};
struct	bin_t	*curbin;


#include "auditbin_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITBIN,n,s)


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
main(int argc, char *argv[]){

	int	sync_pipe[2];
	int	auditreset;
	int	pid;
	char	*p;

	/*
	 * Save the name of this TTY so it can be used to output error
	 * messages before going into the background.
	 */

	if (! (orig_tty = ttyname (0)))
		orig_tty = "/dev/console";

	/* 
	 * Lose controlling terminal & change process group
	 * we must do this VERY early 
	 * (before any interesting files are opened) 
	 */

	setpgrp();
        freopen (NULLDEV, "r", stdin);
        freopen (NULLDEV, "w", stdout);
        freopen (NULLDEV, "w", stderr);
        kleenup(3, 0, 0);

	setlocale(LC_ALL, "");
	catd = catopen(MF_AUDITBIN, NL_CAT_LOCALE);

	if(privcheck(AUDIT_CONFIG)){

		errno = EPERM;

		exit1(errno, 
		MSGSTR(ERRPRIV, 
		"** AUDIT_CONFIG privilege required"), 
		0);

	}

        /*
         * Suspend auditing
         */

        if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0){
                errno = EPERM;

                exit1(errno,
                MSGSTR(ERRSUSPEND,
                "** cannot suspend auditing "),
                0);

        }

	ulimit(2, ((long) 1 << 21));

	p = (char *)getattr(PATH_CONFIG, STZ_BIN, KWD_BINSIZE);

	if(p == NULL){

		if(errno){

			fail(errno, "%s(\"%s\")%s\"%s\"", 
			MSGSTR(ERRATTR1, "** no file attribute "), 
			KWD_BINSIZE,
			MSGSTR(ERRATTR2, " in "), 
			PATH_CONFIG);

		}
		else {

			binsize = 32 * 1024;

		}
	}
	else {
		binsize = atoi (p);
	}

	p = (char *)getattr(PATH_CONFIG, STZ_BIN, KWD_BIN1);

	if(p == NULL){

		fail(0, "%s(\"%s\")%s\"%s\"", 
		MSGSTR(ERRATTR1, "** no file attribute "), 
		KWD_BIN1,
		MSGSTR(ERRATTR2, " in "), 
		PATH_CONFIG);

	}
	bins[0].name = p;

	p = (char *)getattr(PATH_CONFIG, STZ_BIN, KWD_BIN2);

	if(p == NULL){

		fail(0, "%s(\"%s\")%s\"%s\"", 
		MSGSTR(ERRATTR1, 
		"** no file attribute "), 
		KWD_BIN2,
		MSGSTR(ERRATTR2, 
		" in "), 
		PATH_CONFIG);
	}

	bins[1].name = p;

	trail_file = (char *)getattr(PATH_CONFIG, STZ_BIN, KWD_TRAIL);

	if(trail_file == NULL){

		fail(0, "%s(\"%s\")%s\"%s\"", 
		MSGSTR(ERRATTR1, 
		"** no file attribute "), 
		KWD_TRAIL,
		MSGSTR(ERRATTR2, 
		" in "), 
		PATH_CONFIG);
	}

	cmd_file = (char *)getattr(PATH_CONFIG, STZ_BIN, KWD_CMDS);

	if(cmd_file == NULL){

		fail(0, "%s(\"%s\")%s\"%s\"", 
		MSGSTR(ERRATTR1, 
		"** no file attribute "), 
		KWD_CMDS,
		MSGSTR(ERRATTR2, 
		" in "), 
		PATH_CONFIG);

	}

	/* 
	 * Sync pipe 
	 */
 
	pipe(sync_pipe);

	pid = fork();
	if(pid < 0){

		fail(errno,
		MSGSTR(ERRFORK, "** fork failed"),
		0);

	}

	if(pid > 0){

		/* 	
		 * Parent just waits for sync 
		 */

		char	buf[2];

		close(sync_pipe[1]);
		if(read(sync_pipe[0], buf, 2) < 0){

			fail(errno,
			MSGSTR(ERRSYNC, 
			"** synchronization with child process failed"),
			0);

		}
		close(sync_pipe[0]);
		exit(0);
	};

	/* 
	 * child 
	 */

	close(sync_pipe[0]);

	/* 
	 * Exits on failure 
	 */

	lock(AUDIT_LOCK_FILE);

	/* 
	 * Assure the bins exist, and are writable by everyone to enable 
	 * auditing in a diskless environment (all filesystems NFS mounted)
	 */

	if((bins[0].fd = open (bins[0].name, O_RDWR | O_CREAT, 0662)) < 0){
		errno = EACCES;

		fail (errno, "%s\"%s\"", 
		MSGSTR(ERRMKBIN, 
		"** cannot create bin file"), 
		bins[0].name);
	}

	if((bins[1].fd = open (bins[1].name, O_RDWR | O_CREAT, 0662)) < 0){
		errno = EACCES;

		fail (errno, "%s\"%s\"", 
		MSGSTR(ERRMKBIN, 
		"** cannot create bin file"), 
		bins[1].name);
	}

	fchmod (bins[0].fd, 0662);	/* so that permissions don't get */
	fchmod (bins[1].fd, 0662);	    /* overridden by umask */

	recovery();

	/* 
	 * Subsequent error messages should enter loop 
	 */

	background = 1;

	/* 
	 * Post parent 
	 */

	close (sync_pipe[1]);

	auditreset = 0;
	while(!auditreset){
		struct	bin_t	*nextbin;
		int	rc;

		nextbin = (struct bin_t *)NEXTBIN(curbin);

		rc = auditbin(AUDIT_EXCL | AUDIT_WAIT, curbin->fd, nextbin->fd, 
		binsize);

		if(rc < 0){

			if(errno == EINTR){

				auditreset = 1;

			}
			else {

				fail(errno,
				MSGSTR(ERRAUDBIN, 
				"** auditbin() system call failed"), 
				0);

			}
		}

		if(getbin(curbin)){

			fail (errno, "%s\"%s\"",
			MSGSTR(ERRBADBIN, "** corrupted bin file "), 
			curbin->name);

		}

		if(fixbin(curbin)){

			fail(errno, "%s\"%s\"",
			MSGSTR(ERRBADBIN, "** corrupted bin file "), 
			curbin->name);

		}

		runcmds(curbin, 0);
		binreset(curbin);

		/* 
		 * Swap fd_cur & fd_next 
	 	 */

		curbin = nextbin;
	}

	unlock(lockfd);

	exit (0);
};


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
getbin(struct bin_t *binp){

	struct	stat	stbuf;
	struct	aud_bin	*bh;
	int	fd;

	/* 
	 * Assure the bin made it out to disk 
	 */

	fd = binp->fd;
	fsync(fd);

	/*  
	 * Determine the bin size 
	 */

	if(fstat (fd, &stbuf) < 0){
		errno = EACCES;

		fail (errno, "%s%s", 
		MSGSTR(ERRNOBIN, 
		"** cannot find bin file "), 
		binp->name);
	}

	/* 
	 * Assume we have a bin header 
	 */

	binp -> size = stbuf.st_size - sizeof (binp -> binhead);
	if(binp -> size <= 0){
		binp -> size = 0;
		return (0);
	}

	/* 
	 * Verify that have bin header 
	 */

	bh = &(binp -> binhead);
	lseek(fd, (long) 0, 0);

	if(read(fd, bh, sizeof (*bh)) != sizeof (*bh)){

		goto	failed;

	}

	if(bh -> bin_magic != AUDIT_MAGIC){

		goto	failed;

	}

	return(0);

failed:

	errno = EACCES;
	return (-1);

}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
fixbin(struct bin_t *binp){

	struct	stat	stbuf;
	int	fd;

	/* 
	 * Check for an empty bin 
	 */

	if(binp->size <= 0){

		return (0);

	}

	/* 
	 * Read bin tail 
	 */

	fd = binp->fd;
	lseek(fd, -(sizeof binp -> bintail), 2);
	if(read (fd, &binp->bintail, sizeof(binp->bintail)) != 
	sizeof (binp -> bintail)){
		goto	failed;
	}
	
	/* 
	 * Supply, or overwrite, bin tail 
	 */

	if((binp->bintail.bin_magic == AUDIT_MAGIC) && 
	(binp->bintail.bin_tail == AUDIT_BIN_END)){

		/* 
		 * Valid tail - will update in place 
		 */

		binp->size -= sizeof (binp -> bintail);

		/* 
		 * Special case for empty bin 
		 */

		if(binp->size <= 0){

			/* 
			 * Don't want to *fail* on an empty bin ...
			 * but there's not much more for us to do here.
			 * it is the responsibility of the caller to check
			 * for a nonzero bin size 	
			 */

			binp->size = 0;
			return (0);
		}

		if(binp->bintail.bin_time == 0){

			binp -> bintail.bin_time = stbuf.st_mtime;

		}

	}
	else {

		/* 
		 * Invalid tail - will append 
		 */

		binp->bintail = binp -> binhead;
		binp->bintail.bin_tail = AUDIT_BIN_END;
		binp->bintail.bin_time = stbuf.st_mtime;

	}

	binp->binhead.bin_len = binp -> bintail.bin_len = binp -> size;
	binp->binhead.bin_plen = binp -> bintail.bin_plen = 0;

	lseek(fd, (long) 0, 0);
	if(write (fd, &(binp->binhead), sizeof ((binp->binhead))) != 
	sizeof ((binp->binhead))){
		goto	failed;
	}

	lseek(fd, sizeof(binp -> binhead) + binp->size, 0);

	if(write(fd, &binp->bintail, sizeof (binp -> bintail)) != 
	sizeof(binp->bintail)){

		goto	failed;

	}

	fsync(fd);
	return(0);

failed:

	return(-1);

}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
recovery(){

	struct	bin_t	*nextbin;

	/* 
	 * If getbin() fails,
	 * audit records could be directed only to the "other" bin 
	 */

	if(getbin(&(bins[0])) || (bins[0].size <= 0)){

		curbin = &(bins[0]);
		nextbin = &(bins[1]);

	}
	else if(getbin(&(bins[1])) || (bins[1].size <= 0)){

		curbin = &(bins[1]);
		nextbin = &(bins[0]);

	}
	else {

		if(bins[0].binhead.bin_time <= bins[1].binhead.bin_time){

			curbin = &(bins[0]);
			nextbin = &(bins[1]);

		}
		else {

			curbin = &(bins[1]);
			nextbin = &(bins[0]);

		}

		/* 
		 * Run recovery on the older bin 
		 */

		if(fixbin(curbin)){

			fail(errno, "%s%s", 
			MSGSTR(ERRRECOVER, "** cannot run recover on bin file"),
			curbin->name);

		}
		runcmds (curbin, 1);
	}
	binreset(curbin);

	/* 
	 * Direct auditing to the older bin
	 * (which is now available) 
	 */

	if(auditbin(0, curbin -> fd, -1, 0)){

		fail(errno,
		MSGSTR(ERRRCVRBIN, 
		"** auditbin() system call failed during recovery")
		);
	}

	/* 
	 * Run recovery on the "new" bin 
	 */

	if(getbin(nextbin) == 0){

		if(fixbin(nextbin) == 0){

			runcmds(nextbin, 1);

		}
	}

	binreset(nextbin);

}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
runcmds(struct bin_t *bin, int rflag){

	FILE	*fp;
	char	line[BUFSIZ];
	char	cmd[BUFSIZ];

	if(bin->size <= 0){

		return (0);

	}

	if((fp = fopen (cmd_file, "r")) == NULL){

		fail(errno, "%s\"%s\"", 
		MSGSTR(ERRBACKEND, 
		"** cannot open backend command file "), 
		cmd_file);

	}

	while(1){

		char	*from, *to;

		if(fgets (line, sizeof (line), fp) == NULL)break;

		from = line;
		to = cmd;
		while(1){
			char	c;

			c = *from++;
			if ((c == '\n') || (c == '\0'))break;

			if((c == '$') && (strncmp (from, "bin", 3) == 0)){
				char	*p;

				from += 3;

				if(rflag){

					*to++ = '-';
					*to++ = 'r';
					*to++ = ' ';

				}

				for(p = bin->name; *p; p++){

					*to++ = *p;
				}
				continue;
			}

			else if((c == '$') && 
			(strncmp (from, "trail", 5) == 0)){

				char	*p;

				from += 5;
				for(p = trail_file; *p; p++){

					*to++ = *p;
				}
				continue;
			}
			*to++ = c;
		}

		*to = 0;
		if(system(cmd)){

			fclose (fp);

			fail(errno, "%s%s", 
			MSGSTR(ERRBACKCMD, "** failed backend command "), 
			cmd);

		}
	}

	fclose (fp);
	return (0);
}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
lock(char *f){

	struct	flock	flock;

	if((lockfd = creat(f, 0600)) < 0){

		if(errno == EAGAIN){

			fail(errno,
			MSGSTR(ERRDUPBIN, "** duplicate auditbin exiting"),
			0);

		}

		fail(errno, "%s\"%s\"", 
		MSGSTR(ERRNOLOCK, "** cannot create lock file "), 
		f);

	}

	flock.l_type = F_WRLCK;
	flock.l_whence = flock.l_start = flock.l_len = 0;

	/* 
	 * Do this twice since
	 * the old auditbin may be
	 * cleaning up in the special 
	 * case of "audit start", "audit off", "audit start"
	 */

	if(fcntl(lockfd, F_SETLK, &flock) < 0){

		if(fcntl(lockfd, F_SETLK, &flock) < 0){

			fail(errno,
			MSGSTR(ERRDUPBIN, "** duplicate auditbin exiting"),
			0);

		}
	}

}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int 
unlock(int fd){

	struct	flock	flock;

	flock.l_type = F_WRLCK;
	flock.l_whence = flock.l_start = flock.l_len = 0;

        if(fcntl(fd, F_GETLK, &flock) >= 0){

		if(flock.l_type == F_UNLCK){

			/* 
			 * No one else owns this lock - 
			 * OK to remove the file
			 */

			unlink(AUDIT_LOCK_FILE);

		}

	}
}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int
fail(int doperror, ...){

	va_list	ap;
	char	*msg;
	static char *ttydev = "/dev/console";
	int	fd;
	int	i;
	int	save_errno = errno;

	va_start(ap, doperror);
	msg = va_arg(ap, char *);

	/*
	 * Check to see if we have been backgrounded yet.  If we are still
	 * running in the foreground, we can use the TTY we were started
	 * from.
	 */ 

	if (! background)
		ttydev = orig_tty;

	/*
	 * Open the terminal as the error device.  We gave up all
	 * access to the TTY we were started on, so we must get
	 * access back to some device.  If the open is successful,
	 * the file descriptor will be dup'd to 2 so this will
	 * become standard error.
	 */

	if ((fd = open (ttydev, O_WRONLY|O_NDELAY|O_NOCTTY)) >= 0)
		dup2 (fd, 2);

	/*
	 * Every 60 seconds as long as auditing is still enabled, output
	 * the error message we were given.  The only exception is if we
	 * are the in the background, then we use the original device and
	 * exit after the first go.
	 */

	while(1){
		fprintf (stderr, "\n%s",
		MSGSTR(AUDITBIN, 
		"auditbin:  ")
		);

		vfprintf (stderr, msg, ap);
		fprintf (stderr, "\n");
		fflush (stderr);

		if(doperror){

			errno = doperror;
			perror ("auditbin:");

		}

		if(!background){
			exit(save_errno);
		}

		fprintf (stderr, "%s%d%s\n",
		MSGSTR(KILLBIN1, 
		"auditbin: kill process "), getpid(),
		MSGSTR(KILLBIN2, 
		" and check filesystem before restarting <auditbin>")
		);

		fflush (stderr);

		/*
		 * Sleep for a minute to give the operator time to fix
		 * the problem and either kill us or turn off auditing.
		 */

		for (i = 0;i < 60;sleep (1)) {

			/*
			 * Check the audit status.  If the user has turned
			 * auditing off, it would be nice if we exited.
			 */

			if (audit (AUDIT_QUERY, 0) == AUDIT_OFF)
				exit (save_errno);
		}
	}
	va_end(ap);
	exit(save_errno);
}


/*                                                                              
 * NAME: 
 *                                                                             
 * FUNCTION:                                                                   
 *
 * EXECUTION ENVIRONMENT:                                                      
 *
 * NOTES:                                                                       
 * 	USAGE:                    
 *      INPUT:                                                                
 *      OUTPUT:                                                               
 * 	FILES:                                                                
 *                                                                             
 * ERRORS:                                                                     
 *
 * TYPE:                                                                        
 *
 * (RECOVERY OPERATION:) Information describing both hardware and              
 *  software error recovery.                                                    
 *                                                                             
 * DATA STRUCTURES: Effects on global data structures, similar to NOTES.       
 *                                                                             
 * RETURNS:                                                                    
 *
 */                                                                            
static int
binreset(struct bin_t *bin){

	ftruncate (bin->fd, 0); 
	fsync(bin->fd); 
	lseek(bin->fd, 0, 0); 

}
