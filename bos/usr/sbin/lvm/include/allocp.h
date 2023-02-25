/* @(#)17       1.10  src/bos/usr/sbin/lvm/include/allocp.h, cmdlvm, bos41J, 9507A 2/2/95 16:59:52 */
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: chpv.sh
 *
 * ORIGINS: 27
 * 
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * [End of PROLOG]
 */

#define   FALSE 0
#define   LSIZE 150
#define   STRSIZE 25
#define   MAXPPS 1016
#define   MAXPV 32
#define   MAXLV MAXPV*MAXPPS /* cleaned up for > 2 GB work */
#define   MAXPP_PER_VG	MAXPV*MAXPPS
#define   MAXCOPIES   3     /* maximum number of mirror copies */
#define     MAX(a,b)  ( (a) >= (b) ? (a) : (b) )

#define   ASSIGN(pv1,pv2) { pv1.word1 = pv2.word1; pv1.word2 = pv2.word2; pv1.word3 = pv2.word3; pv1.word4 = pv2.word4; } 

#define   SAMEPV(p1,p2) ((p1.word1==p2.word1) && (p1.word2==p2.word2))

#define   SAMEASCIIPV(p1,p2) ( strncmp(p1,p2,16) == 0 )

typedef enum { MIN, MAX, MAP } SPREAD;
typedef enum { INNEREDGE, OUTEREDGE, INNERMIDDLE, OUTERMIDDLE, CENTER } REGION;
typedef enum { EXTEND, COPY, UNCOPY, EXPLICIT } TYPE;

typedef struct lv_id  LVID ; 

typedef struct {
	LVID         lvid;
	int          copies;
	int          strict;
	int          upperbound;
	SPREAD       spread;
        REGION        region;
} LVCHRS;

struct lv {
              int     pvindex;
              int     ppnum;
              int     lpnum;
       short  int     state;
       struct lv   *next;              
} ;

typedef struct lv LVMAP;

struct lvlist {
          LVMAP   *head;
          LVMAP   *tail; 
          int     num_of_lps;
          int     max_lp_num;
} ;

struct range {
	int	        start;
	int             length;
	int	        newstart;
	int             newlength;
        struct range   *newnext;
        int             intheregion;
        int             region_length;
	struct range   *next_range;
};
typedef struct range RANGE;

typedef struct {
	struct unique_id    pvid;
        char                asciipvid[17];
	int                 free_pp_count;
        int                 free_section_count;
	RANGE              *free_range_head;
	int		    safelpcount;
        int                 totallpcount;
        int                 org_lpcount;
        int                 lpcount[3];
        int                *lpnums;
        int                 highestppnum;
        int                 region_start; /* desired region start ppnum */
        int                 region_length;
} PV;

typedef  struct pp LVPP;

typedef struct unique_id ID;

struct lp_map  {
	 int       num_of_copies;    /* num of mirror copies of this LP */
	 struct lv *copy[MAXCOPIES];
};
