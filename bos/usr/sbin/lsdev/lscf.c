static char sccsid[] = "@(#)85	1.17  src/bos/usr/sbin/lsdev/lscf.c, cmdcfg, bos41J, 9520A_all 4/27/95 13:26:32";
/*
 *   COMPONENT_NAME: CMDCFG
 *
 *   FUNCTIONS: derive_cu_from_pd
 *		derive_pd_from_conn
 *		get_at_desc
 *		get_attrs
 *		get_conn
 *		get_cu_dv
 *		get_dev_desc
 *		get_par_cu_dv
 *		get_pdat
 *		get_pd_dv
 *		getallattrs
 *		initialize_cfgodm
 *		terminate_cfgodm
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <ctype.h>

#ifdef MSG
#include <nl_types.h>
#define DEFAULT_DEVICES_CAT_DIR		"/usr/lib/methods"
#define DEFAULT_DEVICES_CAT			"devices.cat"

nl_catd cat_fd;
char msg_cat[80];
#endif

#define DEFAULT_MSG_STR					"N/A"

#include "hlcmds.h"
#include "lscfconst.h"
extern char *cmdname;

char *malloc();
char *realloc();

int Cflag = FALSE;		/* -C option was found */
int Fflag = FALSE;		/* -F option was found */
int Pflag = FALSE;		/* -P option was found */
int hflag = FALSE;		/* -h option was found */
int rflag = FALSE;		/* -r option was found */
int print_hdr = FALSE;		/* -H option was found - print header */
int colon_fmt = FALSE;		/* -O option was found - colon format */
int read_from_stdin = FALSE;    /* - option was found */

int columnize = FALSE;		/* with -F, align values in columns */

char *entry_class = NULL;	/* -c option argument */
char *entry_state = NULL;	/* -S option argument */
char *entry_parent = NULL;	/* -p option argument */
char *entry_subclass = NULL;	/* -s option argument */
char *entry_key = NULL;		/* -k option argument */
char *entry_type = NULL;	/* -t option argument */
char *entry_name = NULL;	/* -l option argument */
char *entry_col = NULL;		/* -r option argument */
char *entry_attrs = NULL;	/* -a option argument */
char *entry_fmt = NULL;		/* -F option argument */

struct CuDv *cudv = NULL;	/* Customized device list */
struct listinfo cu_info;	/* CuDv info struct */

struct CuDv *parcudv = NULL;	/* Customized device list */
struct listinfo par_info;	/* CuDv info struct */

struct PdDv *pddv = NULL;	/* Predefined device list */
struct listinfo pd_info;	/* PdDv info struct */

struct PdCn *conn = NULL;	/* Predefined connection list */
struct listinfo conn_info;	/* PdCn info struct */
struct CombinedAttr {
		struct CuAt *cu;
 		struct PdAt *pd;
 		} combined_attr[MAX_ATTRS];   /* Attribute list */
 
int nattrs = 0;			/* number of attrs */

char msg[BUFSIZ];			/* buffer for strings and messages */

char *get_subclass(),*get_nls(),*strchr();
long strtol();

char **values = NULL;	/* unique values used by -r flag */
int nvalues = 0;		/* number of unique values */
 
char name_chars[BUFSIZ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz_0123456789";
int col_sz[MAX_FCOLS];

/*
 *   NAME:     initialize_cfgodm
 *   FUNCTION: Gets the config lock.
 *             Runs odm_initialize, but if the ODMDIR env variable
 *             is not set it uses /etc/objrepos instead of current dir
 *   RETURNS:  what was returned by odm_initialize
 */
initialize_cfgodm()
{
int rv;
	rv = odm_initialize();
	if(!getenv("ODMDIR"))
		odm_set_path("/etc/objrepos");
	return(rv);
}
/*
 *   NAME:     terminate_cfgodm
 *   FUNCTION: Releases the config lock.
 *             Runs odm_terminate
 *   RETURNS:  what was returned by odm_terminate
 */
terminate_cfgodm()
{
	return odm_terminate();
}

/*
 *   NAME:     get_dev_desc
 *   FUNCTION: return device description string
 *             
 */

char *
get_dev_desc(classp,input_obj)
struct Class *classp;
char *input_obj;
{
#ifdef MSG
int rc,msgno;
struct PdDv *pd;
struct CuDv *cu;
struct PdAt pdattr;
struct CuAt cuattr;
char sstr[256];
char *desc;
char cat[80];

	if(classp == CuDv_CLASS){
		cu = (struct CuDv *)input_obj;
		pd = cu->PdDvLn;
		}
	else{
		pd = (struct PdDv *)input_obj;
		}

	msgno = (int)pd->msgno;
	if(msgno == 0){
		/* No message number in PdDv so check for type T attribute */
		initialize_cfgodm();
		if(classp == CuDv_CLASS){
			sprintf(sstr,"name = '%s' AND type = T",cu->name);
			rc = (int)odm_get_first(CuAt_CLASS,sstr,&cuattr);
			if(rc != 0 && rc != -1)
				msgno = (int)strtol(cuattr.value,NULL,0);
			}

		/* if PdDv class of msgno is STILL 0, look for PdAt attr */
		if(msgno == 0){
			sprintf(sstr,"uniquetype = '%s' AND type = T",
								pd->uniquetype);
			rc = (int)odm_get_first(PdAt_CLASS,sstr,&pdattr);
			if(rc != 0 && rc != -1)
				msgno = (int)strtol(pdattr.deflt,NULL,0);
			}
		terminate_cfgodm();
		}

	/* is pd->catalog same as the one which is currently open ? */
	if (strcmp(pd->catalog,msg_cat))
	{	/* no - open the new catalog */
		catclose( cat_fd );

		strcpy(msg_cat,pd->catalog);

		cat_fd = catopen(msg_cat,NL_CAT_LOCALE);
	}

	/* attempt to get the message */
	desc = catgets( cat_fd, pd->setno, msgno, DEFAULT_MSG_STR );

	/* did we get something besides the default msg ? */
	if (! strcmp(desc,DEFAULT_MSG_STR))
	{	/* no - open the default catalog */
		catclose( cat_fd );

		if (msg_cat[0] != '\0')
			sprintf(cat,"%s/%s", DEFAULT_DEVICES_CAT_DIR, msg_cat);
		else
			sprintf(cat,"%s/%s", DEFAULT_DEVICES_CAT_DIR, DEFAULT_DEVICES_CAT );
		strcpy(msg_cat,cat);

		cat_fd = catopen( msg_cat, NL_CAT_LOCALE );

		/* attempt to get the message */
		desc = catgets( cat_fd, pd->setno, msgno, DEFAULT_MSG_STR );
	}

	return(desc);

#endif MSG

	return DEFAULT_MSG_STR;
}

/*
 *   NAME:     get_at_desc
 *   FUNCTION: return attribute description string
 *             
 */
char *
get_at_desc(pat)
struct PdAt *pat;
{
#ifdef MSG
char *desc;
char cat[80];
short setno;	    /* local set number variable */
char * catalog;     /* local catalog name variable */

	/* get the correct catalog for the attribute */
        if (!strcmp(pat->uniquetype,pddv->uniquetype)) {
		catalog = pddv->catalog;
		setno = pddv->setno;
	} 
	else { 
	/*  get the parent adapter catalog*/
		catalog = parcudv->PdDvLn->catalog;
		setno = parcudv->PdDvLn->setno;
	}

	/* is catalog same as the one which is currently open ? */
	if (strcmp(catalog,msg_cat))
	{	/* no - open the new catalog */
		catclose( cat_fd );

		strcpy(msg_cat,catalog);

		cat_fd = catopen(msg_cat,NL_CAT_LOCALE);
	}

	/* attempt to get the message */
	desc = catgets( cat_fd, setno, pat->nls_index, DEFAULT_MSG_STR );

	/* did we get something besides the default msg ? */
	if (! strcmp(desc,DEFAULT_MSG_STR))
	{	/* no - open the default catalog */
		catclose( cat_fd );

		if (msg_cat[0] != '\0')
			sprintf(cat,"%s/%s", DEFAULT_DEVICES_CAT_DIR, msg_cat);
		else
			sprintf(cat,"%s/%s", DEFAULT_DEVICES_CAT_DIR, DEFAULT_DEVICES_CAT );
		strcpy(msg_cat,cat);

		cat_fd = catopen( msg_cat, NL_CAT_LOCALE );

		/* attempt to get the message */
		desc = catgets( cat_fd, setno, pat->nls_index, DEFAULT_MSG_STR );
	}

	return(desc);

#endif MSG

	return DEFAULT_MSG_STR;
}

/*
 *   NAME:     get_cu_dv
 *   FUNCTION: get set of customized devices and linked predefined,
 *	       given possibly name,class,subclass,type, and state.
 *   GLOBALS CHANGED:cudv, cu_info
 *		 
 */
int
get_cu_dv(name,class,subclass,type,state)
char *name;
char *class;
char *subclass;
char *type;
char *state;
{
int i;
int rc = 0;
int istate;
char crit[256];
char tmp[256];

	crit[0] = '\0';

	if(type || class || subclass){
		sprintf(tmp,"PdDvLn LIKE '%s/%s/%s'",
			class ? class : "*",
			subclass ? subclass : "*",
			type ? type : "*"
			);
		strcat(crit,tmp);
		}
	if(name){
		if(crit[0])
			strcat(crit," AND ");
		sprintf(tmp,"name = '%s'",name);
		strcat(crit,tmp);
		}
	if(state){
		switch(state[0]){
			case 'd':
			case 'D':
			case '0':
				istate = DEFINED;
				break;
			case 'a':
			case 'A':
			case '1':
				istate = AVAILABLE;
				break;
			case 's':
			case 'S':
			case '2':
				istate = STOPPED;
				break;
			default:
				error1(CFG_MISC_SYNTAX_ERR,state);
			}
		if(crit[0])
			strcat(crit," AND ");
		sprintf(tmp,"status = %d",istate);
		strcat(crit,tmp);
		}

	cudv = (struct CuDv *) odm_get_list(CuDv_CLASS,
			crit,&cu_info,64,2);
	if((int)cudv == -1)
		error1(CFG_ODM_ACCESS,"CuDv");
	for(i=0;i<cu_info.num;i++)
		if(!cudv[i].PdDvLn)
		{
		   warning1(CFG_ODM_NOCUPD, cudv[i].name);
		   rc = -1;
	        }

	return(rc);
}

/*
 *   NAME:     get_par_cu_dv
 *   FUNCTION: get set of customized devices and linked predefined,
 *	       given the cudv.parent.
 *   GLOBALS CHANGED:parcudv, par_info
 *		 
 */
int
get_par_cu_dv(name)
char *name;
{
int i;
int rc = 0;
char crit[256];

	crit[0] = '\0';

	sprintf(crit,"name = '%s'",name); 
	parcudv = (struct CuDv *) odm_get_list(CuDv_CLASS,
			crit,&par_info,64,2);
	if((int)parcudv == -1)
		error1(CFG_ODM_ACCESS,"CuDv");
	for(i=0;i<par_info.num;i++)
		if(!parcudv[i].PdDvLn)
		{
		   warning1(CFG_ODM_NOCUPD, parcudv[i].name);
		   rc = -1;
	        }

	return(rc);
}

/*
 *   NAME:     get_pd_dv
 *   FUNCTION: get set of predefined devices
 *	       given possibly class, subclass, and/or type.
 *   GLOBALS CHANGED:pddv, pd_info
 *		 
 */
get_pd_dv(class,subclass,type)
char *class;
char *subclass;
char *type;
{
char crit[256];
char tmp[256];

	crit[0] = '\0';

	if(class){
		sprintf(tmp,"class = '%s'",class);
		strcat(crit,tmp);
		}
	if(subclass){
		if(crit[0])
			strcat(crit," AND ");
		sprintf(tmp,"subclass = '%s'",subclass);
		strcat(crit,tmp);
		}
	if(type){
		if(crit[0])
			strcat(crit," AND ");
		sprintf(tmp,"type = '%s'",type);
		strcat(crit,tmp);
		}
	pddv = (struct PdDv *)
			odm_get_list(PdDv_CLASS,crit,&pd_info,64,2);
	if((int)pddv == -1)
		error1(CFG_ODM_ACCESS, "PdDv");
}
/*
 *   NAME:     get_conn
 *   FUNCTION: get predefined connection objects
 *	       given possibly uniquetype and connkey.
 *   GLOBALS CHANGED:pddv, pd_info
 *		 
 */
get_conn(uniquetype,conntype)
char *uniquetype,*conntype;
{
char crit[BUFSIZE];
char temp[BUFSIZE];
	crit[0] = '\0';
	if(uniquetype){
		sprintf(crit,"uniquetype = '%s'", uniquetype);
		}
	if(conntype){
		if(strlen(crit))
			strcat(crit," AND ");
		sprintf(temp,"connkey = '%s'", conntype);
		strcat(crit,temp);
		}
	conn = (struct PdCn *)
		odm_get_list(PdCn_CLASS,crit,&conn_info,64,2);
	if((int)conn == -1)
		error1(CFG_ODM_ACCESS, "PdCn");
	return(conn_info.num);
}
/*
 *   NAME:     derive_pd_from_conn
 *   FUNCTION: get the set of parent predefined objects corresponding
 *             to a previously fetched list of predefined connections
 *   INPUT GLOBALS:
 *		conns          preexisting list of predefined conn objects
 *              conn_info.num  number of objects in conns
 *   OUTPUT GLOBALS:
 *		pddv           list of predefined device objects
 *              pddv_info.num  number of objects in  pddv
 *   NOTE:     have to avoid duplicates in the output list.  This is
 *             done by first using the rinit/rlist routines
 *		 
 */
derive_pd_from_conn()
{
int i;
char crit[BUFSIZE];
struct PdDv *tmp_pd;
struct listinfo tmp_info;
	pddv = NULL;
	pd_info.num = 0;
	rinit();
	for(i=0;i<conn_info.num;i++){
		rlist(PdCn_MODE,conn+i,"uniquetype");
		}
	for(i=0;i<nvalues;i++){
		sprintf(crit,"uniquetype = '%s'",values[i]);
		tmp_pd = (struct PdDv *)
			odm_get_list(PdDv_CLASS,crit,&tmp_info,1,1);
		if(!tmp_pd){
			error1(CFG_ODM_NOPDDV, "PdDv");
			}
		else if((int)tmp_pd == -1){
			error1(CFG_ODM_ACCESS,"PdDv");
			}
		if(!pddv){
			pddv = tmp_pd;
			pd_info.num = tmp_info.num;
			}
		else {
			pddv =  (struct PdDv *)
			    realloc(pddv,
			     (pd_info.num+tmp_info.num)*sizeof(struct PdDv));
			bcopy(tmp_pd,
				pddv + pd_info.num,
				tmp_info.num*sizeof(struct PdDv));
			pd_info.num += tmp_info.num;
			}
		}
}

/*
 *   NAME:     derive_cu_from_pd
 *   FUNCTION: get the set of customized devices which are instances of
 *             a given list of predefined objects
 *   INPUT GLOBALS:
 *		pddv           list of predefined device objects
 *              pddv_info.num  number of objects in  pddv
 *   OUTPUT GLOBALS:
 *		cudv          preexisting list of cust device objects
 *              cudv_info.num  number of objects in cudv
 *		 
 */
derive_cu_from_pd()
{
int i;
char crit[BUFSIZE];
struct CuDv *tmp_cu;
struct listinfo tmp_info;
	cudv = NULL;
	cu_info.num = 0;
	for(i=0;i<pd_info.num;i++){
		sprintf(crit,"PdDvLn = '%s'",pddv[i].uniquetype);
		tmp_cu = (struct CuDv *)
			odm_get_list(CuDv_CLASS,crit,&tmp_info,1,2);
		if(!tmp_cu)
			continue;
		else if((int)tmp_cu == -1){
			error1(CFG_ODM_ACCESS, "CuDv");
			}
		if(!cudv){
			cudv = tmp_cu;
			cu_info.num = tmp_info.num;
			}
		else {
			cudv =  (struct CuDv *) realloc(
			      cudv,
			      (cu_info.num + tmp_info.num)
					* sizeof(struct CuDv));
			bcopy(tmp_cu,cudv + cu_info.num,
			      tmp_info.num * sizeof (struct CuDv));

			cu_info.num += tmp_info.num;
			}
		}
}


/*
 *   NAME:     get_attrs
 *   FUNCTION: create an array of combined_attr structures, with one
 *             element for each named attribute in the attrs input string.
 *   NOTE:     The combined attr structure has a pointer to a customized
 *             attribute and its customized attribute. We first get the
 *             predefined attr and hook it into the combined_attr struct.
 *             If we are talking about a specific logical device, we have
 *             a non NULL name, so we get the customized attr for that
 *             device and hook it into the customized side.  
 *             Extension objects are ignored.
 */
int
get_attrs(attrs,name,uniquetype)
char *attrs;
char *name;
char *uniquetype;
{
int i,span;
int rc = 0;
char *cp;
char crit[1024];
char seperator[1024];
char attribute[1024];
struct CuAt *cat;
struct listinfo cat_info;
struct PdAt *pat;
struct listinfo pat_info;

	cp = attrs;
	while(*cp){
	        if(span = strcspn(cp,name_chars)){
			strncpy(seperator,cp,span);
			seperator[span] = '\0';
			cp += span;
			}
		if(span = strspn(cp,name_chars)){
			if(nattrs == MAX_ATTRS)
				error1(CFG_MISC_SYNTAX_ERR, "-a");
			strncpy(attribute,cp,span);
			attribute[span] = '\0';
			cp += span;

  			sprintf(crit,
  			    "uniquetype = '%s' AND attribute = '%s'",
  			    uniquetype,attribute);
  			pat = (struct PdAt *)
  				odm_get_list(PdAt_CLASS,
  					crit,&pat_info,1,1);
   
			if(!pat) {
				/* If attribute not found, look under the */
				/* parent for a type=E attribute          */
				if (parcudv != NULL) {
				     sprintf(crit,
                            	     "uniquetype='%s' AND attribute='%s' AND type='E'",
                                                 parcudv->PdDvLn_Lvalue,
                                                                 attribute);
                        		pat = (struct PdAt *)
                                		odm_get_list(PdAt_CLASS,
                                        		crit,&pat_info,1,1);
				}	
			}
			if(!pat) {

				warning1(CFG_ODM_NOPDAT, attribute);
				rc = -1;
				} 
				else
				{
				if((int)pat == -1){
					error1(CFG_ODM_ACCESS,
						"PdAt");
					}
				if(pat_info.num > 1){
					for(i=0;i<pat_info.num;i++)
						if(strchr(pat[i].generic,
							'E')){
						     pat += i; 
						     break;
						     }
					if(i==pat_info.num)
						error1(CFG_BAD_ATTR,
						     attribute);
					}
				if(!strchr(pat->generic,'D'))
					error1(CFG_DISPLAYABLE_ERR,
						pat->attribute);
				}

			combined_attr[nattrs].pd = pat;
		

			if(name){
				sprintf(crit,
			    	   "name = '%s' AND attribute = '%s'",
			   	    name,attribute);
				cat = (struct CuAt *)
				    odm_get_list(CuAt_CLASS,crit,&cat_info,1,1);
				if((int)cat == -1){
					error1(CFG_ODM_ACCESS,"CuAt");
					}
				if(cat_info.num > 1){
				    for(i=0;i<cat_info.num;i++)
					if(strchr(cat[i].generic,'E')){
					     cat += i; 
					     break;
					     }
				    if(i==cat_info.num)
					error1(CFG_BAD_ATTR,attribute);
				    }
				combined_attr[nattrs].cu = cat;
				}
			nattrs++;
			}
		}

	return(rc);
}

/*
 *   NAME:     getallattrs
 *   FUNCTION: create an array of combined_attr structures
 *             with an element for every predefined attribute of the
 *             given uniquetype.
 *   NOTE:     The combined attr structure has a pointer to a customized
 *             attribute and its predefined attribute. We first get the
 *             predefined attr and hook it into the combined_attr struct.
 *             If we are talking about a specific logical device, we have
 *             a non NULL name, so we get the customized attr for that
 *             device and hook it into the customized side.  
 *             Extension objects are ignored.
 */
getallattrs(name,uniquetype)
char *name,*uniquetype;
{
int i,j,k,num;
int rc=0;
char crit[1024];
struct CuAt *cat;
struct listinfo cat_info;
struct PdAt *pat;
struct listinfo pat_info;


	rc = get_pdat(name,uniquetype,NULL,&pat,&num);
	if (rc != 0)
		error1(CFG_ODM_ACCESS,"PdAt");
	
	pat_info.num = num;

	for(j=0,i=0;j<pat_info.num;j++)
	{
	/* skip extensions and not displayable */
		if(strchr(pat[j].generic,'e')|| !strchr(pat[j].generic,'D'))
			continue;
		combined_attr[i].pd = pat + j;
		if(!name)
			combined_attr[i].cu = NULL;
		else
		{
			sprintf(crit,"name = '%s' AND attribute = '%s'",
				    name,(pat+j)->attribute);
			cat = (struct CuAt *)odm_get_list(CuAt_CLASS,
				crit,&cat_info,1,1);
			if((int)cat == -1)
				error1(CFG_ODM_ACCESS,"CuAt");
			if(cat_info.num > 1)
			{
				i--;
				for(k = 0; k < cat_info.num; k++)
					combined_attr[++i].cu = cat++;
			}
			else
				combined_attr[i].cu = cat;
		}
		i++;
	}
	nattrs = i;
}

/*
 *   NAME:     get_pdat
 *   FUNCTION: Get the list of predefined attributes for a uniquetype,      
 *             the list of type=E attributes for the parent uniquetype,
 *             and return a combined list.
 */
int
get_pdat(name,uniquetype,attrs,pat_ptr,count)
char *name;
char *uniquetype;
char *attrs;
struct PdAt **pat_ptr;
int *count;
{
int nums = 0;
int rc = 0;
char crit[1024];

struct PdAt *pt;
struct listinfo pt_info;
struct PdAt *parpt;
struct listinfo parpt_info;
struct PdAt *total;

	/* for SMIT panel support, get all attributes regradless of type */
	if (colon_fmt == TRUE)
        	sprintf(crit,"uniquetype = '%s'",uniquetype ); 
	else
          	sprintf(crit,"uniquetype = '%s' AND type != 'E'",uniquetype); 
        pt = (struct PdAt *) odm_get_list(PdAt_CLASS,crit,&pt_info,1,1);
        if((int)pt == -1)
                error1(CFG_ODM_ACCESS,"PdAt");

        

	nums = pt_info.num;
 	/* get parent attributes of type=E */
 	if (parcudv != NULL ) {
        	sprintf(crit,"uniquetype = '%s' AND type = 'E'",parcudv->PdDvLn_Lvalue);
        	parpt = (struct PdAt *) odm_get_list(PdAt_CLASS,crit,&parpt_info,1,1);
        	if((int)parpt == -1)
                	error1(CFG_ODM_ACCESS,"PdAt");

		if (parpt_info.num > 0) { /* merge list */

		 	nums +=  parpt_info.num;
			total = (struct PdAt *) malloc(sizeof(struct PdAt)
				 * (nums));

	        	if (total == NULL) 
        	        	return(E_MALLOC);

			
 		       	/* copy attributes into a single list */
              		memcpy(&total[0],&pt[0],
				sizeof(struct PdAt) * pt_info.num);
			
              		memcpy(&total[pt_info.num],&parpt[0],
				sizeof(struct PdAt) * parpt_info.num);

			*pat_ptr = total;
        		if (pt) odm_free_list(pt,&pt_info);
                        if (parpt) odm_free_list(parpt, &parpt_info);
		}
	 	else {
			*pat_ptr = pt;
		}

	}
	else {
		*pat_ptr = pt;
	}
        *count = nums;

	return(0);
}
