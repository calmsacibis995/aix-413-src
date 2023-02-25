$ @(#)51  1.3.1.6  src/bos/usr/sbin/savecore/savecr.msg, cmddump, bos411, 9428A410j 3/14/94 09:16:19 
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989 , 1993 
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for savecore  
$quote "         define quote character.

$len 
$set MS_SAVECR        Define initial set#
$ Message numbers have been assigned a range of 180-187
$ Do not translate savecore  
$ Do not translate savecr. 
$ Do not translate dump. 

REBOOT               "Reboot.\n"
$ Do not translate.   

BADMAGIC             "0481-180 The dump has a bad magic number.\n"

NOALLOC              "0481-182 There is not enough memory now for I/O buffers.\n"
$ Do not translate I/O 

BYTES                "0481-183 Saving %1$d bytes of system dump in %2$s\n"

$ %1$d will be replaced with a number of bytes.
$ %2$s will be replaced with full pathname of the file.

VERBOSE_BYTES        "0481-184 Saving %1$d bytes from device %2$d of image in %3$d/core%4$d.\n"

USAGE                "0481-186 Usage:\n\
savecore [-cfd] Directory [System]\n\n\
Save a core dump of the system and the kernel.\n\
-c	Does not copy the dump, but marks the dump as invalid\n\
-f	Copies the dump regardless of the validity\n\
-d 	Copies only the dump\n"

MARK_INVALID         "0481-187 Dump is marked invalid and is not copied.\n"

NO_RECENT_DUMP       "0481-188 Dump was marked invalid. Use -f to force copy.\n"

NO_SPACE1            "0481-189 Not enough space on %1$s to copy the dump and the %2$s.\n"

$ %1$s will be subtituted with the name of the directory.
$ %1$s will be subtituted with the name of the kernel.

NO_SPACE2            "0481-190 Not enough space on %1$s to copy the dump.\n"

CAT_NOT_A_DIR	     "0481-191 %1$s is not a directory.\n"

NO_DUMP  	     "0481-192 No dump available to copy.\n"

SYS_BYTES            "0481-193 Saving %1$s in %2$s\n"

CAT_COPY_SUCCESS     "0481-194 Successfully copied the dump from %1$s to %2$s.\n"

CAT_COPY_FAIL        "0481-195 Failed to copy the dump from %1$s to %2$s.\n"

CAT_ODM_INIT_ERR     "0481-196 Unable to initialize an ODM session.\n"      

CAT_GETATTR          "0481-197 Unable to get attribute %1$s from ODM object class SWservAt.\n"

ATTEMPT_COPY	"\
0481-198 Allowed the customer to copy the dump to external media.\n"
