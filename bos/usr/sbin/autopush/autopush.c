static char sccsid[] = "@(#)71        1.2.1.4  src/bos/usr/sbin/autopush/autopush.c, cmdpse, bos41B, 412_41B_sync 12/13/94 11:20:03";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *
 *   ORIGINS: 27 63 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** autopush.c 2.2, last change 11/19/90
 **/

/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * AUTOPUSH.C
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include <pse/common.h>
#include <pse/nd.h>
#include <sys/stropts.h>
#include <sys/sad.h>
#include <sys/types.h>

#include "autopush_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_AUTOPUSH,n,s)

#ifndef ADMINDEV
#define ADMINDEV	"/dev/sad"
#define USERDEV		"/dev/sad"
#endif

#ifndef	NULL_DEV_NAME
#define	NULL_DEV_NAME	"/dev/nuls"
#endif

staticf int	get_ap_info(   long dmajor, long dminor, struct strapush * infop   );
staticf char	* get_cp(   char * buf, char * name   );
staticf int	get_mm(   int narg, char ** arglist, long * majp, long * minp   );
staticf int	get_next_arg(   char * string1, char * string2, char ** result   );
staticf int	load_ap_modules(   struct strapush * infop   );
staticf void	open_sad_driver(   void   );
staticf void	read_ap_modlist(   FILE * apf   );
staticf int	remove_ap_modules(   long dmajor, long dminor   );
staticf void	show_ap_info(   struct strapush * infop   );
staticf void	show_ap_modules(   long dmajor,  long dminor   );
staticf int	split_line(   char * buf, struct strapush * infop   );
extern	char	* strchr(   char * str1, char chr   );
extern	char	* strtok(   char * str1, char * str2   );
staticf	long	str_to_major(   char * string   );

extern	int	noshare errno;
	int	Fds;			/* file descriptor for stream SAD driver */
	int	Line_count = 0;		/* line count for -f filename input error msgs */

/*------------------------------------------------------------------------------------*/

staticf void
open_sad_driver ()
{
	/* TODO: need os-dependent privileged user check */
	if (getuid() == 0) {
		if ((Fds = stream_open(ADMINDEV, O_RDWR)) == -1) {
			fatal(MSGSTR(MSG1, "SAD open failed for %s\n"), ADMINDEV);
		}
	} else {
		if ((Fds = stream_open(USERDEV, O_RDWR)) == -1) {
			fatal(MSGSTR(MSG2, "SAD open failed for %s\n"), USERDEV);
		}
	}
}


/*------------------------------------------------------------------------------------*/

staticf int
get_ap_info (dmajor, dminor, infop)
	long		dmajor, dminor;
	struct strapush	* infop;
{
	int	ioerr;
	
	infop->sap_major = dmajor;
	infop->sap_minor = dminor;
	if ((ioerr = stream_ioctl(Fds, SAD_GAP, infop)) < 0) {
		switch (errno) {
		case EFAULT:
			warn(MSGSTR(MSG3, "Program bug - bad arg/infop (%p)\n"), infop);
			break;

		case EINVAL:
			warn(MSGSTR(MSG4, "Invalid major (%d)\n"), infop->sap_major);
			break;

		case ENOSTR:
			warn(MSGSTR(MSG5, "Major (%d) is not a STREAMS device\n"), infop->sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(MSG6, "Device (%d,%d) not configured for autopush\n"), infop->sap_major, infop->sap_minor);
			break;

		default:
			warn(MSGSTR(MSG7, "Unexpected IOCTL(SAD_GAP) error: %d\n"), errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf void
show_ap_info (infop)
	struct strapush	* infop;

{
	int	k;
	
	switch (infop->sap_cmd) {
	case SAP_ONE:
	case SAP_RANGE:
	case SAP_ALL:
		printf(MSGSTR(MSG8, "Major Minor Last Minor  %2d module(s)\n"), infop->sap_npush);
		printf("----- ----- ----------  ------------\n");
		printf("%3d   ", infop->sap_major);
		if (infop->sap_cmd == SAP_ALL) {
			printf(MSGSTR(MSG9, " ALL   "));
		} else {
			printf("%3d    ", infop->sap_minor);
		}
		if (infop->sap_cmd == SAP_RANGE) {
			printf("  %3d    ", infop->sap_lastminor);
		} else {
			printf("  ---    ");
		}
		if (infop->sap_npush <= MAXAPUSH) {
			for (k = 0; k < infop->sap_npush; k++) {
	     			printf("  %s", infop->sap_list[k]);
	     		}
	     	}
		printf("\n");
	     	break;
	
	default:
		warn(MSGSTR(MSG10, "infop->sap_cmd = %d\n"), infop->sap_cmd);
		break;
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf void
show_ap_modules (dmajor, dminor)
	long	dmajor, dminor;
{
	struct strapush	ap_info;
	
	if (get_ap_info(dmajor, dminor, &ap_info) == 0) {
		show_ap_info(&ap_info);
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf int
split_line (buf, infop)
	char		* buf;
	struct strapush	* infop;

	/*
	 * buf is a line of the form: maj_ min_ last_min_ mod1 mod2 ... # comment
	 * Returns # of items before #, or -1 if error.  If #>0, *infop is filled with
	 * values found.
	 * Lines starting with # are treated as comments.
	 */
{
	char	* next;
	long	nfield = 0;		/* number of fields found */
	
	if ((next = strchr(buf, '#')) == nilp(char)) {
		next = strchr(buf, '\0');
	} else {
		if (next == buf) 	/* Comment line. */
			return(0);
	}
	*next = '\0';
	
	next = strtok(buf, " \n\t,");
	while (next != nilp(char)) {
		if (++nfield == 1) {
			infop->sap_major = str_to_major(next);
		} else if (nfield == 2) {
			infop->sap_minor = atol(next);
		} else if (nfield == 3) {
			infop->sap_lastminor = atol(next);
		} else if (nfield <= 3 + MAXAPUSH) {
			(void)strncpy(infop->sap_list[nfield-4], next, FMNAMESZ+1);
		} else {
			warn(MSGSTR(MSG11, "More than %d modules in line %d.  Entry ignored\n"),
					MAXAPUSH, Line_count);
			fprintf(stderr, MSGSTR(MSG12, "\tLine in error: %.40s ...\n"), buf);
			next = nilp(char);
			nfield = -1;
		}
		next = strtok(nilp(char), " \n\t,");
	}
	if (nfield < 4) {
		warn(MSGSTR(MSG15, "1 to %d module names required at line %d\n"), MAXAPUSH, Line_count);
		return(-1);
	}
	infop->sap_npush = ( (nfield > 3) ? nfield - 3 : 0 );

	if (infop->sap_minor == -1) {
		infop->sap_cmd = SAP_ALL;
	} else if (infop->sap_lastminor == 0) {
		infop->sap_cmd = SAP_ONE;
	} else 	if (infop->sap_minor < infop->sap_lastminor) {
		infop->sap_cmd = SAP_RANGE;
	} else {
		warn(MSGSTR(MSG13, "Inconsistent Major (%d), Minor (%d), Last_minor (%d) at line %d\n"), 
			infop->sap_major, infop->sap_minor, infop->sap_lastminor, Line_count);
		nfield = -1;
	}
	return nfield;
}

/*------------------------------------------------------------------------------------*/

staticf int
load_ap_modules (infop)
	struct strapush	* infop;
	
{
	struct str_list namelist;
	int		k;
	int		ioerr, ioerr2;
	int		saved_errno;
	char		* sap_cmd_text;
		
	if (ioerr = stream_ioctl(Fds, SAD_SAP, infop) < 0) {
		warn(MSGSTR(MSG14, "Can't push requested modules on STREAM for entry %d\n"), Line_count);
		switch (saved_errno = errno) {

		case EINVAL:
			/*
			 * Validate module names, since otherwise user must re-run
			 * autopush for each device to get list of ok names.  Since
			 * error code is ambiguous, must check all drivers in list first.
			 * If all are ok, then error is major device number; otherwise
			 * if one of the module is bad, we show entire table.
			 */
			 
			if (infop->sap_npush <= 0 || infop->sap_npush > MAXAPUSH) {
				warn(MSGSTR(MSG15, "1 to %d module names required at line %d\n"), MAXAPUSH, Line_count);
			} else {
				namelist.sl_nmods = infop->sap_npush;
				namelist.sl_modlist = (struct str_mlist *)infop->sap_list[0];
				if ((ioerr2 = stream_ioctl(Fds, SAD_VML, &namelist)) == 0) {
					warn(MSGSTR(MSG16, "Invalid major (%d)\n"), infop->sap_major);
				} else if (ioerr2 = 1) {
					fprintf(stderr, MSGSTR(MSG17, "\nModule          Available\n"));
					fprintf(stderr,   "------          ---------\n");
					for (k = 0; k < infop->sap_npush; k++) {
						namelist.sl_nmods = 1;
						namelist.sl_modlist = (struct str_mlist *)infop->sap_list[k];
						fprintf(stderr, " %-16s", infop->sap_list[k]);
						if(stream_ioctl(Fds, SAD_VML, &namelist) == 0) {
							fprintf(stderr, MSGSTR(MSG18, "   Yes\n"));
						} else {
							fprintf(stderr, MSGSTR(MSG19, "   No\n"));
						}
					}
				} else {
					warn(MSGSTR(MSG20, "Unexpected IOCTL(SAD_VML) error (%d) checking module list.\n"), errno);
				}
			}
			break;
			
		case EEXIST:
			warn(MSGSTR(MSG21, "Device (%d,%d) already configured\n"), infop->sap_major, infop->sap_minor);
			break;
			
		case EFAULT:
			warn(MSGSTR(MSG22, "Program bug - bad arg/infop (%p)\n"), infop);
			break;

		case ENOSTR:
			warn(MSGSTR(MSG23, "Major (%d) is not a STREAMS device\n"), infop->sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(MSG24, "Device (%d,%d) not configured for autopush\n"), infop->sap_major, infop->sap_minor);
			break;

		case ERANGE:
			warn(MSGSTR(MSG25, "Minor (%d) not first number in configured range\n"), infop->sap_minor);
			break;

		case ENOSR:
			warn(MSGSTR(MSG26, "No memory for autopush data structure\n"));
			break;

		default:
			switch (infop->sap_cmd) {
			case SAP_ONE:   sap_cmd_text = "ONE";   break;
			case SAP_RANGE: sap_cmd_text = "RANGE"; break;
			case SAP_ALL:   sap_cmd_text = "ALL";   break;
			default:        sap_cmd_text = "???";   break;
			}
			warn(MSGSTR(MSG27, "Unexpected IOCTL(SAD_SAP/%s) error: %d\n"), sap_cmd_text, saved_errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf void
read_ap_modlist (apf)
	FILE	* apf;
	
{
	char		buf[256];
	int		nitem;
	struct strapush	info;
	
	while (fgets(buf, sizeof buf, apf) != nilp(char)) {
		Line_count++;
		nitem = split_line(buf, &info);
		if (nitem > 0 ) {
			if (load_ap_modules(&info) < 0) {
				fprintf(stderr, MSGSTR(MSG28, "Continuing...\n"));
			}
		} else if (nitem < 0) {
			break;
		}
	}
	return;
}

/*------------------------------------------------------------------------------------*/

staticf int
remove_ap_modules (dmajor, dminor)
	long	dmajor, dminor;
{
	struct strapush	ap_info;
	int		ioerr;
	
	ap_info.sap_major = dmajor;
	ap_info.sap_minor = dminor;
	ap_info.sap_cmd = SAP_CLEAR;
	if (ioerr = stream_ioctl(Fds, SAD_SAP, &ap_info) < 0) {
		switch (errno) {

		case EFAULT:
			warn(MSGSTR(MSG29, "Program bug - bad arg/infop (%p)\n"), &ap_info);
			break;

		case EINVAL:
			warn(MSGSTR(MSG30, "Major (%d) configured for ALL minors. Use \"-m0\" for minor.\n"),
						ap_info.sap_major);
			break;

		case ENOSTR:
			warn(MSGSTR(MSG31, "Major (%d) is not a STREAMS device\n"), ap_info.sap_major);
			break;

		case ENODEV:
			warn(MSGSTR(MSG32, "Device (%d,%d) not configured for autopush\n"), ap_info.sap_major, ap_info.sap_minor);
			break;

		case ERANGE:
			warn(MSGSTR(MSG33, "Minor (%d) not first number in configured range\n"), ap_info.sap_minor);
			break;

		case ENOSR:
			warn(MSGSTR(MSG34, "No memory for autopush data structure\n"));
			break;

		default:
			warn(MSGSTR(MSG35, "Unexpected IOCTL(SAD_SAP/CLEAR) error: %d\n"), errno);
			break;
		}
	}
	return ioerr;
}

/*------------------------------------------------------------------------------------*/

staticf int
get_next_arg (string1, string2, result)
	char	* string1, * string2, ** result;
	
	/* string1 & string2 are consecutive elements of argv[]. We expect string1 to
	 * be of the form "-x...", a 1-character switch.  If string1 is longer than
	 * 2 characters, remainder of string is returned as result; otherwise string2 is
	 * returned as result (no checks).  Function returns:
	 *		-1   cmd error
	 *		 1   only string1 used
	 *		 2   both string1 & string2 used
	 */
{
	int	ret_code = -1;
	int	nch;
	
	if (*string1 == '-') {
		if ((nch = (int)strlen(string1)) > 2) {
			*result = string1 + 2;
			ret_code = 1;
		} else if (nch == 2) {
			*result = string2;
			ret_code = 2;
		}
	}
	return ret_code;
}

/*------------------------------------------------------------------------------------*/

staticf int
get_mm (narg, arglist, majp, minp)
	int	narg;
	char	* arglist[];
	long	* majp, * minp;

	/*
	 * Looking for segment of command line "-M major -m minor" where order
	 * doesn't matter, and "-m minor" may be missing (returned as -1 ==> all)
	 */
{
	int	maj_found, min_found;
	int	karg1, karg2, knext;
	int	ret_code = 0;
	char	* result;
	
	
	maj_found = false;
	min_found = false;
	*minp = -1;
	*majp = -1;
	karg1 = 2;
	while (karg1 < narg) {
		karg2 = (karg1 < narg - 1) ? karg1 + 1 : karg1;
		if ((knext = get_next_arg(arglist[karg1], arglist[karg2], &result)) == -1) {
			ret_code = -1;
			break;
		} else {
			switch (arglist[karg1][1]) {

			case 'm':
				if (min_found) {
					ret_code = -1;
				} else {
					*minp = atol(result);
					min_found = true;
				}
				break;

			case 'M':
				if (maj_found) {
					ret_code = -1;
				} else {
					*majp = str_to_major(result);
					maj_found = true;
				}
				break;

			default:
				ret_code = -1;
				break;
			}
		}
		karg1 += knext;
	}
	if (*majp == -1 || *minp == -1) {
		ret_code = -1;
	}
	return (ret_code);
}

staticf long
str_to_major (string)
	char	* string;
{
	char	* cp;
	char	info_buf[128];
	int	fd;
	struct strioctl	stri;

	if ( isdigit(string[0]) )
		return atol(string);

	if ((fd = stream_open(NULL_DEV_NAME, 2)) == -1) {
		warn(MSGSTR(MSG36, "str_to_major: couldn't open '%s', %s\n"), NULL_DEV_NAME, errmsg(0));
		stream_close(fd);
		return -1L;
	}
	if (stream_ioctl(fd, I_PUSH, "sc") == -1) {
		warn(MSGSTR(MSG37, "str_to_major: couldn't PUSH 'sc', %s\n"), errmsg(0));
		stream_close(fd);
		return -1L;
	}
	stri.ic_cmd = ND_GET;
	stri.ic_timout = 0;
	sprintf(info_buf, "dinfo%c%s", '\0', string);
	stri.ic_dp = info_buf;
	stri.ic_len = sizeof(info_buf);
	if (stream_ioctl(fd, I_STR, &stri) == -1) {
		warn(MSGSTR(MSG38, "str_to_major: ioctl to get major number for '%s' failed, %s\n"), string, errmsg(0));
		stream_close(fd);
		return -1L;
	}
	stream_close(fd);
	if (cp = get_cp(info_buf, "device number"))
		return atol(cp);
	return -1L;
}

staticf char *
get_cp (buf, name)
	char	* buf;
	char	* name;
{
reg	char	* cp;
	int	name_len;

	name_len = strlen(name);
	for (cp = buf; cp[0]  &&  cp[1]; cp += (strlen(cp)+1)) {
		if (cp[0] == name[0]  &&  strncmp(cp, name, name_len) == 0) {
			if (cp = strchr(cp, '='))
				cp += 3;
			return cp;
		}
	}
	return nilp(char);
}

/********************************** M A I N ******************************************/


main (argc, argv)
	int	argc;
	char	* argv[];
	
	/* extract command line arguments:
	 *
	 *   -f filename                or
	 *   -r -M major -m minor       or
	 *   -g -M major -m minor  
	 *
	 * -f, -r, -g must appear first; -M, -m can appear in either order
	 *
	 */
	 
{
	FILE		* apf;
	long		dmajor, dminor;			/* major,minor from cmd line */
	int		exit_code = 0;
	char		* filename;
	struct strapush	info;
	
	setlocale(LC_ALL, "");
	catd = catopen(MF_AUTOPUSH, NL_CAT_LOCALE);

	set_program_name(argv[0]);
	open_sad_driver();

	if (argc < 2 || argv[1][0] != '-') {
#ifdef NETWARE
		read_ap_modlist(stdin);
		stream_close(Fds);
		exit(0);
#else
		stream_close(Fds);
		usage(MSGSTR(USAGE1, "-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"));
#endif
	}
	switch (argv[1][1]) {
	case 'f':
		if (argc > 3 || (argc -= get_next_arg(argv[1], argv[argc-1], &filename)) != 1) {
			usage(MSGSTR(USAGE1, "-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"));
			exit_code = 1;
		} else {
	 		if ((apf = fopen(filename, "r")) == nilp(FILE)) {
				stream_close(Fds);
	 			fatal("Can't open file: %s\n", filename);
			} else {
				read_ap_modlist(apf);
			}
	 	}
		break;
		
	case 'r':
		if (get_mm(argc, argv, &dmajor, &dminor) == 0) {
			(void)remove_ap_modules(dmajor, dminor);
		} else {
			usage(MSGSTR(USAGE1, "-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"));
			exit_code = 1;
		}
		break;

	case 'g':
		if (get_mm(argc, argv, &dmajor, &dminor) == 0) {
			show_ap_modules(dmajor, dminor);
		} else {
			usage(MSGSTR(USAGE1, "-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"));
			exit_code = 1;
		}
		break;
		
	default:
		usage(MSGSTR(USAGE1, "-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"));
		exit_code = 1;
		break;
	}
	stream_close(Fds);
	exit(exit_code);
}
