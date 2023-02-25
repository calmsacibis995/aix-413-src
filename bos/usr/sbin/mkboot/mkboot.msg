$ @(#)60  1.5  src/bos/usr/sbin/mkboot/mkboot.msg, bosboot, bos411, 9428A410j 3/10/94 15:05:31
$
$ COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
$
$ FUNCTIONS:  mkboot.msg
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set 100
$ For all messages in this set, the first variable is the command name.
1 "\n usage: %s  -d device [[-i -s -l lvdev -p address]]\n"
2 "\t-f filesys {-k kernel | -e expander} [-D |-I] [-b -c -h -L -r -w]\n"
3 "\n0301-101 %1$s: Device open failure: %2$s\n"
$ Second variable is the name of the device for which the open failed.
4 "\n0301-102 %1$s: The ioctl call failed with return: %2$d\n"
$ Second variable is the value of the return code from the ioctl call.
$ ioctl stands for "io control" and is a standard UNIX term.
5 "\n0301-103 %1$s: error opening \"%2$s\"\n"
$ Second variable is name of device for which the open failed.
6 "\n0301-104 %s: An error occurred on a seek operation!\n"
$ A seek is a standard UNIX operation which positions a file for reading.
$ Deleted mesage 7.
8 "\n0301-106 %1$s: The malloc call failed for size: %2$d\n"
$ A malloc is a standard UNIX memory allocation call. The second
$  variable is the amount of memory requested in bytes.
9 "\n0301-107 %1$s: \"%2$s\" is NOT a properly formatted load module.\n"
$ Second variable is the name of a file which is supposed to contain
$  a load module in a specific format.
10 "\n0301-108 %1$s: Unable to read file blocks. Return code: %2$d\n"
$ The command was unable to read a block(s) from an input file.
$  The second variable is the code returned on the read call.
11 "\n0301-109 %1$s: Unable to write file blocks. Return code: %2$d\n"
$ The command was unable to write a block(s) to an output file.
$  The second variable is the code returned on the write call.
12 "\n0301-111 %1$s: error on Object Data Manager routine \"%2$s\"\n"
$ An error occurred when accessing the Object Data Manager. The second
$  variable is the name of the subroutine which returned the error.
$  The Object Data Manager is an AIX specific facility intended for
$  storing system information.
99 "\n0301-110 %s: Unknown failure!\n"
$ The error routine for this command was called with an error code that
$  does not match any of the known error codes.

$ End of mkboot.msg
