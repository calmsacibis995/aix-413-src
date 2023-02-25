$ ENGLISH_SOURCE_VERSION= 1.10
$ @(#)28	1.1  src/msg/bos/base/udefif.msg, bosmsg, msg411, GOLD410 2/9/94 09:26:22
$
$ Messages for UDEFIF.c
$
$ COMPONENT_NAME:  CMDNET   Network Configuration 
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989.
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$ len 
$
$ Do not translate udefif.
$
$ Message numbers have been assigned a range of 118-132.
$
$set MS_UDEFIF        Define initial set#

CUATDEL "0821-118 udefif: Cannot delete from CuAt.\n"
$ Do not translate CuAt, udefif.
CUDEPDEL "0821-119 udefif: Cannot delete from CuDep.\n"
$ Do not translate udefif, CuDep.
CUDVDEL "0821-120 udefif: Cannot delete from CuDv.\n"
$ Do not translate udefif, CuDv.
GETCUDV "0821-121 udefif: Cannot get records from CuDv.\n"
$ Do not translate udefif, CuDv.
GETIF "0821-122 udefif: Expected one interface instance, retrieved %d.\n"
INETSTAT "0821-123 udefif: %s must not be in either the STOPPED or the AVAILABLE state.\n"
$ Do not translate udefif, STOPPED, AVAILABLE.
OPENCUAT "0821-124 udefif: Cannot open CuAt.\n"
$ Do not translate udefif, CuAt.
OPENCUDEP "0821-125 udefif: Cannot open CuDep.\n"
$ Do not translate udefif, CuDep.
OPENCUDV "0821-126 udefif: Cannot open CuDv.\n"
$ Do not translate udefif, CuDv.
USAGE1 "\nusage:\t%s  [-l interface] ...\n"
