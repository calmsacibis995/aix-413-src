#if !defined(lint)
static char sccsid[] = "@(#)50	1.1  src/tenplus/lib/util/nls_path.c, tenplus, tenplus411, GOLD410 3/17/93 14:03:26";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:  nls_path	
 * 
 * ORIGINS: 27  
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

char	cpth[PATH_MAX];	/*non-auto string used to return result */

#define PATH_FORMAT	"/usr/lib/nls/msg/%L/%N:/usr/lib/nls/msg/prime/%N"

/*************************************************************************/
/*                                                                       */
/* nls_path:	Uses the X/Open approach to find the specified file      */
/*		in the international directory(s) defined by the         */
/*		current locale and NLSPATH environment vrbl.             */
/*                                                                       */
/* arguments:	char *filename - nls file to find                        */
/*                                                                       */
/* return value: 	char * ptr to relevant nls path or NULL          */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

char *nls_path(char *flname)
{
	char 	fl[PATH_MAX];	/*---- place to hold full path ----*/
	char 	*nlspath;	/*---- pointer to the nlspath val ----*/
	FILE	*fp;		/*---- file pointer ----*/
	char    *p,*np;
	char	*fulllang;	/* %L language value */
	char	lang[PATH_MAX]; /* %l language value */
	char	*territory;	/* %t language value */
	char	*codeset;	/* %c language value */
	char	*ptr;		/* for decompose of $LANG */
	char	*str;
	char	*optr;
	int	nchars;
	int	lenstr;
	char	outptr[PATH_MAX];
	int	valid;

	if ((nlspath = getenv("NLSPATH")) == NULL || *nlspath == '\0')
		nlspath = PATH_FORMAT; 
   	fulllang = setlocale(LC_MESSAGES, NULL);

	/*
	** LC_MESSAGES is a composite of three fields:
	** language_territory.codeset
	** and we're going to break it into those
	** three fields.
	*/

	strcpy(lang, fulllang);

	territory = "";
	codeset = "";

	ptr = (char*) strchr( lang, '_' );
	if (ptr != NULL)
	{
		territory = ptr+1;
		*ptr = '\0';
		ptr = (char*) strchr(territory, '.');
		if (ptr != NULL)
		{
			codeset = ptr+1;
			*ptr = '\0';
		}
	} else
	{
		ptr = (char*) strchr( lang, '.' );
		if (ptr != NULL)
		{
			codeset = ptr+1;
			*ptr = '\0';
		}
	}

	np = nlspath;
	while (*np)
	{
		p = cpth;
		while (*np && *np != ':')
			*p++ = *np++;
		*p = '\0';
		if (*np)	/*----  iff on a colon then advance --*/
			np++;
		valid = 0;
		if (strlen(cpth))
		{
			ptr = cpth;
			optr = outptr;

			nchars = 0;
			while (*ptr != '\0')
			{
				while ((*ptr != '\0') && (*ptr != '%') 
					      && (nchars < PATH_MAX))
				{
					*(optr++) = *(ptr++);
					nchars++;
				}
				if (*ptr == '%')
				{
					switch (*(++ptr))
					{
						case '%':
							str = "%";
							break;
						case 'L':
							valid = 1;
							str = fulllang;
							break;
						case 'N':
							valid = 1;
		    					str = flname;
							break;
						case 'l':
							str = lang;
							break;
						case 't':
							str = territory;
							break;
						case 'c':
							str = codeset;
							break;
						default:
							str = "";
							break;
					}
					lenstr = strlen(str);
					nchars += lenstr;
					if (nchars < PATH_MAX)
					{
						strcpy(optr, str);
						optr += lenstr;
					} else
					{	
						break;
					} 
					ptr++;
				} else
				{
					if (nchars >= PATH_MAX)
						break;
				}
			}
			*optr = '\0';
			strcpy(cpth, outptr);
		}
		else
		{ /*----  iff leading | trailing | adjacent colons ... --*/
			valid = 1;
			strcpy(cpth,flname);
		}
		if (valid == 1 && (fp = fopen(cpth,"r")))
		{
			fclose(fp);
			/* if no path, return "." for current dir */
			if ((p = strrchr(cpth,'/')) == NULL)
				return(".");
			*p = '\0';
			return(cpth);
		}
	}
        if (fp = fopen(flname,"r"))
        {
		fclose(fp);
		/* if no path, return "." for current dir */
		if ((p = strrchr(cpth,'/')) == NULL)
			return(".");
		*p = '\0';
		return(cpth);
        }
	return (NULL);
}

 
