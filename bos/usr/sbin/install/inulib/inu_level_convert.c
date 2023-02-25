static char sccsid[] = "@(#)69  1.9  src/bos/usr/sbin/install/inulib/inu_level_convert.c, cmdinstl, bos411, 9428A410j 6/17/93 16:09:56";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_level_convert,inu_level_convert,inu_convert_one_num
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/file.h>
#include <installp.h>
#include <instl_options.h>



/*--------------------------------------------------------------------------*
**
** NAME: inu_level_convert
**
** FUNCTION:
**  Converts a ascii string of levels to a structure of ints for the levels.
**  Input (parm 1) is a string of the level of an lpp-option.
**  Output(parm 2) is a pointer to the Level_t structure of integers
**                 representing the same level.
**
** NOTES:
**  The expected format of the ascii level string (i_level parameter):
**   vv.rr.mmmm.ffff.ppppppp
**   ^  ^  ^    ^    ^
**   |  |  |    |    |the ptf id if this is a update (ascii alpha-numeric)
**   |  |  |    |     (for 3.1 updates this field will be "3.1updt")
**   |  |  |    |the fix level (ascii numeric)
**   |  |  |
**   |  |  |the modification level (ascii numeric)
**   |  |
**   |  |the release level (ascii numeric)
**   |
**   |the version level (ascii numeric)
**
**  Input (parm 1) is a string of the level of an lpp-option.
**  Output(parm 2) is a structure of integer representing the same level.
**
** RETURN VALUE DESCRIPTION:
**          SUCCESS: the level conversion was successful or
**          FAILURE: the level conversion failed.
**
**--------------------------------------------------------------------------*/

static int inu_convert_one_num(char *i_level, short *level, char *format, int maxsize)
{
	char *ptr;
	char tmp_string[TMP_BUF_SZ];
	int count;

	if(i_level==NULL)
		return(FAILURE);

	/* convert the number */
	if(level!=NULL)
		sscanf(i_level,format,level);

	/* strip off the first part */
	strcpy(tmp_string,i_level);
	ptr=strchr(tmp_string,'.');
	if(ptr!=NULL)
		*ptr='\0';

	/* is it too long? */
	if((count=strlen(tmp_string)) > maxsize)
		return(FAILURE);

	/* check for all digits */
	for(ptr = tmp_string; count > 0; ptr++,count--)
		if(!isdigit((int)*ptr))
			return(FAILURE);

	return(SUCCESS);
}

#define INU_NEXT_NUMBER(arg1,arg2)\
arg1=strchr(arg2,'.');\
if(arg1==NULL)\
	return(FAILURE)\

int inu_level_convert(
char *i_level,
Level_t *level)

{
	char *ptr;
	short ver,rel,mod,fix;

	/*--------------------------------------------------------------------*
	 * Convert i_level to level struct.
	 * If i_level is NIL(char) then set level->ver = -1
    	 *--------------------------------------------------------------------*/
	if (i_level[0] == '\0' || !isdigit(i_level[0])) {
		if(level==NULL)
			return(SUCCESS);
		level->ver = -1;
		level->ptf[0] = '\0';
		return(SUCCESS);
	}

	/* convert ver */
	if(inu_convert_one_num(i_level, &ver, "%2hd", 2)==FAILURE)
		return(FAILURE);

	/* convert rel */
	INU_NEXT_NUMBER(ptr,i_level);
	if(inu_convert_one_num(++ptr, &rel, "%2hd", 2)==FAILURE)
		return(FAILURE);

	/* convert mod */
	INU_NEXT_NUMBER(ptr,ptr);
	if(inu_convert_one_num(++ptr, &mod, "%4hd", 4)==FAILURE)
		return(FAILURE);

	/* convert fix */
	INU_NEXT_NUMBER(ptr,ptr);
	if(inu_convert_one_num(++ptr, &fix, "%4hd", 4)==FAILURE)
		return(FAILURE);

       /* ----------------  check for ptf ---------------- */
       /**
	**  The max allowable ptf length is 9 (not counting the 
	**  null terminator).  So if the ptf id length is 10 or 
	**  greater, we will return FAILURE here.
	**/

	ptr=strchr(ptr,'.');
	if(ptr!=NULL && strlen(++ptr) > (sizeof(level->ptf)) - 1)
		return(FAILURE);

	if(ptr!=NULL && *ptr!='\0' && !isalpha((int)*ptr))
		return(FAILURE);

	if(level==NULL)
		return(SUCCESS);
	
	/* put stuff in the level struct */

	level->ver=ver;
	level->mod=mod;
	level->fix=fix;
	level->rel=rel;
	*level->ptf='\0';
	if(ptr!=NULL)
		strcpy (level->ptf, ptr);
	return(SUCCESS);
}

/* redefine the next number stuff */
#undef INU_NEXT_NUMBER
#define INU_NEXT_NUMBER(arg1,arg2) \
arg1=strchr(arg2,'.');\
if(arg1==NULL)\
	return

void inu_31level_convert(char *i_level, Level_t *level)
{
	char *ptr;

	/* was a level speified? */
	if(i_level==0 || *i_level=='\0')
		return;

	/* get version */
	inu_convert_one_num(i_level, &level->ver, "%2hd", 2);

	/* get release */
	INU_NEXT_NUMBER(ptr,i_level);
	inu_convert_one_num(++ptr, &level->rel, "%2hd", 2);

	/* get mod */
	INU_NEXT_NUMBER(ptr,ptr);
	inu_convert_one_num(++ptr, &level->mod, "%4hd", 4);

	/* get fix */
	INU_NEXT_NUMBER(ptr,ptr);
	inu_convert_one_num(++ptr, &level->fix, "%4hd", 4);

}

