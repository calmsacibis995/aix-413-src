static char sccsid[] = "@(#)02	1.6  src/rspc/usr/lib/boot/softros/aixmon/nv_utils.c, rspc_softros, rspc411, 9436E411a 9/9/94 18:14:20";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: nv_load
 *		nv_store
 *		read_nvram
 *		write_nvram
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


/* nv_utils.c -- Routines for accessing the system NVRAM	*/
#include  <aixmon.h>
#include  <iplcb_init.h>

#define NVRAM_IO_BASE		0x80000000

unsigned char	*nvram_buf;

#ifdef DEBUG

void write_nvram(int address, unsigned char val) 
{

int	high;
int	low;
    
*(&nvram_buf) = (uchar *)LOAD_ADDR;       

address &= 0x00000fff;

low = address & 0x0000ff;
high = address >> 8;
    					/* Setup the address		*/
outbyte(NVRAM_IO_BASE + nvram_addr, low);
outbyte(NVRAM_IO_BASE + nvram_addr + 1, high);
					/* Write to the data port	*/
outbyte(NVRAM_IO_BASE + nvram_data, val);
}
#endif /*DEBUG*/

unsigned char read_nvram(int address) 
{

int	high;
int	low;
    
*(&nvram_buf) = (uchar *)LOAD_ADDR;       
address &= 0x00000fff;
low = address & 0x0000ff;
high = address >> 8;
    
					/* Setup the address registers	*/
outbyte(NVRAM_IO_BASE + nvram_addr, low);
outbyte(NVRAM_IO_BASE + nvram_addr + 1, high);
					/* Read from the data port	*/
return (unsigned char)(inbyte(NVRAM_IO_BASE + nvram_data));
}

#ifdef DEBUG

void nv_load(int argc, char **argv) 
{
int	i;
    
if (argc == 99) 
	{
	printf("Use this to load in all of NVRAM into a buffer.\n");
	printf("It will print out the address of the buffer that the\n");
	printf("is stored at, and you can modify the buffer, then use\n");
	printf("the nv_store to write it back out to the real NVRAM.\n");
	return;
	}

bzero(nvram_buf, NVRAM_SIZE);

for (i = 0; i < NVRAM_SIZE; i++) 
	nvram_buf[i] = read_nvram(i);

status |= STAT_VALID_NV;
    
printf("NVRAM read to address: 0x%8.8X\n", nvram_buf); 
}

void nv_store(int argc, char **argv) 
{
int	i;

if (argc == 99) 
	{
	printf("Use this to store the nvram_buffer back into the real NVRAM\n.");
	return;
	}

if(!(status & STAT_VALID_NV))
	{
	printf("No valid NVRAM image loaded store failed\n");
	return;
	}

for (i = 0; i < NVRAM_SIZE; i++) 
	write_nvram(i, nvram_buf[i]); 

printf("NVRAM store complete\n");
}
#endif /*DEBUG*/
