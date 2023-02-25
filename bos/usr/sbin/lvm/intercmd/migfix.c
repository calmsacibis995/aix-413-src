static char sccsid[] = "@(#)07	1.7  src/bos/usr/sbin/lvm/intercmd/migfix.c, cmdlvm, bos411, 9428A410j 9/4/91 12:40:55";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	main,findfreepp,findnolap,breakcycle,printmove,bldmovepp,bldfreepp,
 *	readmovepp,readfreepp,allocfreepp,allocmovepp,putmovepp,putfreepp,
 *	freemovepp
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * IDENTIFICATION:
 *    Name:	migfix.c
 *    Title:	Build PP Migration Path
 *
 * PURPOSE:
 *    To build a set of migration moves that will move all source
 *    pps to there destinations without over-writing any pps.
 * 
 * SYNTAX:
 *   migfix freepp_list migratepp_list migrate_moves
 *   Arguments:
 *	freepp_list - list of free pps for use in migration.
 *	migratepp_list - list of migrations in source destination format.
 *	migrate_moves - list of legal moves to accomplish migration.
 *
 * INPUT/OUTPUT SECTION:
 *	ascii files
 * 	    argv[1]	free pp list
 *	    argv[2]     migrate pp list
 *	    argv[3]	legal migrate moves
 *
 * PROCESSING:
 *   1. Read in free pp map. Create free pp list.
 *   2. Read in migrate map, delete any move where source and destination 
 *	are the same. Create migrate pp list.
 *
 *   While there are moves in the migrate map
 *    {
 *	If the move is a move to a free pp
 *	    a. move source to destination.
 *	    b. remove destination from free pp list.
 *	    c. place source on free pp list.
 *	    d. continue.
 *
 *	If the move is a move whose destination is not free but it is not a source to 
 *          another move
 *	    a. move destination to any free pp.
 *	    b. remove free pp destination from free pp list.
 *	    c. move source to destination.
 *	    d. place source on free pp list.
 *	    e. continue.
 *
 *	There is a cycle of moves
 *	    a. move destination to a free pp.
 *             a.1 put the move from the free pp to the destination's destination
 *                 in the moves list
 *	    b. move source to destination.
 *	    c. place source on free pp list.
 *	    d. remove free pp destination from free pp list.
 *	    e. continue.
 *    }
 *
 * PROGRAMS/FUNCTIONS CALLED:
 *
 * OTHER:
 * 	The freepp list and movepp (migrate) list are both doublely linked
 *	lists.
 *
 * RETURNS:
 *	0 - success
 *	failure
 *		1 - usage
 *		2 - could not open free map file
 *		3 - could not open migrate map file
 *		4 - empty free map
 *		5 - empty migrate map (or no moves required)
 *		6 - could not open legal moves file
 *		7 - memory allocation error
 *
 */

#include <stdio.h>
#include <memory.h>
#include <sys/types.h>

/* physical partition id */
struct ppid
{
	long ppnum;
	long pvid[2];
};
#define PPIDSZ sizeof(struct ppid)

/* free list element */
struct freepp
{
	struct ppid ppid;
	struct freepp *nextpp;
	struct freepp *prevpp;
};
#define FREEPPSZ sizeof(struct freepp)

/* migration move element */
struct movepp
{
	struct ppid from_ppid;
	struct ppid to_ppid;
	struct movepp *nextpp;
	struct movepp *prevpp;
};
#define MOVEPPSZ sizeof(struct movepp)

struct movepp *frstmovepp;	/* head of migration move list */
struct movepp *lastmovepp;	/* tail of migration move list */
struct freepp *frstfreepp;	/* head of free pp list */
struct freepp *lastfreepp;	/* tail of free pp list */

#define ppcmp(x,y) memcmp(x,y,PPIDSZ)	/* compair two physical partition ids */
#define ppcp(x,y) memcpy(x,y,PPIDSZ)	/* copy one physical partition id */

FILE *freelist;
FILE *movelist;
FILE *moves;

/* declare our internal functions */
int breakcycle();
void printmove();
void bldmovepp();
void bldfreepp();
void allocfreepp();
void allocmovepp();
void putmovepp();
void putfreepp();
void freemovepp();
int findfreepp();
int findnolap();
int readmovepp();
int readfreepp();


main(argc,argv)
int argc;
char *argv[];
{
	
	/* all command line arguments specified? */
	if(argc!=4)
		exit(1);

	/* build the free list */
	bldfreepp(argv[1]);
	/* is the free list empty? */
	if(frstfreepp==(struct freepp *)0)
		exit(4);

	/* build the migrate list */
	bldmovepp(argv[2]);
	/* is the migrate list empty? */
	if(frstmovepp==(struct movepp *)0)
		exit(5);

	/* open the output legal moves file */
	moves=fopen(argv[3],"w");
	if(moves==NULL)
		exit(6);

	/* make moves while we have moves to be made */
	while(frstmovepp!=(struct movepp *)0)
	{
		/* Make a free move and continue */
		if(findfreepp())
			continue;
		
		/* we don't have a free move so make a non overlapping move */
		if(findnolap())
			continue;

		/* we don't have a free move or a non-overlapping move so
		 * we must have a cycle of moves (break it).
		 */
		if ( ! breakcycle() ) exit (7)  ;
		
	}

	fclose(moves);
	exit(0);
}

/* find and make a free move and make it */
int findfreepp()
{
struct freepp *freepp;
struct movepp *movepp;

	/* search through the list of migrate moves and find a free move */
	for(freepp=frstfreepp; freepp != (struct freepp *) 0; freepp=freepp->nextpp)
		for(movepp=frstmovepp; movepp != (struct movepp *) 0; movepp=movepp->nextpp)
			/* is the current move a free move? */
			if (ppcmp(&freepp->ppid,&movepp->to_ppid) == 0)
			{
				/* make our free move */
				printmove(&movepp->from_ppid,&movepp->to_ppid);
				/* place the source ppid on the free list */
				ppcp(&freepp->ppid,&movepp->from_ppid);
				/* update the migrate move list */
				freemovepp(movepp);
				return(TRUE);
			}
	return(FALSE);
}

/* find a non-overlapping move and make it */
int findnolap()
{
	struct movepp *from_movepp;
	struct movepp *to_movepp;

	/* search through the list of migrate moves for a non-overlapping move */
	for(from_movepp=frstmovepp; from_movepp != (struct movepp *) 0; from_movepp=from_movepp->nextpp)
	{
		for(to_movepp=frstmovepp; to_movepp != (struct movepp *) 0; to_movepp=to_movepp->nextpp)
		{
			/* does the proposed move overlap? */
			if (ppcmp(&to_movepp->from_ppid,&from_movepp->to_ppid) == 0)
				break;
		}

		/* do we have a move that did not overlap? */
		if(to_movepp==(struct movepp *)0)
		{
			/* move the destination to the first free pp */
			printmove(&from_movepp->to_ppid,&frstfreepp->ppid);
			/* make our move */
			printmove(&from_movepp->from_ppid,&from_movepp->to_ppid);
			/* place the source ppid on the free list */
			ppcp(&frstfreepp->ppid,&from_movepp->from_ppid);
			/* update the migrate move list */
			freemovepp(from_movepp);
			return(TRUE);
		}
	}
	return(FALSE);
}

/* Find the destination given a source , needed to break cycles */

struct ppid     *destination(source)
	struct ppid     *source;
{
struct movepp        *moveptr ;

for(moveptr=frstmovepp; moveptr != (struct movepp *)0; moveptr=moveptr-> nextpp) {
	if ( ppcmp(source,&moveptr->from_ppid) == 0 ) {
        		    return(&moveptr->to_ppid) ;
        }
}
return((struct ppid *)0) ;
}

/* break any cycle by making any move */
int breakcycle()
{
struct movepp       newmove;
struct ppid        *destination(),*dest;

	/* move the destination to the first free pp */
	printmove(&frstmovepp->to_ppid,&frstfreepp->ppid);
	ppcp(&newmove.from_ppid,&frstfreepp->ppid);
	dest = destination(&frstmovepp->to_ppid);
        if( dest == (struct ppid *)0 ) return(0) ;
	ppcp(&newmove.to_ppid,dest);
        newmove.nextpp = (struct movepp *) 0;
        newmove.prevpp = (struct movepp *) 0;
	allocmovepp(&newmove);

	/* make our move */
	printmove(&frstmovepp->from_ppid,&frstmovepp->to_ppid);
	/* place the source ppid on the free list */
	ppcp(&frstfreepp->ppid,&frstmovepp->from_ppid);
	/* update the migrate move list */
	freemovepp(frstmovepp);
        return (1) ;
}

/* print out our legal move */
void printmove(from_ppid,to_ppid)
struct ppid *from_ppid;
struct ppid *to_ppid;
{

	fprintf(moves,"%8.8x%8.8x:%ld:%8.8x%8.8x:%ld\n",
		from_ppid->pvid[0],
		from_ppid->pvid[1],
		from_ppid->ppnum,
		to_ppid->pvid[0],
		to_ppid->pvid[1],
		to_ppid->ppnum);
}

/* build the migrate move list */
void bldmovepp(movefile)
char *movefile;
{
	struct movepp movepp;

	/* open our migrate move file */
	movelist=fopen(movefile,"r");
	if(movelist==NULL)
		exit(2);

	/* read in all migrate moves and add them to the list */
	while(readmovepp(&movepp)!=EOF)
	{
		/* throw out any non-moves
		 *   - moves where source and destination are the same
		 */
		if(ppcmp(&movepp.from_ppid,&movepp.to_ppid)==0)
			continue;

		/* place the new move on the migrate move list */
		allocmovepp(&movepp);
	}
	
	fclose(movelist);
}

/* build the free pp list */
void bldfreepp(freefile)
char *freefile;
{
	struct freepp freepp;

	/* open our free pp file */
	freelist=fopen(freefile,"r");
	if(freelist==NULL)
		exit(3);

	/* read in all free pps and add them to the list */
	while(readfreepp(&freepp)!=EOF)
	{
		/* add the new freepp to the list */
		allocfreepp(&freepp);
	}

	fclose(freelist);
}

/* read in one migrate move */
int readmovepp(movepp)
struct movepp *movepp;
{
	int rc;
	rc=fscanf(movelist,"%8x%8x:%ld:%8x%8x:%ld\n",
		&movepp->from_ppid.pvid[0],
		&movepp->from_ppid.pvid[1],
		&movepp->from_ppid.ppnum,
		&movepp->to_ppid.pvid[0],
		&movepp->to_ppid.pvid[1],
		&movepp->to_ppid.ppnum);
	return(rc);
}

/* read in one free pp */
int readfreepp(freepp)
struct freepp *freepp;
{
	int rc;
	rc=fscanf(freelist,"%8x%8x:%ld\n",
		&freepp->ppid.pvid[0],
		&freepp->ppid.pvid[1],
		&freepp->ppid.ppnum);
	return(rc);
}

/* allocate one freepp element,
 * copy the freepp information to the allocated element,
 * and add it to the end of the free pp list
 */
void allocfreepp(freepp)
struct freepp *freepp;
{
	struct freepp *pfreepp;

	pfreepp=(struct freepp *)malloc(FREEPPSZ);
	if(pfreepp==NULL)
		exit(7);

	bzero(pfreepp,FREEPPSZ);
	ppcp(&pfreepp->ppid,freepp);
	putfreepp(pfreepp);
}

/* allocate one movepp element,
 * copy the movepp information to the allocated element,
 * and add it to the end of the migrate move list
 */
void allocmovepp(movepp)
struct movepp *movepp;
{
	struct movepp *pmovepp;

	pmovepp=(struct movepp *)malloc(MOVEPPSZ);
	if(pmovepp==NULL)
		exit(7);

	bzero(pmovepp,MOVEPPSZ);
	ppcp(&pmovepp->from_ppid,&movepp->from_ppid);
	ppcp(&pmovepp->to_ppid,&movepp->to_ppid);
	putmovepp(pmovepp);
}

/* put migrate move on the end of the migrate move list */
void putmovepp(movepp)
struct movepp *movepp;
{

	/* empty list? */
	if(frstmovepp==(struct movepp *) 0)
	{
		frstmovepp=movepp;
		lastmovepp=movepp;
	}
	else
	{
		lastmovepp->nextpp=movepp;
		movepp->prevpp=lastmovepp;
		lastmovepp=movepp;
	}
}

/* put free pp on the end of the free pp list */
void putfreepp(freepp)
struct freepp *freepp;
{

	/* empty list? */
	if(frstfreepp==(struct freepp *) 0)
	{
		frstfreepp=freepp;
		lastfreepp=freepp;
	}
	else
	{
		lastfreepp->nextpp=freepp;
		freepp->prevpp=lastfreepp;
		lastfreepp=freepp;
	}
}

/* remove a migrate move from the list of migrate moves (update pointers) */
void freemovepp(movepp)
struct movepp *movepp;
{

	if(frstmovepp==movepp)
	{
		frstmovepp=movepp->nextpp;
		if(frstmovepp != (struct movepp *) 0)
			frstmovepp->prevpp=(struct movepp *) 0;
	}
	else
		movepp->prevpp->nextpp=movepp->nextpp;

	if(lastmovepp==movepp)
	{
		lastmovepp=lastmovepp->prevpp;
		if(lastmovepp != (struct movepp *) 0)
			lastmovepp->nextpp=(struct movepp *) 0;
	}
	else
		movepp->nextpp->prevpp=movepp->prevpp;
		
	free((char *)movepp);
}
