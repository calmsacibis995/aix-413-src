$ @(#)52  1.3.3.2  src/bos/usr/sbin/bosboot/bosboot.msg, bosboot, bos411, 9430C411a 7/19/94 13:31:04
$
$ COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
$
$ FUNCTIONS: bosboot.msg
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set 150
1 "\n0301-150 %s: Invalid or no boot device specified!"
2 "\nusage:\t%s -a [-d device] [-p proto] [-k kernel] [-l lvdev]\n\
\t\t[-b file] [-M serv|norm|both] [-D|-I] [-LOUq]\n\
\t%s -w file [-d device] [-q]\n\
\t%s -r file [-d device] [-l lvdev] [-M serv|norm|both] [-q]\n"
3 "\tWhere:\n"
6 "\t-U		Create uncompressed boot image.\n"
8 "\t-w file\t\tCopy given boot image file to device.\n"
9 "\t-a		Create boot image and write to device or file.\n"
14 "\t-p proto	Use given proto file for RAM disk file system.\n"
16 "\t-k kernel	Use given kernel file for boot image.\n"
17 "\t-l lvdev	Target boot logical volume for boot image.\n"
18 "\t-b file		Use given file name for boot image name.\n"
19 "\t-D		Load Low Level Debugger.\n"
20 "\t-I		Load and Invoke Low Level Debugger.\n"
21 "\t-M norm|serv|both	Boot mode - normal or service.\n"
22 "\t-O offset	boot image offset for CDROM file system.\n"
23 "\t-q		Query disk space required to create boot image.\n"
24 "\t-d device	Device for which to create the boot image.\n"
25 "\t-L		Enable MP locks instrumentation.\n"
30 "\n0301-152 %s: not enough file space to create:"
31 "\n\t %s \n"
32 "\t %s has %s free KB.\n"
33 "\t %s needs %s KB.\n"
40 "\n0301-153 %s: %s not found.\n"
41 "\n0301-154 %s: missing proto file: %s\n"
42 "\n0301-155 %s: mkfs failed for %s.\n"
43 "\n0301-156 %s: cp of %s failed.\n"
44 "\n0301-157 %s: strip of %s failed.\n"
$ Do not translate "strip"
45 "\n0301-158 %s: mkboot failed to create %s.\n"
46 "\n0301-159 %s: Either %s or %s is missing!\n"
48 "\n0301-161 %s: dd failed to copy %s.\n"
49 "\n0301-162 %s: savebase failed with %s.\n"
51 "\n%s: Boot image is %s 512 byte blocks.\n"
52 "\n0301-166 %s: Device not ready: %s.\n"
54 "\n0301-168 %s: The current boot logical volume, %s,\n\tdoes not exist on %s.\n"
55 "\nFilesystem\t"
$ Keep the width of this field the same if possible
56 "KB required\n"
$ Keep the width of this field the same if possible
58 "%s\t\t%s\n"
60 "\n0301-170 %s: -w is invalid for device %s.\n"
62 "\n0301-172 %s: %s is not writable.\n"
64 "\n%s: Memory or boot logical volume size is too small.\n\t Bootimage will be compressed.\n"
66 "\n0301-174 %s: Invalid device %s specified!\n"
68 "\n0301-176 %s: update_proto failed to create a proto file.\n"
$ Do not translate "update_proto"
70 "bootimage"
$  This word must be a single word
100 "\n0301-165 %s: WARNING! %s failed - do not attempt to boot device.\n"

105 "\t-r file\t\tCreate ROS emulation network boot image.\n"
106 "\n0301-175 %s: -r is invalid for device %s.\n"
$ End of bosboot.msg
