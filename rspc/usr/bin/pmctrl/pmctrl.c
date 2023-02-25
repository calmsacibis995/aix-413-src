static char sccsid[] = "@(#)93  1.18  src/rspc/usr/bin/pmctrl/pmctrl.c, pwrcmd, rspc41J, 9520A_all 5/15/95 10:25:03";
/*
 *   COMPONENT_NAME: PWRCMD
 *
 *   FUNCTIONS: main()
 *              set_on_off()
 *              set_duration()
 *              set_permission()
 *              set_action()
 *              daily_alarm_gmt()
 *              daily_alarm_localtime()
 *              set_pm_alarm()
 *              query_pm_alarm()
 *              check_time_value()
 *              is_number()
 *              start_action()
 *              query_duration()
 *              chkarg_action()
 *              query_action_setting()
 *              query_permission_setting()
 *              chkarg_duration()
 *              query_on_off_setting()
 *              chkarg_on_off()
 *              set_device_idle_time()
 *              query_device_idle_time()
 *              list_pm_dds()
 *              print_current_state()
 *              print_pm_setting()
 *              short_usage()
 *              long_usage()
 *              bye()
 *              init_args()
 *              parse()
 *              most_significant_state()
 *              find_graphical_output()
 *              pmlib2pmstate()
 *              pmlibonoff2boolean()
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

#include "pmctrl.h"

/*
 *  NAME:       main()
 *
 *  FUNCTION:   the pmctrl command
 *
 *  INPUTS:     argc, argv -- parameters for command line
 *
 */

void
main(int argc, char **argv)
{
    char   *smitenv;
    int    on_off;
    int    flag_status;

    /* open message catalog */

    setlocale( LC_ALL,"");
    catd = catopen( MF_PMCTRL, NL_CAT_LOCALE);

    /* odm initialization */
    
    if (odm_initialize() == PMCTRL_ERROR) {
        fprintf(stderr, MSGSTR(ERR_ODM_INIT, DFLT_ERR_ODM_INIT));
        bye(PMCTRL_ERROR);
    }

    /* if no argument, show usage */

    if (argc < 2){
        /* end exit */
        short_usage();
    }
                                 
    /* if PMCTRL_SMIT environment     */
    /* variable is not null pointer,  */

    in_smit_flag = FALSE;

    if ( (smitenv = getenv("PMCTRL_SMIT")) != NULL){
        /* set in_smit_flag to 1 to  */        
        /* indicate the cmd is executed   */
        /* from smit                      */

        in_smit_flag = TRUE;
    }

    /* parse command line into flag status. */

    flag_status = parse(argc,argv);

    Debug1("flag_status = %x\n",flag_status);


    /*--------------------------------*/
    /* The order of processing for    */
    /* each flag is very important in */
    /* this program. Please take care.*/
    /*--------------------------------*/

    process_h_flag(flag_status);
    process_v_flag(flag_status);
    process_a_flag(flag_status);
    process_d_flag(flag_status);
    process_e_flag(flag_status);
    process_lpcxu_flag(flag_status);

    rc = PMCTRL_SUCCESS;

    /* from now on, flags can be */
    /* specified successively    */

    process_tg_flag(flag_status);
    process_bwrkys_flag(flag_status);
    process_SR_flag(flag_status);

    /* if error occurred somewhere */
    
    if (rc!=PMCTRL_SUCCESS) {                                
        bye(PMCTRL_ERROR);
    }
    else{
        bye(PMCTRL_SUCCESS);
    }

} /* end of main() */

/*
 *  NAME:       get_arg()
 *
 *  FUNCTION:   get argument from option list.
 *
 *  NOTES:      optarg, optind are grobal variables.
 */
int
get_arg(int argc,char **argv,int argnum,char **p_arg)
{
    int idx;
    /* get first argument */

    for (idx=0;idx<argnum;idx++){
        p_arg[idx]=NULL;
    }
    
    if (*optarg == '-' && !isdigit(*(optarg+1)) ) {

        /* option does not have argument,      */
        /* and successive option is parsed as  */
        /* optarg. optind has to be decremented*/
        /* for next parsing stage              */
        /* except for -NNNN (minus number)     */

        optind--;

        return PMCTRL_SUCCESS;
    }                                
    else {              

         /* option argument is optarg */
          
        *p_arg = optarg;
    }

    if ( (optind + argnum -1 ) <= argc ){

        /* get extra arguments */

        for (idx=1;idx<argnum;idx++){
            p_arg[idx]=argv[optind++];
        }

        return  PMCTRL_SUCCESS;

    }
    else{
        /* argv overflowed */

        for (idx=1; idx<argnum || (optind+idx) <= argc ; idx++){
            p_arg[idx]=argv[optind++];
        }

        optind = argc ;

        Debug1(" cannot get arguments: requested[%d] \n",argnum);
        Debug1("                     : optind[%d] \n",optind);
        Debug1("                     : argc[%d] \n",argc);

        return PMCTRL_ERROR;
    }
}


/*
 *  NAME:       set_on_off()
 *
 *  FUNCTION:   set on/off state.
 *
 *  INPUTS:   
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR  -- failure
 */
int set_on_off(char *arg, int cmd)
{
    int   rc = PMCTRL_SUCCESS;
    int   on_off;
    char  attribute[17];


    switch (cmd) {
      case PMLIB_SET_BEEP:
        strcpy(attribute,"pm_beep");
        break;
      case PMLIB_SET_PASSWD_ON_RESUME:
        strcpy(attribute,"resume_passwd");
        break;
      case PMLIB_SET_RINGING_RESUME:
        strcpy(attribute,"ringing_resume");
        break;
      case PMLIB_SET_KILL_LFT_SESSION:
        strcpy(attribute,"lft_session");
        break;
      case PMLIB_SET_KILL_TTY_SESSION:
        strcpy(attribute,"tty_session");
        break;
      case PMLIB_SET_KILL_SYNCD:
        strcpy(attribute,"kill_syncd");
        break;
      default:
        rc = PMCTRL_ERROR;
        break;
    }

    /* check if arg is valid of not   */
    if (chkarg_on_off(arg,&on_off) == PMCTRL_ERROR) {
        rc = PMCTRL_ERROR;
    }
    else {
        /* ask pm core to change data     */
        Debug1("on_off is %d\n",on_off);
        

        if (pmlib_request_parameter(cmd, (caddr_t)&on_off)!= PMLIB_SUCCESS) {
            if( errno == ENOSYS ){
                fprintf(stderr,MSGSTR(ERR_NOSYS_PARAM,DFLT_ERR_NOSYS_PARAM));
                errno = 0;
            }
            else if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            rc = PMCTRL_ERROR;
        }
        else {
            /* change odm data base           */
            if (change_pmmi_data(attribute,
                                 pmlibonoff2boolean(on_off)) == -1){
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }
    }

    return(rc);
}


/*
 *  NAME:       set_duration()
 *
 *  FUNCTION:   set duration of suspend or system idle time.
 *
 *  INPUTS:    
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR  -- failure
 */
int
set_duration(char *arg, int cmd)
{
    int   rc = PMCTRL_SUCCESS;
    long   duration=0;
    char  attribute[17];

    switch (cmd) {
      case PMLIB_SET_SYSTEM_IDLE_TIME:
        strcpy(attribute,"sys_idle_time");
        break;
      case PMLIB_SET_DURATION_TO_HIBERNATION:
        strcpy(attribute,"sus_to_hiber");
        break;
      default:
        rc = PMCTRL_ERROR;
        break;
    }

    /* check if valid duration or not */
    if (chkarg_duration(arg,&duration) == PMCTRL_ERROR) {
        rc = PMCTRL_ERROR;
    }
    else {
        Debug1("timer duration is %d (sec)\n",duration);
      
        /* ask pm core to change data */
            
        if ( pmlib_request_parameter(cmd, (caddr_t)&duration)!= PMLIB_SUCCESS){
            if( errno == ENOSYS ){
                fprintf(stderr,MSGSTR(ERR_NOSYS_PARAM,DFLT_ERR_NOSYS_PARAM));
                errno = 0;
            }
            else if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            rc = PMCTRL_ERROR;
        }
        else {
            /* change data in odm */
            if (change_pmmi_data(attribute, duration) == -1) {
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }
    }
    return(rc);
}


/*
 *  NAME:       set_permission()
 *
 *  FUNCTION:   Set permission for general users
 *
 *  INPUTS:     permitted state -- ignore/standby/suspend/hibernation/shutdown
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR -- failure
 */
int 
set_permission(char *arg)
{
    int   rc = PMCTRL_SUCCESS;
    int   action;
    char  attribute[32];

    memset(attribute,0,sizeof(attribute));

    strcpy(attribute,"permission");

    Debug1("attribute = %s\n",attribute);
    
    /* check if valid action or not */
    if (chkarg_action(arg,&action) == PMCTRL_ERROR) {
        rc = PMCTRL_ERROR;
    }
    else {
        /* build up bitwise ORed state */
        action = bitwiseORed_state(action);

        /* ask pm core to change data */
        if (pmlib_request_parameter( PMLIB_SET_PERMISSION,
                                    (caddr_t)&action)!= PMLIB_SUCCESS) {
            if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            rc = PMCTRL_ERROR;
        }
        else {
            /* if system call return with success, change odm data */
            if (change_pmmi_data(attribute, pmlib2pmstate(action)) == -1 ){

                Debug0("Failed to change pmmi data \n");
                Debug1("attribute = %s\n",attribute);
                Debug1(" action is %x \n",action);
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }
    }

    return(rc);
}
/*
 *  NAME:       set_action()
 *
 *  FUNCTION:   Set action for various events.
 *
 *  INPUTS:     action  -- ignore/standby/suspend/hibernation/shutdown
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR -- failure
 */

int 
set_action(char *arg, int cmd)
{
    int   rc = PMCTRL_SUCCESS;
    int   action;
    char  attribute[32];

    memset(attribute,0,sizeof(attribute));

    switch (cmd) {
      case PMLIB_SET_LOW_BATTERY_ACTION:
        strcpy(attribute,"battery_action");
        break;
      case PMLIB_SET_MAIN_SWITCH_ACTION:
        strcpy(attribute,"switch_action");
        break;
      case PMLIB_SET_LID_CLOSE_ACTION:
        strcpy(attribute,"lid_action");
        break;
      case PMLIB_SET_SYSTEM_IDLE_ACTION:
        strcpy(attribute,"sys_idle_action");
        break;
      case PMLIB_SET_SPECIFIED_TIME_ACTION:
        strcpy(attribute,"specified_act");
        break;
      default:
        rc = PMCTRL_ERROR;
    }

    Debug1("attribute = %s\n",attribute);
    
    /* check if valid action or not   */
    if (chkarg_action(arg,&action) == PMCTRL_ERROR) {
        rc = PMCTRL_ERROR;
    }
    else {

        /* ask pm core to change data     */
        if (pmlib_request_parameter( cmd, (caddr_t)&action)!= PMLIB_SUCCESS) {
            if( errno == ENOSYS ){
                fprintf(stderr,MSGSTR(ERR_NOSYS_PARAM,DFLT_ERR_NOSYS_PARAM));
                errno = 0;
            }
            else if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            rc = PMCTRL_ERROR;
        }
        else {
            /* if system call returns with success, change odm data */
            if (change_pmmi_data(attribute, pmlib2pmstate(action)) == -1 ){

                Debug0("Failed to change pmmi data \n");
                Debug1("attribute = %s\n",attribute);
                Debug1(" action is %x \n",action);
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }
    }

    return(rc);
}

/*
 *  NAME:       daily_alarm_gmt()
 *
 *  FUNCTION:   Convert daily alarm in local time into that in GMT.
 *
 *  INPUTS:     
 *              hour : local time hour.
 *              minute : local time minute.
 *
 *  OUTPUTS:    SUCCESS:  daily alarm value in GMT.
 *              FAILURE:  -1 
 *              
 *  NOTES:      reverse function of daily_alarm_localtime()
 *              
 *              in case the alarm is set to 00:00 in GMT,
 *              the output value is 24*3600 to distinguish from the case
 *              of disable. ( 0 means disable for specified timer)
 *
 */
time_t
daily_alarm_gmt(int hour,int minute)
{
    struct tm *ts = NULL;
    time_t present = 0;
    time_t gmt = 0;

    Debug2("daily_alarm_gmt(hour:%d min:%d)\n",hour,minute);

    /* get present time */
    (void)time(&present);

    /* set time zone */ 
    tzset();
    ts = localtime(&present);

    /* set month,day,hour,min,sec       */
    /* month and day are left unchanged */

    ts->tm_hour  = hour;
    ts->tm_min   = minute;    
    ts->tm_sec   = 0;         
    ts->tm_isdst = -1; /* Daylight Saving Time be taken into account */

    /* set time zone */
    tzset();

    /* now gmt is specified time of the present date */
    if ((gmt = mktime(ts)) < 0){
        return -1;
    };

    /* now gmt is reduced to the specified time of 1970/1/1 */ 
    gmt += 3600*24;  
    gmt %= 3600*24;  

    if (gmt == 0 ){
        gmt = 3600*24;
    }

    Debug1("gmtime : %d\n",gmt);

    return gmt;
}

/*
 *  NAME:       daily_alarm_localtime()
 *
 *  FUNCTION:   Convert daily alarm in gmt into that in local time.
 *
 *  INPUTS:     gmt : daily alarm value in gmt.
 *
 *  OUTPUTS:    SUCCESS: *phour   : local time hour.
 *                       *pminute : local time minute.
 *              FAILURE: *phour   : -1
 *                       *pminute : -1
 *  
 *  NOTES:      reverse function of daily_alarm_gmt()
 *              
 *
 */
void
daily_alarm_localtime(time_t gmt, int *phour, int *pminute)
{
    struct tm *ts = NULL;
    time_t present = 0;

    Debug1("daily_alarm_localtime(gmt:%d )\n",gmt);

    /* NOTE:                                                        */
    /* if gmt == 0, it means timer is disabled.                     */
    /* if gmt == 24*3600, it means timer is set to 00:00 every day. */

    if (0 < gmt <= 24*3600){

        /* get present time */
        (void)time(&present);

        /* plus total seconds till the present day's midnight*/    
        gmt+=(present/(3600*24))*3600*24; 

        /* set time zone */     
        tzset();     

        /* convert into local time */
        ts = localtime(&gmt);

        if (ts != NULL){
            *phour = ts->tm_hour;
            *pminute = ts->tm_min;
        }
        else{
            *phour = *pminute = -1;
        }       
    }
    else{
        *phour = *pminute = -1;
    }   

    Debug2("hour: %d, min: %d\n",*phour,*pminute);

    return;
}

/*
 *  NAME:       set_pm_alarm()
 *
 *  FUNCTION:   Convert specified time string to integer and set alarm
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *              action -- action to be set for specified time
 *              targ   -- pointer to time string
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR  -- failure
 *
 *  NOTES:      There are two alarms in PM. One is for suspend/hibernation
 *              and the other is for resume.
 *
 */

int set_pm_alarm(int cmd,char *timearg,char *actarg)
{
    int       rc = PMCTRL_SUCCESS;
    int       len;
    time_t    timeval;
    struct tm ts;
    char      buf[10], item[50], attribute[17];
    int       year, month, day, hour, minute;

    Debug1("actarg is %s \n",actarg);
    
    timeval=0;
    memset(&ts,0,sizeof(struct tm));
    memset(buf,0,sizeof(buf));
    memset(item,0,sizeof(item));
    memset(attribute,0,sizeof(attribute));

    switch (cmd) {
      case PMLIB_SET_SPECIFIED_TIME:
        strcpy(item, "specified time");
        strcpy(attribute, "specified_time");
        rc = set_action(actarg, PMLIB_SET_SPECIFIED_TIME_ACTION);
        break;
      case PMLIB_SET_RESUME_TIME:
        strcpy(item, "resume time");
        strcpy(attribute, "resume_time");
        break;
      default:
        rc = PMCTRL_ERROR;
        break;
    }


    len = strlen(timearg);
  
    Debug1("time arg length %d\n",len);

    switch(len) {
      case 10:
        /* YYMMDDHHMM format */

        Debug0("YYMMDDHHMM format\n");

        /* parse check */
        if (is_number(timearg,10)){
            Debug0("string is numeral\n");
            sscanf(timearg,"%2d%2d%2d%2d%2d",
                   &year,
                   &month,
                   &day,
                   &hour,
                   &minute);

            Debug1("year: %d\n",year);
            Debug1("month: %d\n",month);
            Debug1("day: %d\n",day);
            Debug1("hour: %d\n",hour);
            Debug1("minute: %d\n",minute);

            /* set time structure */
            /* month field has 0-11 value for ( Jan to Dec) */

            /* set year */
            if ( 0<= year && year <= 38){
                ts.tm_year = year + 100;
                /* offset from 1900 for time_t format */
            }else if ( 70<= year || year <= 99 ) {
                ts.tm_year = year;
            }else{
                ts.tm_year = -1;
            }

            /* set month,day,hour,min,sec */
            ts.tm_mon   = month-1;   
            ts.tm_mday  = day;       
            ts.tm_hour  = hour;      
            ts.tm_min   = minute;    
            ts.tm_sec   = 0;         
            ts.tm_isdst = -1; /* Daylight Saving Time be taken into account */

            /* range check */
            if (check_time_value(&ts)!=PMCTRL_SUCCESS) {
                fprintf(stderr, 
                        MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME), 
                        timearg);
                rc = PMCTRL_ERROR;
            }
            else{
                /* calculate gm time value */

                tzset();
                timeval = mktime(&ts);
                Debug1("time val = %d \n",timeval);

                /* timeval will be -1 if invalid data is specified */

                if  (timeval <= 24 * 3600) {
                    fprintf(stderr, 
                            MSGSTR(ERR_BAD_TIME,DFLT_ERR_BAD_TIME),
                            timearg);
                    rc = PMCTRL_ERROR;
                }
            }
        }
        else{
            Debug0("parse error \n");        
            fprintf(stderr, 
                    MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME),
                    timearg);
            rc = PMCTRL_ERROR;
        }
        break;
        case 4:
            /* HHMM format (everyday) */

        Debug0("HHMM format\n");

        if (!strcmp(timearg, "0000")){
            timeval = daily_alarm_gmt(0,0);   /* this means everyday's 00:00 */
        }
        else {
            /* parse check */
            if (is_number(timearg,4)){

                Debug0("string is numeral\n");
                sscanf(timearg,"%2d%2d",&hour,&minute);

                /* range check */
                if (hour >= 0 && 
                    hour <=23 && 
                    minute >=0 && 
                    minute <=59){
                    timeval = daily_alarm_gmt(hour,minute);
                }
                else{
                    fprintf(stderr, 
                            MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME), 
                            timearg);
                    rc = PMCTRL_ERROR;
                }
            }
            else{
                Debug0("parse error \n");        
                fprintf(stderr, 
                        MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME), 
                        timearg);
                rc = PMCTRL_ERROR;
            }
        }
        break;
      case 1:                   
        /* disable alarm */
        if (*timearg=='0'){
            timeval = 0;
        }
        else {
            fprintf(stderr, 
                    MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME), 
                    timearg);
            rc = PMCTRL_ERROR;
        }
        break;
      default:
        fprintf(stderr, 
                MSGSTR(ERR_BAD_TIME, DFLT_ERR_BAD_TIME), 
                timearg);
        rc = PMCTRL_ERROR;
        break;
    }

    Debug1("time value is %d\n",timeval);
    /* ask pm core to change data */

    if (rc == PMCTRL_SUCCESS){
        
        if (pmlib_request_parameter( cmd, (caddr_t)&timeval) 
            != PMLIB_SUCCESS) {
            if( errno == ENOSYS ){
                fprintf(stderr,MSGSTR(ERR_NOSYS_PARAM,DFLT_ERR_NOSYS_PARAM));
                errno = 0;
            }
            else if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            rc = PMCTRL_ERROR;
        }
        else {
            /* if pm core says o.k, change    */
            /* odm data                       */

            Debug1("attribute is %s\n",attribute);
            Debug1("timeval is %d\n",timeval);

            if (change_pmmi_data(attribute, timeval) == -1) {
                Debug0("failed to change pmmi data\n");
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }       
    }

    return(rc);
}

/*
 *  NAME:       is_number
 *
 *  FUNCTION:   Check if input string is numerous.
 *
 *  INPUTS:     a pointer to character, number of figure
 *
 *  OUTPUTS:    TRUE  -- success
 *              FALSE    -- failure
 */
int
is_number(char *str,int fig)
{
    int i;

    for( i=0; i<fig; i++ ){
        if ('0' > *(str+i) || '9' < *(str+i)){
            return FALSE;
        }
    }
    return TRUE;
}

/*
 *  NAME:       check_time_value()
 *
 *  FUNCTION:   Check if input timer value is valid or not.
 *
 *  INPUTS:     a pointer to struct tm
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR    -- failure
 *
 *  NOTES:   
 */

int 
check_time_value(struct tm *pts)
{
    time_t timebuf;
#define  MIN_yy  70  /* Minimum value of year */
#define  MAX_yy 138  /* Maximum value of year */
#define  MIN_mm  00  /* Minimum value of month - zero based for mktime()  */
#define  MAX_mm  11  /* Maximum value of month - 0 is Jan & 11 is Dec     */    
#define  MIN_dd  01  /* Minimum value of day    */
    static char MAX_dd[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define  MIN_HH  00  /* Minimum value of hour   */
#define  MAX_HH  23  /* Maximum value of hour   */
#define  MIN_MM  00  /* Minimum value of minute */
#define  MAX_MM  59  /* Maximum value of minute */
#define  MIN_SS  00  /* Minimum value of second */
#define  MAX_SS  61  /* Maximum value of second - accounts for leap second */
#define  Leapyear(Year) \
    (((Year) % 4 == 0 && (Year) % 100 != 0 || (Year) % 400 == 0) ? 1 : 0)

    /* Check range of each value */
    
    if ( pts->tm_year < MIN_yy || pts->tm_year > MAX_yy ) {
        return(PMCTRL_ERROR);   
    }   
    if ( pts->tm_mon  < MIN_mm || pts->tm_mon  > MAX_mm ){
        return(PMCTRL_ERROR);
    }   
    if ( pts->tm_mday < MIN_dd || pts->tm_mday > MAX_dd[pts->tm_mon] ){
        return(PMCTRL_ERROR);
    }
    if ( pts->tm_hour < MIN_HH || pts->tm_hour > MAX_HH ){
        return(PMCTRL_ERROR);
    }
    if ( pts->tm_min  < MIN_MM || pts->tm_min  > MAX_MM ){
        return(PMCTRL_ERROR);
    }
    if ( pts->tm_sec  < MIN_SS || pts->tm_sec  > MAX_SS ){
        return(PMCTRL_ERROR);
    }
    /* If month is February, days > 28 and not leap year, then return. */
    if (pts->tm_mon == 1 && 
        pts->tm_mday > 28 && 
        !Leapyear( 1900 + pts->tm_year) ){
        return( PMCTRL_ERROR );
    }

    tzset();
    if ( (timebuf = mktime(pts) ) == (time_t) -1){
        return( PMCTRL_ERROR );
    }           

    return PMCTRL_SUCCESS;
}



/*
 *  NAME:       query_pm_alarm()
 *
 *  FUNCTION:   Display current setting of suspend/hibernation/resume alarms
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *
 *  OUTPUTS:    PMCTRL_SUCCESS  -- success
 *              PMCTRL_ERROR  -- failure
 *
 *  NOTES:      There are two alarms in PM. One is for suspend/hibernation
 *              and the other is for resume.
 *
 */

int query_pm_alarm(int cmd)
{
    int       rc = PMCTRL_SUCCESS;
    int       action;
    char      item[50];
    time_t    alarmval;
    struct tm *ts; 
    char      act_str[32];
    char      time_str[32];
    int       daily_alarm_hour = 0;
    int       daily_alarm_min = 0;


    memset(act_str,0,32);
    memset(time_str,0,32);

    action = PMLIB_NONE;

    switch (cmd) {
      case PMLIB_QUERY_SPECIFIED_TIME:
        strcpy(item, "speicfied time");
        
        if (pmlib_request_parameter(PMLIB_QUERY_SPECIFIED_TIME_ACTION,
                                    (caddr_t)&action)!= PMLIB_SUCCESS) {
            Debug0("pmlib_request_parameter failed\n");
            rc = PMCTRL_ERROR;
        }
   
        switch(action) {
        case PMLIB_SYSTEM_SUSPEND:
            strcpy(act_str,"suspend");
            break;
        case PMLIB_SYSTEM_HIBERNATION:
            strcpy(act_str,"hibernation");
            break;
        case PMLIB_SYSTEM_SHUTDOWN:
            strcpy(act_str,"shutdown");
            break;
        default:
            rc = PMCTRL_ERROR;
        }
        break;
      case PMLIB_QUERY_RESUME_TIME:
        strcpy(item, "resume time");
        strcpy(act_str,"resume");
      
        break;
      default:
        rc = PMCTRL_ERROR;
        break;
    }
    
    if (pmlib_request_parameter(cmd, (caddr_t)&alarmval)!= PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
    }
    else {
        if (alarmval == 0) {
            if (in_smit_flag){
                strcpy(time_str,"0");
            }
            else{
                strcpy(time_str,"disabled");
            }
        }
        else if ( (0 < alarmval) && (alarmval <= 24*3600) ) {
            /* HHMM format (everyday)       */
            daily_alarm_localtime(alarmval,&daily_alarm_hour,&daily_alarm_min);
            if ( daily_alarm_hour <0 || daily_alarm_hour >23 ||
                 daily_alarm_min <0 || daily_alarm_min >59 ){
                rc = PMCTRL_ERROR;
            }
            else{
                if (in_smit_flag){
                    sprintf(time_str,
                            "%02d%02d",
                            daily_alarm_hour, 
                            daily_alarm_min);
                }
                else{
                    sprintf(time_str,
                            "%2d:%02d",
                            daily_alarm_hour, 
                            daily_alarm_min);
                }
            }
        }
        else{   
            /* YYMMDDHHMM format */
            tzset();
            ts = localtime(&alarmval);
            if (ts == NULL) {
                rc = PMCTRL_ERROR;
            }
            else {
                if (in_smit_flag){
                    sprintf(time_str,
                            "%02d%02d%02d%02d%02d",
                            ts->tm_year%100,
                            ts->tm_mon+1,
                            ts->tm_mday,
                            ts->tm_hour,
                            ts->tm_min);
                }
                else{
                    sprintf(time_str,"%02d/%2d/%2d %2d:%02d", ts->tm_year%100,
                           ts->tm_mon+1, ts->tm_mday,
                           ts->tm_hour, ts->tm_min);
                }
            }
        }
    }

    if (rc == PMCTRL_SUCCESS){
        if(in_smit_flag){
            fprintf(stdout,"%s %s\n",act_str,time_str);
        }else{
         fprintf(stdout,MSGSTR(ACTION_TIME,DFLT_ACTION_TIME),act_str,time_str);
        }
    }

    return(rc);
}


/*
 *  NAME:       start_action()
 *
 *  FUNCTION:   request pm state transition
 *
 *  INPUTS:     action -- action to be started
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *
 */
int 
start_action(int action)
{
    int rc = PMCTRL_SUCCESS;
    pmlib_state_t pms;

    /* initialize structure */
    memset(&pms,0,sizeof(pms));
    pms.state = action;

    /* ask core for state transition */
    
    if (pmlib_request_state(PMLIB_REQUEST_STATE, &pms)== PMLIB_SUCCESS) {

        /* change ODM , in case of the request for transition to full-on,
           and enable */

        if (pms.state == PMLIB_SYSTEM_ENABLE || 
            pms.state == PMLIB_SYSTEM_FULL_ON){
            if (change_pmmi_data("sys_state",
                                 pmlib2pmstate(pms.state)) == -1) {
                fprintf(stdout,MSGSTR(WARN_ODM_CHANGE,DFLT_WARN_ODM_CHANGE));
                rc = PMCTRL_SUCCESS;
            }
        }
    }
    else{
        rc = PMCTRL_ERROR;

        Debug1("errno = %d \n",errno);

        /* in case errno is set */
        if( errno == EBUSY ){
            fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
            errno = 0;
        }
        else if( errno == EPERM ){
            fprintf(stderr,MSGSTR(ERR_INSUF_PERM,DFLT_ERR_INSUF_PERM));
            errno = 0;
        }
        else if( errno == EINVAL ){
            fprintf(stderr,MSGSTR(ERR_INVAL,DFLT_ERR_INVAL));
            errno = 0;
        }
        else if( errno == ENOSYS ){
            fprintf(stderr,MSGSTR(ERR_NOSYS_STATE,DFLT_ERR_NOSYS_STATE));
            errno = 0;
        }
        else if( errno == EACCES ){
            fprintf(stderr,MSGSTR(ERR_ACCES,DFLT_ERR_ACCES));
            errno = 0;
        }
        else if( errno == EALREADY ){
            fprintf(stderr,MSGSTR(ERR_ALREADY,DFLT_ERR_ALREADY));
            errno = 0;
        }
    }
    return(rc);
}


/*
 *  NAME:       query_duration()
 *
 *  FUNCTION:   Query current duration setting for the following items.
 *                -- system idle time
 *                -- duration from suspend to hibernation
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *
 *  NOTES:      The duration is displayed in minutes.
 *
 */

int query_duration(int cmd)
{
    int  rc;
    int  result;
    char item[64];
    char result_str[32];

    memset(item,0,sizeof(item));
    memset(result_str,0,sizeof(result_str));

    switch(cmd) {
        /* set msg according to cmd     */
      case PMLIB_QUERY_SYSTEM_IDLE_TIME:
        strcpy(item,"System idle timer");
        break;
      case PMLIB_QUERY_DURATION_TO_HIBERNATION:
        strcpy(item,"Duration of suspend to hibernation");
        break;
      default:
        /* error ...                    */
        rc = PMCTRL_ERROR;
        break;
    }

    if (pmlib_request_parameter(cmd,(caddr_t)&result)
        != PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
        /* indicate error */
    }
    else{
        if (in_smit_flag){
            /* if cmd is executed from smit */
            sprintf(result_str,"%d", result/60);
            /* just output duration(min.).  */
        }
        else{
            if (result!=0){
                sprintf(result_str,"%d. (in minutes)",result/60);
            }
            else{
                sprintf(result_str,"disabled");
            }
        }

        /* display duration             */
        if(!in_smit_flag){
            fprintf(stdout,MSGSTR(DUR_T, DFLT_DUR_T), item, result_str);
        }
        else{
            fprintf(stdout,"%s\n",result_str);
        }
    }

    return(rc);
}


/*
 *  NAME:       chkarg_action()
 *
 *  FUNCTION:   Convert command line string(action) to integer value
 *                ignore      -> PMLIB_NONE
 *                standby     -> PMLIB_SYSTEM_STANDBY
 *                suspend     -> PMLIB_SYSTEM_SUSPEND
 *                hibernation -> PMLIB_SYSTEM_HIBERNATION
 *                shutdown    -> PMLIB_SYSTEM_SHUTDOWN
 *
 *  INPUTS:     arg -- pointer to a command line string
 *              val -- pointer to an area for result value
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- error
 *
 */

int chkarg_action(char *arg, int *val)
{
    int rc = PMCTRL_SUCCESS;

    if (!strcmp(arg,"ignore")){
        /* "ignore" --> PMLIB_NONE      */
        *val = PMLIB_NONE;
    }
    else if (!strcmp(arg,"full_on")){
        *val = PMLIB_SYSTEM_FULL_ON;
    }
    else if (!strcmp(arg,"enable")){
        *val = PMLIB_SYSTEM_ENABLE;
    }
    else if (!strcmp(arg,"standby")){
        *val = PMLIB_SYSTEM_STANDBY;
    }
    else if (!strcmp(arg,"suspend")){
        *val = PMLIB_SYSTEM_SUSPEND;
    }
    else if (!strcmp(arg,"hibernation")){
        *val = PMLIB_SYSTEM_HIBERNATION;
    }
    else if (!strcmp(arg,"shutdown")){
        *val = PMLIB_SYSTEM_SHUTDOWN;
    }
    else {
        rc = PMCTRL_ERROR;
        /* error... invalid argument    */
    }
    return(rc);
}

/*
 *  NAME:       query_permission_setting()
 *
 *  FUNCTION:   Query current permission setting
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 */
int 
query_permission_setting()
{
    int  rc = PMCTRL_SUCCESS;
    int  result;
    char item[128];

    if ( pmlib_request_parameter(PMLIB_QUERY_PERMISSION,
                                 (caddr_t)&result)!= PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
    }
    else {
        result = most_significant_state(result);

        switch(result) {

          case PMLIB_NONE:
            if (in_smit_flag)
                printf("ignore\n");
            else
                fprintf(stdout,MSGSTR(PERMISSION, DFLT_PERMISSION), "ignored");
            break;

          case PMLIB_SYSTEM_STANDBY:
            if (in_smit_flag)
                printf("standby\n");
            else
                fprintf(stdout,MSGSTR(PERMISSION, DFLT_PERMISSION), "standby");
            break;

          case PMLIB_SYSTEM_SUSPEND:
            if (in_smit_flag)
                printf("suspend\n");
            else
                fprintf(stdout,MSGSTR(PERMISSION, DFLT_PERMISSION), "suspend");
            break;

          case PMLIB_SYSTEM_HIBERNATION:
            if (in_smit_flag)
                printf("hibernation\n");
            else
                fprintf(stdout,MSGSTR(PERMISSION, DFLT_PERMISSION), "hibernation");
            break;

          case PMLIB_SYSTEM_SHUTDOWN:
            if (in_smit_flag)
                printf("shutdown\n");
            else
                fprintf(stdout,MSGSTR(PERMISSION, DFLT_PERMISSION), "shutdown");
            break;

          default:
            rc = PMCTRL_ERROR;
            /* indicate error occurred      */
        }
    }

    return(rc);
}


/*
 *  NAME:       query_action_setting()
 *
 *  FUNCTION:   Query current action setting for the following items.
 *                -- system idle timer expiration
 *                -- notebook lid closed
 *                -- main power switch toggled
 *                -- low battery
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 */

int query_action_setting(int cmd)
{
    int  rc = PMCTRL_SUCCESS;
    int  result;
    char item[128];

    switch (cmd) {
      case PMLIB_QUERY_SYSTEM_IDLE_ACTION:
        strcpy(item,"system idle timer expiration");
        break;
      case PMLIB_QUERY_LID_CLOSE_ACTION:
        strcpy(item,"notebook-lid close");
        break;
      case PMLIB_QUERY_MAIN_SWITCH_ACTION:
        strcpy(item,"main power switch pressed");
        break;
      case PMLIB_QUERY_LOW_BATTERY_ACTION:
        strcpy(item,"low battery condition");
        break;
      default:
        rc = PMCTRL_ERROR;
    }

    if ( pmlib_request_parameter(cmd,(caddr_t)&result)!= PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
    }
    else {
        switch(result) {

          case PMLIB_NONE:
            if (in_smit_flag)
                printf("ignore\n");
            else
                fprintf(stdout,MSGSTR(MSG9, DFLT_MSG9), item ,"ignored");
            break;

          case PMLIB_SYSTEM_STANDBY:
            if (in_smit_flag)
                printf("standby\n");
            else
                fprintf(stdout,MSGSTR(ACTION, DFLT_ACTION), item ,"standby");
            break;

          case PMLIB_SYSTEM_SUSPEND:
            if (in_smit_flag)
                printf("suspend\n");
            else
                fprintf(stdout,MSGSTR(ACTION, DFLT_ACTION), item ,"suspend");
            break;

          case PMLIB_SYSTEM_HIBERNATION:
            if (in_smit_flag)
                printf("hibernation\n");
            else
                fprintf(stdout,MSGSTR(ACTION, DFLT_ACTION), item ,"hibernation");
            break;

          case PMLIB_SYSTEM_SHUTDOWN:
            if (in_smit_flag)
                printf("shutdown\n");
            else
                fprintf(stdout,MSGSTR(ACTION, DFLT_ACTION), item ,"shutdown");
            break;

          default:
            rc = PMCTRL_ERROR;
            /* indicate error occurred      */
        }
    }

    return(rc);
}


/*
 *  NAME:       chkarg_duration()
 *
 *  FUNCTION:   Convert command line string(duration in minutes) to integer
 *              value (in seconds).
 *
 *  INPUTS:     arg    -- pointer to a command line string
 *              result -- pointer to an area for result value
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- error
 *
 *  NOTES:      Valid range (1 min. - 120 min.) is also checked.
 *
 */
int 
chkarg_duration(char *arg, long *result)
{
    int  rc = PMCTRL_SUCCESS;
    long duration;

    *result = 0;
    
    if (arg == NULL){
        Debug0("chkarg_duration : arg is NULL.\n");
        return PMCTRL_ERROR;
    }

    if (!is_number(arg,strlen(arg))){
        Debug0("chkarg_duration : arg is NULL.\n");
        return PMCTRL_ERROR;
    }

    duration = strtol(arg, NULL, 10); /* base 10 */
 
    /* strtol returns 0 in case arg is not parsed into long variables */
    if (duration == 0 && *arg!='0' ||
        errno == ERANGE){
        return PMCTRL_ERROR;
    }

    /* convert str to long (sec.)   */
    duration *= 60 ;

    /* duration must be within the  */
    /* following range              */

    if ( !(duration == 0 || 60<=duration && duration<=7200) ){
        rc = PMCTRL_ERROR;
    }
    else {
        /* set result in seconds */
        *result = duration;                
    }
    return(rc);
}


/*
 *  NAME:       query_on_off_setting()
 *
 *  FUNCTION:   Query current on/off setting of the following items.
 *                beep, resume password, ringing resume, lft/tty termination
 *
 *  INPUTS:     cmd -- command which is passed to pmlib_request_parameter().
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 */
int query_on_off_setting(int cmd)
{
    int  rc = PMCTRL_SUCCESS;
    int  result;
    char item[128];

    switch (cmd) {
      case PMLIB_QUERY_BEEP:
        strcpy(item,"Beep");
        break;
      case PMLIB_QUERY_PASSWD_ON_RESUME:
        strcpy(item,"Resume password");
        break;
      case PMLIB_QUERY_RINGING_RESUME:
        strcpy(item,"Ringing resume");
        break;
      case PMLIB_QUERY_KILL_LFT_SESSION:
        strcpy(item,"LFT termination");
        break;
      case PMLIB_QUERY_KILL_TTY_SESSION:
        strcpy(item,"TTY termination");
        break;
      case PMLIB_QUERY_KILL_SYNCD:
        strcpy(item,"Syncd termination");
        break;
      default:
        rc = PMCTRL_ERROR;
        break;
    }


    rc = pmlib_request_parameter(cmd,(caddr_t)&result);
    if (rc != PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
    }
    else {
        switch(result) {
            
        case PMLIB_ON:
            if (in_smit_flag){
                /* if cmd is executed from smit */
                fprintf(stdout,"on\n");
            }
            else{
                /* display 'on' status          */
                fprintf(stdout,MSGSTR(ON_OFF, DFLT_ON_OFF), item, "on");
            }
            break;
            
        case PMLIB_OFF:
            if (in_smit_flag){
                /* if cmd is executed from smit */
                fprintf(stdout,"off\n");
            }
            else{
                /* display 'off' status         */
                fprintf(stdout,MSGSTR(ON_OFF, DFLT_ON_OFF), item, "off");
            }
            break;
            
        default:
            /* error...                     */
            rc = PMCTRL_ERROR;
            break;
        }
    }
    
    return(rc);
}


/*
 *  NAME:       chkarg_on_off()
 *
 *  FUNCTION:   Convert command line string(on/off) to integer value
 *                on -> PMLIB_ON, off-> PMLIB_OFF
 *
 *  INPUTS:     arg    -- pointer to a command line string
 *              result -- pointer to an area for result value
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- error
 */
int chkarg_on_off(char *arg, int *result)
{
    int rc = PMCTRL_SUCCESS;

    if (strcmp(arg,"on")==0) { 
        *result = PMLIB_ON;    
    }
    else if (strcmp(arg,"off")==0){
        *result = PMLIB_OFF;       
    }
    else {
        rc = PMCTRL_ERROR;         
        /* error... invalid argument    */
    }
    return(rc);
}


/*
 *  NAME:       set_device_idle_time()
 *
 *  FUNCTION:   Set idle/standby time for each device
 *
 *  INPUTS:     lname -- device logical name
 *              targ1 -- idle time string      (DPMS dim time string)
 *              targ2 -- suspend time string   (DPMS suspend time string)
 *              targ3 -- turnoff time string   (DPMS off time string)
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *
 *  NOTES:      If lft0 is specified as a device, this routine also changes
 *              DPMS values.
 *
 */
int
set_device_idle_time(char *lname, char *targ1, char *targ2, char *targ3)
{
    int   rc = PMCTRL_SUCCESS;
    pmlib_device_info_t  dinfo, dinfo_cand;
    char  cmdbuf[256];
    static char dpms_lname[16];    

    Debug0("set_device_idle_time\n");
    Debug3("targ1=%s, targ2=%s , targ3=%s\n",targ1,targ2,targ3);

    
    memset(&dinfo,0,sizeof(pmlib_device_info_t));
    memset(&dinfo_cand,0,sizeof(pmlib_device_info_t));
    memset(&cmdbuf,0,sizeof(cmdbuf));
    memset(dpms_lname,0,16);


    if (strcmp(lname, "lft0")==0) {      

        /* if lname is lft0, it means DPMS      */
        /* in case of DPMS */
        
        Debug0("handling DPMS (set various time)\n");

        if (find_graphical_output(dpms_lname) !=0){

            /* there no graphical_output_device */
            return PMCTRL_ERROR;
        }
        
        /* dpms_lname fixed */
        /* set logical name to struct   */
        strncpy(dinfo.name, dpms_lname,16);

	if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
				     (caddr_t)&dinfo) != PMLIB_SUCCESS) {
            if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
	    return PMCTRL_ERROR;	
	}
	
        /* Now check digits for targ                      */
        /* targ can be integer larger than or equal to -1 */

        /* dim time */
        if ( strcmp(targ1,"-1") != 0){
            if ( !is_number(targ1,strlen(targ1)) ){
                return PMCTRL_ERROR;
            }
            else{
                dinfo.idle_time1 = strtol(targ1, NULL, 10);
            }
        }

        /* suspend time                */
        if (strcmp(targ2, "-1")!=0){
            if ( !is_number(targ2,strlen(targ2)) ){
                return PMCTRL_ERROR;
            }
            else{
                dinfo.idle_time2 = strtol(targ2, NULL, 10);
            }
        }

        /* turn off time                    */
        if (strcmp(targ3, "-1")!=0){
            if ( !is_number(targ3,strlen(targ3)) ){
                return PMCTRL_ERROR;
            }
            else{
                dinfo.idle_time = strtol(targ3, NULL, 10);
            }
        }

        /* check disable option */
        if ( dinfo.idle_time == 0 ||
	     dinfo.idle_time1 == 0 ||
	     dinfo.idle_time2 == 0 ){
	    dinfo.idle_time = dinfo.idle_time1 = dinfo.idle_time2 = 0;
	}

        if ( set_timers_to_all_the_graphical_output(dinfo.idle_time,
                                                    dinfo.idle_time1,
                                                    dinfo.idle_time2)
            != PMCTRL_SUCCESS){
            return PMCTRL_ERROR;
        }

        /* change odm data for DPMS */
        
        sprintf(cmdbuf,
                "/usr/sbin/chdev -l lft0 \
-P -a pwr_mgr_t1=%d -a pwr_mgr_t2=%d -a pwr_mgr_t3=%d",
                dinfo.idle_time1,dinfo.idle_time2,dinfo.idle_time);

        if (system(cmdbuf)!=0){
            fprintf(stderr,MSGSTR(ERR_ODM_CHANGE,DFLT_ERR_ODM_CHANGE));
            return  PMCTRL_ERROR;
        }
    }
    else { 
        /* in case of other device than lft0 */

        Debug0("handling other device than DPMS (set various time )\n");
        strncpy(dinfo.name, lname,16);

        /* ask core of device info */

        Debug1("deviceinfo.name = %s \n",dinfo.name);
        if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
                                     (caddr_t)&dinfo) != PMLIB_SUCCESS) {
            if( errno == EBUSY ){
                fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
                errno = 0;
            }
            return PMCTRL_ERROR;
        }
        else if (dinfo.attribute & PMLIB_GRAPHICAL_OUTPUT){
            /* if the specified device is Graphical Output,
               return with error*/
            return PMCTRL_ERROR;
        }
        else if ( strcmp(targ1, "-1") != 0 && !is_number(targ1,strlen(targ1))
                ||strcmp(targ2, "-1") != 0 && !is_number(targ2,strlen(targ2))){
            return PMCTRL_ERROR;
        }
        else{

            /* set dinfo_cand */
            memcpy(&dinfo_cand,&dinfo,sizeof(pmlib_device_info_t));

            /* idle time                                  */
            /* (convert the time in min into that in sec) */

            if ( strcmp(targ1,"-1") != 0 ){
                    dinfo_cand.idle_time = strtol(targ1, NULL, 10) *60;
            }

            /* change the idle time of the specified
               device, if it has the entry of idle time in ODM data */

            if ( get_dev_odm(lname, "pm_dev_itime", NULL)
                != 0 ){
                fprintf(stderr,
                        MSGSTR(ERR_ITIME,
                               DFLT_ERR_ITIME));
                rc = PMCTRL_ERROR;
            }
            else{
                /* request to pmd and pm-core */

                if ( check_dev_odm(lname,
                                   "pm_dev_itime",
                                   dinfo_cand.idle_time) != 0 ){
                    fprintf(stderr,
                            MSGSTR(ERR_ITIME,
                                   DFLT_ERR_ITIME));
                    rc = PMCTRL_ERROR;
                }
                else{
                    if (pmlib_request_parameter(PMLIB_SET_DEVICE_INFO,
                              (caddr_t)&dinfo_cand) != PMLIB_SUCCESS){
			if( errno == EBUSY ){
			    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
			    errno = 0;
			}
                        rc = PMCTRL_ERROR;      
                    }
                    else{
                        if(change_dev_odm(lname, 
                                          "pm_dev_itime",
                                          dinfo_cand.idle_time) != 0){
                            fprintf(stderr,
                                    MSGSTR(WARN_ODM_ITIME,
                                           DFLT_WARN_ODM_ITIME));
                        }
                        rc = PMCTRL_SUCCESS;
                    }
                }
            }

            /* set dinfo_cand */
            if (rc != PMCTRL_SUCCESS){
                /* if changing itime failed */
                memcpy(&dinfo_cand,&dinfo,sizeof(pmlib_device_info_t));
            }

            /* standby time 
               (convert the time in min into that in sec)*/
            
            if ( strcmp(targ2, "-1") !=0 ){
                dinfo_cand.standby_time = strtol(targ2, NULL, 10) *60;
            }

            /* change the standby time of the specified
               device, if it has the entry of idle time in ODM data */

            if ( get_dev_odm(lname, "pm_dev_stime", NULL)
                != 0 ){
                fprintf(stderr,
                        MSGSTR(ERR_STIME,
                               DFLT_ERR_STIME));
                rc = PMCTRL_ERROR;
            }
            else{
                /* request to pmd and pm-core */

                if ( check_dev_odm(lname,
                                   "pm_dev_stime",
                                   dinfo_cand.standby_time) != 0 ){
                    fprintf(stderr,
                            MSGSTR(ERR_STIME,
                                   DFLT_ERR_STIME));
                    rc = PMCTRL_ERROR;
                }
                else{
                    if (pmlib_request_parameter(PMLIB_SET_DEVICE_INFO,
                              (caddr_t)&dinfo_cand) != PMLIB_SUCCESS){
			if( errno == EBUSY ){
			    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
			    errno = 0;
			}
                        rc = PMCTRL_ERROR;      
                    }
                    else{
                        if(change_dev_odm(lname, 
                                          "pm_dev_stime",
                                          dinfo_cand.standby_time) != 0){
                            fprintf(stderr,
                                    MSGSTR(WARN_ODM_STIME,
                                           DFLT_WARN_ODM_STIME));
                        }
                        rc = PMCTRL_SUCCESS;
                    }
                }
            }
        }
    }
    return(rc);
}

/*
 *  NAME:       find_graphical_output()
 *
 *  FUNCTION:   pick up the first one of device logical names for DPMS
 *
 *  INPUTS:     none.
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success (dpms_lname -- device logical name)
 *              PMCTRL_ERROR -- failure
 *              
 */
int
find_graphical_output(char *dpms_lname)
{
    int    rc = PMCTRL_SUCCESS;
    char  *ptr;
    int    dnumber;
    pmlib_device_names_t pmdev;
    pmlib_device_info_t  dinfo;
    int found=FALSE;
    
    Debug0("find_graphical_output\n");


    memset(&dinfo,0,sizeof(dinfo));
    memset(&pmdev,0,sizeof(pmdev));

    if (pmlib_request_parameter(PMLIB_QUERY_DEVICE_NUMBER,
                                (caddr_t)&dnumber)!= PMLIB_SUCCESS) {
	if( errno == EBUSY ){
	    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
	    errno = 0;
	}
        rc = PMCTRL_ERROR;
    }
    else {

        if(dnumber == 0){
            fprintf(stdout,MSGSTR(NO_PM_DD,DFLT_NO_PM_DD));
        }
        else{
            /* set number of devices        */
            pmdev.number = dnumber;

            /* allocate area for dev name   */
            Debug1("device number = %d\n",pmdev.number);

            pmdev.names = (char *)malloc( dnumber*16);
         
            if ( pmdev.names == NULL) {
                fprintf(stderr, MSGSTR(ERRMSG3, DFLT_ERRMSG3));
                rc = PMCTRL_ERROR;
            }
            else {

                /* clear the entry of the device name list */
                memset(pmdev.names,0,dnumber*16);

                if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_NAMES,
                                             (caddr_t)&pmdev)!= PMLIB_SUCCESS){
		    if( errno == EBUSY ){
			fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
			errno = 0;
		    }
                    rc = PMCTRL_ERROR;
                }
                else {
                    /* pm aware dev names */
                    for (ptr = pmdev.names;
                         ptr < pmdev.names+16*(pmdev.number); ptr+=16) {

                        Debug1("now query attributes for %s\n",ptr);
                        
                        strncpy(dinfo.name,ptr,16);
                        if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
                                 (caddr_t)&dinfo) == PMLIB_SUCCESS) {
                            if (dinfo.attribute&PMLIB_GRAPHICAL_OUTPUT){
                                Debug1("DPMS logical name found[%s]\n",ptr);
                                strncpy(dpms_lname,ptr,16);
                                found=TRUE;
                                break; /* exit for loop */
                            }
                        }
			else{
			    if( errno == EBUSY ){
				fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
				errno = 0;
			    }
			}
                    } /* end of for */
                    Debug0("endof search\n");
                }
            }
        }
    }

    if (found == FALSE){
        rc = PMCTRL_ERROR;
    }

    if (pmdev.names!=NULL){
        Free(pmdev.names);     /* free allocated area          */
    }

    return rc;

}
/*
 *  NAME:       set_timers_to_all_the_graphical_output()
 *
 *  FUNCTION:   pick up device logical name for DPMS
 *
 *  INPUTS:     timers' value ( in min )
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success (dpms_lname -- device logical name)
 *              PMCTRL_ERROR -- failure
 *              
 */
int
set_timers_to_all_the_graphical_output(int idle_time,
                                       int idle_time1,
                                       int idle_time2)
{
    int    rc = PMCTRL_SUCCESS;
    char  *ptr;
    int    dnumber;
    pmlib_device_names_t pmdev;
    pmlib_device_info_t  dinfo;
    int found=FALSE;
    
    Debug0("set timer to all the graphical output\n");

    memset(&dinfo,0,sizeof(dinfo));
    memset(&pmdev,0,sizeof(pmdev));

    if (pmlib_request_parameter(PMLIB_QUERY_DEVICE_NUMBER,
                                (caddr_t)&dnumber)!= PMLIB_SUCCESS) {
	if( errno == EBUSY ){
	    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
	    errno = 0;
	}
        rc = PMCTRL_ERROR;
    }
    else {

        if(dnumber == 0){
            fprintf(stdout,MSGSTR(NO_PM_DD,DFLT_NO_PM_DD));
        }
        else{
            /* set number of devices        */
            pmdev.number = dnumber;

            /* allocate area for dev name   */
            Debug1("device number = %d\n",pmdev.number);

            pmdev.names = (char *)malloc( dnumber*16);

            if ( pmdev.names == NULL) {
                fprintf(stderr, MSGSTR(ERRMSG3, DFLT_ERRMSG3));
                rc = PMCTRL_ERROR;
            }
            else {

                /* clear the entry of the device name list */
                memset(pmdev.names,0,dnumber*16);

                if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_NAMES,
                                           (caddr_t)&pmdev)!= PMLIB_SUCCESS){
		    if( errno == EBUSY ){
			fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
			errno = 0;
		    }
                    rc = PMCTRL_ERROR;
                }
                else {
                    /* pm aware dev names */
                    for (ptr = pmdev.names;
                         ptr < pmdev.names+16*(pmdev.number); ptr+=16) {

                        Debug1("now query attributes for %s\n",ptr);
                        
                        memset(&dinfo,0,sizeof(dinfo));
                        strncpy(dinfo.name,ptr,16);

                        if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
                                 (caddr_t)&dinfo) == PMLIB_SUCCESS) {
                            if (dinfo.attribute&PMLIB_GRAPHICAL_OUTPUT){
                                Debug1("DPMS logical name found[%s]\n",
                                       dinfo.name);
                                found = TRUE;

                                /* dim time */
                                if (idle_time1!=-1){
                                    dinfo.idle_time1=idle_time1*60;
                                }

                                /* suspend time */
                                if (idle_time2!=-1){
                                    dinfo.idle_time2=idle_time2*60;
                                }

                                /* turn off time */ 
                                if (idle_time!=-1){
                                    dinfo.idle_time=idle_time*60;
                                }

                                Debug1(" now set the timers to %s\n",
                                       dinfo.name);

                                if (pmlib_request_parameter(
                                      PMLIB_SET_DEVICE_INFO,
                                     (caddr_t)&dinfo)!= PMLIB_SUCCESS) {
				    if( errno == EBUSY ){
					fprintf(stderr,
						MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
					errno = 0;
				    }
                                    rc = PMCTRL_ERROR;
                                }
                            }
                        }
			else{
			    if( errno == EBUSY ){
				fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
				errno = 0;
			    }
			}
                    } /* end of for */
                    Debug0("endof search\n");
                }
            }
        }
    }

    if (found == FALSE){
        rc = PMCTRL_ERROR;
    }

    if (pmdev.names!=NULL){
        Free(pmdev.names);     /* free allocated area          */
    }

    return rc;

}

/*
 *  NAME:       query_device_idle_time()
 *
 *  FUNCTION:   Display device idle time
 *
 *  INPUTS:     lname -- device logical name
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *
 *  NOTES:      If "lft0" is specified as a device, DPMS values are displayed.
 *
 */
int
query_device_idle_time(char *lname)
{
    int rc = PMCTRL_SUCCESS ;
    pmlib_device_info_t  dinfo;
    static char dpms_lname[16];    

    Debug0("query device idle time \n");

    memset(dpms_lname,0,sizeof(dpms_lname));

    /* if lname is lft0, it means DPMS      */

    if (strcmp(lname, "lft0")==0) {      
        Debug0("handling DPMS (query devinfo)\n");

        /* first, necessary to find Graphical Output Device */
        
        if (find_graphical_output(dpms_lname) !=0){

            /* there no graphical_output_device */
            return PMCTRL_ERROR;
        }
        
        /* dpms_lname fixed */
        /* set logical name to struct   */
        strncpy(dinfo.name, dpms_lname,16);
        
    }
    else{
        Debug0("handling other device than DPMS (query devinfo)\n");

        /* set logcal name to struct    */
        strncpy(dinfo.name, lname,16);
    }  

    /* ask core of device info */

    Debug1("deviceinfo.name = %s \n",dinfo.name);

    if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
                                 (caddr_t)&dinfo) != PMLIB_SUCCESS) {
	if( errno == EBUSY ){
	    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
	    errno = 0;
	}
        rc = PMCTRL_ERROR;
    }
    else {
        if ( strcmp(lname, "lft0")==0 ) {
            /* if lft0, show DPMS value in  */
            /* minutes                      */
            if (in_smit_flag) {
                printf("%d %d %d\n",
                       dinfo.idle_time1/60,
                       dinfo.idle_time2/60,
                       dinfo.idle_time/60);
            }
            else {
                if (dinfo.idle_time1 == 0 &&
		    dinfo.idle_time2 == 0 &&
                    dinfo.idle_time == 0 ){
                    fprintf(stdout,MSGSTR(DPMS_DISABLE,DFLT_DPMS_DISABLE));
                }
                else {
                    fprintf(stdout,MSGSTR(DPMS, DFLT_DPMS),
                            dinfo.idle_time1/60,
                            dinfo.idle_time2/60,
                            dinfo.idle_time/60);
                }
            }
        }
        else {
            /* if other devices than lft0 has been specified */

            /* if the specified device is Graphical Output, */
            /* return with error.                           */
            if (dinfo.attribute & PMLIB_GRAPHICAL_OUTPUT){
                rc = PMCTRL_ERROR;
            }
            else{
                if (in_smit_flag) {
                    /* seconds --> minutes */
                    printf("%d %d\n", 
                           dinfo.idle_time/60,
                           dinfo.standby_time/60);
                }
                else {
                    fprintf(stdout,
                            MSGSTR(DEV_TIME, DFLT_DEV_TIME),
                            dinfo.name,
                            dinfo.idle_time/60,
                            dinfo.standby_time/60);
                }
            }
        }
    }
    return(rc);
}

/*
 *  NAME:       list_pm_dds()
 *
 *  FUNCTION:   Display pm aware devices
 *
 *  INPUTS:     none
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *  NOTES:
 *
 */
int
list_pm_dds()
{
    int    rc = PMCTRL_SUCCESS;
    char  *ptr;
    int    dnumber;
    pmlib_device_names_t pmdev;
    pmlib_device_info_t  dinfo;
    int graphical_output_found = FALSE;

    memset(&pmdev,0,sizeof(pmdev));
    memset(&dinfo,0,sizeof(dinfo));

    if (pmlib_request_parameter(PMLIB_QUERY_DEVICE_NUMBER,
                                (caddr_t)&dnumber) != PMLIB_SUCCESS) {
	if( errno == EBUSY ){
	    fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
	    errno = 0;
	}
        rc = PMCTRL_ERROR;
    }
    else {

        if(dnumber == 0){
            fprintf(stdout,MSGSTR(NO_PM_DD,DFLT_NO_PM_DD));
        }
        else{
            /* set number of devices */
            pmdev.number = dnumber;

            /* allocate area for dev name */
            Debug1("device number = %d\n",pmdev.number);

            pmdev.names = (char *)malloc( dnumber*16);
            
            if ( pmdev.names == NULL) {
                fprintf(stderr, MSGSTR(ERRMSG3, DFLT_ERRMSG3));
                rc = PMCTRL_ERROR;
            }
            else {
                /* clear the entry of the device name list */
                memset(pmdev.names,0,dnumber*16);
                if (pmlib_request_parameter(PMLIB_QUERY_DEVICE_NAMES,
                                            (caddr_t)&pmdev)!= PMLIB_SUCCESS) {
		    if( errno == EBUSY ){
			fprintf(stderr,MSGSTR(ERR_BUSY,DFLT_ERR_BUSY));
			errno = 0;
		    }
                    rc = PMCTRL_ERROR;
                }
                else {
                    fprintf(stdout,MSGSTR(MSG2, DFLT_MSG2));
                    /* display pm aware dev names   */
                    for (ptr = pmdev.names;
                         ptr < pmdev.names+16*(pmdev.number); ptr+=16) {

                        Debug1("now query attributes for %s\n",ptr);

                        /* display graphical output device as 'lft0' */

                        strncpy(dinfo.name,ptr,16);

                        if ( pmlib_request_parameter(PMLIB_QUERY_DEVICE_INFO,
                                 (caddr_t)&dinfo) == PMLIB_SUCCESS) {

                            /* indicate the existence of graphical 
                               output only once */

                            if (dinfo.attribute&PMLIB_GRAPHICAL_OUTPUT){
                                if (graphical_output_found == FALSE){
                                    fprintf(stdout,"lft0 (DPMS)\n" );
                                    graphical_output_found = TRUE;
                                }
                            }
                            else{
                                /* query device info succeeded and
                                   other devices than graphical output*/ 
                                fprintf(stdout,"%s\n",ptr);
                            }
                        }
                        else{
                            /* query device info failed */
                            fprintf(stdout,"%s\n",ptr);
                            Debug1("(%s:query device info failed)\n",ptr);
                        }
                    }
                }
            }
        }
    }
    if (pmdev.names!=NULL){
        Free(pmdev.names);
    }
    return(rc);
}


/*
 *  NAME:       print_current_state()
 *
 *  FUNCTION:   Display current pm state
 *
 *  INPUTS:     none
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *  NOTES:
 *
 */
int 
print_current_state()
{
    int rc = PMCTRL_SUCCESS;
    pmlib_state_t pms;

    memset(&pms,0,sizeof(pms));

    /* ask pm core of current state */
    rc = pmlib_request_state(PMLIB_QUERY_STATE, &pms);
    if (rc != PMLIB_SUCCESS) {
        rc = PMCTRL_ERROR;
    }
    else {
      switch (pms.state) {
        case PMLIB_SYSTEM_FULL_ON:
          if (in_smit_flag){        
              fprintf(stdout,"full_on\n"); 
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "full_on");
          }
          break;

        case PMLIB_SYSTEM_ENABLE:
          if (in_smit_flag){
              printf("enable\n"); 
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "enable");
          }
          break;

        case PMLIB_SYSTEM_STANDBY:
          if (in_smit_flag){
              fprintf(stdout,"standby\n"); 
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "standby");
          }
          break;

        case PMLIB_SYSTEM_SUSPEND:
          if (in_smit_flag){                
              fprintf(stdout,"suspend\n");         
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "suspend");
          }
          break;

        case PMLIB_SYSTEM_HIBERNATION:
          if (in_smit_flag){                
              fprintf(stdout,"hibernation\n");     
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "hibernation");
          }
          break;

        case PMLIB_SYSTEM_SHUTDOWN:
          if (in_smit_flag){                
              fprintf(stdout,"shutdown\n");        
          }
          else{
              fprintf(stdout,MSGSTR(MSG7, DFLT_MSG7), "shutdown");
          }
          break;

        default:
          fprintf(stderr, MSGSTR(ERRMSG24, DFLT_ERRMSG24), pms.state);
          rc = PMCTRL_ERROR;
          break;
      }
    }
    return(rc);
}


/*
 *  NAME:       print_pm_setting()
 *
 *  FUNCTION:   Display all pm information
 *
 *  INPUTS:     none
 *
 *  OUTPUTS:    PMCTRL_SUCCESS -- success
 *              PMCTRL_ERROR -- failure
 *  NOTES:
 *
 */
int 
print_pm_setting()
{
    /* error counter                 */
    int errflg = 0;

    /* show current pm state, etc.   */
    if (print_current_state() == PMCTRL_ERROR)
        /*  if err, count up err flag... */
        errflg++;
    if (list_pm_dds() == PMCTRL_ERROR)
        errflg++;
    if (query_duration(PMLIB_QUERY_SYSTEM_IDLE_TIME) == PMCTRL_ERROR)
        errflg++;
    if (query_duration(PMLIB_QUERY_DURATION_TO_HIBERNATION) == PMCTRL_ERROR)
        errflg++;

    /* show current action setting   */
    if (query_action_setting(PMLIB_QUERY_LID_CLOSE_ACTION) == PMCTRL_ERROR)
        errflg++;
    if (query_action_setting(PMLIB_QUERY_MAIN_SWITCH_ACTION) == PMCTRL_ERROR)
        errflg++;
    if (query_action_setting(PMLIB_QUERY_LOW_BATTERY_ACTION) == PMCTRL_ERROR)
        errflg++;
    if (query_action_setting(PMLIB_QUERY_SYSTEM_IDLE_ACTION) == PMCTRL_ERROR)
        errflg++;
    if (query_permission_setting() == PMCTRL_ERROR)
        errflg++;

    /* show current on-off status    */
    if (query_on_off_setting(PMLIB_QUERY_BEEP) == PMCTRL_ERROR)
        errflg++;
    if (query_on_off_setting(PMLIB_QUERY_PASSWD_ON_RESUME) == PMCTRL_ERROR)
        errflg++;
    if (query_on_off_setting(PMLIB_QUERY_RINGING_RESUME) == PMCTRL_ERROR)
        errflg++;
    if (query_on_off_setting(PMLIB_QUERY_KILL_LFT_SESSION) == PMCTRL_ERROR)
        errflg++;
    if (query_on_off_setting(PMLIB_QUERY_KILL_TTY_SESSION) == PMCTRL_ERROR)
        errflg++;
    if (query_on_off_setting(PMLIB_QUERY_KILL_SYNCD) == PMCTRL_ERROR)
        errflg++;

    /* show current alarm setting    */
    if (query_pm_alarm(PMLIB_QUERY_SPECIFIED_TIME) == PMCTRL_ERROR)     
        errflg++;       
    if (query_pm_alarm(PMLIB_QUERY_RESUME_TIME) == PMCTRL_ERROR)
        errflg++;

    /* error encountered...          */
    if (errflg){
        return(PMCTRL_ERROR);
    }
    else{
        return(PMCTRL_SUCCESS);
    }
}


/*
 *  NAME:       short_usage()
 *
 *  FUNCTION:   Display usage message and exit with error
 *
 *  INPUTS:     none
 *
 *  OUTPUTS:    n/a
 *
 *  NOTES:      This routine displays a version of the command for
 *              convenience of development. This should be deleted someday.
 *
 */
void 
short_usage()
{
    /* display short usage */
    fprintf(stdout, MSGSTR(USAGE1, DFLT_USAGE1));
    bye(PMCTRL_ERROR);
}

/*
 *  NAME:       long_usage()
 *
 *  FUNCTION:   Display usage message and exit with error
 *
 *  INPUTS:     none
 *
 *  OUTPUTS:    n/a
 *
 *  NOTES:      This routine displays a version of the command for
 *              convenience of development. This should be deleted someday.
 *
 */
void 
long_usage()
{
    fprintf(stdout, MSGSTR(USAGE1, DFLT_USAGE1));
    fprintf(stdout, MSGSTR(USAGE2, DFLT_USAGE2));
    fprintf(stdout, MSGSTR(USAGE3, DFLT_USAGE3));
    fprintf(stdout, MSGSTR(USAGE4, DFLT_USAGE4));
    fprintf(stdout, MSGSTR(USAGE5, DFLT_USAGE5));
    bye(PMCTRL_ERROR);
}

/*
 *  NAME:       bye()
 *
 *  FUNCTION:   exit program
 *
 *  INPUTS:     rc = return code
 *
 *  OUTPUTS:    none
 *
 *  NOTES:      This routine terminate the process
 *
 */
void 
bye(int rc)
{
    /* terminate odm access */
    odm_terminate();

    /* close message catalog */
    catclose(catd);

    if ( rc == PMCTRL_SUCCESS ){
        exit(EXIT_SUCCESS);
    }
    else{
        exit(EXIT_FAILURE);
    }
}

/*
 *  NAME:       init_args()
 *
 *  FUNCTION:   initialize additional arguments to optional flag.
 *
 *  NOTES:      
 *
 */
void
init_args()
{
    /* initialize arg pointer to NULL */
    b_arg = w_arg = r_arg = k_arg = y_arg = s_arg = NULL;
    a_arg = d_arg = g_arg = R_arg = NULL;
    S_args[0] = S_args[1] = NULL;
    t_args[0] = t_args[1] = t_args[2] = NULL;

    return;
}


/*
 *  NAME:       parse()
 *
 *  FUNCTION:   parses argv into flag_status.
 *
 *  NOTES:      
 *
 */
ulong
parse(int argc, char **argv)
{
    int fl;
    int errflg;
    ulong flag_status;
    ulong flag_status_old;
    int prefix_minus=FALSE;
    char inval_opt[3];

#ifdef PM_DEBUG
    int     argcount;
#endif

    init_args();
    memset(inval_opt,0,sizeof(inval_opt));


    errflg = 0;
    flag_status = 0;
    /* previous flag status for detecting duplicated flags     */
    flag_status_old = 0;

    /* process command line option.   */
    while ( (fl = getopt(argc,argv,":hvb:w:r:k:y:s:a:lpcxuet:S:R:d:g:-"))
            != EOF) {

#ifdef PM_DEBUG
        for(argcount=0;argcount<argc;argcount++){
            Debug2("argv[%d] = %s\n",argcount,argv[argcount]);
        }
#endif
        Debug1("optind = %d\n", optind);
        Debug1("optarg = %s\n", optarg);
        Debug1("optopt = %c\n", optopt);
        Debug1("opterr = %d\n", opterr);

        switch ((char)fl) {
          case 'h':
            flag_status |= H_FLAG;
            /* -h does not take parameters    */
            break;
          case 'v':
            flag_status |= V_FLAG;
            /* -v does not take parameters    */
            break;
          case 'b':
            flag_status |= B_FLAG;
            get_arg(argc,argv,1,&b_arg);
            Debug1("*************** b_arg = %s\n", b_arg);
            break;
          case 'w':
            flag_status |= W_FLAG;
            get_arg(argc,argv,1,&w_arg);
            Debug1("*************** w_arg = %s\n", w_arg);
            break;
          case 'r':
            flag_status |= R_FLAG;
            get_arg(argc,argv,1,&r_arg);
            Debug1("*************** r_arg = %s\n", r_arg);
            break;
          case 'k':
            flag_status |= K_FLAG;
            get_arg(argc,argv,1,&k_arg);
            Debug1("*************** k_arg = %s\n", k_arg);
            break;
          case 'y':
            flag_status |= Y_FLAG;
            get_arg(argc,argv,1,&y_arg);
            Debug1("*************** y_arg = %s\n", y_arg);
            break;
          case 's':
            flag_status |= S_FLAG;
            get_arg(argc,argv,1,&s_arg);
            Debug1("*************** s_arg = %s\n", s_arg);
            break;
          case 'a':
            flag_status |= A_FLAG;
            get_arg(argc,argv,1,&a_arg);
            Debug1("*************** a_arg = %s\n", a_arg);
            break;
          case 'l':
            flag_status |= L_FLAG;
            /* -l does not take parameters    */
            break;
          case 'p':
            flag_status |= P_FLAG;
            /* -p does not take parameters    */
            break;
          case 'c':
            flag_status |= C_FLAG;
            /* -c does not take parameters    */
            break;
          case 'x':
            flag_status |= X_FLAG;
            /* -x does not take parameters    */
            break;
          case 'u':
            flag_status |= U_FLAG;
            /* -u does not take parameters    */
            break;
          case 'e':
            flag_status |= E_FLAG;
            /* -e does not take parameters    */
            break;
          case 't':
            flag_status |= T_FLAG;
            if (flag_status & D_FLAG) { 
                /* if -d flag is specified        */
                /* if specified device is lft0,   */
                if ( strcmp(d_arg,"lft0")==0 ) {
                    /* get three values for DPMS      */
                    Debug0("DPMS setting\n");


                    if (get_arg(argc,argv,3,t_args) != PMCTRL_SUCCESS){
      
                        /* missing operand for -t flag */
                        fprintf(stderr,
                                MSGSTR(ERR_MISS_OPRND,DFLT_ERR_MISS_OPRND),
                                "-t");
                        short_usage();
                        bye(PMCTRL_ERROR);

                    }                        
                    Debug3("t_args[0] = %s, t_args[1] = %s, t_args[2] = %s\n", 
                            t_args[0], t_args[1], t_args[2]);
                }
                else {                     
                    Debug0("other PM aware device than DPMS\n");

                    if (get_arg(argc,argv,2,t_args) != PMCTRL_SUCCESS){

                        /* missing operand for -t flag */
                        fprintf(stderr,
                                MSGSTR(ERR_MISS_OPRND, DFLT_ERR_MISS_OPRND),
                                "-t");
                        short_usage();
                        bye(PMCTRL_ERROR);
                    }                        
                    Debug2("t_args[0] = %s, t_args[1] = %s\n", 
                            t_args[0], t_args[1]);
                }
            }
            else {               
                /* if -d flag is not specified    */
                get_arg(argc,argv,1,t_args);
                Debug3("t_args[0] = %s, t_args[1] = %s, t_args[2] = %s\n", 
                       t_args[0], t_args[1], t_args[2]);
            }
            break;
        case 'S':
            flag_status |= LS_FLAG;
            if (get_arg(argc,argv,2,S_args) != PM_SUCCESS){


                /* missing operand for -S flag */
                fprintf(stderr,
                        MSGSTR(ERR_MISS_OPRND, DFLT_ERR_MISS_OPRND),
                        "-S");
                short_usage();
                bye(PMCTRL_ERROR);
            }                        
            Debug2("S_args[0] = %s, S_args[1] = %s\n", 
                   S_args[0], S_args[1]);
            break;
          case 'R':
            flag_status |= LR_FLAG;
            get_arg(argc,argv,1,&R_arg);
            Debug1("*************** R_arg = %s\n", R_arg);
            break;
          case 'd':
            flag_status |= D_FLAG;
            get_arg(argc,argv,1,&d_arg);
            Debug1("*************** d_arg = %s\n", d_arg);
            break;
          case 'g':
            flag_status |= G_FLAG;
            get_arg(argc,argv,1,&g_arg);
            Debug1("*************** g_arg = %s\n", g_arg);
            break;

          case ':':
            /* if flag which expects argument */
            /* does not have argument,        */
            Debug0(": flag \n");
            switch( optopt ) {
              case 'b':
                flag_status |= B_FLAG;
                break;
              case 'w':
                flag_status |= W_FLAG;
                break;
              case 'r':
                flag_status |= R_FLAG;
                break;
              case 'k':
                flag_status |= K_FLAG;
                break;
              case 'y':
                flag_status |= Y_FLAG;
                break;
              case 's':
                flag_status |= S_FLAG;
                break;
              case 'a':
                flag_status |= A_FLAG;
                break;
              case 't':
                flag_status |= T_FLAG;
                /* t_args are all NULL */
                break;
              case 'S':
                flag_status |= LS_FLAG;
                /* S_args are all NULL */
                break;
              case 'R':
                flag_status |= LR_FLAG;
                break;
              case 'd':
                flag_status |= D_FLAG;
                break;
              case 'g':
                flag_status |= G_FLAG;
                break;
              default:
                errflg++;
                break;
            }
            break;
        default:
            prefix_minus = TRUE;
            errflg++;
            break;
        }

        if (errflg) {
            /* error encountered...           */
            if ( prefix_minus ){

                sprintf(inval_opt,"-%c",optopt);
                fprintf(stderr, 
                        MSGSTR(ERRMSG1, DFLT_ERRMSG1), 
                        inval_opt);
            }
            else{
                fprintf(stderr, 
                        MSGSTR(ERRMSG1, DFLT_ERRMSG1), 
                        argv[optind-1]);
            }
            short_usage();
            bye(PMCTRL_ERROR);
        }
        /* if same flag is specified      */
        /* twice, error...                */

        if (flag_status == flag_status_old) {
            fprintf(stderr, MSGSTR(ERR_TWICE, DFLT_ERR_TWICE));
            bye(PMCTRL_ERROR);
        }
        flag_status_old = flag_status;
        /* update old flag status         */
        /* end of cmd line processing     */
    }

    /* checking the case of '--' specified as option flag */
    if ( strcmp(argv[optind-1],"--")==0){
        fprintf( stderr, MSGSTR(ERRMSG1, DFLT_ERRMSG1), "--");
        short_usage();
        bye(PMCTRL_ERROR);
    }

    /* checking '-' alone and the letter not beginning with '-' */
    argv += optind;
    argc -= optind;
    if (argc) {
        fprintf( stderr, MSGSTR(ERRMSG1, DFLT_ERRMSG1), *argv);
        short_usage();
        bye(PMCTRL_ERROR);
    }

    return flag_status;
}

/*
 * NAME:      process_h_flag
 *
 * FUNCTION:  processing h flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                      /*--------------------------------*/
process_h_flag(int flag_status)           /* -h flag                        */
{                                         /*--------------------------------*/
   Debug0("process_h_flag\n");

   if ( flag_status & (ulong)H_FLAG ) {
        rc = PMCTRL_SUCCESS;
        /* if only -h flag exists, */
        if ( flag_status == (ulong)H_FLAG ) {
            /* print long_usage */
            long_usage();
        }
        else { 
            /* otherwise, error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-h");
            rc = PMCTRL_ERROR;
        }
        bye(rc);
    }
}

/*
 * NAME:      process_v_flag
 *
 * FUNCTION:  processing v flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                      /*--------------------------------*/
process_v_flag(int flag_status)           /* -v flag                        */
{                                         /*--------------------------------*/
   Debug0("process_v_flag\n");


   if ( flag_status & (ulong)V_FLAG ) {
        rc = PMCTRL_SUCCESS;
        /* if only -v flag exists, */
        if ( flag_status == (ulong)V_FLAG ) {
            /* print pm setting */
            if (print_pm_setting() == PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_ALLINFO, DFLT_ERR_ALLINFO));
                rc = PMCTRL_ERROR;
            }
        }
        else {
            /* otherwise, error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-v");
            rc = PMCTRL_ERROR;
        }
        bye(rc);
    }
}

/*
 * NAME:      process_a_flag
 *
 * FUNCTION:  processing a flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                       /*--------------------------------*/
process_a_flag(int flag_status)            /* -a flag                        */
{                                          /*--------------------------------*/
   Debug0("process_a_flag\n");

   /* if -a flag exists */
   if ( flag_status & (ulong)(A_FLAG) ) {

       /* if only -a flag exists, */
       if ( flag_status == (ulong)(A_FLAG) ) {
           rc = PMCTRL_SUCCESS;
       
           if (a_arg == NULL) {
               /* if -a flag with no argument,   */
               /* print current pm state         */
               if (print_current_state() == PMCTRL_ERROR) {
                   fprintf(stderr, MSGSTR(ERRMSG23, DFLT_ERRMSG23));
                   rc = PMCTRL_ERROR;
               }        
           }    
           else {
               /* otherwise, error */
               fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-a");
               rc = PMCTRL_ERROR;
           }
           bye(rc);
       }
       else{
           /* check the combination with -a flags */
           if ( !( (flag_status == (ulong)(E_FLAG|A_FLAG)) ||
                   (flag_status == (ulong)(L_FLAG|A_FLAG)) ||
                   (flag_status == (ulong)(P_FLAG|A_FLAG)) ||
                   (flag_status == (ulong)(C_FLAG|A_FLAG)) ||
                   (flag_status == (ulong)(X_FLAG|A_FLAG)) ||
                   (flag_status == (ulong)(U_FLAG|A_FLAG)) ) ){
               fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-a");
               rc = PMCTRL_ERROR;
               bye(rc);
           } 
       } 
   } 
}


/*
 * NAME:      process_d_flag
 *
 * FUNCTION:  processing d flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                       /*--------------------------------*/
process_d_flag(int flag_status)            /* -d flag                        */
{                                          /*--------------------------------*/
    Debug0("process_d_flag\n");

    if ( flag_status & (ulong)D_FLAG) {
        /* if -d flag exists, */
        rc = PMCTRL_SUCCESS;
        /* if only -d flag exists, */
        if ( flag_status == (ulong)D_FLAG ) {
            /* if -d flag with no argument, */
            if (d_arg == NULL) {
                if (list_pm_dds() == PMCTRL_ERROR) {
                    /* print registered pm devices */
                    fprintf(stderr, MSGSTR(ERRMSG4, DFLT_ERRMSG4));
                    rc = PMCTRL_ERROR;
                }
            }
            else {
               /* d_arg!= NULL */
               /* print idle time for specified device */
 
                if (query_device_idle_time(d_arg) == PMCTRL_ERROR){
                    fprintf(stderr, MSGSTR(ERRMSG18, DFLT_ERRMSG18),d_arg);
                    rc = PMCTRL_ERROR;
                }
            }
        }
        /* only -d and -t flags exist, */
        else if ( flag_status == (ulong)(D_FLAG|T_FLAG) ) {
            /* if -t flag with no argument */
            if ( (t_args[0] == NULL) || (t_args[1] ==NULL) ) {
                if (query_device_idle_time(d_arg) == PMCTRL_ERROR){
                    fprintf(stderr, MSGSTR(ERRMSG18, DFLT_ERRMSG18),d_arg);
                    rc = PMCTRL_ERROR;
                }
            }
            else {
                /* set device idle time */
                /* DPMS needs 3 values  */
                if (set_device_idle_time(d_arg,t_args[0],t_args[1],t_args[2]) 
                    == PMCTRL_ERROR) {
                    fprintf(stderr, MSGSTR(ERRMSG15, DFLT_ERRMSG15),d_arg);
                    rc = PMCTRL_ERROR;
                }
                /* query device idle time */
                if (query_device_idle_time(d_arg) == PMCTRL_ERROR){
                    fprintf(stderr, MSGSTR(ERRMSG18, DFLT_ERRMSG18),d_arg);
                    rc = PMCTRL_ERROR;
                }
            }
        }
        else {   
            /* otherwise, error. */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-d");
            rc = PMCTRL_ERROR;
        }
        bye(rc);
    }
}

/*
 * NAME:      process_e_flag
 *
 * FUNCTION:  processing e flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */

void                                       /*--------------------------------*/
process_e_flag(int flag_status)            /* -e flag                        */
{                                          /*--------------------------------*/
    int action;
    /* the case of only -a flag have  */
    /* been already processed         */
    
    /* if -e flags exist, */
    Debug0("process_e_flag\n");
    if ( flag_status & (ulong)E_FLAG) {
        rc = PMCTRL_SUCCESS;

        if (flag_status == (ulong)(A_FLAG|E_FLAG)) {
            if (a_arg == NULL) {
                fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-a");
                rc = PMCTRL_ERROR;
            }
            else {
                if (chkarg_action(a_arg, &action) == PMCTRL_ERROR) {
                    fprintf(stderr, MSGSTR(ERRMSG7, DFLT_ERRMSG7),a_arg);
                    rc = PMCTRL_ERROR;
                }
                else {
                    /* start system state transition  */
                    if (start_action(action) == PMCTRL_ERROR) {
                        fprintf(stderr, MSGSTR(ERRMSG22, DFLT_ERRMSG22),a_arg);
                        rc = PMCTRL_ERROR;
                    }
                }
            }
        }
        else {      
            /* error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-e");
            rc = PMCTRL_ERROR;
        }
        bye(rc);
    }

}


/*
 * NAME:      process_lpcxu_flag
 *
 * FUNCTION:  processing l, p, c, x, and u flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                       /*--------------------------------*/
process_lpcxu_flag(int flag_status)        /* -l, -p, -c, -x, -u flags       */
{                                          /*--------------------------------*/

    /* a, a & e, a & T:               */
    /*   these flag combinations have */
    /*   been already processed.      */
    
    Debug0("process_lpcxu_flag\n");

    if (flag_status & (ulong)L_FLAG) {
        /* if -l flag exists, */
        rc = PMCTRL_SUCCESS;
       
        /* if -a flag with argument */
        if ( flag_status == (ulong)(A_FLAG|L_FLAG)&&
             a_arg !=NULL ) {

         if (set_action(a_arg,PMLIB_SET_LOW_BATTERY_ACTION) == PMCTRL_ERROR){
                fprintf(stderr,
                        MSGSTR(ERRMSG8, DFLT_ERRMSG8),"low battery condition");
                rc = PMCTRL_ERROR;
            }
        }
        /* query setting for confirmation */
        if ( flag_status == (ulong)(L_FLAG) ||
             flag_status == (ulong)(A_FLAG|L_FLAG)
            ){
            
            if (query_action_setting(PMLIB_QUERY_LOW_BATTERY_ACTION) == 
                PMCTRL_ERROR){
                fprintf(stderr, MSGSTR(ERRMSG11, DFLT_ERRMSG11),
                        "low battery condition");
                rc = PMCTRL_ERROR;
            }
        }
        else {
            /* otherwise, error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-l");
            rc = PMCTRL_ERROR;
        }
        
        bye(rc);
    }

    if (flag_status & (ulong)P_FLAG) {
        /* if -p flag exists, */
        rc = PMCTRL_SUCCESS;

        /* if -a flags with argument */
        if ( flag_status == (ulong)(A_FLAG|P_FLAG) &&
             a_arg != NULL ) {

         if (set_action(a_arg, PMLIB_SET_MAIN_SWITCH_ACTION) == PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERRMSG8, DFLT_ERRMSG8), "main switch pressed");
                rc = PMCTRL_ERROR;
            }
        }
        /* query setting for confirmation */
        if ( flag_status == (ulong)(P_FLAG) ||
             flag_status == (ulong)(A_FLAG|P_FLAG)
            ){

            if (query_action_setting(PMLIB_QUERY_MAIN_SWITCH_ACTION)
                == PMCTRL_ERROR){
                fprintf(stderr,
                        MSGSTR(ERRMSG11, DFLT_ERRMSG11), "main switch pressed");
                rc = PMCTRL_ERROR;
            }
        }
        else {  
            /* otherwise, error. */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-p");
            rc = PMCTRL_ERROR;
        }

        bye(rc);
    }

    if (flag_status & (ulong)C_FLAG){
        /* if -c flag exists, */
        rc = PMCTRL_SUCCESS;

        /* if -a flag with argument */
        if ( flag_status == (ulong)(A_FLAG|C_FLAG) &&
             a_arg != NULL){
                
            if (set_action(a_arg,PMLIB_SET_LID_CLOSE_ACTION) == PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERRMSG8, DFLT_ERRMSG8), "notebook-lid close");
                rc = PMCTRL_ERROR;
            }
        }

        /* query setting for confirmation */
        if ( flag_status == (ulong)(C_FLAG) ||
             flag_status == (ulong)(A_FLAG|C_FLAG)
            ){
            /* print current action setting */
            /* for the time notebook lid is */
            /* closed.                      */

            if (query_action_setting(PMLIB_QUERY_LID_CLOSE_ACTION)
                == PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERRMSG11, DFLT_ERRMSG11), "notebook-lid close");
                rc = PMCTRL_ERROR;
            }
        }
        else {
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-c");
            rc = PMCTRL_ERROR;
        }

        bye(rc);
    }

    if (flag_status & (ulong)X_FLAG) {
        /* if -x flag exists, */
        rc = PMCTRL_SUCCESS;

        /* if only -x flag specified, */
        if ( flag_status == (ulong)(A_FLAG|X_FLAG) &&
             a_arg != NULL){

         if (set_action(a_arg, PMLIB_SET_SYSTEM_IDLE_ACTION) == PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERRMSG8, DFLT_ERRMSG8), 
                        "system idle timer expiration");
                rc = PMCTRL_ERROR;
            }
        }

        /* query setting for confirmation */
        if ( flag_status == (ulong)(X_FLAG) ||
             flag_status == (ulong)(A_FLAG|X_FLAG)
           ){
            /* print current action setting */
            /* for the time system timer    */
            /* expires                      */

            if (query_action_setting(PMLIB_QUERY_SYSTEM_IDLE_ACTION)
                == PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERRMSG11, DFLT_ERRMSG11), 
                        "system idle timer expiration");
                rc = PMCTRL_ERROR;
            }
        }
        else {
            /* otherwise, error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-x");
            rc = PMCTRL_ERROR;
        }
        bye(rc);
    }

    if (flag_status & (ulong)U_FLAG) {
        /* if -u flag exists, */
        rc = PMCTRL_SUCCESS;

        /* if -a flag with argument */
        if ( flag_status == (ulong)(A_FLAG|U_FLAG) &&
             a_arg != NULL) {

            if (set_permission(a_arg) == PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERR_S_PERM, DFLT_ERR_S_PERM));
                rc = PMCTRL_ERROR;
            }
        }

        /* query setting for confirmation */
        if ( flag_status == (ulong)(U_FLAG) ||
             flag_status == (ulong)(A_FLAG|U_FLAG)
           ){
            /* print current permission for */
            /* the actions general user can */
            /* executes                     */
            
            if (query_permission_setting() == PMCTRL_ERROR) {
                fprintf(stderr, 
                        MSGSTR(ERR_Q_PERM, DFLT_ERR_Q_PERM));
                rc = PMCTRL_ERROR;
            }
        }
        else {
            /* otherwise, error */
            fprintf(stderr, MSGSTR(ERR_PARSE, DFLT_ERR_PARSE), "-u");
            rc = PMCTRL_ERROR;
        }
        
        bye(rc);
    }
}

/*
 * NAME:      process_tg_flag
 *
 * FUNCTION:  processing t and g flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void
process_tg_flag(int flag_status)           /*--------------------------------*/
{                                          /* -t, -g flags                   */
                                           /*--------------------------------*/

    /* d & t:                       */
    /*   This flag combination have */
    /*   been already processed     */

    Debug0("process_tg_flag\n");

    /* if only t flag exists   */
    if ( flag_status & (ulong)T_FLAG ) {   

        Debug0("t_flag detected \n");

        /* if t flag has the argument */
        if (t_args[0] != NULL) {
           
            Debug1("t_args[0] = %s \n",t_args[0]);

            if (set_duration(t_args[0],PMLIB_SET_SYSTEM_IDLE_TIME)
                == PMCTRL_ERROR){
                fprintf(stderr, MSGSTR(ERRMSG20, DFLT_ERRMSG20),t_args[0]);
                rc--;
            }
        }
        if (query_duration(PMLIB_QUERY_SYSTEM_IDLE_TIME) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERRMSG21, DFLT_ERRMSG21));
            rc--;
        }
    }
    /* end of processing t_flag */

    if ( flag_status & (ulong)G_FLAG ){
        /* if -g flag exists, */

        if (g_arg != NULL) {

     if (set_duration(g_arg,PMLIB_SET_DURATION_TO_HIBERNATION)== PMCTRL_ERROR){
                fprintf(stderr, MSGSTR(ERR_S_DUR, DFLT_ERR_S_DUR), g_arg);
                rc--;
            }
        }
        if (query_duration(PMLIB_QUERY_DURATION_TO_HIBERNATION)
            == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_DUR, DFLT_ERR_Q_DUR));
            rc--;
        }
    }
    /* end of processing g_flag */
}

/*
 * NAME:      process_bwrkys_flag
 *
 * FUNCTION:  processing b, w, k, y, and s flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void
process_bwrkys_flag(int flag_status)
{
                                           /*--------------------------------*/
                                           /* -b, -w, -r, -k, -y -s flags    */
                                           /*--------------------------------*/
    Debug0("process_bwrkys_flag\n");

    /* if -b flag exists */
    if ( flag_status & (ulong)B_FLAG ) {
        Debug0("b_flag detected.\n");
        if (b_arg != NULL) {               
            if (set_on_off(b_arg, PMLIB_SET_BEEP)== PMCTRL_ERROR) {
                fprintf(stderr,
                        MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "Beep",
                        b_arg);
                rc--;
            }
        }

        /* inside query_on_off_setting ,the setting will be displayed */
        if (query_on_off_setting(PMLIB_QUERY_BEEP) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),"beep");
            rc--;
        }
    }
 
    /* if -w flag exists, */
    if ( flag_status & (ulong)W_FLAG ) {
        Debug0("w_flag detected.\n");
        if (w_arg != NULL) { 
            if (set_on_off(w_arg, PMLIB_SET_PASSWD_ON_RESUME)== PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "resume password", w_arg);
                rc--;
            }
        }
       /* inside query_on_off_setting ,the setting will be displayed */
       if (query_on_off_setting(PMLIB_QUERY_PASSWD_ON_RESUME) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),
                    "resume password");
            rc--;
        }
    }

    /* if -r flag exists, */
    if ( flag_status & (ulong)R_FLAG ) {
        if (r_arg != NULL) { 
            if (set_on_off(r_arg, PMLIB_SET_RINGING_RESUME)== PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "ringing resume", r_arg);
                rc--;
            }
        }
        /* inside query_on_off_setting ,the setting will be displayed */
        if (query_on_off_setting(PMLIB_QUERY_RINGING_RESUME) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),
                    "ringing resume");
            rc--;
        }
    }
    /* if -k flag exists, */
    if ( flag_status & (ulong)K_FLAG ) {
        if (k_arg != NULL) {              
            if (set_on_off(k_arg, PMLIB_SET_KILL_LFT_SESSION)== PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "LFT termination", k_arg);
                rc--;
            }
        }
       /* inside query_on_off_setting ,the setting will be displayed */
       if (query_on_off_setting(PMLIB_QUERY_KILL_LFT_SESSION) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),
                    "LFT termination");
            rc--;
        }
    }

    /* if -y flag exists, */
    if ( flag_status & (ulong)Y_FLAG ) {
        if (y_arg != NULL) {              
            if (set_on_off(y_arg, PMLIB_SET_KILL_TTY_SESSION) == PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "TTY termination",y_arg);
                rc--;
            }
        }
        /* inside query_on_off_setting ,the setting will be displayed */
       if (query_on_off_setting(PMLIB_QUERY_KILL_TTY_SESSION) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),
                    "TTY termination");
            rc--;
        }
    }

    /* if -s flag exists, */
    if ( flag_status & (ulong)S_FLAG ) {
        if (s_arg != NULL) {              
            if (set_on_off(s_arg, PMLIB_SET_KILL_SYNCD) == PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ON_OFF, DFLT_ERR_S_ON_OFF),
                        "syncd termination",s_arg);
                rc--;
            }
        }
        /* inside query_on_off_setting ,the setting will be displayed */
        if (query_on_off_setting(PMLIB_QUERY_KILL_SYNCD) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ON_OFF, DFLT_ERR_Q_ON_OFF),
                    "syncd termination");
            rc--;
        }
    }
}


/*
 * NAME:      process_SR_flag
 *
 * FUNCTION:  processing S and R flags.
 *
 * NOTES:     
 * 
 * RETURN VALUE DESCRIPTION:
 *            none
 */
void                                        /*-------------------------------*/
process_SR_flag(int flag_status)            /* -S, -R flags                  */
{                                           /*-------------------------------*/
                                            /* if -S flag exists,            */
    Debug0("process_SR_flag\n");

    if (flag_status & (ulong)LS_FLAG) {
        if (S_args[0] != NULL) {
            if (set_pm_alarm(PMLIB_SET_SPECIFIED_TIME,S_args[0],S_args[1])
                == PMCTRL_ERROR) {
                fprintf(stderr, MSGSTR(ERR_S_ALARM, DFLT_ERR_S_ALARM), 
                        "power saving state");
                rc--;
            }
        }
        if (query_pm_alarm(PMLIB_QUERY_SPECIFIED_TIME) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ALARM, DFLT_ERR_Q_ALARM),
                    "power saving state");
            rc--;
        }
    }

    /* if -R flag exists, */
    if (flag_status & (ulong)LR_FLAG) {
        if (R_arg != NULL) {
            if (set_pm_alarm(PMLIB_SET_RESUME_TIME,R_arg,NULL) == PMCTRL_ERROR){
            fprintf(stderr, MSGSTR(ERR_S_ALARM, DFLT_ERR_S_ALARM), "resume");
                rc--;
            }
        }
        if (query_pm_alarm(PMLIB_QUERY_RESUME_TIME) == PMCTRL_ERROR) {
            fprintf(stderr, MSGSTR(ERR_Q_ALARM, DFLT_ERR_Q_ALARM), "resume");
            rc--;
        }
    }
}

/*
 * NAME:      most_signigicant_state
 *
 * FUNCTION:  pick up most significant state from bitwiseORed states.
 *
 * NOTES:     inverse function of bitwiseORed_state()
 * 
 * RETURN VALUE DESCRIPTION:
 *
 *            most_significant_state
 */
int 
most_significant_state( int bitwiseORed_state)
{
   int pmlib_state ;

   pmlib_state = PMLIB_NONE;

   if ( bitwiseORed_state == PMLIB_NONE ){
       pmlib_state = PMLIB_NONE;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_FULL_ON ){
       pmlib_state = PMLIB_SYSTEM_FULL_ON;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_ENABLE ){
       pmlib_state = PMLIB_SYSTEM_ENABLE;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_STANDBY ){
       pmlib_state = PMLIB_SYSTEM_STANDBY;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_SUSPEND ){
       pmlib_state = PMLIB_SYSTEM_SUSPEND;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_HIBERNATION ){
       pmlib_state = PMLIB_SYSTEM_HIBERNATION;
   }
   if ( bitwiseORed_state &  PMLIB_SYSTEM_SHUTDOWN ){
       pmlib_state = PMLIB_SYSTEM_SHUTDOWN;
   }

   return pmlib_state;
}

/*
 * NAME:      bitwiseORed_state
 *
 * FUNCTION:  build bitwiseORed state of the most significant state.
 *
 * NOTES:     inverse function of most_significant_state.
 * 
 * RETURN VALUE DESCRIPTION:
 *
 *            bitwiseORed state
 */

int 
bitwiseORed_state( int pmlib_state)
{
   int bitwiseORed_state;

   bitwiseORed_state = PMLIB_NONE;

   switch(pmlib_state){

   case PMLIB_NONE :
       bitwiseORed_state = PMLIB_NONE;
       break;
   case PMLIB_SYSTEM_FULL_ON:   
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON ;
       break;
   case PMLIB_SYSTEM_ENABLE:
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON |
                            PMLIB_SYSTEM_ENABLE ;
       break;
   case PMLIB_SYSTEM_STANDBY:
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON |
                            PMLIB_SYSTEM_ENABLE  |
                            PMLIB_SYSTEM_STANDBY; 
       break;
   case PMLIB_SYSTEM_SUSPEND:
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON |
                            PMLIB_SYSTEM_ENABLE  |
                            PMLIB_SYSTEM_STANDBY |
                            PMLIB_SYSTEM_SUSPEND;
       break;
   case PMLIB_SYSTEM_HIBERNATION:
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON |
                            PMLIB_SYSTEM_ENABLE  |
                            PMLIB_SYSTEM_STANDBY |
                            PMLIB_SYSTEM_SUSPEND |
                            PMLIB_SYSTEM_HIBERNATION;
       break;
   case PMLIB_SYSTEM_SHUTDOWN:
       bitwiseORed_state =  PMLIB_SYSTEM_FULL_ON |
                            PMLIB_SYSTEM_ENABLE  |
                            PMLIB_SYSTEM_STANDBY |
                            PMLIB_SYSTEM_SUSPEND |
                            PMLIB_SYSTEM_HIBERNATION|
                            PMLIB_SYSTEM_SHUTDOWN;
       break;
   }

   return bitwiseORed_state;
}
/*
 * NAME:      pmlib2pmstate
 *
 * FUNCTION:  convert system state macros for PMLIB into those for PM
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *
 *        PM_SYSTEM_XXXXX   :successfully processed. 
 *        PM_NONE           :Invalid arguments specified.
 */
int 
pmlib2pmstate(int pmlib_state )
{
    int pm_state;

    pm_state = PM_NONE;

    if (pmlib_state & PMLIB_SYSTEM_FULL_ON){
        pm_state |= PM_SYSTEM_FULL_ON;
    }
    if (pmlib_state & PMLIB_SYSTEM_ENABLE){
        pm_state |= PM_SYSTEM_ENABLE;
    }
    if (pmlib_state & PMLIB_SYSTEM_STANDBY){
        pm_state |= PM_SYSTEM_STANDBY;
    }
    if (pmlib_state & PMLIB_SYSTEM_SUSPEND){
        pm_state |= PM_SYSTEM_SUSPEND;
    }
    if (pmlib_state & PMLIB_SYSTEM_HIBERNATION){
        pm_state |= PM_SYSTEM_HIBERNATION;
    }
    if (pmlib_state & PMLIB_SYSTEM_SHUTDOWN){
        pm_state |= PMLIB_SYSTEM_SHUTDOWN;
    }

    return pm_state;
}

/*
 * NAME:     pmlibonoff2boolean
 *
 * FUNCTION: convert PMLIB_ON/PMLIB_OFF into TRUE/FALSE.
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE
 *           FALSE
 */
int
pmlibonoff2boolean(int pmlib_onoff)
{
    if( pmlib_onoff == PMLIB_ON ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}
