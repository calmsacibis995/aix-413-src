static char sccsid[] = "@(#)69  1.22  src/bos/usr/sbin/watch/watch.c, cmdsaud, bos411, 9428A410j 4/18/94 13:22:38";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: watch command
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
#include	"fcntl.h"
#include	"errno.h"
#include	"sys/audit.h"
#include	"sys/auditk.h"
#include	"sys/signal.h"
#include	"sys/stat.h"
#include	"sys/ioctl.h"
#include	"audutil.h"
#include	"sys/priv.h"
#include	"ctype.h"   
#include 	"locale.h"
#include 	"sys/resource.h"
#include 	"sys/wait.h"
#include 	"sys/m_wait.h"

DEBUGNAME("WATCH")

#include "watch_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_WATCH,n,s)

/*
 * Forward declarations
 */

static int SetOutput();
static int SetInput();
static int SetAudEvents();
static int SetObjEvents();
static int ProcessFind(int pid);
static int ProcessDel(int pid);
static int ProcessAdd(int);
static int Display(struct aud_rec *, char *);
static int OnIntr();
static int Usage();
static int Quit1(int, char *, char *, ...);
static int ChildCatch();
static int ChildExitNotify();
static int Rd(char *, int);
static int IsEvent(char *, int);
static char *WatchRead(struct aud_rec *);
static int FindEvent(struct aud_rec *);
static int ParseEvents(char *);
static int CleanExit();
static int Watch();

/* 
 * Defines 
 */

#define	TAILBUFSIZ	10024
#define	MAXREC		4096
#define	MAXATR		512

/* 
 * Globals 
 */

FILE	*output;
int	Child;
int	Binary = 0;
int	ProcessGroup = 0;
int	inputfd = 0;
int	*children;
int	nchild = 0;
int	maxchild = 0;
int	nevents = 0;
int	RootHasClass = 0;
char 	TmpClass[] = "TEMP\0";
char 	AllClass[] = "ALL\0";
char 	*Events = NULL;
char	*ProgName;
char	*Output = NULL;
char 	TailBuf[TAILBUFSIZ];
char 	*SaveClasses;
struct {
	char	*buf;
	int	len;
} auditext;


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
Usage(){

	fprintf (stderr, "%s\n",
	MSGSTR(USAGE, 
	"Usage: watch [-e] [-o file] cmd [arg]*")
	);

	exit (EINVAL);
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
Quit1(int doperror, char *msg, char *arg, ...){

	/* 
	 * Reset Classes
	 */

	if(RootHasClass){

		auditproc(0, AUDIT_EVENTS, SaveClasses, list_len(SaveClasses));

	}

	/* 
	 * Reset auditing
	 */

	audit(AUDIT_RESET, AUDIT_NO_PANIC);

	exit1(doperror, msg, arg);
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
OnIntr(){

	int i;

	/* 
	 * Reset Classes
	 */

	if(RootHasClass){

		auditproc(0, AUDIT_EVENTS, SaveClasses, list_len(SaveClasses));

	}

	/* 
	 * Reset auditing
	 */

	audit(AUDIT_RESET, AUDIT_NO_PANIC);

	for(i = 0; i < nchild; i++){

		fprintf(stderr, "%s %d\n",
		MSGSTR(ERRCHILDKILL, 
		"** killing process: "), 
		children[i]);
		kill(children[i], SIGKILL);

	}

	exit(1);
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
ChildCatch(){

	union wait status;
	while(wait3(&status, WNOHANG, 0) > 0);

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
ChildExitNotify(){

	fprintf(stderr, "%s %d\n",
	MSGSTR(ERRCHILDGONE, 
	"** child process exiting: "), 
	Child);

	if(nchild){

		int i;

		for(i = 0; i < nchild; i++){
			fprintf(stderr, "%s %d\n",
			MSGSTR(ERRCHILDCATCH, 
			"** did not track exit of process  "), 
			children[i]);
		}

		fprintf(stderr, "%s \n",
		MSGSTR(ERRSTILLTRCKNG, 
		"** still tracking ")
		); 

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
Rd(char *ptr, int siz){

	int	cnt;

	if(siz > TAILBUFSIZ)return(-1);
	
	cnt = read(inputfd, ptr, siz);

	if(cnt <= 0)return(-1);

	return(0);
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
IsEvent(char *s, int len){

	int	i;
	char	c;

	for(i = 0; 1; i++){ 

		if(i >= len) return (0);
		c = s[i];
		if(c == '\0')break;

		if(!isascii(c)){

			return (0);

		}

	}

	for( ; i < len; i++){
		if (s[i] != '\0') return (0);
	}
	return (1);
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
char *
WatchRead(struct aud_rec *ah){

	if(Rd(ah, sizeof(struct aud_rec)) < 0){
		return(NULL);
	}

	if(!IsEvent(ah->ah_event, sizeof(ah->ah_event))){
		return(NULL);
	}

	/* 
	 * Read next tail 
	 */

	if(ah->ah_length){

		if(Rd(TailBuf, ah->ah_length) < 0){
			return(NULL);
		}

	}

	return(TailBuf);
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
main(int ac, char **av){

	extern	char	*optarg;
	extern	int	optind;

	/* 	
	 * Establish a reasonable environment 
	 */

	kleenup(3, 0, 0);

	setlocale(LC_ALL, "");
	catd = catopen(MF_WATCH, NL_CAT_LOCALE);
	
	if(privcheck(SET_PROC_AUDIT)){ 
		errno = EPERM;

		exit1(EPERM, 
		MSGSTR(ERRPRIV, 
		"** SET_PROC_AUDIT privilege required"), 
		0); 

	}

	signal(SIGINT, OnIntr);
	signal(SIGTERM, OnIntr);
	signal(SIGCHLD, ChildCatch);

	ProgName = av[0];
	while(1){

		int	c;

		c = getopt(ac, av, "bo:e:");

		if(c == EOF)break;

		switch(c){
			case 'b':
				Binary = 1;
				break;
			case 'o':
				Output = optarg;
				break;
			case 'e':
				Events = optarg;	
				break;
			default:
				Usage();
		}
	}

	if(optind >= ac)Usage();

	
	/* 
	 * suspend auditing on ourself 
	 */

	if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)){

		exit1(errno, 
		MSGSTR(ERRSUSPEND, 
		"** cannot suspend auditing"), 
		0); 

	}

	if(audit(AUDIT_QUERY, 0) & AUDIT_ON){

		exit1(EINVAL, 
		MSGSTR(ERRDISABLED, 
		"** auditing must be disabled"), 
		0); 

	}

	SetOutput();

	if(SetObjEvents()){

		Quit1(1, 
		MSGSTR(ERROBJECTS, 
		"** cannot set object events"), 
		0);

	}

	if(SetAudEvents()){

		Quit1(1, 
		MSGSTR (ERREVENTS, 
		"** cannot set audit events"), 
		0);

	}

	inputfd = SetInput();

	/* 
	 * Enable auditing 
	 */

	if((audit(AUDIT_ON, AUDIT_NO_PANIC)) < 0){

		exit1(errno, 
		MSGSTR(ERRENABLED, 
		"** cannot enable auditing"), 
		0); 

	}

	/* 
	 * Query login audit classes for root
  	 */

	if(setuserdb(S_READ) < 0){

		_exit(errno);

	}

	if(getuserattr("root", S_AUDITCLASSES, &SaveClasses, SEC_LIST) < 0){

		RootHasClass = 0;

	}
	else {

		RootHasClass = 1;
	
	}


	Child = xfork();

	/* 
	 * Child 
	 */

	if(Child == 0){

		close(fileno(output));


		/* 
		 * Audit ourself 
		 */

		if(auditproc(0, AUDIT_EVENTS, AllClass, 5) < 0){

			_exit(errno);

		}

		/* 
		 * Enable auditing 
		 */

		if(auditproc(0, AUDIT_STATUS, AUDIT_RESUME, 0) < 0){

			_exit(errno);

		}

		/* reset uid in case we are running watch as a
                   non-root member of the audit group, to avoid
                   enabling the non-root user to exec a program
                   that should not be allowed. */

		setuid(getuid());

		/* 
		 * Run the command 
		 */

		execvp(av[optind], &(av[optind]));
		_exit(errno);
	}

	/* 
	 * Register child 
	 */

	ProcessAdd(Child);

	/* 
	 * Watch child and all spawned processes 
	 */

	Watch();

	CleanExit();
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
SetOutput(){

	int	fd;
	int	p[2];
	int	pid;

	ProcessGroup = setpgrp();

	/* 
	 * Create any output file 
	 */

	if(Output == NULL)fd = dup (1);
	else {

		fd = open (Output, O_WRONLY | O_CREAT, 0640);

	}

	if(fd < 0){

		Quit1(1, 
		MSGSTR(OUTPUT, 
		"** could not open output"), 
		0);

	}

	if(Binary){

		output = fdopen (fd, "w");
		return;

	}

	if(pipe(p)){

		Quit1(1, 
		MSGSTR(PIPE, 
		"** could not open pipe"), 
		0);

	}
	
	pid = xfork();

	/* 
	 * Child 
	 */

	if(pid == 0){

		/* 
	 	 * Child 
		 */

		/* 
		 * Write side of pipe is for parent 
		 */

		close(p[1]);

		/* 
		 * Read side of pipe is stdin 
		 */

		close (0);
		dup (p[0]);
		close (p[0]);

		/* 
	 	 * fd open()'d above is stdout 
		 */

		close (1);
		dup (fd);
		close (fd);

		/* 
		 * fire up <auditpr> 
		 */

		execl("/usr/sbin/auditpr", "auditpr", "-t2", "-m", 
		"***** WATCH *****", "-v", 0);

		_exit (errno);
	}

	/* 
	 * Parent 
	 */

	/* 
	 * Read side of pipe is for child 
	 */

	close (p[0]);

	/* 
	 * fd open()'d above is for child 
	 */

	close (fd);

	/* 
	 * Our output is the write side of the pipe 
	 */

	output = fdopen(p[1], "w");
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
SetInput(){

	int fd;
	auditext.buf = AllClass;
	auditext.len = 5;
	if((fd = openx("/dev/audit", O_RDONLY, 0,&auditext)) < 0){

		Quit1(1, 
		MSGSTR(DEVAUDIT, 
		"/dev/audit"), 
		0);

	}

	return (fd);
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
SetAudEvents(){

	struct  audit_class	ae;

	ae.ae_name = TmpClass;

	if(Events == NULL){

		Events = AllClass;
		ae.ae_list = Events;
		ae.ae_len = 5;

	}
	else {

		if((ae.ae_list = (char *)malloc(strlen(Events)+2)) == NULL){
			perror("SetAudEvents() malloc");
			goto error;
		}
		bzero(ae.ae_list, strlen(Events) + 2);
		bcopy(Events, ae.ae_list, strlen(Events)+1);
		ae.ae_len = strlen(Events) + 2;
		nevents = ParseEvents(Events);
		ParseEvents(ae.ae_list);

	}

	if(auditevents(AUDIT_SET, &ae, 1) < 0){

		perror ("auditevents()");
		goto error;

	}

	free(ae.ae_list);

	return (0);
error:
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
SetObjEvents(){

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

	nambuf = namorg = NULL;
	aobuf = ao = (struct o_event *) NULL;
	if((af = (AFILE_t)AFopen(PATH_OBJECT, MAXREC, MAXATR)) == NULL){

		return (0);

	}

	objtot = evtot = 0;
	for(at = (ATTR_t)AFnxtrec(af); at; at = (ATTR_t)AFnxtrec(af)){

		objtot++;
		at++;
		for(atmp = at; atmp->AT_name; atmp++)evtot++;

	}
	AFrewind(af);

	/* 
	 * If the file was empty, return 
	 */

	if(!objtot)return(0);
	
	/* 
	 * aobuf will contain the o_events structures 
	 */

	aobuf = ao = (struct o_event *)malloc(objtot*sizeof(struct o_event));
	
	/* 
	 * nambuf will contain all of the object names and event names 
	 */

	namorg = nambuf = (char *)malloc((objtot*MAX_PATHSIZ) +
	(evtot*MAX_EVNTSIZ));

	/* 
	 * Read through each object stanza 
	 */

	for(at = (ATTR_t)AFnxtrec(af); at; at = (ATTR_t)AFnxtrec(af), ao++){
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

		for(j = 0; j<MAX_EVNTNUM; ao->o_event[j] = 0, j++);

		for(atmp = ++at; atmp->AT_name; atmp++){
			switch(*(atmp->AT_name)){

				case 'r':
					strncpy(nambuf, atmp->AT_value, 
					MAX_EVNTSIZ);

					ao->o_event[AUDIT_READ] = nambuf;
					nambuf +=MAX_EVNTSIZ;
					continue;
				case 'w':
					strncpy (nambuf, atmp->AT_value, 
					MAX_EVNTSIZ);

					ao->o_event[AUDIT_WRITE] = nambuf;
					nambuf +=MAX_EVNTSIZ;
					continue;
				case 'x':
					strncpy (nambuf, atmp->AT_value, 
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

	if(auditobj(AUDIT_SET, aobuf, objtot) < 0){
		auditobj(AUDIT_SET, (struct o_event *)NULL, 0);
		goto error;
	}

	if(af)AFclose(af);

	if(namorg)free(namorg);

	if(aobuf)free(aobuf);

	return(0);

error:

	if(af)AFclose(af);

	if(namorg)free(namorg);

	if(aobuf)free(aobuf);

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
ProcessFind(int pid){

	int	i;

	for(i = 0; i < nchild; i++){
		if(children[i] == pid)return(i);
	}
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
ProcessAdd(int pid){

	if(nchild >= maxchild)
		if(children == NULL){
			maxchild = 1024;
			children = (int *)xalloc(maxchild * sizeof(int));
		}
		else {
			int	*xralloc();

			maxchild *= 2;
			children = (int *)xralloc(children, 
			maxchild * sizeof(int));

		}

	children[nchild++] = pid;

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
ProcessDel(int pid){

	int	i;

	if((i = ProcessFind(pid)) < 0){
		return;
	}

	for(; i < nchild; i++){
		children[i] = children[i+1];
	}

	nchild--;

	if(pid == Child)ChildExitNotify();

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
Watch(){

	struct	aud_rec	ah;
	char	*tail;

	/* 
	 * Main loop 
	 */

	while(nchild > 0){

		tail = WatchRead(&ah);
		if(tail == NULL)continue;

		if(Display(&ah, tail)){ 
			if(FindEvent(&ah) &&
			(ProcessFind(ah.ah_pid) >= 0)){
				fwrite(&ah, sizeof (ah), 1, output);
				fwrite(tail, ah.ah_length, 1, output);
				fflush(output);
			}
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
Display(struct aud_rec *ah, char *tail){

	if(strncmp(ah->ah_event, "PROC_Create", 11) == 0){
		int	child_pid;

		bcopy(tail, &child_pid, sizeof(int));
		if(ProcessFind(child_pid) < 0)ProcessAdd(child_pid);
		return(0);
	}

	if(strncmp(ah->ah_event, "PROC_Delete", 11) == 0){
		ProcessDel(ah->ah_pid);
		return(0);
	}

	return(1);
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
FindEvent(struct aud_rec *ah){

	char *c = Events;
	int i;

	/* 
	 * Filter watch events 
	 */

	if(strncmp("watch", ah->ah_name, 5) == 0)return(0);

	/* 
	 * All selected 
	 */

	if(!nevents)return(1);

	/* 
	 * Find events 
	 */

	for(i = 0; i < nevents; i++){
		if(strcmp(c, ah->ah_event) == 0)return(1);
		c += strlen(c) + 1;
	}

	return(0);
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
ParseEvents(char *s){

	char *c;
	int number = 0;

	c = strtok(s, ",");
	number++;

	while(c = strtok(NULL, ","))number++;

	return(number);
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
CleanExit(){

	fclose(output);

	/* 
	 * If started <auditpr>, wait for it to complete 
	 */

	if(!Binary){

		while(wait((int*) 0) != -1);

	}

	/* 
	 * Reset Classes
	 */

	if(RootHasClass){

		auditproc(0, AUDIT_EVENTS, SaveClasses, list_len(SaveClasses));

	}

	if(audit(AUDIT_RESET, 0) < 0){

		exit1(errno, 
		MSGSTR(ERRRESET, 
		"** cannot reset auditing"), 
		0); 

	}

	if(nchild <= 0){

		fprintf(stderr, "%s\n",
		MSGSTR(ALLGONE, 
		"** all processes have exited ")
		);

	}
	exit (0);
}
