# @(#)01	1.5  src/tenplus/interface/hlplocal.awk, tenplus, tenplus411, GOLD410 7/19/91 14:46:27
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
	lines = "hlpltab2.c"
	externs = "hlpltab1.c"
	stubs = "edrtab1.c"
	rlines = "edrtab2.c"
	edscript = "edtrc.sed"
	hlpscript= "hlptrc.sed"
	builtin = "builtin.h"
	arg [0] = ""
	arg [1] = "a"
	arg [2] = "a, b"
	arg [3] = "a, b, c"
	arg [4] = "a, b, c, d"
	arg [5] = "a, b, c, d, e"
	arg [6] = "a, b, c, d, e, f"
	arg [7] = "a, b, c, d, e, f, g"
	printf "#ifdef BLTHELP\n" > stubs
	printf "#include \"builtin.h\"\n" > stubs
	printf "extern struct hlprtab hlprtab [];\n" > stubs
	printf "#endif\n\n" > stubs
	printf "extern struct helper g_hep;\n\n" > stubs
# Do not use printf instead of print on following statement, as printf
# will try to interpret the src/tenplus/interface/hlplocal.awk, tenplus, tenplus411, GOLD410 etc (which are used by SCCS).
	print "/* builtin.h: from src/tenplus/interface/hlplocal.awk, tenplus, tenplus411, GOLD410 7/19/91 14:46:27 */\n\n" > builtin
	printf "struct hlprtab {\n" > builtin
	printf "    char *name; /* helper name */\n" > builtin
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
	    printf "    int (*%s)();\n", $3 > builtin
	    cast = "(int)"
	    }
	else if ($1== "char")
	    {
	    printf "char *" > stubs
	    printf "A_CHAR" > lines
	    printf "A_CHAR" > rlines
	    printf "extern char *%s ();\n", $2 > externs
	    printf "    char *(*%s)();\n", $3 > builtin
	    cast = ""
	    }
	else if ($1 == "obj")
	    {
	    printf "char *" > stubs
	    printf "A_OBJ"  > lines
	    printf "A_OBJ"  > rlines
	    printf "extern char *%s ();\n", $2 > externs
	    printf "    char *(*%s)();\n", $3 > builtin
	    cast = ""
	    }
	else if ($1 == "void")
	    {
	    printf "void " > stubs
	    printf "0" > lines
	    printf "0" > rlines
	    printf "extern %s ();\n", $2 > externs
	    printf "    void (*%s)();\n", $3 > builtin
	    cast = "(void)"
	    }
	else
	    {
	    printf "\\??"  > lines
	    printf "\\??"  > rlines
	    }
	printf "},\n"       > lines
	printf "},\n"       > rlines

	printf "s/dolocal (%d):/(local) %s:/\n", NR + 1, $3 >> hlpscript
	printf "s/doremote (%d):/(remote) %s:/\n", NR + 1, $3 >> edscript
	printf "%s (%s)\n", $3, arg [NF-3] > stubs
	for (i = 4; i <= NF; i++)
	    {
	    if ($i == "int")
		printf "int %c;\n", 97+i-4 > stubs
	    else
		printf "char *%c;\n", 97+i-4 > stubs
	    }
	printf "    {\n" > stubs

	printf "#ifdef BLTHELP\n" > stubs
	printf "    if (g_hep.builtin)\n" > stubs
	printf "        {\n" > stubs

	if (cast=="(void)")
	    printf "        ((*(hlprtab [g_hep.builtin].%s))", $3 > stubs
	else
	    printf "        return ((*(hlprtab [g_hep.builtin].%s))", $3 > stubs
	if (NF==3)
	    printf "());\n" > stubs
	else
	    printf "(%s));\n", arg [NF-3] > stubs
	if (cast=="(void)")
	    printf "        return;\n" > stubs
	printf "        }\n" > stubs
	printf "#endif\n" > stubs

	printf "#ifdef PIPEHELP\n" > stubs
	if (cast=="")
	    printf "    return (doremote "  > stubs
	else
	    if (cast=="(void)")
		printf "    (doremote " > stubs
	    else
		printf "    return (%s doremote ", cast  > stubs
	if (NF==3)
	    printf "(%d));\n", NR + 1 > stubs
	else
	    printf "(%d, %s));\n", NR + 1, arg [NF-3] > stubs
	printf "#endif\n" > stubs
	printf "    }\n\n" > stubs
	}
END     {
	printf "\nint g_nlocals = %d;\n", NR + 1 > externs
	printf "\n" >> edscript
	printf "\n" >> hlpscript
	printf "};\n" >> builtin
	}
