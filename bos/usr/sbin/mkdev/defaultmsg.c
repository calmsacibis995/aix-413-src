static char sccsid[] = "@(#)70	1.1.1.4  src/bos/usr/sbin/mkdev/defaultmsg.c, cmdcfg, bos411, 9432B411a 8/11/94 12:58:12";
/*
 * COMPONENT_NAME: (CMDCFG)  Device configuration library
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*---------------------- high level command messages -----------------------*/
char *cmdcfg_msg[] =
{
"True",
"False",
"Defined",
"Stopped",
"Available",
"Unknown",
"%s Defined\n",
"%s Stopped\n",
"%s Available\n",
"%s changed\n",
"%s deleted\n",
"%d bytes were written to address 0x%x in %s\n",
"%s loaded into %s\n",
"too may attributes",
"\nUsage:\nmkdev {[-c Class][-s Subclass][-t Type][-l Name]} [-a Attribute=Value]...\n\
[-d|-S|-R][-p ParentName][-q][-w ConnectionLocation][-f File]\n\
mkdev -l Name [-h][-q][-S|-R][-f File]\n\
mkdev -h\n\n",
"\nUsage:\nchdev -l Name [-a Attribute=Value]...[-p ParentName][-P|-T]\n\
\t[-q][-w ConnectionLocation][-f File]\n\
chdev -h\n\n",
"\nUsage:\nrmdev -l name[-d|-S][-R][-q][-f File]\n\
rmdev -h\n\n",
"\nUsage:\nlsattr {-D[-O]| -E[-O] | -F Format} -l Name [-a Attribute]...[-H]\n\
\t[-f File]\n\
lsattr {-D[-O]| -F Format}{[-c Class][-s Subclass][-t Type]}[-a Attribute]...\n\
\t[-H][-f File]\n\
lsattr -R {-l Name | [-c Class][-s Subclass][-t Type]} -a Attribute [-H]\n\
\t[-f File]\n\
lsattr -h\n\n",
"\nUsage:\nlsconn -p ParentName [-l ChildName | -k ChildConnectionKey][-F format]\n\
\t[-H][-f file]\n\
lsconn {[-c ParentClass][-s ParentSubclass][-t ParentType]}\n\
\t[-l ChildName | -k ChildConnectionKey][-F format][-H][-f file]\n\
lsconn -h\n\n",
"\nUsage:\n\tlsdev -C [-c Class][-s Subclass][-t Type][-S State][-l Name]\n\
\t[-r ColumnName| -F Format][-H][-f File ]\n\
lsdev -P [-c Class][-s Subclass][-t Type][-r ColumnName| -F Format]\n\
\t[-H][-f File ]\n\
lsdev  -h\n\n",
"\nUsage:\nlsparent -P {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file]\n\
lsparent -C {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file]\n\
lsparent -h\n\n",
"\nUsage:\nnvload [-f filename] [-n section] [-t string]\n\n",
"True",
"Diagnose"
};

/*------------------------ high level command errors -----------------------*/
char *cmdcfg_err[] =
{
"%s: 0514-500 Usage error - %s\n",
"%s: 0514-501 Usage error - Conflicting flags: %s.\n",
"%s: 0514-502 Usage error\n\
\tFlag or parameter information is missing: %s.\n",
"%s: 0514-503 Usage error\n\
\tThe %s flag must be followed by a parameter.\n",
"%s: 0514-504 Cannot open file \"%s\".\n",
"%s: 0514-505 The request conflicts with the current state of device %s.\n",
"%s: 0514-506 There is not enough memory available now.\n",
"%s: 0514-507 Attribute %s cannot be displayed.\n",
"%s: 0514-508 Cannot save the base customized information\n\
\ton /dev/ipldevice.\n%s\n",
"%s: 0514-509 System error: %s\n",
"%s: 0514-510 Cannot write to a file. The errno is %d.\n",
"%s: 0514-511 Cannot read a file. The errno is %d.\n",
"%s: 0514-512 Cannot use more than 2 -n flags.\n",
"%s: 0514-513 The nvram section specified was not valid.\n\
\tUse a section number of either 1 or 2.\n",
"%s: 0514-514 nvram section conflict:\n\
\tUse a -n flag only when there is one file to load.\n",
"%s: 0514-515 No device driver files were specified.\n",
"%s: 0514-516 Device configuration database lock service timed out.\n",
"%s: 0514-517 The information provided to this configuration command\n\
\twas not adequate to uniquely identify the device.\n\
\tProvide more device information than the following:\n\t%s\n",
"%s: 0514-518 Cannot access the %s object class in the device\n\
\tconfiguration database.\n",
"%s: 0514-519 The following device was not found in the customized\n\
\tdevice configuration database:\n\t%s\n",
"%s: 0514-520 Cannot find information in the predefined device\n\
\tconfiguration database for the following:\n\t%s\n",
"%s: 0514-521 Cannot find information in the predefined device\n\
\tconfiguration database for the customized device %s.\n",
"%s: 0514-522 Cannot run the following program: '%s'\n",
"%s: 0514-523 Cannot perform the requested function because the\n\
\tspecified device does not have a define method.\n",
"%s: 0514-524 Cannot perform the requested function because the\n\
\tspecified device does not have a configure method.\n",
"%s: 0514-525 Cannot perform the requested function because the\n\
\tspecified device does not have a change method.\n",
"%s: 0514-526 Cannot perform the requested function because the\n\
\tspecified device does not have an undefine method.\n",
"%s: 0514-527 Cannot perform the requested function because the\n\
\tspecified device does not have a start method.\n",
"%s: 0514-528 The \"%s\" attribute does not exist in the predefined\n\
\tdevice configuration database.\n",
"%s: 0514-529 The \"%s\" column does not exist in the device configuration\n\
\tdatabase.\n",
"%s: 0514-530 Cannot display information about \"%s\" because\n\
\tthis type of attribute cannot be displayed.\n",
"%s: 0514-531 Internal error: %s\n",
"Method error (%s):\n",
"%s: 0514-500 Usage error - %s\n",
"%s: 0514-534 Cannot perform requested function because the \n\
\tspecified device is in the diagnose state. \n",
"%s: 0514-535 Cannot perform requested function because the \n\
\tparent device, %s, is in the diagnose state. \n"
};
