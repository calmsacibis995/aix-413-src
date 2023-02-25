static char sccsid[] = "@(#)90  1.8  src/rspc/usr/bin/battery/battery.c, pwrcmd, rspc41J, 9520A_all 5/15/95 14:44:57";
/*
 *   COMPONENT_NAME: PWRCMD
 *
 *   FUNCTIONS: show_battery_info,discharge_battery
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include    <locale.h>
#include    <stdio.h>
#include    <errno.h>

#include    <sys/pm.h>
#include    <pmlib.h>

/*
 *      Message Catalog stuffs.
 */

#include    <nl_types.h>
#include    "battery_msg.h"
#include    "battery_msg_default.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_BATTERY, Num, Str)


/*
 *      Define Macros
 */

/* for battery status */

#define BATTERY_STATUS_UNKNOWN        0
#define BATTERY_STATUS_CHARGING	      1
#define BATTERY_STATUS_FULLY_CHARGED  2
#define BATTERY_STATUS_DISCHARGING    3
#define BATTERY_STATUS_IN_USE         4


#ifdef PM_DEBUG
#define Debug0(format)                fprintf(stderr,format)
#define Debug1(format,arg1)           fprintf(stderr,format,arg1)
#define Debug2(format,arg1,arg2)      fprintf(stderr,format,arg1,arg2)
#define Debug3(format,arg1,arg2,arg3) fprintf(stderr,format,arg1,arg2,arg3)
#else 
#define Debug0(format)                
#define Debug1(format,arg1)           
#define Debug2(format,arg1,arg2)      
#define Debug3(format,arg1,arg2,arg3) 
#endif


int dflag = 0;
int errflg = 0;
extern int errno;

/*
 *      Prototype
 */

int show_battery_info();
int discharge_battery();

/*
 *      Main
 */

main(int argc, char *argv[])
{
    int ch;
    int rc = 0;
    char invalid_option[256];   
 
    memset(invalid_option,0,sizeof(invalid_option));

    /* open message catalogue */
    setlocale(LC_ALL, "");
    catd = catopen(MF_BATTERY, MS_BATTERY);

    /* show battery information in case no flag specified */
    switch( argc ) {
    case 1:
        if(show_battery_info() != TRUE ){
            rc = 1;
        }
        else{
            rc = 0;
        }
	break;
    case 2:
	if (strcmp(argv[1],"-d") != 0){
            fprintf(stderr, MSGSTR(ERR_INVAL_OPT,DFLT_ERR_INVAL_OPT),
                    argv[1]);
            fprintf(stderr, MSGSTR(ERRMSG1,DFLT_ERRMSG1));
            rc=1;
	} 
	else {
            /* dflag */
            if( discharge_battery() != TRUE ){
                rc = 1;
            }		
            else{	
                rc = 0;	
            }
	}	
	break;
    default:
            fprintf(stderr, MSGSTR(ERR_INVAL_OPT,DFLT_ERR_INVAL_OPT),
                    argv[1]);
            fprintf(stderr, MSGSTR(ERRMSG1,DFLT_ERRMSG1));
            rc=1;
	break;
        /* parse end */
    }

    catclose(catd);                           /* close mesage catalog        */
    exit(rc);
}


/*
 * NAME:  show_battery_info
 *
 * FUNCTION:  Show Battery Information
 *            
 *            
 * NOTES:
 *
 * DATA STRUCTURES:
 *        pm_battery - PM battery structure
 *
 * RETURN VALUE DESCRIPTION:  
 *        TRUE  - Successfully processed
 *        FALSE - Failed to call pmlib_request_battery()
 *
 */
int show_battery_info()
{
    pmlib_battery_t p;
    int rc = TRUE;
    int status = BATTERY_STATUS_UNKNOWN;


    if(pmlib_request_battery(PMLIB_QUERY_BATTERY,&p) != PMLIB_SUCCESS ) {
        if( errno == ESRCH ) {
            fprintf(stderr, MSGSTR( ERRMSG5,DFLT_ERRMSG5));
        }
        else {
            fprintf(stderr,MSGSTR( ERRMSG3, DFLT_ERRMSG3));
        }
        rc = FALSE;
   }
   else {

#ifdef PM_DEBUG

       if (p.attribute & PMLIB_BATTERY_SUPPORTED){
           Debug0("PMLIB_BATTERY_SUPPORTED flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_EXIST){
           Debug0("PMLIB_BATTERY_EXIST flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_NICD){
           Debug0("PMLIB_BATTERY_NICD flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_CHARGING){
           Debug0("PMLIB_BATTERY_CHARGING flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_DISCHARGING){
           Debug0("PMLIB_BATTERY_DISCHARGING flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_AC){
           Debug0("PMLIB_BATTERY_AC flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_DC){
           Debug0("PMLIB_BATTERY_DC flag\n");
       }
       if (p.attribute & PMLIB_BATTERY_REFRESH_REQ){
           Debug0("PMLIB_BATTERY_REFRESH_REQ flag\n");
       }
#endif

       if(!( p.attribute & PMLIB_BATTERY_SUPPORTED )) {
           fprintf(stderr, MSGSTR( ERRMSG2, DFLT_ERRMSG2));
           rc = FALSE;
       }
       else if(!( p.attribute & PMLIB_BATTERY_EXIST )) {
           fprintf(stderr, MSGSTR( ERR_NO_BATTERY, DFLT_ERR_NO_BATTERY));
           rc = FALSE;
       }
       else{

           /* battery is supported and exists */

           /* charging or discharging or in use or fully charged */

           if( (p.attribute & PMLIB_BATTERY_AC) &&
               (p.attribute & PMLIB_BATTERY_CHARGING) &&
               !(p.attribute & PMLIB_BATTERY_DISCHARGING) 
                 ) {
               /* Charging */
	       status = BATTERY_STATUS_CHARGING;
	   }
           else if( (p.attribute & PMLIB_BATTERY_AC) &&
		   !(p.attribute & PMLIB_BATTERY_CHARGING) &&
		   !(p.attribute & PMLIB_BATTERY_DISCHARGING) &&
		   ((p.capacity==0 ? 0 :(int)(100.0*p.remain/p.capacity))>85)
		   ){
	       /* Fully Charged */
	       status = BATTERY_STATUS_FULLY_CHARGED;
	   }
           else if ( (p.attribute & PMLIB_BATTERY_AC) &&
                     !(p.attribute & PMLIB_BATTERY_CHARGING) &&
                     (p.attribute & PMLIB_BATTERY_DISCHARGING) ) {
               /* Discharging */
	       status = BATTERY_STATUS_DISCHARGING;
	   }
           else if ( (p.attribute & PMLIB_BATTERY_DC) &&
                     !(p.attribute & PMLIB_BATTERY_CHARGING) &&
                     (p.attribute & PMLIB_BATTERY_DISCHARGING) ) {
               /* In Use */
	       status = BATTERY_STATUS_IN_USE;
	   }
	   else{
               /* Unknown */
	       status = BATTERY_STATUS_UNKNOWN;
	   }

           /* now show the battery information */

	   if ( status != BATTERY_STATUS_UNKNOWN ){

	       /* NiCD or NiMH */
	       if( p.attribute & PMLIB_BATTERY_NICD) {
		   printf(MSGSTR( MSG1, DFLT_MSG1));
	       }
	       else {
		   printf(MSGSTR( MSG2, DFLT_MSG2));
	       }

	       switch( status ){
	       case BATTERY_STATUS_CHARGING:
		   /* Charging */
		   printf(MSGSTR( MSG3,DFLT_MSG3));
		   break;
	       case BATTERY_STATUS_FULLY_CHARGED:
		   /* Fully Charged */
		   printf(MSGSTR( FULL_CHARGE,DFLT_FULL_CHARGE));
		   break;
	       case BATTERY_STATUS_DISCHARGING:
		   /* Discharging */
		   printf(MSGSTR( MSG4,DFLT_MSG4));
		   printf(MSGSTR( DISC_QUANT,DFLT_DISC_QUANT), 
                      p.refresh_discharge_capacity,
                      p.capacity==0 ? 0 :
                      (int)(100.0*p.refresh_discharge_capacity/p.capacity)
                      );
		   printf(MSGSTR( MSG9,DFLT_MSG9), p.refresh_discharge_time);
		   break;
	       case BATTERY_STATUS_IN_USE:
		   /* in use */
		   printf(MSGSTR( MSG5, DFLT_MSG5));
		   break;
	       default:
		   break;
	       }
               printf(MSGSTR( MSG6, DFLT_MSG6),p.capacity);
	       printf(MSGSTR( REMAIN_CAPACITY, DFLT_REMAIN_CAPACITY),
		      p.remain,
		      (p.capacity==0 ? 0 :(int)(100.0*p.remain/p.capacity))
		      );
	       printf(MSGSTR( MSG10,DFLT_MSG10), p.full_charge_count);
	   }
           else{
               printf(MSGSTR( UNKNOWN_STATUS,DFLT_UNKNOWN_STATUS));
           }
       }
   }
   return(rc);
}


/*
 * NAME: discharge_battery
 *
 * FUNCTION:  Discharge the battery.
 *            
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:  
 *        TRUE - Successfully processed
 *        FALSE - Failed to call pmlib_request_battery()
 *
 */
int discharge_battery()
{
    pmlib_battery_t p;
    int rc = TRUE;

    if(pmlib_request_battery(PMLIB_QUERY_BATTERY,&p) != PMLIB_SUCCESS ) {
        if( errno == ESRCH ) {
            fprintf(stderr, MSGSTR( ERRMSG5, DFLT_ERRMSG5));
	    errno = 0;
        }
        else if( errno == EBUSY ) {
            fprintf(stderr, MSGSTR( ERR_BUSY, DFLT_ERR_BUSY));
	    errno = 0;
        }
        else {
            fprintf(stderr, MSGSTR( ERRMSG3, DFLT_ERRMSG3));
        }
        rc = FALSE;
   }
   else {
       if( !(p.attribute & PMLIB_BATTERY_EXIST) ) {
           fprintf(stderr, MSGSTR( ERR_NO_BATTERY,DFLT_ERR_NO_BATTERY));
           rc = FALSE;
       }
       else {
           if(!(p.attribute & PMLIB_BATTERY_DC) &&
              (p.attribute & PMLIB_BATTERY_AC) ) {

               Debug0("battery: attribute is AC and not DC\n");
               Debug0("now request battery discharge \n");

               if(pmlib_request_battery(PMLIB_DISCHARGE_BATTERY,&p)
                  != PMLIB_SUCCESS ) {
		   if( errno == EBUSY ) {
		       fprintf(stderr, MSGSTR( ERR_BUSY, DFLT_ERR_BUSY));
		       errno = 0;
		   }
                   fprintf(stderr, MSGSTR( ERRMSG4, DFLT_ERRMSG4));
                   rc = FALSE;
               }
               else{
                   fprintf(stderr, MSGSTR( DISCHARGE, DFLT_DISCHARGE));
                   rc = TRUE;
               }
           }
           else {
               fprintf(stderr, MSGSTR( ERRMSG4, DFLT_ERRMSG4));
               rc = FALSE;
           }
       }
   }
   return (rc);
}

