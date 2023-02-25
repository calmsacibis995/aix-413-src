/* @(#)60	1.3  src/nfs/usr/include/rpcsvc/yppasswd.x, cmdnfs, nfs411, GOLD410 5/4/90 09:48:19 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *	(#)yppasswd.x	1.2 88/05/08 4.0NFSSRC SMI
 * 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * (#) from SUN 1.2
 */

/*
 * NIS password update protocol
 * Requires unix authentication
 */
program YPPASSWDPROG {
	version YPPASSWDVERS {
		/*
		 * Update my passwd entry 
		 */
		int
		YPPASSWDPROC_UPDATE(yppasswd) = 1;
	} = 1;
} = 100009;


struct passwd {
	string pw_name<>;	/* username */
	string pw_passwd<>;	/* encrypted password */
	int pw_uid;		/* user id */
	int pw_gid;		/* group id */
	string pw_gecos<>;	/* in real life name */
	string pw_dir<>;	/* home directory */
	string pw_shell<>;	/* default shell */
};

struct yppasswd {
	string oldpass<>;	/* unencrypted old password */
	passwd newpw;		/* new passwd entry */
};


