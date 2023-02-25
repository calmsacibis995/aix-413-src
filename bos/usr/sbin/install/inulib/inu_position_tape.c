static char sccsid[] = "@(#)70  1.2.1.14  src/bos/usr/sbin/install/inulib/inu_position_tape.c, cmdinstl, bos411, 9428A410j 6/30/94 08:43:11";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_position_tape
 *		figure_current_tape_position
 *		inu_perror
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <inulib.h>
#include <inu_dev_ctl.h>
#include <fcntl.h>
#include <limits.h>


static void inu_perror(char * msg)
{
	char *errmsg;
	errmsg=strerror(errno);
	inu_msg(FAIL_MSG,"%s: %s\n",msg,errmsg);
}

extern int current_vol;
extern int current_off;
int force_abs_seek=0;

static char * inu_tmpdir;
static void figure_current_tape_position()
{
	char file_path[PATH_MAX+1];

	active_tape_restore();
	if((inu_tmpdir=getenv("INUTEMPDIR"))==NULL)
		return;

	sprintf(file_path,"%s/%s",inu_tmpdir,"UNKNOWN_TAPE_POSITION");
	if(access(file_path,R_OK)==0)
	{
		unlink(file_path);
		force_abs_seek=1;
		current_off=0;
		return;
	}

	sprintf(file_path,"%s/%s",inu_tmpdir,"INU_TAPE_REWIND_DONE");
	if(access(file_path,R_OK)==0)
	{
		unlink(file_path);
		current_off=0;
	}

	sprintf(file_path,"%s/%s",inu_tmpdir,"INU_NEW_TAPE_POSITION");
	if(access(file_path,R_OK)==0)
	{
		FILE *fd;
		fd=fopen(file_path,"r");
		if(fd==NULL)
		{
			unlink(file_path);
			force_abs_seek=1;
			return;
		}
		if(fscanf(fd,"%d",&current_off)==0 || current_off == 0)
			force_abs_seek=1;
		fclose(fd);
		unlink(file_path);
	}
	return;
}


int inu_position_tape(Option_t *sop)
{
	int tape;
	int seek_type=TSEEK_ABS;
	int seek=0;
        int old_current_vol=current_vol;

	figure_current_tape_position();
	
        /* Don't do a seek when it is not necessary */
        if(current_vol==sop->bff->vol && current_off == sop->bff->off)
           return(0);

        if(current_vol==sop->bff->vol)
	{
		if((force_abs_seek) || (current_off > sop->bff->off))
		{
			seek_type=TSEEK_ABS;
			seek=sop->bff->off-1;
			force_abs_seek=0;
		}
		else 
		{
			seek_type=TSEEK_REL;
			seek = sop->bff->off - current_off;
		}
	}

	tape=topen(sop->bff->path,sop->bff->vol,O_RDONLY, FALSE);

        /* get_vol_id () leaves new tape on 4th bff */
        if ( old_current_vol != current_vol)
           {
              seek_type=TSEEK_REL;
              seek=sop->bff->off - current_off;
           }

        if(current_off != sop->bff->off )   /* could do seek != 0 */
	   if(tseek_file(tape,seek,seek_type)!=0)
	   {
		inu_perror(sop->bff->path);
		tclose(tape);
		force_abs_seek=1;
		current_off=0;
		return(1);
	   }

	tclose(tape);

	current_off=sop->bff->off;
	return(0);

}

