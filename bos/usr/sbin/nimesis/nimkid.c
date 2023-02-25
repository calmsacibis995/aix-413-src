/* @(#)46	1.12  src/bos/usr/sbin/nimesis/nimkid.c, cmdnim, bos411, 9428A410j  6/14/94  16:31:47 */

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: CpuidChk
 *		MachChk
 *		Method_req
 *		flushrcmd
 *		getPort
 *		getSockTkn
 *		nackRcmd
 *		nimkid
 *		reg_req
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "cmdnim.h" 
#include "cmdnim_db.h" 
#include "nimesis.h" 
#include <sys/wait.h>

/* ---------------------------- CpuidChk 
 *
 * NAME:	 CpuidChk
 *
 * FUNCTION:
 *	Test to see if this cpu id passed to us is valid for this machine.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			InPacket : The next field from the stream.
 *			id       : The identifier for the machine nim_object
 *
 * RETURNS:
 *		OK		: The machine name may be new ?
 *		SUCCESS		: The cpu id matches.
 *		FAILURE		: cpu id mis-match / zero or ODM had an error.
 *
 * ---------------------------------------------------------------------------*/


CpuidChk ( char *in_packet, 
	   long id)
{ 
	int 	attr_exists;
	char	theId[MAX_VALUE]="\0"; 
	struct  nim_attr *cpuid;
	struct  listinfo info;

	/* 
	 * Get the CPU id for the machine in question
	 */
	attr_exists=get_attr(&cpuid, &info, id, NULL, 0, ATTR_CPUID);
		
	/* 
	 * Let nim_append copy and filter the in_packet
	 */
	nimappend(theId, in_packet);

	/* 
	 * If the cpu id is zero then this is an error
	 */
	if (strcmp(theId, "000000000000") == 0) {
		buglog("NIMkid(%d) : theId is zero", getpid());
		return(FAILURE);
	}
	if (attr_exists == 1) {
		/*
	 	 * Compare the ODM cpu_id, if its zero then update the odm 
	 	 * object with this new cpu_id. 
	 	 */
		if (strcmp(cpuid->value, "000000000000") == 0) {
			/* 
			 * Yep its zero, so remove it and fall through and add the
			 * real one from the machine. 
			 */
			if ( rm_attr( id, NULL, ATTR_CPUID, 0 ) == FAILURE ) {
				buglog("NIMkid  (%d) : rm_attr failure", getpid());
				return(FAILURE);
			}
		}
		else 
			if ( strcmp(cpuid->value, theId) == 0) {
				buglog("NIMkid  (%d) : CPU ID - ok ", getpid());
				return(SUCCESS);
			}
			else {
				buglog("NIMkid  (%d) : %s != %s", getpid(), cpuid->value, theId);
				 return(FAILURE);
			}
	}
	return( mk_attr(id, NULL, theId, 0, ATTR_CPUID, ATTR_CPUID_T)  );

} /* end of CpuidChk */

/* ---------------------------- MachChk 
 *
 * NAME:	 MachChk
 *
 * FUNCTION:
 *	Query the ODM to see if this is a valid machine object name.		
 *
 * DATA STRUCTURES:
 *		parameters:
 *			InPacket : The next field from the stream.
 *
 * RETURNS:
 *		SUCCESS		: The name exists.
 *		FAILURE		: the name does not exist or ODM had an error.
 *
 * ---------------------------------------------------------------------------*/

int
MachChk ( char *InPacket )
{
	char	name[MAX_TMP] = "";

	nimappend(name, InPacket);

	return(get_id(name)); 
} 



/* ---------------------------- nackCmd 
 *
 * NAME:	 nackCmd
 *
 * FUNCTION:	Send a "NACK" to the remote rcmd. 
 * 
 * DATA STRUCTURES:
 *		parameters:
 *			Sd	Socket Descriptor
 *			Msg	A meaningful message.
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *
 *
 * --------------------------------------------------------------------------
 */

nackRcmd(int Sd, char *Msg)
{ 
 	/* 
  	 * NACK the rcmd
  	 */
 	write( Sd, "1", sizeof(char) ); 
 	write( Sd, Msg, strlen(Msg) );
	buglog("NIMkid  (%d) : NACK [%s]", getpid(), Msg);
	return(SUCCESS);

}
/* ---------------------------- getSockTkn 
 *
 * NAME:	 getSockTkn
 *
 * FUNCTION:	Get the next token (null terminated) from the socket. 
 * 
 * DATA STRUCTURES:
 *		parameters:
 *			Sd	Socket Descriptor
 *			Ptr	The address of the buffer to place result
 *			Sz 	Size of the buffer	
 * RETURNS:
 *	OK		-  Hit end of file.
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 *
 * --------------------------------------------------------------------------*/

 getSockTkn(int Sd, char *Ptr, int Sz)
 {
	int rc;
	int spcLeft=Sz;

	/*  
    	 * While we read 1 char and we do not get fatal errno and we 
	 * have not run-over the buffer and we have not got to the
	 * end of the string and we have not reached EOF... LOOP 8-)
	 */
	while (1) { 
		rc = read(Sd, Ptr, 1);
 		if (rc == -1) {
			if (errno == EINTR)
				continue;
			else {
				nene( ERR_ERRNO_LOG, "getSockTkn", "read", NULL);
				return(FAILURE);
			}
		}
		if ( rc == 0 ) 	/* EOF */
			return(OK);

		if (*Ptr++ == '\0' )
			break;

		if ( --spcLeft == 0 ) {
	   		nene( ERR_BUFF_OVER, "getSockTkn", NULL);
	   		return(FAILURE);
		}
	}	
	return(SUCCESS);
 }


/* ---------------------------- getPort 
 *
 * NAME:	 getPort
 *
 * FUNCTION:	Get the secondary port number from the client rcmd. 
 * 
 * 
 * DATA STRUCTURES:
 *		parameters:
 *			Sd	Socket descriptor
 *			Port	The returned port number
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 *
 * ---------------------------------------------------------------------------*/

int
getPort( int 	Sd, 
	 short 	*Port )
{
	char	tmp_buff[MAX_TMP];
	char	*tbPtr;
	int	rc=SUCCESS;

	/* 
	 * Set-up a time out so we don't sit here forever if the 
	 * client goes on vacation.
	 */
	tbPtr=tmp_buff;
	alarm(120);
	rc = getSockTkn(Sd, tbPtr, sizeof(tmp_buff));
	alarm(0);
	buglog("NIMkid  (%d) : Secondary Port [%s]", getpid(), tmp_buff);
	if ( rc  != SUCCESS )
		return(FAILURE);
	*Port=atoi(tmp_buff);
	return(rc);
}

/* ---------------------------- flushrcmd 
 *
 * NAME:	 flushrcmd
 *
 * FUNCTION:	We are not using all the rcmd stuff, so this will flush the 
 *		rcmd specific protocol.
 * 
 * DATA STRUCTURES:
 *		parameters:
 *			Sd	Socket Descriptor
 *
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 *
 * ---------------------------------------------------------------------------*/

int
flushrcmd ( int Sd )
{
	char	tmp_buff[MAX_TMP];
	char	*tbPtr;
	int	count;
	int	rc=SUCCESS;

	/* 
	 * Set-up a time out so we don't sit here forever if the 
	 * client goes on vacation.
	 */
	alarm(180);
	for ( count=0; (count != 3 && rc != FAILURE); count++ ) {
		tbPtr=tmp_buff;
		rc = getSockTkn(Sd, tbPtr, sizeof(tmp_buff));
	}
	/*
	 * Do not forget to shutdown the timer... 
	 */
	alarm(0);
	buglog("NIMkid  (%d) : RCMDflush", getpid());
	return(rc);
}

/* ---------------------------- nimkid 
 *
 * NAME:	 nimkid
 *
 * FUNCTION:  This function processes requests from NIM clients. 
 * 
 * EXECUTION ENVIRONMENT:
 *	This is the first function executed in the child process.
 *
 * DATA STRUCTURES:
 *	parameters:
 *		InMsg 	    - The connected socket of a requesting client.
 *		reg_request - Flag that sez if this is a registration.
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 * ---------------------------------------------------------------------------*/

int
nimkid ( NIM_SOCKINFO 	*InMsg, 
	 int 		reg_request)
{

extern	int 	MachChk(), CpuChk();
extern	int	debug; 

	NIM_FIELDS  which_field=NIM_PKT_NAME; 
	FILE	*sockFP, *sout, *fp;

	char	InPacket[MAX_NIM_PKT];
	char	*Args[METH_MAX_ARGS];

	NIM_SOCKINFO 	clientInfo;
	int	addrSize = sizeof(clientInfo.addr);

	int	rc=SUCCESS;
	int	cmdPid;
	int 	auto_reg=0;
	int 	good_req=0;
	int	replyNeeded=0; 
	int	nargs;
	int	argNdx=0;
	int	eod=0;	
	int	mypid=getpid();
	short	clientPort;

	buglog("NIMkid  (%d) : Started ", mypid );
	memset( &clientInfo, 0, sizeof(clientInfo) );
	/* 
	 * Get peer information
	 */
	clientInfo.addr = InMsg->addr;
	buglog("NIMkid  (%d) : Client Address [0x%x]", mypid, InMsg->addr.sin_addr.s_addr);
	
	/*
	 * ------------- Begin of RCMD Protocol Processing ---------------- 
	 */
	 
	if (getPort(InMsg->fd, &clientPort) != SUCCESS) {
		nackRcmd(InMsg->fd, MSG_msg (MSG_SEC_PORT));
		return(FAILURE);
	}
	/* 
	 * The remote client uses stderr to find out command execution 
	 * status.  
	 */
	if ( clientPort ) {
		clientInfo.addr.sin_port = htons(clientPort);
		/* 
		 * MUST use a reserved port, so flag it... 
		 */
		clientInfo.useRes++;
		if (nim_connect(&clientInfo) != SUCCESS) {
			nackRcmd(InMsg->fd, MSG_msg (MSG_CONNECT_PORT));
			return(FAILURE);
		}
	}

	/* 
	 * Flush the remote/local user and command name passed by 
	 * rcmd call. Right now we are sticking with the NIM protocol
	 * that will be sent to us after we ack the client.
	 */
	if (flushrcmd(InMsg->fd) != SUCCESS) {
		nackRcmd(InMsg->fd, MSG_msg (MSG_NIMESIS_READ));
		return(FAILURE);
	}
	/* 
	 * For ease of use, make socket file pointer
	 */
	if ( (InMsg->FP=fdopen(InMsg->fd, "w+")) == NULL ) { 
		/* 
		 * Had an error creating the FILE pointer 
		 */  
		 nene(ERR_ERRNO_LOG,"nimreq","fdopen", NULL);
		/* 
		 * NACK the rcmd
		 */
		nackRcmd(InMsg->fd, MSG_msg (MSG_NIMESIS_FP_CREAT));
		return(FAILURE);
	}

	/* 
	 * ACK the remote CMD to complete the RCMD protocol so we can 
	 * continue with the NIM processing.
	 */
	if ( write( InMsg->fd, "", 1 ) != 1 ) { 
		nene(ERR_ERRNO_LOG, "nimkid", "write", NULL);
		return(FAILURE);
	}
	buglog("NIMkid  (%d) : ACK", mypid);

	/*
	 * --------------- End of RCMD Protocol Processing ---------------- 
	 */
	
	/* 
	 * From this point on in the processing return code values need to be 
	 * directed to the secondary port (rmt's stderr). In the form of 
	 * "rc=<value>\n [opt msg]" in order to magiclly make this happen we dup the 
	 * remotes secondary port fd to 2 (stderr) and update the error policy 
	 * to enable rc= to be written on stderr. Also we divert the stdout...
	 */ 
	if ( dup2(clientInfo.fd, 2) != 2 ) {
		nackRcmd(clientInfo.fd, MSG_msg (MSG_DUP_STDERR));
		return(FAILURE);
	}	
	if ( dup2(InMsg->fd, 1) != 1 ) {
		nackRcmd(clientInfo.fd, MSG_msg (MSG_DUP_STDOUT));
		return(FAILURE);
	}	
	niminfo.err_policy |= (ERR_POLICY_DISPLAY);

	if (reg_request)
		return(reg_req(InMsg, mypid ));
	return(Method_req(InMsg, mypid ));
}

/* ---------------------------- Method_req 
 *
 * NAME:	 Method_req
 *
 * FUNCTION:
 * This function processes method requests from NIM clients. 
 * 
 * DATA STRUCTURES:
 *	parameters:
 *		InMsg 	- The connected socket of a requesting client.
 *		mypid	- Our process id.
 *
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 *
 * ---------------------------------------------------------------------------*/

int
Method_req ( 	NIM_SOCKINFO 	*InMsg, 
		int 		mypid )
{


	NIM_FIELDS  which_field=NIM_PKT_NAME; 
	char	*fldNames[] = { "NIM_NAME", "CPUID", "REPLY","NARGS", "ARG" };
	char	InPacket[MAX_NIM_PKT];
	char	*Args[METH_MAX_ARGS]= {C_NIMPUSH, NULL};

	long	The_Machine_ID; 

	int	rc=SUCCESS;
	int	cmdPid;
 	union wait kidStat;
	int 	good_req=0;
	int	replyNeeded=0; 
	int	nargs;
	int	argNdx=1;
	int	eod=0;	
	
	buglog("NIMkid  (%d) : METHOD", mypid);
	while ( !eod ) {
	/* 
	 * WHILE we are not complete getting the data and whilst we are 
	 * ok with the data integrity and the remote client has correct 
	 * permission DO... 
	 */
		if ( (rc=getSockTkn(InMsg->fd, InPacket, MAX_NIM_PKT)) != SUCCESS )
			break; 
		
		buglog("NIMkid  (%d) : %s [%s]", mypid, fldNames[which_field], InPacket);
		/* 
		 * Stepping down our packet protocol get the next appropriate 
		 * thing and process it. 
		 */
		switch (which_field) { 
			/* 
			 * Validate the machine name
			 */
			case NIM_PKT_NAME: 
				if ( (The_Machine_ID = MachChk(InPacket)) == 0 ) 
					rc=FAILURE;
				else
					which_field++;
			break;
			
			/* 
			 * Validate and optionally update the cpu id
			 */
			case NIM_PKT_CPUID: 
				rc = CpuidChk(InPacket, The_Machine_ID );
				which_field++;
			break;

			/* 
			 * If client expects reply it'll say Yes or No... 
			 */ 
			case NIM_PKT_REPLY: 
				replyNeeded = (toupper(InPacket[0]) == 'Y');
				which_field++;
			break;

			/*
			 * Number of args we can expect... 
			 */
			case NIM_PKT_NARGS:
				if ( (nargs=atoi(InPacket)) < METH_MAX_ARGS )
					which_field++;
				else 
					rc=FAILURE;
			break;

			/* 
			 * The Args.. 
			 */
			case NIM_PKT_ARGS:
				if (nargs--) { 
					if ((Args[argNdx]=malloc(strlen(InPacket)+1 )) == NULL) {
						rc=FAILURE;
						break;
					}
					strcpy(Args[argNdx++], InPacket);
				}
				if (nargs == 0) {
					Args[argNdx] = NULL;
					eod++;
					good_req++;
				}
			break;
		}

		/* 
		 * If something failed then zero out good_req then
		 * break from all this dross... 
		 */
		if (rc != SUCCESS) {
			good_req = 0;
			break;
		}
	} 
	/* 
	 * If we are all ok up to this point then thunder off and execute 
	 * the method .... 
	 */
	if (good_req) {
		if ( (cmdPid=LocalExec(Args)) <= 0 ) { 
			buglog("NIMkid (%d) : exec failure", mypid);
			nene(ERR_ERRNO_LOG, "Method_req", "LocalExec", NULL);
			rc=FAILURE;
 		}
		else { 
			if (waitpid(cmdPid, &kidStat, 0 ) < 0) {
 				nene(ERR_ERRNO_LOG, "Method_req", "waitpid", NULL);
				rc=FAILURE;
			}
			else  
				rc=WEXITSTATUS(kidStat);   
		}
	}
	else {
		buglog("NIMkid  (%d) : REQUEST DENIED ", mypid );
		nene(ERR_REFUSED, "Method_req", NULL, NULL);
	}	
	buglog("NIMkid  (%d) : RC=%d", mypid, rc);
	buglog(" ");
	return(rc);
} /* end of Method_req */
