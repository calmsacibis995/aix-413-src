static char sccsid[] = "@(#)30	1.1  src/nfs/usr/sbin/rpc.statd/sm_twin.x, cmdnfs, nfs411, GOLD410 7/8/91 21:08:18";
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

const MAXHOSTNAMLEN = 34;

typedef string nametype<MAXHOSTNAMLEN>;

struct	sm_mon_parm {
	nametype host;
	nametype twin;
};

program SM_TWIN {
	version SM_TWIN_VERS {
		/*
		 * Register locking client on twin.
		 * This will register the hostname of the twin system.
		 * This procedure call should be received by the
		 * primary system.
		 */
		int
		SM_TWIN_REGISTER(string) = 1;
		/*
		 * Remove twin registration.
		 * Stop all passing of monitoring requests to the twin
		 * server.  Unmonitoring requests received after this
		 * should still be sent to the twin for help cleanup.
		 * This unmonitoring will continue until another twin
		 * host name is registered at the primary server.
		 * At that time all contact with the prior twin server
		 * will be stopped.
		 */
		int
		SM_TWIN_UNREGISTER(string) = 2;
		/*
		 * Tell the twin's statd about the host to be monitored
		 * When the twin's rpc.statd receives this request an
		 * entry will be entered in the /etc/sm directory on the
		 * twin.  An entry in the dynamic callback list will NOT
		 * be created for a host that is tracked this way.
		 * If the twin's rpc.lockd receives a lock request for
		 * a host that is already in the /etc/sm directory through
		 * the twin monitoring process, an entry will be placed
		 * in the dynamic list at that time.
		 */
		int
		SM_TWIN_MONITOR(sm_mon_parm) = 3;
		/*
		 * Tell the twin's statd that it should stop monitoring host.
		 * Decrement the reference count on this monitored host.
		 * If the reference count it equal to zero, remove the
		 * corresponding entry from the /etc/sm directory.  If
		 * the reference count is not zero, leave the /etc/sm
		 * entry.  This will occur if the local rpc.lockd daemon
		 * has requested monitoring on the same host.
		 */
		int
		SM_TWIN_UNMONITOR(string) = 4;
	} = 1;
} = 200001;

