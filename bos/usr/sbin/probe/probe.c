static char sccsid[] = "@(#)18	1.1  src/bos/usr/sbin/probe/probe.c, cmdfs, bos411, 9428A410j 5/9/91 03:26:18";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: probe_main
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 * In order to support high availability nfs, logredo saves and then
 * replays hanfs records (the latter via the nfs_dupsave() call).
 * This is only possible if nfs is loaded, of course, but logredo
 * must run whether nfs is there or not.  The AIXV3 loader won't
 * normally load an executable bound to unloaded system calls, so we
 * use deferred binding when building logredo.  This apparently
 * useless module then provides the connection to the actual nfs
 * call by importing it from the kernel name space (see probe.imp)
 * and exporting it to logredo (probe.exp).  logredo attempts to
 * load() this module if it has any ha-nfs records.  If the load
 * fails it assumes nfs is unavailable and discards the records.
 * If the load succeeds the necessary binding is performed automatically
 * and logredo can call nfs_dupsav() safely.
 */

int
probe_main()
{
	return 0;
}
