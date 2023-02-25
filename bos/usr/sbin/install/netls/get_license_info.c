static char sccsid[] = "@(#)15  1.6  src/bos/usr/sbin/install/netls/get_license_info.c, cmdinstl, bos411, 9428A410j 6/16/94 14:21:08";
/*
 *   COMPONENT_NAME: cmdinstl
 *
 *   FUNCTIONS: cleanup
 *		get_netls_info
 *		get_vendor_products
 *		main
 *		process_state
 *		write_status
 *		usage
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*---------------------------------------------------------------------------*
 * NOTE - This program is not intended to be used by users directly.
 *       It is used by the SMIT/GUI install screens for various activities that
 *       are performed within the SMIT/GUI install environment.
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 * get_license_info
 *
 * This program is used to obtain a list of NetLS enabled products which have 
 * been registered with the server (s) on a network.
 *
 * Flags:
 *	-l		#long output
 *	-f file		# write status of execution to file. This file is
 *                      # removed when the program exits succesfully.
 *
 * To obtain a list of vendor id's, product id's and product version in a 
 * colon separated format, execute
 *
 * 		get_license_info
 *
 * To log the status of execution to a file for this output, execute
 *		get_license_info -f file
 * (particularly, to write a status code to file indicating whether or not 
 * program has finished -- used for synchronization between this program and 
 * caller)
 *
 * To obtain a list of vendor names, vendor id's, product name, product id's and
 * product versions in a colon separated format, execute
 *
 *		get_license_info -l
 *
 * To log the status of execution to a file for this output, execute
 *              get_license_info -l -f file
 *
 * Assumptions:
 * -- NetLS client code is installed on machine upon which program is running
 * -- machine is configured to access the server (s) on the network.
 *
 * MODULE DESIGN:
 *
 * - Parse the command line.
 * - Execute /usr/lib/netls/bin/ls_tv to get a list of servers on the network.
 *   Save this output in a file.
 * - For every server name obtained from the above action, execute 
 *   /usr/lib/netls/bin/ls_admin to get a list of vendors registered with the 
 *   server. Save this in a file.
 * - For every server-vendor combination, execute /usr/lib/netls/bin/ls_admin
 *   to get a list of products for the vendor registered with the server. Save
 *   this output in a file.
 * - Parse the output obtained above depending on the options to the program.
 *   Save the parsed output in a file.
 * - Sort the output depending on the options to the program.
 * 
 *---------------------------------------------------------------------------*/

#include <inulib.h>
#include <get_license_info.h>

/*
 * key search pattern used when parsing netls output.
 */
char active_srvrs_string[] = "Active iFOR/LS Servers:";

main (int argc, char *argv[])
{

   char   flag;                          /* command line option flag */
   char   netls_buf[MAXHOSTNAMELEN + 1];
   FILE   *vendor_file, * server_file;      /* File pointers            */
   char   sys_cmd [PATH_MAX * 2];           /* array to construct the system
                                               commands                 */
   extern char *optarg;                     /* pointer used by getopt () */
   extern int  optind;                      /* index used by getopt () */
   extern int  opterr;
   int    status, i, ls_tv_worked = FALSE;
   char   server_name [MAXHOSTNAMELEN + 1];
   int    server_found, rc;
   struct stat stat_buf;
   struct sigaction action;
   char    status_file[PATH_MAX + 1];          /* output of strcpy */
   size_t  length_of_server_string;


   strcpy (progname, argv[0]);
   setlocale (LC_ALL, "");

   CATOPEN ();

  /*--------------------------------------------------------------------*
   *  Set an alarm for 3 minutes as we only want this program to execute
   *  for 3 minutes. If it takes longer than that then that is an 
   *  indication of some problem with the network or netls.
   *---------------------------------------------------------------------*/

   alarm (SLEEP_TIME);

  /*-------------------------------------------------------------------*
   * Register a signal handler. This will delete all of the temporary
   * files used by this program on a SIGHUP and SIGALRM signal.
   *-------------------------------------------------------------------*/

   action.sa_handler = cleanup;
   action.sa_mask.losigs = 0;
   action.sa_mask.hisigs = 0;
   action.sa_flags = 0;
   rc = sigaction (SIGHUP, &action, (struct sigaction *) 0);
   rc = sigaction (SIGALRM, &action, (struct sigaction *) 0);

   errno = 0;
   long_output = FALSE;
   status_flag = FALSE;

   /*---------------------------------------------------------------*
    *  Parse the command line arguments. Check that a filename has
    *  been provided to this program.
    *---------------------------------------------------------------*/

      opterr = 0;
      while ((flag = (char) getopt (argc, argv, "lf:")) != (char) EOF) {
        switch (flag) {

            case 'f':   /* Netls Status File */
                 status_flag = TRUE;
                 strcpy (status_file, optarg);
                 if (status_file[0] == '\0') { /* then there was no argument */
                    inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, 
                                  GLI_NO_STATUS_FILE_E, GLI_NO_STATUS_FILE_D));
                    inu_msg (INFO_MSG, MSGSTR (MS_GET_LICENSE_INFO, GLI_USAGE_E,
                                               GLI_USAGE_D));
                    exit (NO_STATUS_FILE_NAME);
                 } /* end if (status_file[0] == '\0') */

               /*-----------------------------------------------------------*
                *  Open the Netls status file for writing. This file will
                *  contain the status codes for the various stages of the 
                *  Netls program. This will be needed by the sm_inst script
                *  to check if the get_license_info finished successfully.
                *  Write a code of "0" to this file. This code implies
                *  success of get_license_info.
                *-----------------------------------------------------------*/

                 if ((statusfile = fopen (status_file, "w")) == NULL)
                 {
                    perror (progname);
                    exit (STATUS_FILE_OPEN_ERROR);
                 }
                 fprintf (statusfile, "%d\n", EXECUTING_PROGRAM);
                 fflush (statusfile);

                 break;

            case 'l':  /* Long output */
                 long_output = TRUE;
                 break;

            default :    /* Catch all */
                inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, 
                              GLI_INVALID_ARGUMENT_E, GLI_INVALID_ARGUMENT_D));
                usage ();
                exit (1); 

         } /* end switch (flag) */

       } /* end while (getopt) */


   /*--------------------------------------------------------------------*
    *   Create a temporary file in /tmp to store the output of the NetLS
    *   function call "ls_tv". 
    *--------------------------------------------------------------------*/

      tmpnam (server_file_name);
      
   /*--------------------------------------------------------------------*
    *  Construct the command to be executed. Redirect the output to the
    *  temporary file created by the above call. This function call will
    *  return the names of all the servers on the network.
    *--------------------------------------------------------------------*/

      sprintf (sys_cmd, "LANG=C;/usr/lib/netls/bin/ls_tv > %s ", 
                                                     server_file_name);
      if ((status = MASK(system (sys_cmd))) != 0)
      {
         perror (progname);
         unlink (server_file_name);
         if (status_flag)
            write_status (LS_TV_NOT_FOUND);

         exit (LS_TV_NOT_FOUND);
      }
    
   /*--------------------------------------------------------------*
    *  Open the file just populated with the output of "ls_tv"
    *  for reading. 
    *--------------------------------------------------------------*/

      if ((server_file = fopen (server_file_name, "r")) == NULL)
      {
         perror (progname);
         if (status_flag)
            write_status (SERVER_FILE_OPEN_ERROR);

         exit (SERVER_FILE_OPEN_ERROR);
      }


  /*----------------------------------------------------------------------*
   *  Read each line until we come across the words "Active iFOR/LS Servers:"
   *  We can now access the names of the servers on the network. They appear
   *  one name on each line. 
   *----------------------------------------------------------------------*/

   length_of_server_string = sizeof (active_srvrs_string);
   while (fgets (netls_buf, MAXHOSTNAMELEN + 1, server_file) != NULL)
   {
      if (strncmp (netls_buf, active_srvrs_string,
                   length_of_server_string -1) == 0)
      {
          ls_tv_worked = TRUE;
          break;
      }
   }  /**** end of while ****/

   server_found = FALSE;
   if ( ! ls_tv_worked)
   {
      inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, GLI_LS_TV_ERROR_E, 
                             GLI_LS_TV_ERROR_D));
      unlink (server_file_name);
      if (status_flag)
         write_status (NO_SERVERS_FOUND);

      exit (NO_SERVERS_FOUND);
   }
   else
   {
      /*-------------------------------------------------------------------*
       *  Ok ! We can now access the server names. For each server get a
       *  list of vendors which have products registered with that server.
       *  Use ls_admin -s -n < server_name> -v. Save this output in a 
       *  file for later use.
       *-------------------------------------------------------------------*/

      /*----------------------------------------------------------*
       *  Create a temporary file to store the result of ls_admin
       *----------------------------------------------------------*/

       tmpnam (vendor_file_name);
       tmpnam (netls_file_name);

      /*-----------------------------------------------*
       *    Read a server name from the server file.
       *-----------------------------------------------*/

       while (fgets (netls_buf, MAXHOSTNAMELEN + 1, server_file) != NULL)
       {
         sscanf (netls_buf, "%s", server_name);
         server_found = TRUE;
         
        /*----------------------------------------------------------------*
         *  Execute ls_admin for each server in the server file to get the
         *  names of the vendors registered with that server. Save this 
         *  output in the vendor file.
         *----------------------------------------------------------------*/

          sprintf (sys_cmd, "LANG=C;/usr/lib/netls/bin/ls_admin -n %s -s -v | /usr/bin/grep \"Vendor:\" | /usr/bin/awk -F: '{ print SERVER\":\"substr ($2, 2) }' SERVER=%s >> %s ", 
                   server_name, server_name, vendor_file_name);     

          if ((status = MASK(system(sys_cmd))) != 0)
          {
             perror (progname);
             cleanup ();
             if (status_flag)
                write_status (LS_ADMIN_NOT_FOUND);

             exit (LS_ADMIN_NOT_FOUND);
          }

       }  /*** end while (fgets...) ***/

       if ( ! server_found)
       {
          inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, 
                     GLI_NO_SERVER_NAMES_FOUND_E, GLI_NO_SERVER_NAMES_FOUND_D));

          cleanup ();
          if (status_flag)
             write_status (NO_SERVERS_FOUND);

          exit (NO_SERVERS_FOUND);
       }


       /*-----------------------------------------------------------------*
        *  Check if the vendor file is empty. If it is then we know that 
        *  the call to ls_admin failed for all the available servers.
        *-----------------------------------------------------------------*/

       if (stat (vendor_file_name, &stat_buf) < 0) 
       {
          perror (progname);
          cleanup ();
          if (status_flag)
             write_status (STAT_CALL_ERROR);

          exit (STAT_CALL_ERROR);
       }
       if (stat_buf.st_size <= 0)
       {
          inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, 
                             GLI_LS_ADMIN_FAILED_E, GLI_LS_ADMIN_FAILED_D));
          cleanup ();
          if (status_flag)
             write_status (STAT_CALL_ERROR);

          exit (STAT_CALL_ERROR);
       }

      /*--------------------------------------------------------------*
       *  Open the file just populated with the output of "ls_admin"
       *  for reading. This file has the server and vendor names.
       *--------------------------------------------------------------*/

       if ((vendor_file = fopen (vendor_file_name, "r")) == NULL)
       {
           perror (progname);
           if (status_flag)
              write_status (VENDOR_FILE_OPEN_ERROR);

           exit (VENDOR_FILE_OPEN_ERROR);
       } 

      /*--------------------------------------------------------------*
       *  For every entry in the vendor file get the products for the
       *  vendor by using ls_admin -s -n<server> -p<vendor>. From this
       *  output we can get the vendor id, product id and version.
       *--------------------------------------------------------------*/
            
       get_vendor_products (vendor_file);

       /*-------------------------------------------------------------------*
        *  Parse the output from the file pointed to by netls_file to get
        *  vendor id, product id and versions. Save this output in a file.
        *-------------------------------------------------------------------*/

       get_netls_info ();

       /*-----------------------------------------------------------------*
        *  Sort the file obtained from the call to get_netls_info. This
        *  is done to avoid multiple entries in the output which might
        *  arise if the vendors/products are registered with multiple
        *  servers. 
        *-----------------------------------------------------------------*/

      /*---------------------------------------------------------------*
       *  If a short output was selected then sort the file in the 
       *  following order : Vendor id, product id and product version.
       *---------------------------------------------------------------*/
  
       if ( ! long_output)
       {
          sprintf (sys_cmd, "sort -t: -k1,1 -k2,2n -k3,3 -u %s ", 
                            unsorted_product_file);
          if ((status = MASK(system (sys_cmd))) != 0)
          {
              perror (progname);
              cleanup ();
              if (status_flag)
                 write_status (SORT_CMD_ERROR); 

              exit (SORT_CMD_ERROR);
          }
       }

       /*---------------------------------------------------------------*
        *  If a long output was selected then sort the file in the 
        *  following order : Vendor Name, Vendor id, Product Name, Product
        *  id and product version.
        *---------------------------------------------------------------*/
  
       else
       {
          sprintf (sys_cmd, "sort -t: -k1,1 -k2,2 -k3,3 -k4,4n -k5,5 -u %s ", 
                            unsorted_product_file);
          if ((status = MASK(system (sys_cmd))) != 0)
          {
              perror (progname);
              cleanup ();
              if (status_flag)
                 write_status (SORT_CMD_ERROR); 

              exit (SORT_CMD_ERROR);
          }
       }

    }  /***** end of if (found) *****/


   fclose (server_file);
   cleanup ();
   if (status_flag)
   {
      unlink (status_file); 
   /* write_status (STATUS_FILE_REMOVE_ERROR); */
   } 

   CATCLOSE ();

   exit (0);

} /****end main ****/


 /*---------------------------------------------------------------------*
  *                                                                     *
  *    FUNCTION      : get_vendor_products                              *
  *                                                                     *
  *    RETURNS       : this is a void function                          *
  *                                                                     *
  *    PURPOSE       : This function executes the call "ls_admin with   *
  *                    a server and vendor name as arguments to get a   *
  *                    list of products for that vendor associated with *
  *                    server. The server and vendor name are obtained  *
  *                    from the vendor_file.                            *
  *                                                                     *
  *---------------------------------------------------------------------*/
 
static void get_vendor_products (FILE *vendor_file)
{
 char vendor_name [VENDOR_NAME_LENGTH];
 char vendor [BUFFER_LENGTH];
 char server_name [MAXHOSTNAMELEN + 1];
 char   sys_cmd [PATH_MAX + 1];
 int    status;
 int    vendor_found;

 /*--------------------------------------------------------------------*
  *  Read the vendor file and get a vendor name. Execute ls_admin to get
  *  the list of all products for that vendor which have been registered
  *  with the server. Store this result in a file. Do this for every
  *  vendor in the vendor file for a particular server.
  *--------------------------------------------------------------------*/

 vendor_found = FALSE;
 while (fgets (vendor, BUFFER_LENGTH, vendor_file) != NULL)
 {
    vendor_found = TRUE;

   /*----------------------------------------------------------------------*
    *  A sequence of calls of strtok (s, ct) splits s into tokens, each 
    *  delimited by a character from ct. The first call in a sequence has
    *  a non-NULL s. Each subsequent call, indicated by a NULL value of s, 
    *  returns the next such token, searching from just past the end of the
    *  previous one.
    *----------------------------------------------------------------------*/

    strcpy (server_name, strtok (vendor, ":\t\n"));
    strcpy (vendor_name, strtok (NULL, "\t\n"));
    sprintf (sys_cmd, "LANG=C;/usr/lib/netls/bin/ls_admin -s -n %s -p \"%s\" >> %s ", 
                       server_name, vendor_name, netls_file_name);

    if ((status = MASK(system (sys_cmd))) != 0)
    {
        perror (progname);
        cleanup ();
        if (status_flag)
        {
           write_status (LS_ADMIN_NOT_FOUND);
        }
        exit (LS_ADMIN_NOT_FOUND);
    }

 }  /**** end while (fgets....) ****/

 if ( ! vendor_found)
 {
    inu_msg (FAIL_MSG, MSGSTR (MS_GET_LICENSE_INFO, GLI_NO_VENDORS_FOUND_E, 
                             GLI_NO_VENDORS_FOUND_D));
    if (status_flag)
       write_status (NO_VENDORS_FOUND);

    cleanup ();
    exit (NO_VENDORS_FOUND);
 }

 fclose (vendor_file);

}    /**** end of get_vendor_products ****/



 /*---------------------------------------------------------------------*
  *                                                                     *
  *    FUNCTION      : get_netls_info                                   *
  *                                                                     *
  *    RETURNS       : this is a void function                          *
  *                                                                     *
  *    PURPOSE       : This function parses the output obtained from    *
  *                    the call to ls_admin to get a list of products   *
  *                    for all vendors on all servers.                  *
  *                                                                     *
  *---------------------------------------------------------------------*/
 
static void get_netls_info (void)
{

  FILE  * netls_file, * product_file;
  states action_state, result_state;
  char  file_buf [BUFFER_LENGTH];
  size_t  length_of_vendor_string, length_of_product_string;


 /*--------------------------------------------------------*
  *  Create a temporary file to store the final output.
  *--------------------------------------------------------*/

  tmpnam (unsorted_product_file);

 /*------------------------------------------*
  *  Open the temporary file for writing.
  *------------------------------------------*/
 
  if ((product_file = fopen (unsorted_product_file, "w")) == NULL)
  {
     perror (progname);
     if (status_flag)
        write_status (OUTPUT_FILE_OPEN_ERROR);

     exit (OUTPUT_FILE_OPEN_ERROR);
  }

 /*----------------------------------------------------------------------*
  *  Open the netls file for reading. This file contains a list of all the
  *  products for all vendors for all servers. Parse this file and get the
  *  necessary information to write to the output file specified by the 
  *  user.
  *----------------------------------------------------------------------*/

    if ((netls_file = fopen (netls_file_name, "r")) == NULL)
    {
      perror (progname);
      if (status_flag)
         write_status (NETLS_FILE_OPEN_ERROR);

      exit (NETLS_FILE_OPEN_ERROR);
    }

  /*----------------------------------------------------------------------*
   *  Set the action_state = LOOKING_FOR_VENDOR as the output is arranged
   *  by vendor and then the products of that vendor.
   *----------------------------------------------------------------------*/

    action_state = LOOKING_FOR_VENDOR;

    length_of_vendor_string = sizeof ("Vendor:");
    length_of_product_string = sizeof ("Product:");

   /*---------------------------------------------------------*
    *  Read the output file one line at a time till empty.
    *---------------------------------------------------------*/

    while (fgets (file_buf, BUFFER_LENGTH, netls_file) != NULL)
    {
       switch (action_state)
       {
      
         case LOOKING_FOR_VENDOR:
            if (strncmp (file_buf, "Vendor:", length_of_vendor_string -1) == 0)
            {
               result_state = FOUND_VENDOR;
               process_state (result_state, file_buf, netls_file, product_file);

              /*-----------------------------------------------------*
               *  Set the action_state = LOOKING_FOR_PRODUCT as we have
               *  found a vendor.
               *-----------------------------------------------------*/

               action_state = LOOKING_FOR_PRODUCT;
            }
            break;

        case LOOKING_FOR_PRODUCT:
        case LOOKING_FOR_VEND_PROD:
            if (strncmp (file_buf, "Product:", length_of_product_string -1) == 0)
            {
               result_state = FOUND_PRODUCT;
               process_state (result_state, file_buf, netls_file, product_file);

              /*-----------------------------------------------------*
               *  Set the action_state = LOOKING_FOR_VEND_PROD as we 
               *  found a product. We could now come across a new
               *  vendor or another product for the old vendor.
               *-----------------------------------------------------*/

               action_state = LOOKING_FOR_VEND_PROD;
            }
            else
            if (strncmp (file_buf, "Vendor:", length_of_vendor_string -1) == 0)
            {
                result_state = FOUND_VENDOR;
                process_state (result_state,file_buf, netls_file, product_file);

               /*-----------------------------------------------------*
                *  Set the action_state = LOOKING_FOR_PRODUCT as we 
                *  found a new vendor. We could now come across a 
                *  product for the vendor.
                *-----------------------------------------------------*/

                action_state = LOOKING_FOR_PRODUCT;
            }
            break;
    
         default:
            break;

       } /*** end switch ***/

    } /*** end while ***/

   fclose (netls_file);
   fclose (product_file);
}  /**** get_netls_info ****/



 /*---------------------------------------------------------------------*
  *                                                                     *
  *    FUNCTION      : process_state                                    *
  *                                                                     *
  *    RETURNS       : this is a void function                          *
  *                                                                     *
  *    PURPOSE       : This function writes the parsed output to a      *
  *                    temporary file. This is the final unsorted       *
  *                    output.                                          *
  *                    The output can be in the following two formats:  *
  *                    if a long output is desired, then the format is  *
  *                    vendor name:vendor_id:product name:product_id:   *
  *                    product version.                                 *
  *                    The regular output is of the following format    *
  *                    vendor_id:product_id:product_version.            *
  *                                                                     *
  *---------------------------------------------------------------------*/
 

static void process_state (states result_state, char *buf, FILE *netls_file, 
                           FILE *product_file)
{
 static char vendor_name[VENDOR_NAME_LENGTH], 
             vendor_id [VENDOR_ID_LENGTH], 
             product_name [PRODUCT_NAME_LENGTH], 
             product_version[PRODUCT_VERSION_LENGTH];
 long        product_id ;

 char file_buf [BUFFER_LENGTH];

   switch (result_state)
   {
       case FOUND_VENDOR:
              strtok (buf, ":\t\n");
              strcpy (vendor_name, strtok (NULL, ":\t\n"));
              if (fgets (file_buf, BUFFER_LENGTH, netls_file) != NULL)
              {
                 sscanf (file_buf, "%*s %*s %s", vendor_id);
              } 
              break;
              
       case FOUND_PRODUCT:
              strtok (buf, ":\t\n");
              strcpy (product_name, strtok (NULL, "["));
              strcpy (product_version, strtok (NULL, "]"));
              if (fgets (file_buf, BUFFER_LENGTH, netls_file) != NULL)
              {
                 sscanf (file_buf, "%*s %ld", &product_id);
              } 
              if (long_output)
              {
                 fprintf (product_file, "%s:%s:", vendor_name, vendor_id);
                 fprintf (product_file, "%s:%ld:%s\n", product_name, product_id,
                                                   product_version);
              }
              else
              {
                 fprintf (product_file, "%s:%ld:%s\n", vendor_id, product_id, 
                                                   product_version);
              }
              break; 

       default:
                 break;
   }  /*** end switch ****/

}   /*** end of process_state ***/


 /*---------------------------------------------------------------------*
  *                                                                     *
  *    FUNCTION      : cleanup                                          *
  *                                                                     *
  *    RETURNS       : this is a void function                          *
  *                                                                     *
  *    PURPOSE       : Delete all the temporary files created by this   *
  *                    program.                                         *
  *                                                                     *
  *---------------------------------------------------------------------*/
 
static void cleanup (void)
{
   unlink (server_file_name); 
   unlink (vendor_file_name);
   unlink (netls_file_name);
   unlink (unsorted_product_file); 

}

 /*---------------------------------------------------------------------*
  *                                                                     *
  *    FUNCTION      : write_status                                     *
  *                                                                     *
  *    RETURNS       : this is a void function                          *
  *                                                                     *
  *    PURPOSE       : Writes the status code to the status file.       *
  *                                                                     *
  *---------------------------------------------------------------------*/
 
static void write_status (int error_code)
{
     rewind (statusfile);
     fprintf (statusfile, "%d\n", error_code);
     fflush (statusfile);

}


static void usage (void)
{
inu_msg (INFO_MSG, "\n Usage: get_license_info [-l][-f file]\n");

inu_msg (INFO_MSG, "\tThis lists NetLS enabled products which have been registered with\n");
inu_msg (INFO_MSG, "\tNetLS server (s) on a network.\n\n");
inu_msg (INFO_MSG, " -f logs status of execution to a file (for communication with caller).\n");
inu_msg (INFO_MSG, " -l lists vendor names, vendor id's, product name, product id's and\n");
inu_msg (INFO_MSG, "    product versions in a colon separated format.\n");
inu_msg (INFO_MSG, " default:\n");
inu_msg (INFO_MSG, "    lists vendor id's, product id's and product version in a colon\n");
inu_msg (INFO_MSG, "    separated format.\n");

}
