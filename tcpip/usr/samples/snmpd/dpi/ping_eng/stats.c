static char sccsid[] = "@(#)38	1.2  src/tcpip/usr/samples/snmpd/dpi/ping_eng/stats.c, snmp, tcpip411, GOLD410 2/7/94 16:09:24";
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
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/stats.c
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
 * This file contains the statics functions for the data the pings return 
 */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "snmp_dpi.h"
#include "stats.h"

struct rtt_stats *stats_list;

/*
 * FUNCTION:	new_stats_record
 *
 * INPUT:
 *	address		address of thing being pinged, used to index data
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	entry	if successful, a pointer to the stats entry
 *	null	if unsuccessful
 *
 * NOTE:
 *	The function adds a new entry in a linked list ordered by address.
 */
static struct rtt_stats *new_stats_record(address)
unsigned long address;
{
	struct rtt_stats *last, *next, *entry;

	entry = (struct rtt_stats *) malloc(sizeof(struct rtt_stats));
	if (entry == 0) return(0);


	entry->address = address;
	entry->RTTtries = 0;
	
	next = stats_list;
	last = 0;
	while (next != 0) {
		if (entry->address < next->address) break;
		last = next;
		next = next->next;
	}
	entry->next = next;
	if (last == 0) {
		stats_list = entry;
	} else {
		last->next = entry;
	}
	return (entry);
}

/*
 * FUNCTION:	find_stats_record
 *
 * INPUT:
 *	address		address of thing being pinged, used to index data
 *	compare		an integer value indicating the comparision to use
 *				-1	returns first entry > address
 *				 0	returns first entry = address
 *				 1 	returns first entry < address
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	entry	if successful, a pointer to the stats entry
 *	null	if unsuccessful
 *
 * NOTE:
 *	The function finds a entry in a linked list ordered by address.
 */
struct rtt_stats *find_stats_record(address, compare)
unsigned long address;
int compare;
{
	struct rtt_stats *next;
	int found;

	next = stats_list;
	found = 0;
	while (next != 0) {
		switch (compare) {
		case -1:
			if (address < next->address) found = 1;	
			break;
		case 0:
			if (address == next->address) found = 1;
			break;
		case 1:
			if (address > next->address) found = 1;
		}
		if (found) return(next);
		next = next->next;
	}
	return (0);	/* not found */
}


/*
 * FUNCTION:    count_stats_record
 *
 * INPUT:
 *  none
 *
 * OUTPUT:
 *  none
 *
 * RETURN:
 *    number of stats entries
 *
 * NOTE:
 *  The function counts entries in a linked list ordered by address.
 */
int count_stats_record()
{
  struct rtt_stats *statslist2;
  int i = 0;

  statslist2 = stats_list;
  while (statslist2) {
    statslist2 = statslist2 -> next;
    i++;
  }

  return i;
}


/*
 * FUNCTION:	retPINGdata
 *
 * INPUT:
 *	id		unknown, not used currently
 *	delta_ms	the millesecond delay from ping send and receive
 *	address		address of thing being pinged, used to index data
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 *
 * NOTE:
 *	The function finds an entry associated with address and updates the 
 *	stats by delta_ms.
 */
void retPINGdata(id, delta_ms, address)
int id;
int delta_ms;
unsigned long address;
{
	struct rtt_stats *entry;

	entry = find_stats_record(address, 0);
	if (entry == 0) return;

	entry->RTTsum += delta_ms;
	if (entry->RTTresponses++ == 0) {
		entry->minRTT = delta_ms;
		entry->maxRTT = delta_ms;
	} else {
		if (delta_ms < entry->minRTT) entry->minRTT = delta_ms;
		if (delta_ms > entry->maxRTT) entry->maxRTT = delta_ms;
	}
}

/*
 * FUNCTION:	queue_PINGS
 *
 * INPUT:
 *	address		address of thing to be pinged
 *	count		the number of times to ping it
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 *
 * NOTE:
 *	The function trys to find an entry associated with address.  If its not
 *      found, add an entry to the list.  Reinitialize the stats and set the
 *	number of times to ping and when it was started.
 */
void queue_PINGs(address, count)
unsigned long address;
int count;
{
	struct rtt_stats *entry;

	entry = find_stats_record(address, 0);
	if (entry == 0) {
		entry = new_stats_record(address);
		if (entry == 0) return;
	}
	if (entry->RTTtries == 0) {
		entry->minRTT = -1;
		entry->maxRTT = -1;
		entry->RTTsum = 0;
		entry->RTTresponses = 0;
	}
	if (entry->RTTtries < count) entry->RTTtries = count;
	entry->timestamp = time(0) - 1;
}

/*
 * FUNCTION:	next_timeout
 *
 * INPUT:
 *	none
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1	if no timers are active
 *	 1	if a timer is still active
 *
 * NOTE:
 *	The function looks to see if the timestamp in the data entries is 
 *	less than the current time.  If it is less, the function  sends a ping
 *	and decrements the number of times to ping.  1 is returned, if a
 *	ping still needs to be sent out.
 */
int next_timeout()
{
	struct rtt_stats *next;
	int active, rc;
	unsigned long cur_time;

	active = -1;
	cur_time = time(0);
	
	next = stats_list;
	while (next != 0) {
		if (next->timestamp < cur_time) {
			if (next->RTTtries > 0) {	/* work to do */
				/* send ping packet */
				rc = mkICMPreq(next->address, next->RTTtries);
				next->RTTtries--;
				next->timestamp = cur_time;
				if (rc < 0) next->RTTtries = 0; /* error */
			}
		}
		if (next->RTTtries > 0) active = 1;
		next = next->next;
	}
	return (active);	
}

/*
 * FUNCTION:	next_timeout
 *
 * INPUT:
 *	entry	stats data entry 
 *	var_id	id of variable to get
 *
 * OUTPUT:
 *	type	type of value being returned
 *
 * RETURN:
 *	value	Returns value of requested variable
 *
 * NOTE:
 *	The function looks to see if the timestamp in the data entries is 
 *	less than the current time.  If it is less, the function  sends a ping
 *	and decrements the number of times to ping.  1 is returned, if a
 *	ping still needs to be sent out.
 */
unsigned long getValue(entry, var_id, type)
struct rtt_stats *entry;
int var_id;
int *type;
{
	switch(var_id) {
	case 1:
		*type = SNMP_TYPE_INTERNET;
		return (entry->address);
	case 2:
		*type = SNMP_TYPE_NUMBER;
		return (htonl(entry->minRTT));
	case 3:
		*type = SNMP_TYPE_NUMBER;
		return (htonl(entry->maxRTT));
	case 4:
		*type = SNMP_TYPE_NUMBER;
		if (entry->RTTresponses == 0) return(htonl(~0));
		else return (htonl(entry->RTTsum / entry->RTTresponses));
	case 5:
		*type = SNMP_TYPE_NUMBER;
		return (htonl(entry->RTTtries));
	case 6:
		*type = SNMP_TYPE_NUMBER;
		return (htonl(entry->RTTresponses));
	case 8:
		*type = SNMP_TYPE_NUMBER;
		return (htonl((long)count_stats_record()));
	}
}
