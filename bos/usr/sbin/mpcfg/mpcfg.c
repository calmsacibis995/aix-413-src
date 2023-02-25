static char sccsid[] = "@(#)47	1.8  src/bos/usr/sbin/mpcfg/mpcfg.c, dsaubump, bos41B, 412_41B_sync 12/23/94 03:04:09";
/*
 * COMPONENT_NAME: DSAUBUMP
 *
 * FUNCTIONS: main, parse_options, output_..., get_..., put_...,
 *            select_changes, display_corrupted, searchlastfield,
 *	      isnumber, ask_passwd, handle_error, init_..., errusage
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/systemcfg.h>

#include "mpcfg_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_MPCFG,Num,Str)
nl_catd catd;

#define NCONTRACTFLAGS	4  /* Contract = Service Support */
#define CONTRACT_TYPE	NCONTRACTFLAGS-1
#define CONTRACT_FL	1 /* 0 & 1 are boolean */
#define NDIAGFLAGS	11 /* 12 -> 11 to suppress ineffective last flag */
/*#define DIAG_FL		(NDIAGFLAGS - 1)*//* last is not boolean */
/*#define MAXMESSMODE	6 *//* last flag <= 6 */
/*#define MAXMESSMODE	1 *//* last flag <= 1 */
#define NMODEMCONF	8
#define MODEMCONFINT	4 /* 1 .. 4 are integer */
#define NPHONES		6

#define MAXARG		NDIAGFLAGS /* maximum number of arguments with cflag */

#define MAXINDEXLEN	7
#define MAXFLAGLEN	45
#define MAXMODEMCONFLEN 45
#define MAXPHONELEN	45

#define MAXLINELEN	256 /* line size for restore */
#define BUFLEN		512 /* buffer size for restore */

#define CONTRACT_FLAGS	1 /* BUMP info type */
#define DIAG_FLAGS	2
#define MODEMCONF	4
#define PHONES		3
#define PASSWORD	5

#define READPASSLEN	20
#define CONTRACT_PASSLEN 6
#define CONTRACT_FLAGS_SIZE (sizeof(struct contract))
#define DIAG_PASSOFFSET 12
#define DIAG_PASSLEN	16
#define DIAG_FLAGS_SIZE (DIAG_PASSOFFSET + DIAG_PASSLEN)
#define MODEMCONF_PASSOFFSET (MODEMCONF_SIZE - DIAG_PASSLEN)
#define MODEMCONF_SIZE	512
#define MODEMCONF_DATA	0 /* new bump interface config type in info type */
#define PHONE_LEN	20
#define PHONE_SIZE	(sizeof(struct phone_number))
#define PASSWORD_SIZE	(sizeof(struct password))

#define SITEPARAM	0x00000 /* new bump interface added to info type */
#define MODEMPARAM	0x10000

#define RemoteServiceSupport	0x01
#define QuickOnCallService	0x02
#define UpdateServiceSupportDays	0x01
#define UpdateRemoteServiceSupport	0x02
#define UpdateQuickOnCallService	0x04
#define UpdateServiceSupportType	0x08
#define UpdateDiagFlag			0x80
#define UpdatePhone			0x01
#define RazPhone			0x00

struct contract {
		unsigned char update;
		unsigned char contracted_remote;
		/* RemoteServiceSupport & QuickOnCallService */
		short contract_validation; /* nb of days */
		char  contract_type;
		char  rolling_password[CONTRACT_PASSLEN];
		char  filler;
	};
struct phone_number {
		short update;
		short type;
		char  phnb[PHONE_LEN];
		char  password[DIAG_PASSLEN];
	};
struct password {
		int   type;
		char  old[16];
		char  new[16];
	};

/* procedures */
static void output_contractflags();
static void get_contractflags();
static void put_contractflags();
static void output_diagflags();
static void get_diagflags();
static void put_diagflags();
static void output_modemconf();
static void get_modemconf();
static void put_modemconf();
static void output_phones();
static void get_phones();
static void put_phones();
static void put_password();
static void select_changes();
static void display_corrupted();
static char *searchlastfield();
static int  isnumber();
static void ask_passwd();
static void handle_error();
static void init_contractflags();
static void init_diagflags();
static void init_modemconf();
static void init_phones();
static void parse_options(int, char**);
static void errusage();

extern void read_modemparam(char *file, char *buffer);

static int dflag, cflag, rflag, sflag;
static int fflag, mflag, pflag, Sflag, wflag;

		/* number of valid index */
static int	contract_nbchg;
static int	diag_nbchg;
static int	modemconf_nbchg;
static int	phone_nbchg;
		/* list of index of flags to modify */
static int	contractflag_index[NCONTRACTFLAGS];
static int	diagflag_index[NDIAGFLAGS];
static int	modemconf_index[NMODEMCONF];
static int	phone_index[NPHONES];
		/* flag value only if flag to modify or all values to display */
static int	contractflag_value[NCONTRACTFLAGS];
static unsigned char diagflag_value[NDIAGFLAGS];
static char	*modemconf_value[NMODEMCONF];
static char	*phone_value[NPHONES];
static int	password_number; /* 0 = customer, 1 = general */
static char	diag_password[DIAG_PASSLEN];
static int	diag_pass = 0;
static char	*modem_file; /* modem file name for error message in yacc */
static int	contract_autorization = 0;

static MACH_DD_IO param;
static ulong data_length;

static int site_halflen[NMODEMCONF]={64,3,1,1,1,6,6,8};
static char site_mnemonic[NMODEMCONF][2]={
		"--","RS","RL","RT","RN","CA","LI","PW"};
static char phone_mnemonic[NPHONES][2]={"PS","PS","PH","PH","PY","PO"};
static char nullstring[1] = "";

/* 
 * NAME: main
 *
 * FUNCTION: 
 *
 * RETURNS : exit code 0 or 1.
 */

main(int argc, char **argv)
{
	int nvram;
	char *file = "/etc/lpp/diagnostics/data/bump";
	FILE *stream;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_MPCFG, NL_CAT_LOCALE);

#ifdef __rs6k_smp_mca
	if (!__rs6k_smp_mca()) {
#else
	if (!__power_pc()) {
#endif
		fprintf(stderr, MSGSTR(NOTSUP,
			"0044-004: Command not supported\n"));
		exit(1);
	}

	parse_options(argc, argv);
	

	/* open nvram to enable ioctls usage */
	nvram = open("/dev/nvram", O_RDWR);
	if (nvram == -1) {
		fprintf(stderr, MSGSTR(NV, "0044-001: Cannot open /dev/nvram\n"));
		exit(1);
	}

	param.md_incr = MV_BYTE; /* user buffer size in bytes */
	param.md_length = &data_length;

	if (dflag) {
		if (fflag) {
			output_diagflags(stdout, nvram);
		}
		if (mflag) {
			output_modemconf(stdout, nvram);
		}
		if (pflag) {
			output_phones(stdout, nvram);
		}
		if (Sflag) {
			output_contractflags(stdout, nvram);
		}
	}
	else if (sflag) {
		stream = fopen(file, "w");
		if (stream == NULL) {
			fprintf(stderr, MSGSTR(OPEN, "0044-002: Cannot open %s\n"), file);
			exit(1);
		}
		fprintf(stream, "%s:\n", MSGSTR(SUPPORT,"SERVICE SUPPORT"));
		output_contractflags(stream, nvram);
		fprintf(stream, "\n%s:\n", MSGSTR(DIAGNOSTIC,"DIAGNOSTIC FLAGS"));
		output_diagflags(stream, nvram);
		fprintf(stream, "\n%s:\n",
			MSGSTR(MODEMSITECONF,"MODEM AND SITE CONFIGURATION"));
		output_modemconf(stream, nvram);
		fprintf(stream, "\n%s:\n", MSGSTR(PHONENB,"PHONE NUMBERS"));
		output_phones(stream, nvram);
		(void)fclose(stream);
	}
	else if (cflag) {
		if (fflag) {
			put_diagflags(nvram);
		}
		else if (mflag) {
			put_modemconf(nvram);
		}
		else if (pflag) {
			put_phones(nvram);
		}
		else if (Sflag) {
			put_contractflags(nvram);
		}
		else if (wflag) {
			put_password(nvram);
		}
	}
	else if (rflag) {
		select_changes(nvram, file);
		put_diagflags(nvram);
		put_phones(nvram);
		put_modemconf(nvram);
		/* contract flags do not have to be restored
		* put_contractflags(nvram);
		*/
	}
	(void)close(nvram);
}

/*
 * NAME: output_contractflags
 *
 * FUNCTION: calls get_contractflags from NVRAM and write the flags to a file.
 *
 * RETURNS : None
 *
 */
  
void
output_contractflags(FILE *stream, int nvram)
{
	int ret, i;
	int flaglen;
	int difflen;
	char	*flag_name[NCONTRACTFLAGS];

	init_contractflags(flag_name);
	get_contractflags(nvram, contractflag_value);

	/* display all contract flags */
	/* if string "index" too long, reduce string "name" */
	difflen = strlen(MSGSTR(INDEX, "Index")) - MAXINDEXLEN;
	flaglen = (difflen <= 0) ? MAXFLAGLEN : MAXFLAGLEN - difflen ;
	fprintf(stream, "%-*s %-*s\t%s\n", MAXINDEXLEN, MSGSTR(INDEX, "Index"),
		flaglen, MSGSTR(NAME, "Name"), MSGSTR(VALUE, "Value"));
	/* Warning: number+string <= 50 chars for diag change menu */
	/* => "%-3d\t%-*s" = 49 and \t must be in str[3] */
	/* instead of "%-7d %-*s" or "%d\t%-*s" */
	/* do not change without verify ubump */
	for (i=0; i<CONTRACT_TYPE; i++)
		fprintf(stream, "%-3d\t%-*s\t%d\n", i+1,
			MAXFLAGLEN, flag_name[i], contractflag_value[i]);
	i = CONTRACT_TYPE;
	if (contractflag_value[i] == '\0') {
		fprintf(stream, "%-3d\t%-*s\t\n", i+1,
			MAXFLAGLEN, flag_name[i]);
	}
	else {
		fprintf(stream, "%-3d\t%-*s\t%c\n", i+1,
			MAXFLAGLEN, flag_name[i], contractflag_value[i]);
	}
}

/*
 * NAME: output_diagflags
 *
 * FUNCTION: calls get_diagflags from NVRAM and write the flags to a file.
 *
 * RETURNS : None
 *
 */

void
output_diagflags(FILE *stream, int nvram)
{
	int ret, i;
	int flaglen;
	int difflen;
	char	*flag_name[NDIAGFLAGS];

	init_diagflags(flag_name);
	get_diagflags(nvram, diagflag_value);

	/* display all diagnostic flags */
	/* if string "index" too long, reduce string "name" */
	difflen = strlen(MSGSTR(INDEX, "Index")) - MAXINDEXLEN;
	flaglen = (difflen <= 0) ? MAXFLAGLEN : MAXFLAGLEN - difflen ;
	fprintf(stream, "%-*s %-*s\t%s\n", MAXINDEXLEN, MSGSTR(INDEX, "Index"),
		flaglen, MSGSTR(NAME, "Name"), MSGSTR(VALUE, "Value"));
	for (i=0; i<NDIAGFLAGS; i++)
		fprintf(stream, "%-3d\t%-*s\t%d\n", i+1,
			MAXFLAGLEN, flag_name[i], diagflag_value[i]);
}

/*
 * NAME: output_modemconf
 *
 * FUNCTION: calls get_modemconf from NVRAM and write the config to a file.
 *
 * RETURNS : None
 *
 */
  
void
output_modemconf(FILE *stream, int nvram)
{
	int ret, i;
	int flaglen;
	int difflen;
	char	*modemconf_name[NMODEMCONF];

	init_modemconf(modemconf_name);
	get_modemconf(nvram, modemconf_value);

	/* display all flags */
	/* if string "index" too long, reduce string "name" */
	difflen = strlen(MSGSTR(INDEX, "Index")) - MAXINDEXLEN;
	flaglen = (difflen <= 0) ? MAXMODEMCONFLEN : MAXMODEMCONFLEN - difflen ;
	fprintf(stream, "%-*s %-*s\t%s\n", MAXINDEXLEN, MSGSTR(INDEX, "Index"),
		flaglen, MSGSTR(NAME, "Name"), MSGSTR(VALUE, "Value"));
	for (i=0; i<NMODEMCONF; i++)
		fprintf(stream, "%-3d\t%-*s\t%s\n", i+1,
			MAXMODEMCONFLEN, modemconf_name[i], modemconf_value[i]);
}

/*
 * NAME: output_phones
 *
 * FUNCTION: calls get_phones from NVRAM and write the phones to a file.
 *
 * RETURNS : None
 *
 */
  
void
output_phones(FILE *stream, int nvram)
{
	int ret, i;
	int flaglen;
	int difflen;
	char	*phone_name[NPHONES];

	init_phones(phone_name);
	get_phones(nvram, phone_value);

	/* display all phones */
	/* if string "index" too long, reduce string "name" */
	difflen = strlen(MSGSTR(INDEX, "Index")) - MAXINDEXLEN;
	flaglen = (difflen <= 0) ? MAXPHONELEN : MAXPHONELEN - difflen ;
	fprintf(stream, "%-*s %-*s\t%s\n", MAXINDEXLEN, MSGSTR(INDEX, "Index"),
		flaglen, MSGSTR(PHONE, "Phone"), MSGSTR(NUMBER, "Number"));
	for (i=0; i<NPHONES; i++)
		fprintf(stream, "%-3d\t%-*s\t%s\n", i+1,
			MAXPHONELEN, phone_name[i], phone_value[i]);
}

/*
 * NAME: get_contractflags
 *
 * FUNCTION: get contract flags from NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
get_contractflags(int nvram, int *flag_value)
{
	int ret, i;
	struct contract buf_contract;

	/* get contract maintenance flags */
	param.md_data = (char *)&buf_contract;
	param.md_type = CONTRACT_FLAGS;
	param.md_size = CONTRACT_FLAGS_SIZE;
	ret = ioctl(nvram, MDINFOGET, &param);
	if ((ret == -1) || (data_length != CONTRACT_FLAGS_SIZE))
		handle_error(nvram);

	/* parse buffer */
	flag_value[0] = 
		buf_contract.contracted_remote & RemoteServiceSupport ? 1 : 0;
	/* 1 bit -> int */
	flag_value[1] =
		buf_contract.contracted_remote & QuickOnCallService ? 1 : 0;
	flag_value[2] = buf_contract.contract_validation; /* 2 bytes -> int */
	flag_value[3] = buf_contract.contract_type; /* 1 b -> char */
}

/*
 * NAME: get_diagflags
 *
 * FUNCTION: get diagnostic flags from NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
get_diagflags(int nvram, unsigned char *flag_value)
{
	int ret, i;
	unsigned char buf_diag[DIAG_FLAGS_SIZE];

	/* get diagnostic flags */
	param.md_data = (char *)&buf_diag[0];
	param.md_type = DIAG_FLAGS;
	param.md_size = DIAG_FLAGS_SIZE;
	ret = ioctl(nvram, MDINFOGET, &param);
	if ((ret == -1) || (data_length != DIAG_FLAGS_SIZE))
		handle_error(nvram);

	/* parse buffer */
	for (i = 0; i < NDIAGFLAGS; i++)
		flag_value[i] = buf_diag[i];
}

/*
 * NAME: get_modemconf
 *
 * FUNCTION: get configuration from NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
get_modemconf(int nvram, char **value)
{
	int ret, i;
	static char buf_modemconf[MODEMCONF_SIZE];
	static char	line[MAXLINELEN];
	/* static to return buf addresses instead of string */
	char *ptbuf;
	char *file = "/etc/lpp/diagnostics/data/.modem";
	FILE *stream;

	for (i = 0; i < NMODEMCONF; i++) {
		value[i] = nullstring;
	}

	/* get modem file name */
	stream = fopen(file, "r");
	if (stream != NULL ) {

		if (fgets(line,MAXLINELEN,stream) == NULL)
			display_corrupted(file, 1);
		fclose(stream);
		i = strlen(line);
		if (i == 0) display_corrupted(file, 1);
		i--;
		if (line[i] != '\n') display_corrupted(file, 1);
		line[i] = '\0';
		value[0] = line;
	}

	/* get site configuration */
	param.md_data = &buf_modemconf[0];
	param.md_type = MODEMCONF + SITEPARAM;
	param.md_size = MODEMCONF_SIZE;
	ret = ioctl(nvram, MDINFOGET, &param);
	if ((ret == -1) || (data_length > MODEMCONF_SIZE))
		handle_error(nvram);

	/* parse buffer */
	ptbuf = &buf_modemconf[MODEMCONF_DATA];
	while (*ptbuf == '*') {
		char first, second;
		int delta;

		*ptbuf = '\0'; /* end of the previous string */
		ptbuf++;
		first = *ptbuf;
		ptbuf++;
		second = *ptbuf;
		ptbuf++;
		delta = *ptbuf;
		for (i= 1; i<NMODEMCONF; i++) {
			if ((second == site_mnemonic[i][1]) /* more specific */
			   && (first == site_mnemonic[i][0])) {
				value[i] = ptbuf+1; /* char address */
				break;
			}
		}
		delta *= 2;
		ptbuf += delta - 3;
	}
}

/*
 * NAME: get_phones
 *
 * FUNCTION: get phone numbers from NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
get_phones(int nvram, char **value)
{
	int ret, i;
	static char buf_modemconf[MODEMCONF_SIZE];
	char *ptbuf;

	for (i = 0; i < NPHONES; i++) {
		value[i] = NULL;
	}

	/* get phone numbers */
	param.md_data = &buf_modemconf[0];
	param.md_type = MODEMCONF + SITEPARAM;
	param.md_size = MODEMCONF_SIZE;
	ret = ioctl(nvram, MDINFOGET, &param);
	if ((ret == -1) || (data_length > MODEMCONF_SIZE))
		handle_error(nvram);

	/* parse buffer */
	ptbuf = &buf_modemconf[MODEMCONF_DATA];
	while (*ptbuf=='*') {
		char first, second;
		int delta;

		*ptbuf = '\0'; /* end of the previous string */
		ptbuf++;
		first = *ptbuf;
		ptbuf++;
		second = *ptbuf;
		ptbuf++;
		delta = *ptbuf;
		for (i = 0; i<NPHONES; i++) {
			if ((second == phone_mnemonic[i][1]) /* more specific */
			   && (first == phone_mnemonic[i][0])) {
				if (value[i] == NULL) {
					/* multi phones with same mnemonic */
					value[i] = ptbuf+1; /* char address */
					break;
				}
			}
		}
		delta *= 2;
		ptbuf += delta - 3;
	}

	for (i = 0; i < NPHONES; i++) {
		if (value[i] == NULL) {
			value[i] = nullstring;
		}
	}
}

/*
 * NAME: put_contractflags
 *
 * FUNCTION: write contract flags to NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
put_contractflags(int nvram)
{
	struct contract buf_contract;
	int cnt, ind, ret;

	buf_contract.update = 0;
	buf_contract.contracted_remote = 0;
	/* fill buffer */
	for (cnt = 0; cnt < contract_nbchg ; cnt++) {
		ind = contractflag_index[cnt];
		switch (ind) {
		case 0:
			if (contractflag_value[0]) {
			 buf_contract.contracted_remote |= RemoteServiceSupport;
			}
			buf_contract.update |= UpdateRemoteServiceSupport;
			break;
		case 1:
			if (contractflag_value[1]) {
			  buf_contract.contracted_remote |= QuickOnCallService;
			}
			buf_contract.update |= UpdateQuickOnCallService;
			break;
		case 2:
			buf_contract.contract_validation =
				contractflag_value[2];
			buf_contract.update |= UpdateServiceSupportDays;
			break;
		case 3:
			buf_contract.contract_type =
				contractflag_value[3];
			buf_contract.update |= UpdateServiceSupportType;
			break;
		}
	}
	if (contract_nbchg > 0) {
		ask_passwd(buf_contract.rolling_password, CONTRACT_FLAGS);
		/* communicate new contract flags to the bump */
		param.md_data = (char *)&buf_contract;
		param.md_type = CONTRACT_FLAGS;
		param.md_size = CONTRACT_FLAGS_SIZE;
		ret = ioctl(nvram, MDINFOSET, &param);
		if (ret == -1)
			handle_error(nvram);
	}
}

/*
 * NAME: put_diagflags
 *
 * FUNCTION: write diagnostic flags to NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
put_diagflags(int nvram)
{
	unsigned char buf_diag[DIAG_FLAGS_SIZE];
	int cnt, ind, ret;

	/* initialize buffer to 0 */
	for (ind = 0;ind < DIAG_FLAGS_SIZE; ind++)
		buf_diag[ind] = 0;
	
	/* fill buffer */
	for (cnt = 0; cnt < diag_nbchg ; cnt++) {
		ind = diagflag_index[cnt];
		buf_diag[ind] = diagflag_value[ind] | UpdateDiagFlag;
	}
	if (diag_nbchg) {
		/* ask_passwd(&buf_diag[DIAG_PASSOFFSET, DIAG_FLAGS); */
		/* At this time, no password is required */
		param.md_data = (char *)&buf_diag[0];
		param.md_type = DIAG_FLAGS;
		param.md_size = DIAG_FLAGS_SIZE;
		if (buf_diag[0] !=0) {
			/* special error message when no contract */
			contract_autorization = 1;
		}
		ret = ioctl(nvram, MDINFOSET, &param);
		if (ret == -1)
			handle_error(nvram);
		contract_autorization = 0;
	}
}

/*
 * NAME: put_modemconf
 *
 * FUNCTION: write configuration to NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
put_modemconf(int nvram)
{
	char buf_modemconf[MODEMCONF_SIZE];
	char *ptbuf;
	char *upd_modemconf_value[NMODEMCONF];
	int cnt, ind, ret, modemchg, sitechg;

	/* initialize buffer with old values */
	get_modemconf(nvram,upd_modemconf_value);
	
	/* fill intermediate buffer */
	/* modemchg = 0; load modem parameters at each time */
	modemchg = 1;
	sitechg = 0;
	for (cnt = 0; cnt < modemconf_nbchg ; cnt++) {
		ind = modemconf_index[cnt];
		upd_modemconf_value[ind] = modemconf_value[ind];
		if (ind == 0) {
			/* modem parameters */
			modemchg = 1;
		}
		else {
			/* site configuration */
			sitechg = 1;
		}
	}
	if (modemchg + sitechg) {
	    /* ask_passwd(&buf_modemconf[MODEMCONF_PASSOFFSET], DIAG_FLAGS); */
	    /* At this time, no password is required */
	    param.md_data = &buf_modemconf[0];
	    param.md_size = MODEMCONF_SIZE;
	    if (modemchg) {
		char *file = "/etc/lpp/diagnostics/data/.modem";
		FILE *stream;

		if (*upd_modemconf_value[0] != '\0') {
			read_modemparam(upd_modemconf_value[0],
				&buf_modemconf[MODEMCONF_DATA]);
		}
		else {
			/* no file name -> reset modem conf */
			buf_modemconf[MODEMCONF_DATA] = '\0';
		}
		param.md_type = MODEMCONF + MODEMPARAM;
		ret = ioctl(nvram, MDINFOSET, &param);
		if (ret == -1)
			handle_error(nvram);

		/* put modem file name */
		stream = fopen(file, "w");
		if (stream == NULL ) {
			fprintf(stderr,
			    MSGSTR(OPEN, "0044-002: Cannot open %s\n"), file);
			exit(1);
		}
		fprintf(stream,"%s\n",upd_modemconf_value[0]);
		fclose(stream);
	    }
	    if (sitechg) {
		/* fill buffer */
		param.md_type = MODEMCONF + SITEPARAM;
		ptbuf = &buf_modemconf[MODEMCONF_DATA];
		for (ind = 1; ind < NMODEMCONF; ind++) {
			*ptbuf = '*';
			ptbuf++;
			memcpy(ptbuf, site_mnemonic[ind], 2);
			ptbuf += 2;
			*ptbuf = (site_halflen[ind] + 2);
			ptbuf++;
			strncpy(ptbuf, upd_modemconf_value[ind],
				site_halflen[ind] * 2);
			ptbuf += (site_halflen[ind] * 2);
		}
		ret = ioctl(nvram, MDINFOSET, &param);
		if (ret == -1)
			handle_error(nvram);
	    }
	}
}

/*
 * NAME: put_phones
 *
 * FUNCTION: write phone numbers to NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
put_phones(int nvram)
{
	struct phone_number buf_phone;
	char *upd_phone_value[NPHONES];
	int cnt, ind, phone_t, ret;
	static int ph_type[NPHONES] = {0,0,1,1,2,3};
	static int ph_first[4] = {0,2,4,5};
	static int ph_second[4] = {1,3,4,5};

	get_phones(nvram, upd_phone_value);
	for (cnt = 0; cnt < phone_nbchg ; cnt++) {
		ind = phone_index[cnt];
		upd_phone_value[ind] = phone_value[ind];
	}

	/* fill buffer */
	param.md_data = (char *)&buf_phone;
	param.md_type = PHONES;
	param.md_size = PHONE_SIZE;
	/* ask_passwd(&buf_phone.password, DIAG_FLAGS); */
	for (cnt = 0; cnt < phone_nbchg ; cnt++) {
		ind = phone_index[cnt];
		phone_t = ph_type[ind];
		buf_phone.type = phone_t;
		if (upd_phone_value[ph_first[phone_t]] != NULL) {
			buf_phone.update = RazPhone;
			ret = ioctl(nvram, MDINFOSET, &param);
			if (ret == -1)
				handle_error(nvram);
			strncpy(&buf_phone.phnb[0],
				upd_phone_value[ph_first[phone_t]], PHONE_LEN);
			buf_phone.update = UpdatePhone;
			ret = ioctl(nvram, MDINFOSET, &param);
			if (ret == -1)
				handle_error(nvram);
			upd_phone_value[ph_first[phone_t]] = NULL;
			if (upd_phone_value[ph_second[phone_t]] != NULL) {
				/* only if 2 phones */
				strncpy(&buf_phone.phnb[0],
					upd_phone_value[ph_second[phone_t]],
					PHONE_LEN);
				ret = ioctl(nvram, MDINFOSET, &param);
				if (ret == -1)
					handle_error(nvram);
			}
		}
	}
}

/*
 * NAME: put_password
 *
 * FUNCTION: write customer or general password to NVRAM.
 *
 * RETURNS : returns or exits 1
 *
 */

void
put_password(int nvram)
{
	struct password buf_password;
	char new_pass[DIAG_PASSLEN];
	int ret, same;

	/* fill buffer */
	buf_password.type = 1 - password_number;
	ask_passwd(&buf_password.old, PASSWORD);
	do {
		ask_passwd(&buf_password.new, PASSWORD+1);
		ask_passwd(&new_pass[0], PASSWORD+2);
		same = strcmp(buf_password.new,new_pass);
		if (same != 0) {
		    fprintf(stdout, MSGSTR(NEED_DIAGPASS8,
		    "The password entry does not match, please try again.\n"));
		}
	} while (same);
	param.md_data = (char *)&buf_password;
	param.md_type = PASSWORD;
	param.md_size = PASSWORD_SIZE;
	ret = ioctl(nvram, MDINFOSET, &param);
	if (ret == -1)
		handle_error(nvram);
}

/*
 * NAME: select_changes
 *
 * FUNCTION: when restore function is invoked store changes
 *           between data in nvram and data to be restored.
 *
 * RETURNS : returns or exits 1
 *
 */

void
select_changes(int nvram, char *file)

{
	int	cnt, noline;
	FILE	*stream;
	int	old_contractflag_value[NCONTRACTFLAGS];
	unsigned char old_diagflag_value[NDIAGFLAGS];
	char	*old_modemconf_value[NMODEMCONF];
	char	*old_phone_value[NPHONES];
	char	*str, *str2;
	char	line[MAXLINELEN];
	static char bufstring[BUFLEN];
	int	buflen, stlen;

	/* get flags from NVRAM  */
	get_contractflags(nvram, old_contractflag_value);
	get_diagflags(nvram, old_diagflag_value);
	get_modemconf(nvram, old_modemconf_value);
	get_phones(nvram, old_phone_value);

	/* open file */
	stream = fopen(file, "r");
	if (stream == 0) {
		fprintf(stderr,
			MSGSTR(OPEN, "0044-002: Cannot open %s\n"), file);
		exit(1);
	}

	/* read contract flags from file and select the changes */
	if (fgets(line,MAXLINELEN,stream) == NULL) /* first title line */
		display_corrupted(file, 1);
	if (fgets(line,MAXLINELEN,stream) == NULL) /* second title line */
		display_corrupted(file, 2);
	noline = 2;
	contract_nbchg = 0;
	for (cnt=0; cnt<NCONTRACTFLAGS; cnt++) {
		noline++;
		if (fgets(line,MAXLINELEN,stream) == NULL)
			display_corrupted(file, noline);
		str = searchlastfield(line, cnt+1, file, noline);
		if (cnt == CONTRACT_TYPE) {
			if (strlen(str) > 1)
				display_corrupted(file, noline);
			contractflag_value[cnt] = *str;
		}
		else	{
			if (!isnumber(str))
				display_corrupted(file, noline);
			contractflag_value[cnt] = atoi(str);
			if (cnt <= CONTRACT_FL) {
				if (contractflag_value[cnt] < 0
				|| contractflag_value[cnt] > 1)
					display_corrupted(file, noline);
			}
		}
		if (contractflag_value[cnt] != old_contractflag_value[cnt]) {
			contractflag_index[contract_nbchg] = cnt;
			contract_nbchg++;
		}
	}

	/* read diagnostic flags from file and select the changes */
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* white line */
		display_corrupted(file, noline);
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* first title line */
		display_corrupted(file, noline);
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* second title line */
		display_corrupted(file, noline);
	diag_nbchg = 0;
	for (cnt=0; cnt<NDIAGFLAGS; cnt++) {
		noline++;
		if (fgets(line,MAXLINELEN,stream) == NULL)
			display_corrupted(file, noline);
		str = searchlastfield(line, cnt+1, file, noline);
		if (!isnumber(str))
			display_corrupted(file, noline);
		diagflag_value[cnt] = atoi(str);
		if ((diagflag_value[cnt] < 0) || (diagflag_value[cnt] > 1))
			display_corrupted(file, noline);
		/*if (diagflag_value[cnt] < 0)
		*	display_corrupted(file, noline);
		*if ((cnt < DIAG_FL)
		*&& (diagflag_value[cnt] > 1))
		*	display_corrupted(file, noline);
		*if ((cnt == DIAG_FL)
		*&& (diagflag_value[cnt] > MAXMESSMODE))
		*	display_corrupted(file, noline);
		*/

		if (diagflag_value[cnt] != old_diagflag_value[cnt]) {
			diagflag_index[diag_nbchg] = cnt;
			diag_nbchg++;
		}
	}

	/* read modem & site configuration from file and select the changes */
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* white line or "12 ..." */
		display_corrupted(file, noline);
	if ((line[0] == '1') && (line[1] == '2')) {
		noline++;
		if (fgets(line,MAXLINELEN,stream) == NULL) /* white line */
			display_corrupted(file, noline);
	}
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* first title line */
		display_corrupted(file, noline);
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* second title line */
		display_corrupted(file, noline);
	modemconf_nbchg = 0;
	buflen = 0;
	for (cnt=0; cnt<NMODEMCONF; cnt++) {
		noline++;
		if (fgets(line,MAXLINELEN,stream) == NULL)
			display_corrupted(file, noline);
		str = searchlastfield(line, cnt+1, file, noline);
		if (cnt > 0 && cnt <= MODEMCONFINT) {
			if (!isnumber(str))
				display_corrupted(file, noline);
		}
		stlen = strlen(str);
		if (stlen > (site_halflen[cnt] * 2))
			display_corrupted(file, noline);
		if (buflen + stlen >= BUFLEN)
			display_corrupted(file, noline);
		strcpy(&bufstring[buflen],str);
		modemconf_value[cnt] = &bufstring[buflen];
		buflen += (stlen + 1);
			
		if (strcmp(modemconf_value[cnt], old_modemconf_value[cnt])) {
			modemconf_index[modemconf_nbchg] = cnt;
			modemconf_nbchg++;
		}
	}

	/* read phone numbers from file and select the changes */
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* white line */
		display_corrupted(file, noline);
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* first title line */
		display_corrupted(file, noline);
	noline++;
	if (fgets(line,MAXLINELEN,stream) == NULL) /* second title line */
		display_corrupted(file, noline);
	phone_nbchg = 0;
	for (cnt=0; cnt<NPHONES; cnt++) {
		noline++;
		if (fgets(line,MAXLINELEN,stream) == NULL)
			display_corrupted(file, noline);
		str = searchlastfield(line, cnt+1, file, noline);
		stlen = strlen(str);
		if (stlen > PHONE_LEN)
			display_corrupted(file, noline);
		if (buflen + stlen >= BUFLEN)
			display_corrupted(file, noline);
		strcpy(&bufstring[buflen],str);
		phone_value[cnt] = &bufstring[buflen];
		buflen += (stlen + 1);
			
		if (strcmp(phone_value[cnt], old_phone_value[cnt])) {
			phone_index[phone_nbchg] = cnt;
			phone_nbchg++;
		}
	}
	(void)fclose(stream);
}

/*
 * NAME: display_corrupted
 *
 * FUNCTION: display the "corrupted data" message
 *
 * RETURNS : exits 1
 *
 */

void
display_corrupted(char *file, int noline)
{
	fprintf(stderr,
		MSGSTR(CORRUPT2, "0044-008: Corrupted data in %s (line %d)\n"),
		file, noline);
	exit(1);
}

/*
 * NAME: searchlastfield
 *
 * FUNCTION: search the last field of the line and verify the index number
 *
 * RETURNS : found string
 *
 */

char *
searchlastfield(char *line, int index, char *file, int noline)
{
	char *str;
	char *str2;
	char *stind;

	str2 = line;
	while (*str2 == ' ' || *str2 == '\t') {
		str2++; /* before index */
	}
	stind = str2;
	while (*str2 >= '0' && *str2 <= '9') {
		str2++;
	}
	if (*str2 != ' ' && *str2 != '\t')
		display_corrupted(file, noline);
	*str2 = '\0';
	if (index != atoi(stind))
		display_corrupted(file, noline);

	str2++;
	str = str2 + strlen(str2) - 1;
	if (*str == '\n') {
		*str = '\0'; /* suppress <RC> */
		str--;
	}
	/* last field is separate with a '\t' */
	while (*str != '\t' && *str != '\0') {
		str--;
	}
	str++; /* first char != '\t' & '\0' */
	return(str);
}

/*
 * NAME: isnumber
 *
 * FUNCTION: verify if the str string is a number
 *
 * RETURNS : 1 if true, 0 if false
 *
 */

int
isnumber(char *str)
{
	char *str2;

	str2 = str;
	if (*str2 == '+' || *str2 == '-')
	{
		str2++;
		if (!*str2)
			return(0);
	}
	if (strlen(str2) > 6) return(0);
	while (*str2) {
		if (*str2 < '0' || *str2 > '9')
			return(0);
		str2++;
	}
	return(1);
}

/*
 * NAME: ask_passwd
 *
 * FUNCTION: ask for password for contract flags
 *
 * RETURNS : returns or exits 1
 *
 */

void
ask_passwd(char *buf, int type)
{
	FILE *stream;
	char contract_password[CONTRACT_PASSLEN];
	char line[MAXLINELEN];
	char *password;
	char *pass;
	char *prompt;
	int len, lg;
	char *diagkey;
	
	len = DIAG_PASSLEN;
	password = diag_password;

	/* Call from Diag ? */
	diagkey = getenv("diagbumpkey");
	if ((diagkey != NULL) && !strcmp(diagkey,"jwkq%374!")) {
		switch (type) {
		case CONTRACT_FLAGS:
			len = CONTRACT_PASSLEN;
			pass = getenv("diagbumproll");
			password = contract_password;
			break;
		case DIAG_FLAGS:
			pass = getenv("diagbumpop");
			break;
		case PASSWORD:
			pass = getenv("diagbumpop");
			break;
		case PASSWORD+1:
			pass = getenv("diagbumpnp");
			break;
		case PASSWORD+2:
			pass = getenv("diagbumpnp");
			break;
		}
	}
	else {
		switch (type) {
		case CONTRACT_FLAGS:
			prompt = MSGSTR(NEED_ROLLPASS, "Rolling Password:");
			len = CONTRACT_PASSLEN;
			password = contract_password;
			break;
		case DIAG_FLAGS:
			if (diag_pass) {
				memcpy(buf, password, len);
				return;
			}
			diag_pass++; /* only one password reading */
			prompt = MSGSTR(NEED_DIAGPASS1,
				"Maintenance Menu Password:");
			break;
		case PASSWORD:
			if (password_number) {
			    fprintf(stdout, MSGSTR(NEED_DIAGPASS2,
				"Changing Maintenance Menu Password\n"));
			    prompt = MSGSTR(NEED_DIAGPASS3,
				"Customer Maintenance Menu Password:");
			}
			else {
			    fprintf(stdout, MSGSTR(NEED_DIAGPASS4,
			    "Changing Customer Maintenance Menu Password\n"));
			    prompt = MSGSTR(NEED_DIAGPASS5, "Old Password:");
			}
			break;
		case PASSWORD+1:
			prompt = MSGSTR(NEED_DIAGPASS6, "New password:");
			break;
		case PASSWORD+2:
			prompt = MSGSTR(NEED_DIAGPASS7,
				"Enter the new password again:");
		}

		pass = getpass(prompt);
	}
	if (pass == NULL) {
		fprintf(stderr, MSGSTR(IO, "0044-005: I/O error\n"));
		exit(1);
	}
	if (type == CONTRACT_FLAGS) {
		if (*pass == '#') {
			/* autorized only if # in front of rolling password */
			pass++;
		}
		else {
			fprintf(stderr, MSGSTR(PASSERR,
				"0044-012: You entered an invalid password\n"));
			exit(1);
		}
	}
	memcpy(password, pass, len);
	memcpy(buf, pass, len);
}

/*
 * NAME: handle_error
 *
 * FUNCTION: display error message and exits
 *
 * RETURNS : exits 1
 *
 */

void
handle_error(int nvram)
{
	(void)close(nvram);
	switch (errno) {
	case EINVAL :
		fprintf(stderr, MSGSTR(NOTSUP,
			"0044-004: Command not supported\n"));
		break;
	case EIO :
		fprintf(stderr, MSGSTR(IO, "0044-005: I/O error\n"));
		break;
	case EACCES :
		if (contract_autorization) {
			fprintf(stderr, MSGSTR(NOPERM,
			"0044-013: Remote Authorization useless:\nRemote Service Support not available\n"));
		}
		else {
			fprintf(stderr, MSGSTR(PASSERR,
				"0044-012: You entered an invalid password\n"));
		}
		break;
	default :
		fprintf(stderr, MSGSTR(FAIL, "0044-006: Command failed\n"));
	}
	exit(1);
}

/* 
 * NAME: init_contractflags
 *
 * FUNCTION: initialize the flag names
 *
 * RETURNS : None
 *
 */

void
init_contractflags(char **flag_name)
{
	/* initialize contract flags */
	flag_name[0] = MSGSTR(CONTRACTFLAG0, "Remote Service Support");
	flag_name[1] = MSGSTR(CONTRACTFLAG1, "Quick On Call Service");
	flag_name[2] = MSGSTR(CONTRACTFLAG2, "Service Contract Validity");
	flag_name[3] = MSGSTR(CONTRACTFLAG3, "Service Support Type");
}

/* 
 * NAME: init_diagflags
 *
 * FUNCTION: initialize the flag names
 *
 * RETURNS : None
 *
 */

void
init_diagflags(char **flag_name)
{
	/* initialize flags */
	flag_name[0] = MSGSTR(DIAGFLAG0, "Remote Authorization");
	flag_name[1] = MSGSTR(DIAGFLAG1, "Autoservice IPL");
	flag_name[2] = MSGSTR(DIAGFLAG2, "BUMP Console");
	flag_name[3] = MSGSTR(DIAGFLAG3, "Dial-Out Authorization");
	flag_name[4] = MSGSTR(DIAGFLAG4, "Set Mode to Normal When Booting");
	flag_name[5] = MSGSTR(DIAGFLAG5,
		"Electronic Mode Switch from Service Line");
	flag_name[6] = MSGSTR(DIAGFLAG6, "Boot Multi-user AIX in Service");
	flag_name[7] = MSGSTR(DIAGFLAG7, "Extended Tests");
	flag_name[8] = MSGSTR(DIAGFLAG8, "Power On Tests in Trace Mode");
	flag_name[9] = MSGSTR(DIAGFLAG9, "Power On Tests in Loop Mode");
	flag_name[10] = MSGSTR(DIAGFLAG10, "Fast IPL");
	/*flag_name[11] = MSGSTR(DIAGFLAG11, "Power On Test Error Message");*/
}

/* 
 * NAME: init_modemconf
 *
 * FUNCTION: initialize the modem configuration names
 *
 * RETURNS : None
 *
 */

void
init_modemconf(char **modemconf_name)
{
	modemconf_name[0] = MSGSTR(MODEMCONF0, "Modem Parameters File Name");
	modemconf_name[1] = MSGSTR(MODEMCONF1, "Service Line Speed");
	modemconf_name[2] = MSGSTR(MODEMCONF2,
			"Protocol Inter Data Block Delay");
	modemconf_name[3] = MSGSTR(MODEMCONF3, "Protocol Time Out");
	modemconf_name[4] = MSGSTR(MODEMCONF4, "Retry Number");
	modemconf_name[5] = MSGSTR(MODEMCONF5, "Customer ID");
	modemconf_name[6] = MSGSTR(MODEMCONF6, "Login ID");
	modemconf_name[7] = MSGSTR(MODEMCONF7, "Password ID");
}

/* 
 * NAME: init_phones
 *
 * FUNCTION: initialize the phone names
 *
 * RETURNS : 
 *
 */

void
init_phones(char **phone_name)
{
	static char ph[4][MAXPHONELEN];
	int i, l;

	/* initialize flags */
	phone_name[0] = MSGSTR(PHONE0, "Service Center Dial-Out");
	phone_name[1] = MSGSTR(PHONE0, "Service Center Dial-Out");
	phone_name[2] = MSGSTR(PHONE1, "Customer Hub Dial-Out");
	phone_name[3] = MSGSTR(PHONE1, "Customer Hub Dial-Out");
	phone_name[4] = MSGSTR(PHONE2, "System Dial-In");
	phone_name[5] = MSGSTR(PHONE3, "System Operator Voice");
	for (i = 0; i < 4; i++) {
		strcpy(ph[i],phone_name[i]);
		l = strlen(ph[i]);
		strcpy(&ph[i][l],(i % 2) ? " (2)" : " (1)");
		phone_name[i] = ph[i];
	}
}

/* 
 * NAME: parse_options
 *
 * FUNCTION: read the command line
 *
 * RETURNS : nothing.
 */

void
parse_options(int argc, char **argv)
{
	extern char *optarg;
	extern int   optind;
	char        *getstr = "dcrsfmpSw";
	int c, cnt;

	dflag = 0;
	cflag = 0;
	rflag = 0;
	sflag = 0;
	
	fflag = 0;
	mflag = 0;
	pflag = 0;
	Sflag = 0;
	wflag = 0;

  	while ((c = getopt(argc, argv, getstr))!= EOF)
  	{
  		switch (c)
  		{
		case 'd':
			if (dflag + cflag + rflag + sflag)
				errusage();
			else
				dflag = 1;
			break;
		case 'c':
			if (dflag + cflag + rflag + sflag)
				errusage();
			else
				cflag = 1;
			break;
		case 'r':
			if (dflag + cflag + rflag + sflag)
				errusage();
			else
				rflag = 1;
			break;
		case 's':
			if (dflag + cflag + rflag + sflag)
				errusage();
			else
				sflag = 1;
			break;
		case 'f':
			if (fflag)
				errusage();
			else
				fflag = 1;
			break;
		case 'm':
			if (mflag)
				errusage();
			else
				mflag = 1;
			break;
		case 'p':
			if (pflag)
				errusage();
			else
				pflag = 1;
			break;
		case 'S':
			if (Sflag)
				errusage();
			else
				Sflag = 1;
			break;
		case 'w':
			if (wflag)
				errusage();
			else
				wflag = 1;
			break;
  		case '?':
  			errusage();
  		} /* case */
  	} /* while */

	/* check the flags consistency */
	if ((dflag + cflag + rflag + sflag) != 1)
		errusage();

	if (dflag) 
		if ((fflag + mflag + pflag + Sflag) == 0)
			errusage();
	if (dflag) 
		if (wflag)
			errusage();

	if (cflag)
		if ((fflag + mflag + pflag + Sflag + wflag) != 1)
			errusage();

	if (rflag || sflag)
		if ((fflag + mflag + pflag + Sflag + wflag) != 0)
			errusage();

	argc -= optind;
	argv += optind;

	/* check the arguments */
	if ((dflag + rflag + sflag) && (argc != 0))
		errusage();

	if (cflag)
		if ((argc <= 0) || (argc > 2 * MAXARG))
			errusage();

	if (cflag)
		if (wflag) {
			if (argc != 1)
				errusage();
		}
		else if ((argc % 2) != 0)
			errusage();

	if (cflag){
		int i, index;

		cnt = 0;
		while (argc > 0) {
			if (!isnumber(argv[2*cnt])) errusage();
			index = atoi(argv[2*cnt]);
			if (index < 1) errusage();
			index--;
			if (Sflag) {
				if (index >= NCONTRACTFLAGS) errusage();
				for (i = 0; i < cnt ; i++)
				    if ( contractflag_index[i] == index)
					errusage();
				contractflag_index[cnt] = index;
				if (index == CONTRACT_TYPE) {
				    if (strlen(argv[2*cnt+1]) == 1)
					contractflag_value[index] =
						*argv[2*cnt+1];
				    else
					errusage();
				}
				else if (isnumber(argv[2*cnt+1])) {
				    contractflag_value[index] =
					atoi(argv[2*cnt+1]);
				    if (index <= CONTRACT_FL) {
					if (contractflag_value[index] < 0
					|| contractflag_value[index] > 1)
						errusage();
				    }
				    else {
					if (contractflag_value[index] < -1
					|| contractflag_value[index] > 32767)
						errusage();
				    }
				}
				else
					errusage();
			}
			else if (fflag) {
				if (index >= NDIAGFLAGS) errusage();
				for (i = 0; i < cnt ; i++)
				    if ( diagflag_index[i] == index)
					errusage();
				diagflag_index[cnt] = index;
				if (isnumber(argv[2*cnt+1])) {
				    int itmp;

				    itmp = atoi(argv[2*cnt+1]);
				    if ((itmp < 0) || (itmp > 1))
					errusage();
				    /*if (diagflag_value[index] < 0)
				    *   errusage();
				    *if ((index < DIAG_FL)
				    *&& (diagflag_value[index] > 1))
				    *   errusage();
				    *if ((index == DIAG_FL)
				    *&& (diagflag_value[index] > MAXMESSMODE))
				    *   errusage();
				    */
				    diagflag_value[index] = itmp;
				}
				else
					errusage();
			}
			else if (mflag) {
				char *str;

				if (index >= NMODEMCONF) errusage();
				for (i = 0; i < cnt ; i++)
				    if ( modemconf_index[i] == index)
					errusage();
				modemconf_index[cnt] = index;
				if (index > 0 && index <= MODEMCONFINT) {
				    if (!isnumber(argv[2*cnt+1]))
					errusage();
				}
				modemconf_value[index] = argv[2*cnt+1];
				if (strlen(modemconf_value[index]) >
				   (site_halflen[index] * 2))
					errusage();
				str = modemconf_value[index];
				/* supp tab in str */
				while (*str) {
					if (*str == '\t') *str = ' ';
					str++;
				}
			}
			else if (pflag) {
				if (index >= NPHONES) errusage();
				for (i = 0; i < cnt ; i++)
				    if ( phone_index[i] == index)
					errusage();
				phone_index[cnt] = index;
				phone_value[index] = argv[2*cnt+1];
				if (strlen(phone_value[index]) > PHONE_LEN)
					errusage();
			}
			else if (wflag) {
				if (index != 0 && index != 1)
					errusage();
				password_number = index;
			}
			cnt++;
			argc -= 2;
		}
		contract_nbchg = 0;
		diag_nbchg = 0;
		modemconf_nbchg = 0;
		phone_nbchg = 0;
		if (Sflag) contract_nbchg = cnt;
		else if (fflag) diag_nbchg = cnt;
		else if (mflag) modemconf_nbchg = cnt;
		else if (pflag) phone_nbchg = cnt;
	}
} /* parse_options */

/* 
 * NAME: errusage
 *
 * FUNCTION: print usage error and exit
 *
 * RETURNS : exit 1
 */

void
errusage()
{
	char *usage =
"Usage: mpcfg { -c {-f|-m|-p|-S} <index1> <value1> ...\n\
              | -c -w {1|2} | -d {-f -m -p -S} | -r | -s }\n";

	fprintf(stderr, MSGSTR(USAGE2, usage));
	exit(1);
}
