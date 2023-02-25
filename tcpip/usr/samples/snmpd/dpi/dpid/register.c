static char sccsid[] = "@(#)16	1.3  src/tcpip/usr/samples/snmpd/dpi/dpid/register.c, snmp, tcpip411, GOLD410 3/29/94 15:26:23";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/dpid/register.c
 */

/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

#include <sys/errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <isode/snmp/smux.h>
#include <isode/snmp/objects.h>

#include <isode/tailor.h>

#include "snmp_dpi.h"

/* Externs to use from dpid.c */
extern int debug;
extern advise();
extern int DPI_port_num;		/* so we can return it */

/* The variable that the smux-dpi daemon is responsible for */
char *DPI_port_objID = "1.3.6.1.4.1.2.2.1.1.";
static char *DPI_port_instance = "1.3.6.1.4.1.2.2.1.1.0";

#define MAX_PACKET_LEN 1024

/*
 * The object list structure and variable that are used to track the objects
 * registered with the dpi daemon.
 */
struct mib_object {
	char	*object_id;
	int	agent_fd;
	struct mib_object *next;
};
struct mib_object *mib_object_list;

/*
 * Function: register_object
 *
 * Input: obj_id - the object identifier of the tree to register in character
 *                 string notation.
 *        fd - the file descriptor to associate the tree with.
 *
 * Output: none
 *
 * Return: code for return (int)
 *
 * Note: Adds the object to the list of trees that a request can belong to and
 *       then calls register_tree_smux.
 */
register_object(obj_id,fd)
char *obj_id;
int fd;
{
	int	rc;
	struct mib_object *mib_obj, *mop, *last_mop;

	if (debug)
		advise(LLOG_EXCEPTIONS, "DEBUG", 
			"register_object:  %s on fd=%d\n", obj_id, fd);

	mib_obj = (struct mib_object* ) malloc(sizeof(struct mib_object));
	if (fd == -1) {
		mib_obj->object_id = obj_id;
	} else {
		mib_obj->object_id = malloc(strlen(obj_id) + 1);
		strcpy(mib_obj->object_id,obj_id);
	}
	mib_obj->agent_fd = fd;
	last_mop = 0;
	mop = mib_object_list;
	while (mop != 0) {	/* not at end of list */
		if (obj_strcmp(obj_id,mop->object_id) <= 0) {
			break;
		}
		last_mop = mop;
		mop = mop->next;
	}
	mib_obj->next = mop;
	if (last_mop == 0) mib_object_list = mib_obj;
	else last_mop->next = mib_obj;

	rc = register_tree_smux(obj_id, fd);
	return (rc);
}

/*
 * Function: unregister_fd
 *
 * Input: fd - the file descriptor to associate the tree with.
 *
 * Output: none
 *
 * Return: code for return (int)
 *
 * Note: Removes objects in the list of trees that a request can belong to,
 *       unregisters them, and then closes fd.
 */
unregister_fd(fd)
int fd;
{
	struct mib_object *mop, *last_mop, *temp_mop;

	mop = mib_object_list;
	last_mop = 0;
	while (mop != 0) {
		if (mop->agent_fd == fd) {	/* delete */
			unregister_tree_smux(mop->object_id, fd);
			if (last_mop == 0) mib_object_list = mop->next;
			else last_mop->next = mop->next;
			temp_mop = mop->next;
			if (fd != -1) free(mop->object_id);
			free(mop);
			mop = temp_mop;
		} else {
			last_mop = mop;
			mop = mop->next;
		}
	}
	close(fd);
	del_from_mask(fd);
}

/* placeholders for dealing with object ID comparisons */
/* Handles OID comparison when in string notation, similar to strcmp */
obj_strcmp(s1,s2)
char *s1, *s2;
{
	int rc;
	char *c1, *c2;
	int val1, val2;

	c1 = s1;
	c2 = s2;
	/* quickly skip initial common part */
	while (*c1 == *c2) {
		if (*c1 == '\0') return(0);	/* identical */
		c1++;
		c2++;
	}
	if (*c1 == '.') c1--;
	if (*c2 == '.') c2--;
	/* back up pointers to start of last ID element */
	while ((c1 != s1) && (*c1 != '.')) c1--;
	while ((c2 != s2) && (*c2 != '.')) c2--;
	if (*c1 == '.') c1++;
	if (*c2 == '.') c2++;
	/* now move forward and get value of object element */
	val1 = 0;
	while ((*c1 != '.') && (*c1 != '\0')) {
		val1 *= 10;
		val1 += *c1 - '0';
		c1++;
	}
	val2 = 0;
	while ((*c2 != '.') && (*c2 != '\0')) {
		val2 *= 10;
		val2 += *c2 - '0';
		c2++;
	}
	rc = val1 - val2;
	return(rc);
}

/* Handles OID comparison when only a length of the OID is to be compared. */
/* Similar to strncmp */
obj_strncmp(s1,s2,len)
char *s1, *s2;
int  len;
{
	char str1[512], str2[512];
	int rc;

	strncpy(str1,s1,len);
	str1[len] = '\0';
	strncpy(str2,s2,len);
	str2[len] = '\0';
	rc = obj_strcmp(str1,str2);
	return(rc);
}

/* type_SNMP_ObjectSyntax is really a PElement */

/*
 * Function: process_req
 *
 * Input: ObjectName - the object name extracted from the incoming packet. (OID)
 *        ObjectSynyax - the value of the name extracted from the packet. (PE)
 *        cmd - the packet type
 *
 * Output: ret_oid - the name of the object being returned. (OID)
 *         ret_ObjectSyntax - the value of the object being returned. (PE)
 *
 * Return: code for return (int)
 *
 * Note: This function handles the get, get-next, and set commands.  The 
 *   function takes the ObjectName and ObjectSyntax for sets and determines
 *   which DPI peer is to handle the request.  It does this by searching its
 *   list of registered trees and knowing if it is a get/set or a get-next
 *	 request.  If a smux-dpi daemon is supposed to handle the request, then
 *	 do_mib_request is called.  If it is a remote DPI peer, then remote_mib_req
 *	 is called.  The loop around the xx_mib_request calls is to allow for proper
 *	 get-next processing.  The get-next request is supposed to get the next
 *	 variable, so if one tree doesn't have it, we need to search the next.
 */
process_req(ObjectName, ObjectSyntax, cmd, ret_oid, ret_ObjectSyntax)
OID	ObjectName;
struct type_SMI_ObjectSyntax *ObjectSyntax;
int             cmd;
OID *ret_oid;
struct type_SMI_ObjectSyntax **ret_ObjectSyntax;
{
	int rc, oid_len;
	char oid_str[512];
	struct mib_object *mib_obj;

	strncpy(oid_str,sprintoid(ObjectName), 511);
	oid_len = strlen(oid_str);
	oid_str[oid_len] = '.';			/* add trailing dot */
	oid_str[oid_len+1] = '\0';

	mib_obj = mib_object_list;
	while (mib_obj != 0) {
		rc = obj_strncmp(oid_str,mib_obj->object_id,
		    strlen(mib_obj->object_id));
		if (debug)
			advise(LLOG_EXCEPTIONS, "DEBUG",
				"looking at %s vs. %s, rc=%d\n", oid_str,
				mib_obj->object_id,rc);

		if ((rc == 0) ||
            ((rc < 0) && (cmd == type_SNMP_SMUX__PDUs_get__next__request))) {
			break;
		}
		mib_obj = mib_obj->next;
	}
	if (mib_obj == 0) {
		return(SNMP_NO_SUCH_NAME);		/* no luck */
	}
	do {
		if (debug)
			advise(LLOG_EXCEPTIONS, "DEBUG", 
				"group=%s agent_fd=%d\n", mib_obj->object_id, 
				mib_obj->agent_fd);

		if (mib_obj->agent_fd == -1) {	/* internal */
			rc = do_mib_request(cmd, ObjectName, ObjectSyntax,
			    mib_obj->object_id, ret_oid, ret_ObjectSyntax);
		} else {
			rc = remote_mib_req(mib_obj->agent_fd, cmd, ObjectName,
			    ObjectSyntax, mib_obj->object_id,
			    ret_oid, ret_ObjectSyntax);
		}
		mib_obj = mib_obj->next;
	} while ((rc == SNMP_NO_SUCH_NAME)
	    && (cmd == type_SNMP_SMUX__PDUs_get__next__request)
	    && (mib_obj != 0));
	return(rc);
}

/*
 * Function: do_mib_request
 *
 * Input: cmd - the packet type (int)
 *        oid - the object name extracted from the incoming packet. (OID)
 *        obj_syntax - the value of the name extracted from the packet. (PE)
 *        group_oid - the name of the object identifier for the top of the 
 *                    tree that the oid is thought to be in. (char *)
 *
 * Output: ret_oid - the name of the object being returned. (OID)
 *         ret_ObjectSyntax - the value of the object being returned. (PE)
 *
 * Return: code for return (int)
 *
 * Note: This function handles the get, get-next, and set requests for the one
 *    variable that is maintained by the DPI-SMUX converter agent.  The set
 *    request does not do anything.  The get request checks to make sure the OID
 *    matches the proper instance name exactly.  If it matches, then
 *    do_mib_request returns the object name in ret_oid and the value in
 *    ret_ObjectSyntax.  For get-next requests, the function checks to see if
 *    the variable oid is not the instance name for the variable.  If it is not,
 *    then do_mib_request returns the name and value in ret_oid and
 *    ret_ObjectSyntax.  In either case, on failure the value SNMP_NO_SUCH_NAME
 *    is returned.
 */
do_mib_request(cmd, oid, obj_syntax, group_oid, ret_oid, ret_ObjectSyntax)
int cmd;
OID oid;
struct type_SMI_ObjectSyntax *obj_syntax;
char *group_oid;
OID *ret_oid;
struct type_SMI_ObjectSyntax **ret_ObjectSyntax;
{
	char	oid_name[512];

	strcpy(oid_name,sprintoid(oid));
	if (cmd != type_SNMP_SMUX__PDUs_get__request) {
		if ((strncmp(DPI_port_instance, oid_name, strlen(DPI_port_instance))==0)
            || (strlen(oid_name) > strlen(DPI_port_instance)))
            return (SNMP_NO_SUCH_NAME); 
        else { 
		    *ret_oid = oid_cpy(str2oid(DPI_port_instance));
		    *ret_ObjectSyntax = (struct type_SMI_ObjectSyntax *)
			    malloc(sizeof(struct type_SMI_ObjectSyntax));
		    (*ret_ObjectSyntax) -> offset = type_SMI_ObjectSyntax_number;
		    (*ret_ObjectSyntax) -> un.number = DPI_port_num;
		    return (0);
        }
	} else {
		/* be picky and make sure instance is ".0" */
		if (strcmp(oid_name, DPI_port_instance) != 0) {
			return (SNMP_NO_SUCH_NAME);
		}
		*ret_oid = oid_cpy(oid);
		*ret_ObjectSyntax = (struct type_SMI_ObjectSyntax *)
			malloc(sizeof(struct type_SMI_ObjectSyntax));
		(*ret_ObjectSyntax) -> offset = type_SMI_ObjectSyntax_number;
		(*ret_ObjectSyntax) -> un.number = DPI_port_num;
		return (0);
	}
}

/*
 * Function: remote_mib_req
 *
 * Input: fd - file descriptor responsible for the oids (int)
 *        cmd - the packet type (int)
 *        oid - the object name extracted from the incoming packet. (OID)
 *        obj_syntax - the value of the name extracted from the packet. (PE)
 *        group_oid - the name of the object identifier for the top of the 
 *                    tree that the oid is thought to be in. (char *)
 *
 * Output: ret_oid - the name of the object being returned. (OID)
 *         ret_ObjectSyntax - the value of the object being returned. (PE)
 *
 * Return: code for return (int)
 *
 * Note: This function handles the get, get-next, and set requests for the 
 *    variables that are maintained by the DPI subagents.  The function takes
 *    the value to send to the DPI subagent in obj_syntax and converts it to a 
 *    DPI message format.  Then the function sets the variable command to the
 *    proper DPI command.  The rmt_call function handles the sending and 
 *    receiving of the DPI message.  If the rmt_call function has any problem,
 *    the return code will contain the error number, so we will return that
 *    value.  If all goes well, the values returned from the DPI subagent are
 *    returned.  These are converted back into SMUX format/structures that are
 *    returned in ret_oid and ret_ObjectSnytax.
 */
remote_mib_req(fd, cmd, oid, obj_syntax, group_oid,
ret_oid, ret_ObjectSyntax)
int fd;
int cmd;
OID oid;
struct type_SMI_ObjectSyntax *obj_syntax;
char *group_oid;
OID *ret_oid;
struct type_SMI_ObjectSyntax **ret_ObjectSyntax;
{
	char oid_name[512];
	char set_value_str[1024];
	unsigned long set_value_int;
	char new_oid[512];
	int new_len, new_type;
	char new_value[1024];
	int command, rc;
	char *cp, *value;
	int base_type, len;
	unsigned long *ulp;
	struct type_SMI_ObjectSyntax *ret_syntax;
	struct qbuf *qb;

	strcpy(oid_name,sprintoid(oid));
	set_value_int = 0;
	base_type = 1;
	value = 0;
	len = 0;
	if (obj_syntax != 0) {
		switch (obj_syntax->offset) {
		case type_SMI_ObjectSyntax_number:
			set_value_int = obj_syntax->un.number;
			len = sizeof(set_value_int);
			base_type = 2;
			break;
		case type_SMI_ObjectSyntax_string:
			qb = obj_syntax->un.string->qb_forw;
			/* count the length of the string */
			len = 0;
			while (qb != obj_syntax->un.string) {
				len += qb->qb_len;
				qb = qb->qb_forw;
			}
			cp = qb2str(obj_syntax->un.string);
			bcopy(cp, set_value_str, len);
			free(cp);
			base_type = 1;
			break;
		case type_SMI_ObjectSyntax_object:
			strcpy(set_value_str,sprintoid(obj_syntax->un.object));
			len = strlen(set_value_str);
			base_type = 1;
			break;
		case type_SMI_ObjectSyntax_internet:
			cp = qb2str(obj_syntax->un.internet);
			ulp = (unsigned long *) cp;
			set_value_int = *ulp;
			set_value_int = ntohl(set_value_int);
			free(cp);
			len = sizeof(set_value_int);
			base_type = 2;
			break;
		case type_SMI_ObjectSyntax_counter:
		case type_SMI_ObjectSyntax_gauge:
		case type_SMI_ObjectSyntax_ticks:
			set_value_int = obj_syntax->un.counter->parm;
			len = sizeof(set_value_int);
			base_type = 2;
			break;
		case type_SMI_ObjectSyntax_empty:
		default:
			set_value_str[0] = '\0';
			len = 0;
			base_type = 1;
			break;
		} /* end switch */
		switch (base_type) {
		case 1:
			value = (char *) set_value_str;
			break;
		case 2:
			value = (char *) &set_value_int;
			break;
		}
	}
	switch (cmd) {
	case type_SNMP_SMUX__PDUs_get__request:
		command = SNMP_DPI_GET;
		break;
	case type_SNMP_SMUX__PDUs_get__next__request:
		command = SNMP_DPI_GET_NEXT;
		break;
	case type_SNMP_SMUX__PDUs_set__request:
		command = SNMP_DPI_SET;
		break;
	default:
		return(SNMP_GEN_ERR);
	}
	rc = rmt_call(fd, command, oid_name, group_oid, base_type, len, value,
	    new_oid, &new_type, &new_len, new_value);
	if (rc == 0) {
		*ret_oid = oid_cpy(str2oid(new_oid));
		ret_syntax = (struct type_SMI_ObjectSyntax *)
			malloc(sizeof(struct type_SMI_ObjectSyntax));

		ret_syntax->offset = new_type & SNMP_TYPE_MASK;
		switch(ret_syntax->offset) {
		case type_SMI_ObjectSyntax_number:
			ret_syntax->un.number = int_val(new_value);
			break;
		case type_SMI_ObjectSyntax_string:
			ret_syntax->un.string =
				str2qb(new_value,new_len,1);
			break;
		case type_SMI_ObjectSyntax_object:
			ret_syntax->un.object = oid_cpy(str2oid(new_value));
			break;
		case type_SMI_ObjectSyntax_internet:
			ret_syntax->un.internet = 
				str2qb(new_value,sizeof(unsigned long),1);
			break;
		case type_SMI_ObjectSyntax_counter:
		case type_SMI_ObjectSyntax_gauge:
		case type_SMI_ObjectSyntax_ticks:
			ret_syntax->un.counter =
				(struct type_SMI_Counter *)
				malloc(sizeof(struct type_SMI_Counter));
			ret_syntax->un.counter->parm = int_val(new_value);
			break;
		case type_SMI_ObjectSyntax_empty:
		default:
			ret_syntax->un.empty = 0;
			break;
		} /* end switch on set object type */
		*ret_ObjectSyntax = ret_syntax;
	}
	return(rc);
}

/*
 * Function: process_trap
 *
 * Input: generic - the value of the generic part of the trap (int)
 *        specific - the value for the specific part of the trap (int)
 *        value_list - a list of name/value pairs to return in the trap
 *                     (dpi_set_packet *)
 *
 * Output: none
 *
 * Return: none
 *
 * Note: This function takes the parts of the trap extracted from the DPI packet
 *    and converts them smux/snmp form and uses the generate_trap function to 
 *    send the trap.
 * 
 */
void process_trap(generic,specific,value_list)
int generic,specific;
struct dpi_set_packet *value_list;
{
	struct type_SMI_ObjectName  *ret_oid = 0;
	struct type_SMI_ObjectSyntax *ret_syntax = 0;

	int rc;
	struct dpi_set_packet *value_list2 = value_list;
	struct type_SNMP_VarBindList    *list = 0;
	struct type_SNMP_VarBindList    *list2;
	struct type_SNMP_VarBind    *var_bind2;
	struct type_SNMP_VarBindList    *list3;

	while (value_list2 != 0) {
		ret_oid = oid_cpy(str2oid(value_list2->object_id));
		if ((ret_oid == NULLOID) || (ret_syntax=(struct type_SMI_ObjectSyntax *)
						malloc(sizeof(struct type_SMI_ObjectSyntax))) == NULL)
			return;
		
		if ((list2 = (struct type_SNMP_VarBindList *)
					malloc(sizeof(struct type_SNMP_VarBindList))) == NULL)
			return;
		if ((var_bind2 = (struct type_SNMP_VarBind *)
					malloc(sizeof(struct type_SNMP_VarBind))) == NULL)
			return;

		list2 -> next = 0;
		list2 -> VarBind = var_bind2;

		ret_syntax->offset = value_list2->type & SNMP_TYPE_MASK;
		switch(ret_syntax->offset) {
		case type_SMI_ObjectSyntax_number:
			ret_syntax->un.number = int_val(value_list2->value);
			break;
		case type_SMI_ObjectSyntax_string:
			ret_syntax->un.string =
				str2qb(value_list2->value,value_list2->value_len,1);
			break;
		case type_SMI_ObjectSyntax_object:
			ret_syntax->un.object = oid_cpy(str2oid(value_list2->value));
			break;
		case type_SMI_ObjectSyntax_internet:
			ret_syntax->un.internet = 
				str2qb(value_list2->value,sizeof(unsigned long),1);
			break;
		case type_SMI_ObjectSyntax_counter:
		case type_SMI_ObjectSyntax_gauge:
		case type_SMI_ObjectSyntax_ticks:
			ret_syntax->un.counter =
				(struct type_SMI_Counter *)
				malloc(sizeof(struct type_SMI_Counter));
			ret_syntax->un.counter->parm = int_val(value_list2->value);
			break;
		case type_SMI_ObjectSyntax_empty:
		default:
			ret_syntax->un.empty = 0;
			break;
		} /* end switch on set object type */

		var_bind2 -> name = ret_oid;
		var_bind2 -> value = ret_syntax;

		if (list == 0) 
			list = list2;
		else {
			list3 = list;
			while (list3 -> next != 0)
				list3 = list3 -> next;
			list3 -> next = list2;
		}

		value_list2 = value_list2 -> next;
	}

	rc = smux_trap (generic, specific, list);

	while (list != 0) {
		oid_free(list -> VarBind -> name);

		switch(list -> VarBind -> value -> offset) {
		case type_SMI_ObjectSyntax_string:
			qb_free(list -> VarBind -> value -> un.string);
			break;
		case type_SMI_ObjectSyntax_object:
			oid_free(list -> VarBind -> value -> un.object);
			break;
		case type_SMI_ObjectSyntax_internet:
			qb_free(list -> VarBind -> value -> un.internet);
			break;
		case type_SMI_ObjectSyntax_counter:
		case type_SMI_ObjectSyntax_gauge:
		case type_SMI_ObjectSyntax_ticks:
			free (list -> VarBind -> value -> un.counter -> parm);
			break;
		case type_SMI_ObjectSyntax_number:
		case type_SMI_ObjectSyntax_empty:
		default:
			break;
		} 

		free(list -> VarBind -> value);
		free(list -> VarBind);
		list2 = list;
		list = list -> next;
		free(list2);
	}

	if (rc == NOTOK) {
		advise(LLOG_EXCEPTIONS, "Failed", "smux_trap: %s [%s]",
			smux_error (smux_errno), smux_info);
		return;
	}
}

/*
 * Function: rmt_call
 *
 * Input: fd - file descriptor responsible for the OIDs. (int)
 *        command - the packet type. (int)
 *        oid_name - the object name extracted from an incoming packet. (char *)
 *        group_oid - the name of the object identifier for the top of the 
 *                    tree that the oid is thought to be in. (char *)
 *        base_type - the type of value number or pointer. (int)
 *        len - the length/size of the value in bytes. (int)
 *        value - a pointer or a number depending on base_type. (long)
 *
 * Output: new_oid - the returned object name. (char [])
 *         new_type - the type of the value, number or pointer. (int *)
 *         new_len - the length of the new values in bytes. (int *)
 *         new_value - the new values. (char **)
 *
 * Return: code for return (int)
 *
 * Note: This function sends the DPI packet and then waits for a response.
 *    The function first creates a DPI packet using the mkMIBquery function. 
 *    If that is unsuccessful, a gen_err is returned.  Otherwise, the packet
 *    length is calcuated and the packet sent.  If the write fails, a gen_err 
 *    is returned.  The function then waits for a response.  It uses must_read
 *    to get the length of the response packet.  If a must_read times out or
 *    fails, the connection is closed and -1 is returned.  The must_read
 *    is used to read the rest of the packet and the pDPIpacket function is
 *    used to parse the packet.  If the parse fails, a -1 is returned.  If the 
 *    packet is a trap packet, the trap is processed, sent, and the rmt_call
 *    goes back to wait for a new response.  If the packet is not a trap or
 *    a response, the function closes the file descriptor and returns gen_err.
 *    If it is a response packet, the values are put in the output variables and
 *    the resp_code from the packet is returned.
 */
static int rmt_call(fd, command, oid_name, group_oid, base_type, len, value,
new_oid, new_type, new_len, new_value)
int fd;
int command;
char *oid_name;
char *group_oid;
int base_type;
int len;
unsigned long value;
char new_oid[];
int *new_type;
int *new_len;
char new_value[];
{
	static struct snmp_dpi_hdr *hdr = 0;
	struct dpi_resp_packet *resp;
	struct dpi_set_packet *ret_data;
	int rc;
	int packet_len;
	unsigned char *packet;
	unsigned char bfr[1024];

	packet = mkMIBquery(command,oid_name,group_oid,base_type,len,value);
	if (packet == 0) return(SNMP_GEN_ERR);
	packet_len = (*packet << 8) + *(packet+1);
	packet_len += 2;
	rc = write(fd, packet, packet_len);
	if (rc != packet_len) {
		advise(LLOG_EXCEPTIONS, "Failed",
			"error sending query to remote agent\n");
		unregister_fd(fd);
		return(SNMP_GEN_ERR);
	}
	do {
		rc = must_read(fd,bfr,2);
		if (rc != 2) {
			advise(LLOG_EXCEPTIONS, "Failed",
				"premature EOF or timeout on fd=%d\n",fd);
			unregister_fd(fd);
			return(-1);
		}
		len = (bfr[0] << 8) + bfr[1];
		rc = must_read(fd,bfr+2,len);
		if (rc != len) {
			advise(LLOG_EXCEPTIONS, "Failed",
				"can't read response packet or timeout\n");
			unregister_fd(fd);
			return(-1);
		}
		if (hdr)
			fDPIparse(hdr);
		hdr = pDPIpacket(bfr);
		if (hdr == 0) {
			unregister_fd(fd);
			return(-1);
		}
		if (hdr->packet_type == SNMP_DPI_TRAP) {
			process_trap(hdr->packet_body.dpi_trap->generic,
			    hdr->packet_body.dpi_trap->specific,
			    hdr->packet_body.dpi_trap->info);
		}
		else if (hdr->packet_type != SNMP_DPI_RESPONSE) {
			advise(LLOG_EXCEPTIONS, "Failed", 
				"unexpected packet type:  %d\n", 
				hdr->packet_type);
			fDPIparse(hdr);
			unregister_fd(fd);
			return(SNMP_GEN_ERR);
		}
	} while (hdr->packet_type != SNMP_DPI_RESPONSE);

	resp = hdr->packet_body.dpi_response;
	rc = resp->ret_code;
	ret_data = resp->ret_data;
	if (ret_data != 0) {
		strcpy(new_oid,ret_data->object_id);
		*new_type = ret_data->type;
		*new_len = ret_data->value_len;
		bcopy(ret_data->value, new_value, ret_data->value_len);
	}
	fDPIparse(hdr);
	return(rc);
}
