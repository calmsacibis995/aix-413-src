static char sccsid[] = "@(#)01	1.2  src/rspc/usr/lib/boot/softros/aixmon/mcmnd.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:34:11";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: dump_words
 *		getbase
 *		in_word
 *		jmm_dump
 *		jmm_modify
 *		jmm_reset
 *		mem_read
 *		mem_write
 *		out_word
 *		stoui
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
#ifdef DEBUG
#include <types.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "model.h"

extern void dbugout(char *, int);
extern void soft_reboot();

/*   This file contains a bunch of utilities for the SANDALFOOT AIX ROS/BOOT */
/* program.  These utilities are in the standard COX format for linking into */
/* the command executor.                                                     */


/* getbase - Return the current default RADIX                                */
static int getbase(int def)

{
  int base;                        /* Default base to use for numbers        */
  char *pbase;                     /* Pointer to base environment var        */

  /* Figure out what base we should use                                      */
  pbase = NULL;
  base = pbase ? atoi(pbase) : def;
  switch (base)
    {
    case  2:
    case  8:
    case 10:
    case 16:
      break;

    default:
      base = def;
      printf("Using default RADIX %d!\n ",base);
      break;
    }
  return(base);
}


/* stoui - Convert a string to an unsigned int using either the default      */
/*         RADIX or one encoded in the string itself.  Return the result     */
/*         to passed pointer.  If invalid string encountered, return 0 else  */
/*         return 1.                                                         */
int stoui(char *buf, unsigned int *value, int def)

{
  int base;
  unsigned int total = 0;
  char c;

  /* Get the default base                                                    */
  base = getbase(def);

  /* Skip leading spaces and tabs - Not really needed by AIXMON              */
  while ((*buf == ' ') || (*buf == '\t'))
    ++buf;

  if ( *buf == 'x'  ||  *buf == 'X' )
    {
    ++buf;
    base = 16;
    }

  /* Return 0 if there is no other data                                      */
  if ( *buf == '\0')
    return(0);

  /* Accept characters with a prefix of "'"                                  */
  if (*buf == 0x27)
    {
    *value = (int)buf[1];
    return(1);
    }

  /* Main loop looks for end of string or invalid data                       */
  while ((c = toupper(*buf++)) != '\0' )
    if ((c >= '0') && (c <= '1'))
      total = total * base + c - '0';
    else
      if ((base >= 8) && (c >= '2') && (c <= '7'))
        total = total * base + c - '0';
      else
        if ((base >= 10) && (c >= '8') && (c <= '9'))
          total = total * base + c - '0';
        else
          if ((c >= 'A') && (c <= 'F'))
            total = total * base + c - 'A' + 10;
          else
            return(0);

  /* All converted - time to go                                              */
  *value = total;
  return(1);
}


/* jmm_modify - This routine steps through memory/IO/Amem and allows the     */
/*              user to change values.                                       */

int jmm_modify(int argc, char *argv[])

{
  unsigned char *where;            /* Pointer to address to modify           */
  unsigned char in, out;           /* Original value, new value              */
  unsigned int invalue;            /* Value read from user                   */
  char *prompt;                    /* Replace value prompt                   */
  int base;                        /* Base to use for numbers                */
  signed char buffer[80];          /* A place to read users data into        */

  /* Check parameters and display help message if wrong                      */
  if (argc != 2)
    {
    printf("  Format is: %s <addr>\n",argv[0]);
    printf("    where <addr> is the address to start looking at data\n");
    return(0);
    }

  /* Construct a pointer to the address they have passed                     */
  if (!stoui(argv[1], (uint *)&where, 16))
    {
    printf("Invalid starting address %s\n",argv[1]);
    return(0);
    }

  /* Create a prompt string for now on                                       */
  if (where < (uchar *)0x80000000)
    prompt = "Mem  %8.8X: Is %2.2X[%c] <-- ";
  else
    if (where > (uchar *)0xC0000000)
      prompt = "Amem %8.8X: Is %2.2X[%c] <-- ";
    else
      prompt = "I/O  %8.8X: Is %2.2X[%c] <-- ";

  /* Loop until they are done                                                */
  for (;1;)                        /* Do forever                             */
    {
    in = *where;                   /* Get the current value in memory        */
    out = isprint(in) ? in : '.';
    printf(prompt, where, in, out);
    gets((uchar *)buffer);
    if (strlen((uchar *)buffer) > sizeof(buffer))
      {
      printf("Over ran input buffer in gets()!\n");
      exit(8);
      }
    if (buffer[0] == EOF)         /* EOF to end                             */
      break;
    if (buffer[0] == 0)           /* <CR> goes to the next line             */
      {
      where++;
      continue;
      }
    if (!stoui((uchar *)buffer, &invalue, 16))
      printf("  Invalid value(%2.2X)!  <^D> to end\n",buffer[0]);
    else
      if (invalue > 255)
        printf("  Value must be a byte!\n");
      else
        {
        *where = invalue;          /* Update the value with new              */
        where++;
        }
    }
  printf("Done\n");
}

/* mem_read() Function returns a byte from the io space			*/	
/* 	      and displays the value					*/

void mem_read(int argc, char **argv) 
{
    unsigned char	in;
    unsigned int	where;
    char *prompt;                    /* Replace value prompt 		*/

    /* Check parameters and display help message if wrong 		*/
    if (argc != 2)
    {
	printf("  Format is: %s <addr>\n",argv[0]);
	printf("    where <addr> is the address to read data\n");
	return;
    }
    
    /* Construct a pointer to the address they have passed       	*/
    if (!stoui(argv[1], &where, 16))
    {
	printf("Invalid starting address %s\n",argv[1]);
	return;
    }
    
					/* Create prompt		*/
if (where > 0xC0000000)
      prompt = "Amem %8.8X: Is %2.2X [%c]";
   else
      prompt = "I/O  %8.8X: Is %2.2X [%c]";

    where |= 0x80000000;

    in = inbyte(where);		/* Get the current value in memory	*/
    printf(prompt, where, in, (in>32)?in:'.');
}

/* mem_write() Function writes a byte to the io space			*/	
/* 	      								*/

void mem_write(int argc, char **argv) 
{
unsigned char	out;
unsigned int	where,vin;
    
					/* Check parms print message	*/
if (argc != 3)
	{
	printf("  Format is: %s <addr> [value]\n",argv[0]);
	printf("    where <addr> is the address to write data\n");
	printf("    and value is the value to write\n");
	return;
	}
					/* Get address from args	*/
if (!stoui(argv[1], &where, 16))
	{
	printf("Invalid starting address %s\n",argv[1]);
	return;
	}
if (!stoui(argv[2], &vin, 16)) 
	{
    	printf("  Invalid value %s ",argv[2]);
    	return;
	}
out = vin;
where |= 0x80000000;

outbyte(where, out);
printf("Wrote [%x] to [%8.8x]\n", out, where);

}


int jmm_dump(int argc, char *argv[])

{
  unsigned char *where;            /* Pointer to where to start dumping      */
  unsigned int length;             /* Length to dump                         */

  /* Make sure we have enough parameters                                     */
  if (argc != 3)
    {
    printf("  Format is %s <start_addr>,<length>\n",argv[0]);
    printf("    where <addr> is tha address to start looking and\n");
    printf("          <length> is the number of bytes to dump\n");
    return(0);
    }

  /* Get the pointer converted                                               */
  if (!stoui(argv[1], (uint *)&where, 16))
    {
    printf("Invalid starting address %s\n",argv[1]);
    return(0);
    }

  /* Get the length converted                                                */
  if (!stoui(argv[2], &length, 16))
    {
    printf("Invalid length %s\n",argv[2]);
    return(0);
    }

  /* Call the Cox dumper                                                     */
  dbugout(where,length);
}


void jmm_reset()

{
  printf("Reseting the machine.....\n");
  if (processor != VER_601) {
	  prep_hid0();
	  printf("HID0 = 0x%x\n",read_hid0());
  }
  soft_reboot();
}


/* in_word() Function returns a word from the io space			*/	
/* 	      and displays the value					*/

void in_word(int argc, char **argv) 
{
    unsigned int	in,i;
    unsigned int	where, wherea;
    char *prompt;                    /* Replace value prompt 		*/

					/* Check parms and give help	*/
if (argc != 2)
	{
	printf("  Format is: %s <addr>\n",argv[0]);
	printf("    where <addr> is the address to read data\n");
	return;
	}
    
/* Construct a pointer to the address they have passed       	*/
if (!stoui(argv[1], &where, 16))
	{
	printf("Invalid starting address %s\n",argv[1]);
	return;
	}
    
wherea = where & 0xFFFFFFFC;	
if(where != wherea)
	printf("Warning: Address 0x%8.8X not word aligned\n",where);

wherea |= 0x80000000;
					/* Create prompt		*/
if (wherea > 0xC0000000)
      prompt = "Amem %8.8X: Is %8.8X [";
   else
      prompt = "I/O  %8.8X: Is %8.8X [";

in = inword(wherea);		/* Get the current value in memory	*/
printf(prompt, wherea, in);
				/* Just for Jim Mott here's the char	*/
prompt = (char *) &in;
for(i=0;i<4;i++)
	printf("%c",(prompt[i] > 32)?prompt[i]:'.');
printf("]");

}

/* out_word() Function writes a word to the io space			*/	
/* 	      								*/

void out_word(int argc, char **argv) 
{
unsigned int	out;
unsigned int	where, wherea;
    
					/* Check parms and display help	*/
if (argc != 3)
	{
	printf("  Format is: %s <addr> [value]\n",argv[0]);
	printf("    where <addr> is the address to write data\n");
	printf("    and value is the value to write\n");
	return;
	}

    					/* Get address pointer		*/
if (!stoui(argv[1], &where, 16))
	{
	printf("Invalid starting address %s\n",argv[1]);
	return;
	}

					/* Get the value from argv	*/
if (!stoui(argv[2], &out, 16))
	{
	printf("Invalid value %s\n",argv[1]);
	return;
	}
				/* Make sure it is word aligned		*/
wherea = where & 0xFFFFFFFC;	
if(where != wherea)
	{
	printf("Invalid address 0x%8.8X must be word aligned\n",where);
	return;
	}

where |= 0x80000000;

where |= 0x80000000;
outword(where,out);

}

/* dump_words() Function to dump from I/O space on word boundaries	*/
/* 	      								*/

int dump_words(int argc, char *argv[])

{
unsigned int where;		/* Pointer to where to start dumping    */
unsigned int length;            /* Length to dump                       */
int	i,j,k,lines;
int	tbuf[4];		/* Line buffer				*/

				/* Check parms and give help		*/
if (argc != 3)
	{
	printf("  Format is %s <start_addr>,<length>\n",argv[0]);
	printf("    where <addr> is tha address to start looking and\n");
	printf("          <length> is the number of bytes to dump\n");
	return(0);
	}

				/* Get starting Address			*/
if (!stoui(argv[1], &where, 16))
	{
	printf("Invalid starting address %s\n",argv[1]);
	return(0);
	}

				/* Get the length to dump		*/
if (!stoui(argv[2], &length, 16))
	{
	printf("Invalid length %s\n",argv[2]);
	return(0);
	}

where &= 0xFFFFFFFC;			/* Word align the from address	*/
lines = (length/16)+((length%16) != 0);
for(j=0;j<lines;j++)
	{
	for(i=0;i<4;i++)
		tbuf[i] = inword(where+(i*4));
	hexdump((char *)tbuf,16,where);
	where += 16;
	}
}
#endif /*DEBUG*/
