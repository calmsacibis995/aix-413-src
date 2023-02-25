static char sccsid[] = "@(#)27	1.4  src/bos/usr/sbin/crash/stab.c, cmdcrash, bos411, 9428A410j 6/7/94 10:26:54";

/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: stab
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */
/*
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

#include <stdio.h>
#include <errno.h>
#include <dbxstclass.h>
#include <sys/dump.h>
#include <a.out.h>
#include "crash_msg.h" 
#include "crash.h"

extern nl_catd  scmc_catd;
extern 	int	dumpflag;
extern	char	*namelist;
extern struct cm *cm;

#define NAMESZ 128
static char *prog;

static struct s_ent {
    char *name;
    int offset;		/* in bits */
    int size;		/* in bits */
    int tid, btid;	/* type id, base type id (for arrays) */
    int type;		/* 's', 'a', etc */
    struct s_ent *next;	/* next field for structures */
} symtab[8192];
static int sent = 0;


static char *s;

#define eat(c) s++
#define isfirsttid(c) (((c) >= '0' && (c) <= '9') || (c) == '-' || (c) == '*')
#define issimpletid(c) ((c) < 0 && (c) >= -(TP_NTYPES))
#define isnamec(c) ((c) && (c)!=':' && (c)!=';' && (c)!='"' && (c)!='\'')
#define getint() (int) strtol(s, &s, 10)

#define PTROFF 20000
#define MINPTR (PTROFF-1024)
#define isptr(t) ((t) >= MINPTR)
#define tobase(t) ((t) - PTROFF)
#define toptr(t) ((t) + PTROFF)


static char *
xmalloc(sz)
int sz;
{
    char *s;

    s = (char *)malloc(sz);
    if (s == (char *)0) {
	fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, STAB_MSG_1,
		"%s: out of memory!\n"), prog);
	exit(ENOMEM);
    }
    return s;
}

static void
addfield(sym, id, tid, off, bits)
int sym;
char *id;		/* identifier */
int tid;		/* type id */
int off, bits;		/* bit offset, number of bits */
{
    struct s_ent *f, *g, *a;

    f = (struct s_ent *)xmalloc(sizeof(struct s_ent));
    f->name = id;
    f->offset = off;
    f->size = bits;
    if (isptr(tid)) {
	fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, STAB_MSG_2,
	    "WARNING: pointer in field definition\n") );
	tid = tobase(tid);
	f->type = 'p';
    }
    else
	f->type = ' ';
    f->tid = tid;
    f->next = NULL;

    /* add it */
    for (a = g = symtab + sym; g != NULL; g = g->next)
	a = g;
    a->next = f;
}

static int
addsym(id, tid, size, type, btid)
char *id;
int tid;
int size;
char type;
int btid;
{
    struct s_ent *s;

    s = symtab + sent;

    if (isptr(tid))
	tid = tobase(tid);
    if (isptr(btid)) {
	btid = tobase(btid);
	type = 'p';
    }

    s->type = type;
    s->name = id;
    s->offset = 0;
    s->size = size;
    s->tid = tid;
    s->btid = btid;
    s->next = NULL;

    return sent++;
}

static char *
name()
{
    char id[NAMESZ], *n = id;

    while (isnamec(*s))
        *n++ = *s++;
    *n = '\0';
    n = xmalloc(strlen(id)+1);
    strcpy(n, id);
    return n;
}

static void
field(sym)
int sym;
{
    char *id;
    int bitoffset, numbits, tid;

    id = name();
    eat(';');
    tid = typeid(id);
    eat(',');
    bitoffset = getint();
    eat(',');
    numbits = getint();
    eat(';');
    addfield(sym, id, tid, bitoffset, numbits);
    return;
}

static void
fieldlist(sym)
int sym;
{
    while (isnamec(*s))
        field(sym);
    return;
}

static void
record(id, tid)
char *id;
int tid;
{
    char su;
    int numbytes;
    int sym;

    su = *s++;
    numbytes = getint();
    sym = addsym(id, tid, numbytes*8, su, 0);
    fieldlist(sym);
    return;
}

static void
array(id, tid)
char *id;
int tid;
{
    int btid;
    void typedefd();

    eat('a');
    typedefd(id, tid);
    eat(';');
    btid = typeid(id);
    addsym(id, tid, 0, 'a', btid);
    return;
}

static void
subrange(id, tid)
char *id;
int tid;
{
    int lbound, ubound;
    int btid;

    eat('r');
    btid = typeid(id);
    eat(';');
    lbound = getint();
    eat(';');
    ubound = getint();
    addsym(id, btid, 0, ' ', 0);
    return;
}

static void
typedefd(id, tid)
char *id;
int tid;
{
    int btid, sz;

    switch (*s) {
    case 'a':
        array(id, tid);
        break;
    case 'r':
        subrange(id, tid);
        break;
    case 's':
    case 'u':
        record(id, tid);
        break;
    case 'x':
        eat('x');
        typedefd(id, tid);
        break;
    case 'f':
        eat('f');
        btid = typeid(id);
        eat(',');
        getint();
        eat(',');
        getint();
	addsym(id, tid, 0, 'f', btid);
        break;
    case 'g':
        eat('g');
        btid = typeid(id);
        eat(';');
        sz = getint();
	addsym(id, tid, sz, ' ', btid);
        break;
    default:
        if (isfirsttid(*s)) {
            btid = typeid(id);
	    addsym(id, tid, 0, ' ', btid);
	}
        break;
    }
    return;
}

static void
typeattrs()
{
    /* this is useless info */
    while (*s == '@') {
        while (*s != ';')
            s++;
        s++;
    }
    return;
}

static int
typeid(id)
char *id;
{
    int tid, ptr = 0;

    if (*s == '*') {
	ptr = 1;
        eat('*');
    }
    tid = getint();
    if (ptr)
	tid = toptr(tid);

    if (*s == '=') {
        while (*s == '=') {
            eat('=');
            if (*s == '@')
                typeattrs();
            typedefd(id, tid);
        }
    }
    return tid;
}

static void
namedtype(id)
char *id;
{
    int type;
    int tid;

    type = *s++;
    tid = typeid(id);
    return;
}

static void
class(id)
char *id;
{

    if (*s != 't' && *s != 'T')
        return;

    namedtype(id);
    return;
}

static void
stabline()
{
    /* simple parser for dbx storage class stuff */
    char *id;

    id = "";
    if (*s != ':') {
        id = name();
        if (*s != ':')
            return;
    }
    eat(':');
    class(id);
    return;
}

static void
indent(i)
int i;
{
    while (i--)
	fputs("    ", stdout);
}


static void
writeval(mem, tid, boff, bsize)
unsigned char *mem;
int tid;
int boff, bsize;
{
    char c;
    float f;
    double d;
    int i;
    unsigned long mask, shift, off, data;

    off = boff / 8;
    switch (tid) {
    case TP_CHAR:
	c = mem[off] & 0xff;
	printf("\t= %#02x", c);
	if (isprint(c))
	    printf(" '%c'", c);
	fputs(";\n", stdout);
	break;
    case TP_FLOAT:
	f = *((float *)(mem+off));
	printf("\t= %g;\n", f);
    case TP_DOUBLE:
    case TP_LDOUBLE:
	d = *((double *)(mem+off));
	printf("\t= %g;\n", d);
	break;
    default:
	switch (bsize) {
	case 8:
	    printf("\t= %#02x;\n", mem[off] & 0xff);
	    break;
	case 16:
	    printf("\t= %#04x;\n", *((short *)(mem+off)) & 0xffff);
	    break;
	case 32:
	    printf("\t= %#08x;\n", *((int *)(mem+off)));
	    break;
	default:
	    mask = (1 << bsize) - 1;
	    shift = 32 - bsize - (boff % 32);
	    data = *((unsigned int *)mem + boff / 32);
	    printf(":%d\t= %#x;\n",
		bsize, ((data >> shift) & mask));
	    break;
	}
	break;
    }
    return;
}

static int
getsize(tid)
int tid;
{
    /* returns the size of type in bits */

    static sizes[TP_NTYPES] = {
	/* for tid TP_INT (-1), ... */
	32, 8, 16, 32, 8, 8, 16, 32,
	32, 32, 32, 32, 64, 64, 32, 8,
	32, 64, 32, 8, 8, 16, 32, 32,
	64, 128, 8, 16, 32, 16
    };
    int size = -1, i;

    if (issimpletid(tid))
	size = sizes[-tid-1];
    else {
	for (i=0; i<sent; i++) {
	    if (symtab[i].tid == tid) {
		size = symtab[i].size;
		break;
	    }
	}
    }
    return size;
}

static unsigned char words[64*1024];

static int
display(tid, name, off, size, l, dostruct)
int tid;
char *name;
int off, size;
int l;
int dostruct;
{
    int i, j, k, bsize;
    char nname[NAMESZ];

    for (i=0; i<sent; i++) {
	if (symtab[i].tid == tid) {
	    struct s_ent *s = symtab + i;
	    int ai;
	    switch (s->type) {
	    case 's':
	    case 'u':
		indent(l);
		printf("%s %s ", (s->type == 's')? "struct": "union", s->name);
		if (dostruct) {
		    struct s_ent *f = s;
		    printf("{\n");
		    for (f = s->next; f != NULL; f = f->next) {
			if (display(f->tid,
			f->name, off+f->offset, f->size, l+1, 1) == 0)
			    fputs(";\n", stdout);
		    }
		    indent(l);
		    fputs("} ", stdout);
		    fputs(name, stdout);
		    return(0);
		}
		else {
		    fputs(name, stdout);
		    writeval(words, s->tid, off, size);
		}
		break;
	    case 'a':
		if ((bsize = getsize(s->btid)) <= 0)
		    bsize = size;
		/* If s->btid is a signed char we should probably print
		 * the array as a string.  if s->btid is a simple
		 * type (issimpletid) we should consider printing it
		 * in the standard C format like { 0, 0, 0, 0 };  For
		 * now we print each element separately in all cases.
		 */
		for (j=0; j<size/bsize; j++) {
		    sprintf(nname, "%s[%d]", name, j);
		    if (display(s->btid, nname, off+j*bsize, bsize, l, 1) == 0)
			fputs(";\n", stdout);
		}
		break;
	    case 'f':
		*nname = '(';
		strcpy(nname+1, name);
		strcat(nname, ")()");
		display(s->btid, nname, off, size, l, 0);
		break;
	    case 'p':
		*nname = '*';
		strcpy(nname+1, name);
		display(s->btid, nname, off, size, l, 0);
		break;
	    default:
		indent(l);
		if (*symtab[i].name == '\0')
		    printf("unknown %s", name);
		else
		    printf("%s %s", s->name, name);
		writeval(words, s->tid, off, size);
		break;
	    }
	    return 1;
	}
    }
    indent(l);
    printf("(%d) %s", tid, name);
    return 1;
}

static int
sdisplay(st, saddr)
char *st;
char *saddr;
{
    static int wordsz;
    char *addr;
    ulong ext;
    int i;

    for (i=0; i<sent; i++) {
	if (strcmp(symtab[i].name, st) == 0) {
	    struct s_ent *f, *s;
	    s = symtab+i;

	    addr = saddr;
	    wordsz = s->size / 8;
	    if (cm->cm_thread != -1) {
		/* verify that this is a valid segment for the process */
		if (verify(cm) == 0) {
		    printf(NLcatgets(scmc_catd, MS_crsh, SYMTAB_MSG_16,
			"Segment %d has become invalid for currently mapped process\n"),
			cm->cm_reg);
		    return(-1);
		}
		ext = cm->cm_segid;
	    }

	    if ((!dumpflag) && (cm->cm_thread != -1)) {
		lseek(mem, addr, 0);
		if (readx(mem, words, wordsz, ext) != wordsz) {
		    printf( NLcatgets(scmc_catd, MS_crsh,
			SYMTAB_MSG_24, "  read error"));
		    return -1;
		}
	    }
	    else {
		if (readmem_thread(words, addr, wordsz, cm->cm_thread) != wordsz) {
		    printf( NLcatgets(scmc_catd, MS_crsh,
			SYMTAB_MSG_24, "  read error"));
		    return -1;
		}
	    }
	    if (display(s->tid, "foo", 0, s->size, 0, 1) == 0)
		printf(";\n");
	    return 0;
	}
    }
    printf(NLcatgets(scmc_catd, MS_crsh, STAB_MSG_3,
	"type definition not found\n") );
    return -1;
}

int
stab(struc, addr, file)
char *struc;
char *addr;
char *file;
{
#  define NB_MAGIC(c) ((c).filehdr.f_magic != U802WRMAGIC \
    && (c).filehdr.f_magic != U802ROMAGIC \
    && (c).filehdr.f_magic != U802TOCMAGIC)

    static char *init_stab[] = TP_ARRAY;
    static int initted = 0;
    int ifd;
    int rc;

    char *buf;
    short len;
    int i;
    struct scnhdr scn;
    struct xcoffhdr coff;
    struct syment syment;

    prog = "print";

    /* Fill in symbol table if not already done */
    if (!initted) {
	/* AIX requires that the symbol table be pre-loaded.  */
	for (i = 0; i < sizeof(init_stab)/sizeof(char *); i++) {
	    s = init_stab[i];
	    (void) stabline();
	}

	if (file == NULL && (file = getenv("CRASHSYM")) == NULL)
	    file = "/etc/crash.sym";

	if ((ifd = open(file, O_RDONLY)) < 0) {
	    fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, STAB_MSG_4,
		"%s: no debug/include file(s) specified\n"), prog, file);
	    return -1;
	}

	read(ifd, &coff, sizeof(coff));
	if (NB_MAGIC(coff)) {
	    fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, STAB_MSG_5,
		"no debug info found (bad magic)\n") );
	    return 1;
	}
	for (i=0; i < coff.filehdr.f_nscns; i++) {
	    read(ifd, &scn, sizeof(scn));
	    if (strcmp(scn.s_name, _DEBUG) == 0)
		break;
	}
	for (i=0; i < coff.filehdr.f_nsyms; i++) {
	    lseek(ifd, coff.filehdr.f_symptr + i*SYMESZ, 0);
	    read(ifd, &syment, sizeof(syment));
	    if ((short)syment.n_scnum != N_DEBUG)
		    continue;
	    if (syment.n_zeroes == 0) {
		lseek(ifd, scn.s_scnptr + syment.n_offset - sizeof(len), 0);
		read(ifd, &len, sizeof(len));
		buf = malloc(len);
		read(ifd, buf, len);
	    }
	    else {
		buf = malloc(9);
		strncpy(buf, syment.n_name, 8);
		buf[8] = '\0';
	    }
	    s = buf;
	    (void) stabline();
	    free(buf);
	}
	initted = 1;
	(void) close(ifd);
    }
    rc = sdisplay(struc, addr);
    putchar('\n');
    return rc;
}
