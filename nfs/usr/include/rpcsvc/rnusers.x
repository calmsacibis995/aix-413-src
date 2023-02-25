/* @(#)62	1.2  src/nfs/usr/include/rpcsvc/rnusers.x, cmdnfs, nfs411, GOLD410 5/4/90 09:45:58 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * (#)rnusers.x 1.3 88/02/08 Copyr 1987 Sun Micro 
 */

/*
 * Find out about remote users
 */

const MAXUSERS = 100;
const MAXUTLEN = 256;

struct utmp {
	string ut_line<MAXUTLEN>;
	string ut_name<MAXUTLEN>;
	string ut_host<MAXUTLEN>;
	int ut_time;
};


struct utmpidle {
	utmp ui_utmp;
	unsigned int ui_idle;
};

typedef utmp utmparr<MAXUSERS>;

typedef utmpidle utmpidlearr<MAXUSERS>;

program RUSERSPROG {
	/*
	 * Includes idle information
	 */
	version RUSERSVERS_IDLE {
		int
		RUSERSPROC_NUM(void) = 1;

		utmpidlearr
		RUSERSPROC_NAMES(void) = 2;

		utmpidlearr
		RUSERSPROC_ALLNAMES(void) = 3;
	} = 1;

	/*
	 * Old version does not include idle information
	 */
	version RUSERSVERS_ORIG {
		int
		RUSERSPROC_NUM(void) = 1;

		utmparr
		RUSERSPROC_NAMES(void) = 2;

		utmparr
		RUSERSPROC_ALLNAMES(void) = 3;
	} = 2;
} = 100002;
	
