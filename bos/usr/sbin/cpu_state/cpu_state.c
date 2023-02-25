static char sccsid[] = "@(#)79        1.9  src/bos/usr/sbin/cpu_state/cpu_state.c, cmdmp, bos412, 9446A412a 11/14/94 10:10:22";
/*
 * COMPONENT_NAME: (CMDMP) 
 *
 * FUNCTIONS: main, parse_options, status
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <errno.h>
#include <sys/mdio.h>
#include <locale.h>
#include <nl_types.h>
#include <sys/systemcfg.h>
#include <sys/processor.h>
#include <nlist.h>
#include <odmi.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <fcntl.h>
#include <sys/systemcfg.h>

#include "cpu_state_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CPUSTATE,Num,Str)
nl_catd catd;

#define MAX_CPUS 8

static char *status();
static void parse_options();

static short lflag, eflag, dflag, error;
static int target;

/* 
 * NAME: main
 *
 * FUNCTION: list the cpus and their state or enable/disable a cpu.
 *			 looks for the number of cpus, the location codes and the
 *			 iplcb slot number of the cpus in ODM.
 *			 uses the MCPUGET and MCPUSET ioctls.  
 *
 * RETURNS : exit code 0 or 1.
 */

main(int argc, char **argv)
{
	int ret, nvram, arch;
	MACH_DD_IO param;
	short cnt, index, i;
	short failed = 0;
	int logic = 0;
	int valid = FALSE;
	struct CuDv *cpu_list, *free_pt;
	int fail[MAX_CPUS];
	struct listinfo listinfo;
	int en_or_dis;
	char *name;
	char odm_string[50];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_CPU_STATE, NL_CAT_LOCALE);	

	/* test if we are on a Pegasus machine */

#ifdef __rs6k_smp_mca()
		if (!__rs6k_smp_mca()) {
#else
			if (!__power_pc()) {
#endif 
		fprintf(stderr, MSGSTR(NOTSUP,
		"1731-007: Command not supported\n"));
		exit(1);
	}

	parse_options(argc, argv);
  
	param.md_size = 1;
	param.md_incr = MV_WORD;
	param.md_data = (char *)&en_or_dis;

	/*
	 * read the cpu objects in ODM in order to get the physical numbers 
	 * of the cpus and their location 
	 */
	odm_initialize();
	
	sprintf(odm_string, "name like 'proc*'");
	
	cpu_list = (struct CuDv *)odm_get_list(CuDv_CLASS, odm_string, &listinfo, MAX_CPUS, 1);
	if (cpu_list == (struct CuDv *) -1) {
		fprintf(stderr, MSGSTR(ODM_ERR, "1731-005: Cannot read cpu objects in ODM\n"));
		exit(1);
	}
	free_pt = cpu_list;

	/* open nvram to enable ioctls usage */
	nvram = open("/dev/nvram", O_RDWR);
	if (nvram == -1) {
		fprintf(stderr, MSGSTR(NV, "1731-010: Cannot open /dev/nvram\n"));
		odm_free_list(free_pt,&listinfo);
		exit(1);
	}

	if (lflag) { 
		fprintf(stdout, MSGSTR(HEAD, "\tName\tCpu\t Status \tLocation\n"));
		/* get the state of each cpu and display it */
		for (i = 0; i < listinfo.num; i++) {
			name = cpu_list->name;
			/* skip "proc" string */
			name += 4;
			cnt = atoi(name);
			param.md_cpunum = (long)cnt;
			ret = ioctl(nvram, MCPUGET, &param);
			if (ret == -1) {
				switch (errno) {
				case EINVAL :
					fprintf(stderr, MSGSTR(FAIL,
					"1731-008: Command failed\n"));
					odm_free_list(free_pt,&listinfo);
					exit(1);
				case ENXIO :
				default :			
					if (cpu_list->status == AVAILABLE) {
					  fprintf(stdout, "\tproc%1d\t%1d\tno reply\t%11s\n", cnt, 
							  logic, cpu_list->location);
					  logic++;
					  /* memorize failure to display an error message later */
					  fail[failed]=cnt;
					  failed++;
					} 
					else {
					  fprintf(stdout, "\tproc%1d\t-\tno reply\t%11s\n", cnt, 
							  cpu_list->location);
					}
				}
			}
			else {
				if (cpu_list->status == AVAILABLE) {
					fprintf(stdout, "\tproc%1d\t%1d\t%8s\t%11s\n", cnt, 
							logic, status(en_or_dis), cpu_list->location);
					logic++;
				}
				else {
					fprintf(stdout, "\tproc%1d\t-\t%8s\t%11s\n", cnt, 
							status(en_or_dis), cpu_list->location);
				}
			}
			cpu_list++;
		}
		if (failed) {
			for (i =0; i<failed; i++) {
				fprintf(stderr, MSGSTR(GETFAIL,
				"1731-011: Failed in getting state of proc%1d\n"), fail[i]);
			}
			odm_free_list(free_pt, &listinfo);
			exit(1);
		}
	}

	else {
	  /* eflag or dflag */
	  i=0;
	  while ((i<listinfo.num) && (valid == FALSE)) {
		name = cpu_list->name;
		/* skip "proc" string */
		name += 4;
		cnt = atoi(name);
		if (cnt == target) {
		  valid=TRUE;
		}
		i++;
		cpu_list++;
	  }
	  
	  if (valid == FALSE) {
		fprintf(stderr, MSGSTR(NUMBER, "1731-009: Invalid cpu number\n"));
		  odm_free_list(free_pt,&listinfo);
		  exit(1);
		}
		 
		param.md_cpunum = (ulong)target;
			
		if (eflag) {
			/* enable the specifed cpu */
			en_or_dis = MCPU_ENABLED;
			ret = ioctl(nvram, MCPUSET, &param);
			if (ret == -1) {
				switch (errno) {
				case ENXIO :
					fprintf(stderr, MSGSTR(NOTAVAIL2,
							"1731-003: Cannot enable proc%1d\n"),target);
					break;
				case EINVAL :
				default:
					fprintf(stderr, MSGSTR(FAIL,
							"1731-008: Command failed\n"));
				}
				odm_free_list(free_pt,&listinfo);
				exit(1);
			}
		}

		else {	   /* dflag */
			/* disable the specified cpu */
			en_or_dis = MCPU_DISABLED;
			ret = ioctl(nvram, MCPUSET, &param);
			if (ret == -1) {
				switch (errno) {
				case ENXIO :
					fprintf(stderr, MSGSTR(NOTAVAIL3, 
					"1731-004: Cannot disable proc%1d\n"),target);
					break;

				case EINVAL :
				default:
					fprintf(stderr, MSGSTR(FAIL,
					"1731-008: Command failed\n"));
				}
			odm_free_list(free_pt,&listinfo);
			exit(1);
			}
		}
	}
	if (odm_free_list(free_pt,&listinfo) == -1)
		exit(1);
	odm_terminate();
	exit(0);
} /* main */

/* 
 * NAME: status
 *
 * FUNCTION: decode the status of the cpu.	
 *
 * RETURNS : status of the cpu.
 */

char *status(int en_or_dis)
{
	switch (en_or_dis) {
	case MCPU_DISABLED :
		return(MSGSTR(DISABLED, "disabled"));
	case MCPU_ENABLED :
		return(MSGSTR(ENABLED, " enabled"));
	default:
		return(MSGSTR(NONE, "????????"));
	}
}

/* 
 * NAME: parse_options
 *
 * FUNCTION: read the command line.	 
 *
 * RETURNS : nothing.
 */

void parse_options(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	char *getstr = "le:d:";
	char *usage = "Usage: cpu_state { -l | -d Number | -e Number }\n";
	int c;

	if (argc == 1) {
		fprintf(stderr, MSGSTR(USAGE, usage));
		exit(1);
	}
	lflag = FALSE;
	eflag = FALSE;
	dflag = FALSE;
	error = FALSE;
	while ((c = getopt(argc, argv, getstr))!= EOF)
	{
		switch (c)
		{
			case 'l':
				if ((lflag) || (eflag) || (dflag))
					error = TRUE;
				else
					lflag = TRUE;
				break;
			case 'e':
				if ((lflag) || (eflag) || (dflag))
					error = TRUE;
				else {
					eflag  = TRUE;
					if (isdigit((int)*optarg))
						target = atoi(optarg);
					else
						error = TRUE;
				}
				break;
			case 'd':
				if ((lflag) || (eflag) || (dflag))
					error = TRUE;
				else {
					dflag = TRUE;
					if (isdigit((int)*optarg))
						target = atoi(optarg);
					else
						error = TRUE;
				}
				break;
			case '?':
				error = TRUE;
		} /* case */
		if (error)
		{
			fprintf(stderr, MSGSTR(USAGE, usage));
			exit(1);
		}
	} /* while */
	if (!(lflag || eflag || dflag)) {
		fprintf(stderr, MSGSTR(USAGE, usage));
		exit(1);
	}
} /* parse_options */

