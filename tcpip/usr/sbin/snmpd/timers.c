static char sccsid[] = "@(#)05	1.1  src/tcpip/usr/sbin/snmpd/timers.c, snmp, tcpip411, GOLD410 1/21/94 17:37:20";
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
 * FILE:        src/tcpip/usr/sbin/snmpd/timers.c
 */

/* INCLUDES */
#include "snmpd.h"
#include "ddmibs.h"
#include "interfaces.h"

void update_timers();
extern void print_list();

void update_timers(is, timeout, rcvornot)
struct interface *is;
int timeout;
int rcvornot;
{
  unsigned int i = alarm(0);
  unsigned int j = 0;
  struct interface *is2;

  /* Get the least amount of time remaining */
  is2 = iftimers;
  while (is2 && is2 -> time_next && (is2 -> time_next -> timeout > 0))
    is2 = is2 -> time_next;

  if (!is2) 
    j = 0;
  else j = is2 -> timeout - i;

  /* Reset the current is's time information */
  if (!rcvornot)
      is -> flush_cache = 0;
  else is -> flush_rcv = 0;
  is -> timeout = j + timeout;

  /* Remove is from the list */
  is2 = iftimers;
  if (is2 == is) {
    iftimers = is2 -> time_next;
  }
  else {
    while (is2 && (is2 -> time_next != is))
      is2 = is2 -> time_next;
    if (is2)
      is2 -> time_next = is -> time_next;
    else advise(SLOG_EXCEPTIONS, MSGSTR(MS_TIMERS, TIMERS_1, 
		"Ran out of items in timer list."));
  }

  if ((iftimers == NULL) || (is -> timeout > iftimers -> timeout)) {
    is -> time_next = iftimers;
    iftimers = is;
  }
  else {
    is2 = iftimers;
    while (is2 -> time_next && is2 -> time_next -> timeout >= is -> timeout)
      is2 = is2 -> time_next;
    is -> time_next = is2 -> time_next;
    is2 -> time_next = is;
  }

  is2 = iftimers;
  while (is2 && (is2 -> timeout > 0)) {
    is2 -> timeout -= j;
    if (is2 -> timeout < 0) {
      is2 -> timeout = 0;
      is2 -> flush_cache = 1;
      is2 -> flush_rcv = 1;
      if (is -> datarcvtable)
        free(is -> datarcvtable);
      is -> datarcvtable = NULL;
    }
    is2 = is2 -> time_next;
  }

  is2 = iftimers;
  while (is2 -> time_next && (is2 -> time_next -> timeout > 0))
    is2 = is2 -> time_next;

  alarm(is2 -> timeout);
}

