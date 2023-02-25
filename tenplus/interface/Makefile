# @(#)05	1.9  src/tenplus/interface/Makefile, tenplus, tenplus41J, 9517A_all 4/24/95 21:11:18
#
# COMPONENT_NAME: (TENPLUS) INed Editor
# 
# FUNCTIONS:
# 
# ORIGINS:  9, 10, 27
# 
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1995
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# The files in this directory are useful for automating the implementation
# procedure for helper interface changes.  The make procedure builds four files.

#    eimports.c    Editor file containing editor side of helper interface
#    himports.c    Helper file containing helper side of helper interface
#    edtrc.sed     Sed script for converting editor pipe trace files
#    hlptrc.sed    Sed script for converting helper pipe trace files
#
# To build these files, six input files are needed.
#
#    editor.c:     Template for building eimports.c
#    editor.in:    Data file describing editor side of the helper interface
#    edlocal.awk:  Awk script for building eimports.c
#    helper.c:     Template for building himports.c
#    helper.in:    Data file describing helper side of the helper interface
#    hlplocal.awk: Awk script for building himports.c
#
# Two parallel build procedures are needed, one for the editor side and one
# for the helper side.  The editor side will be described.  The first step
# is to run awk.
#
#    awk -f edlocal.awk editor.in
#
# Editor.in contains one line for each helper-callable routine available from
# the editor.  Each line contains at least three fields.  The fields are:
#
#    $1            The return type of the routine
#    $2            The name of the routine on the editor side
#    $3            The name of the routine on the helper side
#    $4...         Argument types
#
# If lines are added at the end, there is no need to recompile all of the
# helpers.
#
# If the name on the helper side begins with 'I', there should be a
# corresponding routine (whose name has 'E' instead of 'I') in help.c.  Its
# responsibility is to sprintf a variable number of arguments into a
# single argument string.
#
# Edlocal.awk creates four files from editor.in.
#
#     edltab1.c     Extern declarations for the editor side of the
#		  helper-callable routines
#     edltab2.c     Struct elements decribing editor side of the
#		  helper-callable routines
#     hlprtab1.c    Actual helper routines for calling editor
#     hlprtab2.c    Struct elements describing helper side of the
#		  editor-callable routines
#
# Now the helper side of the build procedure will be described.  The first step
# is to run awk.
#
#    awk -f hlplocal.awk helper.in
#
# Helper.in contains one line for each editor-callable routine available from
# the helper.  Each line contains at least three fields.  The fields are:
#
#    $1            The type of the routine
#    $2            The name of the routine on the helper side
#    $3            The name of the routine on the editor side
#    $4...         Argument types
#
# Hlplocal.awk creates four files from editor.in.
#
#    hlpltab1.c    Extern declarations for the helper side of the
#		  editor-callable routines
#    hlpltab2.c    Struct elements describing the helper side of the
#		  editor-callable routines
#    edrtab1.c     Actual editor routine for calling the helper
#    edrtab2.c     Struct elements describing the editor side of the
#		  editor-callable helper routines
#
# Once these eight files have been built by awk, they must be inserted into
# the template files by the C preprocessor.  The two commands are
#
#    $(CC) -I$(INC) -P editor.c
#    $(CC) -I$(INC) -P helper.c
#
# These commands create, respectively, editor.i and helper.i which are then
# moved to eimports.c and himports.c.  Note that preprocessing editor.c
# causes the inclusion of four files, two from the first awk and two from the
# second.  The same is true for building helper.i.
#
# As a side-effect of running awk, two sed scripts are built,
# edtrc.sed and hlptrc.sed.  These are passed to sed via the "-f" parameter
# for converting pipe trace debugging files to a more readable form.
#

OTHERS = eimports.c himports.c edtrc.sed hlptrc.sed
INCLUDES = builtin.h
EXPINC_TARGETS = export_builtin.h
IDIR = /usr/include/
SCRIPTS	=	editor.in.S helper.in.S
INCFLAGS = -I${MAKETOP}/tenplus/include
CFLAGS = -DBLTHELP -DPIPEHELP

eimports.c:	editor.i
	-${RM} -f eimports.c
	${SED} -e '/^# [0-9][0-9]*/d' < ${editor.i:P} > ${.TARGET}

himports.c:	helper.i
	-${RM} -f himports.c
	${SED} -e '/^# [0-9][0-9]*/d' < ${helper.i:P} > ${.TARGET}

.include <${RULES_MK}>

editor.i:	editor.c builtin.h
	${RM} -f ${.TARGET} editor.u
	${CC} -P ${_CCFLAGS_} ${editor.c:P}

helper.i:	helper.c builtin.h
	${RM} -f ${.TARGET} helper.u
	${CC} -P ${_CCFLAGS_} ${helper.c:P}

edtrc.sed hlptrc.sed: builtin.h

builtin.h : editor.in edlocal.awk hlplocal.awk helper.in
	${RM} -f ${.TARGET} edltab1.c edltab2.c edrtab1.c edrtab2.c \
		edtrc.sed hlptrc.sed hlpltab1.c hlpltab2.c hlprtab1.c hlprtab2.c
	$(AWK) -f ${edlocal.awk:P} ${editor.in:P}
	$(AWK) -f ${hlplocal.awk:P} ${helper.in:P}

