# @(#)05	1.1  src/packages/bos/diagboot/createshipfile.awk.S, diagboot, pkg411, 9430C411a 7/20/94 14:37:26
#
#   COMPONENT_NAME: DIAGBOOT
#
#   FUNCTIONS:	Create a file in the current directory
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
   ode_tools = ENVIRON["ODE_TOOLS"]
}

function createfile(file1, file ) {

  if (system("test -r " file) && system("test -L " file) ) {
	 flag = 0
  }
  else
    {
    system(ode_tools"/usr/bin/makepath -quiet " file1)
    system(ode_tools"/usr/bin/cp " file " " file1)
    flag = 1
    }
  }

{
    npath = split($1,filename,"/")
    createfile($1, filename[npath])
}
