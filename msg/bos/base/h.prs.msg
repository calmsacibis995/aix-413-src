$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)49        1.1 2/9/94 09:01:22
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote #
$set 4
1 #Specify the cutoff date in the following format:\n\
YY[MM[DD[HH[MM[SS]]]]]\n\
You must specify a cutoff date.\n\
Either no cutoff date was specified, or the format of the\n\
date was not intelligible.#
$set 5
1 #Do not specify both a cutoff date and an SID.\n\
The -c and -r flags are mutually exclusive.#
$set 6
1 #Specify the -e flag or the -l flag with the -c flag.\n\
You must specify whether you want earlier or later deltas than the cutoff date.#
$set 49
1 #Usage: [-a ...] [-dString ...] [-rSID [-e|-l]|-cCutoff {-e|-l}\n\
\t{File ...|Directory ...|-}\n\
For a description of recognized data keywords, key in the following:\n\
\tsccshelp prs_kywds\n#
