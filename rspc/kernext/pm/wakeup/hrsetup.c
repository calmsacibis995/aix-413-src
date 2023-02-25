static char sccsid[] = "@(#)85  1.3  src/rspc/kernext/pm/wakeup/hrsetup.c, pwrsysx, rspc41J, 9519A_all 5/9/95 06:59:19";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: hrsetup
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *   hrsetup sets up all memory before passing control to CPU restore.
 *
 *        Note : Current Memory Usage
 *
 *               The wakeup code doesn't use the first 8KB in the wakeup area.
 *               The area is used to store compressed image.
 *
 *          address
 *             0  |------------------------------|
 *                |                              |
 *                |                              |
 *                |                              |
 *                |------------------------------|
 *                |  compressed image       (a)  |
 *                |                      --------|
 *                |                              |
 *                |                              |
 *                |                      --------|
 *                |                         (b)  |
 *                |------------------------------|
 *                |                              |
 *                |                              |
 *             YY |------------------------------|
 *                |                              |
 *                |                              |
 *                |                     ---------|
 *                |                     AREA(B)  |
 *                |------------------------------|
 *                |  wakeup area                 |
 *             XX |-                             |
 *                |  hrstart.s hrsetup.c  etc    |
 *                |------------------------------|
 *                |    AREA(A)                   |
 *         Highest|------------------------------|
 *
 *
 *
 *        Flow :
 *          - Move compressed image to higher address to fit address YY
 *            to address XX (wakeup area start address + 8KB).
 *            Wakeup code (hrstart.s, hrsetup.c, etc.) starts from XX.
 *          - The lowest address data in compress data is for AREA(A).
 *            Decompress the low address data(a) to fill up AREA(A)
 *          - Then decompress data from lower address compressed data to
 *            lower address memory.
 *          - At last, compressed data(b), which is in the beginning of
 *            the wakeup area now, is decompressed to AREA(B).
 *
 *
 *  Compressed Code Header.
 *    - First 2 bytes (short) is length (including the header).
 *    - Next 1 byte is flag (0x01 == compressed, 0x00 == not compressed)
 *    - Next 1 byte is check sum of original code.
 */

#include <sys/residual.h>
#include "hibernation.h"
#include "debug.h"

#define PAGE_SIZE	0x1000		/* 4KBytes */
#define BITMAP_MAX	8192		/* 8192*8*4096 = 256M support */

#define FREE_PAGE(addr) \
		(((bitmap[addr/(PAGE_SIZE*8)]) << ((addr/PAGE_SIZE)%8)) & 0x80)

static char	bitmap[BITMAP_MAX];	/* copy of bitmap in wakeup area */
static int	bitmaplen = BITMAP_MAX;
static int	D_CacheLineSize = 32;	/* D-cache size (default = 32) */

/****************************************************************/
/*                                                              */
/* hrsetup                                                      */
/*                                                              */
/*  rdata      : residual data starting address                 */
/*  pmhh_old   : header(when ROS ends) data starting address    */
/*  stk_bottom : our initial stack address                      */
/*                                                              */
/****************************************************************/
void
hrsetup(RESIDUAL *rdata, pm_hibernation_header_t *pmhh_old, void *stk_bottom)
{
	char			*wakeupaddr;	/* wakeup area + 8KB */
	pm_config_data_t	*pmcd;
	pm_hibernation_header_t	*pmhh;
	int			*si, *ti, cnt;
	char			*s, *t, *s_limit, *t_limit;

	DbgInit();	/* console initialize */
 
	DbgPrint(("\nWelcome to PM wakeup code: COMPORT setting 9600,N,8,1\n"));

	D_CacheLineSize = rdata->VitalProductData.D_CacheLineSize;
	wakeupaddr = (char *)((int)pmhh_old->wakeup_area + PAGE_SIZE * 2);
	pmhh = (pm_hibernation_header_t *)wakeupaddr;	/* copied header */
	pmcd = (pm_config_data_t *)(pmhh->config_data_offset + (int)pmhh);

	DbgPrint(("WAKEUP: new pmhh = %lx\n", pmhh));
	DbgPrint(("WAKEUP: new pmcd = %lx\n", pmcd));
	DbgPrint(("WAKEUP: stack bottom = %lx\n", stk_bottom));
	DbgPrint(("WAKEUP: wakeup_area_len = %lx\n", pmhh->wakeup_area_length));
	DbgPrint(("WAKEUP: wakeup_code = %lx\n", pmhh->wakeup_code_offset));
	DbgPrint(("WAKEUP: wakeup_code_len = %lx\n", pmhh->wakeup_code_length));
	DbgPrint(("WAKEUP: D_CacheLineSize = %lx\n", D_CacheLineSize));

	/*-----------------------------------------*/
	/* Move compressed image to higher address */
	/*-----------------------------------------*/
	DbgPrint(("WAKEUP: move compressed image\n"));

	ti = (int*)wakeupaddr;
	si = (int*)(pmhh->image_offset + pmhh->image_length + (int)pmhh_old);
	cnt = pmhh->image_length / sizeof(int);
	for(; cnt > 0; --cnt){
		*--ti = *--si;
	}

	t = (char *)ti;
	s = (char *)si;
	cnt = (pmhh->image_length) % sizeof(int);
	for(; cnt > 0; --cnt){
		*--t = *--s;
	}

	/*-----------------------------------------*/
	/* Move bitmap data, if exists.            */
	/*-----------------------------------------*/
	DbgPrint(("WAKEUP: move bitmap data\n"));

	s = (char *)(pmhh->bitmap_offset + (int)pmhh_old);
	t = &bitmap[0];
	bitmaplen = pmhh->bitmap_length;

	if(bitmaplen>BITMAP_MAX){
		bitmaplen = BITMAP_MAX;
	}
	DbgPrint(("WAKEUP: bitmap len = %lx, src= %lx\n", bitmaplen, s));
	for(cnt = 0; cnt < bitmaplen; cnt++){
		*t++ = *s++;
	}

	/*-----------------------------------------*/
	/* Decompress Highest Address Area         */
	/*-----------------------------------------*/
	DbgPrint(("WAKEUP: decompress highest address\n"));

	s = (char *)(wakeupaddr - pmhh->image_length);
	t = (char *)(wakeupaddr + pmhh->wakeup_area_length - PAGE_SIZE * 2);
	s_limit = (char *)(wakeupaddr - 4);
	t_limit = (char *)(pmcd->memory_size);
  
	if(decompmain(&s, &t, s_limit, t_limit) != 0){
		DbgPrint(("WAKEUP: decompress highest address failed\n"));
		goto_reboot();
	}
 
	/*-----------------------------------------*/
	/* Decompress Lower Address Area           */
	/*-----------------------------------------*/
	/* Since the last portion of the decompressed image is in the */
	/* first 8KB of wakeup area, we have no need to worry about   */
	/* data overlap.                                              */
	DbgPrint(("WAKEUP: decompress lower address\n"));

	t = 0;
	s_limit = (char *)(wakeupaddr - 4);	/* last 4bytes is terminator */
	t_limit = (char *)wakeupaddr;

	if(decompmain(&s, &t, s_limit, t_limit) != 0){
		DbgPrint(("WAKEUP: decompress lower address failed\n"));
		goto_reboot();
	}

	DbgPrint(("WAKEUP: jump to resume address= %lx\n", pmhh->restart));
	hrmain(pmhh);			/* Goto CPU resume and Resume */

	/* NOT REACHED HERE */
}


/****************************************************************/
/*                                                              */
/* Decompress image main process                                */
/*                                                              */
/*   returns  -1     : error                                    */
/*             0     : success                                  */
/****************************************************************/ 
int
decompmain(char **source, char **target, char *s_limit, char *t_limit)
{
	char	*s, *t;
	int	ssize;
   
	DbgPrint(("WAKEUP: t = %lx, t_limit = %lx, s = %lx, s_limit = %lx\n",
					*target, t_limit, *source, s_limit));

	t = *target;
	s = *source;
	while(t < t_limit && s < s_limit){

		if(FREE_PAGE((int)t)){
			DbgPrint(("WAKEUP: FREE_PAGE %lx\n", t));
			t += PAGE_SIZE;	/* the page is free then skip it */ 
			continue;
		}

		if(t + PAGE_SIZE > t_limit){
			return -1;	/* not enough target space */
		}

		ssize = decompsub(s, t);
		if(ssize < 0){
			DbgPrint(("WAKEUP: ssize = %lx\n", ssize));
			return -1;
		}

		s += ssize;
		t += PAGE_SIZE;
	}
	*target = t;
	*source = s;
 
	return 0;
 
}


/****************************************************************/
/*                                                              */
/* Decompress one page                                          */
/*                                                              */
/*   returns  -1     : terminator found                         */
/*            -2     : decompressed data size != PAGE_SIZE      */
/*            -3     : source data overlapped by target data    */
/*   other positives : source data increment count              */
/*                                                              */
/****************************************************************/
#pragma mc_func		dcbf	{ "7c0018ac" }		/* dcbf 0, r3 */
#pragma reg_killed_by	dcbf	gr3
int
decompsub(char *source, char *target)
{
	char		sum, flag;
	int		i, len, outcnt;
	register char	*s, *t;

	s = source;
	t = target;

	if(*((int *)s) == 0){ 
		return -1;		/* terminator found */ 
	}

	/* get compressed data header */
	len = ((int)*s++) << 8;
	len |= *s++;
	len -= 4;			/* adjust header length */
	flag = *s++;
	sum = *s++;
      
	if(flag){		/* decompress the data */
		decompress(s, len, t, &outcnt);
		s += len;
	}else{			/* copy the data */
		for(i = 0; i < len; i++){
			*t++ = *s++;
		}
		outcnt = len;
	}
	if(outcnt != PAGE_SIZE){
		return -2;
	}

	/* flush D-cache */
	for(i = 0; i < outcnt; i += D_CacheLineSize){
		dcbf(target + i);
	}

	return(len + 4);

}
