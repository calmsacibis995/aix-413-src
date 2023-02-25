# @(#)00	1.5  src/tenplus/interface/edlocal.awk, tenplus, tenplus411, GOLD410 7/19/91 14:46:17
# COMPONENT_NAME: (INED) INed Editor
# 
# ORIGINS:  9, 10
# 
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1988
# All Rights Reserved
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

BEGIN   {
	lines = "edltab2.c"
	externs = "edltab1.c"
	stubs = "hlprtab1.c"
	rlines = "hlprtab2.c"
	edscript = "edtrc.sed"
	hlpscript= "hlptrc.sed"
	arg [0] = ""
	arg [1] = "a"
	arg [2] = "a, b"
	arg [3] = "a, b, c"
	arg [4] = "a, b, c, d"
	arg [5] = "a, b, c, d, e"
	arg [6] = "a, b, c, d, e, f"
	arg [7] = "a, b, c, d, e, f, g"
	}
	{
	printf "{(char * (*)()) %s, ", $2  > lines
	printf "{" > rlines
	if (NF < 4)
	    {
	    printf "0, "  > lines
	    printf "0, "  > rlines
	    }
	else
	    {
	    printf "\"" > lines
	    printf "\"" > rlines
	    for (i = 4; i <= NF; i++)
		{
		if ($i == "int")
		    {
		    printf "\\01" > lines
		    printf "\\01" > rlines
		    }
		else if ($i == "char")
		    {
		    printf "\\02" > lines
		    printf "\\02" > rlines
		    }
		else if ($i == "obj")
		    {
		    printf "\\03"  > lines
		    printf "\\03"  > rlines
		    }
		else
		    {
		    printf "\\??" > lines
		    printf "\\??" > rlines
		    }
		}
	    printf "\", " > lines
	    printf "\", " > rlines
	    }

	if ($1 == "int")
	    {
	    printf "A_INT" > lines
	    printf "A_INT" > rlines
	    printf "extern %s ();\n", $2 > externs
	    cast = "(int)"
	    }
	else if ($1== "char")
	    {
	    printf "char *" > stubs
	    printf "A_CHAR" > lines
	    printf "A_CHAR" > rlines
	    printf "extern char *%s ();\n", $2 > externs
	    cast = ""
	    }
	else if ($1 == "obj")
	    {
	    printf "char *" > stubs
	    printf "A_OBJ"  > lines
	    printf "A_OBJ"  > rlines
	    printf "extern char *%s ();\n", $2 > externs
	    cast = ""
	    }
	else if ($1 == "void")
	    {
	    printf "0" > lines
	    printf "0" > rlines
	    printf "extern %s ();\n", $2 > externs
	    cast = "(int)"
	    }
	else
	    {
	    printf "\\??"  > lines
	    printf "\\??"  > rlines
	    }
	printf "},\n"       > lines
	printf "},\n"       > rlines

	printf "s/dolocal (%d):/(local) %s:/\n", NR + 1, $3 > edscript
	printf "s/doremote (%d):/(remote) %s:/\n", NR + 1, $3 > hlpscript
	printf "%s (%s)\n", $3, arg [NF-3] > stubs
	for (i = 4; i <= NF; i++)
	    {
	    if ($i == "int")
		printf "int %c;\n", 97+i-4 > stubs
	    else
		printf "char *%c;\n", 97+i-4 > stubs
	    }
	printf "    {\n    " > stubs
	if (cast=="")
	    printf "return (doremote "  > stubs
	else
	    printf "return (%s doremote ", cast  > stubs
	if (NF==3)
	    printf "(%d));\n    }\n\n", NR + 1 > stubs
	else
	    printf "(%d, %s));\n    }\n\n", NR + 1, arg [NF-3] > stubs
	}
END     {
	printf "\nint g_nlocals = %d;\n", NR + 1 > externs
	}
