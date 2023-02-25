static char sccsid[] = "@(#)11	1.1  src/bos/usr/sbin/adfutil/dosnext.c, cmdadf, bos411, 9428A410j 8/20/93 10:22:16";
/*
 * COMPONENT_NAME: CMDADF
 *
 * FUNCTIONS: dosnext is_dir 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "tables.h"
#include "doserrno.h"
#include <sys/stat.h>

/* data structures used by match */
int _bnum;
struct BIND {
	      char *addr,length;
	    } _bind[100];

int     strlen();
char    *malloc();

char    *dosnext(srch)
DOSFIND *srch;
{
	register int i;
	register char *s,*t;
	static   char *result=0;
	char     prefix[16],suffix[16];
	pc_dirent *dnext(),*d;
	struct { short inode;
		 char  name[14]; }
		 entry;
	int read(), close();

	doserrno = 0;

	/* Handles no patterns in directory */

	if(result) { free(result); result=0; }

	for(;;) {

	    /*
	     *   Read the next directory, exit on EOF or error.
	     */
	TRACE(("dosnext: searchpath=%s\n", srch->path));

	    if (srch->is_dos) {
		d = dnext(&(srch->dos_srch));
		if (d==NULL) return(NULL);

		if  (d->df_attr & FIL_VOL)       continue;

		if ((d->df_attr & FIL_HDF) &&
		   ((srch->mode & S_HIDDEN)==0)) continue;

		if ((d->df_attr & FIL_HDD) &&
		   ((srch->mode & S_DIR)==0))    continue;

		if ((d->df_attr & FIL_SYS) &&
		   ((srch->mode & S_SYSTEM)==0)) continue;

		if(((d->df_attr & (FIL_HDF|FIL_HDD|FIL_SYS))==0) &&
		   ((srch->mode & S_REG)==0))    continue;

		_dcopy(&(d->df_use),prefix,8);
		_dcopy(d->df_ext,suffix,3);

	    } else {
		i = _devio(read, srch->handle, &entry, sizeof(entry) );
		if (i<=0) {
			if (i<0) doserrno = errno;
			_devio(close, srch->handle);
			return(NULL);
		}
		if (entry.inode==0) continue ;

		i = 0;
		if (*entry.name=='.') {
			i++;
			if ((srch->mode & S_HIDDEN)==0) continue;
		}
		if (is_dir(srch,entry.name)) {
			i++;
			if ((srch->mode & S_DIR)==0)    continue;
		}
		if (i==0 && (srch->mode & S_REG)==0)    continue;

		_getparts( entry.name, prefix, suffix );
	    }

	    /*
	     *   If it doesn't match pattern, loop around.
	     */

	    _bnum = 0;                               /* reset match memory */
	    if( _match(srch->base, prefix) &&
		_match(srch->extn, suffix) ) break;
	}

	TRACE(("dosnext: found (%s,%s)\n", prefix, suffix ));
	i = srch->base - srch->path;
	result=malloc(i+16);           /* room for directory plus one name */
	strncpy( result, srch->path, i);
	result[i]=0;                   /* null-terminate the n characters  */
	strcat(result, prefix);
	if ( *suffix || *prefix==0 ) {
	    strcat(result,".");
	    strcat(result,suffix);
	}
	return(result);
}

/*
 *  Copy n characters or up to blank from s to t, 0-terminate t.
 */

static
_dcopy(s,t,n)
register char *s,*t;
int n;
{
	register int i;
	for(i=0;i<n;i++) {
		if(*s==' ') break;
		*t++ = *s++;
	}
	*t=0;
}

/*
 *  Get DOS style prefix and suffix
 */

static
_getparts(source,prefix,suffix)
char *source, *prefix, *suffix;
{
	register int i;
	register char *s,*t;
	s = source;
	t = prefix;
	suffix[0] = 0;
	for(i=0;i<14;i++) {
	       if(*s==0) break;
	       if(*s=='.') {
			strncpy(suffix,++s,13-i);
			suffix[13-i] = 0;
			break;
	       }
	       *t++ = *s++;
	}
	*t=0;
}


/*
 *  Match(p,n)
 *
 *  Given a pattern string p and a name string n, return true
 *  if p matches n.  P may contain Global Filename character
 *  "?" and "*".
 *
 *  The table _bind is filled in with the bindings of each pattern
 *  character, in sequence, so that the bindings can later be used,
 *  e.g. in the case of COPY *.x *.y
 */

static
int _match(p,n)
register char *p,*n;
{
    register char *nsave;
    register int  bsave;

    for( ;; n++,p++ ){

	switch(*p) {
		case 0:   return(*n==0);

		case '*': if (p[1]==0) {
			      _Bind(n,strlen(n));
			      return(1);
			  }
			  nsave = n; bsave = _bnum;
			  for ( ; *n; n++ ) {
			      _Bind(nsave,n-nsave);
			      if(_match(p+1,n)) return(1);
			      _bnum = bsave;
			  }
			  return(0);

		case '?': if (*n==0) return(0);
			  _Bind(n,1);
			  continue;

		default:  if (*p!=*n) return(0);
			  if (*n==0)  return(1);
			  continue;
	}
    }
}

static
_Bind(addr,length)
char *addr;
int length;
{
	_bind[_bnum].addr     = addr;
	_bind[_bnum++].length = length;
}

is_dir(srch,s)
DOSFIND *srch;
char *s;
{
	APATH   a;
	int i;
	char name[128], *p;
	struct stat buf;
	p = srch->path;
	i = srch->base - p;                 /* get pathname prefix */
	strncpy( name, p, i);
	name[i] = 0;                        /* null-terminate if necessary */
	strncat( name, s, 14);
	name[i+14] = 0;

	_analyze( name, &a );
	stat( a.upath, &buf );
	TRACE(("is_dir: st_mode=0%o\n",buf.st_mode ));
	return( (buf.st_mode&0170000) == 0040000 );
}
