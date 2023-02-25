$ "@(#)83  1.16  src/bos/usr/sbin/cfgmgr/cfgmgr.msg, cmdcfg, bos411, 9428A410j 6/23/94 15:47:34"

$ 
$  COMPONENT_NAME: (CMDCFG) Generic config support cmds
$ 
$  FUNCTIONS: Makefile
$ 
$  ORIGINS: 27
$ 
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set 1
1	"****************** stdout ***********\n%s\n"
$ stdout is not to be translated

2	"****************** no stdout ***********\n"
$ stdout is not to be translated

3	"****************** stderr ***********\n%s\n"
$ stderr is not to be translated

4	"****************** no stderr ***********\n"
$ stderr is not to be translated

5	"cfgmgr is running in PHASE1\n"
$ cfgmgr & PHASE1 are not to be translated

6	"cfgmgr is running in PHASE2\n"
$ cfgmgr & PHASE2 are not to be translated

7	"cfgmgr is running in PHASE2MAINT\n"
$ cfgmgr & PHASE2MAINT are not to be translated

8	"cfgmgr : unknown phase = %d\n"
9	"invoking top level program -- \"%s\"\n"
10	"return code = %d\n"
11	"calling savebase\n"
$ savebase is not to be translated

12	"attempting to configure device '%s'\n"
13	"invoking %s %s\n"
14	"Method error (%s):\n"
15 ""
$ no longer used
16 ""
$ no longer used
17 ""
$ no longer used
18 "Packages to install %s...\n" 
$ Packages are filesets of software 
19 "Installing devices packages from: %s\n\twith devinstall switches:%s\n"
$ devinstall is not to be translated
$ switches is like "options"

$set 2

$ in the following, the word "cfgmgr" is not to be translated
1 "cfgmgr: 0514-600 Usage error : %s\nUsage: cfgmgr [-f|-s|-p phase] \
[-l name] [-v] [-i device]\n"
2 "cfgmgr: 0514-601 Flag conflict : %s\nUsage: cfgmgr [-f|-s|-p phase] \
[-l name] [-v] [-i device]\n"
3	"cfgmgr: 0514-602 Cannot initialize the ODM data base.\n"
$ ODM is not to be translated

4	"cfgmgr: 0514-603 Cannot access the Config_Rules object class in the\n\
\tdevice configuration database.\n"
$ Config_Rules is not to be translated

5	"cfgmgr: 0514-604 Cannot access the CuDv object class in the\n\
\tdevice configuration database.\n"
$ CuDv is not to be translated

6	"cfgmgr: 0514-605 Cannot access the CuDvDr object class in the\n\
\tdevice configuration database.\n"
$ CuDvDr is not to be translated

7	"cfgmgr: 0514-606 Running phase 1 at this point is not valid.\n"

8	"cfgmgr: 0514-607 There are no sequence rules to execute for the\n\
\tspecified phase.\n"
9	"cfgmgr: 0514-608 Cannot update an object in the device\n\
\tconfiguration database.\n"
10	"cfgmgr: 0514-609 Unable to save the base customized information\n\
\ton /dev/ipldevice.\n%s\n"
11	"cfgmgr: 0514-610 Cannot access the PdAt object class in the\n\
\tdevice configuration database.\n"
$ PdAt is not to be translated

12	"cfgmgr: 0514-611 There is not enough memory available now.\n"
13	"cfgmgr: 0514-612 Unable to configure the %s device because\n\
\tit either does not exist in the customized device database or it does not\n\
\thave a corresponding entry in the predefined device database.\n"
14	"cfgmgr: 0514-613 Unable to acquire the device configuration database\n\\tlock. The errno is %d.\n"
15	"cfgmgr: 0514-614 Warning - unable to determine the \"type_of_boot\"\n"
16	"cfgmgr: 0514-616 Unable to write the file %s\n"
17	"cfgmgr: 0514-617 Unable to open the file %s\n"
18	"cfgmgr: 0514-618 Bosboot verification failed. Resolve the problem and re-run bosboot\n"
19	"cfgmgr: 0514-619 Bosboot failed. Resolve the problem and re-run bosboot\n"
20	"cfgmgr: 0514-620 The following device packages are required for device\n\tsupport but were not found on the current installation,\n\tor encountered an error during installation.\n"
21	"cfgmgr: 0514-621 WARNING: The following device packages are required for\n\tdevice support but are not currently installed.\n"


