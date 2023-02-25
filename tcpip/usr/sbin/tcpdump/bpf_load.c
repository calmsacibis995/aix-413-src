static char sccsid[] = "@(#)27	1.2  src/tcpip/usr/sbin/tcpdump/bpf_load.c, tcpip, tcpip411, GOLD410 10/13/93 09:49:45";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: MSGSTR
 *		bpf_load
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <odmi.h>
#include <sys/mode.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/stat.h>
#include <net/bpf.h>

#include <nl_types.h>
#include "tcpdump_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TCPDUMP,n,s)

#define BPFMODE	S_IFCHR|S_IRUSR
#define BPFDRIVER	"/usr/lib/drivers/bpf"

/* loads the berkeley packet filter kernel extension */
bpf_load()
{
  long  bpf_major;
  long  bpf_minor;
  int   how_many;
  int   odmlock;
  int   i;
  int   bpfload;
  char  path[128];
  struct stat stat_buf;
  char dname[BUFSIZ];
  struct cfg_load load;
  struct cfg_kmod kmod;
  struct bpf_config bpf_config;

  bzero(&load, sizeof(load));
  bzero(&kmod, sizeof(kmod));

  if (odm_initialize() == -1) {
	fprintf(stderr,MSGSTR(ODMINIT, "tcpdump: odm_initialize failed!\n"));
	exit(1);
  }
  if ((odmlock=odm_lock("/etc/objrepos/config_lock", ODM_WAIT)) == -1) {
	fprintf(stderr,MSGSTR(ODMLOCK, "tcpdump: odm_lock failed!\n"));
	exit(1);
  }

  if ((bpf_major=genmajor("bpf")) == -1) { 
	fprintf(stderr, MSGSTR(NOMAJOR,
		"tcpdump: Could not get major number for bpf"));
	exit(1);

  } else {
	if (!(bpf_minor=getminor(bpf_major, &how_many, "bpf")))
		if (!(bpf_minor=genminor("bpf",bpf_major,0,NBPFILTER,1,1))) {
			fprintf(stderr, MSGSTR(NOMINOR,
				"tcpdump: getminor failed!"));
			exit(1);
		}
  }

  if (odm_unlock(odmlock) == -1) {
	fprintf(stderr, MSGSTR(ODMULOCK, "tcpdump: odm_unlock failed!\n"));
	exit(1);
  }
  odm_terminate();

  if (stat("/dev/bpf0",&stat_buf) == -1 && errno == ENOENT ) {
	for (i=0; i<NBPFILTER; i++) {
		sprintf(dname, "%s%d", "/dev/bpf", i);
		if ((mknod(dname,BPFMODE,makedev(bpf_major,i))) == -1) {
			fprintf(stderr, MSGSTR(NOCREATE,
				"tcpdump: error creating %s.\n"), dname);
			exit(1);
		}
  	}
  } 

  load.libpath = (char *)NULL;
  load.path = path;
  sprintf(load.path, "%s", BPFDRIVER);
  if (!(bpfload=sysconfig(SYS_QUERYLOAD, &load, sizeof(load))))
	if (sysconfig(SYS_SINGLELOAD, &load, sizeof(load)) == CONF_FAIL) {
		fprintf(stderr, MSGSTR(NOLOAD,
			"tcpdump: error loading /usr/lib/drivers/bpf\n"));
		exit(1);
	}

  if (!bpfload) {
	kmod.kmid = load.kmid;
	kmod.cmd = CFG_INIT;
	kmod.mdiptr = (caddr_t)&bpf_config;
	kmod.mdilen = sizeof(bpf_config);
	for (i=0; i<NBPFILTER; i++) {
		bpf_config.devno = makedev(bpf_major,i);
		if (sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod)) == CONF_FAIL) {
			fprintf(stderr,MSGSTR(NOENTRY,
				"Error calling entry point for "));
			perror(load.path);
			exit(1);
		}
	}
  }

  return(0);
}
