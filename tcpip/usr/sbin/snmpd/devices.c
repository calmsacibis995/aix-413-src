static char sccsid[] = "@(#)98	1.4  src/tcpip/usr/sbin/snmpd/devices.c, snmp, tcpip411, GOLD410 3/23/94 14:14:18";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/devices.c
 */

#include <stdio.h>
#include <isode/snmp/io.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "snmpd.h"
#include "interfaces.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/cdli.h>
#include <sys/ndd_var.h>
#include <sys/fddi_demux.h>

extern int SNMP_PDU_variable_position;
extern int debug;

#ifdef DEBUG
extern void print_ethernet();
extern void print_token_ring();
extern void print_generic();
#endif /* DEBUG */

int open_device();
void close_device();
int ask_device();
int get_rcvtable();
int comp_addr();
int set_device();

#define MAXADDRESSSIZE	6
#define EXTRARCVTABLE	3
#define DEFTABLESIZE	10
#define SET 		4
#define OPEN		5

int open_device(is)
struct interface *is;
{
  int fd = -1;
  char devname[20];
  struct sockaddr_ndd_8022 sa;
  int tmp = 1;

  sprintf (devname, "%s", is -> ndd_name);

  if (debug >= 1)
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_2,
	   "%s called for %s for doing a %d"), "open_device", devname, OPEN);

  fd = socket(AF_NDD, SOCK_DGRAM, 0); 
  if (fd < 0) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_3,
	   "open_device: Unable to open AF_NDD socket."));
    is -> mib_envt = ENVTNOTOK;
    return (NOTOK);
  }
  is -> fi_fd = fd;

  sa.sndd_8022_family = AF_NDD;
  sa.sndd_8022_len = sizeof(struct sockaddr_ndd);
  if (is -> ifn_type == IFT_FDDI)
  {
    sa.sndd_8022_filterlen = sizeof(struct fddi_dmx_filter);
    sa.sndd_8022_filtertype = FDDI_DEMUX_SMT;
  }
  else sa.sndd_8022_filterlen = 0;
  bcopy(devname, sa.sndd_8022_nddname, sizeof(sa.sndd_8022_nddname));

  if (is -> ifn_type == IFT_FDDI)
  {
    if (bind(fd, &sa, sizeof(sa))) 
    {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_4,
	     "open_device: errno = %d Unable to bind to device driver."), 
	     errno);
      close_device(is);
      is -> mib_envt = ENVTNOTOK;
      return (NOTOK);
    }
  }

  if (connect(fd, &sa, sizeof(sa))) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_4,
	   "open_device: errno = %d Unable to connect to device driver."), 
	   errno);
    close_device(is);
    is -> mib_envt = ENVTNOTOK;
    return (NOTOK);
  }

  if (ioctl(fd, FIONBIO, &tmp) == NOTOK)
  {
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_INTER, INTER_21,
	     "open_device: file descripter cannot be made non-blocking"));
      close_device(is);
      is -> mib_envt = ENVTNOTOK;
      return (NOTOK);
  } 

  is -> mib_envt = ENVTOK;
  return(OK);
}


void close_device(is)
struct interface *is;
{
    close(is -> fi_fd);
    is -> fi_fd = NOTOK;

    return;
}


int ask_device(is, param)
struct interface *is;
int param;
{
  int rc;
  char devname[20];
  struct nddctl io_block;
  int fd;

  sprintf (devname, "%s", is -> ndd_name);

  if (open_device(is) == NOTOK)
      return(NOTOK);
  fd = is -> fi_fd;

  if (debug >= 1)
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_2,
	   "%s called for %s doing a %d"), "ask_device", devname, param);

  if (fd == NOTOK) 
  {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_7,  
	   "%s: Bad file descripter for device %s"), "ask_device", devname);
    is -> mib_envt = ENVTNOTOK;
    return(NOTOK);
  }

  switch (is -> ifn_type)
  {
    case IFT_ETHER:
    case IFT_ISO88023:
      io_block.nddctl_buflen = sizeof(ethernet_all_mib_t);
      break;

    case IFT_FDDI:
      io_block.nddctl_buflen = sizeof(generic_mib_t);
      break;

    case IFT_ISO88025:
      io_block.nddctl_buflen = sizeof(token_ring_all_mib_t);
      break;

    default:
      advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_5,
	     "%s: Unknown method for device %s"), "ask_device", devname);
      is -> mib_envt = ENVTNOTOK;
      close_device(is);
      return(NOTOK);
  }

  io_block.nddctl_buf = (param == QUERY) ? (caddr_t) is -> environ :
					   (caddr_t) is -> datacache;
  rc = ioctl (fd, (param == QUERY) ? NDD_MIB_QUERY : NDD_MIB_GET, 
	      (caddr_t)&io_block);

  if (rc == NOTOK) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_6,
           "%s: ioctl on device %s failed with errno = %d"), "ask_device",
	   devname, errno);

    if (errno == EOPNOTSUPP)
    {
        bzero(is -> datacache, sizeof(combo_mibs_t)); 
        bzero(is -> environ, sizeof(combo_mibs_t)); 
	if (param == QUERY)
	    is -> flush_cache = 0;
	close_device(is);
        is -> mib_envt = ENVTOK;
	return(OK);
    }
    
    is -> flush_cache = 1;
    close_device(is);
    is -> mib_envt = ENVTNOTOK;
    return(NOTOK);
  }

  is -> mib_envt = ENVTOK;
  close_device(is);
  return(OK);
}


int get_rcvtable(is) 
struct interface *is;
{
  int rc;
  int i;
  int fd;
  char devname[20];
  char *tmp;
  int pos;
  struct nddctl io_block;

  sprintf(devname, "%s", is -> ndd_name);

  if (debug >= 1)
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_2,
	   "%s called for %s doing a %d"), "get_rcvtable", devname, GET);

  if (open_device(is) == NOTOK)
      return(NOTOK);
  fd = is -> fi_fd;

  if (fd == NOTOK) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_7, 
	   "%s: Bad file descripter for device %s"), "get_rcvtable", devname);
    is -> mib_envt = ENVTNOTOK;
    return(NOTOK);
  }

  /* Free existing table, we are about to allocate a new one */ 
  if (is -> datarcvtable)
    free(is -> datarcvtable);

  /* set default table size if we are supposed to flush the datacache */
  if (is -> flush_cache)
    is -> datacache -> ethernet_all.Generic_mib.RcvAddrTable = DEFTABLESIZE;

  /* Set the buffer size we are sending to the driver */
  i = sizeof(int) + (sizeof(ndd_mib_addr_elem_t) + MAXADDRESSSIZE) * 
      (EXTRARCVTABLE+is -> datacache -> ethernet_all.Generic_mib.RcvAddrTable);

  /* allocate the new rcvtable */
  if ((is -> datarcvtable = (char *)calloc(1, i)) == NULL) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
	   "get_rcvtable");
    close_device(is);
    is -> mib_envt = ENVTNOTOK;
    return (NOTOK);
  }

  io_block.nddctl_buflen = i;
  io_block.nddctl_buf = (caddr_t) is -> datarcvtable;
  rc = ioctl (fd, NDD_MIB_ADDR, (caddr_t)&io_block);

  /* 
   *  If the read fails and the errno is not E2big, close the socket or remove
   *  the support for this mib.
   *
   *  If E2BIG is the errno, we did not use a big enough buffer, so
   *  we should force a rcvtable refresh and update the size of the 
   *  buffer we want;  The driver sent us valid data that would fit in this
   *  buffer.
   */
  if ((rc == NOTOK) && (errno != E2BIG)) 
  {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_6,
           "%s: ioctl on device %s failed with errno = %d"), "get_rcvtable", 
	   devname, errno);

    if (is -> datarcvtable)
      free(is -> datarcvtable);

    if (errno == EOPNOTSUPP)
    {
        bzero(is -> datacache, sizeof(combo_mibs_t)); 
        bzero(is -> environ, sizeof(combo_mibs_t)); 
        close_device(is);
        is -> mib_envt = ENVTOK;
	return(NOTOK);
    }

    is -> flush_rcv = 1;
    close_device(is);
    is -> mib_envt = ENVTNOTOK;
    return(NOTOK);
  }
  else if ((rc == NOTOK) && (errno == E2BIG))
    is -> flush_rcv = 1;

  close_device(is);
  is -> datacache -> ethernet_all.Generic_mib.RcvAddrTable = 
				*(int *)(is -> datarcvtable);

  /*
   *  Sort rcvtable so that we can have it in order for the get routines.
   */
  tmp = is -> datarcvtable;
  qsort((void *)((ndd_mib_addr_t *)tmp) -> mib_addr, 
	((ndd_mib_addr_t *)tmp) -> count, sizeof(ndd_mib_addr_elem_t) + 4,
	comp_addr);

  is -> mib_envt = ENVTOK;
  return(OK);
}

/* comparison function for addresses */
int comp_addr(a, b)
ndd_mib_addr_elem_t *a;
ndd_mib_addr_elem_t *b;
{
  int results = 0;
  int i;
  unsigned char *ac = (unsigned char *)a -> address;
  unsigned char *bc = (unsigned char *)b -> address;
  int len;

  len = a -> addresslen > b -> addresslen ? b -> addresslen : a -> addresslen;
  for (i = 0; i < len && results == 0; i++, ac++, bc++) {
    if (*ac < *bc) results = -1;
    if (*ac > *bc) results = 1;
  }

  if (results == 0 && a -> addresslen != b -> addresslen)
  {
      if (a -> addresslen < b -> addresslen)
	  results = -1;
      else results = 1;
  }

  return (results);
}


int set_device(is)
struct interface *is;
{
  int rc, i;
  int fd;
  char devname[20];
  ndd_mib_t *ptr;
  struct nddctl io_block;

  sprintf (devname, "%s", is -> ndd_name);

  if (debug >= 1)
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_2,
	   "%s called for %s doing a %d"), "set_device", devname, SET);

  if (open_device(is) == NOTOK)
      return (int_SNMP_error__status_noSuchName);
  fd = is -> fi_fd;

  io_block.nddctl_buflen = is -> setsize;
  io_block.nddctl_buf = (caddr_t) is -> setrequests;
  rc = ioctl (fd, NDD_MIB_SET, (caddr_t) &io_block);

  if (rc == NOTOK) {
    advise(SLOG_EXCEPTIONS, MSGSTR(MS_DEVICES, DEVICES_6,
	   "%s: ioctl on device %s failed with errno = %d"), "set_device",
	   devname, errno);

    if (errno == EOPNOTSUPP)
    {
        bzero(is -> datacache, sizeof(combo_mibs_t)); 
        bzero(is -> environ, sizeof(combo_mibs_t)); 
        is -> flush_cache = 1;
	close_device(is);
        is -> mib_envt = ENVTOK;
        return (int_SNMP_error__status_noSuchName);
    }
    close_device(is);
    is -> mib_envt = ENVTNOTOK;
    return(int_SNMP_error__status_genErr);
  }

  close_device(is);
  is -> mib_envt = ENVTOK;
  ptr = (ndd_mib_t *)(&is -> setrequests[sizeof(u_int)]);
  for (i = 0; i < ((ndd_mib_set_t *)(is -> setrequests)) -> count; i++)
  {
      switch (ptr -> status)
      {
	  case MIB_OK :
	      break;

	  case MIB_NOT_SETTABLE :
	      SNMP_PDU_variable_position = is -> setpositions[i];
	      return (int_SNMP_error__status_noSuchName);

	  case MIB_WRONG_STATE :
	  case MIB_FAILED :
	      SNMP_PDU_variable_position = is -> setpositions[i];
	      return (int_SNMP_error__status_genErr);

	  case MIB_BAD_VALUE :
	      SNMP_PDU_variable_position = is -> setpositions[i];
	      return (int_SNMP_error__status_badValue);

	  default:
	      SNMP_PDU_variable_position = is -> setpositions[i];
	      return (int_SNMP_error__status_genErr);
      }
      ptr = ptr + sizeof(mib_t) + sizeof(u_int) + sizeof(u_short) + 
	    ptr -> mib_len;
  }

  return(int_SNMP_error__status_noError);
}
