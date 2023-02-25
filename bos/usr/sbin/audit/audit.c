static char sccsid[] = "@(#)16  1.23  src/bos/usr/sbin/audit/audit.c, cmdsaud, bos41J, 9520A_a 5/9/95 11:08:49";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: audit command
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"stdio.h"
#include	"usersec.h"
#include	"audutil.h"
#include	"sys/audit.h"
#include	"sys/auditk.h"
#include	"sys/errno.h"
#include	"sys/priv.h"
#include        "locale.h"
#include	"fcntl.h"

DEBUGNAME("AUDIT")

#include "audit_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDIT,n,s)

#define	MAXREC		4096
#define	MAXATR		512
#define	AUDITDIR	"/audit"
#define	AUDIT_LOCK_FILE	"/audit/auditb"

static struct CmdTable {

	char *fullname;
	int (*func)();

};

static int AuditOn(int, char **);
static int AuditOff(int, char **);
static int AuditStart(int, char **);
static int AuditShut(int, char **);
static int AuditQuery();

static struct CmdTable AuditCmdTable[] ={

	{ "on", AuditOn },
	{ "off", AuditOff },
	{ "query", AuditQuery },
	{ "start", AuditStart },
	{ "shutdown", AuditShut },
	{ NULL, NULL }

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
main(int argc, char *argv[]){

	int	tblidx;

        setlocale(LC_ALL, "");
	catd = catopen(MF_AUDIT, NL_CAT_LOCALE);

	/* 
	 * Acquire privilege
	 */

	if(privcheck(AUDIT_CONFIG)){
		errno = EPERM;

		exit1 (0, 
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

	if((tblidx = GetCommandIndex(argv[1], AuditCmdTable)) < 0){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(USAGE, 
		"Usage: audit {on|off|query|start|shutdown} args"), 
		0);

	}

	(*AuditCmdTable[tblidx].func)(--argc, &argv[1]);

	/*
	 * Resume auditing
	 */

	auditproc(0, AUDIT_STATUS, AUDIT_RESUME, 0);

	exit (0);
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
GetCommandIndex(char *cmd, struct CmdTable tbl[]){

	int	count = 0;
	int	cmdlen;
	int	match;
	struct CmdTable *audtbl;
	cmdlen = strlen (cmd);

	for(audtbl = &tbl[0]; audtbl->fullname; audtbl++){
		if(strncmp (cmd, audtbl -> fullname, cmdlen) == 0){
			match = audtbl - &tbl[0];
			count++;
		}
	}

	return (count == 1 ? match : -1);
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
static 
int
AuditOn(int argc, char *argv[]){

	int	rc;
	int	mode = AUDIT_NO_PANIC;

	if(argc > 2){

		errno = EINVAL;

		exit1 (0, 
		MSGSTR(USAGEON, 
		"Usage: audit on [panic]"), 
		0);

	}

	if(argc == 2){

		if(strcmp(argv[1], "panic") != 0){
			errno = EINVAL;

			exit1(errno, 
			MSGSTR(USAGEON, 
			"Usage: audit on [panic]"), 
			0);

		}
		mode = AUDIT_PANIC;
	};

	if((rc = audit (AUDIT_QUERY, NULL)) < 0){

		exit1(1, 
		MSGSTR(ERRREAD, 
		"** cannot read auditing status"), 
		0);

	}

	if(rc & AUDIT_ON){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(WASON, 
		"** auditing enabled already"), 
		0);

	}

	if(audit(AUDIT_ON, mode) < 0){

		exit1(1, 
		MSGSTR(ERRENABLE, 
		"** cannot enable auditing"), 
		0);

	}

	fprintf (stdout,"%s\n",
	MSGSTR(ENABLED, "auditing enabled"));

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
static 
int
AuditOff(int argc, char *argv[]){

	int	rc;

	if(argc > 1){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(USAGEOFF, 
		"Usage: audit off"), 
		0);
	}

	if((rc = audit (AUDIT_QUERY, NULL)) < 0){

		exit1(1, 
		MSGSTR(ERRREAD, 
		"** cannot read auditing status"), 
		0);

	}

	if(rc == AUDIT_OFF){

		errno = EINVAL;

		exit1(errno, 
		MSGSTR(WASOFF, 
		"** auditing disabled already"), 
		0);
	}

	if(audit(AUDIT_OFF, NULL) < 0){

		exit1 (1, 
		MSGSTR(ERRDISABLE, 
		"** cannot disable auditing"), 
		0);
	}

	fprintf (stdout,"%s\n",
	MSGSTR(DISABLED, "auditing disabled"));
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
static 
int
AuditStart(int argc, char *argv[]){

	int	mode;
	int 	rc;
	char	*temp;
	char 	*SaveClasses;
	struct	stat	stbuf;
	char	errmsg[130];		/* for composing err msg text */
					/* before passing to exit1() */

	if(argc > 1){

		errno = EINVAL;

		exit1(errno, 
		MSGSTR(USAGESTART, 
		"Usage: audit start"), 
		0);

	}

	if((rc = audit(AUDIT_QUERY, NULL)) < 0){

		exit1 (1, 
		MSGSTR(ERRREAD, 
		"** cannot read auditing status"), 
		0);

	}

	/* 
	 * If audit is on reject
	 */

	if(rc & AUDIT_ON){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(WASON, 
		"** auditing enabled already"), 
		0);
	}

	/* 
	 * Do a reset now in case
	 * someone has done an "audit start",
	 * "audit off", "audit start".
	 */

	if(audit(AUDIT_RESET, 0) < 0){
	
		exit1 (1, 
		MSGSTR(ERRRESET, 
		"** cannot reset auditing"), 
		0);
	
	}

	/*
	 * Create audit directory if necessary
	 */

	if (stat(AUDITDIR, &stbuf) < 0)
	{
		if (mkdir(AUDITDIR, 0770) < 0) 
			exit1 (0, MSGSTR(ERRMKDIR, 
					"** cannot make /audit directory"), 0); 
	}
	else if (!S_ISDIR(stbuf.st_mode))
			exit1 (0, MSGSTR(ERRNOTDIR, 
					"** /audit is not a directory"), 0); 


	/*
	 * Update user's data base 
	 * of audit classes per user
	 */

	if(AuditSetClasses() < 0){

		/* 
		 * Failed: 
		 * Reset and exit 
		 */

		if(audit(AUDIT_RESET, 0) < 0){
	
			exit1 (1, 
			MSGSTR(ERRRESET, 
			"** cannot reset auditing"), 
			0);
	
		}

		exit1(0, 
		MSGSTR(ERRSOBJS, 
		"** failed setting kernel audit objects"), 
		0);

	}

	/* 
	 * enable object auditing 
	 */

	if(AuditSetObjects() < 0){

		/* 
		 * Failed: 
		 * Reset and exit 
		 */

		if(audit(AUDIT_RESET, 0) < 0){
	
			exit1 (1, 
			MSGSTR(ERRRESET, 
			"** cannot reset auditing"), 
			0);
	
		}

		exit1(0, 
		MSGSTR(ERRSOBJS, 
		"** failed setting kernel audit objects"), 
		0);

	}

	if(AuditSetEvents() < 0){

		/* 
		 * Failed: 
		 * Reset and exit 
		 */

		if(audit(AUDIT_RESET, 0) < 0){
	
			exit1 (1, 
			MSGSTR(ERRRESET, 
			"** cannot reset auditing"), 
			0);
	
		}

		exit1(1, 
		MSGSTR(ERRSEVNTS, 
		"** failed setting kernel audit events"), 
		0);

	}

	/* 
	 * enable stream auditing 
	 */

	if(AuditSetStream() < 0){

		/* 
		 * Failed: 
		 * Reset and exit 
		 */

		if(audit(AUDIT_RESET, 0) < 0){
	
			exit1 (1, 
			MSGSTR(ERRRESET, 
			"** cannot reset auditing"), 
			0);
	
		}

		exit1(0, 
		MSGSTR(ERRSTREAM, 
		"** failed running stream commands"), 
		0);

	}


	/* 
	 * encoding of <mode> is:
	 * 0 - start auditing
	 * 1 - start bin manager and start auditing
	 * 2 - start bin manager and start auditing in panic mode 
	 */

	mode = 0;
	temp = getattr(PATH_CONFIG, STZ_START, KWD_BINMODE);

	if(temp){

		if(strcmp (temp, "on") == 0){
			mode = 1;
		}
		else if (strcmp (temp, "panic") == 0){
			mode = 2;
		}

	}
	else {

	/* compose the text for the err msg in one string */

		sprintf(errmsg, "%s\"%s\"%s\"%s\"%s\"%s\"",
		MSGSTR(ERRKWD1, "** cannot find "), KWD_BINMODE,
		MSGSTR(ERRKWD2, " keyword in "), STZ_START,
		MSGSTR(ERRKWD3, " stanza in "), PATH_CONFIG);

		errno = EINVAL;

		exit1 (0, "%s", errmsg);

	}

	if(mode){

		/* 
		 * start auditbin daemon 
		 */

		if(fork() == 0){

			/* 
			 * child 
			 */

			execl(PROG_AUDITB, "auditbin", 0);

			exit1(1, "%s%s",
			MSGSTR(ERRBIN, 
			"** cannot exec "), 
			PROG_AUDITB);

		}

		else {
			int	status;

			/* 
			 * parent 
			 */

			wait(&status);
			if(status){

				exit1(0, 
				MSGSTR(ERRSTART, 
				"audit not started"), 
				0);

			}
		}
	}

	/* 
	 * ALWAYS start auditing 
	 */

	if(audit(AUDIT_ON, (mode == 2) ? AUDIT_PANIC : AUDIT_NO_PANIC) < 0){

		exit1(1, 
		MSGSTR(ERRENABLE, 
		"** cannot enable auditing"), 
		0);

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
static 
int
AuditShut(int argc, char *argv[]){
	struct	flock	flock;
	int	timeout;
	int	fd;
	pid_t	audit_bin;

	if(argc > 1){
		errno = EINVAL;

		exit1(errno, 
		MSGSTR(USAGESHUT, 
		"Usage: audit shut"), 
		0);

	}

	/*
	 * See if there is a bin manager currently running.  We must wait
	 * for that bin manager to exit so that starting auditing right
	 * away will not cause that bin manager to linger and the new
	 * bin manager to die with an error.
	 */

	flock.l_type = F_WRLCK;
	flock.l_whence = flock.l_start = flock.l_len = 0;

	if ((fd = open (AUDIT_LOCK_FILE, O_RDONLY)) != -1) {
		if(fcntl(fd, F_GETLK, &flock) >= 0){
			if(flock.l_type != F_UNLCK)
				audit_bin = flock.l_pid;
			else
				audit_bin = (pid_t) 0;

		}
	}

	/*
	 * Stop producing audit records.  A bin manager will see this
	 * and exit once all bins have been processed.
	 */

	if(audit(AUDIT_RESET, 0) < 0){

		exit1 (1, 
		MSGSTR(ERRRESET, 
		"** cannot reset auditing"), 
		0);

	}

	fprintf (stdout,"%s\n",
	MSGSTR(RESET, "auditing reset"));

	/*
	 * The process "audit_bin" is holding the audit bin lock.  We must
	 * wait until that bin process exits because that is when the
	 * auditing has really stopped.
	 */

	if (audit_bin) {
		for (timeout = 0;timeout < 100 && (kill (audit_bin, 0) == 0 ||
				errno == EPERM);timeout++)
			usleep (10000);
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
int
AuditSetClasses(){

	AFILE_t	af;
	ATTR_t	at;
	ATTR_t	atmp;
	struct stat stbuf;

	if( stat(PATH_CONFIG, &stbuf) ||
	   (af = AFopen(PATH_CONFIG, stbuf.st_size, stbuf.st_size)) == NULL){

		perror (PATH_CONFIG);
		goto error;

	}

	if((at = AFgetrec (af, "users")) == NULL){

		errno = EINVAL;
		fprintf (stderr, "%s\"%s\"%s\"%s\"\n",
		MSGSTR(ERRSTANZA1, "** cannot find "), "users",
		MSGSTR(ERRSTANZA2, " stanza in "), PATH_CONFIG);
		goto error;

	}

	/* 
	 * First one is "users" 
 	 */

	at++;

	/* 
	 * Update user's auditclasses
	 */

	for(atmp = at; atmp->AT_name; atmp++){

		/* 
		 * Put in user data base
	  	 */

        	if(putuserattr(atmp->AT_name, S_AUDITCLASSES,
		atmp->AT_value, SEC_LIST) < 0){

			goto error;

        	}

		/* 
		 * Commit to disk
	  	 */

        	if(putuserattr(atmp->AT_name, NULL, NULL,
		SEC_COMMIT)){

			goto error;

		}

	}

error:

	if(af){

		AFclose(af);

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
int
AuditSetObjects(){

	AFILE_t	af;
	ATTR_t	at;
	ATTR_t	atmp;
	int	objtot;
	int	evtot;
	int	j;
	struct 	o_event *ao;
	struct 	o_event *aobuf;
	char 	*nambuf;
	char 	*namorg;
	struct stat stbuf;

	nambuf = namorg = NULL;
	aobuf = ao = (struct o_event *) NULL;

	if( stat(PATH_OBJECT, &stbuf) ||
	   (af = AFopen(PATH_OBJECT, stbuf.st_size, stbuf.st_size)) == NULL){

		/* 
		 * reset any previously defined objects 
		 */

		if(auditobj(AUDIT_SET, ao, 0) < 0){
			goto error;
		}
		return (0);
	}

	objtot = evtot = 0;
	for(at = AFnxtrec(af); at; at=AFnxtrec(af)){
		objtot++;
		at++;
		for(atmp = at; atmp->AT_name; atmp++){
			evtot++;
		}
	}
	AFrewind(af);

	/* 
	 * if the file was empty, return 
	 */

	if(!objtot)return(0);
	
	/* 
	 * aobuf will contain the o_events structures 
	 */

	aobuf = ao = (struct o_event *)malloc(objtot*sizeof(struct o_event));
	
	/* 
	 * nambuf will contain all of the 
	 * object names and event names 
	 */

	namorg = nambuf = (char *)malloc((objtot*MAX_PATHSIZ)+
	(evtot*MAX_EVNTSIZ));

	/* 
	 * Read through each 
	 * object stanza 
	 */

	for(at = AFnxtrec(af); at; at = AFnxtrec(af), ao++){

		ao->o_type = AUDIT_FILE;

		/* 	
		 * Copy object name into buffer 
		 */

		strncpy(nambuf, at->AT_value, MAX_PATHSIZ);
		ao->o_name = nambuf;
		nambuf += MAX_PATHSIZ;

		/* 
		 * Zero out event array  
 		 */

		for(j = 0; j < MAX_EVNTNUM; ao->o_event[j] = 0, j++);

		/* 
		 * Stuff o_event structs 
		 */

		for(atmp = ++at; atmp->AT_name; atmp++){

			switch (*(atmp->AT_name)) {

				case 'r':
					strncpy(nambuf, atmp->AT_value, 
					MAX_EVNTSIZ);

					ao->o_event[AUDIT_READ] = nambuf;
					nambuf +=MAX_EVNTSIZ;
					continue;

				case 'w':
					strncpy(nambuf, atmp->AT_value, 
					MAX_EVNTSIZ);

					ao->o_event[AUDIT_WRITE] = nambuf;
					nambuf +=MAX_EVNTSIZ;
					continue;

				case 'x':
					strncpy(nambuf, atmp->AT_value, 
					MAX_EVNTSIZ);

					ao->o_event[AUDIT_EXEC] = nambuf;
					nambuf +=MAX_EVNTSIZ;
					continue;

				default:
					errno = EINVAL;
					goto error;
			}
		}
	}

	/* 
	 * Object audit 
	 * system call
	 */

	if(auditobj(AUDIT_SET, aobuf, objtot) < 0){

		auditobj(AUDIT_SET, (struct o_event *)NULL, 0);
		goto error;

	}

done:

	if(af){

		AFclose(af);

	}

	if(namorg){

		free(namorg);

	}

	if(aobuf){

		free(aobuf);

	}
	return (0);

error:
	if(af){

		AFclose(af);

	}

	if(namorg){

		free(namorg);

	}

	if(aobuf){

		free (aobuf);

	}

	exit1(0, 
	MSGSTR(ERRSOBJS, 
	"** failed setting kernel audit objects"), 
	0);

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
static 
int
AuditSetStream(){

	extern	char	*getattr();
	FILE	*fp;
	char	line[BUFSIZ];
	char	cmd[BUFSIZ];
	char	*cmd_file;
	char	*temp;
	char	errmsg[130];		/* for composing err msg text */
					/* before passing to exit1() */


	if(temp = getattr (PATH_CONFIG, STZ_START, KWD_STRMODE)){

		if(strcmp(temp, "off") == 0){
			return(0);
		}
		else if(strcmp(temp, "on") != 0){

				errno = EINVAL;

				exit1(errno, 
				MSGSTR(ERRSTRMODE, 
				"** illegal streammode value"), 
				0);

		}
	}
	else{

	/* compose the text for the err msg in one string */

		sprintf(errmsg, "%s\"%s\"%s\"%s\"%s\"%s\"",
		MSGSTR(ERRKWD1, "** cannot find "), KWD_STRMODE,
		MSGSTR(ERRKWD2, " keyword in "), STZ_START,
		MSGSTR(ERRKWD3, " stanza in "), PATH_CONFIG);

		errno = EINVAL;

		exit1(errno, "%s", errmsg);

	}

	cmd_file = getattr (PATH_CONFIG, STZ_STREAM, KWD_CMDS);
	if(cmd_file == NULL){

	/* compose the text for the err msg in one string */

		sprintf(errmsg, "%s\"%s\"%s\"%s\"%s\"%s\"",
		MSGSTR(ERRKWD1, "** cannot find "), KWD_CMDS,
		MSGSTR(ERRKWD2, " keyword in "), STZ_STREAM,
		MSGSTR(ERRKWD3, " stanza in "), PATH_CONFIG);

		errno = EINVAL;

		exit1(errno, "%s", errmsg);

	}

	if((fp = fopen (cmd_file, "r")) == NULL){

		exit1(1,
		MSGSTR(ERRSTREAM, 
		"** failed running stream commands"), 
		0);

	}

	while(1){

		if(fgets(line, sizeof (line), fp) == NULL){
			break;
		}

		if(system(line)){
			fclose (fp);
			exit1 (1,
			MSGSTR(ERRSTREAM, 
			"** failed running stream commands"), 
			0);
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
static 
int
AuditSetEvents(){

	AFILE_t af;
	ATTR_t at, atmp;
	struct audit_class *ae = NULL;
	int i; 
	int n;
	struct stat stbuf;

	/* 
	 * Get the "classes" stanza 
	 */

	if( stat(PATH_CONFIG, &stbuf) ||
	   (af = AFopen (PATH_CONFIG, stbuf.st_size, stbuf.st_size)) == NULL){

		perror (PATH_CONFIG);
		goto error;

	}

	if((at = AFgetrec (af, KWD_CLASSES)) == NULL){

		errno = EINVAL;
		fprintf (stderr, "%s\"%s\"%s\"%s\"\n",
		MSGSTR(ERRSTANZA1, "** cannot find "), KWD_CLASSES,
		MSGSTR(ERRSTANZA2, " stanza in "), PATH_CONFIG);
		goto error;

	}

	/* 
	 * First one is "classes" 
 	 */

	at++;

	/* 
	 * Count number of classes 
	 */

	n = 0;
	for(atmp = at; atmp -> AT_name; atmp++){

		n++;

	}
	if(n == 0){

		goto done;

	}

	ae = (struct audit_class *)xcalloc(n, sizeof (struct audit_class));

	/* 
	 * Point to classes from auditclasses structure 
	 */

	for(i = 0, atmp = at; i < n; i++, atmp++){

		/* 
		 * Point to (don't copy) name 
		 */

		ae[i].ae_name = atmp->AT_name;

		/* 
		 * Copy data into malloc()'d buffer 
		 */

		ae[i].ae_list = atmp->AT_value;
		ae[i].ae_len = list_len (ae[i].ae_list);

	}

	/* 
	 * Issue system call 
	 */

	if(auditevents(AUDIT_SET, ae, n) < 0){

		perror ("auditevents()");
		goto error;

	}

done:

	if(af){

		AFclose(af);

	}
	if(ae){

		free(ae);

	}
	return (0);

error:
	if(af){

		AFclose (af);

	}
	if(ae){

		free (ae);

	}
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
static 
int
AuditQuery(){

	struct	audit_class	*ae_list;
	struct	audit_class	*ae;
	struct	o_event	*ao_list;
	struct	o_event	*ao;
	struct	flock	lock;
	int	ae_len;
	int	ae_cnt;
	int	ao_len;
	int	ao_cnt;
	int	i;
	int	fd;
	int	pid;

	if((i = audit (AUDIT_QUERY, NULL)) < 0){

		exit1(1, 
		MSGSTR(ERRREAD, 
		"** cannot read auditing status"), 
		0);

	}

	/* 
	 * Print current status of auditing 
	 */

	fprintf (stdout,"%s", 
	MSGSTR(AUDITING, "auditing ")
	);

	if(i & AUDIT_ON){

		fprintf (stdout,"%s", 
		MSGSTR(ON, "on")
		);

		if(i & AUDIT_PANIC){

			fprintf (stdout,"%s", 
			MSGSTR(PANIC, "[panic]")
			);

		}
	}

	else {

		fprintf (stdout,"%s", 
		MSGSTR(OFF, "off")
		);

	}

	fprintf(stdout, "\n");

	/* 
	 * Show process ID of bin manager 
	 * This is done by checking the pid
	 * of the owner of the lock file
	 */

	pid = 0;

	if((fd = open ("/audit/auditb", O_RDONLY)) >= 0){

		lock.l_type = F_WRLCK;
		lock.l_whence = lock.l_start = lock.l_len = 0;
		if(fcntl(fd, F_GETLK, &lock) >= 0){
			if(lock.l_type != F_UNLCK)
				pid = lock.l_pid;
		}
		close (fd);
	}

	if(pid == 0){

			fprintf (stdout,"%s\n", 
			MSGSTR(BINOFF, "bin processing off"));

	}
	else {

			fprintf (stdout,"%s%d\n", 
			MSGSTR(BINON, 
			"audit bin manager is process "),
			pid);

	}

	/* 
	 * Show audit events defined to kernel 
	 */

	fprintf(stdout, "%s\n",
	MSGSTR(AUDEVENTS, "audit events:"));

	if((ae_cnt = auditevents(AUDIT_GET, &ae_len, sizeof(ae_len))) < 0){

		if(errno != ENOSPC){

			exit1(1, 
			MSGSTR(ERRGEVNTS, 		
			"** failed getting kernel audit events"), 
			0);

		}
	}

	if((ae_cnt == 0) || (ae_len == 0)){

		fprintf (stdout, "\t%s\n",
		MSGSTR(NONE, "none")
		);

	}
	else {

		ae_list = (struct audit_class *) xalloc(ae_len);
		ae_cnt = auditevents (AUDIT_GET, ae_list, ae_len);

		if(ae_cnt < 0){

			exit1(1, 
			MSGSTR(ERRGEVNTS, 
			"** failed getting kernel audit events"), 
			0);

		}
		
		for(ae = ae_list; ae < &(ae_list[ae_cnt]); ae++){
			char	*p;
	
			fprintf(stdout, "\t%s - ", ae->ae_name);
			list_print (stdout, ae->ae_list);
			fprintf (stdout, "\n");
		}
	}

	/* 
	 * Show object events defined to kernel 
	 */

	fprintf(stdout, "\n%s\n",
	MSGSTR(AUDOBJECTS, "audit objects:")
	);

	if((ao_cnt = auditobj(AUDIT_GET, &ao_len, sizeof (ao_len))) < 0){

		if(errno != ENOSPC){

			exit1(1, 
			MSGSTR(ERRGOBJS, 
			"** failed getting kernel audit objects"), 
			0);

		}
	}

	if((ao_cnt == 0) || (ao_len == 0)){

		fprintf (stdout, "\t%s\n",
		MSGSTR(NONE, "none")
		);

	}
	else {

		ao_list = (struct o_event *) xalloc(ao_len);
		ao_cnt = auditobj (AUDIT_GET, ao_list, ao_len);

		if(ao_cnt < 0){

			exit1(1, 
			MSGSTR(ERRGOBJS, 
			"** failed getting kernel audit objects"), 
			0);

		}
		
		for(ao = ao_list; ao < &(ao_list[ao_cnt]); ao++){
			char	*p;
	
			fprintf (stdout, "\t%s:\n", ao->o_name);

			if(ao->o_event[AUDIT_READ]){

				fprintf (stdout, "\t\t r = %s\n", 
				ao->o_event[AUDIT_READ]);

			}

			if(ao->o_event[AUDIT_WRITE]){

				fprintf (stdout, "\t\t w = %s\n", 
				ao->o_event[AUDIT_WRITE]);

			}

			if(ao->o_event[AUDIT_EXEC]){

				fprintf(stdout, "\t\t x = %s\n", 
				ao->o_event[AUDIT_EXEC]);

			}

		}
	}
	return;
}
