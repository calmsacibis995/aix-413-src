$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)33        1.1 2/9/94 09:00:19
$
$ COMPONENT_NAME: CMDSCCS	source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$quote "
$set 49
1 "Usage: admin {-n|-i[Name]} [-f{l{a|Number ...}|\n\
\t{b|cNumber|dSID|fNumber|j|mModule|n|qText|tType}} ...]\n\
\t[-fv[Program]] [-m[MRList]] [-aUser ...] [-rNumber.Number]\n\
\t[-t File] [-y[Comment]] File ...\n\
\tCreates and initializes SCCS files.\n\
\n\
\tUsage: admin [-aUser ...] [-eUser ...] [-f{l{a|Number ...}|\n\
\tb|cNumber|dSID|fNumber|i|j|mModule|n|qText|tType} ...]\n\
\t[-d{l{a|Number ...}|b|c|d|f|i|j|m|n|q|t} ...] [-dv|-fv[Program]]\n\
\t[-t[File]] File ...\n\
\tChanges existing SCCS files.\n"
