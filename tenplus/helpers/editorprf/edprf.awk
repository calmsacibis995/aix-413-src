# @(#)15	1.2  src/tenplus/helpers/editorprf/edprf.awk, tenplus, tenplus411, 9430C411a 7/18/94 17:37:47
#
# COMPONENT_NAME: TENPLUS
#
# FUNCTIONS: Convert text file to .psz file.
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
    printf "$set %d\n$ $%%1\n$ $.0\n$ $:/*\n", line ;
    printf "$ $%%3\n$ $.0\n$ $:/*/text\n" ;
    printf "AA \"%%+%d\\n\\\n%s\"\n", line, str ;
    printf "$ $$\n\n" ;
}

BEGIN {
    line=0 ;
    printf "$ $Nedprf.hdq\n\n$quote \"\n\n" ;
}

{
    prtline($0) ;
}

END {
    while( line < 1000 ) {
	prtline("") ;
    }
}

