#
#   COMPONENT_NAME: DIAGBOOT
#
#   FUNCTIONS:	findfile
#		
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
BEGIN {
   ship = ENVIRON["SHIP_PATH"]
   npath = split(ship,shipdir,":")
   ode_tools = ENVIRON["ODE_TOOLS"]
}

function findfile( file,fileship ) {

if (system("test -r " file) && system("test -L " file) ) {
for (i = 1; i <= npath; i++)
{
  file1 = shipdir[i] fileship

  if (system("test -r " file1) && system("test -L " file1) ) {
   flag = 0
  }
  else
  {
    system(ode_tools"/usr/bin/makepath -quiet " file)
    system(ode_tools"/usr/bin/cp " file1 " " file)
    flag = 1
    break
  }
}
}
else
{
  flag = 1
}
  if (flag == 0) print "Error: " file " not found in ship tree"
}

{
    nfile=substr($1,2,length($1))
    findfile($1,nfile)
}
