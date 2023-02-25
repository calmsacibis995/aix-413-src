$ @(#)45	1.2  src/bos/usr/sbin/autopush/autopush.msg, cmdpse, bos411, 9428A410j 5/18/91 20:01:09
$
$ COMPONENT_NAME: (CMDPSE) STREAMS commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for autopush.
$ Do not translate autopush.
$ Message numbers have been assigned a range of XXX - YYY
$
$quote "		define quote character.
$set MS_AUTOPUSH	Define initial set#

MSG1	"SAD open failed for %s\n"
MSG2	"SAD open failed for %s\n"
MSG3	"Program bug - bad arg/infop (%p)\n"
MSG4	"Invalid major (%d)\n", infop->sap_major
MSG5	"Major (%d) is not a STREAMS device\n"
MSG6	"Device (%d,%d) not configured for autopush\n"
MSG7	"Unexpected IOCTL(SAD_GAP) error: %d\n"
$ PRESERVE SPACING IN FOLLOWING TEXT
MSG8	"Major Minor Last Minor  %2d module(s)\n"
MSG9	" ALL   "
$ DO NOT TRANSLATE FOLLOWING LINE
MSG10	"infop->sap_cmd = %d\n"
MSG11	"More than %d modules in line %d.  Entry ignored\n"
MSG12	"\tLine in error: %.40s ...\n"
MSG13	"Inconsistent Major (%d), Minor (%d), Last_minor (%d) at line %d\n"
MSG14	"Can't push requested modules on STREAM for entry %d\n"
MSG15	"1 to %d module names required at line %d\n"
MSG16	"Invalid major (%d)\n"
$ PRESERVE SPACING IN FOLLOWING TEXT
MSG17	"\nModule          Available\n"
MSG18	"   Yes\n"
MSG19	"   No\n"
MSG20	"Unexpected IOCTL(SAD_VML) error (%d) checking module list.\n"
MSG21	"Device (%d,%d) already configured\n"
MSG22	"Program bug - bad arg/infop (%p)\n"
MSG23	"Major (%d) is not a STREAMS device\n"
MSG24	"Device (%d,%d) not configured for autopush\n"
MSG25	"Minor (%d) not first number in configured range\n"
MSG26	"No memory for autopush data structure\n"
MSG27	"Unexpected IOCTL(SAD_SAP/%s) error: %d\n"
MSG28	"Continuing...\n"
MSG29	"Program bug - bad arg/infop (%p)\n"
MSG30	"Major (%d) configured for ALL minors. Use \"-m0\" for minor.\n"
MSG31	"Major (%d) is not a STREAMS device\n"
MSG32	"Device (%d,%d) not configured for autopush\n"
MSG33	"Minor (%d) not first number in configured range\n"
MSG34	"No memory for autopush data structure\n"
MSG35	"Unexpected IOCTL(SAD_SAP/CLEAR) error: %d\n"
MSG36	"str_to_major: couldn't open '%s', %s\n"
MSG37	"str_to_major: couldn't PUSH 'sc', %s\n"
MSG38	"str_to_major: ioctl to get major number for '%s' failed, %s\n"
USAGE1	"-f filename\n       autopush -r -M major -m minor\n       autopush -g -M major -m minor"
