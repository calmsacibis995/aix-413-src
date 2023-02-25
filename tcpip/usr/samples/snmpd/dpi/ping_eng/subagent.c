static char sccsid[] = "@(#)40	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/subagent.c, snmp, tcpip411, GOLD410 9/13/93 15:10:04";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/subagent.c
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

/*
 * This file contains the process_request function.
 */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "snmp_dpi.h"

#include "init.h"
#include "stats.h"

static char pingeng_prefix[]   = "1.3.6.1.4.1.2.2.1.2.";
static char table_prefix[]     = "1.3.6.1.4.1.2.2.1.2.2.1.";
static char number_prefix[]    = "1.3.6.1.4.1.2.2.1.2.1.";
static char number_instance[]  = "1.3.6.1.4.1.2.2.1.2.1.0.";
#define LAST_INSTANCE	6
#define NUMBER_VAL		8

extern int agent_fd;

/*
 * FUNCTION:	process_request
 *
 * INPUT:
 *	fd	file descriptor of incoming message
 *	buffer	data packet received
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1 	unsuccessful
 *	len	returns amount of byte written in response
 *
 * NOTE:
 *	Takes the packet and parses it.  It decides upon an action and sends
 *	the appropriate response.
 *
 */
int process_request(fd, buffer)
int fd;
unsigned char buffer[];
{
	struct snmp_dpi_hdr *hdr;
	struct dpi_set_packet *ret_value;
	unsigned char obj_id_copy[512];
	unsigned char *cp, *start;
	char *obj_id;
	unsigned long data, address, *ulp;
	int len, rc, type;
	int var_id;
	struct rtt_stats *entry;

	bzero(obj_id_copy, 512);
	hdr = pDPIpacket(buffer);
	if (hdr == 0) {
		fprintf(stderr,"could not parse packet!\n");
		return(-1);
	}
	switch (hdr->packet_type) {
	case SNMP_DPI_GET:
		obj_id = hdr->packet_body.dpi_get->object_id;
		break;
	case SNMP_DPI_GET_NEXT:
		obj_id = hdr->packet_body.dpi_next->object_id;
		break;
	case SNMP_DPI_SET:
		obj_id = hdr->packet_body.dpi_set->object_id;
		break;
	default:
		fprintf(stderr,"wierd command! (%d)\n", hdr->packet_type);
		return(-1);
	}
	strcpy(obj_id_copy,obj_id);

	if (strncmp(obj_id_copy, pingeng_prefix, strlen(pingeng_prefix)) != 0) {
		if (hdr->packet_type == SNMP_DPI_GET_NEXT) {
			var_id = NUMBER_VAL;
			address = 0;
		}
		else {
			cp = mkDPIresponse(SNMP_NO_SUCH_NAME,0);
			fDPIparse(hdr);
			len = *cp * 256 + *(cp + 1) + 2;
			rc = write(fd, cp, len);
			return(rc);
		}
	}
	else if (strncmp(number_prefix, obj_id_copy, 
					 strlen(number_prefix) - 1) == 0) {
		var_id = NUMBER_VAL;
		address = 0;
	}
	else if (strlen(obj_id_copy) <= sizeof(table_prefix) - 1) {
		var_id = 0;
		address = 0;
	} else {
		start = obj_id_copy + sizeof(table_prefix) - 1;
		cp = start;

		while ((*cp != '.') && (*cp != '\0')) cp++;

		if (cp == '\0') {
			var_id = 0;
			address = 0;
		} else {
			*cp = '\0';
			var_id = atoi(start);
			start = cp + 1;
			if (*start == '\0') address = 0;
			else address = inet_addr(start);
		}
	}
	entry = 0;
	switch (hdr->packet_type) {
	case SNMP_DPI_GET:	/* need exact */
		entry = find_stats_record(address, 0);
		break;
	case SNMP_DPI_GET_NEXT:
		if (var_id == 0) var_id = 1;
		entry = find_stats_record(address, -1);
		if ((entry == 0) && (var_id != NUMBER_VAL)) {	/* end of table */
			if (++var_id <= LAST_INSTANCE) {
				/* still possible to be in table */
				address = 0;
				entry = find_stats_record(address, -1);
			}
		}
		break;
	}

/* Trap Test - This sends a trap.  It does not affect operations. */
{
struct dpi_set_packet *set = NULL;
int len = 0;
long int num = 15;
unsigned long int ctr = 1234;
char str[] = "a string";
unsigned char *packet = NULL;

set = mkDPIlist(set, "1.3.6.1.4.1.2.2.1.4.1", SNMP_TYPE_NUMBER, sizeof(num), &num);
set = mkDPIlist(set, "1.3.6.1.4.1.2.2.1.4.2", SNMP_TYPE_STRING, strlen(str), str);
set = mkDPIlist(set, "1.3.6.1.4.1.2.2.1.4.6", SNMP_TYPE_COUNTER, sizeof(ctr), &ctr);

packet = mkDPItrape(6L, 37L, set, "1.3.6.1.4.1.2.2.1.4"); 

len = *packet * 256 + *(packet+1);
len += 2;

write(agent_fd, packet, len);
}

	switch (hdr->packet_type) {
	case SNMP_DPI_GET_NEXT:
		if ((var_id == NUMBER_VAL) && 
			(strncmp(number_instance, obj_id_copy, 
					 strlen(number_instance) - 1) == 0))
			var_id = 1;

	case SNMP_DPI_GET:
		if (((entry == 0) || (var_id < 1) ||
			 (var_id > LAST_INSTANCE)) && (var_id != NUMBER_VAL)) {	
			cp = mkDPIresponse(SNMP_NO_SUCH_NAME,0);
		} else {
			data = getValue(entry, var_id, &type);
			if ((entry) && (var_id != NUMBER_VAL))
				sprintf(obj_id_copy,"%s%d.%s", table_prefix, var_id,
					inet_ntoa(entry->address));
			else
				sprintf(obj_id_copy,"%s0", number_prefix);
			ret_value = mkDPIset(obj_id_copy, type, sizeof(data),
				 &data);
			cp = mkDPIresponse(0,ret_value);
		}
		break;
	case SNMP_DPI_SET:
		if (var_id != 5) {	/* number of requests made */
			cp = mkDPIresponse(SNMP_NO_SUCH_NAME,0);
		} else {
			ulp = (unsigned long *) hdr->packet_body.dpi_set->value;
			data = *ulp;
                     fprintf (stderr, "QUEUING PINGs to %d \n",address);
			queue_PINGs(address, ntohl(data));
			ret_value = mkDPIset(obj_id,SNMP_TYPE_NUMBER,
				sizeof(data), &data);
			cp = mkDPIresponse(0,ret_value);
		}
		break;
	}
	fDPIparse(hdr);
	len = *cp * 256 + *(cp + 1) + 2;
	rc = write(fd, cp, len);
	return(rc);
}
