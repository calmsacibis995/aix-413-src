char	sccs_id[] = " @(#)03 1.12.1.3  src/bos/usr/sbin/nimesis/nimreg.c, cmdnim, bos411, 9435A411a  8/26/94  18:20:58";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: reg_req
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


/*---------------------------- globals    -----------------------------*/
ATTR_ASS_LIST reg_attrs;

VALID_ATTR valid_reg_attrs[] = {
 	{ ATTR_NAME,      	ATTR_NAME_T,		TRUE,	NULL  },
 	{ ATTR_CPUID,		ATTR_CPUID_T,		TRUE,	NULL  },
 	{ ATTR_PIF_NAME,  	ATTR_PIF_NAME_T,	FALSE,  NULL  },
 	{ ATTR_RING_SPEED,	ATTR_RING_SPEED_T,	FALSE,	NULL  },
 	{ ATTR_CABLE_TYPE,	ATTR_CABLE_TYPE_T,	FALSE,	NULL  },
 	{ ATTR_NET_ADDR,  	ATTR_NET_ADDR_T,	FALSE,  NULL  },
 	{ ATTR_SNM,	  	ATTR_SNM_T,	     	FALSE,	NULL  },
 	{ ATTR_PLATFORM,	ATTR_PLATFORM_T,	FALSE,	NULL  },
 	{ ATTR_ADPT_NAME,	ATTR_ADPT_NAME_T,	FALSE,	NULL  },
 	{ ATTR_ADPT_ADDR,	ATTR_ADPT_ADDR_T,	FALSE,	NULL  },
 	{ ATTR_IPLROM_EMU,	ATTR_IPLROM_EMU_T,	FALSE,	NULL  },
 	{ ATTR_COMMENTS,	ATTR_COMMENTS_T,	FALSE,	NULL  },
	{ 0,			NULL,			FALSE,	NULL }
};

gen_new_mach(int mypid)
{

extern	 int		debug; 

unsigned long     netmask, netaddr, ipaddr;

char    *s_netmask, *s_netaddr, *s_ipaddr; 
char    *name; 
char    pif[MAX_TMP]; 

int     rc, num_attrs, loop;
int     attr_idx, net_addr_idx, net_mask_idx;

int     cmdPid, attr_indx;
ATTR_ASS   *aa_ptr;

char    *arg_ptr, dash_a[] = "-a"; 

char    *Args[40] = {  M_MKMAC, 
			"-t", ATTR_STANDALONE_T, 
            		dash_a, pif , NULL
}; 

union 	  wait kidStat;
struct    nim_attr   *attr; 
struct    listinfo   info; 
struct    hostent *hent; 


	buglog("NIMkid  (%d) : Unique name specified", mypid ); 
	niminfo.errstr[0]   = NULL_BYTE;
	/* 
	 * Determin network address. Then find the network object that represents 
	 * this network. 
	 */
	s_netmask= attr_value(&reg_attrs, ATTR_SNM);
	s_ipaddr = attr_value(&reg_attrs, ATTR_NET_ADDR);

	netmask = inet_addr(s_netmask);
	ipaddr 	= inet_addr(s_ipaddr);
	netaddr = ipaddr&netmask; 
	s_netaddr = inet_ntoa(netaddr); 

	buglog("NIMkid  (%d) : Attempting to locate network object", mypid ); 

	 if ( get_attr( &attr, &info, 0, s_netaddr,  0, ATTR_NET_ADDR ) < 1 ) {
		buglog("NIMkid  (%d) : Network not in database ", mypid );
		RC( FAILURE, ERR_REFUSED, "No matching network object", NULL, NULL);
	}

	name = get_name( attr->id ); 
	buglog("NIMkid  (%d) : Found network %s - known as %s", mypid, s_netaddr, name);

	/* 
	 * Validate this network has a route to the master
	 */ 
	if ( get_state(attr->id, NULL, ATTR_NSTATE) != STATE_NREADY ) { 
		buglog("NIMkid  (%d) : %s  not ready", mypid, name );
		RC(FAILURE, ERR_STATE, name , NULL, NULL);
	}

	/*
	 * Resolve the ip address to a hostname. 
	 */
	if ((hent=gethostbyaddr(&ipaddr, sizeof(ipaddr), AF_INET)) == NULL){
		buglog("NIMkid  (%d) : unable to resolve %s",mypid,s_ipaddr);
		RC(FAILURE, ERR_IP_RESOLVE, s_ipaddr, "hostname", NULL);	
	}

	buglog("NIMkid  (%d) : %s - known as %s",mypid,s_ipaddr,hent->h_name);
	
	/* 
	 * Now we know the network object and hostname build the pif attribute.
	 */ 
	sprintf(pif, "%s=%s %s %s %s" , PIF,
			name,
			hent->h_name,
			attr_value(&reg_attrs, ATTR_ADPT_ADDR), 
			attr_value(&reg_attrs, ATTR_ADPT_NAME)  ); 
	/* 
	 * Fill in the nim command arg array:
	 *   - First calculate the length of the array.
	 *   - Then add the attrs in "-a <name>=<value>" format.
	 *   - Add the ATTR_NAME as the last element of array.
	 */ 
	for (loop=0; Args[loop] != NULL; loop++)
			; 
	for (attr_indx = 0; attr_indx < reg_attrs.num; attr_indx++) {
		aa_ptr = reg_attrs.list[attr_indx];
		switch ( aa_ptr->pdattr  ) {

		case ATTR_RING_SPEED:
		case ATTR_CABLE_TYPE:
			Args[loop++] = dash_a; 
			if ( (arg_ptr=malloc( strlen(aa_ptr->value) + 
					      strlen(aa_ptr->name) + 7)) == NULL )
					RC(FAILURE, ERR_ERRNO_LOG, "malloc", NULL, NULL);
			if ( aa_ptr->seqno > 0 )
				sprintf(arg_ptr,"%s%d=%s",aa_ptr->name,aa_ptr->seqno,aa_ptr->value);
			else
				sprintf(arg_ptr,"%s=%s", aa_ptr->name, aa_ptr->value);
			Args[loop++] = arg_ptr; 
		break;

		case ATTR_CPUID:
		case ATTR_PLATFORM:
		case ATTR_IPLROM_EMU:
			Args[loop++] = dash_a; 
			if ( (arg_ptr=malloc( strlen(aa_ptr->value) + 
					      strlen(aa_ptr->name) + 4)) == NULL )
					RC(FAILURE, ERR_ERRNO_LOG, "malloc", NULL, NULL);
			sprintf(arg_ptr,"%s=%s", aa_ptr->name, aa_ptr->value);
			Args[loop++] = arg_ptr; 
		break;

		case ATTR_COMMENTS: 
			Args[loop++] = dash_a; 
			if ( (arg_ptr=malloc( strlen(aa_ptr->value) + 
					      strlen(aa_ptr->name) + 4)) == NULL )
					RC(FAILURE, ERR_ERRNO_LOG, "malloc", NULL, NULL);
			sprintf(arg_ptr,"%s=\"%s\"", aa_ptr->name, aa_ptr->value);
			Args[loop++] = arg_ptr; 
		break;
		}
	}
	Args[loop++] = attr_value(&reg_attrs, ATTR_NAME); 
	Args[loop] = NULL; 
	
	/* 
	 * For debugging purpose
	 */
	if (debug)
		for (loop=0; Args[loop]; loop++) {
			buglog("NIMkid  (%d) : [%s] ", mypid, Args[loop]);
		}

	/* 
	 * Exec the command. 
	 */
	if ( (cmdPid=LocalExec(Args)) <= 0 ) { 
		buglog("NIMkid  (%d) : exec failure", mypid);
		RC(FAILURE, ERR_ERRNO_LOG, "reg_req", "LocalExec", NULL);
 	}
	/*
	 * Wait for the completion of the command execution
	 */
	if ( (rc=waitpid(cmdPid, &kidStat, 0) ) < 0) 
		ERROR(ERR_ERRNO_LOG, "reg_req", "waitpid", NULL);
	
	rc=WEXITSTATUS(kidStat);
	
	if ( rc > 0 )
		return(FAILURE);

	return(SUCCESS);

}
/*---------------------------- check_input
 *
 * NAME: check_input
 *
 * FUNCTION:
 *	checks to make sure that the information supplied by user is sufficient
 *	to complete operation
 *
 * NOTES:
 *		calls nim_error
 *
 * DATA STRUCTURES:
 *	parameters:
 *		valid_reg_attrs
 *		reg_attrs
 *
 * RETURNS: (int)
 *	SUCCESS	= nothing missing
 *	FAILURE	= definition incomplete
 *
 *-----------------------------------------------------------------------------*/

int
check_input()

{
	int	va_indx, attr_indx;
	VALID_ATTR 	 * va_ptr;
	ATTR_ASS 	 * aa_ptr;

	/* 
	 * check for required attrs 
	 */
	for (va_indx = 0; valid_reg_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_reg_attrs[va_indx];
		if ( va_ptr->required ) {
			for (attr_indx = 0; attr_indx < reg_attrs.num; attr_indx++) {
				aa_ptr = reg_attrs.list[attr_indx];
				if ( aa_ptr->pdattr == va_ptr->pdattr )
					break;
			}
			/* 
			 * is the required attr present? 
			 */
			if ( attr_indx == reg_attrs.num )
				ERROR( ERR_MISSING_ATTR, va_ptr->name, NULL, NULL );
		}
	}
	return(SUCCESS);
} /* end of check_input */

/* ---------------------------- reg_req 
 *
 * NAME:	 reg_req
 *
 * FUNCTION:
 * 	This function processes registration requests from NIM clients. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 *
 * DATA STRUCTURES:
 *		parameters:
 *			InMsgSocket - The connected socket of a requesting 
 *				      client.
 *		global:
 *
 * RETURNS:
 *	SUCCESS		-  Everything worked ok
 *	FAILURE		-  Something went wrong.
 *
 *
 * ---------------------------------------------------------------------------*/

int
reg_req ( NIM_SOCKINFO 	*InMsg, 
	  int 		mypid )
{

extern	 int		debug; 

struct	 hostent	*hent; 
struct	 sockaddr_in	s_addr; 


	 char		InPacket[MAX_NIM_PKT];

	 int		attr_indx;
	 ATTR_ASS	*aa_ptr;
	 NIM_OBJECT( client, cinfo ) 
	 LIST list;
	 char		*name; 
 	 union 		wait kidStat;
	 int	i, rc, num_attrs, loop;
	
	buglog("NIMkid  (%d) : REGISTRATION", mypid);
	niminfo.err_policy |= ERR_POLICY_WRITE_RC;

	/* 
	 * Get the number of attrs that follow. 
	 */
	if ( (rc=getSockTkn(InMsg->fd, InPacket, MAX_NIM_PKT)) != SUCCESS ) 
		RC(rc, ERR_ERRNO_LOG, "reg_req", "getSockTkn", NULL);

	if ( (num_attrs=atoi(InPacket) ) < 1) {
		buglog("NIMkid  (%d) : Bad incomming packet ", mypid );
		RC( FAILURE, ERR_REFUSED, "Missing data in packet", NULL, NULL);
	}
	buglog("NIMkid  (%d) : NUMBER OF ATTRS [%d] ", mypid, num_attrs );

	/* 
	 * Now loop getting the attrs and build the global attr list. 
	 */
	for ( loop=0; loop < num_attrs; loop++) { 
		if ( (rc=getSockTkn(InMsg->fd, InPacket, MAX_NIM_PKT)) != SUCCESS) 
			RC(rc, ERR_ERRNO_LOG, "reg_req", "getSockTkn", NULL);

		buglog("NIMkid  (%d) : ATTR [%s] ", mypid, InPacket );
		if (!parse_attr_ass( &reg_attrs, valid_reg_attrs, InPacket, FALSE ) )
			RC(FAILURE, 0, NULL, NULL, NULL );
	}

	/* 
	 * Validate the attribute sent to us. 
	 */
	buglog("NIMkid  (%d) : Validating attributes sent to us.. ", mypid ); 
	if (check_input() != SUCCESS)
		RC(FAILURE, 0, NULL, NULL, NULL );

	/* 
	 * This could be a registration request (define new machine)  or a request to 
	 * refresh the niminfo file on the client.
	 */
	name = attr_value(&reg_attrs, ATTR_NAME);
	buglog("NIMkid  (%d) : Checking name .. %s ", mypid, name ); 
	if ( lag_object( 0, name, &client, &cinfo ) == SUCCESS ) {
		buglog("NIMkid  (%d) : Name Exists - checking if same cpuid ", mypid );
		if ( (i=find_attr( client, NULL, NULL, 0, ATTR_CPUID)) >= 0 ) 
			if (strcmp(client->attrs[i].value,attr_value(&reg_attrs,ATTR_CPUID))!=0)
				RC(FAILURE, ERR_BAD_CPUID, NULL, NULL, NULL);
		buglog("NIMkid  (%d) : Refreshing /etc/niminfo file ", mypid );
	} 
	else if (gen_new_mach(mypid) == FAILURE) {
				buglog("NIMkid  (%d) : gen_new_mach FAILED !! ", mypid );
				RC( FAILURE, 0, NULL, NULL, NULL );
		  }

	if ( lag_object( 0, name, &client, &cinfo ) == FAILURE )
		RC(FAILURE, 0, NULL, NULL, NULL );
	/* generate a LIST of res_access structs */
	if ( LIST_res_access( client, &list ) == FAILURE )
		RC(FAILURE, 0, NULL, NULL, NULL );
	if ( mk_niminfo( client, &list, InMsg->FP ) != SUCCESS )
		RC(FAILURE, 0, NULL, NULL, NULL );
	buglog(" ");
	return(rc);
} 
