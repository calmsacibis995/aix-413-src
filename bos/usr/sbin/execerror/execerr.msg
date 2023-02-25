$ @(#)95	1.6.2.1  src/bos/usr/sbin/execerror/execerr.msg, cmdcntl, bos41J, 9507C 2/8/95 08:10:18
$
$ COMPONENT_NAME: (CMDCNTL) system control commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1995
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ The execerror command is called automatically when exec cannot
$ load the exec'd program.  A list of error strings are passed
$ as parameters.  Each string contains an error number follow
$ by variable data which does not need translation.
$

$set MS_EXECERROR
$ Do not translate exec()
$ Leave the tabs at the start of the message...except for EXECERROR

$quote "

$
$ The following messages are for the error messages.
$       0509-021, 0509-022, 0509-022, 0509-023, 0509-024,
$       0509-025, 0509-026, 0509-027, 0509-028, 0509-029,
$       0509-030, 0509-031, 0509-032, 0509-033, 0509-034,
$       0509-035, 0509-036, 0509-037
$
$       $ XXXX-XXX                  message number
$       MSGID "XXXX-XXX ......."    messages
$       $ ......                    note for translating (if need)
$

$ 0509-021
TOOMANY         "\t0509-021 Additional errors occurred but are not reported.\n"
$ 0509-022
NOLIB           "\t0509-022 Cannot load library %s.\n"
$ This and following are the same error - first when library is a file, second
$ when library is an archive member.  Library refers to a shared library.  Use
$ the same translation as is used in LD and the description of shared libraries.
$ 0509-022
NOLIB1          "\t0509-022 Cannot load library %1$s[%2$s].\n"
$ following are the same error - first when library is a file, second
$ when library is an archive member.  Library refers to a shared library.  Use
$ the same translation as is used in LD and the description of shared libraries.
$ 0509-023
UNDEF           "\t0509-023 Symbol %1$s in %2$s is not defined.\n"
$ 0509-024
RLDBAD          "\t0509-024 Relocation entry number %1$s is defective in %2$s.\n"
$ 0509-025
$ relocation entry number refers to the entry in ldrel defined in loader.h
FORMAT          "\t0509-025 The %s file is not executable or not in correct XCOFF format.\n"
$ Do not translate XCOFF
$ 0509-026
EWAS            "\t0509-026 System error"
$ This is followed by colon
$ this message prefixes the output of perror. It represents an error number
$ set when a service used by the loader failed.
$ 0509-027
MEMBER          "\t0509-027 Member %s is not found or file is not an archive.\n"
$ archive refers to an archive file - see the ar command.
$ 0509-028
TYPE            "\t0509-028 Symbol %1$s used in %2$s type does not match exported type.\n"
$ import and export refer to symbols imported or exported.  See LD.
$ type refers to the symbol hashtype used by some compilers to guarantee that
$ the declarations of uses and definitions match.
$ 0509-029
ALIGN           "\t0509-029 Alignment of text does not match required alignment.\n"
$ do not translate 'loader domain'.
$ 0509-030
DOMCREAT         "\t0509-030 Insufficient permission to create loader domain %s\n"
$ do not translate 'loader domain'.
$ 0509-031
DOMADD		"\t0509-031 Insufficient permission to add shared object %1$s to loader domain %2$s\n"
$ 0509-031
DOMADD1		"\t0509-031 Insufficient permission to add shared object %1$s[%2$s] to loader domain %3$s\n"
$ error numbers correspond to the values in sys/ldr.h.
$ 0509-032
SPARE12         "\t0509-032"
$ 0509-033
SPARE13         "\t0509-033"
$ 0509-034
SPARE14         "\t0509-034"
$ 0509-035
SPARE15         "\t0509-035"
$ following two errors do not correspond to errors defined in sys/ldr.h so the
$ numbers are "at the end"
$ 0509-036
EXECERROR       "exec(): 0509-036 Cannot load program %s because of the following errors:\n"
$ 0509-037
SYSTEM          "\t0509-037 System error - error data is: %s\n"
$ error data may contain text - which should be reported to the fix team.

