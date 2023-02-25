static char sccsid[] = "@(#)72	1.5  src/bos/usr/sbin/crash/POWER/decode.c, cmdcrash, bos411, 9435A411a 8/25/94 17:12:32";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS:	decode, Bform, SCform, XLform, Dform, Iform, Xform, XFXform,
 *		XFLform, XOform, Aform, Mform, DSform
 *
 *		decode is the main instruction code routine.  It calls 
 *		isearch_ppc to search the powerpc instruction table.	
 *		if no match is found the isearch is called.
 *
 *		The Bform, SCform, XLform, Dform, Iform, Xform, XFXform,
 *		XFLform, XOform, Aform, Mform, DSform routines correspond
 *		to each instruction format as defined in the assembler
 *		documentation.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

/* Instruction table search routine. */
/* isearch(instr,opcode,exop,format,xformat,AA,LK,SA,RC,OE,retstr) */
extern isearch(uint, uint, uint, uint *, uint *, uint, uint, uint, 
	uint, uint, char **);

extern arch_type;

#include <sys/types.h>
#include <sys/systemcfg.h>
#include "crash.h"

/*
 * NAME:  decode
 *
 * FUNCTION:  Given an instruction and address, this routine returns
 *	the disassembled instruction.
 *
 * in:     instr          machine language instruction
 *         addinst        address of instruction
 *
 * out:    sendstr        mnemonic string (disassembled instruction)
 */

decode(instr,addinst,sendstr)
char *sendstr;
unsigned long instr,addinst;
{
	unsigned long int opcode,exop,AA,LK,SA,RC,OE,format,xformat;
	int tmp;
	char *retstr;
	OE=9;   /*initialize to invalid value*/
	RC=9;   /*initialize to invalid value*/
	LK=9;   /*initialize to invalid value*/
	exop=9999; /*initialize to invalid value*/
	format=15; /*initialize to invalid value*/
	opcode=((instr>>26)&0x3F);
	if((opcode==16)||(opcode==18)) {
	     AA=((instr>>1)&0x1);
	}
	if(opcode==17) {
	     SA=((instr>>1)&0x1);
	}
	if((opcode==16)||(opcode==17)||(opcode==18)) {
	     LK=((instr)&0x1);
	}
	if((opcode==31)||(opcode==63)||(opcode==20)||(opcode==21)||(opcode==22)
	||(opcode==23)||(opcode==59)) {
	     RC=((instr)&0x1);
	}
/***************** powerpc start ********************/
	if((opcode==19)||(opcode==31)||(opcode==63)||(opcode==59)) {
	     exop=((instr>>1)&0x3FF); /*read bits 21-30*/
	     if((exop==16)||(exop==528)) {
		  LK=((instr)&0x1);
	     }
	     isearch_ppc(instr,opcode,exop,&format,&xformat,AA,LK,SA,RC,OE,&retstr);
	     if((format>11)&&(opcode==31)) { /*check for XO format*/
	       exop=(exop)&(0x1ff);      /*read bits 22-30*/
	       OE=((instr>>10)&0x1);
	       isearch_ppc(instr,opcode,exop,&format,&xformat,
		AA,LK,SA,RC,OE,&retstr);
	     }
	     if((format>11)&&((opcode==63)||(opcode==59))) {/*check for A format*/
	       exop=(exop)&(0x1f);        /*read bits 26-30*/
	       isearch_ppc(instr,opcode,exop,&format,&xformat,
		AA,LK,SA,RC,OE,&retstr);
	     }
	}
	else 
	    isearch_ppc(instr,opcode,exop,&format,&xformat,AA,LK,SA,RC,OE,&retstr);
/***************** powerpc end ********************/
        if (arch_type != POWER_PC) {	/* mfspr, mtspr, sync	*/
		if((opcode==31)&&((exop==467)||(exop==339)||(exop==598)))
			format=9999;
		if(opcode==17) 	/* sc	*/
			format=9999;
        }

if (format>11) {
/***************** power start ********************/
	if((opcode==19)||(opcode==31)||(opcode==63)) {
	     exop=((instr>>1)&0x3FF); /*read bits 21-30*/
	     if((exop==16)||(exop==528)) {
		  LK=((instr)&0x1);
	     }
	     isearch(instr,opcode,exop,&format,&xformat,AA,LK,SA,RC,OE,&retstr);
	     if((format>11)&&(opcode==31)) { /*check for XO format*/
	       exop=(exop)&(0x1ff);      /*read bits 22-30*/
	       OE=((instr>>10)&0x1);
	       isearch(instr,opcode,exop,&format,&xformat,
		AA,LK,SA,RC,OE,&retstr);
	     }
	     if((format>11)&&(opcode==63)) {/*check for A format*/
	       exop=(exop)&(0x1f);        /*read bits 26-30*/
	       isearch(instr,opcode,exop,&format,&xformat,
		AA,LK,SA,RC,OE,&retstr);
	     }
	}
	else 
	    isearch(instr,opcode,exop,&format,&xformat,AA,LK,SA,RC,OE,&retstr);
/***************** power end ********************/
}
	switch(format) {
	     case 0:
		  Dform(instr,xformat,retstr,&sendstr);
		  break;
	     case 1:
		  Bform(instr,opcode,AA,LK,addinst,retstr,&sendstr);
		  break;
	     case 2:
		  Iform(instr,opcode,AA,LK,addinst,retstr,&sendstr);
		  break;
	     case 3:
		  SCform(instr,opcode,SA,LK,retstr,&sendstr,xformat);
		  break;
	     case 4:
		  Xform(instr,opcode,exop,xformat,RC,retstr,&sendstr);
		  break;
	     case 5:
		  XLform(instr,opcode,exop,xformat,LK,retstr,&sendstr);
		  break;
	     case 6:
		  XFXform(instr,opcode,exop,retstr,&sendstr,xformat,&format);
		  break;
	     case 7:
		  XFLform(instr,opcode,exop,RC,retstr,&sendstr);
		  break;
	     case 8:
		  XOform(instr,opcode,exop,xformat,OE,RC,retstr,&sendstr);
		  break;
	     case 9:
		  Aform(instr,opcode,exop,xformat,RC,retstr,&sendstr);
		  break;
	     case 10:
		  Mform(instr,opcode,RC,retstr,&sendstr,xformat);
		  break;
	     case 11:
		  DSform(instr,xformat,retstr,&sendstr);
		  break;
	     default:
		  sprintf(sendstr,"%s", IVD_INST);
		  break;
	}
}


Dform(instr,xformat,retstr,sendstr)
unsigned long int instr;
char *retstr,**sendstr;
{
	int RT,RA,TO,BF,FRT,field;

	RT=(instr>>21)&(0x1F);
	RA=(instr>>16)&(0x1F);
	field=(instr)&(0xFFFF);
	BF=(instr>>23)&(0x7);
	TO=RT;
	FRT=RT;
	switch(xformat) {
	     case 0:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x",retstr,RT,RA,field);
		  break;
	     case 1:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x",retstr,RA,RT,field);
		  break;
	     case 2:
		  sprintf(*sendstr,"%8s   r%d,0x%x(r%d)",retstr,RT,field,RA);
		  break;
	     case 3:
		  sprintf(*sendstr,"%8s   %d,r%d,0x%x",retstr,TO,RA,field);
		  break;
	     case 4:
		  sprintf(*sendstr,"%8s   cr%d,r%d,0x%x",retstr,BF,RA,field);
		  break;
	     case 5:
		  sprintf(*sendstr,"%8s   fr%d,0x%x(r%d)",retstr,FRT,field,RA);
		  break;
	     default:
		  sprintf(*sendstr,"invalid xformat encountered");
		  break;
	}
}

DSform(instr,xformat,retstr,sendstr)
unsigned long int instr;
char *retstr,**sendstr;
{
	int RT,RA,TO,BF,FRT,field;

	RT=(instr>>21)&(0x1F);
	RA=(instr>>16)&(0x1F);
	field=(instr)&(0xFFFF);
	BF=(instr>>23)&(0x7);
	TO=RT;
	FRT=RT;
	switch(xformat) {
	     case 0:
		  sprintf(*sendstr,"%8s   fr%d,0x%x(r%d)",retstr,RT,field,RA);
		  break;
	     case 1:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x",retstr,RA,RT,field);
		  break;
	     case 2:
		  sprintf(*sendstr,"%8s   r%d,0x%x(r%d)",retstr,RT,field,RA);
		  break;
	     case 3:
		  sprintf(*sendstr,"%8s   %d,r%d,0x%x",retstr,TO,RA,field);
		  break;
	     case 4:
		  sprintf(*sendstr,"%8s   cr%d,r%d,0x%x",retstr,BF,RA,field);
		  break;
	     case 5:
		  sprintf(*sendstr,"%8s   fr%d,r%d,0x%x",retstr,FRT,RA,field);
		  break;
	     default:
		  sprintf(*sendstr,"invalid ds-format encountered");
		  break;
	}
}

Bform(instr,opcode,AA,LK,addinst,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int opcode,AA,LK,addinst;
long instr;
{
	long target;
	int BO,BI;

	/* strip off first 16 bits and sign extend */
	target=(((instr)&0xfffc) << 16) >> 16;
	if(AA==0) {		/* relative addressing */
	     target += addinst;
	}

	BO=(instr>>21)&(0x1F);
	BI=(instr>>16)&(0x1F);
	sprintf(*sendstr,"%8s   %x,%x,0x%x",retstr,BO,BI,target); 
}

Iform(instr,opcode,AA,LK,addinst,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int opcode,AA,LK,addinst;
long	instr;
{
	long target;

	/* strip off first 6 bits and sign extend */
	target=(((instr)&0x03fffffc) << 6) >> 6;
	if(AA==0) {		/* relative addressing */
	     target += addinst;

	}
	
	sprintf(*sendstr,"%8s   0x%x",retstr,target);
}

SCform(instr,opcode,SA,LK,retstr,sendstr,xformat)
char *retstr,**sendstr;
unsigned long int instr,opcode,SA,LK,xformat;
{
	unsigned long int LEV,FL1,FL2,SV;
	switch(xformat) {
	     case 0:
		{
		if(SA==0) {
		     LEV=((instr>>5)&0x7f);
	   	  FL1=((instr>>12)&0xf);
	    	 FL2=((instr>>2)&0x7);
	    	 sprintf(*sendstr,"%8s   0x%x,0x%x,0x%x",retstr,LEV,FL1,FL2);
	   	  return (0);
		}
		if(SA==1) {
		     SV=((instr>>2)&0x3fff);
	    		 sprintf(*sendstr,"%8s   0x%x",retstr,SV);
	    	 /* if ip bit is set,branch is to ros. otherwise it is to 0x1fe0 */
			return ( (long) 0);
		} 
		}
		  break;
	     case 1:
		  sprintf(*sendstr,"%8s",retstr);
			return ( (long) -1);
		  break;
	     default:	
	     sprintf(*sendstr,"invalid SC-format encountered");
		break;
	}
}


Xform(instr,opcode,exop,xformat,RC,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,xformat,RC;
{
	unsigned long int RT,RA,RB,NB,SH,FRS,SPR,FRT,BF,BFA,I,BT,SR,FRA,FRB,TO;

	RT=(instr>>21)&(0x1f);
	RA=(instr>>16)&(0x1f);
	RB=(instr>>11)&(0x1f);
	BF=(instr>>23)&(0x7);
	BFA=(instr>>18)&(0x7);
	I=(instr>>21)&(0xf);
	FRS=RT;
	FRT=RT;
	FRA=RA;
	FRB=RB;
	BT=RT;
	NB=RB;
	SH=RB;
	SPR=RA;
	SR=RA;
	TO=RT;
	switch(xformat) {
	     case 1:
		  sprintf(*sendstr,"%8s",retstr);
		  break;
	     case 2:
		  sprintf(*sendstr,"%8s   r%d",retstr,RT);
		  break;
	     case 3:
		  sprintf(*sendstr,"%8s   %d,r%d",retstr,SPR,RT);
		  break;
	     case 4:
		  sprintf(*sendstr,"%8s   r%d,r%d",retstr,RT,RB);
		  break;
	     case 5:
		  sprintf(*sendstr,"%8s   r%d,r%d",retstr,RT,RA);
		  break;
	     case 6:
		  sprintf(*sendstr,"%8s   r%d,r%d",retstr,RA,RB);
		  break;
	     case 7:
		  sprintf(*sendstr,"%8s   r%d,r%d,r%d",retstr,RT,RA,RB);
		  break;
	     case 8:
		  sprintf(*sendstr,"%8s   r%d,r%d,r%d",retstr,RA,RT,RB);
		  break;
	     case 9:
		  sprintf(*sendstr,"%8s   r%d,r%d",retstr,RA,RT);
		  break;
	     case 10:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x",retstr,RT,RA,NB);
		  break;
	     case 11:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x",retstr,RA,RT,SH);
		  break;
	     case 12:
		  sprintf(*sendstr,"%8s   fr%d,r%d,r%d",retstr,FRS,RA,RB);
		  break;
	     case 13:
		  sprintf(*sendstr,"%8s   fr%d",retstr,FRT);
		  break;
	     case 14:
		  sprintf(*sendstr,"%8s   cr%d,0x%x",retstr,BF,BFA);
		  break;
	     case 15:
		  sprintf(*sendstr,"%8s   cr%d,0x%x",retstr,BF,I);
		  break;
	     case 16:
		  sprintf(*sendstr,"%8s   0x%x",retstr,BT);
		  break;
	     case 17:
		  sprintf(*sendstr,"%8s   0x%x,r%d",retstr,SR,RT);
		  break;
	     case 18:
		  sprintf(*sendstr,"%8s   r%d,0x%x",retstr,RT,SR);
		  break;
	     case 19:
		  sprintf(*sendstr,"%8s   cr%d,fr%d,fr%d",retstr,BF,FRA,FRB);
		  break;
	     case 20:
		  sprintf(*sendstr,"%8s   cr%dx,fr%d",retstr,BF,FRB);
		  break;
	     case 21:
		  sprintf(*sendstr,"%8s   fr%d,fr%d",retstr,FRT,FRB);
		  break;
	     case 22:
		  sprintf(*sendstr,"%8s   cr%d,r%d,r%d",retstr,BF,RA,RB);
		  break;
	     case 23:
		  sprintf(*sendstr,"%8s   cr%d",retstr,BF);
		  break;
	     case 24:
		  sprintf(*sendstr,"%8s   %d,r%d,r%d",retstr,TO,RA,RB);
		  break;
	     case 26:
		  sprintf(*sendstr,"%8s   r%d",retstr,RB);
		  break;
	     case 27:
		  sprintf(*sendstr,"%8s   r%d,%d",retstr,RT,SPR);
		  break;
	     default:
		  sprintf(*sendstr,"invalid xformat encountered");
		  break;
	}
}


XLform(instr,opcode,exop,xformat,LK,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,xformat,LK;
{
	unsigned long int BO,BI,BB,BF,BFA,BT,BA;

	switch(xformat) {
	     case 0:
		sprintf(*sendstr,"%8s",retstr);
		break;
	     case 1:
		BO=((instr>>21)&0x1f);
		BI=((instr>>16)&0x1f);
		sprintf(*sendstr,"%8s   cr%d,%x",retstr,BO,BI);
		break;
	     case 2:
		BF=((instr>>23)&0x7);
		BFA=((instr>>18)&0x7);
		sprintf(*sendstr,"%8s   cr%d,cr%d",retstr,BF,BFA);
		break;
	     case 3:
		BT=((instr>>21)&0x1f);
		BA=((instr>>16)&0x1f);
		BB=((instr>>11)&0x1f);
		sprintf(*sendstr,"%8s   cr%d,cr%d,cr%d",retstr,BT,BA,BB);
		break;
	     default:
		sprintf(*sendstr,"invalid xl-format encountered");
		break;
	}
}


XFXform(instr,opcode,exop,retstr,sendstr,xformat)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,xformat;
{
	unsigned long int RT,FXM,TRB;

	RT=((instr>>21)&0x1f);
	switch(xformat) {
	case 0:
		FXM=((instr>>12)&0xff);
		sprintf(*sendstr,"%8s   fxm%d,r%d",retstr,FXM,RT);
		break;
	case 1:
		FXM=( ((instr>>6)&0x3e0) | ((instr>>16)&0x1f) );
		sprintf(*sendstr,"%8s   %d,r%d",retstr,FXM,RT);
		break;
	case 2:
		FXM=( ((instr>>6)&0x3e0) | ((instr>>16)&0x1f) );
		sprintf(*sendstr,"%8s   r%d,%d",retstr,RT,FXM);
		break;
	case 3:
		TRB=( ((instr>>6)&0x3e0) | ((instr>>16)&0x1f) );
		if(TRB==268)
		sprintf(*sendstr,"%8s   r%d,TB",retstr,RT);
		else 
		if(TRB==269)
			sprintf(*sendstr,"%8s   r%d,TBU",retstr,RT);
		else 
		  	sprintf(*sendstr,"invalid xformat encountered");
		break;
	default:
		sprintf(*sendstr,"invalid xfx-format encountered");
		break;
	}
}


XFLform(instr,opcode,exop,RC,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,RC;
{
	unsigned long int FLM,FRB;

	FLM=((instr>>17)&0xff);
	FRB=((instr>>11)&0x1f);
	sprintf(*sendstr,"%8s   flm%d,fr%d",retstr,FLM,FRB);
}


XOform(instr,opcode,exop,xformat,OE,RC,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,xformat,OE,RC;
{
	unsigned long int RT,RA,RB;

	RT=((instr>>21)&0x1f);
	RA=((instr>>16)&0x1f);
	switch(xformat) {
	case 0:
	     sprintf(*sendstr,"%8s   r%d,r%d",retstr,RT,RA);
	     break;
	case 1:
	     RB=((instr>>11)&0x1f);
	     sprintf(*sendstr,"%8s   r%d,r%d,r%d",retstr,RT,RA,RB);
	     break;
	default:
	     sprintf(*sendstr,"invalid xo-format encountered");
	     break;
	}
}


Aform(instr,opcode,exop,xformat,RC,retstr,sendstr)
char *retstr,**sendstr;
unsigned long int instr,opcode,exop,xformat,RC;
{ 
	int FRT,FRA,FRB,FRC;
	
	FRT=((instr>>21)&(0x1f));
	FRA=((instr>>16)&(0x1f));
	FRB=((instr>>11)&(0x1f));
	FRC=((instr>>6)&(0x1f));
	switch(xformat) {
	     case 0:
		  sprintf(*sendstr,"%8s   fr%d,fr%d,fr%d",retstr,FRT,FRA,FRB);
		  break;
	     case 1:
		  sprintf(*sendstr,"%8s   fr%d,fr%d,fr%d",retstr,FRT,FRA,FRC);
		  break;
	     case 2:
		  sprintf(*sendstr,"%8s   fr%d,fr%d,fr%d,fr%d",retstr,FRT,FRA,
	FRC,FRB);
		  break;
	     case 3:
		  sprintf(*sendstr,"%8s   fr%d,fr%d",retstr,FRT,FRB);
		  break;
	     default:
		  sprintf(*sendstr,"invalid a-format encountered");
		  break;
	}
}


Mform(instr,opcode,RC,retstr,sendstr,xformat)
char *retstr,**sendstr;
unsigned long int instr,opcode,RC,xformat;
{
	unsigned long int RS,RA,SH,MB,ME,RB;
	
	RS=((instr>>21)&0x1f);
	RA=((instr>>16)&0x1f);
	SH=((instr>>11)&0x1f);
	MB=((instr>>6)&0x1f);
	ME=((instr>>1)&0x1f);
	RB=SH;
	switch(xformat) {
	     case 0:
		  sprintf(*sendstr,"%8s   r%d,r%d,r%d,%d,%d",retstr,RA,RS,
			RB,MB,ME);
		  break;
	     case 1:
		  sprintf(*sendstr,"%8s   r%d,r%d,0x%x,%d,%d",retstr,RA,RS,
			SH,MB,ME);
		  break;
	     default:
		  sprintf(*sendstr,"invalid m-format encountered");
		  break;
	}
}
