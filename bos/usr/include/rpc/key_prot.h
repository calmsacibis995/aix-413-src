/* static char sccsid[] = "@(#)67  1.7  src/bos/usr/include/rpc/key_prot.h, libcrpc, bos411, 9428A410j 10/25/93 20:47:19"; */
/*
 *   COMPONENT_NAME: LIBCRPC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 24
 *
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 * 1.4 87/03/10 Copyr 1986 Sun Micro 
 */

/*	@(#)key_prot.h	1.2 90/07/17 4.1NFSSRC SMI	*/


#ifndef _RPC_KEY_PROT_H
#define _RPC_KEY_PROT_H

/* 
 * Compiled from key_prot.x using rpcgen.
 * DO NOT EDIT THIS FILE!
 * This is NOT source code!
 */

#define KEY_PROG 100029
#define KEY_VERS 1
#define KEY_SET 1
#define KEY_ENCRYPT 2
#define KEY_DECRYPT 3
#define KEY_GEN 4
#define KEY_GETCRED 5

#define PROOT 3
#define HEXMODULUS "d4a0ba0250b6fd2ec626e7efd637df76c716e22d0944b88b"
#define HEXKEYBYTES 48
#define KEYSIZE 192
#define KEYBYTES 24
#define KEYCHECKSUMSIZE 16

enum keystatus {
	KEY_SUCCESS = 0,
	KEY_NOSECRET = 1,
	KEY_UNKNOWN = 2,
	KEY_SYSTEMERR = 3
};
typedef enum keystatus keystatus;
bool_t xdr_keystatus();

#ifndef _KERNEL

typedef char keybuf[HEXKEYBYTES];
bool_t xdr_keybuf();

#endif /* _KERNEL */

typedef char *netnamestr;
bool_t xdr_netnamestr();


struct cryptkeyarg {
	netnamestr remotename;
	des_block deskey;
};
typedef struct cryptkeyarg cryptkeyarg;
bool_t xdr_cryptkeyarg();


struct cryptkeyres {
	keystatus status;
	union {
		des_block deskey;
	} cryptkeyres_u;
};
typedef struct cryptkeyres cryptkeyres;
bool_t xdr_cryptkeyres();

#define MAXGIDS 16

struct unixcred {
	int uid;
	int gid;
	struct {
		u_int gids_len;
		int *gids_val;
	} gids;
};
typedef struct unixcred unixcred;
bool_t xdr_unixcred();


struct getcredres {
	keystatus status;
	union {
		unixcred cred;
	} getcredres_u;
};
typedef struct getcredres getcredres;
bool_t xdr_getcredres();

#endif /*!_RPC_KEY_PROT_H*/
