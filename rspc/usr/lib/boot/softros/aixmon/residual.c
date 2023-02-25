static char sccsid[] = "@(#)74	1.6  src/rspc/usr/lib/boot/softros/aixmon/residual.c, rspc_softros, rspc411, 9434B411a 8/25/94 08:56:50";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: setup_bitmap, get_nv_env
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* residual.c  - process residual data for Model 6015			*/

#include	"aixmon.h"
#include	"iplcb_init.h"
#include	<sys/residual.h>
#include	"nvram.h"


/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
	setup_bitmap() - Build the bitmap from residual data.

 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/

int setup_bitmap(int size, unsigned int *bitmap)
{
int i,j;

RESIDUAL	*rp;			/* Pointer to residual data 		*/
int		map_bytes;		/* Total bytes needed for bitmap	*/
int		map_words;		/* Total words needed for bitmap 	*/
uint		mask;

dbgmsg(AMON_D3,"Entering setup_bitmap");

rp = (RESIDUAL *) residual_save;	/* Let's point at residual data		*/

					/* Attempt to check for redisual level	*/

					/* Calculate number of bytes of bitmap  */
map_bytes = rp->GoodMemory / (8*BYTES_PER_BIT);
map_bytes += ((rp->GoodMemory % (8*BYTES_PER_BIT)) != 0)?1:0;

dbgtst(AMON_D11) printf("Map Bytes = 0x%8.8X  [%d]\n",map_bytes,map_bytes);


  				/* Fill in bitmap for memory size
				   Memory on Sandalfoot is contig and good	*/

map_words = map_bytes/4;	/* Each word = 512K of memory in bitmap		*/
map_words += ((map_bytes % 4) != 0)?1:0;

				/* Set all the bitmap to bad memory		*/
for(i=0;i<size;i++)
	bitmap[i] = 0xFFFFFFFF;
				/* Set the good bits in the bitmap		*/
for(i=0;i<map_words;i++)
        {
        mask = ~0x80000000;
        for(j=0;j<32;j++)
		{
		if(((i*32*BYTES_PER_BIT)+(j*BYTES_PER_BIT)) <= rp->GoodMemory)
			bitmap[i] &= mask;
                mask = mask >> 1;
                }
	}

dbgmsg(AMON_D11, "Returning from setup_bitmap ");
	
return(map_words);
}


/*****************************************************************************/
/*  get_nv_env() - Get the value of an environment variable from nvram	     */
/*****************************************************************************/

get_nv_env(char *name, char *answer)
{
NVRAM_MAP	nv;
char		*nvbuf;			/* Pointer to nvram buffer	  */
char		*envbuf;		/* Pointer to environment string  */
char		*envendbuf;		/* Pointer to end of env string   */
char		*env_name, *env_value;	/* Environment var pair		  */
int		i;
int		state;
int		match = FALSE;		/* Found a match to variable 	  */

					/* Get the NVRAM info into buffer */
nvbuf = (char *) &nv;
for(i=0;i<NVSIZE;i++)
	nvbuf[i] = read_nvram(i);

					/*********** FUTURE ***************/
					/*********** FUTURE ***************/
					/*********** FUTURE ***************/
/* Check here for the nvram level once the format has solidified	  */
					/*********** FUTURE ***************/
					/*********** FUTURE ***************/
					/*********** FUTURE ***************/
	
envbuf = nvbuf + (int) nv.Header.GEAddress;
envendbuf = envbuf+nv.Header.GELength;
state = 0;

while(state < 4)
	{
	switch(state)
		{
		case	0:		/* Find the name	*/
		while(envbuf[0] == ' ') envbuf++;
		env_name = envbuf;
		while(envbuf[0] != ' ' && envbuf[0] != '=' && envbuf[0] != 0) envbuf++;
		switch(envbuf[0])
			{
			case	' ':
			envbuf[0] = 0;
			envbuf++;
			if(strcmp(name,env_name) != 0)
				{
				state = 2;
				}
			else
				{
				match = TRUE;
				state = 1;
				}
			break;

			case	'=':
			envbuf[0] = 0;
			if(strcmp(name,env_name) == 0)
				match = TRUE;
			envbuf++;
			while(envbuf[0] == ' ' && envbuf[0] != 0) envbuf++;
			env_value = envbuf;
			state = 2;
			break;

			case	0:
			state = 4;
			break;
			}
		break;

		case	1:		/* Find start of variable */
		while(envbuf[0] != '=' && envbuf[0] != 0) envbuf++;
		switch(envbuf[0])
			{
			case	'=':
			while(envbuf[0] == ' ' && envbuf[0] != 0) envbuf++;
			env_value = envbuf;
			state = 2;
			break;

			case	0:
			state = 4; 
			break;
			}
		break;

		case	2:		/* Move pointer to the end */
		while(envbuf[0]) envbuf++;
		state = 3;
		break;

		case	3:		/* Process output and setup for next one */
		dbgtst(AMON_D11) printf("Name = %s  <--> Value = %s  Match = %s\n",
				        env_name, env_value,(match)?"TRUE":"FALSE"); 
		if(match)
			{
			strcpy(answer,env_value);
			return(match);
			}

		while(envbuf < envendbuf && envbuf[0] == 0) envbuf++;
		if(envbuf < envendbuf) 
			state = 0;
		else
			state = 4;
		break;

		}
	}
return(match);
}
