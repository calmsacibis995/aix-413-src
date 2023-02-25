static char sccsid[] = "@(#)49	1.5  src/tcpip/usr/sbin/lsadpnm/lsadpnm.c, tcpip, tcpip411, GOLD410 3/8/94 17:47:18";
/*
 * COMPONENT_NAME:  CMDNET   Network Configuration 
 *
 * FUNCTIONS: main, lsadpnm, sort_names, usage
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <strings.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <odmi.h>

char progname[128];		/* = argv[0] at run time */

#include        <nl_types.h>
#include        "lsadpnm_msg.h"

#define         MSGSTR(num,str) catgets(catd,MS_LSADPNM,num,str)  /*MSG*/

nl_catd         catd;
#include <locale.h>

#ifdef DEBUG
#define  DBGMSG(x) if (getenv("TCPDEBUG") != NULL) { \
		       fprintf(stderr, "DEBUG(%s) ", progname); \
	               fprintf x ; fprintf(stderr, "\n"); \
		   }
#else
#define  DBGMSG(x)
#endif

#define  CFGEXIT(x)  odm_terminate(); exit(x);
#define	 OBJREPOS    "/etc/objrepos"

/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */

main(int argc, char **argv)
{
	int c, iflag, cflag, phase = 0;
	char *p, *cp;
	char *subclass = NULL, *type = NULL, *class = NULL;
	char *prefix;		/* prefix */
	extern char *optarg;

	setlocale(LC_ALL, "");
	catd = catopen(MF_LSADPNM, NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	/* process command line args */
	while ((c = getopt(argc, argv, "c:s:t:")) != EOF) {
		switch (c) {
		case 'c':
			class = optarg;
			break;
		case 's':
			subclass = optarg;
			break;
		case 't':
			type = optarg;
			break;
		case '?':
		default:
			usage();
		}
	}

	if (odm_set_path(OBJREPOS) == -1) {
		fprintf(stderr, MSGSTR(BADPATH,
		"lsadpnm: Cannot set the default path to %s.\n"),
		    OBJREPOS);
		CFGEXIT(1);
	}

	if (odm_initialize() == -1) {
		fprintf(stderr, MSGSTR(INITFAIL,
		"lsadpnm: The database initialization failed.\n"));
		CFGEXIT(1);
	}

	lsadpnm(class, subclass, type);
	odm_terminate();
	exit(0);
}

/*
 * NAME: lsadpnm
 *                                                                    
 * FUNCTION: 
 *	send all available adapter names with no "if" attached to
 *	stdout.
 *                                                                    
 * NOTES:
 *	this command is intented to be used with smit in which all
 *	the parameters are set.
 *
 * RETURNS:  0 if something was defined.
 *	     -1 if nothing defined.
 */
lsadpnm( char *class,	/* class */
	char *subclass,		/* subclass of IF */
	char *type)		/* type of IF */
{
	char crit[128];
	struct PdAt pdat;
	struct CuDv *cudvp, *cudvip, *p, *ifp;
	struct PdDv pddv, *pddvp;
	struct PdCn *pdcnp;
	char adp_nam[NAMESIZE];	/* adapter name prefix */
	char ifname[NAMESIZE];	/* str to be compared with adapter */
	char	**namelist;		/* list of available adapter names */
	struct listinfo cudv_info, cudvi_info, pddv_info, pdcn_info;
	int	i, j, k, n;
	int	ifprfxsz;	/* "if" prefix size */
	int	adprfxsz;	/* adapter prefix size */

	/* ensure all the parameters are defined */
	if ( class == NULL || subclass == NULL ||
	    type == NULL )
		usage();

	sprintf (crit,"uniquetype = '%s/%s/%s' and attribute = 'if_keyword'",
	    class, subclass, type);
	DBGMSG((stderr, "search crit. for if_keyword -- %s", crit));
	if ( (odm_get_first(PdAt_CLASS, crit, &pdat)) <= 0 ) {
		fprintf(stderr, MSGSTR(NOIFKEY,
		"lsadpnm: Cannot get \"if_keyword\" attribute from PdAt.\n"));
		CFGEXIT(1);
	}

	/* get prefix from the data base */
	strcpy(crit, "connkey = '");
	strcat(crit, pdat.deflt);
	strcat(crit, "'");
	DBGMSG((stderr, "Querying PdCn for: %s", crit));
	pdcnp = get_PdCn_list(PdCn_CLASS, crit, &pdcn_info, 1, 1);
	if ((int) pdcnp == -1) {
		fprintf(stderr, MSGSTR(GETADAPT,
		"0822-060 lsadpnm: Cannot get records from PdCn.\n"));
		CFGEXIT(1);
	}

	/*
	 * find the predefined adapter.
	 */
	sprintf(crit, "uniquetype = '%s'", pdcnp->uniquetype);
	DBGMSG((stderr, "looking for Pd adapter: %s", crit));
	pddvp = get_PdDv_list(PdDv_CLASS, crit, &pddv_info,
			      1, 1);
	if ((int) pddvp == -1) {
		fprintf(stderr, MSGSTR(GETPDADAP,
	"0822-061 lsadpnm: Cannot get predefined adapter.\n"));
		CFGEXIT(1);
	}
	if (pddv_info.num != 1) {
		fprintf(stderr, MSGSTR(PDADAPNUM,
	"0822-062 lsadpnm: Expected one predefined adapter, retreived %d.\n"),
			pddv_info.num);
		CFGEXIT(1);
	}

	strcpy( adp_nam, pddvp->prefix);	/* get the prefix of adapter */

	/* get the appropriate adapter names which is marked available */
	sprintf (crit, "name like '%s*' and status = %d ", adp_nam, AVAILABLE);
	DBGMSG((stderr, "search criteria for adptor -- %s", crit));
	cudvp = get_CuDv_list(CuDv_CLASS, crit, &cudv_info, 1, 1);
	if ((int) cudvp == -1) {
		DBGMSG((stderr, "no customized adapters (%s) found",
			adp_nam));
		fprintf(stderr, MSGSTR(GETCUDV,
			"lsadpnm: Cannot get records from CuDv.\n"));
		CFGEXIT(2);
	}
	if ( cudv_info.num == 0 ) {
		fprintf(stderr, MSGSTR(NOCUDV,
		    "lsadpnm: Cannot find adapter in CuDv with prefix %s.\n"),adp_nam);
		CFGEXIT(1);
	}
	DBGMSG((stderr, "%d adapter found", cudv_info.num));

	/* get if prefix */
	sprintf( crit, "uniquetype = '%s/%s/%s'", class, subclass, type) ;
	if ( (odm_get_first(PdDv_CLASS, crit, &pddv)) <= 0 ) {
		fprintf(stderr, MSGSTR(GETPDDV,
		    "lsadpnm: Cannot get records from PdDv.\n"));
		CFGEXIT(1);
	}

	/* get all "if" defined in the system */
	sprintf (crit, "name like '%s*'", pddv.prefix);
	DBGMSG((stderr, "search criteria for \"if\" -- %s", crit));
	cudvip = get_CuDv_list(CuDv_CLASS, crit, &cudvi_info, 1, 1);
	if ((int) cudvip == -1) {
		DBGMSG((stderr, "no customized adapters (%s) found",
			adp_nam));
		fprintf(stderr, MSGSTR(GETCUDV,
			"lsadpnm: Cannot get records from CuDv.\n"));
		exit(2);
	}
	DBGMSG((stderr, "%d \"if\" found",cudvi_info.num));

	if ( (namelist =
	    (char **) malloc ( cudv_info.num * sizeof (char *))) == 0 ) {
		fprintf(stderr, MSGSTR(NOMEM,
		    "lsadpnm: Cannot allocate memory\n"));
		CFGEXIT (2);
	}

	ifprfxsz = strlen(pddv.prefix);
	adprfxsz = strlen(adp_nam);
	DBGMSG((stderr, "if prefix ln = %d, adp = %d", ifprfxsz,
		adprfxsz));
	strcpy(ifname, pddv.prefix);	/* get the adapter name prefix */
	/* get rid of the adapters that have "if" attached */
	for ( i=0, p=cudvp, n=0; i< cudv_info.num; i++,p++ ){
		for ( j=0, ifp=cudvip; j < cudvi_info.num; j++, ifp++) {
			/* find corresponding adapter name */
			if (!strcmp(&p->name[adprfxsz], &ifp->name[ifprfxsz])) break;
		}
		if ( j < cudvi_info.num ) continue;	/* "if" name found */
		/* make sure the adapter prefix is not part of the prefix
		 * use by other devices -- assume at least one of the
		 * the remaining name has at least one none hex digit
		 */
		j = strlen(p->name);
		for( k = adprfxsz; k < j; k++)
			if(! isxdigit(p->name[k] )) break;
		/* unique name found */
	       	DBGMSG((stderr, "adp. %s", p->name));
		/* use the adapter customized data base to store the name
		 * of the "if" to be used 
		 */
		if ( k >= j ) {
			/* form "if" name */
			strcpy (&ifname[ifprfxsz], &p->name[adprfxsz]);
			strcpy (p->name, ifname);
			namelist[n++] = p->name;	/* save the name */
		}
	}
	if (n == 0){
		fprintf(stderr, MSGSTR(NOADPAV,
		    "lsadpnm: No available adapter.\n"));
		CFGEXIT (2);
	}
	sort_names( namelist, n);	/* sort all the names */
	/* print names to std_out */
	for ( i = 0; i < n; i++)
		printf("%s\n", namelist[i]);
	return(0);
}
	 
/*
 * NAME:  sort_names
 *
 * FUNCTION: sort names in acending order.  The algorithm used is the quick
 *		sort.
 * 
 * INPUT PARAMETERS:
 *	names -- the array of character pointers for names
 *	cnt   -- size of table
 *
 * OUTPUT PARAMETERS:
 *      names -- rearranged class names pointers
 */
sort_names(names, cnt)
char *names[];
int  cnt;
{
	int	gap, i, j, inttmp;
	char	*temp;

	for ( gap = cnt/2; gap > 0; gap /= 2)
		for ( i = gap; i < cnt; i++)
			for( j = i-gap; j >= 0; j -= gap ) {
				if ( strcmp (names[ j ], names [ j+gap ]) <= 0)
					break;
				temp = names [j];
				names [ j ] = names [ j+gap ];
				names [ j+gap ] = temp;
			}
}
/*
 * NAME: usage
 *
 * FUNCTION: prints out the usage message when user makes a mistake on 
 *           the command line.
 *
 * RETURNS:  nothing, exits.
 */
usage()
{
	fprintf(stderr,
	    MSGSTR(USAGE, "\nusage:\tlsadpnm  -c class -s subclass -t type\n"));
	odm_terminate();
	exit(1);
}
