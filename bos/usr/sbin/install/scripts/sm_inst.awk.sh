#!/bin/awk
# @(#)95   1.27  src/bos/usr/sbin/install/scripts/sm_inst.awk.sh, cmdinstl, bos41B, 412_41B_sync 12/14/94 15:44:22
#
#   COMPONENT_NAME: cmdinstl
#
#   FUNCTIONS:   print_latest_packages
#      print_install_packages
#      print_maintenance_enhancement
#      print_fileset_packages
#      flush_fileset
#      swap
#      hsort
#      heapify
#      get_netls_code
#      match_bundle
#      match_filter
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#=============================================================================
# NOTE - This program is not intended to be used by users directly.
#    It is used by the SMIT install screens for various activities that
#        are performed within the SMIT install environment.
#
# PROGRAMMERS NOTE: 
#    please, please, please do not use tabs when adding lines - it makes this
#       file VERY unreadable
#    please use 3 spaces to show identation
#
# COMMAND LINE PARAMETERS:
# the following variables are passed in from the command line when this script
# is invoked:
#
#    OUTFILE      = name of file to write output to
#    NETLS_FILE   = name of file containing license information
#    STATUS_FILE  = name of file which is used to determine whether the netls
#                      program is still active or not
#    BUNDLE       = non-NULL indicates that bundle file being used
#    BUNDLE_FILE  = filename of bundle file
#    FILTER       = non-NULL indicates that a filter is be used
#    FILTER_FILE  = filename of filter to use
#    LATEST       = non-NULL if latest packages to be printed
#    INST         = "true" if install packages to be printed
#    ENH          = "true" if enhancements are to be printed
#    ML           = "true" if maintenance levels are to be printed
#    SS           = "true" if filesets are to be printed
#    DO_NETLS     = "true" if /usr/lib/instl/get_license_info started up in the
#                      background prior to execution of this script
#    NETLS_PID    = PID for get_license_info
#    LIST_ONLY    = "true" if headers are to be printed
#
# GENERAL DESIGN:
#    1) read the "installp -L" output and put it into one of 5 global arrays:
#          L = "latest" packages
#          I = "installed" packages
#          C = maintenance levels
#          E = enhancements
#          S = filesets
#    2) display the information based upon the command line parameters; this
#          entails calling one of the "print_..." functions
#
#=============================================================================

#===============================================================================
#============================= BEGIN                  ==========================
#===============================================================================
BEGIN {

   # the following global variables are used by the debug function to determine
   #    if and where to write debug output
   # to turn debug output off, set DBG to NULL (eg, DBG = "")
   # to turn debug output on, set DBG to the pathname of the file where you
   #    want the debug output to go
   # NOTE that when specifying that filename, you MUST quote it
   # eg. DBG = "/tmp/dbg"
   #
   DBG = ""         # name of debug file to create -or- NULL to turn debug off
   DBG_reset = 0    # >0 if DBG file has been already been reset 

   Lrecs = 0      # Latest array counter
   Irecs = 0      # Install Package array counter
   Crecs = 0      # Maintenance Level array counter
   Erecs = 0      # Enhancement array counter
   Srecs = 0      # Fileset updates array counter

   show_install_packages = INST
   show_enhancements = ENH
   show_maintenance_levels = ML
   show_filesets = SS

   # Set the Field Separator to a ":" as installp -L output is colon
   # separated.

   FS = ":"

   # Set the delimiter. This delimiter will be printed with a spacing of
   # 40 characters from the opp name. It will be followed by the product
   # name, version.all. By doing this we can directly get the product 
   # name to be sent to installp without doing any translation of 
   # the nice description to installp recognizable formats.
   delimiter = "@@"

   # the following delimiters are printed in the first field to denote the
   #    state of the fileset
   # these definitions correspond to messages INU_SM_INST_COMMON_HEADER_STRING,
   #    INU_SM_INST_BUNDLE_HEADER_STRING, and INU_SM_INST_ALREADY_INSTALLED
   #    in the /src/bos/usr/sbin/install/inuumsg/inuumsg.msg file
   NO_LICENSE_REQUIRED = "+"
   LICENSE_FOUND = "$"
   NO_LICENSE_FOUND = "!"
   CURRENTLY_INSTALLED = "@"

   debug("---------------- debug output from sm_inst.awk ---------------")
   debug( "OUTFILE =", OUTFILE )
   debug( "NETLS_FILE =", NETLS_FILE )
   debug( "STATUS_FILE =", STATUS_FILE )
   debug( "BUNDLE =", BUNDLE )
   debug( "BUNDLE_FILE =", BUNDLE_FILE )
   debug( "FILTER =", FILTER )
   debug( "FILTER_FILE =", FILTER_FILE )
   debug( "LATEST =", LATEST )
   debug( "INST =", INST )
   debug( "ENH =", ENH )
   debug( "ML =", ML )
   debug( "SS =", SS )
   debug( "DO_NETLS =", DO_NETLS )
   debug( "NETLS_PID =", NETLS_PID )
   debug( "LIST_ONLY =", LIST_ONLY )
   debug( "NO_DELIMITER =", NO_DELIMITER )

} # end of BEGIN

#===============================================================================
#============================= main                   ==========================
# This installp -L output is in the following colon separated format.
#
#INed:INed.obj:3.2.0.0::I:C:s:::U418539:N:INed Editor:5ce6f0617216.02.81.23.85.e3.00.00.00:1926:3.3
#INed:INed.obj:3.2.0.0:U418539:S:T:::::N:INed Text Editor::::
#INed:INed.obj:3.2.0.0:U491041:C:T:::::N:3240 INed Maintenance Level::::
#
# this output is printed from the function "print_gui_output", which resides in
#    the file /src/bos/usr/sbin/install/installp/inu_ls_toc.c
# the Option_t struct is defined in the /src/bos/usr/include/inu_toc.h file
#
# the fields map as follows:
#    1   = Option_t->prodname
#    2   = Option_t->name
#    3   = Option_t->level.version.release.mod.fix
#    4   = Option_t->level.ptf
#    5   = type
#    6   = state
#    7   = supersede
#    8   = superseding_ptf
#    9   = sup_state
#    10  = latest_sup
#    11  = Option_t->quiesce
#    12  = Option_t->desc
#    13  = Option_t->netls->vendor_id
#    14  = Option_t->netls->prod_id
#    15  = Option_t->netls->prod_ver
#
# depending on whether it is an installable image, a PTF or a maintenance 
# level
#
# Each of the fields will be saved and when the next line is read in,
# both lines will be stuffed into an array if they need to be kept (again,
# based on the cmd line flags).
#===============================================================================

{
   debug("-------------------------> main")
   debug( "$0 = ", $0 )

   image = $2
   version = $3
   pkg_type = $5
   state = $6

   if ($5 != "I") {
     ptf = $4
     total_version = version "." ptf
   }
   else {
     if ( $13 != "")
        base_level_with_netls = "true"
     total_version = version
   }

   # Here we make sure that we do not do a lot of processing if the
   # current package type is one that we are not interested in printing
   # out, if it is not, we move on to the next record in the input
   if ( pkg_type == "I" && show_install_packages != "true" )
     next

   if ( (pkg_type == "E" || pkg_type == "M") &&
       show_enhancements != "true" )
     next

   if ( (pkg_type == "C" || pkg_type == "P") &&
       show_maintenance_levels != "true" )
     next

   if ( pkg_type ~ "[SFG]" && show_filesets != "true" )
     next

   # If we are using a filter and not a bundle, F4 selection list
   # is not supported and we can prevent unnecessary processing of
   # filesets which are already at the latest level.
   if (BUNDLE == "")
   {
      if (((state ~ /C/) || (state ~ /A/)) && (FILTER != ""))
         next    # skip to the next record
   }

   # Get the description from the line.  Remove the trailing
   # white space from the description
   description = $12
   gsub(" *$", "", description)

   # Now read the NetLS information.
   vendor_id = $13
   product_id = $14
   product_version = $15

#-----------------------------------------------------------------------------
#
# Algorithm:
#        Add a record for this PTF to a global list with necessary fields
#    New list looks like:
#
#        1 2        3    4           5           6       7   8
#        S|nice_opp|3.02|nice_option|description|U404194|opp|option|
#
#        9                10                      11        12
#        bosnet.tcpip.obj|03.02.0000.0000.U404194|vendor id|product id|
#
#        13
#        product_version
#
# Note that the following groupings are used:
#    Images          - I types
#    Enhancements    - E and M types
#    Fileset Updates - S, F, and G types
#-----------------------------------------------------------------------------

   # Get the opp/product name. This is the first field in the installp -L
   # output.

   opp = $1
   nice_opp = opp

   # Get the option name from the image saved above.
   #
   # The option name !!!should!!! be the string after the first period
   # of the image name.  However, many of the opp/option names for our
   # products are named incorrectly.  Thus, instead of using the string
   # after the first period, the entire image name will be used.  This
   # is the only method to insure option name uniqueness.

   option = image
   nice_option = option

   # It was important to get the nice names for the opp and the option
   # here; and the way all the information about the package is
   # placed into the array is also very important because a sort will
   # be done later

   # Depending on the type of the current package being worked on, it
   # will be appended to one of the following arrays.

   if (LATEST == "true" && pkg_type == "I") {
      L[++Lrecs] = nice_opp "  |" version "|" description "  |" \
                   opp "|" option "|" image "|" total_version "|" \
                   vendor_id "|" product_id "|" product_version "|" state

      debug( "L[", Lrecs, "] = ", L[Lrecs] )
     next
   }

   # The install package array
   if (pkg_type == "I") {
     I[++Irecs] = pkg_type "|" nice_opp "  |" version "|" \
                  description "  |" ptf "|" opp "|" option "|" \
                  image "|" total_version "|" vendor_id "|" \
                  product_id "|" product_version "|" state

      debug( "I[", Irecs, "] = ", I[Irecs] )
     next
   }

   # The maintenance level array
   if (pkg_type == "ML" || pkg_type == "MC" || pkg_type == "P") {
     C[++Crecs] = pkg_type "|" nice_opp "  |" version "|" ptf "|" \
                  nice_option "  |" description "  |" opp "|" \
                  option "|" image "|" total_version "|" state

      debug( "C[", Crecs, "] = ", C[Crecs] )
     next
   }

   # The enhancement array
   if (pkg_type == "ME" || pkg_type == "M") {
     E[++Erecs] = pkg_type "|" nice_opp "  |" version "|" ptf "|" \
                  nice_option "  |" description "  |" opp "|" \
                  option "|" image "|" total_version "|" state

      debug( "E[", Erecs, "] = ", E[Erecs] )
     next
   }

   # The fileset (and ptf) array
   if (pkg_type ~ "[SFG]") {
     S[++Srecs] = nice_opp "  |" version "|" nice_option "  |" \
                  description "  |" ptf "|" pkg_type "|" opp "|" \
                  option "|" image "|" total_version "|" state

      debug( "S[", Srecs, "] = ", S[Srecs] )
     next
   }

} # end of main

#===============================================================================
#============================= END                    ==========================
# At this point, we have a list of all the PTFs to be printed.
#===============================================================================
END {
   debug("-------------------------> END")

   # Read in the results of the call to the NetLS server into an array
   # if the call returned succesfully. 
   netls_done = "false"

   #-----------------------------------------------------------------------
   # Check if the base_level_with_netls flag is set. If this flag is set 
   # then we have seen a base level on the media which is licensed. 
   # Also check if the DO_NETLS flag is set.If both the flags have not been 
   # set then we need not wait for the netls call to return as the netls 
   # information exists only for options and we didn't set the DO_NETLS flag.
   #-----------------------------------------------------------------------

   if (base_level_with_netls == "true" && DO_NETLS == "true")
   {

      #------------------------------------------------------------------
      # Set up a counter. We want to give the netls program only a minute
      # more from this point on to finish getting the license information.
      #-------------------------------------------------------------------
      count = 0
     
      #------------------------------------------------------------------
      # Check if the STATUS_FILE exists at this point. If it doesn't then
      # this could mean that the netls program has finished executing as
      # the STATUS_FILE is removed when it terminates. So set netls_done
      # to be true. If this file exists, then the netls program has not
      # finished and so we wait for 60s. The get_license_info program 
      # will time itself out after 3 minutes.
      #------------------------------------------------------------------ 

      if (system("/bin/test -r " STATUS_FILE))
      {
         netls_done = "true"
      }
      else
      {
         #---------------------------------------------------
         # Give the netls program 60s to finish executing.
         #---------------------------------------------------

         while (++count <= 60)
         {

            #----------------------------------------------------------
            # Get the status code from the STATUS_FILE. If the status
            # code is 0, then sleep as the program is still executing.
            # If the status code is non zero then break out of this
            # loop as the program failed.
            #----------------------------------------------------------

            if (( getline code < STATUS_FILE) > 0 )
            {
               if (code == "0") {
                  #  printf ("Sleeping \n")
                  system("/bin/sleep 1")
                  close (STATUS_FILE)
               }
               else {
                  #  printf ("Error\n")
                  break
               }
            }

            #-----------------------------------------------------------
            # If we could not read the status code from the STATUS_FILE
            # then check to see if the status file exists as at this
            # point the netls program might have finished and would have
            # deleted the status file. If the file still exists then
            # sleep again.
            #-----------------------------------------------------------

            else
            {
               if (system("/bin/test -s " STATUS_FILE))
               {
                  netls_done = "true"
                  break
               }
            }
         } # while (++count <= 60)

         # If get_license_info has not finished executing, then 
         # remove the temporary files used by the process as we 
         # will not be waiting for the netls output.

         if (netls_done == "false")
         {
            if (system("/bin/test -r " STATUS_FILE) == 0)
               system ("/usr/bin/rm -f " STATUS_FILE)

            if (system("/bin/test -r " NETLS_FILE) == 0)
               system ("/usr/bin/rm -f " NETLS_FILE)

            # If the ALL_LICENSED filter was specified, print warning
            # message since we have no license info to match.
            if (toupper(FILTER) == "ALL_LICENSED")
               system("/usr/sbin/inuumsg 176")
         }

      } # else part of if (system("/bin/test -r " STATUS_FILE))
   }

   #------------------------------------------------------------------------
   # If the netls program finished successfully, then read the output
   # into an array. This output will be compared with the netls information
   # obtained from the installp -L output to get license information.
   #------------------------------------------------------------------------

   if (netls_done == "true" && DO_NETLS == "true")
   {
      netls_count = 0
      close (NETLS_FILE)
      while ((getline netls_input < NETLS_FILE) > 0) {
         netls_array [++netls_count] = netls_input
      }
   }

   #------------------------------------------------------------------------
   # If a bundle has been specified and is not ALL_LICENSED, then read
   # the output into an array.  This output will be compared to the
   # installp -L output to determine what gets displayed.
   #------------------------------------------------------------------------

   if (BUNDLE != "")
   {
      bundle_count=0
      close (BUNDLE_FILE)
      while ((getline bundle_input < BUNDLE_FILE) > 0)
         bundle_array[++bundle_count] = bundle_input":false"
   }

   #------------------------------------------------------------------------
   # If a filter has been specified and is not ALL_LICENSED, then read
   # the output into an array.  This output will be compared to the
   # installp -L output to determine what gets displayed.
   #------------------------------------------------------------------------

   if (FILTER != "" && toupper(FILTER) != "ALL_LICENSED")
   {
     filter_count=0
     close (FILTER_FILE)
     while ((getline filter_input < FILTER_FILE) > 0)
        filter_array[++filter_count] = filter_input
   }

   # Variable used so that the very first line of the output is not a
   # space, but everything printed after that that is an lpp is prepended
   # by a space
   first_package = "true"

   # Variable used to determine if there is any output so that return code
   # of 2 can be used if nothing has met the selection criteria either
   # by virtue of what was on the media or by selection of bundles and
   # / or filters
   print_record = "false"

   if (LATEST == "true") {
      if (Lrecs > 0)
         print_latest_packages(L, Lrecs)
      else { 
         if (LIST_ONLY == "true")
            system("/usr/sbin/inuumsg 139")
      }
   }
   else {

      # If the install packages are supposed to be shown, and 
      # there are some in the array...
      if (show_install_packages == "true" && Irecs > 0) {
         print_install_packages(I, Irecs, first_package)
         first_package = "false"
      }
      else if ( Irecs == 0 && show_maintenance_levels != "true" &&
                show_enhancements != "true" && 
                show_filesets != "true" && LIST_ONLY == "true") {
         system("/usr/sbin/inuumsg 139")
      }
    
      # If the maintenance levels are supposed to be shown, and 
      # there are some in the array...
      if (show_maintenance_levels == "true" && Crecs > 0) {
         print_maintenance_enhancement(C, Crecs, "ML", first_package)
         first_package = "false"
      }
      else if ( Crecs == 0 && show_install_packages != "true" &&
                show_enhancements != "true" && 
                show_filesets != "true" && LIST_ONLY == "true") {
         system("/usr/sbin/inuumsg 141")
      }
    
      # If the enhancements are supposed to be shown, and there 
      # are some in the array...
      if (show_enhancements == "true" && Erecs > 0) {
         print_maintenance_enhancement(E, Erecs, "ENH", first_package)
         first_package = "false"
      }
      else if ( Erecs == 0 && show_install_packages != "true" &&
                show_maintenance_levels != "true" && 
                show_filesets != "true" && LIST_ONLY == "true") {
         system("/usr/sbin/inuumsg 140")
      }
    
      # If the filesets are supposed to be shown, and there are some
      # in the array...
      if (show_filesets == "true" && Srecs > 0) {
         print_fileset_packages(S, Srecs, first_package)
      }
      else if ( Srecs == 0 && show_install_packages != "true" &&
                show_maintenance_levels != "true" && 
                show_enhancements != "true" && LIST_ONLY == "true") 
      {
         system("/usr/sbin/inuumsg 142")
      }

   } 

   # Following display of options, list any unmatched bundle items
   # to stderr (smit listing)
   if (BUNDLE != "")
   {
     n=0
     first_unmatched = "true"
     while (++n <= bundle_count) 
     {
        split(bundle_array[n], N, ":")
        if (match(N[2],"false"))
        {
           if (first_unmatched == "true")
           {
              print
              system("/usr/sbin/inuumsg 152 " BUNDLE )
              first_unmatched = "false"
           }
           system("/bin/echo '#' " N[1] " 1>&2")
        }
     }
     if (first_unmatched == "false")
        system("/bin/echo 1>&2")
   }

   # If nothing has been printed (ie. no records of any selected type
   # or everything was filtered out) return error 2 unless only listing
   if (print_record == "false" && LIST_ONLY != "true")
     exit 2
} # end of END

#===============================================================================
#============================= debug                  ==========================
# this function is used to put debug information into the specified file
# to turn off debugging, merely set the DBG variable to NULL
#===============================================================================
function debug( str1, str2, str3, str4 ) {

   if ( DBG == "" )
      return 0

   if ( DBG_reset )
      print str1 str2 str3 str4 >>DBG
   else
   {
      print str1 str2 str3 str4 >DBG
      DBG_reset = 1
   }
}

#===============================================================================
#============================= print_latest_packages  ==========================
#===============================================================================
function print_latest_packages( L, 
                                Lrecs )
{
   debug("-------------------------> print_latest_packages")

   hsort(L, Lrecs)

   first_package = "true"
   old_version = ""         # always contains cur version
   old_opp = ""            # always contains the curr opp
   old_image = ""            # contains the curr option
   print_option = ""
   state = ""

   if (LIST_ONLY == "true") 
     system("/usr/sbin/inuumsg 179")

   for (i = 1; i <= Lrecs; i++) {

      split(L[i], a, "|")
 
      nice_opp = a[1]
      version = a[2]
      nice_option = a[3]
      opp = a[4]
      option = a[5]
      image = a[6]
      vendor_id = a[8]
      product_id = a[9]
      product_version = a[10]
      state = a[11]
 
      # We do not want to show any base levels for any products which have
      # any of the following substrings in their name :
      # .loc. .msg. ^printers ^devices
      # So we will ignore the base level entries for any product which
      # contains any of the above strings. This is being done as the base
      # levels which contain these strings will be installed automatically
      # based on certain conditions being met.
 
      if ( (BUNDLE == "") &&
           (option ~ /.*\.loc\..*|.*\.msg\..*|^printers.*|^devices.*/) )
      {
         if (option != "printers.rte")
            continue
      } 
 
      # Match the opp or option name against the bundle
      # and ignore if no match is found.
      if (match_bundle(option) == "false") 
         continue
 
      # Match the option name against the filter and
      # ignore if no match is found.
      if (match_filter(option,netls_code) == "false") 
         continue
 
      # If we get this far we will print something
      print_record = "true"
 
      if (old_opp != opp) {
 
         old_opp = opp
         old_version = version
 
         if (first_package == "true")
            first_package = "false" 
         else if (LIST_ONLY == "true")
            printf("\n") 
 
         # Here we figure out what the format of the
         # printf should be based on the length of the
         # version and the amount of space we have to
         # play with.
         version_length = length(version)
 
         if (LIST_ONLY == "true") {
            fmt = sprintf("%%%d.%ds  %%-%d.%ds ALL %%%ds%%s\n",
               version_length, version_length,
               64 - version_length,
               64 - version_length,2);
            printf(fmt,version,nice_opp,delimiter,opp)
         }
         else {
            fmt = \
                sprintf("%%%d.%ds  %%-%d.%ds ALL|%%s\n",
               version_length, version_length,
               64 - version_length,
               64 - version_length);
            printf(fmt, version,
                   nice_opp, opp) >> OUTFILE
         }
 
         print_option = "true"
      }
 
      if (image == old_image && print_option == "false") {
         continue
      }
 
      if (pkg_type == "I" && DO_NETLS == "true")
         get_netls_code( state )
      else if ((state ~ /C/) || (state ~ /A/))
         netls_code = CURRENTLY_INSTALLED
      else
         netls_code = NO_LICENSE_REQUIRED
 
      print_option = "false"
      old_image = image
 
      if (LIST_ONLY == "true") {
         fmt = sprintf("  %%%ds %%%d.%ds  %%-%d.%ds %%%ds%%s %%s.all\n",
            1, version_length, version_length,
            64 - version_length, 64 - version_length,2)
         printf(fmt, netls_code, version, nice_option,delimiter,option,version)
      }
      else {
         fmt = sprintf("    %%%d.%ds  %%-%d.%ds|%%s|%%s|%%s.all\n",
            version_length, version_length,
            64 - version_length, 64 - version_length)
         printf(fmt, version, nice_option, netls_code,image, version) >> \
            OUTFILE
      }
   } # for
} # end of print_latest_packages

#===============================================================================
#============================= print_install_packages ==========================
#===============================================================================
function print_install_packages( array, 
                                 array_count, 
                                 first_package )
{
   debug("-------------------------> print_install_packages")

   hsort(array, array_count)      # sort the whole array

   previous_opp = ""
   previous_version = ""
   previous_description = ""

   for (i = 1; i <= array_count; i++) {

      split(array[i], a, "|")

      version = a[3]
      description = a[4]
      opp = a[6]
 
      if (opp != previous_opp || version != previous_version) {
         previous_opp = opp
         previous_version = version
         previous_description = description
         continue
      }
 
      if (description == previous_description) {
         j = i - 1
         array_duplications[j] = "true"
         array_duplications[i] = "true"
         continue
      }
 
      previous_description = description

   } # for (i = 1; i <= array_count; i++)
 
   for (i = 1; i <= array_count; i++) {
 
      if (array_duplications[i] == "true") {

         split(array[i], a, "|")
 
         pkg_type = a[1]
         nice_opp = a[2]
         version = a[3]
         ptf = a[5]
         opp = a[6]
         option = a[7]
         image = a[8]
         total_version = a[9]
         vendor_id = a[10]
         product_id = a[11]
         product_version = a[12]
         state = a[13]
 
         array[i] = pkg_type "|" nice_opp "|" version "|" \
                    option "|" ptf "|" opp "|" option "|" \
                    image "|" total_version "|" vendor_id "|" \
                    product_id "|" product_version "|" state
      }

   } # for (i = 1; i <= array_count; i++)
 
   if (first_package == "false" && LIST_ONLY == "true")
      printf("\n")
 
   if (LIST_ONLY == "true")
     system("/usr/sbin/inuumsg 179")
 
   old_version = ""         # always contains cur version
   old_opp = ""            # always contains the curr opp
 
   first_package_record = "true"      # first record or not
 
   for (i = 1; i <= array_count; i++) {
 
      split(array[i], a, "|")
 
      pkg_type = a[1]
      nice_opp = a[2]
      version = a[3]
      description = a[4]
      ptf = a[5]
      opp = a[6]
      option = a[7]
      image = a[8]
      total_version = a[9]
      vendor_id = a[10]
      product_id = a[11]
      product_version = a[12]
      state = a[13]
 
      if (pkg_type == "I" && DO_NETLS == "true")
         get_netls_code( state )
      else if ((state ~ /C/) || (state ~ /A/))
         netls_code = CURRENTLY_INSTALLED
      else
         netls_code = NO_LICENSE_REQUIRED
 
      # Match the opp or option name against the bundle
      # and ignore if no match is found.
      if (match_bundle(option) == "false") 
         continue
 
      # Match the option name against the filter and
      # ignore if no match is found.
      if (match_filter(option,netls_code) == "false") 
         continue
 
      # If we get this far we will print something
      print_record = "true"
 
      # if the current opp is not equivalent to the old opp
      # then we have changed OPPs, in which case, we will print
      # "opp header" line containing the version/opp and "ALL"
      if (old_opp != opp) {
 
         old_version = version   # now a new "old_version" value
         old_opp = opp      # now a new "old_opp" value
 
         if (first_package_record == "true")
            first_package_record = "false"
         else if (LIST_ONLY == "true") 
            printf("\n")
 
         version_length = length(version)
 
         if (NO_DELIMITER == "true") {
            fmt = sprintf("%%%d.%ds  %%-%d.%ds\n",
                          version_length, 
                          version_length,
                          64 - version_length,
                          64 - version_length )
            printf( fmt,
                    version,
                    nice_opp )
         }
         else if (LIST_ONLY == "true") {
            fmt = sprintf("%%%d.%ds  %%-%d.%ds ALL %%%ds%%s \n",
               version_length, version_length,
               64 - version_length,
               64 - version_length,2);
            printf(fmt,version,nice_opp,delimiter,opp)
         }
         else {
            fmt = \
            sprintf("%%%d.%ds  %%-%d.%ds ALL|%%s|%%s|%%s\n",
                    version_length, 
                    version_length,
                    64 - version_length,
                    64 - version_length )
            printf( fmt, 
                    version, 
                    nice_opp,
                    pkg_type,
                    version,
                    opp ) >> OUTFILE
         }
      } # if (version != old_version || old_opp != opp)
 
      if (NO_DELIMITER == "true") {
         fmt = sprintf("  %%%ds %%%d.%ds  %%-%d.%ds\n",
                       1, 
                       version_length, 
                       version_length,
                       64 - version_length,
                       64 - version_length )
         printf( fmt, 
                 netls_code, 
                 version, 
                 description )
      }
      else if (LIST_ONLY == "true") {
         fmt = sprintf("  %%%ds %%%d.%ds  %%-%d.%ds %%%ds%%s %%s.all\n",
            1, version_length, version_length,
            64 - version_length, 64 - version_length,2)
         printf(fmt, netls_code, version, description,delimiter,option,total_version)
      }
      else {
         fmt = sprintf("    %%%d.%ds  %%-%d.%ds|%%s|%%s|%%s|%%s|%%s|%%s\n",
                       version_length, 
                       version_length,
                       64 - version_length,
                       64 - version_length )
         printf( fmt,
                 version,
                 description,
                 pkg_type,
                 netls_code,
                 version,
                 opp,
                 image,
                 total_version ) >> OUTFILE
      }

   } # for (i = 1; i <= array_count; i++)

} # end of print_install_packages

#===============================================================================
#============================= print_maintenance_enhancement ===================
#===============================================================================
function print_maintenance_enhancement( array,
                                        array_count,
                                        all_type,
                                        first_package)
{
   debug("-------------------------> print_maintenance_enhancement")

   if (first_package == "false" && LIST_ONLY == "true") printf("\n")

   if (LIST_ONLY == "true") {
     if (all_type == "ML")
        system("/usr/sbin/inuumsg 172")
     else
        system("/usr/sbin/inuumsg 173")
   }

   hsort(array, array_count)   # sort the whole array

   saved_version = ""      # always contains current version
   saved_opp = "nil"      # always contains the current opp
   saved_ptf = "nil"      # always contains the current ptf

   first_package_record = "true"   # first record or not

   for (i = 1; i <= array_count; i++) {

      split(array[i], a, "|")
 
      pkg_type = a[1]
      nice_opp = a[2]
      version = a[3]
      ptf = a[4]
      nice_option = a[5]
      description = a[6]
      opp = a[7]
      option = a[8]
      image = a[9]
      total_version = a[10]
 
      # Match the opp or option name against the bundle
      # and ignore if no match is found.
      if (match_bundle(option) == "false") 
         continue
 
      # Match the option name against the filter and
      # ignore if no match is found.
      if (match_filter(option,netls_code) == "false") 
         continue
 
      # If we get this far we will print something
      print_record = "true"
 
      # if the current version is not equivalent to the old version
      # or the current opp is not equivalent to the old opp
      # then we have changed OPPs, in which case, we will print
      # "opp header" line containing the version/opp and "ALL"
      if (version != saved_version || saved_opp != opp) {
 
         saved_version = version   # new "saved_version" value
         saved_opp = opp      # new "saved_opp" value
 
         if (first_package_record == "true") 
            first_package_record = "false"
         else 
            if (LIST_ONLY == "true") printf("\n")
 
         version_length = length(version)
 
         if (LIST_ONLY == "true") {
            fmt = sprintf("# %%%d.%ds  %%-%d.%ds %%3s\n",
               version_length, version_length,
               62 - version_length,
               62 - version_length)
            printf(fmt,version,nice_opp,all_type)
         }
         else {
            fmt = sprintf("%%%d.%ds  %%-%d.%ds %%3s/ALL|%%s|%%s|%%s\n",
               version_length, version_length,
               60 - version_length,
               60 - version_length)
            printf(fmt,
               version,
               nice_opp,
               all_type,
               pkg_type,
               version,
               opp) >> OUTFILE
         }
      }
 
# Commented the following 'if' to fix 168568 
#     if (ptf != saved_ptf) {
#        saved_ptf = ptf
         if (ptf == "") { pptf = ""}
         else { pptf = sprintf(".%s",ptf) }
         if (LIST_ONLY == "true") {
            fmt = sprintf("    %%-%d.%ds %%%ds%%s %%s%%s\n",
               64,64,4)
            if (ptf == "") { pptf = ""}
            else { pptf = sprintf(".%s",ptf) }
            printf(fmt,description,delimiter,option,version,pptf)
         }
         else {
            # Modify the image that will go on the
            # command line so it is something like:
            # bosadt.all
            # This will make sure all the options are
            # installed for that maintenance level or
            # enhancement.
            tmp_image = substr(image, 0, \
                  index(image, "\.")) "all"
                                if (tmp_image == "all") {
                                    tmp_image = image
                                }
            printf("    %-58.58s %7.7s|%s|%s|%s|%s|%s\n",
               description,
               ptf,
               pkg_type,
               version,
               opp,
               tmp_image,
               total_version) >> OUTFILE
         }
#     }

   } # for (i = 1; i <= array_count; i++)

} # end of print_maintenance_enhancement

#===============================================================================
#============================= print_fileset_packages ==========================
#===============================================================================
function print_fileset_packages( S,
                                 Srecs,
                                 first_package)
{
   debug("-------------------------> print_fileset_packages")

   if (first_package == "false" && LIST_ONLY == "true") printf("\n")

   if (LIST_ONLY == "true") system("/usr/sbin/inuumsg 174")

   hsort(S, Srecs)         # sort the records
   first_printed_opp = "true"
   last_version = ""
   last_opp = ""
   last_nice_option = ""

   for (i = 1; i <= Srecs; i++) {
      # split the string in S[i] into the array a
      split(S[i], a, "|")
 
      # give each member of the array "a" a nice name
      nice_opp = a[1]
      version = a[2]
      nice_option = a[3]
      fileset = a[4]
      ptf = a[5]
      pkg_type = a[6]
      opp = a[7]
      option = a[8]
      image = a[9]
      total_version = a[10]
 
      # Match the opp or option name against the bundle
      # and ignore if no match is found.
      if (match_bundle(option) == "false") 
         continue
 
      # Match the option name against the filter and
      # ignore if no match is found.
      if (match_filter(option,netls_code) == "false") 
         continue
 
      # If we get this far we will print something
      print_record = "true"
 
      # check to see if nice_option name has changed
      # print new heading line if it has
      if (version != last_version || nice_option != last_nice_option) {
         last_version = version      # reset the version
         last_nice_option = nice_option   # reset the nice_option
 
         # Blank line at start of listing
         if (first_printed_opp == "true")
            first_printed_opp = "false"
         else
            if (LIST_ONLY == "true") printf("\n")
 
         version_length = length(version)
 
         if (LIST_ONLY == "true") {
            fmt = sprintf("# %%%d.%ds  %%-%d.%ds FS\n",
               version_length, version_length,
               63 - version_length,
               63 - version_length)
            printf(fmt,version,nice_option)
         }
         else {
            fmt = sprintf("    %%%d.%ds  %%-%d.%ds FS/ALL|S|%%s|OPTION|%%s\n",
               version_length, version_length,
               64 - version_length, 
               64 - version_length)
            printf(fmt,
               version,
               nice_option,
               version,
               option) >> OUTFILE
         }
      }
 
      # print the fileset selection line 
      if (LIST_ONLY == "true") {
         fmt = sprintf("  %%%d.%ds  %%-%d.%d7s %%s  %%%ds%%s %%s%%s\n",
            version_length, 
            version_length,
            58 - version_length,
            58 - version_length,1)
         if (ptf == "") { dptf="       " ; pptf = "" }
         else { dptf=ptf ; pptf = sprintf(".%s",ptf) }
         printf(fmt,version,fileset,dptf,delimiter,option,version,pptf)
      }
      else {
         printf("%8s%-55.55s%7.7s|S|%s|FS|%s|%s|%s|%s|%s\n",
            " ",fileset,ptf,version,opp,option,
            fileset,image,total_version) >> OUTFILE
      }

   } # for (i = 1; i <= Srecs; i++)

} # end of print_fileset_packages

#===============================================================================
#============================= swap                   ==========================
#===============================================================================
function swap(A, i, j,  t)
{
   t = A[i]; A[i] = A[j]; A[j] = t
} # end of swap

#===============================================================================
#============================= hsort                  ==========================
#===============================================================================
function hsort(A, n,  i)
{
   for (i = int(n / 2); i >= 1; i--) {
     heapify(A, i, n)
   }
   for (i = n; i > 1; i--) {
     swap(A, 1, i)
     heapify(A, 1, i - 1)
   }
} # end of hsort

#===============================================================================
#============================= heapify                ==========================
#===============================================================================
function heapify(A, left, right,  p, c)
{
   for (p = left; (c = 2 * p) <= right; p = c) {
     if (c < right && A[c + 1] > A[c]) {
        c++;
     }
     if (A[p] < A[c]) {
        swap(A, c, p)
     }
   }
} # end of heapify

#===============================================================================
#============================= get_netls_code         ==========================
#===============================================================================
function get_netls_code( state )
{
   debug("-------------------------> get_netls_code")

   # Now compare the vendor_id, product_id and product version of the
   # product with the output in the netls_array

   if ( netls_count == 0 && vendor_id != "")
     netls_code = NO_LICENSE_FOUND
 
   # If the product does not have a vendor id then set the netls code to an
   # asterisk. This implies that the product is not licensed.

   else if (vendor_id == "")
   {
     if ((state ~ /C/) || (state ~ /A/))
        netls_code = CURRENTLY_INSTALLED
     else
        netls_code = NO_LICENSE_REQUIRED
   }

   # If the product has a vendor id, then try and match the vendor id, product
   # id and product version with the information in the netls array. If a 
   # match is found, then that implies that we have obtained the license 
   # for the product. However, if a match does not exist then this implies 
   # that we do do not have a license for this product.

   else
   {
     n = 0
     not_found = "true"
     while ( not_found == "true" && ++n <= netls_count )
     {
        split(netls_array[n], N, ":")
        if (vendor_id == N[1] && product_id == N[2] && product_version == N[3])
        {
           netls_code = LICENSE_FOUND
           not_found = "false"
        }
        else
           netls_code = NO_LICENSE_FOUND
     }
   }

   debug( "netls_code = ", netls_code )

} # end of get_netls_code

#-----------------------------------------------------------------------------
#----------------------------- match_bundles          --------------------------
# Match option provided to the bundle contents.
# Record all matches, then return either "true" or "false"
#-----------------------------------------------------------------------------
function match_bundle( option, n, N, found )
{
   # Always true if we have no bundle
   if (BUNDLE == "")
     return "true"
   n = 0
   # Look for a match
   found = "false"
   while (++n <= bundle_count) 
   {
     split(bundle_array[n], N, ":")
     if (match(option,N[1]))
     {
        found = "true"
        bundle_array[n] = N[1]":true"
     }
   }
   return found

} # end of match_bundle

#-------------------------------------------------------------------------------
#----------------------------- match_filter           --------------------------
# Match option provided to the filter contents or check license status for
# ALL_LICENSED filter.  Record the match, then return either "true" or "false"
#-----------------------------------------------------------------------------
function match_filter( option, netls_code, n )
{
   # Always true if we have no filter
   if (FILTER == "")
     return "true"
   # Handle ALL_LICENSED filter first since it is quick
   if (toupper(FILTER) == "ALL_LICENSED")
   {
     if (netls_code == NO_LICENSE_FOUND)
        return "false"
     else
        return "true"
   }
   # Require an exact match of the option name with the filter
   # to eliminate as much as possible.  This differs from bundles
   # since they will match from an opp downwards.
   n = 0
   while (++n <= filter_count)
   {
     if (match(filter_array[n],option))
     {
        return "true"
     }
   }
   return "false"

} # end of match_filter

