static char sccsid[] = "@(#)51	1.8  src/bos/usr/sbin/crash/ndb.c, cmdcrash, bos411, 9434B411a 8/24/94 17:21:44";
/* 
 * COMPONENT_NAME: (CMDCRASH) 
 * 
 * FUNCTIONS: ndbmain, getcommand, show_ifnet, head_tcb, show_mbuf,
 *            show_sock, show_tcb,
 *            show_tcbcb, showudb, snd_mbuf, rcv_mbuf,getflags, 
 *            gettype, next_tcb, prev_tcb, next_udb, prev_udb, top_level,
 *            exit_func, prhelp, prock_sb, prmenu_ndb, prmenu_tcb, prmenu_udb,
              prmenu_ifnet, prmenu_sock, prmenu_mbuf, prinpcb, 
 *            ndbprsock, prtcpcb, readsymbol, readaddr, ndbprmbuf, 
 *            makeprintable, next_ifnet, prifnet,mbuf_next,mbuf_next_act,
 *	      mbuf_head_next, mbuf_head_act.
 *
 * ORIGINS: 27,3
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* ndb.c - a tool for examining network structures in the kernel 
   for a more descriptive explanation of its features read the long
   help info in prhelp().
*/

#include <storclass.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <netdb.h>		/* hostent struct */
#include <nlist.h>		/* nlist struct */
#define _KERNEL
#include <sys/file.h>
#undef _KERNEL
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>		/* mbuf struct */
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>	/* socketstruct */
#include <sys/errno.h>
#include <net/if.h>		/* ifnet struct */
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>	/* inpcb struct */
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>	/* tcpcb struct */
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <unpack.h>
#include "ndb.h"
#include "crash.h"
#include <sys/param.h>
#include "crash_msg.h"
#include <string.h>
extern nl_catd scmc_catd;
extern int dumpflag;
char line[256];

/* ndbmain reads in the tcb and udb data structures. Next it takes the
   subcommand input from the command line and begin subcommand processing */ 
ndbmain()
{
	int c = 1;
	struct nlist Tcbnl;
	struct nlist Udbnl;

	if (dumpflag == 0)
	    rflag = 1;


	Tcbnl.n_name = "tcb";
	Udbnl.n_name = "udb";

		srch_knlist(&Tcbnl, 1, sizeof(struct nlist));
		srch_knlist(&Udbnl, 1, sizeof(struct nlist));

#ifdef	XXX
		if (Tcbnl.n_value == 0) {
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_1, "0452-417: Cannot find tcb\n") );
		return 0;
	}
		  if (Udbnl.n_value == 0) {
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_2, "0452-418: Cannot find udb\n") );
		return 0;
	}
#endif	/* XXX */
	   tcbaddr = Tcbnl.n_value;
	   udbaddr = Udbnl.n_value;

	proto = NONE;
	level = 0;
	prompt1[0] = "ndb";

	tcbhd = udbhd = NULL;

	if (tcbaddr != NULL) {
		readaddr(tcbaddr, &tcbhd, sizeof(tcbhd));
		readaddr(udbaddr, &udbhd, sizeof(udbhd));
	}

	if (!tcbhd) {
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_3, "0452-419: It looks like inet isn't loaded.\n") );
	}

	curtcb = tcbhd;
	curudb = udbhd;
	curifnet = ifnethd;

	top_level();

	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_4, "type ? for help\n") );
	for (;;) {
		c = getcommand();
		if (c >= 0)
			if (cmdtab[c].fun) {
				(*cmdtab[c].fun)();
			} else
				break;
	}
	exit_func();
}
/* gets the input from the command line. */
getcommand()
{
	struct cmdent *c = cmdtab;
	char *cp, *ap, *bp, **av = args;
	int cmd = -1, num = 0, i;
	int j,n, value = 1;
	
	printf("\n");
	for (i=0; i<=level; i++)
		printf(prompt1[i]);
	printf("> ");
	if (fgets(line, 256, stdin) == NULL) {
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_5,  "0452-420: ndb: error reading input, goodbye.\n") );
		exit(1);
	}
	/* nuke the newline */
	line[strlen(line)-1] = '\0';
	cp = line;
	SKIPDELIM(cp);
	ap = cp;
	if (mbufflag == 0)  {
	 SKIPTOKEN(ap); /* move past initial command */
	 if (*ap)  {
		bp = ap;
		SKIPDELIM(ap);
		*bp = '\0';
	 }

	 if (*cp == '\0') { 	/* NULL input == do last command */
		return(0);
	 }   
	 while (c->cmd) {
		if (strncmp(cp, c->cmd, strlen(line)) == 0) {
			if (cmd != -1) {
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_6,  "0452-421: that wasn't a unique command\n") );
				return(-1);
		        } else
				cmd = num;
		}
		c++; num++;
	   }
	   if (cmd == -1)
		   printf( catgets(scmc_catd, MS_crsh, NDB_MSG_7,"0452-422: unknown command\n") );
	   else {    /* process args */
		   lastcommand = cmd;
		   memset(args, 0, sizeof(args));
		   if (*ap)
		      PARSE_LIST(ap, av);
		}
	 }
	 else {
	 if (*cp == '\0') { 	/* NULL input == do last command */
		return(0);
	 } 
	 while (c->cmd) {
		if (strncmp(cp, c->cmd, strlen(line)) == 0) {
			cmd = num;
        	}    
		c++; num++;
	   }

	if (cmd == -1 || cmd == 1)
	 {
	   n = strlen(cp);
	   for ( j = 0; j <= n -1; ++j)
	    {
	     if ( ! isxdigit(cp[j]) )
		value = 0;
            }
	  }
	  if ( value == 0 )
		printf( catgets(scmc_catd, MS_crsh, NDB_MSG_23,"0452-428: Not a valid hexadecimal number for an address\n") );
	else {	  /* process args */
		if ( cmd != 0 && cmd != 2 && cmd != 3)
		cmd = 1;
		lastcommand = cmd;
		memset(args, 0, sizeof(args));
		if (*ap)
			PARSE_LIST(ap, av);
	}
      }
	return(cmd);
}
/* shows the ifnet structures */
show_ifnet()
{
	char **av = args;
	caddr_t addr, ifp;

	level = 1;
	cmdtab = ifnettab;
	prompt1[1] = ".ifnet";

	if (*av == 0) {
		/* the ifnet symbol is a pointer to an ifnet struct.
		   so we have to read the value at that address, then
		   use the value at that address as the pointer to
		   the first ifnet structure. */



		if (!readaddr(Ifnet->n_value, &ifp, sizeof(ifp)))
			return;

		curifnet = ifnethd = ifp;
		prifnet(curifnet);
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
			return;
		}
		curifnet = addr;
		prifnet(curifnet);
	}
}
/* shows the head tcb in the chain */
head_tcb()
{
	curtcb = tcbhd;
	prinpcb(curtcb);
}
/* shows mbuf structure at that address */
show_mbuf()
{
	char **av = args;
	caddr_t addr;

	if (cmdtab != mbuftab) {
		cmdtab = mbuftab;
		level++;
		prompt1[level] = ".mbuf";
		mbufflag = 1 ;
	}
	
	if (*av != 0) {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
			return;
        	}
		curmbuf = addr;
	        ndbprmbuf(curmbuf);
        } 	 
 }


/* shows the sockets from the file table */
show_sock()
{
	char **av = args;
	caddr_t addr;
	struct socket sockbuf;

	/* must have an argument */
	if (*av == 0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_11, "0452-426: An address is required\n") );; 
		return;
	}
	sscanf(*av, "%x", &addr);
	if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
		return;
	}
	readaddr(addr, &sockbuf, sizeof(sockbuf));
	curtcb = sockbuf.so_pcb;
	ndbprsock(addr);
}
/* shows the tcb's */
show_tcb()
{
	char **av = args;
	caddr_t addr;

	level = 1;
	cmdtab = tcbtab;
	prompt1[1] = ".tcb";
	proto = TCP;

	if (*av == 0)
		prinpcb(curtcb);
	else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
			return;
		}
		curtcb = addr;
		prinpcb(curtcb);
	}
}
/* reads the control blocks */
show_tcpcb()
{
	char **av = args;
	caddr_t addr;
	struct inpcb inpcb;
	struct tcpcb tcpcbuf;

	level = 1;
	cmdtab = tcbtab;
	prompt1[1] = ".tcpcb";
	proto = TCP;
	
	if (*av == 0) {
		readaddr(curtcb, &inpcb, sizeof(inpcb));
		prtcpcb(inpcb.inp_ppcb);
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
			return;
		}
		readaddr(addr, &tcpcbuf, sizeof(tcpcbuf));
		curtcb = (caddr_t)tcpcbuf.t_inpcb;
		prtcpcb(addr);
	}
}
/* shows the udb's */
show_udb()
{
	char **av = args;
	caddr_t addr;

	level = 1;
	cmdtab = udbtab;
	prompt1[1] = ".udb";
	proto = UDP;

	if (*av == 0)
		prinpcb(curudb);
	else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_8, "0452-423: Invalid argument: %s\n") , *av); 
			return;
		}
		prinpcb(addr);
	}
}
/* shows the send mbufs */
snd_mbuf()
{
	struct inpcb inpcb;
	struct socket sockbuf;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	readaddr(inpcb.inp_socket, &sockbuf, sizeof(sockbuf));

	if (sockbuf.so_snd.sb_mb) {
		curmbuf = (caddr_t)sockbuf.so_snd.sb_mb;
		mbufhdnext = mbufhdact = curmbuf;
		show_mbuf();
	} else
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_12, "the send buffer has no mbufs\n") ); 
}
/* shows the receive mbufs */
rcv_mbuf()
{
	struct inpcb inpcb;
	struct socket sockbuf;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	readaddr(inpcb.inp_socket, &sockbuf, sizeof(sockbuf));

	if (sockbuf.so_rcv.sb_mb)
		ndbprmbuf(sockbuf.so_rcv.sb_mb);
	else
 		      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_13, "the receive buffer has no mbufs\n") );
}
/* shows the next mbuf in the act chain */
mbuf_next_act()
{
	struct mbuf mb;

	readaddr(curmbuf, &mb, sizeof(mb));
	if (mb.m_act) {
		curmbuf = (caddr_t)mb.m_act;
		mbufhdnext = curmbuf;
		ndbprmbuf(curmbuf);
	} else
       printf( catgets(scmc_catd, MS_crsh, NDB_MSG_32, "at end of act chain\n")
);
}
/* shows the next mbuf  in the act chain */
mbuf_next()
{
	struct mbuf mb;

	readaddr(curmbuf, &mb, sizeof(mb));
	if (mb.m_next) {
        	curmbuf = (caddr_t)mb.m_next;
		ndbprmbuf(curmbuf);
	} else
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_33, "at end of mbuf list\n") )
;
}
/* shows the first mbuf in the act chain */
mbuf_head_act()
{
      curmbuf = mbufhdact;
      ndbprmbuf(curmbuf);
}
/* shows the head mbuf in the "next" chain */
mbuf_head_next()
{
      curmbuf = mbufhdnext;
      ndbprmbuf(curmbuf);
}
/* used to get flags of socket, tcp, and ifnet */
char *
getflags(val, flaglist)
	int val;
	struct flagent *flaglist;
{
	static char buf[128];
	struct flagent *f = flaglist;

	buf[0] = '\0';
	while (f->name) {
		if (val & f->bit) {
			if (buf[0]) strcat(buf, "|");
			strcat(buf, f->name);
		}
		f++;
	}
	return(buf);
}
/* used to get socket type, mbuf type, tcb type while printing the 
   corresponding information */
char *
gettype(val, typelist, dflt)
	int val;
	struct typeent *typelist;
	char *dflt;
{
	struct typeent *t = typelist;

	while (t->name) {
		if (val == t->val) {
			return(t->name);
		}
		t++;
	}
	return(dflt);
}
/* shows the next tcb/socket with respect to the current tcb. */
next_tcb()
{
	static struct inpcb inpcb;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	curtcb = (caddr_t)inpcb.inp_next;
	prinpcb(curtcb);
}
/* shows the previous tcb with respect to the current tcb. */
prev_tcb()
{
	static struct inpcb inpcb;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	curtcb = (caddr_t)inpcb.inp_prev;
	prinpcb(curtcb);
}
/* shows the next udb with respect to the current udb */	
next_udb()
{
	static struct inpcb inpcb;
	
	readaddr(curudb, &inpcb, sizeof(inpcb));
	curudb = (caddr_t)inpcb.inp_next;
	prinpcb(curudb);
}
/* shows the previous udb with respect to the current udb */
prev_udb()
{
	static struct inpcb inpcb;
	
	readaddr(curudb, &inpcb, sizeof(inpcb));
	curudb = (caddr_t)inpcb.inp_prev;
	prinpcb(curudb);
}
/* returns to the main menu */	
top_level()
{
	level = 0;
	cmdtab = ndbtab;
	mbufflag = 0;
}
/* prints the end of the ndb */
exit_func()
{
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_14, "\nEnd of NDB\n")); 
	return(0);
}
/* prints detailed help */
prhelp()
{
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_15, "Welcome to ndb, a network debugger for network kernel structures\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_24, "At any prompt type a '?' for context sensitive help.\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_25, "There are two main functions currently supported by ndb:\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_26, "\t* show sockets and related control blocks and buffers,\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_27, "allowing you to move back and forth along the chain of control blocks\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_28, "\t* show interface structures\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_29, "following the chain from the head to the interface list\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_30, "THINGS TO REMEMBER :\n"));
	printf( catgets(scmc_catd, MS_crsh, NDB_MSG_31, " - When you just hit ENTER, the option menu will be displayed.\n\n"));

}

prmenu_ndb()
{
      /* Make sure that these message is consistent with that in ndb.h */       
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_35,"help           a more lengthy help text\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_36,"tcb [addr]     show TCBs, default is HEAD TCB\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_37,"udb [addr]     show UDBs, default is HEAD UDB\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_38,"sockets addr   show sockets at given addr\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_39,"mbuf [addr]    show mbuf at address, default is enter mbuf menu\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_40,"ifnet [addr]   show ifnet structures, default is \"ifnet\"\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_41,"quit           exit ndb\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));

}

prmenu_tcb()
{
      /* Make sure that these message is consistent with that in ndb.h */       
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_43,"next           go to next tcb/socket\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_44,"previous       go to previous tcb/socket\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_45,"tcb [addr]     reread tcb/socket, default is current\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_46,"ctrlblk [addr] read the control block\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_47,"sendmbufs      show the send mbufs\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_48,"recvmbufs      show the receive mbufs\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_49,"sock addr      show socket at given address\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_50,"head           show the head tcb in the chain\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_51,"quit           return to top_level\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));
};


prmenu_udb()
{
      /* Make sure that these message is consistent with that in ndb.h */	
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n")); 
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_52,"next           go to next udb\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_53,"previous       go to previous udb\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_51,"quit           return to top_level\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));
};


prmenu_ifnet()
{
      /* Make sure that these message is consistent with that in ndb.h */
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_54,"head           go to head of ifnet list\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_55,"next           go to next ifnet struct\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_51,"quit           return to top_level\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));
};

prmenu_sock()
{
      /* Make sure that these message is consistent with that in ndb.h */
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_56,"socket addr    show socket at addr\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_51,"quit           return to top_level\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));
};

prmenu_mbuf()
{
      /* Make sure that these message is consistent with that in ndb.h */
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_16,"\n\nHelp stuff: \n\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_34,"?              help - you're reading it\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_57,"addr           show mbuf at address\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_51,"quit           return to top_level\n"));
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_42,"xit            exit ndb\n"));
};

/* prints the inpcb info */
prinpcb(inp_addr)
	caddr_t inp_addr;
{
	struct inpcb tcbbuf, *inp;
	struct hostent *hp;
	char lhost[128], fhost[128];

	readaddr(inp_addr, &tcbbuf, sizeof(tcbbuf));
	inp = &tcbbuf;

	hp = gethostbyaddr(&inp->inp_faddr, sizeof(inp->inp_faddr), AF_INET);
	strcpy(fhost, (hp) ? hp->h_name : "NONE");
	
	hp = gethostbyaddr(&inp->inp_laddr, sizeof(inp->inp_laddr), AF_INET);
	strcpy(lhost, (hp) ? hp->h_name : "NONE");

      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_19,  
	"    %s%s----------- INPCB  INFO -------------------\n"),
	       (proto == TCP) ? "TCB" : "UDB",
	       ((proto == TCP && curtcb == tcbhd) ||
		(proto == UDP && curudb == udbhd)) ? "-HEAD" :" ------");

	printf("    next:0x%08x    prev:0x%08x    head:0x%08x\n",
	       inp->inp_next, inp->inp_prev, inp->inp_head);
	 printf("    ppcb:0x%08x  inp_socket:0x%08x\n\n",
		inp->inp_ppcb, inp->inp_socket);
	printf("    lport:%5d   laddr:0x%08x (%s)\n",
	       inp->inp_lport, inp->inp_laddr, lhost);
	printf("    fport:%5d   faddr:0x%08x (%s)\n\n",
	       inp->inp_fport, inp->inp_faddr, fhost);

	ndbprsock(inp->inp_socket);
}
/* prints the socket information */
ndbprsock(sockaddr)
	caddr_t sockaddr;
{
	struct socket sockbuf, *so;
	
	if (sockaddr == 0)
		return;

	readaddr(sockaddr, &sockbuf, sizeof(sockbuf));
	so = &sockbuf;
	
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_20,  
	"    --------------------- SOCKET INFO -------------------\n"));
	printf("    type:0x%04x (%s)    opts:0x%04x  (%s)\n",
	       so->so_type, gettype(so->so_type, socktypes, "BOGUS"),
	       so->so_options, getflags(so->so_options, sockoptflags));
	printf("    state:0x%04x  (%s)\n",
	       so->so_state, getflags(so->so_state, sockstates));
	printf("    linger:0x%04x  pcb:0x%08x  proto:0x%08x\n",
	       so->so_linger, so->so_pcb, so->so_proto);
	printf("    q0:0x%08x  q0len:%5d q:0x%08x\n", 
	       so->so_q0, so->so_q0len, so->so_q);
	printf("    qlen:%5d  qlimit:%5u  head:0x%08x\n",
	       so->so_qlen, so->so_qlimit, so->so_head);
	printf("    timeo:%5d  error:%5u  oobmark:%5u  pgid:%5u\n\n",
	       so->so_timeo, so->so_error, so->so_oobmark,
	       so->so_pgid);
	ndbprsock_sb("snd", &(so->so_snd));
	printf("\n");
	ndbprsock_sb("rcv", &(so->so_rcv));
}
/* print the socket information of send and receive socket buffers */
ndbprsock_sb( which, sb )
	char *which;		/* say which one you are */
	struct sockbuf *sb;
{
	printf("    %s: cc:%5u  hiwat:%5u  mbcnt:%5u  mbmax:%5u\n", which,
	    	sb->sb_cc, sb->sb_hiwat, sb->sb_mbcnt, sb->sb_mbmax);
	printf("    lowat:%5u  mb:0x%08x  events:0x%4x\n",
	    	sb->sb_lowat, sb->sb_mb, sb->sb_reqevents);
	printf("    iodone:0x%08x  ioargs:0x%08x ",
		sb->sb_iodone,sb->sb_ioarg);
	printf("    flags:0x%04x  (%s)\n",
	       sb->sb_flags, getflags(sb->sb_flags, sbflags));
}
/* prints the tcp cb information */
prtcpcb(tcpcbaddr)
	caddr_t tcpcbaddr;
{
	struct tcpcb tcpcb, *tp;
	int i;

	readaddr(tcpcbaddr, &tcpcb, sizeof(tcpcb));
	tp = &tcpcb;
	
        printf( catgets(scmc_catd, MS_crsh, NDB_MSG_21,  
	"  ------------------------ TCPCB ----------------------\n"));
	printf("  seg_next 0x%08x  seg_prev 0x%08x  t_state 0x%02x  (%s)\n",
	       tp->seg_next, tp->seg_prev, tp->t_state,
	       gettype(tp->t_state, tcpstates, ""));
	printf("  timers:");
	for (i=0; i<TCPT_NTIMERS; i++)
		printf("   %s:%d", gettype(i, tcptimers, ""), tp->t_timer[i]);
	printf("\n");
	printf("  t_txtshift %d  t_txtcur %d  t_dupacks %d  t_maxseg %d  t_force %1d\n",
	       tp->t_rxtshift, tp->t_rxtcur, tp->t_dupacks,
	       tp->t_maxseg, tp->t_force);
	printf("  flags:0x%04x  (%s)\n",
	       tp->t_flags, getflags(tp->t_flags, tcpcbflags));
	printf("  t_template 0x%08x  inpcb 0x%08x  snd_cwnd:%05d  snd_ssthresh:%05d\n",
	       tp->t_template, tp->t_inpcb, tp->snd_cwnd, tp->snd_ssthresh);
	printf("  snd_una=%05d  snd_nxt=%05d  snd_up=%05d  snd=wl1=%05d  snd_wl2=%05d  iss=%05d\n",
	       tp->snd_una, tp->snd_nxt, tp->snd_up, tp->snd_wl1, tp->snd_wl2,
	       tp->iss);
	printf("  snd_wnd:%6d  rcv_wnd:%6d\n", tp->snd_wnd, tp->rcv_wnd);
	printf("  t_idle=%05d  t_rtt=%05d t_rtseq=%05d  t_srtt=%05d  t_rttvar=%05d\n",
	       tp->t_idle, tp->t_rtt, tp->t_rtseq, tp->t_srtt, tp->t_rttvar);
	printf("  max_sndwnd=%05d  t_iobc=0x%02x  t_oobflags=0x%02x (%s)\n",
	       tp->max_sndwnd, tp->t_iobc, tp->t_oobflags,
	       gettype(tp->t_oobflags, oobflags, ""));
}
/* prints the mbuf information */
ndbprmbuf(addr)
	caddr_t addr;
{
	struct	mbuf	mbuf;
	char		*type;
	int		dptr;
	int		buf[4];
	register int	i,j,size;
	register char 	*p;
	
	if ((int)addr & (sizeof(struct mbuf) - 1)) {
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_17, "0452-427: invalid mbuf ptr:%8x, must be on %d byte boundary!\n"),addr, sizeof(struct mbuf)); 
		return;
	}
	if (!readaddr(addr, &mbuf, sizeof(mbuf)))
		return;
	
	dptr = mbuf.m_data;
	printf("\n    addr:0x%08x    dptr:0x%08x     m_act:0x%08x\n",
	       addr, dptr, mbuf.m_act);
	printf("    m_next:0x%08x  data:0x%08x  len:%5d  type:%s\n",
	       mbuf.m_next, mbuf.m_data, mbuf.m_len,
	       gettype(mbuf.m_type, mbuftypes, "weird"),
	       mbuf.m_act);
	printf("    ---------------------------------------\n");

	if (mbuf.m_len > 0 && mbuf.m_len <= MLEN)
		size = mbuf.m_len;
	else
		size = MLEN;
	
	for (; size>0; dptr += 4*sizeof(int)) {
		p = (char *)buf;
		
		printf("    ");
		if (!readaddr(dptr, buf, sizeof(buf)))
			return;
		
		for (i=j=0; j<16;j++) {
			if (size) {
				size--; 
				i++;
				printf("%02x",*p++);
			} else
				printf("  ");
			if (!((j+1)%4))
				printf(" ");
		}
		printf(" |%-16s|\n",
		       makeprintable(buf, size>0 ? 16 : i));
	}
}
/* calls the ndbprmbuf to print mbuf structure */
char *
makeprintable(buf, len)
	unsigned char *buf;
	int len;
{
	int i;
	static unsigned char outbuf[128];

	for (i=0; i<len; i++) {
		if (buf[i]< 0x20 || buf[i] > 0xFE)
			outbuf[i] = '.';
		else
			outbuf[i] = buf[i];
	}
	outbuf[i] = '\0';
	return(outbuf);
}
/* shows the next ifnet structures */
next_ifnet()
{
	struct ifnet ifnet;
	
	readaddr(curifnet, &ifnet, sizeof(ifnet));
	if (ifnet.if_next) {
		curifnet = (caddr_t)ifnet.if_next;
		prifnet(curifnet);
	} else
      printf( catgets(scmc_catd, MS_crsh, NDB_MSG_18, "at the end of ifnet chain\n") ); 
}
/* prints the ifnet information */
prifnet(addr)
	caddr_t addr;
{

	struct ifaddr ifaddr,*ifaddrp;
	struct ifnet interface, *i;
	char name[12], *cp, host[64];
	struct sockaddr_in *sin;
	struct hostent *hp;
	int n;
	struct sockaddr sa;

	ifaddrp = (struct ifaddr *)malloc(sizeof(struct ifaddr));
	if (!ifaddrp) {
	 	perror("malloc");
		return;
	}
	i = &interface;
	if (!readaddr(addr, &interface, sizeof(interface))) 
	     return;
	if (i->if_name && readaddr(i->if_name, name, sizeof(name)-1)) {
		name[sizeof(name)-2] = '\0';
		cp = strchr(name, '\0');
		*cp++ = i->if_unit + '0';
		if ((i->if_flags&IFF_UP) == 0)
			*cp++ = '*';
		*cp = '\0';
	} else
		name[0] = '\0';

        printf( catgets(scmc_catd, MS_crsh, NDB_MSG_22,  
	"    ---------------------- IFNET INFO -----(@0x%08x)----------\n"),
	       addr);
	printf("    name: %-8.8s  unit:%01d  mtu:%10lu  if_next:0x%08x\n",
	       name, i->if_unit, i->if_mtu, i->if_next);
	printf("    flags:0x%08x \n\t(%s)\n",
	       i->if_flags, getflags(i->if_flags, ifflags));
	printf("    timer:%05d  metric:%02d\n\n", i->if_timer, i->if_metric);
        if (!readaddr(i->if_addrlist,ifaddrp,sizeof(struct ifaddr)))
	     return;
	while ((ifaddrp->ifa_addr) && readaddr(ifaddrp->ifa_addr, &sa, sizeof(struct sockaddr)) == sizeof(struct sockaddr)) {
		switch (sa.sa_family) {
		case AF_UNSPEC:
			printf("\tUNIX family socket, no address\n");
			break;
		case AF_INET:
			sin = (struct sockaddr_in *)&sa;
			printf("\t    address: %s  ",
			       inet_ntoa(sin->sin_addr));
			hp = gethostbyaddr(&sin->sin_addr,
					   sizeof(sin->sin_addr),
					   AF_INET);
			strcpy(host, (hp) ? hp->h_name : "NONE");
			printf("hostname: %s\n", host);
			break;
		}
       	if ((ifaddrp->ifa_next == NULL) || (readaddr(ifaddrp->ifa_next,ifaddrp,sizeof(struct ifaddr)) != sizeof (struct ifaddr))) 
			break;
	}
	free(ifaddrp);
	printf("\n");
	printf("    ifq_head:0x%08x  if_init():0x%08x      ipackets:%08d\n",
	       i->if_snd.ifq_head, i->if_init, i->if_ipackets);
	printf("    ifq_tail:0x%08x  if_output():0x%08x    ierrors: %05d\n",
	       i->if_snd.ifq_tail, i->if_output, i->if_ierrors);
	printf("    ifq_len:%05d        if_ioctl():0x%08x     opackets:%08d\n",
	       i->if_snd.ifq_len, i->if_ioctl, i->if_opackets);
	printf("    ifq_maxlen:%05d     if_reset():0x%08x     oerrors: %05d\n",
	       i->if_snd.ifq_maxlen, i->if_reset, i->if_oerrors);
	printf("    ifq_drops:%05d      if_watchdog():0x%08x\n",
	       i->if_snd.ifq_drops, i->if_watchdog);
	
}
