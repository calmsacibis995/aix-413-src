$ @(#)40	1.13.2.7  src/bos/usr/sbin/lsdev/cmdcfg.msg, cmdcfg, bos411, 9438A411a 9/16/94 15:41:54
$
$
$   COMPONENT_NAME: CMDCFG
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1989,1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
  
$ @(#)cmdcfg.msg	1.6  com/cmd/cfg,3.1,9008d.03 2/9/90 16:36:11
$  Do not translate any %s or (%s) found in this message catalog. $ 
$  These messages are to be used by the system manager for maintaining \n\
$ the hardware devices and pseudodevices attached to his computer.  He must \n\
$ have a certain basis of knowledge about the how this is accomplished and \n\
$ with the vocabulary, especially the concepts of the ODM, class, and \n\
$ and object. $ 
$ If some of the error messages seem generic, it is because in our \n\
$ system management architecture the routine which reports the error is a \n\
$ different process from the command which detected it, and it has no way \n\
$ to know the details of the error. 
$quote "
$set 1

$ do not document or translate "dummy"- it is for the build process
$ DUMMY1 "dummy"

$  This set contains only information messages. $ 
1	"False"
2	"Defined"
3	"Stopped"
4	"Available"
5	"Unknown"
6	"%s Defined\n"
7	"%s Stopped\n"
8	"%s Available\n"
9	"%s changed\n"
10	"%s deleted\n"
11	"%d bytes were written to address 0x%x in %s\n"
12	"%s loaded into %s\n"
13	"too may attributes"
14	"\nUsage:\nmkdev {[-c Class][-s Subclass][-t Type][-l Name]} [-a Attribute=Value]...\n\
\t[-d|-S|-R][-p ParentName][-q][-w ConnectionLocation][-f File]\n\
mkdev -l Name [-h][-q][-S|-R][-f File]\n\
mkdev -h\n\n"
15	"\nUsage:\nchdev -l Name [-a Attribute=Value]...[-p ParentName][-P|-T]\n\
\t[-q][-w ConnectionLocation][-f File]\n\
chdev -h\n\n"
16	"\nUsage:\nrmdev -l name[-d|-S][-R][-q][-f File]\n\
rmdev -h\n\n"
17	"\nUsage:\nlsattr {-D[-O]| -E[-O] | -F Format} -l Name [-a Attribute]...[-H]\n\
\t[-f File]\n\
lsattr {-D[-O]| -F Format}{[-c Class][-s Subclass][-t Type]}[-a Attribute]...\n\
\t[-H][-f File]\n\
lsattr -R {-l Name | [-c Class][-s Subclass][-t Type]} -a Attribute [-H]\n\
\t[-f File]\n\
lsattr -h\n\n"
18	"\nUsage:\nlsconn -p ParentName [-l ChildName | -k ChildConnectionKey][-F format]\n\
\t[-H][-f file]\n\
lsconn {[-c ParentClass][-s ParentSubclass][-t ParentType]}\n\
\t[-l ChildName | -k ChildConnectionKey][-F format][-H][-f file]\n\
lsconn -h\n\n"
19	"\nUsage:\nlsdev -C [-c Class][-s Subclass][-t Type][-S State][-l Name]\n\
\t[-r ColumnName| -F Format][-H][-f File ]\n\
lsdev -P [-c Class][-s Subclass][-t Type][-r ColumnName| -F Format]\n\
\t[-H][-f File ]\n\
lsdev  -h\n\n"
20	"\nUsage:\nlsparent -P {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file]\n\
lsparent -C {-k ChildConnectionKey | -l ChildName}[-F format][-H][-f file]\n\
lsparent -h\n\n"
21	"\nUsage:\nnvload [-f filename] [-n section] [-t string]\n\n"
22	"True"
23	"Diagnose"

$set 2

$ do not document or translate "dummy"- it is for the build process
$ DUMMY2 "dummy"

$  This set contains only error messages. $ 
$  The initial %s in each message is the name of the command. $ 

1	"%s: 0514-501 Usage error - Conflicting flags: %s.\n"
2	"%s: 0514-502 Usage error - \n\
\tFlag or parameter information is missing: %s.\n"
3	"%s: 0514-503 Usage error - \n\
\tThe %s flag must be followed by a parameter.\n"
4	"%s: 0514-504 Cannot open file \"%s\".\n"
5	"%s: 0514-505 The request conflicts with the current state of device %s.\n"
6	"%s: 0514-506 There is not enough memory available now.\n"
7	"%s: 0514-507 Attribute %s cannot be displayed.\n"
8	"%s: 0514-508 Cannot save the base customized information\n\
\ton /dev/ipldevice.\n%s\n"
9	"%s: 0514-509 System error: %s\n"
10	"%s: 0514-510 Cannot write to a file. The errno is %d.\n"
11	"%s: 0514-511 Cannot read a file. The errno is %d.\n"
12	"%s: 0514-512 Cannot use more than 2 -n flags.\n"
13	"%s: 0514-513 The nvram section specified was not valid.\n\
\tUse a section number of either 1 or 2.\n"
$ nvram is not to be translated

14	"%s: 0514-514 nvram section conflict:\n\
\tUse a -n flag only when there is one file to load.\n"
$ nvram is not to be translated

15	"%s: 0514-515 No device driver files were specified.\n"
16	"%s: 0514-516 Device configuration database lock service timed out.\n"
17	"%s: 0514-517 The information provided to this configuration command\n\
\twas not adequate to uniquely identify the device.\n\
\tProvide more device information than the following:\n\t%s\n"
18	"%s: 0514-518 Cannot access the %s object class in the device\n\
\tconfiguration database.\n"
19	"%s: 0514-519 The following device was not found in the customized\n\
\tdevice configuration database:\n\t%s\n"
20	"%s: 0514-520 Cannot find information in the predefined device\n\
\tconfiguration database for the following:\n\t%s\n"
21	"%s: 0514-521 Cannot find information in the predefined device\n\
\tconfiguration database for the customized device %s.\n"
22	"%s: 0514-522 Cannot run the following program: '%s'\n"
23	"%s: 0514-523 Cannot perform the requested function because the\n\
\tspecified device does not have a define method.\n"
$ method is not to be translated

24	"%s: 0514-524 Cannot perform the requested function because the\n\
\tspecified device does not have a configure method.\n"
$ method is not to be translated

25	"%s: 0514-525 Cannot perform the requested function because the\n\
\tspecified device does not have a change method.\n"
$ method is not to be translated

26	"%s: 0514-526 Cannot perform the requested function because the\n\
\tspecified device does not have an undefine method.\n"
$ method is not to be translated

27	"%s: 0514-527 Cannot perform the requested function because the\n\
\tspecified device does not have a start method.\n"
$ method is not to be translated

28	"%s: 0514-528 The \"%s\" attribute does not exist in the predefined\n\
\tdevice configuration database.\n"
29	"%s: 0514-529 The \"%s\" column does not exist in the device configuration\n\
\tdatabase.\n"
30	"%s: 0514-530 Cannot display information about \"%s\" because\n\
\tthis type of attribute cannot be displayed.\n"
31	"%s: 0514-531 Internal error: %s\n"
32	"Method error (%s):\n"
33	"%s: 0514-500 Usage error - %s\n"
34	"%s: 0514-534 Cannot perform requested function because the \n\
\tspecified device is in the diagnose state. \n"
35	"%s: 0514-535 Cannot perform requested function because the \n\
\tparent device, %s, is in the diagnose state. \n"

$set 3

$ do not document or translate "dummy"- it is for the build process
$ DUMMY3 "dummy"

$  This set contains only error messages. $ 
$  The initial %s in each message is the name of the command. $ 

$  The word "Method" is not to be translated

1	"\t0514-001 System error:\n"
2	"\t0514-002 Cannot initialize the ODM.\n"
$  ODM is not to be translated

3	"\t0514-003 Cannot obtain the device configuration database lock.\n"
4	"\t0514-004 Cannot open an object class in the device\n\
\t         configuration database.\n"
5	"\t0514-005 Cannot close an object class in the device\n\
\t         configuration database.\n"
6	"\t0514-006 Cannot retrieve an object from the device\n\
\t         configuration database.\n"
7	"\t0514-007 Cannot update an object in the device \n\
\t         configuration database.\n"
8	"\t0514-008 Cannot add an object to the device configuration\n\
\t         database.\n"
9	"\t0514-009 Cannot delete an object from the device\n\
\t         configuration database.\n"
10	"\t0514-010 Error returned from odm_run_method.\n"
11	"\t0514-011 Invalid arguments:\n"
12	"\t0514-012 Cannot open a file or device.\n"
13	"\t0514-013 Logical name is required.\n"
14	"\t0514-014 Device class, subclass, and type required.\n"
15	"\t0514-015 Parent or connection required.\n"
16	"\t0514-016 Parent or connection not required.\n"
17	"\t0514-017 The following attributes are not valid for the\n\
\t         specified device:\n"
18	"\t0514-018 The values specified for the following attributes \n\
\t         are not valid:\n"
19	"\t0514-019 This method does not support the -P flag.\n"
20	"\t0514-020 This method does not support the -T flag.\n"
21	"\t0514-021 This method does not support a parent or \n\
\t         connection change.\n"
22	"\t0514-022 The specified connection is not valid.\n"
23	"\t0514-023 The specified device does not exist in the \n\
\t         customized device configuration database.\n"
24	"\t0514-024 The specified predefined information does not exist \n\
\t         in the predefined device configuration database.\n"
25	"\t0514-025 Cannot perform the requested function because the\n\
\t         parent of the specified device does not exist.\n"
26	"\t0514-026 It is illegal to define another device of this type.\n"
27	"\t0514-027 The specified device is in the wrong state to\n\
\t         perform the requested function.\n"
28	"\t0514-028 Cannot perform the requested function because the\n\
\t         parent of the specified device is not in a correct state.\n"
29	"\t0514-029 Cannot perform the requested function because a\n\
\t         child device of the specified device is not in a correct state.\n"
30	"\t0514-030 Cannot perform the requested function because\n\
\t         the specified device is dependent on another device which is\n\
\t         not in a correct state.\n"
31	"\t0514-031 A device is already configured at the specified location.\n"
32	"\t0514-032 Cannot perform the requested function because the\n\
\t         specified device is dependent on another device which does\n\
\t         not exist.\n"
33	"\t0514-033 Cannot perform the requested function because the\n\
\t         specified device is dependent on an attribute which does\n\
\t         not exist.\n"
34	"\t0514-034 The following attributes do not have valid values:\n"
35	"\t0514-035 Cannot perform the requested function because of\n\
\t         missing predefined information in the device configuration\n\
\t         database.\n"
36	"\t0514-036 Cannot perform the requested function because of\n\
\t         missing customized information in the device configuration\n\
\t         database.\n"
37	"\t0514-037 Cannot generate a logical name.\n"
38	"\t0514-038 Error loading kernel extension.\n"
39	"\t0514-039 Error unloading kernel extension.\n"
40	"\t0514-040 Error initializing a device into the kernel.\n"
41	"\t0514-041 Error terminating device from driver.\n"
42	"\t0514-042 Error getting or assigning a major number.\n"
43	"\t0514-043 Error getting or assigning a minor number.\n"
44	"\t0514-044 Cannot make a special file.\n"
45	"\t0514-045 Error building a DDS structure.\n"
46	"\t0514-046 A file containing microcode or adapter software was\n\
\t         not accessable.\n"
47	"\t0514-047 Cannot access a device.\n"
48	"\t0514-048 Error downloading microcode or software.\n"
49	"\t0514-049 Error getting VPD for a device.\n"
50	"\t0514-050 Cannot perform the requested function because the\n\
\t         specified device was not detected.\n"
51	"\t0514-051 Device to be configured does not match the physical\n\
\t         device at the specified connection location.\n"
52	"\t0514-052 Bus resources could not be resolved for a device.\n"
53	"\t0514-053 Error returned from sys_config.\n"
54	"\t0514-054 There is not enough memory available now.\n"
55	"\t0514-055 Cannot release devno because it does not exist.\n"
56	"\t0514-056 Requested devno is already in use.\n"
57	"\t0514-057 File has wrong status.\n"
58	"\t0514-058 Cannot remove a special file.\n"
59	"\t0514-059 Cannot create symbolic link.\n"
60	"\t0514-060 Cannot create another process.\n"
61	"\t0514-061 Cannot find a child device.\n"
62	"\t0514-062 Cannot perform the requested function because the\n\
\t         specified device is busy.\n"
63	"\t0514-063 The specified device driver name is too long.\n"
64	"\t0514-064 Cannot perform the requested function because the\n\
\t         AIO kernel extension is permanent and cannot be unloaded.\n"
65	"\t0514-065 Cannot perform the requested instance number function.\n"
66	"\t0514-066 Parent required.\n"
67	"\t0514-067 There are no slots available for a device.\n"
68	"\t0514-066 Cause not known.\n"

$set 4

$  This set contains messages for the lsresource command. $
$  The word "lsresource" is not to be translated

1   "lsresource : Device configuration database could not be initialized.\n",
2   "lsresource : Device configuration database lock service timed out.\n",
3   "lsresource : Device configuration database could not be opened.\n",
4   "lsresource : Data could not be retrieved from the device configuration database\n",
5   "lsresource : Usage error\n\tUsage : lsresource [-a] -l Name\n",
6   "lsresource : Cannot open a required file.\n",
7   "lsresource : Bus type not supported\n",
8   "lsresource : The specified device was not found in the customized\n\
device configuration database.\n",
9   "lsresource : Cannot find information in the predefined device\n\
device configuration database for the specified device.\n",
10  "lsresource : This device is not a child of a bus, and therefore\n\
does not have any bus attributes.\n",
11  "lsresource : A parent device is not in the AVAILABLE state\n",
12  "lsresource : No attributes were found for this device\n",
13  "lsresource : Bad attribute detected\n",
14  "lsresource : Machine Device Driver ioctl failed\n",
15  "lsresource : Insufficient bus resources for all devices\n",
16  "lsresource : Insufficient virtual storage to proceed\n",
17  "Devices resolvable :\n",
18  "Devices not resolvable :\n",
19  "Attributes which user must change to resolve devices shown above :\n",
20  "REQUIRED VALUE",
21  "No attributes need be modified by the user\n",
22  "lsresource : This device has no bus resource attributes\n",
23  "TYPE",
24  "lsresource : Internal error\n",
25  "NONE",
26  "lsresource : No bus resources are assigned because device\n\
status is Defined\n",
27  "ADAPTER",
28  "ATTRIBUTE",
29  "S",
30  "G",
31  "CURRENT VALUE",
