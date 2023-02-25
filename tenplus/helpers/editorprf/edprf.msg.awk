# @(#)95	1.1  src/tenplus/helpers/editorprf/edprf.msg.awk, tenplus, tenplus411, 9430C411a 7/18/94 17:38:34
#
# COMPONENT_NAME: TENPLUS
#
# FUNCTIONS: Convert text file to .msg file.
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
#
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

function prtline(str)
{
    if ( str ~ "^#" ) {
	return ;
    }
    line++ ;
    printf "%d \"%s\"\n", line, str ;
}

BEGIN {
    line=0 ;
    printf "\n" ;
    printf "$quote \"\n" ;
    printf "$set 1\n" ;
    printf "\n" ;
}

{
    prtline($0) ;
}

END {
    while( line < 1000 ) {
	prtline("") ;
    }
}
