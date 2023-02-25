static char sccsid[] = "@(#)86	1.1  src/bos/usr/lib/nls/loc/iconv/fcconv/IBM-udcCN_UCS-2.c, ils-zh_CN, bos41B, 9504A 12/19/94 14:49:33";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: _iconv_close
 *		_iconv_exec
 *		_iconv_init
 *		instantiate
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/********************* START OF MODULE SPECIFICATION ***********************/
/*                                                                         */
/* MODULE NAME:        IBM-udcCN_UCS-2                                     */
/*                                                                         */
/* DESCRIPTIVE NAME:   Convert EUC Encoding to UCS-2(Transform of Unicode) */
/*                                                                         */
/* FUNCTION:           _iconv_exec         : Convert Data Encoding in One  */
/*                                           Codeset to Another            */
/*                                                                         */
/*                     _iconv_close        : Free Memory Allocated for     */
/*                                           Converter early               */
/*                                                                         */
/*                     _iconv_init         : Allocate Memory for Specific  */
/*                                           Converter Descriptor, and     */
/*                                           Initialize It                 */
/*                                                                         */
/*                     instantiate         : Initialize Core Converter     */
/*                                           Descriptor                    */
/*                                                                         */
/* MODULE TYPE:        C                                                   */
/*                                                                         */
/* COMPILER:           AIX C                                               */
/*                                                                         */
/* AUTHOR:             Dong Meiting, Wang Ying                             */
/*                                                                         */
/* STATUS:             Simplified Chinese Iconv Version 1.0                */
/*                                                                         */
/* CHANGE ACTIVITY:    None                                                */
/*                                                                         */
/********************* END OF SPECIFICATION ********************************/

#include <stdio.h>
#include <stdlib.h>
#include "iconv.h"
#include "iconvP.h"
#include "fcconv.h"
#include "udccn-ucs.h"

/**************************************************************************/
/* FUNCTION    : _iconv_exec                                              */
/* DESCRIPTION : Convert Data Encoding in IBM-udcCN Codeset to            */
/*               UCS-2(Transform of Unicode)                              */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd, inbuf, outbuf, inbytesleft, outbytesleft             */
/* OUTPUT      : ret_value                                                */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static int _iconv_exec (_LC_fcconv_iconv_t *cd,
                                unsigned char** inbuf, size_t *inbytesleft,
                                unsigned char** outbuf, size_t *outbytesleft)
{
        unsigned char   *in, *out;      /* Point to in/out buffers */
        unsigned char   *e_in, *e_out;  /* Point to the end of in/out buffers */
        int             ret_value;      /* Hold the return value */
	wchar_t		*ucs, ucsbuf;
        unsigned short  euc2, *euc2in;
	unsigned short  indx;           /* Used to locate the positon in a array */
	unsigned short  *outtmp;
	unsigned short  start, middle, end;

        if (!cd) {
            errno = EBADF; return NULL;
        };

        if (!inbuf) return 0;
 /*******************
  Initialize Pointers
  *******************/
 	e_in = (in = *inbuf) + *inbytesleft;
        e_out = (out = *outbuf) + *outbytesleft;

	euc2in = &euc2;
	ucs = &ucsbuf;

        ret_value = 0;

        while (in < e_in ) { 

 	    /* Handle ASCII Characters*/
            if (e_in - in < 2) {
               errno = EINVAL; 
	       ret_value = -1;
               break;
            }

	    /* Handle Characters belong to GB */
            if (in[0] < 0x21 || in[0] == 0x7f) {
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

            if (in[1] < 0x21 || in[1] == 0x7f) {
                errno = EILSEQ;
                ret_value = -1;
                break;
            };

 	    /* Obtain One Input Word, Either Hanzi or Graphic */
            *((unsigned char *)euc2in) = in[0];
            *((unsigned char *)euc2in +1) = in[1];

            start  = BEGIN;
            end    = END;
            middle = ( start + end ) / 2;
            while( *euc2in != udccntoucs[middle][0] && start <= end ) {
                if( *euc2in < udccntoucs[middle][0] ) 
                    end = middle - 1;
	        else
                    if( *euc2in > udccntoucs[middle][0] ) 
                        start = middle + 1;
                middle = ( start + end ) / 2;
            };

	    if(*euc2in == udccntoucs[middle][0])
                *ucs = udccntoucs[middle][1];
 /*******************************************
  Locate User-Defined Hanzi and Graphic Words
  *******************************************/
            else{
	        errno = E2BIG;
                ret_value = -1;
                break;
            };
  /***************** 
   Begin to convert all characters located before 
   *****************/
	    out[0] = *(unsigned char*)ucs;
	    out[1] = *((unsigned char*)ucs + 1);
	    in += 2;
	    out += 2;
			
      }; /* The end of while */
        *inbytesleft = e_in - in;
        *outbytesleft = e_out - out;
        *inbuf = in;
        *outbuf = out;
        return ret_value;
}

/**************************************************************************/
/* FUNCTION    : _iconv_close                                             */
/* DESCRIPTION : Free Memory Allocated for IBM-udcCN_UCS-2                */
/*               Converter Descriptor                                     */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : cd                                                       */
/* OUTPUT      : none                                                     */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static void     _iconv_close(iconv_t cd)
{
        if (cd){
              free(cd);
	}
        else
                errno = EBADF;
}

/**************************************************************************/
/* FUNCTION    : _iconv_init                                              */
/* DESCRIPTION : Initialize IBM-sbdCN_UCS-2  Converter-Specific           */
/*               Descriptor                                               */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : core_cd, toname, fromname                                */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
static _LC_fcconv_iconv_t       *_iconv_init (_LC_core_iconv_t *core_cd,
                                 char* toname, char* fromname)
{
        _LC_fcconv_iconv_t      *cd;

        if (!(cd = (_LC_fcconv_iconv_t *) malloc(sizeof(_LC_fcconv_iconv_t))))
                return (NULL);
        cd->core = *core_cd;
        return cd;
}

/**************************************************************************/
/* FUNCTION    : instantiate                                              */
/* DESCRIPTION : Initialize Core Converter Descriptor , It is program     */
/*               Entry Point                                              */
/* EXTERNAL REFERENCES:                                                   */
/* INPUT       : void                                                     */
/* OUTPUT      : cd                                                       */
/* CALLED      : iconv()                                                  */
/* CALL        :                                                          */
/**************************************************************************/
_LC_core_iconv_t        *instantiate(void)
{
        static _LC_core_iconv_t cd;

        cd.hdr.__magic   = _LC_MAGIC;
        cd.hdr.__version = _LC_VERSION;
        cd.hdr.__type_id = _LC_ICONV;
        cd.hdr.__size    = sizeof (_LC_core_iconv_t);
        cd.init          = (_LC_core_iconv_t*(*)())_iconv_init;
        cd.exec          = (size_t(*)())_iconv_exec;
        cd.close         = (int(*)())_iconv_close;
        return &cd;
}
