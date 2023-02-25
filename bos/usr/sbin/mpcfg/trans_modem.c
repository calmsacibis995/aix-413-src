static char sccsid[] = "@(#)89  1.3  src/bos/usr/sbin/mpcfg/trans_modem.c, dsaubump, bos411, 9436C411a 9/6/94 08:40:44";
/*
 * COMPONENT_NAME: DSAUBUMP
 *
 * FUNCTIONS: yyerror, errlimit, trans_commands, trans_scripts,
 *            fill_buffer, read_modemparam 
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include "modem.h"

#include "mpcfg_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_MPCFG,Num,Str)
nl_catd catd;

#define NBSCRIPTS 7
static	char *mnemo_script[NBSCRIPTS]={"CO","CX","RT","DX","CI","CW","WC"};
static	char *str_script[NBSCRIPTS]={
		"condout",
		"connect",
		"retry",
		"disconnect",
		"condin",
		"condwait",
		"waitcall"};
static  char *modem_file_name;

/*
 * NAME: yyerror
 *
 * FUNCTION: display the message of yacc
 *
 * RETURNS : NONE
 *
 */

void
yyerror(char *errmess)
{
	fprintf(stderr,
		MSGSTR(YYERR, "%s in %s (line %d)\n"),
		errmess, modem_file_name, line_number);
}

/*
 * NAME: errlimit
 *
 * FUNCTION: display the message of limit
 *
 * RETURNS : exit 1
 *
 */

void
errlimit()
{
	fprintf(stderr,
	MSGSTR(LIMIT, "0044-010: Modem Configuration buffer too large\n"));
	exit(1);
}

/*
 * NAME: trans_commands
 *
 * FUNCTION: translate the commands of a script in buf
 *
 * RETURNS : pointer on buffer end
 *
 */

static char *
trans_commands (command_t *cmd, char *buf, char *limit)
{
	strlst_t *strlst;

	while (cmd != NULL) {
		switch (cmd->type) {
		case CMD_SEND :
			*buf++ = 's';
			strcpy(buf, cmd->str);
			buf += strlen(cmd->str) + 1; /* end with null char */
			break;
		case CMD_EXPECT :
		case CMD_IGNORE :
			if (cmd->type == CMD_IGNORE)
				*buf++ = 'i'; /* ignore */
			else if (cmd->abs)
				*buf++ = 'a'; /* expect abs */
			else
				*buf++ = 'e'; /* expect */
			*buf++ = '(';
			strlst = cmd->strlst;
			while (strlst != NULL) {
				strcpy(buf, strlst->str);
				buf += strlen(strlst->str) + 1;
				strlst = strlst->next;
				if (strlst != NULL) *buf++ = 'o';
				if (buf >= limit) errlimit;
			}
			if (cmd->str != NULL) {
				*buf++ = 'b'; /* busy */
				strcpy(buf, cmd->str);
				buf += strlen(cmd->str) + 1;
			}
			if (cmd->val != DEFAULT_VALUE) {
				*buf++ = 't'; /* timeout */
				if (cmd->val == NONE_VALUE)
					strcpy(buf, "I");
				else if (cmd->val == N_VALUE)
					strcpy(buf, "N");
				else
					sprintf(buf, "%d", cmd->val);
				buf += strlen(buf) + 1;
			}
			*buf++ = ')';
			break;
		case CMD_DELAY :
			*buf++ = 'd';
			if (cmd->val == N_VALUE)
				strcpy(buf, "N");
			else
				sprintf(buf, "%d", cmd->val);
			buf += strlen(buf) + 1;
			break;
		default : ;
		}
		if (buf >= limit) errlimit;
		cmd = cmd->next;
	}
	return(buf);
}  /* trans_commands */

/*
 * NAME: trans_scripts
 *
 * FUNCTION: translate the scripts of the modem parameter file in buf
 *
 * RETURNS : NONE
 *
 */

static void
trans_scripts(char *buf, char *limit)
{
	script_t *sc;
	int	 nosc, delta, error;
	char 	 *first, *size;

	sc = first_script;
	error = 0;
	while (sc != NULL) {
		for (nosc = 0; nosc < NBSCRIPTS; nosc++) {
			if (!strcmp(sc->name, str_script[nosc])) {
				first = buf;
				*buf++ = '*';
				*buf++ = mnemo_script[nosc][0];
				*buf++ = mnemo_script[nosc][1];
				size = buf;
				buf++;
				buf = trans_commands(sc->cmd, buf, limit);
				delta = buf - first;
				if (delta % 2 == 1)
					*buf++ = '\0';
				delta = buf - first;
				*size = delta / 2;
				break;
			}
		}
		/* to accept future evolution of the script
		*if (nosc >= NBSCRIPTS) {
		*	fprintf(stderr, MSGSTR(SCRIPT,
		*		"0044-009: Unknown script in %s:%s\n"),
		*		modem_file_name,  sc->name);
		*	error = 1;
		*}
		*/
		sc = sc->next;
	}
	if (buf >= limit) errlimit;
	if (error) exit(1);
	*buf = '\0';
}  /* trans_scripts */

/*
 * NAME: fill_buffer
 *
 * FUNCTION: translate the variables and call trans_scripts
 *
 * RETURNS : NONE
 *
 */

static void
fill_buffer(char *buf, char *limit)
{
	int lg;

	sprintf(buf, "*IC.%d", icdelay);
	lg = strlen(buf);
	if (lg % 2)
		lg++;
	buf[3] = lg / 2;
	buf += lg;
	sprintf(buf, "*DT.%d", defaultto);
	lg = strlen(buf);
	if (lg % 2)
		lg++;
	buf[3] = lg / 2;
	buf += lg;
	sprintf(buf, "*CD.%d", calldelay);
	lg = strlen(buf);
	if (lg % 2)
		lg++;
	buf[3] = lg / 2;
	buf += lg;
	trans_scripts(buf,limit);
}

/*
 * NAME: read_modemparam
 *
 * FUNCTION: read modem parameters and parse into buffer
 *
 * RETURNS : returns or exits 1
 *
 */

void
read_modemparam(char *file, char *buffer)
{
	/* open file */
	modem_file = fopen(file, "r");
	if (modem_file == 0) {
		fprintf(stderr,
			MSGSTR(OPEN, "0044-002: Cannot open %s\n"), file);
		exit(1);
	}
	modem_file_name = file;

	/* syntax, lexical and semantical analysis */
	if (yyparse () != 0) {
                exit (1);
        }

	/* fill buffer */
	fill_buffer(buffer,buffer+492);

	/* close file */
	fclose (modem_file);
}
