$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)73	1.1  src/msg/bos/base/mkfs.msg, bosmsg, msg411, GOLD410 2/9/94 09:07:51
$
$  COMPONENT_NAME: (CMDFS) commands that manipulate files
$
$  FUNCTIONS: message file
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ mkfs' messages   
$quote "         define quote character.

$len 
$set MS_MKFS_ERR        Define initial set#

$ Do not add new lines.  
$ $ Message numbers have been assigned a range of 201-219  
$ Do not translate mkfs.  

ErrExit_USAGE        "Usage: mkfs [-b BootProgram] [-i Inodes] [-l Label] [-o Options] [-p Prototype]\n\
\t[-s Size] [-v VolumeLabel] [-V vfs] {Device|Filesystem}\n"

ErrExit_BADSIZE      "mkfs: 0506-202 Specify a positive size.\n"

ErrExit_BADINODES    "mkfs: 0506-203 The number of inodes must be positive.\n"

ErrExit_NOVFS        "mkfs: 0506-204 Specify a vfs type.\n"
$ Do not translate vfs.  

ErrExit_HELPER       "mkfs: 0506-205 An internal software error has occurred.\n"

ErrExit_NOMEM        "mkfs: 0506-206 There is not enough memory available now.\n"

ErrExit_STRFAIL      "mkfs: 0506-207 The string functions failed.\n"

ErrExit_INTERNAL     "mkfs: 0506-208 An internal software error occurred.\n"

ErrExit_BADVFS       "mkfs: 0506-209 The vfs type specified is not recognized.\n"
$ Do not translate vfs.  

ErrExit_MOUNTED      "mkfs: 0506-210 The filesystem is currently mounted.\n"

ErrExit_ARGCONF      "mkfs: 0506-211 The copyboot and labelit options must not both be specified.\n"
$ Do not translate copyboot, labelit.  

ErrExit_NOBOOT       "mkfs: 0506-212 Specify a boot program.\n"

ErrExit_NOLABEL      "mkfs: 0506-213 Specify a label or volume label.\n"

ErrExit_BADDEV       "mkfs: 0506-214: "

$set MS_MKFS_MISC        Redefine set#

Misc_RELABEL         "Change the label on"

Misc_REPLACEBOOT     "Replace the boot program on"

Misc_DESTROY         "destroy"

Misc_IT              "it"



