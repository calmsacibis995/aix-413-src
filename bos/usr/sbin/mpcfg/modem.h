/*
 * COMPONENT_NAME: DSAUBUMP
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <stdio.h>

#define MAX_SIZE_NAME 50	/* max identifier length */
#define MAX_SIZE_STRING 256	/* max characters between double quote */

/* values for timeout */
#define NONE_VALUE	-1
#define N_VALUE		-2
#define DEFAULT_VALUE	-3

/* types of commandes */
#define CMD_SEND 1
#define CMD_EXPECT 2
#define CMD_IGNORE 3
#define CMD_DELAY 4

/* list of strings structure */
typedef struct st_strlst_t {
	char *str;
	int cur_char;	/* use to scan the result from the modem */
	struct st_strlst_t *next;
	} strlst_t;

typedef struct st_command_t {
	int type;	/* type of command : CMD_SEND, CMD_EXPECT ... */
	int val;	/* value for delay or timeout */
	char *str;	/* string for send or for busy */
	strlst_t *strlst;	/* string list for expect or ignore */
	int abs;	/* boolean value for expect */
	struct st_command_t *next;
	} command_t;

typedef struct st_script_t {
	char *name;
	command_t *cmd;
	struct st_script_t *next;
	} script_t;

extern FILE *modem_file;	/* must be open before yyparse() */

extern script_t *first_script;	/* contain the firrst script after yyparse() */
extern int icdelay;	/* inter-commands delay */
extern int defaultto;	/* default time-out */
extern int calldelay;	/* call delay */
extern int line_number;	/* current line number in the modem file */

extern int yyparse ();

