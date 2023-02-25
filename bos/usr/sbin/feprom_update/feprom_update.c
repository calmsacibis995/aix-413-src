static char sccsid[] = "@(#)88	1.3  src/bos/usr/sbin/feprom_update/feprom_update.c, cmdcntl, bos411, 9436C411a 9/8/94 04:56:30";
/*
 * COMPONENT_NAME: (CMDCNTL) 
 *
 * FUNCTIONS: main, parse_options
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <sys/mdio.h>
#include <locale.h>
#include <nl_types.h>
#include <sys/systemcfg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/reboot.h>

#include "feprom_update_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_FEPROMUPDATE,Num,Str)
nl_catd catd;

#define FIRSTSIZE	512
#define NEXTSIZE	4096


static char fflag = 0;
static const char *lockfile = "/tmp/feprom.lock";
static char *file_name;

static void parse_options(int, char**);
static void exit_prog(int);

/* 
 * NAME: main
 *
 * FUNCTION: read a file and copy it into the Flash Eprom memory
 *           using the MFEPROMPUT ioctl.
 *           reboot the system. 
 *
 * RETURNS : exit code (0 or 1).
 */

main(int argc, char **argv)
{
	int ret, nb_read, nvram;
	int to_read;
	unsigned long written;
	MACH_DD_IO param;
	char *buf;
	int fd;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_FEPROM_UPDATE,NL_CAT_LOCALE);

	/* check if command is supported */
#ifdef __rs6k_smp_mca()
		if (!__rs6k_smp_mca()) {
#else
			if (!__power_pc()) {
#endif 
		fprintf(stderr, MSGSTR(NOTSUP,
		"1734-007: Command not supported\n"));
		exit(1);
	}

	/* check that the lock is free */
	fd = open(lockfile, O_EXCL|O_CREAT);
	if (fd == -1) {
		fprintf(stderr, MSGSTR(LOCK, "1734-015: Cannot run command for \
lock file %s exists\n"), lockfile);
		exit(1);
	}
	(void)close(lockfile);

	parse_options(argc, argv);
	
	/* open nvram to enable ioctls usage */
	nvram = open("/dev/nvram", O_RDWR);
	if (nvram == -1) {
		fprintf(stderr, MSGSTR(NV, "1734-014: Cannot open /dev/nvram\n"));
		exit_prog(1);
	}

	buf = (char *) malloc((size_t) NEXTSIZE);
	if (buf == NULL) {
		fprintf(stderr, MSGSTR(MEM, "1734-001: No memory\n"));
		exit_prog (1);
	}
	param.md_data = buf;
	param.md_length = &written;
	to_read = FIRSTSIZE; 	/* the first time only 512 bytes are sent */
	
	fd = (int) open(file_name, O_RDONLY, 0);
	if (fd == -1) {
		fprintf(stderr, MSGSTR(OPEN,
		"1734-002: Cannot open file %s\n"), file_name);
		exit_prog(1);
	}
	do {
		/* read in the file */
		if ((nb_read = read(fd, buf, to_read)) == -1) {
			fprintf(stderr, MSGSTR(READ, "1734-012: Cannot read file %s\n"), file_name);
			exit_prog(1);
		}
		
		/* if read didn't return end-of-file write into feprom */
		if (nb_read != 0) {		
			param.md_size = nb_read;
			/* send data to the bump through the ioctl command */
			ret = ioctl(nvram, MFEPROMPUT, &param);
			if (ret == -1) {
				switch (errno) {
				case EACCES : 
					fprintf(stderr, MSGSTR(ACCESS, 
					"1734-003: This commands only runs on service mode\n"));
					break;
				case EFAULT :
					fprintf(stderr, MSGSTR(FAULT, 
					"1734-004: Cannot transfer data between kernel space and user memory\n"));
					break;
				case EINVAL :
					fprintf(stderr, MSGSTR(FORMAT, 
					"1734-005: The format of this file %s is invalid\n"), file_name);
					break;
				default:
					fprintf(stderr, MSGSTR(FAIL, "1734-008: Command failed\n"));
				}
				exit_prog(1);
			}
			if (written != nb_read) {
				fprintf(stderr, MSGSTR(WRITE, "1734-009: Cannot write in FEPROM\n"));
				exit_prog(1);
			}
			if (to_read == FIRSTSIZE)
				to_read = NEXTSIZE;
		}
	} while (nb_read != 0);
	if (close(fd) != 0) {
		fprintf(stderr, MSGSTR(CLOSE, "1734-006: Cannot close file %s\n"), file_name);
		exit_prog(1);
	}
	/* reboot the system */
	ret = reboot(RB_HARDIPL, 0);
	if (ret == -1) {
		fprintf(stderr, MSGSTR(REBOOT, "1734-013: Reboot failed\n"));
		exit_prog(1);
	}
	exit_prog(0);
} /* main */

/* 
 * NAME: exit_prog
 *
 * FUNCTION: remove the lock file and exits
 *
 * RETURNS : exits.
 */

void
exit_prog(int mode)
{
	int ret;

	ret = unlink(lockfile);
	if (ret == -1) {
		fprintf(stderr, MSGSTR(UNLINK, "1734-016: Cannot remove %s file\n"), lockfile);
		exit(1);
	}
	exit(mode);
}

/* 
 * NAME: parse_options
 *
 * FUNCTION: read the command line.
 *           if -f flag is not mentionned, ask for a confirmation
 *
 * RETURNS : nothing.
 */

void 
parse_options(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	int c;
	short errflag;
	char *getstr = "f";
	char *usage = "Usage: feprom_update [-f] { file }\n";

	errflag = 0;
  	while ((c = getopt(argc, argv, getstr))!= EOF)
  	{
  		switch (c)
  		{
		case 'f':
			fflag++;
			break;
			
  		case '?':
  			errflag++;
  		} /* case */
  		if (errflag)
  		{
			fprintf(stderr, MSGSTR(USAGE, usage));
  			exit_prog(1);
  		}
  	} /* while */
	if (fflag) {
		if (argc != 3) {
			fprintf(stderr, MSGSTR(USAGE, usage));
			exit_prog(1);
		} else file_name = argv[2];
	}
	else {
		if (argc != 2) {
			fprintf(stderr, MSGSTR(USAGE, usage));
			exit_prog(1);
		}
		else {
			char answer[2];

			fprintf(stdout, MSGSTR(CONFIRM, "This command will reboot the system. \
Are you sure you want to run it <y/n> ?\n"));
			answer[0] = getchar();
			answer[1] = '\0';
			if (!strcmp(answer, MSGSTR(Y, "y")))
				file_name = argv[1];
			else {
				fprintf(stdout, MSGSTR(EXIT, "1734-011: Command exited\n"));
				exit_prog(0);
			}
		}
	}
} /* parse_options */

