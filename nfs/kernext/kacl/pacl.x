/* @(#)88       1.2  src/nfs/kernext/nfs/sec/pacl.x, sysxnfs, nfs410, nfs4.15293 4/24/90 13:19:56 */
/* 
 * COMPONENT_NAME: (SYSXNFS) Network File System Kernel
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
/*
 * Known types of access identifiers.
 */
enum access_id_type {
	ACCESS_USER_XDR	= 1,
	ACCESS_GROUP_XDR= 2
};

/*
 * How the access control entry handles permissions
 */
enum access_type {
	ACC_PERMIT_XDR	= 1,
	ACC_DENY_XDR	= 2,
	ACC_SPECIFY_XDR	= 3
};

/*
 * What permissions can be given by an ACL entry.
 */
enum access_permissions {
	ACL_EXECUTE	= 0x01,
	ACL_WRITE	= 0x02,
	ACL_READ	= 0x04
};

typedef access_permissions acc_permlist<>;

/*
 * What simple modes are in this file.  This is a copy of the bits from the
 * standard unix permission bits.  The ACL_IACL bit enables/disables the ACL.
 * These modes are copied into the bits looked at by stat and normal chmod
 * will change these also (except the ACL_IACL bit)
 */
enum acl_mode_type {
	ACL_IACL	= 0100000000,
	ACL_ISUID	= 0000004000,
	ACL_ISGID	= 0000002000,
	ACL_ISVTX	= 0000001000
};

/*
 * The diffent types of access controls assoiated data
 */
union access_identifier switch (access_id_type id_type) {
case ACCESS_USER_XDR:
	unsigned long uids<>;
case ACCESS_GROUP_XDR:
	unsigned long gids<>;
};

/*
 * ACL data.  Here is the type of access control, along with what permissions
 * are affected and the entries that this is good for
 */
struct acl_entry_xdr {
	access_type	ace_type;
	acc_permlist ace_access;
	access_identifier ace_entries<>;
};

/*
 * The actual ACL entry.  The first four fields correspond to the mode bits
 * normally returned by stat.  Next is the list of access control lists
 * associated with the file.
 */
struct acl_xdr {
	acl_mode_type acl_mode<>;
	acc_permlist u_access;
	acc_permlist g_access;
	acc_permlist o_access;
	acl_entry_xdr acl_entry<>;
};

/*
 * The various privileges that can be specified
 */
enum pcl_privileges {
	PCL_SET_OBJ_DAC		= 1,
	PCL_SET_OBJ_MAC		= 2,
	PCL_SET_OBJ_ACCT	= 3,
	PCL_SET_OBJ_TPATH	= 4,
	PCL_SET_OBJ_PRIV	= 5,
	PCL_SET_OBJ_STAT	= 6,
	PCL_SET_PROC_DAC	= 7,
	PCL_SET_PROC_ENV	= 8,
	PCL_SET_PROC_ACCT	= 9,
	PCL_SET_PROC_AUDIT	= 10,
	PCL_ACCT_CONFIG		= 12,
	PCL_AUDIT_CONFIG	= 13,
	PCL_LVM_CONFIG		= 14,
	PCL_FS_CONFIG		= 15,
	PCL_VMM_CONFIG		= 16,
	PCL_DEV_CONFIG		= 17,
	PCL_NET_CONFIG		= 18,
	PCL_DS_CONFIG		= 19,
	PCL_TPATH_CONFIG	= 20,
	PCL_RAS_CONFIG		= 21,
	PCL_SYS_AUDIT		= 22,
	PCL_SYS_CONFIG		= 23,
	PCL_SYS_OPER		= 24,
	PCL_BYPASS_DAC		= 25,
	PCL_BYPASS_DAC_READ	= 26,
	PCL_BYPASS_DAC_WRITE	= 27,
	PCL_BYPASS_ACCT		= 28,
	PCL_BYPASS_TPATH	= 29,
	PCL_BYPASS_DAC_KILL	= 30,
	PCL_SET_PROC_RAC	= 31
};

typedef pcl_privileges pcl_privlist<>;

/*
 * An entry has the privileges granted a process running this file and the
 * list of identifiers that the list applies to.
 */
struct pcl_entry_xdr {
	pcl_privlist pce_access;
	access_identifier pce_entries<>;
};

/*
 * Current modes of the PCL
 */
enum pcl_mode_type {
	PCL_PRIV_ENABLE = 1,
	PCL_PRIV_DISABLE = 2
};

/*
 * A PCL entry.  Current modes are that it is enabled or disabled,
 * we have default privileges given to a user if there is not a specified one
 * in the pcl_entry list.
 */
struct pcl_xdr {
	pcl_mode_type pcl_mode<>;		/* Enable, disabled, etc */
	pcl_privlist pce_access_default;	/* Default Privileges */
	pcl_entry_xdr pcl_entry<>;	/* List of specific entries */
};


/*
 * The client uses the NFS file handle to ask for the info on a file.
 * We do not care what is in here, so it is opaque.
 */
typedef opaque fh_cookie<>;	/* An NFS file handle */

/*
 * Since we use UDP it is possible that we will have to send the ACL/PCL list
 * in multiple packets over the wire.  We first XDR encode the data then we
 * send it over to the other side in block transfers.
 *
 * We checksum the ACL or PCL so that future read/write request blocks can
 * detect if the ACL/PCL specified has changed and the transaction has to
 * be restarted.  If the server crashes in the middle of a read request
 * from the client, the transaction should be able to restart where it left
 * off provided someone did not change the ACL/PCL.  If the server crashes
 * during the client write, the entire transaction will have to be restarted.
 *
 * Most transactions should work in less than 8K anyway, so the block transfer
 * will usually be only one block.
 */

/*
 * The magic cookie to pass back and forth identifing the file during the
 * transfer.
 */
struct acl_pcl_cookie {
	fh_cookie cookie;	/* An NFS file handle */
	opaque acl_pcl_id<>;	/* Check sum of the PCL/ACL XDR encoded */
	unsigned long size;	/* Number of bytes in the encoded buffer */
};

union rep_acl_pcl_cookie switch (int status) {
case 0:
	acl_pcl_cookie cookie;
default:
	int error;
};

/*
 * A block of data sent over the wire
 */
struct acl_pcl_block {
	acl_pcl_cookie cookie;	/* An NFS file handle */
	unsigned long offset;	/* Where in the XDR data encode block we are */
	unsigned long count;	/* Count of how many bytes in block */
	opaque acl_pcl_data<>;	/* The data bytes */
	bool eof;		/* This is the last buffer */
};

/*
 * A request for the block of data.
 */
struct acl_pcl_req_block {
	acl_pcl_cookie cookie;
	unsigned long offset;
	unsigned long count;
};

/*
 * The reply back.  We either have the data or an error code.  An error code
 * of "-1" means that we have to restart the transaction.
 */

const RESTART_TRANSACTION = -1;

union acl_pcl_reply switch (int status) {
case 0:
	acl_pcl_block block_reply;
default:
	int error;
};

/*
 * Short status telling the client if ACLs and PCLs are in use on the server.
 */
struct acl_pcl_inuse {
	int error;
	bool acl_inuse;
	bool pcl_inuse;
};

/*
 * The program definition.
 */
program ACL_PCL_PROG {
	version INITIAL {

		/*
		 * Stat ACL and PCL short format.  Just returns whether they
		 * are in use on the file.
		 */

		acl_pcl_inuse
		ACL_PCL_STAT(fh_cookie) = 1;
		/*
		 * Stat ACL long format which returns a checksum and length
		 */

		rep_acl_pcl_cookie
		ACL_STAT_LONG(fh_cookie) = 2;

		/*
		 * Stat PCL long format which returns a checksum and length
		 */

		rep_acl_pcl_cookie
		PCL_STAT_LONG(fh_cookie) = 3;

		/*
		 * Read an ACL
		 */
		acl_pcl_reply
		ACL_READ(acl_pcl_req_block) = 4;

		/*
		 * Read a PCL
		 */
		acl_pcl_reply
		PCL_READ(acl_pcl_req_block) = 5;

		/*
		 * Write an ACL (returns error code or RESTART_TRANSACTION)
		 */
		int
		ACL_WRITE(acl_pcl_block) = 6;

		/*
		 * Write an PCL (returns error code or RESTART_TRANSACTION)
		 */
		int
		PCL_WRITE(acl_pcl_block) = 7;
	} = 1;
} = 200006;
