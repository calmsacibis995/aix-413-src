static char sccsid[] = "@(#)30	1.24  src/bos/usr/sbin/arp/arp.c, cmdnet, bos411, 9428A410j 2/2/94 08:50:53";
/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: main, file, set, 
 *            set, delete, dump, dump_et, dump_802_3,
 *            dump_802_5, x25_print, hwaddr_print, hwaddr_aton,
 *            hwaddr_fcs, fcs_print, _802_5_aton, _802_5_rton, usage  
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1990 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*
 * arp - display, set, and delete arp table entries
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netdb.h>
#include <nlist.h>
#include <net/if.h>
#include <net/if_types.h>
#include <netinet/in_netarp.h>
#include <locale.h>
#include <ctype.h>
 
#ifndef IFT_FCS			/* May not be in <net/if_types.h> yet */
#define IFT_FCS		0x1f	/* IP over FCS */
#define FCS_MAX_IPA_LEN	8
struct fcs_hw_addr {
        u_long  nport;                          /* FCS Nport */
        char    ipa[FCS_MAX_IPA_LEN + 1];       /* ipa[0] = lpa len,
                                                /* ipa[1..8] = ipa string */
};
#else
#include <netinet/if_fcs.h>
#endif

extern int errno;

#include <nl_types.h>
#include "arp_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ARP,n,s) 

int u_def_type = 0;  /* user define an adapter type in command line */
int bynumber;	     /* Report arp table of contents by host addr   */

main(argc, argv)
	char **argv;
{

	setlocale(LC_ALL,"");
        catd = catopen(MF_ARP,NL_CAT_LOCALE);

	if (argc >= 2 && ((strcmp(argv[1], "-a") == 0) || 
			  (strcmp(argv[1], "-an") == 0))) {
		char *mem = "/dev/kmem";

		if (strcmp(argv[1], "-a") == 0)
			bynumber = 0;
		else 
			bynumber = 1;
			
		if (argc >= 3)
			mem = argv[2];
		dump(mem);
		exit(0);
	}
	if (argc == 2 ) 
	    if ( argv[1][0] != '-' ) {
		get(argv[1]);
		exit(0);
	    }
	    else {
		usage();
		exit(1);
	    }
	
	if (setuid(getuid())) {
		perror("arp: setuid");
		exit(1);
	}

	if (argc >= 4 && strcmp(argv[1], "-s") == 0) {
		set(argc-2, &argv[2]);
		exit(0);
	}
	if (argc == 3 && strcmp(argv[1], "-d") == 0) {
		delete(argv[2]);
		exit(0);
	}
	if ((argc == 3 || argc ==4) && strcmp(argv[1], "-f") == 0) {
		if ( argc == 4 )
		     u_def_type = TRUE;
		file(argv[2],argv[3],u_def_type);
		exit(0);
	}
	usage();
	exit(1);
}

/*
 * Process a file to set standard arp entries
 */
file(name,type,u_def_type)
	char *name, *type;
	int u_def_type;
{
	FILE *fp;
	int i;
	char line[100], arg[5][50], *args[5];

	if ((fp = fopen(name, "r")) == NULL) {
		fprintf(stderr, 
		     MSGSTR(CANT_OPEN, "arp: cannot open %s\n"), name); /*MSG*/
		exit(1);
	}
	args[0] = &arg[0][0];
	args[1] = &arg[1][0];
	args[2] = &arg[2][0];
	args[3] = &arg[3][0];
	args[4] = &arg[4][0];
	while(fgets(line, 100, fp) != NULL) {
	        if (u_def_type)
			i = sscanf(line, "%s %s %s %s",  arg[1], arg[2], 
							 arg[3], arg[4]);
		    else
 			i = sscanf(line, "%s %s %s %s %s", arg[0], arg[1], 
                                                   arg[2], arg[3], arg[4]);
		if ((i < 1 && u_def_type) || (i < 2 && ! u_def_type)) {
			fprintf(stderr, 
			 MSGSTR(BAD_LINE, "arp: bad line: %s\n"), line); /*MSG*/
			continue;
		}
	        if (u_def_type) {
			strcpy(arg[0],type);
			set(++i, args);
		     }
		     else 
			set(i, args);
	}
	fclose(fp);
}

/*
 * set -	process the `-s' option
 */
set(c, v)
char **v; {
	int type;
	if (getuid()) {
		fprintf(stderr, 
		     MSGSTR(PERM_DENIED, "arp: Permission denied.\n")); /*MSG*/
		exit(2);
	}

	if (!strcmp(*v, "ether"))
		type = IFT_ETHER;
	else if (!strcmp(*v, "802.3"))
		type = IFT_ISO88023;
	else if (!strcmp(*v, "802.5"))
		type = IFT_ISO88025;
	else if (!strcmp(*v, "fddi"))
		type = IFT_FDDI;
	else if (!strcmp(*v, "fcs"))
		type = IFT_FCS;
	else
		exit(1, usage());

	arp_set(--c, ++v, type);
}


/*
 * Set an individual arp entry 
 */
arp_set(argc, argv, type)
	int  argc;
	char **argv;
	int  type;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *aa;
	int s;
	char *host = argv[0], *haddr = argv[1];

	argc -= 2;
	argv += 2;
	bzero((caddr_t)&ar, sizeof ar);
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
              if ((hp = gethostbyname(host))==(struct hostent *) 0) {
                     fprintf(stderr,MSGSTR(NME_NT_FND,
                                 "arp: host name %s NOT FOUND\n"), host);
                     exit(1);
                }
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
                    sizeof sin->sin_addr);
        }
        else {
		if (sin->sin_addr.s_addr == -1) {
                        fprintf(stderr, MSGSTR(BADADDR,
                                "arp: %s: address misformed\n"), host);
                        exit(2);
                        }
        }

	aa = (u_char *)ar.arp_ha.sa_data;
	ar.arp_flags = ATF_PERM;
	ar.ifType = type;
	if (type == IFT_FCS) {
		if ((ar.at_length = hwaddr_fcs(haddr, argc, argv, aa)) == 0)
			return;
	}
	else
		if ((ar.at_length = hwaddr_aton(haddr, aa)) == 0)
			return;
	if (type == IFT_ISO88025) {
		/*
		 * if it looks like we have a route specified, try to make sense
		 * of it.
		 */
		if (argc > 0 && strchr(*argv, ':')) {
			if (_802_5_rton(*argv, &ar.arp_rcf, ar.arp_seg))
				return;
			--argc, ++argv;
		}
	} else
	if (type == IFT_FDDI) {
		/*
		 * if it looks like we have a route specified, try to make sense
		 * of it.
		 */
		if (argc > 0 && strchr(*argv, ':')) {
			if (fddi_rton(*argv, &ar.arp_fddi_rcf, ar.arp_fddi_seg))
				return;
			--argc, ++argv;
		}
	}

	while (argc-- > 0) {
		if (strncmp(argv[0], "temp", 4) == 0)
			ar.arp_flags &= ~ATF_PERM;
		if (strncmp(argv[0], "pub", 3) == 0)
			ar.arp_flags |= ATF_PUBL;
		if (strncmp(argv[0], "trail", 5) == 0)
			ar.arp_flags |= ATF_USETRAILERS;
		argv++;
	}
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
                perror("arp: socket");
                exit(1);
        }
	if (ioctl(s, SIOCSARP, (caddr_t)&ar) < 0) {
		perror(host);
		exit(1);
	}
	close(s);
}

/*
 * Display an individual arp entry
 */
get(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *ea;
	int s;
	register i, j;

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
              if ((hp = gethostbyname(host))==(struct hostent *) 0) {
                     fprintf(stderr,MSGSTR(NME_NT_FND,
                                 "arp: host name %s NOT FOUND\n"), host);
                     exit(1);
                }
                bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
                    sizeof sin->sin_addr);
        }
        else {
                if (sin->sin_addr.s_addr == -1) {
                        fprintf(stderr, MSGSTR(BADADDR,
                                "arp: %s: address misformed\n"), host);
                        exit(2);
                        }
        }

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
                perror("arp: socket");
                exit(1);
        }
	
	if (setuid(getuid())) {
		perror("arp: setuid");
		exit(1);
	}

	if (ioctl(s, SIOCGARP, (caddr_t)&ar) >= 0) {
		/*
		 * found and ARP entry in the ARP table
		 */
		ea = (u_char *)ar.arp_ha.sa_data;
		printf(MSGSTR(PRNT_INFO, "%s (%s) at "), 
			host, inet_ntoa(sin->sin_addr)); /*MSG*/
		if (ar.arp_flags & ATF_COM)
			switch(ar.ifType) {
				case IFT_X25:
					x25_print(ea);
					break;
				case IFT_FCS:
					fcs_print(ea);
					break;
				default:
					hwaddr_print(ea);
			}
		else
			printf(MSGSTR(INCOMPLETE, "(incomplete)")); /*MSG*/

		switch (ar.ifType) {
			case IFT_ETHER:
				printf(MSGSTR(TYPE_EN_PRINT, " [ethernet]"));
				break;
			case IFT_ISO88023:
				printf(MSGSTR(TYPE_8023_PRINT, " [802.3]"));
				break;
			case IFT_ISO88025:
				printf(MSGSTR(TYPE_8025_PRINT,
					 " [token ring]"));
				break;
			case IFT_FDDI:
				printf(MSGSTR(TYPE_FDDI_PRINT,
					" [fddi]"));
				break;
			case IFT_X25:
				printf(MSGSTR(TYPE_X25_PRINT, " [X.25]"));
				break;
			case IFT_FCS:
				printf(MSGSTR(TYPE_FCS_PRINT, " [fcs]"));
				break;
			default:	
				break;
		}
		if (ar.arp_flags & ATF_PERM) 
			printf(MSGSTR(PERMANENT, " permanent")); /*MSG*/
		if (ar.arp_flags & ATF_PUBL) 
 			printf(MSGSTR(PUBLISHED, " published")); /*MSG*/
		if (ar.arp_flags & ATF_USETRAILERS) 
			printf(MSGSTR(TRAILERS, " trailers")); /*MSG*/

		if (ar.arp_rcf && 
			(ar.ifType == IFT_ISO88025)) {
			/*
			 * print token ring routing info
			 */
			printf(MSGSTR(RT," rt=%x"), ar.arp_rcf);

			j  = (ar.arp_rcf >> 8) & 0x1f;
			j -= sizeof (ar.arp_rcf);
			j /= sizeof (ar.arp_seg[0]);

			for (i = 0; i < j; ++i)
				printf(":%x", ar.arp_seg[i]);
		} else
		if (ar.arp_fddi_rcf && 
			(ar.ifType == IFT_FDDI)) {
			/*
			 * print fddi routing info
			 */
			printf(MSGSTR(RT," rt=%x"), ar.arp_fddi_rcf);

			j  = (ar.arp_fddi_rcf >> 8) & 0x1f;
			j -= sizeof (ar.arp_fddi_rcf);
			j /= sizeof (ar.arp_fddi_seg[0]);

			for (i = 0; i < j; ++i)
				printf(":%x", ar.arp_fddi_seg[i]);
		} 
		printf("\n");
	} else {
		printf(MSGSTR(NO_ENTRY, "%s (%s) -- no entry\n"), 
			host, inet_ntoa(sin->sin_addr)); /*MSG*/
		exit(1);
	}
	close(s);
	printf("\n");

}

/*
 * Delete an arp entry 
 */
delete(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	int s;
	int got_one = 0;
	int perm_err = 0;

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
              if ((hp = gethostbyname(host))==(struct hostent *) 0) {
                     fprintf(stderr,MSGSTR(NME_NT_FND,
                                 "arp: host name %s NOT FOUND\n"), host);
                     exit(1);
                }
                bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
                    sizeof sin->sin_addr);
        }
        else {
                if (sin->sin_addr.s_addr == -1) {
                        fprintf(stderr, MSGSTR(BADADDR,
                                "arp: %s: address misformed\n"), host);
                        exit(2);
                        }
        }

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
                perror("arp: socket");
                exit(1);
        }
	if (ioctl(s, SIOCDARP, (caddr_t) &ar) >= 0)
		++got_one;
	else {
		if (errno != ENXIO)
			perm_err++;
	}
	if (!got_one && perm_err) {
		fprintf(stderr, MSGSTR(PERM_DENIED, 
			"arp: Permission denied.\n")); /*MSG*/
		exit(2);
	}
	if (!got_one) {
		printf(MSGSTR(ENT_NT_FND, 
			"%s (%s) -- entry not found\n"), 
			host, inet_ntoa(sin->sin_addr)); /*MSG*/
		exit(1);
	}
	close(s);
	printf(MSGSTR(DELETED, "%s (%s) deleted\n"), 
		host, inet_ntoa(sin->sin_addr)); /*MSG*/
}

#define NELEM_NLIST 3

struct nlist nl[] = {
#define	X_ARPTAB		0
	{ "arptabp" },
#define	X_ARPTAB_SIZE		1
	{ "arptabsize" },
	{ (char *)NULL }
};

/*
 * Dump the entire arp table
 */
dump(mem)
	char *mem;
{
	int mf;

	if (knlist(nl, NELEM_NLIST, sizeof(struct nlist)) == -1) {
		perror("arp: knlist");
		exit(1);
	}

	mf = open(mem, 0);
	if(mf < 0) {
		fprintf(fprintf, 
			MSGSTR(CANT_OPEN, "arp: cannot open %s\n"), mem);
		exit(1);
	}

	dump_all(mf);
}

dump_all(mf)
	int mf;
{
	int arptab_size, sz;
	struct arptab *at;
	struct hostent *hp;
	char *host;
	int i,j;
	int type;
	int arp;
	struct ifnet *ifp, *get_ifp();

	if (!nl[X_ARPTAB_SIZE].n_value || !nl[X_ARPTAB].n_value)
		return;
	lseek(mf, (long)nl[X_ARPTAB_SIZE].n_value, 0);
	read(mf, &arptab_size, sizeof arptab_size);
	if (arptab_size <=0 ) {
		fprintf(stderr, MSGSTR(NAMLST_WRONG, "arp: namelist wrong\n"));
		exit(1);
	}
	sz = arptab_size * sizeof(struct arptab);
	at = (struct arptab *) malloc(arptab_size * sizeof(struct arptab));
	if (at == (struct arptab *)NULL) {
		perror(MSGSTR(NO_RD_ARPTAB, "arp: error reading arptab\n"));
		exit(1);
	}
	lseek(mf, (long)nl[X_ARPTAB].n_value, 0);
	if (read(mf, &arp, sizeof arp) != sizeof(arp)) {
		perror(MSGSTR(NO_RD_ARPTAB, "arp: error reading arptab\n"));
		exit(1);
	}
	lseek(mf, (long)arp, 0);
	if (arp == 0) {
		fprintf(stderr, MSGSTR(NO_RD_ARPTAB,"arp: error reading arptab\n"));
		exit(1);
	}
	if (read(mf, at, sz) != sz) {
		perror(MSGSTR(NO_RD_ARPTAB, "arp: error reading arptab\n"));
		exit(1);
	}
	for ( ; arptab_size-- > 0 ; at++) {
		if (at->at_iaddr.s_addr == 0 || at->at_flags == 0)
			continue;
		if (bynumber == 0)
			hp = gethostbyaddr((caddr_t)&at->at_iaddr,
			    sizeof at->at_iaddr, AF_INET);
		else
			hp = 0;

		if (hp)
			host = hp->h_name;
		else
			host = "?";

		printf(MSGSTR(NEW_PRINT, "  %s (%s) at "), 
			host, inet_ntoa(at->at_iaddr)); /*MSG*/
		if (at->at_flags & ATF_COM) {
			ifp = get_ifp(mf, at->at_ifp);
			if (ifp)
				type = ifp->if_type;
			else 
				type = IFT_OTHER;

			switch (type) {
				case IFT_X25:
					x25_print(at->hwaddr);
					break;
				case IFT_FCS:
					fcs_print(at->hwaddr);
					break;
				default:
					hwaddr_print(at->hwaddr);
					break;
			}
		} else {
			printf(MSGSTR(INCOMPLETE, "(incomplete)")); /*MSG*/
			printf("\n");
			continue;
		}
		switch (type) {
			case IFT_ETHER:
				printf(MSGSTR(TYPE_EN_PRINT, " [ethernet]"));
				break;
			case IFT_ISO88023:
				printf(MSGSTR(TYPE_8023_PRINT, " [802.3]"));
				break;
			case IFT_ISO88025:
				printf(MSGSTR(TYPE_8025_PRINT, 
					" [token ring]"));
				break;
			case IFT_FDDI:
				printf(MSGSTR(TYPE_FDDI_PRINT, 
					" [fddi]"));
				break;
			case IFT_X25:
				printf(MSGSTR(TYPE_X25_PRINT, " [X.25]"));
				break;
			case IFT_FCS:
				printf(MSGSTR(TYPE_FCS_PRINT, " [fcs]"));
				break;
			default:	
				break;
		}
		if (at->at_flags & ATF_PERM)
			printf(MSGSTR(PERMANENT, " permanent")); /*MSG*/
		if (at->at_flags & ATF_PUBL) 
 			printf(MSGSTR(PUBLISHED, " published")); /*MSG*/
		if (at->at_flags & ATF_USETRAILERS) 
			printf(MSGSTR(TRAILERS, " trailers")); /*MSG*/
		if (at->at_rcf && 
			(type == IFT_ISO88025)) {
			/*
			 * print token ring routing info
			 */
			printf(MSGSTR(RT," rt=%x"), at->at_rcf);

			j  = (at->at_rcf >> 8) & 0x1f;	/* bytes of route */
			j -= sizeof (at->at_rcf);	/* and RCF	  */
			j /= sizeof (at->at_seg[0]);	/* # segments	  */

			for (i = 0; i < j; ++i)
				printf(":%x", at->at_seg[i]);
		} else
		if (at->at_fddi_rcf && 
			(type == IFT_FDDI)) {
			/*
			 * print fddi routing info
			 */
			printf(MSGSTR(RT," rt=%x"), at->at_fddi_rcf);

			j  = (at->at_fddi_rcf >> 8) & 0x1f;	/* bytes of route */
			j -= sizeof (at->at_fddi_rcf);	/* and RCF	  */
			j /= sizeof (at->at_fddi_seg[0]);	/* # segments	  */

			for (i = 0; i < j; ++i)
				printf(":%x", at->at_fddi_seg[i]);
		}
		printf("\n");
	}
}

x25_print(ap)
struct x25_addr *ap; {

	printf("%-16.16s", ap);
}

fcs_print(cp)
	u_char *cp; {

	int i;

	printf("Nport[%x%x%x]", cp[1], cp[2], cp[3]);
	if (cp[4]) {
		printf(" IPA[");
		for (i=cp[4]; i>0; i--) 
			printf("%x", cp[i+4]);
		printf("]");
	}

}

hwaddr_print(cp)
	u_char *cp;
{
	printf("%x:%x:%x:%x:%x:%x", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
}

hwaddr_aton(a, n)
	char *a;
	u_char *n;
{
	int i, o[6];

	i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],
					   &o[3], &o[4], &o[5]);
	if (i != 6) {
		fprintf(stderr, 
			MSGSTR(BAD_HW_ADDR, 
			"arp: invalid hardware address '%s'\n"), a); /*MSG*/
		return (0);
	}
	for (i=0; i<6; i++)
		n[i] = o[i];
	return (6);
}

#define fcs_struct_size	((sizeof(u_long)) + (sizeof(char[FCS_MAX_IPA_LEN+1])))
hwaddr_fcs(a, argc, argv, n)
	char *a;
	int  argc;
	char **argv;
	u_char *n;
{
	struct fcs_hw_addr fcshwaddr;
	int i, len=0, ipalen=0, convert, times;
	char ipavalue;

	/* check and get optional ipa */
	if ((argc == 0) || (argv[0][0] == 't') || (argv[0][0] == 'p'))
		fcshwaddr.ipa[0] = 0;
	else {
		if(((len = strlen(argv[0])) > (FCS_MAX_IPA_LEN*2)) ||
		    len == 0) {
			fprintf(stderr, MSGSTR(BAD_IPA, 
				"arp: invalid ipa '%s'\n"), argv[0]); /*MSG*/
			return (0);
		}
		else {
			for(i=len-1; i>=0; i--) {
				argv[0][i] = tolower(argv[0][i]);
				if (!isxdigit(argv[0][i])) {
					fprintf(stderr , MSGSTR(BAD_IPA, 
						"arp: invalid ipa '%s'\n"), 
						argv[0]); /*MSG*/
					return (0);
				}
			}
		}
	}
	while (len > 0) {
		if (len == 1) {
			sscanf((argv[0]+(len-1)),"%x",&convert);
			fcshwaddr.ipa[ipalen+1] = (char)convert;
			len--;
		}
		else { /* cunsume 2 chars for 1 octet */
			sscanf((argv[0]+(len-2)),"%x",&convert);
			fcshwaddr.ipa[ipalen+1] = (char)convert;
			*(argv[0]+(len-2)) = (char)NULL;
			len -= 2;
		}
		ipalen++;
	}
	if((fcshwaddr.ipa[0] = ipalen) > 1) {
		times = (ipalen/2);
		for (i=1; i<=times; i++) {
			ipavalue = fcshwaddr.ipa[ipalen];
			fcshwaddr.ipa[ipalen--] = fcshwaddr.ipa[i];
			fcshwaddr.ipa[i] = ipavalue;
		}
	} /* got ipa */
			

	/* check and get fcs Nport */
	if (!(len=strlen(a)) || (len > 6)) {
		fprintf(stderr , MSGSTR(BAD_NPORT, 
			"arp: invalid Nport address '%s'\n"), a); /*MSG*/
		return (0);
	}
	for(i=len-1; i>=0; i--) {
		if(!isxdigit((a[i] = tolower(a[i])))) {
			fprintf(stderr , MSGSTR(BAD_NPORT, 
				"arp: invalid Nport address '%s'\n"), a); /*MSG*/
			return (0);
		}
	}
	if (sscanf(a, "%x", &fcshwaddr.nport) == EOF) {
		perror("arp: sscanf");
		return(0);
	} /* got Nport */

	bcopy((char *)&fcshwaddr, (char *)n, fcs_struct_size);

	return(fcs_struct_size);
}

/*
 * _802_5_rton	-	xlate token ring route string
 */
_802_5_rton(r, rcf, seg)
char *r;
u_short *rcf, *seg; {
	int i, j, rt_bytes;
	int o[8+1];

	i = sscanf(r, "%x:%x:%x:%x:%x:%x:%x:%x:%x"
		   , &o[0], &o[1], &o[2], &o[3]
		   , &o[4], &o[5], &o[6], &o[7], &o[8]);

	rt_bytes = (o[0] >> 8) & 0x1f;

	if (rt_bytes / sizeof (*seg) != i) {
		printf(MSGSTR(TKNRG_BYTES, 
			"rt_bytes = %d, i = %d\n"), rt_bytes, i); /*MSG*/
		fprintf(stderr, 
			MSGSTR(BAD_TKNRG_RTE, 
			"arp: invalid Token Ring route '%s'\n"), r); /*MSG*/
		return (1);
	}

	*rcf = o[0];

	for (j = 1; j < i; ++j)
		seg[j - 1] = o[j];

	return (0);
}

/*
 * fddi_rton	-	xlate fddi route string
 */
fddi_rton(r, rcf, seg)
char *r;
u_short *rcf, *seg; {
	int i, j, rt_bytes;
	int o[14+1];

	i = sscanf(r, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x"
		   , &o[0], &o[1], &o[2], &o[3], &o[4]
		   , &o[5], &o[6], &o[7], &o[8], &o[9]
		   , &o[10], &o[11], &o[12], &o[13], &o[14]);

	rt_bytes = (o[0] >> 8) & 0x1f;

	if (rt_bytes / sizeof (*seg) != i) {
		printf(MSGSTR(TKNRG_BYTES, 
			"rt_bytes = %d, i = %d\n"), rt_bytes, i); /*MSG*/
		fprintf(stderr, 
			MSGSTR(BAD_FDDI_RTE, 
			"arp: invalid FDDI Ring route '%s'\n"), r); /*MSG*/
		return (1);
	}

	*rcf = o[0];

	for (j = 1; j < i; ++j)
		seg[j - 1] = o[j];

	return (0);
}

usage()
{
	printf(MSGSTR(USAGE1, "Usage: arp hostname\n\tarp -a[n] [/dev/kmem]\n\tarp -d hostname\n\tarp -s ether hostname ether_addr [temp] [pub]\n\tarp -s 802.3 hostname ether_addr [temp] [pub]\n")); /*MSG*/
	printf(MSGSTR(USAGE3, "\tarp -s fddi hostname fddi_addr [fddi_route] [temp] [pub]\n"));  /*MSG*/
	printf(MSGSTR(USAGE2, "\tarp -s 802.5 hostname token_addr [token_route] [temp] [pub]\n\tarp -f filename [type]\n"));  /*MSG*/
}

struct ifnet *
get_ifp(mf, ifp)
int mf;
struct ifnet *ifp;
{
	static struct ifnet ifnet;

	lseek(mf, (long)ifp, 0);
	if (read(mf, &ifnet, sizeof (struct ifnet)) != sizeof(struct ifnet))
		return((struct ifnet *)NULL);
	return(&ifnet);
}
