# @(#)37        1.2  src/bos/usr/sbin/install/scripts/oslevel.awk.sh, cmdswvpd, bos411, 9428A410j 5/23/94 17:04:45
#
#   COMPONENT_NAME: CMDSWVPD
#
#   FUNCTIONS:
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

#   Awk script invoked via oslevel shell script
#   Reads truncated lslpp -L output provided by oslevel shell script
#   oslevel shell script sets some of the variables used herein

BEGIN {

FS=":"
fileset_ct=0 # number of filesets read (used for -g in END section)
header=0 # have we printed the output heading?

# Start looking for a complete maintenance level starting with the most
# recent unless a specific level has been specified using -l
if (down != 1)
{
   # Get list of all known maintenance levels, most recent first, into ml_array
   system("/bin/grep START " mlinfo " | sort -n -r -t . -k3.1 | cut -f2 -d' ' >" work)
   ml_count=0
   close (work)
   while ((getline ml_input < work) >0)
      ml_array[++ml_count] = ml_input
   ml_find=1
   # Get fileset and VRMF information for the most recent maintenance level
   get_ml_filesets(ml_array[1])
}
else
   # Get fileset and VRMF information for the selected maintenance level
   get_ml_filesets(level)

} # end of BEGIN

# Start of BODY

# "Fileset:Level" is first line of output
NR == 1 {continue}

# Is this fileset in the current maint level? (assuming cumulative supersedes)
{if (system("/bin/grep -q " $1 " " work) != 0) {continue}}

{
# -l processing - compare to selected VRMF level and print out if lower
   if (down == 1)
   {
      if (compare_VRMF($2,ml_fileset_level[$1]) == -1)
         display($1,$2,ml_fileset_level[$1])
      continue
   }

# basic and -g processing -
# If this fileset is at a lower level than the fileset should be for the
# maintenance level being evaluated, then the maintenance level is incomplete
# and we should select prior maintenance levels for evaluation until the
# fileset is matched or is not present in the maintenance level.

   while (compare_VRMF($2,ml_fileset_level[$1]) == -1)
   {
      # Have we exhausted the list of maintenance levels to compare to?
      if (++ml_find > ml_count)
      {
         system("/usr/sbin/inuumsg 169")
         up=0 # cancel any -g output since it is not valid
         basic=0 # cancel basic output since there is no result
         exit 0
      }
      else
      {
         # Get next prior maintenance level fileset and VRMF information
         get_ml_filesets(ml_array[ml_find])
         # Check that this fileset was present in the prior maintenenace level
         if (system("/bin/grep -q " $1 " " work) != 0) {continue}
      }
   }
   
# Save selected fileset and VRMF for subsequent processing by -g 
fileset_array[++fileset_ct] = $1
level_array[$1] = $2

}

# end of BODY

END {

# If basic command, print out the matched maintenance level
if (basic == 1) 
{
   printf("%s\n",ml_array[ml_find])
   exit 0
}

# If -l specified, or -g not specified, or no longer valid, we have finished
if (down == 1 || up == 0) {exit 0}

# -g processing
# For all saved filesets, check that they are included in the complete
# Maintenance level found, then compare VRMF and print if higher.

for (x = 0 ; x <= fileset_ct ; ++x)
   if (system("/bin/grep -q " fileset_array[x] " " work) == 0)
      if (compare_VRMF(level_array[fileset_array[x]],ml_fileset_level[fileset_array[x]]) == 1)
         display(fileset_array[x],level_array[fileset_array[x]],ml_fileset_level[fileset_array[x]])

exit 0

} # end of END

#-----------------------------------------------------------------------------
# Formatted output for -g and -l results
#-----------------------------------------------------------------------------
function display(fileset,v1,v2)
{
   if (header == 0)
   {
      system("/usr/sbin/inuumsg 170")
      header = 1
   }
   printf("%-40s%-20s%s\n",fileset,v1,v2)
}

#-----------------------------------------------------------------------------
# Compare the VRMF elements of two VRMF values returning the following:
# First is less than second: -1
# First is greater than second: 1
# First is equal to second: 0
#-----------------------------------------------------------------------------
function compare_VRMF(v1,v2)
{
   split(v1,VRMF1,".")
   split(v2,VRMF2,".")
   if (VRMF1[1] < VRMF2[1]) {return -1}
   if (VRMF1[1] > VRMF2[1]) {return 1}
   if (VRMF1[2] < VRMF2[2]) {return -1}
   if (VRMF1[2] > VRMF2[2]) {return 1}
   if (VRMF1[3] < VRMF2[3]) {return -1}
   if (VRMF1[3] > VRMF2[3]) {return 1}
   if (VRMF1[4] < VRMF2[4]) {return -1}
   if (VRMF1[4] > VRMF2[4]) {return 1}
   return 0
}

#-----------------------------------------------------------------------------
# Extract all fileset information for a particular maintenance level from
# the mlinfo file and also place it in temp work file for subsequent greps
#-----------------------------------------------------------------------------
function get_ml_filesets(level,    start)
{
   system("cat </dev/null >" work)
   close (mlinfo)
   start = "false"
   ml_recs=0
   while ((getline ml_input < mlinfo) >0)
      if (start == "true")
      {
         if (ml_input == "END "level)
            return
         split(ml_input,ml_fields," ")
         printf("%s\n",ml_fields[1]) >> work
         ml_fileset_level[ml_fields[1]] = ml_fields[2]
      }
      else
         if (ml_input == "START "level)
            start = "true"
}
