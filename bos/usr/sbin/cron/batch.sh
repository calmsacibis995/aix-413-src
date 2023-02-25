#!/usr/bin/sh
# @(#)23	1.8  src/bos/usr/sbin/cron/batch.sh, cmdcntl, bos411, 9428A410j 11/19/92 18:35:29
#
# COMPONENT_NAME: (CMDCNTL) system control commands
#
# FUNCTIONS: batch
#
# ORIGINS: 3, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1992
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# restrictions as set forth in paragraph (b)(3)(B) of the Rights in
# Technical Data and Computer Software clause in DAR 7-104.9(a).
#
# batch: Runs commands at a later time.
#
# This change was made to comply with POSIX P1003.2
# 
at -q b -m now
