static char sccsid[] = "@(#)04  1.7  src/bldenv/pkgtools/ilvmsg.c, pkgtools, bos412, GOLDA411a 8/23/94 13:35:48";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

char *commandName = "ilverify";

char *Usage =
"USAGE:  \n\t%s [ -p <searchDir> ] [ -e <exceptList> ] [ -i <insLists> ]\n\
		[ -t <idTableFile> ] [ -n <dbFileIn> ] [ -d <dbFileOut> ]\n\n\
	where the -d and -n flags are mutually exclusive.\n";

char *noInsCurDir =
"\n\tNo *.il files were found in the current directory.\n";

char *noInsSearchDir =
"\n\tNo *.il files were found in search directory\n\t%s.\n";

char	*BosNotFound =
"\tThe bos.rte inslist was not found.\n";

char	*prereqInslistNotFound =
"\tThe inslist for prereq lpp/lpp option %s was not found.\n\
        This option was prereqed in the file %s.prereq.S.\n";

char *insOpenFailed =
"\topen failed on inslist %s\n";

char    *OpenError =
"\tCould not open file \"%s\" for access '%c'.\n";

char	*linkCrossFileSys =
"\tThe %s hard link for target file\n\
	%s should not cross base filesystems.\n";

char	*uidNotFound =
"\tuser id %d for inslist entry %s\n\
	was not found in the system tables.\n";

char	*gidNotFound =
"\tgroup id %d for inslist entry %s\n\
	was not found in the system tables.\n";

char	*invalidTableEnt =
"\tThe following entry in the id table file is invalid:\n\
	%s\n\tThe entry will be ignored.\n";

char	*dbFileError =
"\tThe DB_FILE_IN environment variable used to specify an input\n\
	database file cannot be used with the -d option.\n";

char	*statFailed =
"\tThe stat system call failed on the ./ilpaths file, which should\n\
	have been generated by ilverify.\n";

char	*numInslistsExceededLimit =
"\tThe number of inslists exceeded the limit of %d.\n";

char	*numOptionsExceededLimit =
"\tThe number of options exceeded the limit of %d.\n";

char	*tcbFlagNotSetRoot =
"\tThe tcb flag for inslist entry %s \n\
        is not set for set-uid root.\n";

char	*tcbFlagNotSetSys =
"\tThe tcb flag for inslist entry %s \n\
        is not set for set-gid system.\n";
