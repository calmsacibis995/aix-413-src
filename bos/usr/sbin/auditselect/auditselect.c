static char sccsid[] = "@(#)21  1.24  src/bos/usr/sbin/auditselect/auditselect.c, cmdsaud, bos411, 9428A410j 11/12/93 14:40:09";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditselect command
 *
 * ORIGINS: 27 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include	"stdio.h"
#include	"fcntl.h"
#include	"varargs.h"
#include	"ctype.h"
#include	"pwd.h"
#include	"sys/stat.h"
#include	"sys/audit.h"
#include	"sys/errno.h"
#include	"audutil.h"
#include	"sys/priv.h"
#include	"time.h"
#include 	"locale.h"

DEBUGNAME("AUDITSELECT")

#include "auditslt_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITSLT,n,s)

#define	AND	(0x0000 | 1)
#define	OR	(0x0000 | 2)
#define	NOT	(0x0000 | 3)
#define	RELOP	0x0100
#define	EQ	(RELOP | 1)
#define	NE	(RELOP | 2)
#define	GT	(RELOP | 3)
#define	GE	(RELOP | 4)
#define	LT	(RELOP | 5)
#define	LE	(RELOP | 6)
#define	unary	left
#define	field	left
#define	value	right
#define	EVENT	1
#define	RESULT	2
#define	COMMAND	3
#define	LOGIN	4
#define	REAL	5
#define	PID	6
#define	PPID	7
#define	DATE	8
#define	TIME	9
#define TID     10

char	*ProgName;
char	*ExprFile;
char	*FileName;
char	*expr_input;
char	*expr_inp;

struct	expr {
	int	op;
	struct	expr	*left;
	struct	expr	*right;
};

/* functions */
struct	expr *
new_node(){
	struct	expr	*n;
	extern	char	*xalloc ();
	static	struct	expr	zexpr;

	n = (struct expr *) xalloc (sizeof (struct expr));
	*n = zexpr;
	return (n);
}


/*
 * this is a "predictive" scanner:
 * the parser indicates the type of token which may be accepted next
 * and the scanner verifies that such a token is present.
 *
 * all input is scanned from an in-memory buffer.
 *
 * all scanning routines return a boolean indicating success:
 *	0 - failure
 *	1 - success (token found)
*/
error (va_alist)
va_dcl {

	va_list	ap;
	char	*p;
	char	*nl;

	va_start (ap);
	fflush (stdout);

	/* print input up to and including the error */
	nl = NULL;
	for(p = expr_input; *p && (p <= expr_inp); p++){
		if(*p == '\n')nl = p;
		fputc (*p, stderr);
	}
	/* print rest of line containing the error */
	if(p != nl){
		while((*p) && (*p != '\n')){
			fputc (*p, stderr);
			p++;
		}
		fputc ('\n', stderr);
	}

	/* print pointer to position of error */
	for(p = nl ? nl+1 : expr_input; p < expr_inp; p++)fputc ('-', stderr);
	fputc ('^', stderr);
	fputc ('\n', stderr);

	/* print error message */
	p = va_arg (ap, char *);
	vfprintf (stderr, p, ap);
	fprintf (stderr, "\n");
	exit (EINVAL);
}


/* skip white space in input */
skipwh(){

	for( ; 1; expr_inp++){

		char	c;

		c = *expr_inp;
		if (c == ' ')
			continue;
		if (c == '\t')
			continue;
		if (c == '\n')
			continue;
		break;
	}
}


/* check for an arbitrary token (<str>) in the input stream.
 * treats upper and lower case as equivalent in string tokens
 * (i.e., field names) */
scan(char *str){

	char	*p;

	skipwh ();
	for(p = expr_inp; 1; p++){
		register char	c;

		c = *str++;
		if(c == '\0'){
			expr_inp = p;
			return (1);
		}
		if(isalpha(c)){
			if(toupper(c) != toupper(*p)){
				return (0);
			}
		}
		else if(c != *p){
			return (0);
		}
	}
}


/*
 * scan a blank- or quote-delimited string from the input.
 * assumes leading white space has already been stripped off.
 * sets <vp> to point to a malloc'd copy of the string
 */

char *
stralloc(char *s, int len){

	char	*p;
	extern	char	*xalloc ();

	p = xalloc (len + 1);
	strncpy(p, s, len);
	p[len] = '\0';
	return(p);
}

scan_str(char **vp){
	char	*p;

	p = expr_inp;

	/* quoted string ? */
	if((*p == '\'') || (*p == '"')){
		char	quote;

		quote = *p++;
		while(1){
			char	c;

			c = *p++;
			if(c == '\0'){

				error ("%s%c",
				MSGSTR(MISSCLOSE, 
				"** missing closing "), 
				quote);
			}

			if(c == quote){
				break;
			}
		}
		*vp = stralloc(expr_inp + 1, (p-2) - (expr_inp+1) + 1);
		expr_inp = p;
		return (1);
	}

	/* not quoted.
	 * the "string" may contain anything not "special" to us */
	for( ; 1; p++){

		switch(*p){
			/* eoln */
			case '\0':
			/* white space */
			case '\t':
			case '\n':
			case ' ':
			/* special characters */
			case '(':
			case ')':
			case '=':
			case '<':
			case '>':
			case '!':
			case '|':
			case '&':
				break;
			default:
				continue;
		}
		break;
	}
	if(p == expr_inp)return(0);

	*vp = stralloc (expr_inp, p - expr_inp);
	expr_inp = p;
	return (1);

}

/* scan a number
 * sets <ip> to the resulting numeric value */
scan_num(int *ip){

	int	n;

	if (!isdigit (*expr_inp))
		return (0);
	n = 0;
	while(1){
		if (!isdigit (*expr_inp))
			break;
		n *= 10;
		n += *expr_inp++ - '0';
	}
	*ip = n;
	return (1);
}

scan_date(int *vp){

	struct	tm	tm; 
	char	*str;
	/* 
	 * Date must be a "string" 
	 */

	if(scan_str(&str) == 0)return (0);

	memset (&tm, 0, sizeof (tm));
	strptime(str, " %D ", &tm);
	if(tm.tm_year == 0){
		struct tm *current;
		char *test;
		long tbuf;

		time(&tbuf);
		current = localtime(&tbuf);
		tm.tm_year = current->tm_year;

	}

	*vp = mktime(&tm);
	return (1);
}

scan_time(int *vp){

	struct	tm	tm;
	char	*str;

	if(scan_str (&str) == 0) return (0);

	memset (&tm, 0, sizeof (tm));
	strptime(str, " %T ", &tm);
	*vp = (((tm.tm_hour * 60) + tm.tm_min) * 60) + tm.tm_sec;
	return (1);
}


/*
 * scan a relational operator
 * sets <rp> to the resulting operator value 
 */

scan_relop(int *rp){

	skipwh ();
	switch(*expr_inp){
		case '=':
			if(expr_inp[1] == '='){
				expr_inp += 2;
				*rp = EQ;
				return (1);
			}
			return (0);

		case '>':
			expr_inp++;
			if(*expr_inp == '='){
				expr_inp++;
				*rp = GE;
				return (1);
			}
			*rp = GT;
			return (1);

		case '<':
			expr_inp++;
			if(*expr_inp == '='){
				expr_inp++;
				*rp = LE;
				return (1);
			}
			*rp = LT;
			return (1);

		case '!':
			if(expr_inp[1] == '='){
				expr_inp += 2;
				*rp = NE;
				return (1);
			}
			return (0);
		
		default:
			return (0);
	}
}


/* scan a field name from the input stream.
 * sets <fp> to the resulting field type.  */
scan_field(int *fp){

	skipwh ();
	if(scan ("event")){
		*fp = EVENT;
		return (1);
	}
	if(scan ("result")){
		*fp = RESULT;
		return (1);
	}
	if(scan ("command")){
		*fp = COMMAND;
		return (1);
	}
	if(scan ("login")){
		*fp = LOGIN;
		return (1);
	}
	if(scan ("real")){
		*fp = REAL;
		return (1);
	}
	if(scan ("pid")){
		*fp = PID;
		return (1);
	}
	if(scan ("ppid")){
		*fp = PPID;
		return (1);
	}
	if(scan ("date")){
		*fp = DATE;
		return (1);
	}
	if(scan ("time")){
		*fp = TIME;
		return (1);
	}
	if(scan ("tid")){
		*fp = TID;
		return (1);
	}
	return (0);
}


/* scan a value for the field of the given type (<fp>).
 * sets <vp> to the resulting field value.  */
scan_value(int *vp, int fp){

	skipwh ();
	switch(fp){
		case EVENT:
		case COMMAND:
			return (scan_str (vp));
		case LOGIN:
		case REAL:
			if(isalpha(*expr_inp)){
				int	*p;
				struct	passwd	*pw;
				extern	struct	passwd	*getpwnam();

				if(!scan_str (&p))return (0);

				if((pw = getpwnam ((char *) p)) == NULL){

					error ("%s\"%s\"",
					MSGSTR(BADUSER, 
					"** unknown user name "), 
					p);

				}
				*vp = pw -> pw_uid;
				free (p);
				return (1);
			}
		case RESULT:

			if(isalpha(*expr_inp)){

				if(scan_str(vp)){

					return(1);

				}
				else {
				
					return(0);
				}

			}

		case PID:
		case PPID:
		case TID:
			return (scan_num (vp));
		case DATE:
			return (scan_date(vp));
		case TIME:
			return (scan_time (vp));
		default:

			error ("%s%d",
			MSGSTR(INTERNAL, 
			"** internal error: scan_value(), field type = "), 
			fp);

	}
}


/* this is a recursive-descent parser,
 * based on the usual expression grammar:
 *
 *	E -> T T*
 *	T* -> +T T* | empty
 *	T -> F F*
 *	F* -> * F F* | empty
 *	F -> (E) | !F | P
 *	P -> field relop value */
struct expr *
expr(){

	struct expr *E();
	struct expr *e;

	e = E();
	if(*expr_inp != '\0'){

		error(
		MSGSTR(MISSOPER, 
		"** missing operator")
		);
	}

	return (e);
}

/* 	E  -> T T*
 *	T* -> OR T T* | (empty) */
struct expr *
E(){

	struct	expr	*e;
	struct	expr	*n;
	struct	expr	*T();

	e = T();
	while(1){

		if(!scan ("||"))return (e);

		n = new_node();
		n -> left = e;
		n -> op = OR;
		n -> right = T();
		e = n;
	}
}

/* 	T  -> F F*
 *	F* -> AND F F* | (empty) */
struct expr *
T(){

	struct	expr	*t;
	struct	expr	*n;
	struct	expr	*F();

	t = F();
	while(1){
		if (!scan ("&&"))
			return (t);
		n = new_node();
		n -> left = t;
		n -> op = AND;
		n -> right = F();
		t = n;
	}
}

/*	F -> (E) | !F | "field relop value" */
struct expr *
F(){

	struct	expr	*f;
	if(scan ("(")){
		f = E ();
		if(!scan (")")){

			error(
			MSGSTR(MISSRPAREN, 
			"** missing right parenthesis")
			);

		}
		return (f);
	}

	/* otherwise, if successful, are creating new node */
	f = new_node ();
		
	if(scan ("!")){
		f -> op = NOT;
		f -> unary = F ();
		return (f);
	}
		
	/* field name */
	if(!scan_field (&(f -> field))){
		char	*savep;

		/* hack for intelligible diagnostic on bad field name */
		savep = expr_inp;
		if(scan_str (&(f -> field))){
			expr_inp = savep;

			error(
			MSGSTR(BADFIELD, 
			"** unknown field name")
			);

		}

		error(
		MSGSTR(MISSTERM, 
		"** missing term after logical operator")
		);

	}

	/* relop */
	if(!scan_relop (&(f -> op))){

		error(
		MSGSTR(MISSRELAT, 
		"** missing relational operator after field name")
		);
	}

	/* error checking of allowed comparisons */
	if((int) (f -> field) == EVENT){
		if((f -> op != EQ) && (f -> op != NE)){

			error(
			MSGSTR(BADORDER, 
			"** events are not ordered")
			);
		}
	}
	else if ((int) (f -> field) == RESULT){
		if((f -> op != EQ) && (f -> op != NE)){
			error(
			MSGSTR(BADRESULTS, 
			"** results are not ordered")
			);
		}
	}
	else if((int) (f -> field) == COMMAND){
		if((f -> op != EQ) && (f -> op != NE)){
			error(
			MSGSTR(BADCMDS, 
			"** commands are not ordered")
			);
		}
	}

	/* value */
	if(!scan_value(&(f->value), f->field)){
		error(
		MSGSTR(MISSVALUE, 
		"** missing value after relational operator")
		);
	}

	return (f);
}


/*
 * prune the tree to include only time-related terms.  
 * this pruned tree allows quick examination of bin headers.
 *
 * as implemented, this routine makes it next to impossible to
 * deallocate trees;
 * we now have multiple expression nodes pointing to the same string
 * buffers (so they can't be deallocated with the tree).
 */
struct	expr *
prune(struct expr *e){

	struct expr	*left;
	struct expr	*right;
	struct	expr	*n;

	/* the only primary we care about in the bin header
	 * is the DATE field */
	if(e -> op & RELOP){
		if ((int) (e -> field) != DATE)
			return (NULL);
		n = new_node ();
		*n = *e;
		return (n);
	}

	/* trim any subtrees:
	 * any subtree not involving DATE comparisons
	 * is assumed to be satisified (or satisfiable) */
	if(e -> op == NOT){
		left = right = NULL;
		if(unary = prune (e -> unary)){
			n = new_node ();
			n -> unary = unary;
			n -> op = NOT;
			return (n);
		}
		return (NULL);
	}
	left = prune (e -> left);
	right = prune (e -> right);
	if(left == NULL){
		if(right == NULL)return (NULL);
		return (right);
	}
	if(right == NULL)return (left);

	n = new_node ();
	n -> left = left;
	n -> right = right;
	n -> op = e -> op;
	return (n);
}


/* evaluate an expression against an audit record*/
eval(struct expr *e, struct aud_rec *a, char *tail){

	unsigned long	val;

	if(!(e -> op & RELOP)){
		/* not a primary */
		switch(e -> op){
			case AND:
				if (!eval (e -> left, a, tail))
					return (0);
				return (eval (e -> right, a, tail));
			case OR:
				if (eval (e -> left, a, tail))
					return (1);
				return (eval (e -> right, a, tail));
			case NOT:
				return (!eval (e -> unary, a, tail));
		}
	}

	switch((int)(e -> field)){
		case EVENT:
			if(e -> op == EQ){
				/* return "success" if ANY event name matches */
				if (strcmp (e -> value, a -> ah_event) == 0)
					return (1);
				return (0);
			}
			else {

				/* 
				 * Return "success" only if 
				 * NO event name matches 
				 */

				if (strcmp (e -> value, a -> ah_event) == 0)
					return (0);
				return (1);
			}

		case RESULT:
			switch(a->ah_result){

				case 0x0:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"OK") == 0);
					}
					else {

						return(strcmp(e->value, 
						"OK") != 0);
					}

				case 0x03:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"FAIL_AUTH") == 0);
					}
					else {

						return(strcmp(e->value, 
						"FAIL_AUTH") != 0);
					}

				case 0x05:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"FAIL_PRIV") == 0);
					}
					else {

						return(strcmp(e->value, 
						"FAIL_PRIV") != 0);
					}

				case 0x09:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"FAIL_ACCESS") == 0);
					}
					else {

						return(strcmp(e->value, 
						"FAIL_ACCESS") != 0);
					}

				case 0x19:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"FAIL_DAC") == 0);
					}
					else {

						return(strcmp(e->value, 
						"FAIL_DAC") != 0);
					}

				default:

					if(e->op == EQ){

						return(strcmp(e->value, 
						"FAIL") == 0);
					}
					else {

						return(strcmp(e->value, 
						"FAIL") != 0);
					}
			}
			break;

		case COMMAND:
			if (e -> op == EQ)
				return (strcmp (e -> value, a -> ah_name) == 0);
			else
				return (strcmp (e -> value, a -> ah_name) != 0);
		case LOGIN:
			val = a -> ah_luid;
			break;
		case REAL:
			val = a -> ah_ruid;
			break;
		case PID:
			val = a -> ah_pid;
			break;
		case PPID:
			val = a -> ah_ppid;
			break;
		case  TID:
			val = a -> ah_tid;
			break;
		case DATE:
		{
			struct	tm	*tm;

			/*
			 * figure out date of this record wrt
			 * the local time zone and dst rules.
			 * can't give tm_isdst to mktime because
			 * it wasn't known when the date was
			 * converted initially, and leaving it
			 * non-zero will give a different time
			 * back for midnight.
			 */

			tm = localtime(&a->ah_time);
			tm->tm_hour = tm->tm_min = tm->tm_sec = 0;
			tm->tm_isdst = 0;
			val = mktime(tm);
			break;
		}
		case TIME:
		{
			struct	tm	*tm;

			/*
			 * figure out time of this record wrt
			 * the local time zone and dst rules
			 */

			tm = localtime (&a->ah_time);
			val = (tm->tm_hour * 60 + tm->tm_min) * 60 + tm->tm_sec;
			break;
		}
		default:
			error("%s%d\n",
			MSGSTR(INTERNAL2, 
			"** internal error: eval_primary(), field type "), 
			e->field);
	}
	switch(e->op) {
		case EQ:
			return (val == (unsigned) (e -> value));
		case NE:
			return (val != (unsigned) (e -> value));
		case LT:
			return (val < (unsigned) (e -> value));
		case LE:
			return (val <= (unsigned) (e -> value));
		case GT:
			return (val > (unsigned) (e -> value));
		case GE:
			return (val >= (unsigned) (e -> value));
		default:
			error ("%s%d\n",
			MSGSTR(INTERNAL3, 
			"** internal error: eval_primary(), relop type "), 
			e->op);
	}
}

audwrite(FILE *fp, struct aud_rec *ah, char *buf){

	/* write the (aligned) header */
	fwrite ((char *) ah, sizeof (struct aud_rec), 1, fp);

	/* write the tail */
	if(ah -> ah_length)fwrite (buf, ah -> ah_length, 1, fp);

	fflush (fp);
}


Usage(){

	errno = EINVAL;

	exit1(errno, 
	MSGSTR(USAGE, 
	"Usage: auditselect { -e <expr> | -f <file> } <file> ...")
	);

}

main(int ac, char *av[]){

	int	c;
	extern	int	optind;
	extern	char	*optarg;
	
	struct	expr	*e;
	struct	aud_rec ah;
	char	*tail;
	FILE	*output;
	extern	char	*auditread();

	ProgName = av[0];

	setlocale(LC_ALL, "");
	catd = catopen(MF_AUDITSLT, NL_CAT_LOCALE);
	
	if(privcheck(SET_PROC_AUDIT)){ 
		errno = EPERM;

		exit1(0, 
		MSGSTR(ERRPRIV, 
		"** SET_PROC_AUDIT privilege required"), 
		0); 

	}

	if(ac == 1){
		Usage();
	}

        /*
         * Suspend auditing
         */

        if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0){
                errno = EPERM;

                exit1(errno,
                MSGSTR(ERRSUSPEND,
                "** cannot suspend auditing "),
                0);

        }

	while(1){
		c = getopt (ac, av, "e:f:");
		if (c == EOF)
			break;
		switch(c){
			case 'e':
				if (ExprFile)
					/* cannot specify both -e and -f */
					Usage ();
				expr_input = optarg;
				break;
			case 'f':
				if (expr_input)
					/* cannot specify both -e and -f */
					Usage ();
				ExprFile = optarg;
				break;
			default:
				Usage ();
		};
	};

	if(ExprFile){
		struct	stat	stbuf;
		int	fd;

		fd = open (ExprFile, O_RDONLY);
		if (fd == -1)
			exit1 (1, ExprFile, 0);
		if (fstat (fd, &stbuf))
			exit1 (1, ExprFile, 0);
		if (stbuf.st_size <= 0)
			exit1 (1, ExprFile, 0);
		expr_input = xalloc (stbuf.st_size+1);
		if(read (fd, (char *) expr_input, stbuf.st_size) != stbuf.st_size){
			errno = EINVAL;
			exit1 (1, ExprFile, 0);
		}
		close (fd);

		expr_input[stbuf.st_size] = '\0';  /* null-terminated string */

	}


	if(expr_input){
		expr_inp = expr_input;
		e = expr ();
	}
	/* otherwise, e is NULL => usage error; add code for this case.*/

	output = stdout;
	if(optind >= ac){
		/* no trail file supplied => use stdin */
		FileName = "stdin";
		while(1){
			tail = auditread (stdin, &ah);
			if (tail == NULL)
				break;
			if ((e == NULL) || eval (e, &ah, tail))
				audwrite (output, &ah, tail);
		};
	}
	else {
		/* one or more trail files supplied */
		for( ; optind < ac; optind++){
			FILE	*input;

			FileName = av[optind];
			input = fopen (FileName, "r");
			if(input == NULL){
				errno = EINVAL;
				exit1 (1, FileName, 0);
			}
			while(1){
				tail = auditread (input, &ah);
				if(tail == NULL){ 
					if(!errno)break;
					else exit1(1, FileName, 0);
				}
				if((e == NULL) || eval (e, &ah, tail)){
					audwrite (output, &ah, tail);
				}
			}
		}
	}
}


#ifdef	DEBUG
/* debugging only -
 * print an expression tree */

#define	INDENT	3

indent(int i){
	while (i-- > 0)
		fputc (' ', stdout);
}

char *
relop(int op){

	static	char	*rels[] = { 0, "EQ", "NE", "GT", "GE", "LT", "LE" };
	static	int	nrel = sizeof (rels) / sizeof (rels[0]);

	op &= ~RELOP;
	if ((op <= 0) || (nrel <= op))
		error ("** internal error: relop(), op = %d\n", op);
	return (rels[op]);
}

pr_op(int i, int op){

	static	char	*ops[] = { 0, "&&", "||", "!" };
	static	int	nop = sizeof (ops) / sizeof (ops[0]);

	if ((op <= 0) || (nop <= op))
		error ("** internal error: pr_op(), op = %d\n", op);

	indent (i);
	fprintf (stdout, "%s\n", ops[op]);
}

print(int i, struct expr *n){

	/* a pruned tree may be NULL */
	if(n == NULL){
		indent (i);
		fprintf (stdout, "NULL\n");
		return;
	}

	if(n -> op & RELOP){
		switch ((int)(n -> field)){
			case EVENT:
				indent (i+INDENT);
				fprintf (stdout, "EVENT %s %s\n", 
				relop (n -> op), n -> right);
				return;
			case RESULT:
				indent (i+INDENT);
				fprintf (stdout, "RESULT %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case COMMAND:
				indent (i+INDENT);
				fprintf (stdout, "COMMAND %s %s\n", 
				relop (n -> op), n -> right);

				return;
			case LOGIN:
				indent (i+INDENT);
				fprintf (stdout, "LOGIN %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case REAL:
				indent (i+INDENT);
				fprintf (stdout, "REAL %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case PID:
				indent (i+INDENT);
				fprintf (stdout, "PID %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case PPID:
				indent (i+INDENT);
				fprintf (stdout, "PPID %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case  TID:
				indent (i+INDENT);
				fprintf (stdout, "TID %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case DATE:
				indent (i+INDENT);
				fprintf (stdout, "DATE %s %d\n", 
				relop (n -> op), n -> right);

				return;
			case TIME:
				indent (i+INDENT);
				fprintf (stdout, "TIME %s %d\n", 
				relop (n -> op), n -> right);

				return;
			default:
				error ("internal error: print(), field = %d\n", 
				n -> field);
		}
	}

	if (n -> op == NOT){
		pr_op (i, n -> op);
		print (i+INDENT, n -> unary);
	}
	else {
		print (i+INDENT, n -> right);
		pr_op (i, n -> op);
		print (i+INDENT, n -> left);
	}
}
#endif
