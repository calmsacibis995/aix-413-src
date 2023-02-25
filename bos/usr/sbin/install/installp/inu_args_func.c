static char sccsid[] = "@(#)87  1.19  src/bos/usr/sbin/install/installp/inu_args_func.c, cmdinstl, bos41J, 9521B_all 5/26/95 16:50:12";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: inu_get_list, correct_usage (installp)
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <installp.h>
#include <instl_options.h>

/*
 * NAME: inu_args_func
 *
 * FUNCTION: This is a group of functions used by inu_instl_args.c
 *
 */

/*----------------------------------------------------------------------------
 * NAME: inu_get_list (infile, outfile, err_count)
 *
 * FUNCTION: determines if the file passed in was from a previous -l operation
 *           and places the input into a /tmp/inu_list.
 *
 * RETURNS: FAILURE if the user specified file or output file was not found
 *          SUCCESS if it was successful
 *-------------------------------------------------------------------------*/

int inu_get_list (
char *infile,         /* the input file given with -f option */
char *outfile,        /* the output file for the lpp list */
int  err_count)       /* count of invalid lpps found */

{
    FILE *ifp, *ofp;            /* pointers to the input and output files */
    int  rc;                    /* the return code for different func calls */
    char Line[MAX_LPP_FULLNAME_LEN + MAX_LEVEL_LEN + 2];
                                /* string containing one line of input */
    char LppName[MAX_LPP_FULLNAME_LEN];
                                /* string containing the lppname read in */
    char Level[MAX_LEVEL_LEN];  /* string containing the level of the lpp */

    /* initialize all the variables */
    memset (Line, '\0', sizeof (Line));
    memset (LppName, '\0', sizeof (LppName));
    memset (Level, '\0', sizeof (Level));
    rc = 0;
    ifp = NIL (FILE);
    ofp = NIL (FILE);

    /* open the input and output files */
    if (strcmp (infile, "-") == 0)  /* if the input file is a "-" then */
        ifp = stdin;
    else if ((ifp = fopen (infile, "r")) == NULL)
    {
        err_count++;
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                     CMN_CANT_OPEN_D), "installp", USER_FILE);
        return (FAILURE);
    } /* end else (fopen error) */

    if ((ofp = fopen (outfile, "w")) == NULL) {  /* open the outfile */
        fclose (ifp);
        err_count++;
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                     CMN_CANT_OPEN_D), "installp", LPPLIST);
        return (FAILURE);
    }

    /* reading the first line */
    if ((fgets (Line, (MAX_LPP_FULLNAME_LEN+MAX_LEVEL_LEN+2), ifp)) == NULL) {
        err_count++;
        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                     CMN_CANT_OPEN_D), "installp", USER_FILE);
        return (FAILURE);
    }

    if (strstr (Line, MSGSTR (MS_INSTALLP, INP_LPP_LABEL_I, 
	INP_LPP_LABEL_D)) != NULL) {
        /* is the input from the -l output */

        /* if it is, then skip next line */
        if ((fgets (Line, MAX_LPP_FULLNAME_LEN, ifp)) == NULL) {
            fclose (ifp);
            fclose (ofp);
            err_count++;
            instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                 CMN_CANT_OPEN_D), "installp", USER_FILE);
            return (FAILURE);
        } /* end if (the line is part of the title) */
    }
    else { /* process the line already read in */

	if (Line[0] != '#') {  /* if this line not commented */

	    rc = sscanf (Line, "%s %s", LppName, Level);

	    if (rc != EOF) {          /* not a blank line */

		if (rc == 0) {        /* an error occurred */
		    fclose (ifp);
		    fclose (ofp);
		    err_count++;
                    instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                       CMN_CANT_OPEN_D), "installp", USER_FILE);
		    return (FAILURE);
		}

		if (Level[0] != '\0' && Level[0] != '#') 
                {    /* level found? */
		    if (isdigit (Level[0])) /* valid level */
			fprintf (ofp, "%s %s\n", LppName, Level);

		    else { /* invalid level */
			fprintf (ofp, "%s\n", LppName);
                        instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                             CMN_LEVEL_NUM_E, CMN_LEVEL_NUM_D), 
                                             INU_INSTALLP_CMD, Level);
			err_count++;
		    } /* end else (invalid level) */
		}

		else { /* level not found */
				fprintf (ofp, "%s\n", LppName);
		}

	    } /* end if not blank line */

	    memset (Line, '\0', sizeof (Line));
	    memset (LppName, '\0', sizeof (LppName));
	    memset (Level, '\0', sizeof (Level));

	} /* end if not commented */
    } /* end else process line read in */

    while ((fgets (Line, MAX_LPP_FULLNAME_LEN+MAX_LEVEL_LEN+2, ifp)) != NULL) {

	if (Line[0] != '#') {       /* skip commented lines */

	    rc = sscanf (Line, "%s %s", LppName, Level);

	    if (rc != EOF) {        /* skip blank lines */

		if (rc == 0) {            /* an error occurred */
		    fclose (ifp);
		    fclose (ofp);
		    err_count++;
		    instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_CANT_OPEN_E, 
                                       CMN_CANT_OPEN_D), "installp", USER_FILE);
		    return (FAILURE);
		}

                if (Level[0] != '\0' && Level[0] != '#')
                {    /* level found? */
		    if (isdigit (Level[0])) /* valid level */
			fprintf (ofp, "%s %s\n", LppName, Level);

		    else { /* invalid level */
			fprintf (ofp, "%s\n", LppName);
			instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
                                             CMN_LEVEL_NUM_E, CMN_LEVEL_NUM_D),
                                             INU_INSTALLP_CMD, Level);
			err_count++;
		    } /* end else (invalid level) */
		}

		else { /* level not found */
				fprintf (ofp, "%s\n", LppName);
		}
	    } /* end if not blank line */
	} /* end if not commented */

       memset (Line, '\0', sizeof (Line));
       memset (LppName, '\0', sizeof (LppName));
       memset (Level, '\0', sizeof (Level));

    } /* end while (read the input to eof) */

    fclose (ifp); /* close the input file */
    fclose (ofp); /* close the output file */

    return (SUCCESS);

} /* end inu_get_list () */

/*--------------------------------------------------------------------------
 * NAME: correct_usage ()
 *
 * FUNCTION: This function will parse the flag structure available
 *           to create a  reasonable correct usage message
 *
 * RETURNS:
 *    stderr   the correct usage messages
 *    INUGOOD  if a match was made and a message was output
 *    INUINVAL if there was no match made and no error message
 *------------------------------------------------------------------------*/

int correct_usage ()
{
   if (a_FLAG && c_FLAG) 
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_C_I, 
                                                INP_USAGE_A_C_D));
   else if (a_FLAG) 
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_AC_I, 
                                                INP_USAGE_A_AC_D));
   else if (c_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_CO_I, 
                                                INP_USAGE_CO_D));
   else if (r_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_R_I, 
                                                INP_USAGE_R_D));
   else if (u_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_U_I, 
                                                INP_USAGE_U_D));
   else if (A_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_AL_I, 
                                                INP_USAGE_AL_D));
   else if (i_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_I_I, 
                                                INP_USAGE_I_D));
   else if (l_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_L_I, 
                                                INP_USAGE_L_D));
   else if (s_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_S_I, 
                                                INP_USAGE_S_D));
   else if (C_FLAG)
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_CL_I, 
                                                INP_USAGE_CL_D));
   else
   {
      /* else no major function specified, so print out full usage */
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_AC_I, 
                                                INP_USAGE_A_AC_D));
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_C_R_U_I, 
                                                INP_USAGE_C_R_U_D));
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_CL_I, 
                                                INP_USAGE_CL_D));
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_L_I, 
                                                INP_USAGE_L_D));
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_S_I, 
                                                INP_USAGE_S_D));
      instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_USAGE_A_I_I, 
                                                INP_USAGE_A_I_D));

      return (INUINVAL);
   }

   return (INUGOOD);

} /* end correct_usage */
