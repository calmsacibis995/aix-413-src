static char sccsid[] = "@(#)22  1.13  src/rspc/usr/bin/pmd/pmd_socket.c, pwrcmd, rspc41J, 9523A_all 6/5/95 23:30:56";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS:socket_interface,
 *           socket_loop,
 *           create_socket,
 *           close_socket,
 *           cleanup_for_socket_thread,
 *           receive_data,
 *           send_data,
 *           process_data,
 *           check_version,
 *           switch_api_operations,
 *           process_request_state,
 *           reset_request_entry_count,
 *           count_request_entry,
 *           request_count_reached,
 *           check_state_change_process_flag,
 *           check_request_state_permission,
 *           process_get_event_notice,
 *           process_request_battery,
 *           query_battery,
 *           discharge_battery,
 *           process_request_parameter,
 *           ddlist_filter,
 *           check_superuser_privilege,
 *           process_register_application,
 *           register_application,
 *           show_entry(for debugging),
 *           search_entry,
 *           broadcast_signal,
 *           unregister_application,
 *           send_data
 *           wakeup_main_thread,
 *           pm2pmlibstate,
 *           pmlib2pmstate,
 *           is_pm_state,
 *           is_pm_action_state,
 *           is_pm_permission,
 *           is_pmlib_system_support,
 *           is_pmlib_state,
 *           is_pmlib_action_state,
 *           is_pmlib_permission,
 *           is_pmlib_system_support,
 *           is_pmlib_onoff,
 *           is_boolean,
 *           is_timer,
 *           is_specified_time,
 *           is_pm_supported,
 *           pmd2userparam,
 *           user2pmdparam,
 *           pmlibonoff2boolean,
 *           pm2pmlibdevinfo,
 *           pm2pmlibbattery,
 *           pmstate2asc,
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*-------------------------------*/ 
/* Subroutines for Socket Thread */
/*-------------------------------*/

#include "pmd.h"
#include "pmd_global_ext.h"

/*------------------*/
/* local variables  */
/*------------------*/

int sockfd,servlen,clilen;
struct sockaddr_un serv_addr,cli_addr;

/* data for IPC */

struct pmlib_client clientdata;
struct pmlib_server *pserverdata;

int workarea_size;

/* signal set */
sigset_t sigset_socket;

/*
 * NAME:      create_socket
 *
 * FUNCTION:  create the IPC socket between PM library and PM daemon.
 *
 * NOTES:     this routine is used for IPC between PM library and PM daemon.
 *
 * RETURN VALUE DESCRIPTION:
 *             PM_SUCCESS - succesfully processed
 *             PM_ERROR - invalid ctrl argument or Unsuccessfully processed
 */

int
create_socket(int *psockfd, 
              struct sockaddr_un *pcli_addr,
              struct sockaddr_un *pserv_addr,
              char *unixdg_path
              )
{
    /* 
     * Open a Socket ( a UNIX domain datagram socket)
     */

    if ( ( *psockfd = socket(AF_UNIX,SOCK_DGRAM,0) ) < 0){
        fprintf(stderr,MSGSTR(ERR_OPEN_SOCKET,DFLT_ERR_OPEN_SOCKET));
        return PM_ERROR;
    }
  
    /*
     * Bind our local address so that the client can send to us
     */
   
    unlink(unixdg_path);   /* in case it was left from last time */

    memset((char *)pserv_addr,0,sizeof(struct sockaddr_un));
    pserv_addr->sun_family = AF_UNIX ;
    strncpy(pserv_addr->sun_path,unixdg_path,sizeof(pserv_addr->sun_path));
    servlen = SUN_LEN(pserv_addr);

    if ( bind( *psockfd, (struct sockaddr *)pserv_addr,servlen) <0){
        fprintf(stderr,MSGSTR(ERR_BIND,DFLT_ERR_BIND));
        return PM_ERROR;
    }

   return PM_SUCCESS;
}


/*
 * NAME:     close_socket
 *
 * FUNCTION: close the socket for the IPC between pm daemon and pm library.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: successfully processed.
 */
int
close_socket(int sockfd,
             struct sockaddr_un serv_addr)
{
  close(sockfd);
  unlink(serv_addr.sun_path);
  return PM_SUCCESS;
}

/*
 * NAME:     cleanup_for_socket_thread
 *
 * FUNCTION: cleanup handler for socket thread
 *
 * NOTES:    executed on the thread environment (socket_thread).
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE
 */

void 
cleanup_for_socket_thread(void* arg)
{
  Debug0("clean up handler for socket thread called.\n");

  close_socket(sockfd,serv_addr);

  return;
}

/*
 * NAME:     socket_interface
 *
 * FUNCTION: handles IPC with pm library
 *
 * NOTES:    executed on the thread environment (socket_thread).
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE 
 */
void *
socket_interface(void* arg)
{
    Debug0("*** Socket Interface Start ***\n");

    /* increment thread_birth */

    Mutex_Lock(&thread_birth_mutex);

    thread_birth++;

    Mutex_Unlock(&thread_birth_mutex);

    /* set signal mask */
    signal_ignore(&sigset_socket);

    /* create socket */
    create_socket(&sockfd,&cli_addr,&serv_addr,PMLIB_DAEMON_PATH);

    /* push cleanup handler */
    pthread_cleanup_push(cleanup_for_socket_thread,NULL);

    /* socket thread loop */
    socket_loop();

    /* NOT REACHED */

    /* pop cleanup handler */
    pthread_cleanup_pop(0);  
    /* to be called when cleanup handler is no longer needed.*/

    pthread_exit(NULL);
}

/*
 * NAME:     socket_loop
 *
 * FUNCTION: handles IPC with pm library.
 *
 * NOTES:
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE
 */
void
socket_loop()
{
    Debug0("socket_loop\n");

    for(;;){
        receive_data();    /* receive data from pmlib through IPC */
        process_data();    /* process data */
        send_data();       /* send back data to pmlib through IPC */
    }
    /* NOT REACHED */

    return;
}


/*
 * NAME:     receive_data  
 *
 * FUNCTION: receives IPC data  
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
receive_data()
{
    int n;

    Debug0("receive data \n");

    /* clear the member variables in the data structures for IPC*/

    memset( (char *)&clientdata,0,sizeof(struct pmlib_client));
    
    /* server data is handled by pserverdata */

    if ( ( pserverdata = malloc(sizeof(struct pmlib_server)))!= NULL){
        memset(pserverdata,0,sizeof(struct pmlib_server));
    }
    else{
        fprintf(stderr,MSGSTR(ERR_MEMORY,DFLT_ERR_MEMORY));
        exit(EXIT_FAILURE);
    }

    /* receive datagram socket */  
  
    clilen = sizeof(struct sockaddr);

    n = recvfrom(
                sockfd,
                &clientdata,
                sizeof(struct pmlib_client),
                0,
                (struct sockaddr *)&cli_addr,&clilen
                );

        
    if(n<0){
            Debug0("receivedata:recvfrom error\n");
            fprintf(stderr,MSGSTR(ERR_RECV,DFLT_ERR_RECV));
            return PM_ERROR;
        }

    /* set identifier in server data */
    
    pserverdata->version = clientdata.version;
    pserverdata->ftype   = clientdata.ftype;
    pserverdata->ctrl    = clientdata.ctrl;

    Debug1("recvdata: pserverdata->version = %d \n",pserverdata->version);
    Debug1("recvdata: pserverdata->ftype = %d \n",pserverdata->ftype);
    Debug1("recvdata: pserverdata->ctrl = %d \n",pserverdata->ctrl);

    return PM_SUCCESS;
}


/*
 * NAME: process_data  
 *
 * FUNCTION: process IPC data  
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : processed unsuccessfully.
 */

int 
process_data()
{
    Debug0("process_data\n");

    /* check version of IPC between lib and daemon */

    if (check_version()==PM_SUCCESS){
        switch_api_operations();
    }
    else{
    /* if check_version failed, error is set inside it
       and return , then proceeds to send_data */

    /* if switch_api_operations failed, error is set inside it
       and return , then proceeds to send_data */
        return PM_ERROR;
    }
    return PM_SUCCESS;    /* data processing is always successful */
}

/*
 * NAME:     check_version
 *
 * FUNCTION: check version of IPC
 *
 * NOTES:    error is set in *pserverdata in unsuccessful case.
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
check_version()
{
    Debug1("check_version(#%d)\n",clientdata.version);

    if ( clientdata.version == PMLIB_VERSION 
        /* currently 1 */
        ){

        return PM_SUCCESS;
    }
    else{

        fprintf(stderr,MSGSTR(ERR_VERSION,DFLT_ERR_VERSION));

        /* version mismatch error set */ 

        pserverdata->rc = PMLIB_ERROR;
        pserverdata->length = sizeof(struct pmlib_server);

        return PM_ERROR;

    }
    /* NOT REACHED */
}

/*
 * NAME:     switch_api_operations
 *
 * FUNCTION: switch operations according to the clients' control.
 *
 * NOTES:    error is set in *pserverdata in unsuccessful case.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */

int
switch_api_operations()
{
    Debug0("switch_api_operations\n");

    switch ( clientdata.ftype ){
    case PMLIB_INTERNAL_REQUEST_STATE:
        process_request_state();
        break;
    case PMLIB_INTERNAL_GET_EVENT_NOTICE:
        process_get_event_notice();
        break;
    case PMLIB_INTERNAL_REQUEST_PARAMETER:
        process_request_parameter();
        break;
    case PMLIB_INTERNAL_REQUEST_BATTERY:
        process_request_battery();
        break;
    case PMLIB_INTERNAL_REGISTER_APPLICATION:
        process_register_application();
        break;
    default:
        fprintf(stderr,MSGSTR(ERR_INVAL_FUNC,DFLT_ERR_INVAL_FUNC));

        pserverdata->rc = PMLIB_ERROR;
        pserverdata->length = sizeof(struct pmlib_server);
        return PM_ERROR;

    }/* end of switch */

    return PM_SUCCESS;  /* operation has been processed successfully */
}


/*
 * NAME:     process_request_state
 *
 * FUNCTION: processes the pmlib_request_state API call.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */

int process_request_state()
{
    int lc;  /* local condition variable */
    int rc;

    Debug0("process_request_state \n");

    switch(clientdata.ctrl){
    case PMLIB_REQUEST_STATE:

        Debug0("prc_req_stat:request state\n");
        
        /* set request entry */        

        /* request state is set in request_entry*/

        if (is_pmlib_state(clientdata.pcu.pms.state) &&
            clientdata.pcu.pms.state != PMLIB_NONE     ){

            /* lock request entry */
            Mutex_Lock(&request_entry_mutex);

            request_entry =  pmlib2pmstate(clientdata.pcu.pms.state); 

            Debug1("prc_req_stat:request entry = %d\n",request_entry);

            /* make sure that request_entry should have the same value as 
               neither PMD_FLAG_NONE nor PMD_FLAG_BUSY, 
               since request_entry will be
               copied to request_flag only if request_flag is not PMD_FLAG_BUSY,
               ,for PMD_FLAG_NONE and PMD_FLAG_BUSY are used for semaphore */
                        
            /* unlock request entry */
            Mutex_Unlock(&request_entry_mutex);
        }
        else{
            /* invalid argument specified */
            pserverdata->rc = EINVAL;
            pserverdata->length = sizeof (struct pmlib_server);
            return PM_ERROR;
        }
        

        /* check if request is being processed or not */
        if (check_state_change_process_flag() == PM_SUCCESS){

            /* lock request entry */
            Mutex_Lock(&request_entry_mutex);
    

            /* check permission */
            if(check_request_state_permission(request_entry) == PM_SUCCESS){
                
                /* if requested state is different from present state */
                /* set request_flag and wakeup main thread */

		if (request_entry != pmd_parameters.pm_present_state.state){

                /* lock request flag */
                Mutex_Lock(&request_flag_mutex);
        
                
                request_flag = request_entry;
                
                /* unlock request flag */
                Mutex_Unlock(&request_flag_mutex);
        
                /* wake up main thread for state change start */


		wakeup_main_thread(&state_change_predicate,
				   &state_change_condvar,
				   &state_change_cond_mutex);

                }

                pserverdata->rc = PMLIB_SUCCESS;
                pserverdata->length = sizeof(struct pmlib_server);
                rc = PM_SUCCESS;
            }
            else{
                Debug0("chk req stat perm failed\n");

                /* if check_request_state_permission fail, */
                /* error is set into server data inside    */
                /* check_request_state_permission()        */
                /* and return , then proceeds to send_data */

            }

            /* unlock request entry */
            Mutex_Unlock(&request_entry_mutex);
    
            return rc;

        }/* end of if */
        else{
            
            /* if check_state_change_process_flag and 
             * error is set into server data
             * and return , then proceedsto send_data */

            pserverdata->rc = EBUSY;
            pserverdata->length = sizeof(struct pmlib_server);
            return PM_ERROR;

        }/* end of if */

        break;

    case PMLIB_QUERY_STATE:
        Debug0("prc_req_stat:query state\n");

        /* set present state */
        pserverdata->psu.pms.state = pmd_parameters.pm_present_state.state;
        Debug1("query state state=%s\n",
               pmstate2asc(pserverdata->psu.pms.state));

     
        pserverdata->rc = PMLIB_SUCCESS;
        pserverdata->length = sizeof(struct pmlib_server);
        break;

    case PMLIB_QUERY_ERROR:

        Debug0("prc_req_stat:query error \n");

        /* lock pmlib_latest_error */
        Mutex_Lock(&pmlib_latest_error_mutex);


        /* set latest error */
        pserverdata->psu.pms = pmlib_latest_error;

        /* unlock pmlib_latest_error */
        Mutex_Unlock(&pmlib_latest_error_mutex);


        pserverdata->rc = PMLIB_SUCCESS;
        pserverdata->length =  sizeof (struct pmlib_server);
        break;

    case PMLIB_CONFIRM:
        Debug0("prc_req_stat:confirm \n ");

        /* confirmation of state change for 1st trial 
         * count up the number of the client and check pid 
         * and the check the target state
         * if count reached upto the number of registered application 
         * main_thread awakened to proceed 
         */

        switch( clientdata.pcu.pms.state ){
        case PMLIB_SYSTEM_CHANGE_OK :
           
            Debug0("prc_confirm: change_ok \n");

            /* lock application_answer */
            Mutex_Lock(&application_answer_mutex);
    
	    if (appl_answer == PMD_UNANIMOUS_OK){

		/* once appl_answer has been set to PMD_REJECT */
                /* it keeps on being PMD_REJECT till the next  */
                /* state transition.                           */

		appl_answer = PMD_UNANIMOUS_OK;	
		appl_pid    = 0; /* default value */
	    }


            /* unlock application_answer */
            Mutex_Unlock(&application_answer_mutex);
    

            break;
        case PMLIB_SYSTEM_CHANGE_NO :

            Debug0("prc_confirm: change_no \n");

            /* lock application_answer */
            Mutex_Lock(&application_answer_mutex);
    

            appl_answer = PMD_REJECT;
            appl_pid    = clientdata.cli_pid;

            /* unlock application_answer */
            Mutex_Unlock(&application_answer_mutex);
    

            break;
        default:
            Debug0("prc_confirm: invalid arg. specified. \n");

            /* in case the application replies invalid answer */
            pserverdata->rc = PMLIB_ERROR;
            pserverdata->length = sizeof (struct pmlib_server);
            return PM_ERROR;
        }/* end of switch */

        
        /* proceed to count up the requests from the applications */
        
        /* lock count_confirmation_flag */
        Mutex_Lock(&count_confirmation_flag_mutex);
        
        
        if ( count_confirmation_flag ){
            count_request_entry(PMD_INVOKE_CONFIRMATION,
                                clientdata.cli_pid,
                                pclientinfo_head);
        }
            
        /* unlock count_confirmation_flag */
        Mutex_Unlock(&count_confirmation_flag_mutex);
        
        
        pserverdata->rc = PMLIB_SUCCESS;
        pserverdata->length =sizeof (struct pmlib_server);
        
        break;
    default:
        Debug0("prc_req_state:default\n");
        fprintf(stderr,MSGSTR(ERR_INVAL_CTRL,DFLT_ERR_INVAL_CTRL));
        
        pserverdata->rc = PMLIB_ERROR;
        pserverdata->length = sizeof (struct pmlib_server);
        return PM_ERROR;
        
    } /* end of switch */
    
    return PM_SUCCESS;
}

/*
 * NAME:     reset_request_entry_count
 *
 * FUNCTION: reset the variables for counting the number of the queries from 
 *           PM aware applications.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   pclinfo_reset_head
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : Invalid arguments specified.
 */
int
reset_request_entry_count(clientinfo_t *pclinfo_reset_head)
{
    clientinfo_t *pclinfo_entry;

    Debug0("reset_request_entry_count\n");

    /* if there is no application registered, just return and proceed */
    
    Debug1("number of registered clients : %d \n",client_number);

    if (client_number == 0){
        return PM_ERROR;
    }

    pclinfo_entry=pclinfo_reset_head;

    Mutex_Lock(&client_chain_mutex);

    /* reset event_queried application number */
    event_queried_application =0;
    /* reset confirmed application number */
    confirmed_application = 0;

    do{	
	pclinfo_entry->event_query_count = 0;
	pclinfo_entry->confirmation_count = 0;
	pclinfo_entry = pclinfo_entry->next;
    }while ( pclinfo_entry != pclinfo_reset_head );

    Mutex_Unlock(&client_chain_mutex);

    return PM_SUCCESS;
}

/*
 * NAME:     count_request_entry
 *
 * FUNCTION: count the number of the queries from  pm-aware applications.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   ctrl , pid, pclinfo_count_head
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int
count_request_entry(int ctrl,pid_t pid,clientinfo_t *pclinfo_count_head)
{
    
    clientinfo_t *pclinfo_entry;

    Debug0("count_request_entry\n");
    Debug1("number of registered clients : %d \n",client_number);

    if (client_number > 0 ){

        switch(ctrl){
        case PMD_INVOKE_EVENT_QUERY:

            Mutex_Lock(&client_chain_mutex);

            if ( (pclinfo_entry = 
                      search_entry(pclinfo_count_head,pid,PMD_WITHOUT_LOCK)
                  )!=NULL){

                if ( pclinfo_entry->event_query_count++ == 0 ){
                    event_queried_application ++;
                }
                else{
                    Debug1("count_request_entry: duplicate entry (pid %d)\n",
                           pid);
                }
            }
            else{
            Debug1("count_request_entry: application not registered(pid %d)\n",
                    pid);
            }

            Mutex_Unlock(&client_chain_mutex);

            break;
        case PMD_INVOKE_CONFIRMATION:

            Mutex_Lock(&client_chain_mutex);

            if ( (pclinfo_entry = 
                       search_entry(pclinfo_count_head,pid,PMD_WITHOUT_LOCK)
                  )!=NULL){

                if ( pclinfo_entry->confirmation_count++ == 0 ){
                    confirmed_application ++;
                }
                else{
                    Debug1("count_request_entry: duplicate entry (pid %d)\n",
                           pid);
                }
            }
            else{
             Debug1("count_request_entry: application not registered(pid %d)\n",
                    pid);
            }

            Mutex_Unlock(&client_chain_mutex);

            break;
        default:
            return PM_ERROR;
        }/* end of switch */
        
    }/*end of if */


    if ( request_count_reached(ctrl,pclinfo_count_head)){
        Debug0(" all registered application respond to signal \n");
        wakeup_main_thread(&response_predicate,
                           &response_condvar,
                           &response_cond_mutex);
    }

    return PM_SUCCESS;

}

/*
 * NAME:     request_count_reached
 *
 * FUNCTION: check the number of the queries from  pm-aware applications.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   ctrl , pclinfo_check_head
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE:    request count is reached up to the client number.
 *           FALSE:   request count is not reached yet.
 */

int
request_count_reached(int ctrl,clientinfo_t *pclinfo_check_head)
{
    clientinfo_t *pclinfo_entry;
    int check_flag = TRUE;

    Debug0("request_count_reached\n");

    Debug1("number of registered clients : %d \n",client_number);

    /* if the number of client is 0, unnecessary to count */
    if (client_number == 0){
        return TRUE;
    }

    Mutex_Lock(&client_chain_mutex);

    pclinfo_entry=pclinfo_check_head;

    switch(ctrl){
    case PMD_INVOKE_EVENT_QUERY:
      event_queried_application =0;
      do{
          if (pclinfo_entry->event_query_count == 0 ){
              check_flag = FALSE;
          }
          pclinfo_entry = pclinfo_entry->next;
          event_queried_application++;
      }while ( pclinfo_entry != pclinfo_check_head );
      break;
    case PMD_INVOKE_CONFIRMATION:
      confirmed_application = 0;

      /* if any application refuses to proceed, check flag is set TRUE */
      /* (appl_pid has already been  set to error-caused application's pid )*/

      if ( appl_answer != PMD_UNANIMOUS_OK){
          check_flag = TRUE;
      }
      else{
      /* if all the applications accept to proceed, check flag is set TRUE */

          do{
              if (pclinfo_entry->confirmation_count == 0 ){
                  check_flag = FALSE;
              }
              pclinfo_entry = pclinfo_entry->next;
              confirmed_application++;
          }while ( pclinfo_entry != pclinfo_check_head );
      }
      break;
  default:
      check_flag = FALSE;
  }/* end of switch */

    Mutex_Unlock(&client_chain_mutex);

    return check_flag;
}


/*
 * NAME:     check_state_change_process_flag
 *
 * FUNCTION: check if state change request is being processed or not.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   request_flag,event_flag
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: the system can initiate state transition.
 *           PM_ERROR:   the system is busy processing the state change.
 */

    
int check_state_change_process_flag()
{
    int rc;
    int lc; 
    /* local condition variables to ensure the comparison is atomic */

    Debug0("check_state_change_process_flag\n");


    /* lock event flag */
    Mutex_Lock(&event_flag_mutex);
    
    /* lock request flag */
    Mutex_Lock(&request_flag_mutex);
    
    lc = (event_flag==PMD_FLAG_NONE && request_flag==PMD_FLAG_NONE);

    /* unlock request flag */
    Mutex_Unlock(&request_flag_mutex);
    
    /* unlock event flag */
    Mutex_Unlock(&event_flag_mutex);

    if (lc){
        rc = PM_SUCCESS;
    }
    else{
        rc = PM_ERROR;
        Debug0(" ================= Main thread busy . Request Rejected \n");
    }

    return rc;
}


/*
 * NAME:     check_request_state_permission
 *
 * FUNCTION: check the permission for state change. 
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   target
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: the request for the transition is permitted.
 *           PM_ERROR:   the request for the transition is not permitted.
 */
int
check_request_state_permission( int target)
{
    int rc = PM_ERROR;
    int permitted;       /* permitted state (bitwise OR of PM_SYSTEM_XXXX)*/
    int supported;       /* supported state (bitwise OR of PM_SYSTEM_XXXX)*/
    int present;         /* present state (PM_SYSTEM_XXXX)*/
    int superuser=FALSE;

    Debug0("check_request_state_permission\n");

    /* lock pmd_parameters */
    Mutex_Lock(&pmd_parameters_mutex);
    
    present   = pmd_parameters.pm_present_state.state;
    permitted = pmd_parameters.permission;
    supported = pmd_parameters.system_support;

    /* unlock pm_parameters entry */
    Mutex_Unlock(&pmd_parameters_mutex);

    /* superuser has a privilege to request the transition to any state.*/
    if (check_superuser_privilege()==PM_SUCCESS){

        Debug0("superuser privilege\n");

        superuser = TRUE;
        permitted = (PM_SYSTEM_FULL_ON |
                     PM_SYSTEM_ENABLE  |
                     PM_SYSTEM_STANDBY  |
                     PM_SYSTEM_SUSPEND  |
                     PM_SYSTEM_HIBERNATION |
                     PM_SYSTEM_SHUTDOWN );
    }
    else{
        superuser = FALSE;
        permitted &= ~PM_SYSTEM_FULL_ON;

	/* transition from enable to enable is inhibited for */
        /* general users.                                    */
    }




    Debug1("permitted :%s \n",pmstate2asc(permitted));
    Debug1("permitted :%x \n",permitted);
    Debug1("supported :%s \n",pmstate2asc(supported));
    Debug1("supported :%x \n",supported);
    Debug1("present   :%s \n",pmstate2asc(present));
    Debug1("present   :%x \n",present);
    Debug1("target   :%s \n",pmstate2asc(target));
    Debug1("target   :%x \n",target);

    switch(present){
    case PM_SYSTEM_FULL_ON:
        if ( target == PMLIB_SYSTEM_ENABLE && superuser){
	    Debug0("ok\n");                 
            rc = PM_SUCCESS;
        }
        else if ( target == PMLIB_SYSTEM_ENABLE && !superuser){
	    Debug0("not ok (EPERM)\n");
            /* system does not authorize the state transition */
	    pserverdata->rc = EPERM;
	    pserverdata->length = sizeof(struct pmlib_server);
	    rc = PM_ERROR;
        }
	else if ( target == PMLIB_SYSTEM_FULL_ON ){
	    Debug0("not ok (EALREADY)\n");                 
	    /* the same state specified as present state */
	    pserverdata->rc = EALREADY;
	    pserverdata->length = sizeof(struct pmlib_server);
	    rc = PM_ERROR;
	}
	else{
	    Debug0("not ok (EACCES)\n");                 
	    /* system does not permit the transition */
	    pserverdata->rc = EACCES;
	    pserverdata->length = sizeof(struct pmlib_server);
	    rc = PM_ERROR;
	}
        break;
    case PM_SYSTEM_ENABLE:
    case PM_SYSTEM_STANDBY:
	if ( target & permitted ){
 	    if ( target & supported ){
		if ( ( present == PM_SYSTEM_STANDBY &&
		       target  != PM_SYSTEM_FULL_ON ) ||
                     ( present == PM_SYSTEM_ENABLE ) ){
		    if ( target & ~present ){
			Debug0("ok\n");                 
			rc = PM_SUCCESS;
		    }
		    else{
			Debug0("not ok (EALREADY)\n");                 
                        /* the same state specified as present state */
			pserverdata->rc = EALREADY;
			pserverdata->length = sizeof(struct pmlib_server);
			rc = PM_ERROR;
		    }
		}
		else{
		    Debug0("not ok (EACCES)\n");      
		    /* system does not permit the state transition */
		    pserverdata->rc = EACCES;
		    pserverdata->length = sizeof(struct pmlib_server);
		    rc = PM_ERROR;
		}
	    }
	    else{
		Debug0("not ok (ENOSYS)\n");                 
                /* system does not support the state transition */
		pserverdata->rc = ENOSYS;
		pserverdata->length = sizeof(struct pmlib_server);
		rc = PM_ERROR;
	    }
	}
	else{
	    Debug0("not ok (EPERM)\n");                 
            /* system does not authorize the state transition */
	    pserverdata->rc = EPERM;
	    pserverdata->length = sizeof(struct pmlib_server);
	    rc = PM_ERROR;
        }
        break;
    default:
        break;
    }/*end of switch present*/

    return rc;
}

/*
 * NAME:     process_get_event_notice
 *
 * FUNCTION: process the pmlib_get_event_notice API call.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS: processed successfully
 *        PM_ERROR:   processed unsuccessfully
 */

int
process_get_event_notice()
{
    Debug0("process_get_event_notice\n");

    pserverdata->psu.pme.event = pmlib_event_entry ;

    /* count up the number of the client and check pid */

    /* if count reached up to the number of registered application */
    /* main_thread awakened to proceed */

    /* lock count_event_query_flag */
    Mutex_Lock(&count_event_query_flag_mutex);
    
    if ( count_event_query_flag ){
        count_request_entry(PMD_INVOKE_EVENT_QUERY,
                            clientdata.cli_pid,
                            pclientinfo_head);
    }
            
    /* unlock count_event_query_flag */
    Mutex_Unlock(&count_event_query_flag_mutex);
    
    
    
    pserverdata->rc = PMLIB_SUCCESS;
    pserverdata->length =  sizeof(struct pmlib_server);

    return PM_SUCCESS;
}


/*
 * NAME:     process_request_battery
 *
 * FUNCTION: process the pmlib_get_event_notice API call .
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS: processed successfully
 *        PM_ERROR:   processed unsuccessfully
 */
int
process_request_battery()
{
   Debug0("process_request_battery\n");

   switch(clientdata.ctrl){
   case PMLIB_QUERY_BATTERY:
       query_battery();
       break;
   case PMLIB_DISCHARGE_BATTERY:
       discharge_battery();
       break;
   default:
      Debug0("invalid ctrl\n"); 
      pserverdata->rc = PMLIB_ERROR;
       pserverdata->length = sizeof (struct pmlib_server);
       return PM_ERROR;
   }/*end of switch*/

   return PM_SUCCESS;

}

/*
 * NAME:     query_battery
 *
 * FUNCTION: query for battery information.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */

int
query_battery()
{
    static pm_battery_data_t pmb;
    static int initialize_call = TRUE;
    static int rc = PM_ERROR;

    if ( initialize_call ){
	errno=0;
	rc = pm_battery_control(PM_BATTERY_QUERY, 
				(struct pm_battery *)&pmb);
	initialize_call = FALSE;
	return rc;
    }

    if (!syscall_busy){
	RDebug0(128,"syscall_busy FALSE now ask core for battery info\n");
	errno=0;
	rc = pm_battery_control(PM_BATTERY_QUERY, 
				(struct pm_battery *)&pmb);
    }
    else{
	RDebug0(128,"syscall_busy TRUE now use previous battery info\n");
    }	

    RDebug1(128,"pm_battery_control status :%d\n",rc);


    if (rc == PM_SUCCESS ){
	pserverdata->rc = PMLIB_SUCCESS;
	pserverdata->length = sizeof(struct pmlib_server);
	pm2pmlibbattery(&pmb,&(pserverdata->psu.pmb));
    }
    else{
	if (errno == EIO ){
	    pserverdata->rc = errno;
	    pserverdata->length = sizeof(struct pmlib_server);
	    fprintf(stderr,MSGSTR(ERR_BAT_SPRT,DFLT_ERR_BAT_SPRT));
	}
	else{
	    pserverdata->rc = PMLIB_ERROR;
	    pserverdata->length = sizeof(struct pmlib_server);
	    fprintf(stderr,MSGSTR(ERR_BAT_INFO,DFLT_ERR_BAT_INFO));
	}
    }

 
    return rc;
}

/*
 * NAME:     discharge_battery
 *
 * FUNCTION: discharge battery.
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */
int
discharge_battery()
{
    pm_battery_data_t pmb;

    Debug0("discharge battery \n");

    if (!syscall_busy){
	if (pm_battery_control(PM_BATTERY_QUERY, (struct pm_battery *)&pmb) 
	    != PM_SUCCESS ){
	    if (errno == EIO ){
		Debug0("error number is EIO.\n");
		pserverdata->rc = errno;
		pserverdata->length = sizeof(struct pmlib_server);
		fprintf(stderr,MSGSTR(ERR_BAT_SPRT,DFLT_ERR_BAT_SPRT));
		return PM_ERROR;
	    }
	    else{
		pserverdata->rc = PMLIB_ERROR;
		pserverdata->length = sizeof(struct pmlib_server);
		fprintf(stderr,MSGSTR(ERR_BAT_INFO,DFLT_ERR_BAT_INFO));
		return PM_ERROR;
	    }
	}

	if (!(pmb.attribute & PM_BATTERY)){
	    Debug0("pmb.attribute does not have PM_BATTERY bit.\n");
	    pserverdata->rc = PMLIB_ERROR;
	    pserverdata->length = sizeof(struct pmlib_server);
	    fprintf(stderr,MSGSTR(ERR_BAT_SPRT,DFLT_ERR_BAT_SPRT));
	    return PM_ERROR;      
	}

	if ((pmb.attribute & PM_BATTERY_EXIST)&&(pmb.attribute & PM_AC)){

	    Debug0("now start discharging \n"); 
	    if ( pm_battery_control(PM_BATTERY_DISCHARGE,NULL) != PM_SUCCESS ){
		pserverdata->rc = PMLIB_ERROR;
		pserverdata->length = sizeof(struct pmlib_server);
		fprintf(stderr,MSGSTR(ERR_BAT_DISC,DFLT_ERR_BAT_DISC));
		return PM_ERROR;      
	    }
	}
	else{
	    Debug0("pmb.attribute has neither PM_AC bit \
nor PM_BATTERY_EXIST bit.\n"
		   );
	    pserverdata->rc = PMLIB_ERROR;
	    pserverdata->length = sizeof(struct pmlib_server);
            fprintf(stderr,MSGSTR(ERR_BAT_DISC,DFLT_ERR_BAT_DISC));
	    return PM_ERROR;      
	}

	Debug0("discharge succeeded. \n"); 

	pserverdata->rc = PMLIB_SUCCESS;
	pserverdata->length = sizeof(struct pmlib_server);

	return PM_SUCCESS;
    }
    else{
	pserverdata->rc = EBUSY;
	pserverdata->length = sizeof(struct pmlib_server);
	fprintf(stderr,MSGSTR(ERR_BAT_DISC,DFLT_ERR_BAT_DISC));
	return PM_ERROR;      
    }
}



/*
 * NAME:     process_request_parameter
 *
 * FUNCTION: process the pmlib_request_parameter API call.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */

int 
process_request_parameter()
{
   int i;
   int number;
   int requested_number;
   int actual_number;
   int value;
   pm_parameters_t param_candidate;
   pm_device_info_t device_info;      
   pm_device_names_t device_names;
   

   memset(&param_candidate,0,sizeof( pm_parameters_t));
   memset(&device_info,0,sizeof( pm_device_info_t));
   memset(&device_names,0,sizeof( pm_device_names_t));


   Debug0("process_request_parameter\n");

   /* check if request has  super-user privilege */


   switch(clientdata.ctrl){

       /* Query Cases */

   case PMLIB_QUERY_SYSTEM_IDLE_ACTION:
       Debug0("query_system_idle_action\n");

       pserverdata->psu.pmp.value =
           pm2pmlibstate(pmd_parameters.system_idle_action);

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_LID_CLOSE_ACTION:
       Debug0("query_lid_close_action\n");

       pserverdata->psu.pmp.value = 
           pm2pmlibstate(pmd_parameters.lid_close_action);

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */

   case PMLIB_QUERY_MAIN_SWITCH_ACTION:
       Debug0("query_main_switch_action\n");

       pserverdata->psu.pmp.value = 
           pm2pmlibstate(pmd_parameters.main_switch_action);

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_LOW_BATTERY_ACTION:
       Debug0("query_low_battery_action\n");

       pserverdata->psu.pmp.value =
           pm2pmlibstate(pmd_parameters.low_battery_action);

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_SPECIFIED_TIME_ACTION:
       Debug0("query_specified_time_action\n");

       pserverdata->psu.pmp.value = 
           pm2pmlibstate(pmd_parameters.specified_time_action);

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_KILL_LFT_SESSION:
       Debug0("query_kill_lft_session\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.kill_lft_session;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_KILL_TTY_SESSION:
       Debug0("query_kill_tty_session\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.kill_tty_session;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_PASSWD_ON_RESUME:
       Debug0("query_passwd_on_resume\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.resume_passwd;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_PERMISSION:
       Debug0("query_permission\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.permission;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_SYSTEM_IDLE_TIME:
       Debug0("query_system_idle_time\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.system_idle_time;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_BEEP:
       Debug0("query_beep\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.pm_beep;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_RINGING_RESUME:
       Debug0("query_ringing_resume\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.ringing_resume;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_RESUME_TIME:
       Debug0("query_resume_time\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.resume_time;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_DURATION_TO_HIBERNATION:
       Debug0("query_duration_to_hibernation\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.sus_to_hiber;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_SUPPORTED_STATES:
       Debug0("query_supported_states\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.system_support;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_SPECIFIED_TIME:
       Debug0("query_specified_time\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.specified_time;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_KILL_SYNCD:  
       Debug0("query_kill_syncd\n");

       pserverdata->psu.pmp.value = 
           pmd_parameters.kill_syncd;

       pserverdata->rc = PMLIB_SUCCESS;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_SUCCESS;

       break;
       /* always success */
   case PMLIB_QUERY_DEVICE_INFO:
       Debug0("query_device_info\n");

       if (!syscall_busy){
	   pmlib2pmdevinfo(&(clientdata.pcu.pmdi),
			   &device_info);


	   if (pm_control_parameter(PM_CTRL_QUERY_DEVICE_INFO,
				    (caddr_t)&device_info)
	       == PM_SUCCESS){
   
	       pm2pmlibdevinfo(&device_info,
			       &(pserverdata->psu.pmdi)
			       );
	       
	       pserverdata->rc = PMLIB_SUCCESS;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_SUCCESS;
	   }
	   else{
	       pserverdata->rc = errno;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_QUERY_DEVICE_NUMBER:
       Debug0("query_device_number\n");
       if (!syscall_busy){
	   if (pm_control_parameter(PM_CTRL_QUERY_DEVICE_NUMBER,
				    (caddr_t)&value)
	       == PM_SUCCESS){
	       pserverdata->psu.pmp.value = value;
	       pserverdata->rc = PM_SUCCESS;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_SUCCESS;
	   }
	   else{
	       pserverdata->rc = errno;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_QUERY_DEVICE_NAMES:
       Debug0("query_device_names\n");

       /* first necessary to ask for number of device drivers */

       if (!syscall_busy){
	   requested_number = clientdata.pcu.pmdn.number;
	   Debug1("requested number %d\n",requested_number);
	   Debug1("device name length %d\n",PM_DEVICE_NAME_LENGTH);

	   if ((device_names.names= (char *)
		malloc(requested_number*PM_DEVICE_NAME_LENGTH)) == NULL){
	       fprintf(stderr,MSGSTR(ERR_MEMORY,DFLT_ERR_MEMORY));
	       pserverdata->rc = errno;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }

	   device_names.number = requested_number;

	   if (pm_control_parameter(PM_CTRL_QUERY_DEVICE_LIST,
				    (caddr_t)&device_names)
	       == PM_SUCCESS){
           
	       /* set number */
	       actual_number = device_names.number;
           
	       Debug1("number = %d\n",actual_number);
	       
	       number = ( (actual_number<requested_number) ?
		     actual_number:requested_number);

           /* return data to be sent to client has the size as much as      */
	   /* sizeof(struct pmlib_server) + devnamelength(16)*(DD number-1) */
	   /* (refer to pmlib_internal.h in more details.)                  */

	       workarea_size = sizeof (struct pmlib_server); 
	                                                 /* global variable */
	       workarea_size += PM_DEVICE_NAME_LENGTH * (number-1);

	       if ((pserverdata = (struct pmlib_server *)
		    realloc(pserverdata,workarea_size)) == NULL){
		   pserverdata->rc = errno;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }

	       pserverdata->length = workarea_size;
	       pserverdata->rc     = PMLIB_SUCCESS;
	       pserverdata->psu.pmdn.number = number;
           
           
	       for(i=0;i<number;i++){
          	  strncpy(pserverdata->psu.pmdn.names[i],
			  device_names.names + 
			  PM_DEVICE_NAME_LENGTH * i,
			  PM_DEVICE_NAME_LENGTH);
		  pserverdata->psu.pmdn.names[i][PM_DEVICE_NAME_LENGTH-1]='\0';
	       }

	       if (device_names.names !=NULL){
		   Free(device_names.names);
	       }


	       ddlist_filter(&(pserverdata->psu.pmdn));

	       Debug1("psd->length = %d\n",pserverdata->length);
	       Debug1("psd->rc = %d\n",pserverdata->rc);
	       for(i=0;i<number;i++){
		   Debug2("psd->psu.pmdn.names[%d] = %s\n",
			  i,
			  pserverdata->psu.pmdn.names[i]);
	       }
	       return PM_SUCCESS;
	       
	   }
	   else{
	       pserverdata->rc = errno;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;

       /* End of Query Cases */


       /* Set Cases */

   case PMLIB_SET_SYSTEM_IDLE_ACTION:
       Debug0("set_system_idle_action\n");
       if (!syscall_busy){
	   if (check_superuser_privilege() == PM_SUCCESS){
	       if(is_pmlib_action_state(clientdata.pcu.pmp.value)){
		   if (is_pm_supported(pmlib2pmstate(clientdata.pcu.pmp.value))
		   && ( !( check_pm_attribute(pmd_parameters.firmware_support,
					      inhibit_auto_transition)
		  && (clientdata.pcu.pmp.value == PMLIB_SYSTEM_SUSPEND ||
		      clientdata.pcu.pmp.value == PMLIB_SYSTEM_HIBERNATION))
		       )
		       ){

		       pmd2userparam(&pmd_parameters,&param_candidate);

		       param_candidate.daemon_data.system_idle_action =
			   pmlib2pmstate(clientdata.pcu.pmp.value) ;

		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){

			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
           
			   /* modify event_priority table */
			   set_event_priority(
				      pmd_parameters.pm_present_state.state);

			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }	
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_LID_CLOSE_ACTION:
       Debug0("set_lid_close_action\n");
       if (!syscall_busy){
	   if (check_superuser_privilege() == PM_SUCCESS){
	       if(is_pmlib_action_state(clientdata.pcu.pmp.value)){
		   if (is_pm_supported(pmlib2pmstate(clientdata.pcu.pmp.value))
		       && (check_pm_attribute(pmd_parameters.firmware_support,
					      LID_support)
			   ||clientdata.pcu.pmp.value == PMLIB_NONE )
		       ){
		       pmd2userparam(&pmd_parameters,&param_candidate);

		       param_candidate.daemon_data.lid_close_action =
			   pmlib2pmstate(clientdata.pcu.pmp.value) ;

		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){

			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
           
			   /* modify event_priority table */
			   set_event_priority(
				     pmd_parameters.pm_present_state.state);

			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_MAIN_SWITCH_ACTION:
       Debug0("set_main_switch_action\n");
       Debug1("state = %s\n", pmstate2asc( pmlib2pmstate(
           clientdata.pcu.pmp.value
            )));
       if (!syscall_busy){
	   if (check_superuser_privilege() == PM_SUCCESS){
	       if(is_pmlib_action_state(clientdata.pcu.pmp.value)){
		   if (is_pm_supported(pmlib2pmstate(clientdata.pcu.pmp.value))){
		       pmd2userparam(&pmd_parameters,&param_candidate);

		       param_candidate.daemon_data.main_switch_action =
			   pmlib2pmstate(clientdata.pcu.pmp.value) ;

		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){

			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
           
			   /* modify event_priority table */
			   set_event_priority(
				     pmd_parameters.pm_present_state.state);

			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_LOW_BATTERY_ACTION:
       Debug0("set_low_battery_action\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_action_state(clientdata.pcu.pmp.value)){
		   if (is_pm_supported(pmlib2pmstate(clientdata.pcu.pmp.value))
		       && (check_pm_attribute(pmd_parameters.firmware_support,
					      battery_support)
			   ||clientdata.pcu.pmp.value == PMLIB_NONE )
		       ){
		       pmd2userparam(&pmd_parameters,&param_candidate);

		       param_candidate.daemon_data.low_battery_action =
			   pmlib2pmstate(clientdata.pcu.pmp.value) ;

		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){

			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
           
			   /* modify event_priority table */
			   set_event_priority(
				  pmd_parameters.pm_present_state.state);
			   
			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_SPECIFIED_TIME_ACTION:
       Debug0("set_specified_time_action\n");
       if (!syscall_busy){
	   if (check_superuser_privilege() == PM_SUCCESS){
	       if(is_pmlib_action_state(clientdata.pcu.pmp.value)){
		   if (is_pm_supported(pmlib2pmstate(clientdata.pcu.pmp.value))
		    && ( !( check_pm_attribute(pmd_parameters.firmware_support,
		      		      inhibit_auto_transition)
		    && (clientdata.pcu.pmp.value == PMLIB_SYSTEM_SUSPEND ||
			clientdata.pcu.pmp.value == PMLIB_SYSTEM_HIBERNATION))
		       )
		       ){

		       pmd2userparam(&pmd_parameters,&param_candidate);

		       param_candidate.daemon_data.specified_time_action =
			   pmlib2pmstate(clientdata.pcu.pmp.value) ;

		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){

			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
           
			   /* modify event_priority table */
			   set_event_priority(
				   pmd_parameters.pm_present_state.state);

			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }	
	       }	
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_KILL_LFT_SESSION:
       Debug0("set_kill_lft_session\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);

		   param_candidate.daemon_data.kill_lft_session =
		       pmlibonoff2boolean(clientdata.pcu.pmp.value);

		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){

		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
           
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }

		   pserverdata->rc = PMLIB_SUCCESS;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_SUCCESS;
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_KILL_TTY_SESSION:
       Debug0("set_kill_tty_session\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);

		   param_candidate.daemon_data.kill_tty_session =
		       pmlibonoff2boolean(clientdata.pcu.pmp.value);

		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){
		       
		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
           
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
		   pserverdata->rc = PMLIB_SUCCESS;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_SUCCESS;
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_SUCCESS;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_PASSWD_ON_RESUME:
       Debug0("set_passwd_on_resume\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);

		   param_candidate.daemon_data.resume_passwd =
		       pmlibonoff2boolean(clientdata.pcu.pmp.value);

		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){

		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
           
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }

		   pserverdata->rc = PMLIB_SUCCESS;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_SUCCESS;
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }	
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_PERMISSION:
       Debug0("set_permission\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_permission(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);

		   param_candidate.daemon_data.permission =
		       clientdata.pcu.pmp.value;
		   
		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){
		       
		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
           
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_SUCCESS;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;

   case PMLIB_SET_SYSTEM_IDLE_TIME:
       Debug0("set_system_idle_time\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_timer(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);

		   param_candidate.core_data.system_idle_time =
		       clientdata.pcu.pmp.value;
		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){
                   
		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
               
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }

	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_BEEP:
       Debug0("set_beep\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   pmd2userparam(&pmd_parameters,&param_candidate);
		   
		   param_candidate.core_data.pm_beep =
		       pmlibonoff2boolean(clientdata.pcu.pmp.value);
		   
		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){

		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
               
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_RINGING_RESUME:
       Debug0("set_ringing_resume\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if (is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   if (check_pm_attribute(pmd_parameters.firmware_support,
					  ringing_resume_from_suspend)
		       ||check_pm_attribute(pmd_parameters.firmware_support,
					    ringing_resume_from_hibernation)
		       || clientdata.pcu.pmp.value == PMLIB_OFF
		       ){
		       pmd2userparam(&pmd_parameters,&param_candidate);
		       
		       param_candidate.core_data.ringing_resume =
			   pmlibonoff2boolean(clientdata.pcu.pmp.value);
		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){
			   
			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
		       
			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }	
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_RESUME_TIME:
       Debug0("set_resume_time\n");
       if (!syscall_busy){
	   if (check_superuser_privilege() == PM_SUCCESS ){
	       if (is_specified_time(clientdata.pcu.pmp.value)){
		   if (!check_pm_attribute(pmd_parameters.firmware_support,
					   inhibit_auto_transition)
		       &&( check_pm_attribute(pmd_parameters.firmware_support,
				     resume_from_suspend_at_specified_time)
			  ||check_pm_attribute(pmd_parameters.firmware_support,
				   resume_from_hibernation_at_specified_time))
		       || clientdata.pcu.pmp.value == 0
		       ){
		       pmd2userparam(&pmd_parameters,&param_candidate);
		       
		       param_candidate.core_data.resume_time = 
			   clientdata.pcu.pmp.value;
		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate)
			   == PM_SUCCESS ){
			   
			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
			   
			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_DURATION_TO_HIBERNATION:
       Debug0("set_duration_to_hibernation\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS){
	       if(is_timer(clientdata.pcu.pmp.value)){
		   if (!check_pm_attribute(pmd_parameters.firmware_support,
					   inhibit_auto_transition)
		       && (pmd_parameters.system_support & PM_SYSTEM_SUSPEND)!=0
		  && (pmd_parameters.system_support & PM_SYSTEM_HIBERNATION)!=0
		       ||clientdata.pcu.pmp.value == 0 
		       ){		       
		       
		       pmd2userparam(&pmd_parameters,&param_candidate);
		       
		       param_candidate.core_data.sus_to_hiber = 
			   clientdata.pcu.pmp.value;
		       if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
						(caddr_t)&param_candidate) 
			   == PM_SUCCESS ){
			   
			   Mutex_Lock(&pmd_parameters_mutex);
			   user2pmdparam(&param_candidate,&pmd_parameters);
			   Mutex_Unlock(&pmd_parameters_mutex);
		       
			   pserverdata->rc = PMLIB_SUCCESS;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_SUCCESS;
		       }
		       else{
			   pserverdata->rc = PMLIB_ERROR;
			   pserverdata->length = sizeof(struct pmlib_server);
			   return PM_ERROR;
		       }
		   }
		   else{
		       pserverdata->rc = ENOSYS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_SPECIFIED_TIME:
       Debug0("set_specified_time\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_specified_time(clientdata.pcu.pmp.value)){
		   
		   pmd2userparam(&pmd_parameters,&param_candidate);
		   
		   param_candidate.core_data.specified_time =
		       clientdata.pcu.pmp.value;
		   if (pm_control_parameter(PM_CTRL_SET_PARAMETERS,
					    (caddr_t)&param_candidate)
		       == PM_SUCCESS ){
		       
		       Mutex_Lock(&pmd_parameters_mutex);
		       user2pmdparam(&param_candidate,&pmd_parameters);
		       Mutex_Unlock(&pmd_parameters_mutex);
		       
		       pserverdata->rc = PMLIB_SUCCESS;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_SUCCESS;
		   }
		   else{
		       pserverdata->rc = PMLIB_ERROR;
		       pserverdata->length = sizeof(struct pmlib_server);
		       return PM_ERROR;
		   }
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_KILL_SYNCD:    
       Debug0("set_kill_syncd\n");
       if (!syscall_busy){
	   if ( check_superuser_privilege() == PM_SUCCESS ){
	       if(is_pmlib_onoff(clientdata.pcu.pmp.value)){
		   
		   /* It is not necessary to send the flag setting for
		      terminating syncd , since it it a pm daemon
		      specific data.
		      pm core would know if syncd has been killed
		      during standby, which is sent before standby 
		      enter */
  
		   Mutex_Lock(&pmd_parameters_mutex);
		   pmd_parameters.kill_syncd = 
                   pmlibonoff2boolean(clientdata.pcu.pmp.value);
		   Mutex_Unlock(&pmd_parameters_mutex);

		   pserverdata->rc = PMLIB_SUCCESS;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_SUCCESS;
	       }
	       else{
		   pserverdata->rc = EINVAL;
		   pserverdata->length = sizeof(struct pmlib_server);
		   return PM_ERROR;
	       }
	   }
	   else{
	       pserverdata->rc = EPERM;
	       pserverdata->length = sizeof(struct pmlib_server);
	       return PM_ERROR;
	   }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   case PMLIB_SET_DEVICE_INFO:   
       Debug0("set_device_info\n");
             if (!syscall_busy){
		 if ( check_superuser_privilege() == PM_SUCCESS){
		     
		     pmlib2pmdevinfo(&clientdata.pcu.pmdi,
				     &device_info);

		     if (pm_control_parameter(PM_CTRL_SET_DEVICE_INFO,
					      (caddr_t)&device_info)
			 == PM_SUCCESS ){

			 pserverdata->rc = PMLIB_SUCCESS;
			 pserverdata->length = sizeof(struct pmlib_server);
			 return PM_SUCCESS;
		     }
		     else{
			 pserverdata->rc = errno;
			 pserverdata->length = sizeof(struct pmlib_server);
			 return PM_ERROR;
		     }
		 }
		 else{
		     pserverdata->rc = EPERM;
		     pserverdata->length = sizeof(struct pmlib_server);
		     return PM_ERROR;
		 }
       }
       else{
	   pserverdata->rc = EBUSY;
	   pserverdata->length = sizeof(struct pmlib_server);
	   return PM_ERROR;
       }
       break;
   default:
       pserverdata->rc = PMLIB_ERROR;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_ERROR;
       break;
   }/*end of switch */

   /* NOT REACHED */

   return PM_ERROR;
}


/*
 * NAME:     ddlist_filter
 *
 * FUNCTION: cancel out the duplicate device logical names.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */
int 
ddlist_filter(pmlib_internal_device_names_t *pddlist)
{
    int srch = 0;
    int cpy = 0;
    int idx = 0;
    int match = FALSE;

    if (pddlist->number <= 1){
	/* not necessary to cancel out */
        /* the duplicate logical names.*/
	return PM_ERROR;
    }
    
    Debug2("now handling %d:%s\n",srch,pddlist->names[srch]);

    /* set the pointer */
    srch = cpy = 1;

    /* start cancelling out */
    do{
	Debug2("now handling %d:%s\n",srch,pddlist->names[srch]);
	match = FALSE;
	for(idx=0;idx<cpy;idx++){
	    if (strcmp(pddlist->names[srch],pddlist->names[idx])==0){
		match = TRUE;
		break;
	    }
	}
	
	if (!match){
	    if ( cpy< srch){
		strcpy(pddlist->names[cpy],pddlist->names[srch]);
		memset(pddlist->names[srch],0,16);
	    }
	    cpy++;
	}
	else{
	    memset(pddlist->names[srch],0,16);
	}
	srch++;

    }while(srch < pddlist->number);

    /* substract the duplicate number */
    pddlist->number -= (srch - cpy);

    return PM_SUCCESS;
}



/*
 * NAME:     check_superuser_privilege
 *
 * FUNCTION: check the permission for set of parameters.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */

int 
check_superuser_privilege()
{
   pid_t pid;
   uid_t uid,euid;

   pid = clientdata.cli_pid;
   uid = clientdata.cli_uid;
   euid = clientdata.cli_euid;

/* check privilege */

   if (euid !=0 ){
       Debug0("chk_set_param_perm: not superuser privilege \n");
       return PM_ERROR;
   }
   
   Debug0("chk_set_param_perm: superuser privilege \n");

   return PM_SUCCESS;
}



/*
 * NAME:     process_register_application
 *
 * FUNCTION: process the pmlib_control_application API.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully.
 *           PM_ERROR:   processed unsuccessfully.
 */
int process_register_application()
{
   clientinfo_t clientinfo;
 

   Debug0("process_register_application\n");

   memset(&clientinfo,0,sizeof(clientinfo_t));

   clientinfo.pid = clientdata.cli_pid;
   clientinfo.uid = clientdata.cli_uid;
   clientinfo.euid = clientdata.cli_euid;

   switch( clientdata.ctrl){
   case PMLIB_REGISTER:
       register_application(&clientinfo);
       break;
   case PMLIB_UNREGISTER:
       unregister_application(&clientinfo,PMD_WITH_LOCK);
       break;
   default:
       pserverdata->rc = PMLIB_ERROR;
       pserverdata->length = sizeof(struct pmlib_server);
       return PM_ERROR;
   }

#ifdef PM_DEBUG
       show_entry(pclientinfo_head);
#endif


   return PM_SUCCESS;        
}


/*
 * NAME:     register application
 *
 * FUNCTION: register the entry of specified application
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   clientdata,pclientinfo
 *  OUTPUT:  pclientinfo_head
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */
int 
register_application(clientinfo_t *pclientinfo )
{
    clientinfo_t *pclinfo_entry;

    Debug0("register application \n");

    if (client_number >0){
        if( search_entry(pclientinfo_head,pclientinfo->pid,
                         PMD_WITH_LOCK)!=NULL ){

            /* in case of arleady registered application */
            pserverdata->rc = PMLIB_ERROR;
            pserverdata->length = sizeof(struct pmlib_server);
            return PM_ERROR;
        }
    }      

    /* registeration of application */

    if ((pclinfo_entry= (clientinfo_t *)malloc(sizeof(clientinfo_t)))==NULL) {

        /* memory allocation error */
        pserverdata->rc = errno ;
        pserverdata->length = sizeof(struct pmlib_server);
        return PM_ERROR;

    }

    /* add the entry to the chain of applications as pclientinfo_head */

    Mutex_Lock(&client_chain_mutex);

    if (client_number == 0){
        pclinfo_entry->prev = pclinfo_entry;
        pclinfo_entry->next = pclinfo_entry;

        pclientinfo_head =  pclinfo_entry;
    }
    else{
        pclinfo_entry->prev = pclientinfo_head->prev;
        pclinfo_entry->next = pclientinfo_head;
        pclientinfo_head->prev->next = pclinfo_entry;
        pclientinfo_head->prev = pclinfo_entry;

        pclientinfo_head = pclinfo_entry;
    }

 /* internal data structure <- pclientinfo (socket data) */

    pclientinfo_head->pid = pclientinfo->pid;
    pclientinfo_head->uid = pclientinfo->uid;
    pclientinfo_head->euid = pclientinfo->euid;
    pclientinfo_head->event_query_count  = pclientinfo->event_query_count;
    pclientinfo_head->confirmation_count = pclientinfo->confirmation_count;

    Mutex_Unlock(&client_chain_mutex);


    /* lock client_number mutex */
    Mutex_Lock(&client_number_mutex);
    

    client_number++;

    /* unlock client_number mutex */
    Mutex_Unlock(&client_number_mutex);
    


    pserverdata->rc = PMLIB_SUCCESS;
    pserverdata->length = sizeof(struct pmlib_server);
    return PM_SUCCESS;

}

#ifdef PM_DEBUG
/*
 * NAME:     show_entry
 *
 * FUNCTION: shows the entries of all the applications registered.
 *
 * NOTES:    this routine is for debugging purpose.
 * 
 * DATA STRUCTURES:
 *  INPUT:   pclinfo_show_head
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: successfully processed.
 *           PM_ERROR:   unsuccessfully processed.
 */
int
show_entry(clientinfo_t *pclinfo_show_head)
{
    
    int number=0;
    clientinfo_t *pclinfo_entry;


    Debug0("show entry\n");
    Debug1("number of registered clients : %d \n",client_number);

    if (client_number == 0 ){
        return PM_ERROR;
    }

    Mutex_Lock(&client_chain_mutex);


    pclinfo_entry=pclinfo_show_head;

    do{
        Debug0("\n");
        Debug1("client number :%d \n",number);
        Debug1("client pid    :%d \n",pclinfo_entry->pid);
        Debug1("client uid    :%d \n",pclinfo_entry->uid );
        Debug1("client euid   :%d \n",pclinfo_entry->euid);
        Debug1("event_query_count   :%d \n",pclinfo_entry->event_query_count);
        Debug1("confirmation_count  :%d \n",pclinfo_entry->confirmation_count);
        
        pclinfo_entry = pclinfo_entry->next;
        number++;

    }while ( pclinfo_entry != pclinfo_show_head );

    Mutex_Unlock(&client_chain_mutex);
        
        return PM_SUCCESS;
}

#endif /* PM_DEBUG */

/*
 * NAME:     search_entry
 *
 * FUNCTION: searches for the specified entry in the list of the applications.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   pclinfo_search_head,pid,lock
 *  OUTPUT:  pclientinfo_head
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully.
 *           PM_ERROR:   processed unsuccessfully.
 */

clientinfo_t *
search_entry(clientinfo_t * pclinfo_search_head,pid_t pid,int lock)
{
    clientinfo_t *pclinfo_entry,*rc;


    rc = NULL; /* if not found, return NULL */

    Debug0("search_entry \n");

    if (lock){
        Debug0("search_entry with lock : lock\n");
        Mutex_Lock(&client_chain_mutex);
    }

    pclinfo_entry=pclinfo_search_head;

    do{
        Debug0("\n");
        Debug1("client pid    :%d \n",pclinfo_entry->pid);
        Debug1("client uid    :%d \n",pclinfo_entry->uid );
        Debug1("client euid   :%d \n",pclinfo_entry->euid);

        if ( pclinfo_entry->pid == pid ){

            rc = pclinfo_entry;
            Debug0("found\n");

            break;
        }
        pclinfo_entry = pclinfo_entry->next ; 

    }while( pclinfo_entry != pclinfo_search_head );


    if (lock){
        Debug0("search entry with lock : unlock\n");
        Mutex_Unlock(&client_chain_mutex);
    }

    
    return rc;
}

/*
 * NAME:     broadcast_signal
 *
 * FUNCTION: broadcasts signal to all the applications registered.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   pclinfo_broadcast_head,pid
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully.
 *           PM_ERROR:   processed unsuccessfully.
 */

int
broadcast_signal(clientinfo_t * pclinfo_broadcast_head)
{
    clientinfo_t *pclinfo_entry=NULL;
    int unregister = FALSE;
    clientinfo_t *to_be_unregistered=NULL;

    Debug0("broadcast signal \n");

    Debug1("client_number :%d \n",client_number);

    if (client_number == 0) {
        /* not necessary to broadcast signal. */
        return PM_ERROR;
    }

    /* process to send signal to all the applications registered */

    Mutex_Lock(&client_chain_mutex);

    pclinfo_entry=pclinfo_broadcast_head;

    do{
        Debug1("broadcast signal to client (pid    :%d)\n",pclinfo_entry->pid);

        /* initialize */
        unregister = FALSE;
        to_be_unregistered = NULL;

        if (pclinfo_entry->pid != 0 && pclinfo_entry->pid != 1){

            /* check kill status */
            if( kill(pclinfo_entry->pid,SIGPM) == -1) {

                fprintf(stderr,
                        MSGSTR(ERR_BROADCAST,DFLT_ERR_BROADCAST),
                        pclinfo_entry->pid);

                /* the process in question has already been terminated */
                if (errno == ESRCH ){
                    unregister = TRUE;
                    to_be_unregistered = pclinfo_entry;
                }
            }
        }

        /* go ahead to next */
        pclinfo_entry = pclinfo_entry->next ; 

        if (unregister && to_be_unregistered != NULL){

            /* unregister the application which would not
               respond to the signal */
        
            /* display messages */
            fprintf(stderr,
                    MSGSTR(WARN_UNREGISTER,DFLT_WARN_UNREGISTER),
                    to_be_unregistered->pid);

            /* if to_be_unregistered is pclinfo_broadcast_head, */
            /* pclinfo_entry will never be the same as          */
            /* pclinfo_broadcast_head, since the the entry of   */
            /* pclinfo_broadcast_head will be discarded by the  */
            /* call to unregister_application().                */
            /* so pclinfo_broadcast_head should be next entry   */
            /* of the chain of clients.                         */
 
            if ( to_be_unregistered == pclinfo_broadcast_head ){
                pclinfo_broadcast_head = pclinfo_entry;
                /*                     = to_be_unregistered->next */
                /*                     = pclinfo_broadcast_head->next */

            }

            /* unregister the application */        
            unregister_application(to_be_unregistered,PMD_WITHOUT_LOCK);
        }

    }while( pclinfo_entry != pclinfo_broadcast_head );
    /* chain is circulated */

    Mutex_Unlock(&client_chain_mutex);

    return PM_SUCCESS;
}



/*
 * NAME:     unregister_application
 *
 * FUNCTION: unregisters the entry of the specified application.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   pclientinfo,pid
 *  OUTPUT:  pclientinfo_head
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */

int
unregister_application( clientinfo_t *pclientinfo, int lock)
{
    int rc;
    clientinfo_t *pclinfo_entry;
    clientinfo_t *prevclinfo;
    clientinfo_t *pclinfo_search_head;

    Debug0("unregister application \n");

    if (client_number == 0){
        /* no client registered */
        pserverdata->rc = PMLIB_ERROR;
        pserverdata->length = sizeof(struct pmlib_server);
        return PM_ERROR;
    }

    if (lock == PMD_WITH_LOCK ){
        Mutex_Lock(&client_chain_mutex);
    }

    pclinfo_entry = 
        search_entry(pclientinfo_head , pclientinfo->pid , PMD_WITHOUT_LOCK);

    if (pclinfo_entry !=NULL) {

        /* unregister specified entry out of the client chain */
        
        pclinfo_entry->prev->next = pclinfo_entry->next; 
        pclinfo_entry->next->prev = pclinfo_entry->prev;
        
        /* head entry is next to the one unregistered */
        pclientinfo_head = pclinfo_entry->next;          
        
        /* deallocate memory for that entry */
        Free(pclinfo_entry);

        /* lock client_number mutex */
        Mutex_Lock(&client_number_mutex);


        client_number--;
        Debug1("number of registered clients : %d \n",client_number);
              
        /* unlock client_number mutex */
        Mutex_Unlock(&client_number_mutex);

        pserverdata->rc = PMLIB_SUCCESS;
        pserverdata->length = sizeof(struct pmlib_server);
        rc = PM_SUCCESS;

    }/* end of if */
    else{
        /* in case of failing to search specified entry */
        pserverdata->rc = PMLIB_ERROR;
        pserverdata->length = sizeof(struct pmlib_server);
        rc = PM_ERROR;

    }/* end of else */

    if (lock == PMD_WITH_LOCK ){
        Mutex_Unlock(&client_chain_mutex);
    }

    return rc;
}

/*
 * NAME:     send_data
 *
 * FUNCTION: send the server data to the pm library through IPC.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */

int
 send_data()
{
    int n;

    Debug0("send_data\n");

    n = pserverdata->length;

    Debug1("server data length = %d \n",n);

    if (sendto(sockfd,
               pserverdata,
               n,
               0,
               (struct sockaddr *)&cli_addr,
               clilen)!=n){
        
        Debug0("socket_interface:sendto error\n");
        fprintf(stderr,MSGSTR(ERR_SEND,DFLT_ERR_SEND));
    }
    
    /* liberate pserverdata */
    if (pserverdata != NULL){
        Debug0("now free pserverdata\n");
        Free(pserverdata);
    }
    return PM_SUCCESS;
}

/*
 * NAME:     wakeup_main_thread
 *
 * FUNCTION: wakes up the main thread.
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *        INPUT:   ppredicate,
 *                 pcond_var,
 *                 pcond_mutex
 *        OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : invalid arguments specified.
 */
int 
wakeup_main_thread(int *ppredicate,
                   pthread_cond_t *pcond_var,
                   pthread_mutex_t *pcond_mutex )
{
        Debug0("wakeup_main_thread: called \n");

        /* lock condition mutex */
        Mutex_Lock(pcond_mutex);

        /* set predicate TRUE */
        *ppredicate = TRUE;
        
        if ( pthread_cond_broadcast(pcond_var) == -1 ){
            fprintf(stderr,MSGSTR(ERR_COND_BRDCST,DFLT_ERR_COND_BRDCST));
        }
         
        /* unlock condition mutex */
        Mutex_Unlock(pcond_mutex);
              
        return PM_SUCCESS;
}

/*
 * NAME:      pm2pmlibstate
 *
 * FUNCTION:  convert system state macros for PM into those for PMLIB
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *
 *        PMLIB_SYSTEM_XXXXX   :successfully processed. 
 *        PMLIB_NONE           :invalid arguments specified.
 */
int 
pm2pmlibstate(int pm_state )
{
    int pmlib_state;

    pmlib_state = PMLIB_NONE;

    if (pm_state & PM_SYSTEM_FULL_ON){
        pmlib_state |= PMLIB_SYSTEM_FULL_ON;
    }
    if (pm_state & PM_SYSTEM_ENABLE){
        pmlib_state |= PMLIB_SYSTEM_ENABLE;
    }
    if (pm_state & PM_SYSTEM_STANDBY){
        pmlib_state |= PMLIB_SYSTEM_STANDBY;
    }
    if (pm_state & PM_SYSTEM_SUSPEND){
        pmlib_state |= PMLIB_SYSTEM_SUSPEND;
    }
    if (pm_state & PM_SYSTEM_HIBERNATION){
        pmlib_state |= PMLIB_SYSTEM_HIBERNATION;
    }
    if (pm_state & PM_SYSTEM_SHUTDOWN){
        pmlib_state |= PMLIB_SYSTEM_SHUTDOWN;
    }

    return pmlib_state;

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
 *        PM_NONE           :invalid arguments specified.
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
 * NAME: is_pm_state
 *
 * FUNCTION: check if the argument represents pm system state or not.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pm system state
 *        FALSE : argument is not pm system state
 */

int
is_pm_state(int pm_state)
{
    return((pm_state == PM_SYSTEM_FULL_ON) ||
           (pm_state == PM_SYSTEM_ENABLE)  ||
           (pm_state == PM_SYSTEM_STANDBY) ||
           (pm_state == PM_SYSTEM_SUSPEND) ||
           (pm_state == PM_SYSTEM_HIBERNATION) ||
           (pm_state == PM_SYSTEM_SHUTDOWN) ||
           (pm_state == PM_SYSTEM_TERMINATE) ||
           (pm_state == PM_NONE) );

}

/*
 * NAME:     is_pm_action_state
 *
 * FUNCTION: check if the argument represents pm system action state or not.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pm system action state
 *        FALSE : argument is not pm system action state
 */

int
is_pm_action_state(int pm_state)
{
    return(pm_state == PM_SYSTEM_STANDBY ||
           pm_state == PM_SYSTEM_SUSPEND ||
           pm_state == PM_SYSTEM_HIBERNATION ||
           pm_state == PM_SYSTEM_SHUTDOWN ||
           pm_state == PM_NONE) ;
}

/*
 * NAME:     is_pm_permission
 *
 * FUNCTION: check if the argument represents pm system state permission
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pm system state permission
 *        FALSE : argument is not pm system state permission
 */

int
is_pm_permission(int pm_permission)
{
    return (
             (pm_permission ==  PM_NONE ) ||

             (pm_permission == (PM_SYSTEM_FULL_ON |
                                PM_SYSTEM_ENABLE  |
                                PM_SYSTEM_STANDBY )) ||

             (pm_permission == (PM_SYSTEM_FULL_ON |
                                PM_SYSTEM_ENABLE  |
                                PM_SYSTEM_STANDBY |
                                PM_SYSTEM_SUSPEND )) ||

             (pm_permission == (PM_SYSTEM_FULL_ON |
                                PM_SYSTEM_ENABLE  |
                                PM_SYSTEM_STANDBY |
                                PM_SYSTEM_SUSPEND |
                                PM_SYSTEM_HIBERNATION )) ||

             (pm_permission == (PM_SYSTEM_FULL_ON |
                                PM_SYSTEM_ENABLE  |
                                PM_SYSTEM_STANDBY |
                                PM_SYSTEM_SUSPEND |
                                PM_SYSTEM_HIBERNATION |
                                PM_SYSTEM_SHUTDOWN) )
        );
            
}

/*
 * NAME:     is_pm_system_support
 *
 * FUNCTION: check if the argument represents pm system state supported.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is pm system state supported.
 *           FALSE : argument is not pm system state supported.
 */

int
is_pm_system_support(int pm_system_support)
{

#define PM_SYSTEM_SUPPORT_STATES (PM_SYSTEM_FULL_ON |      \
                                  PM_SYSTEM_ENABLE  |      \
                                  PM_SYSTEM_STANDBY |      \
                                  PM_SYSTEM_SUSPEND |      \
                                  PM_SYSTEM_HIBERNATION |  \
                                  PM_SYSTEM_SHUTDOWN )     

    return ((pm_system_support ==
             (PM_SYSTEM_SUPPORT_STATES & ~PM_SYSTEM_SUSPEND)
            )||
            (pm_system_support ==
             (PM_SYSTEM_SUPPORT_STATES & ~PM_SYSTEM_HIBERNATION)
            )
           );
}


/*
 * NAME:     is_pmlib_state
 *
 * FUNCTION: check if the argument represents pm system state or not.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pm system state
 *        FALSE : argument is not pm system state
 */

int
is_pmlib_state(int pmlib_state)
{
    return((pmlib_state == PMLIB_SYSTEM_FULL_ON) ||
           (pmlib_state == PMLIB_SYSTEM_ENABLE ) ||
           (pmlib_state == PMLIB_SYSTEM_STANDBY) ||
           (pmlib_state == PMLIB_SYSTEM_SUSPEND) ||
           (pmlib_state == PMLIB_SYSTEM_HIBERNATION) ||
           (pmlib_state == PMLIB_SYSTEM_SHUTDOWN) ||
           (pmlib_state == PMLIB_NONE) );

    /* terminate is omitted on purpose */
}
/*
 * NAME:     is_pmlib_action_state
 *
 * FUNCTION: check if the argument represents pmlib system action state or not.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pmlib system action state
 *        FALSE : argument is not pmlib system actionstate
 */

int
is_pmlib_action_state(int pmlib_state)
{
    return(pmlib_state == PMLIB_SYSTEM_STANDBY ||
           pmlib_state == PMLIB_SYSTEM_SUSPEND ||
           pmlib_state == PMLIB_SYSTEM_HIBERNATION ||
           pmlib_state == PMLIB_SYSTEM_SHUTDOWN ||
           pmlib_state == PMLIB_NONE) ;
}

/*
 * NAME:     is_pmlib_permission
 *
 * FUNCTION: check if the argument represents pmlib system state permission
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *        TRUE  : argument is pm system state permission
 *        FALSE : argument is not pm system state permission
 */

int
is_pmlib_permission(int pmlib_permission)
{
    return (
             (pmlib_permission == PMLIB_NONE ) ||

             (pmlib_permission == (PMLIB_SYSTEM_FULL_ON |
                                   PMLIB_SYSTEM_ENABLE  |
                                   PMLIB_SYSTEM_STANDBY )) ||

             (pmlib_permission == (PMLIB_SYSTEM_FULL_ON |
                                   PMLIB_SYSTEM_ENABLE  |
                                   PMLIB_SYSTEM_STANDBY |
                                   PMLIB_SYSTEM_SUSPEND )) ||

             (pmlib_permission == (PMLIB_SYSTEM_FULL_ON |
                                   PMLIB_SYSTEM_ENABLE  |
                                   PMLIB_SYSTEM_STANDBY |
                                   PMLIB_SYSTEM_SUSPEND |
                                   PMLIB_SYSTEM_HIBERNATION )) ||

             (pmlib_permission == (PMLIB_SYSTEM_FULL_ON |
                                   PMLIB_SYSTEM_ENABLE  |
                                   PMLIB_SYSTEM_STANDBY |
                                   PMLIB_SYSTEM_SUSPEND |
                                   PMLIB_SYSTEM_HIBERNATION |
                                   PMLIB_SYSTEM_SHUTDOWN) )
        );
            
}

/*
 * NAME:     is_pmlib_system_support
 *
 * FUNCTION: check if the argument represents pmlib system state supported.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is pmlib system state supported.
 *           FALSE : argument is not pmlib system state supported.
 */

int
is_pmlib_system_support(int pmlib_system_support)
{

#define PMLIB_SYSTEM_SUPPORT_STATES (PMLIB_SYSTEM_FULL_ON |      \
                                     PMLIB_SYSTEM_ENABLE  |      \
                                     PMLIB_SYSTEM_STANDBY |      \
                                     PMLIB_SYSTEM_SUSPEND |      \
                                     PMLIB_SYSTEM_HIBERNATION |  \
                                     PMLIB_SYSTEM_SHUTDOWN )     \

    return ((pmlib_system_support ==
             (PMLIB_SYSTEM_SUPPORT_STATES & ~PMLIB_SYSTEM_SUSPEND)
            )||
            (pmlib_system_support ==
             (PMLIB_SYSTEM_SUPPORT_STATES & ~PMLIB_SYSTEM_HIBERNATION)
            )
           );
}
/*
 * NAME:     is_pmlib_onoff
 *
 * FUNCTION: check if the argument is PMLIB_ON / PMLIB_OFF
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is either PMLIB_ON or OFF
 *           FALSE : otherwise
 */

int
is_pmlib_onoff(int pmlib_onoff)
{
    return( pmlib_onoff==PMLIB_ON || pmlib_onoff==PMLIB_OFF );
}

/*
 * NAME:     is_boolean
 *
 * FUNCTION: check if the argument is either TRUE or FALSE
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is either TRUE or FALSE
 *           FALSE : argument is neither TRUE nor FALSE
 */

int
is_boolean(int arg)
{

    return ( (arg == TRUE) || (arg == FALSE  ));
            
}

/*
 * NAME:     is_timer
 *
 * FUNCTION: check if the argument represents the value of the timer
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is appropriate for the value of the timer.
 *           FALSE : argument is not appropriate for the value of the timer.
 */

int
is_timer(int time)
{
    return (time == -1 ||
            time == 0  ||
            (60 <= time && time <= 7200)
            );
}

/*
 * NAME:     is_specified_time
 *
 * FUNCTION: check if the argument represents the value of the timer
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : argument is appropriate for the value of the timer.
 */
int
is_specified_time(int time)
{
    return (time>=0);
}

/*
 * NAME:     is_pm_system_support
 *
 * FUNCTION: check if the specified pm_state is supported by system.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : pm_state is supported.
 *           FALSE : pm_state is not supported.
 */

int
is_pm_supported(int pm_state)
{
    return ( (pm_state == PM_NONE )|| 
	     (pm_state & pmd_parameters.system_support)!=0 );
}


/*
 * NAME:     pmd2userparam
 *
 * FUNCTION: converts pmd_parameter structure into pm_parameters_t structure.
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 *        
 */
int pmd2userparam(pmd_parameters_t *p_pmd_param,pm_parameters_t *p_pm_param)
{

   /* Following member of pmd_parameters_t will NOT be sent to pm core  
     p_pmd_param->pm_present_state,
     p_pmd_param->pm_target_state,
     p_pmd_param->kill_syncd,
     p_pmd_param->system_support
   */

   p_pm_param->daemon_data.system_idle_action =
       p_pmd_param->system_idle_action;     

   p_pm_param->daemon_data.lid_close_action =
       p_pmd_param->lid_close_action;       

   p_pm_param->daemon_data.main_switch_action = 
       p_pmd_param->main_switch_action;     

   p_pm_param->daemon_data.low_battery_action = 
       p_pmd_param->low_battery_action;     

   p_pm_param->daemon_data.specified_time_action =
       p_pmd_param->specified_time_action;  

   p_pm_param->daemon_data.resume_passwd = 
       p_pmd_param->resume_passwd;          

   p_pm_param->daemon_data.kill_lft_session = 
       p_pmd_param->kill_lft_session;       

   p_pm_param->daemon_data.kill_tty_session =
       p_pmd_param->kill_tty_session;       

   p_pm_param->daemon_data.permission =
       p_pmd_param->permission;             

   p_pm_param->core_data.system_idle_time =  
       p_pmd_param->system_idle_time;         

   p_pm_param->core_data.pm_beep =
       p_pmd_param->pm_beep;                  

   p_pm_param->core_data.ringing_resume =
       p_pmd_param->ringing_resume;           

   p_pm_param->core_data.resume_time =           
       p_pmd_param->resume_time;           

   p_pm_param->core_data.sus_to_hiber =
       p_pmd_param->sus_to_hiber;             

   p_pm_param->core_data.specified_time = 
       p_pmd_param->specified_time;        

   /* for debugging */
   memcpy(&(p_pm_param->core_data.reserve),
          &(p_pmd_param->reserved2),
          sizeof(int));

   return PM_SUCCESS;

}


/*
 * NAME: user2pmdparam
 *
 * FUNCTION: convert pm_parameters_t structure into pmd_parameters_t structure.
 *
 * NOTES:   pmd_parameters_t.kill_syncd -> (flag to kill syncd)
 *          pm_parameters_t.kill_syncd -> (flag if syncd killed during standby)
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 */
int
user2pmdparam(pm_parameters_t *p_pm_param,pmd_parameters_t *p_pmd_param)
{
   /* Following member of pmd_parameters_t will NOT be sent from pm core  
     p_pmd_param->pm_present_state,
     p_pmd_param->pm_target_state,
     p_pmd_param->kill_syncd,
     p_pmd_param->system_support
   */

   p_pmd_param->system_idle_action =      
   p_pm_param->daemon_data.system_idle_action;     

   p_pmd_param->lid_close_action =        
   p_pm_param->daemon_data.lid_close_action;       

   p_pmd_param->main_switch_action =      
   p_pm_param->daemon_data.main_switch_action;     

   p_pmd_param->low_battery_action =      
   p_pm_param->daemon_data.low_battery_action;     

   p_pmd_param->specified_time_action =   
   p_pm_param->daemon_data.specified_time_action;  

   p_pmd_param->resume_passwd =           
   p_pm_param->daemon_data.resume_passwd;          

   p_pmd_param->kill_lft_session =        
   p_pm_param->daemon_data.kill_lft_session;       

   p_pmd_param->kill_tty_session =        
   p_pm_param->daemon_data.kill_tty_session;       

   p_pmd_param->permission =              
   p_pm_param->daemon_data.permission;             

   p_pmd_param->system_idle_time =          
   p_pm_param->core_data.system_idle_time;         

   p_pmd_param->pm_beep =                   
   p_pm_param->core_data.pm_beep;                  

   p_pmd_param->ringing_resume =            
   p_pm_param->core_data.ringing_resume;           

   p_pmd_param->resume_time =            
   p_pm_param->core_data.resume_time;           

   p_pmd_param->sus_to_hiber =              
   p_pm_param->core_data.sus_to_hiber;             

   p_pmd_param->specified_time =         
   p_pm_param->core_data.specified_time;        

   /* for debugging */
   p_pmd_param->reserved2 =
   *(int *)(p_pm_param->core_data.reserve) ;

   return PM_SUCCESS;

}

/*
 * NAME:     pmlibonoff2boolean
 *
 * FUNCTION: convert PMLIB_ON/PMLIB_OFF into TRUE/FALSE.
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           TRUE   : the specified argument is PMLIB_ON
 *           FALSE  : the specified argument is PMLIB_OFF
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

/*
 * NAME:      pm2pmlibdevinfo
 *
 * FUNCTION:  convert device information for PM into that for PM library
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 *
 */
int
pm2pmlibdevinfo(pm_device_info_t *p_pm_devinfo,
                pmlib_device_info_t *p_pmlib_devinfo)
{
    Debug0("pm2pmlibdevinfo\n");
    memset(p_pmlib_devinfo,0,sizeof(pmlib_device_info_t));

    strncpy(p_pmlib_devinfo->name,p_pm_devinfo->name,PM_DEVICE_NAME_LENGTH);

    Debug1("pmdevinfo.name = %s \n",p_pm_devinfo->name);

    p_pmlib_devinfo->mode      = p_pm_devinfo->mode;
    p_pmlib_devinfo->idle_time = p_pm_devinfo->idle_time;
    p_pmlib_devinfo->standby_time = p_pm_devinfo->standby_time;
    p_pmlib_devinfo->idle_time1 = p_pm_devinfo->idle_time1;
    p_pmlib_devinfo->idle_time2 = p_pm_devinfo->idle_time2;

    /* now convert attribute */

    if (p_pm_devinfo->attribute & PM_GRAPHICAL_INPUT){
        p_pmlib_devinfo->attribute |= PMLIB_GRAPHICAL_INPUT;
    }
    if (p_pm_devinfo->attribute & PM_GRAPHICAL_OUTPUT){
        p_pmlib_devinfo->attribute |= PMLIB_GRAPHICAL_OUTPUT;
    }
    if (p_pm_devinfo->attribute & PM_AUDIO_INPUT){
        p_pmlib_devinfo->attribute |= PMLIB_AUDIO_INPUT;
    }
    if (p_pm_devinfo->attribute & PM_AUDIO_OUTPUT){
        p_pmlib_devinfo->attribute |= PMLIB_AUDIO_OUTPUT;
    }
    if (p_pm_devinfo->attribute & PM_RING_RESUME_SUPPORT){
        p_pmlib_devinfo->attribute |= PMLIB_RING_RESUME_SUPPORT;
    }

    return PM_SUCCESS;
}
/*
 * NAME:      pmlib2pmdevinfo
 *
 * FUNCTION:  convert device information for PM library into that for PM.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 *
 */
int
pmlib2pmdevinfo(pmlib_device_info_t *p_pmlib_devinfo,
                pm_device_info_t *p_pm_devinfo)
{
    Debug0("pmlib2pmdevinfo\n");
    memset(p_pm_devinfo,0,sizeof(pm_device_info_t));

    strncpy(p_pm_devinfo->name,p_pmlib_devinfo->name,PM_DEVICE_NAME_LENGTH);

    Debug1("pmdevinfo.name = %s \n",p_pm_devinfo->name);

    p_pm_devinfo->mode      = p_pmlib_devinfo->mode;
    p_pm_devinfo->idle_time = p_pmlib_devinfo->idle_time;
    p_pm_devinfo->standby_time = p_pmlib_devinfo->standby_time;
    p_pm_devinfo->idle_time1 = p_pmlib_devinfo->idle_time1;
    p_pm_devinfo->idle_time2 = p_pmlib_devinfo->idle_time2;

    /* now convert attribute */

    if (p_pmlib_devinfo->attribute & PMLIB_GRAPHICAL_INPUT){
        p_pm_devinfo->attribute |= PM_GRAPHICAL_INPUT;
    }
    if (p_pmlib_devinfo->attribute & PMLIB_GRAPHICAL_OUTPUT){
        p_pm_devinfo->attribute |= PM_GRAPHICAL_OUTPUT;
    }
    if (p_pmlib_devinfo->attribute & PMLIB_AUDIO_INPUT){
        p_pm_devinfo->attribute |= PM_AUDIO_INPUT;
    }
    if (p_pmlib_devinfo->attribute & PMLIB_AUDIO_OUTPUT){
        p_pm_devinfo->attribute |= PM_AUDIO_OUTPUT;
    }
    if (p_pmlib_devinfo->attribute & PMLIB_RING_RESUME_SUPPORT){
        p_pm_devinfo->attribute |= PM_RING_RESUME_SUPPORT;
    }

    return PM_SUCCESS;
}





/*
 * NAME:      pm2pmlibbattery
 *
 * FUNCTION:  convert battery information for PM into that for PM library
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS: processed successfully
 *           PM_ERROR:   processed unsuccessfully
 *
 */
int
pm2pmlibbattery(pm_battery_data_t *p_pm_battery,
                pmlib_battery_t *p_pmlib_battery)
{
    static pm_battery_data_t pm_battery;

    Debug0("pm2pmlibbattery\n");
    
    memset(pm_battery,0,sizeof(pm_battery));

    /* copy the data */

    p_pmlib_battery->capacity  = p_pm_battery->capacity  ;
    p_pmlib_battery->remain    = p_pm_battery->remain;
    p_pmlib_battery->refresh_discharge_capacity = 
       p_pm_battery->refresh_discharge_capacity;
    p_pmlib_battery->refresh_discharge_time  =
       p_pm_battery->refresh_discharge_time;
    p_pmlib_battery->full_charge_count =
       p_pm_battery->full_charge_count;

    /* now convert the attribute member */

    if (p_pm_battery->attribute & PM_BATTERY){
        Debug0("PM_BATTERY flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_SUPPORTED;
    }

    if (p_pm_battery->attribute & PM_BATTERY_EXIST){
        Debug0("PM_BATTERY_EXIST flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_EXIST;
    }

    if (p_pm_battery->attribute & PM_NICD){
        Debug0("PM_BATTERY_NICD flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_NICD;
    }

    if (p_pm_battery->attribute & PM_CHARGE){
        Debug0("PM_CHARGE flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_CHARGING;
    }

    if (p_pm_battery->attribute & PM_DISCHARGE){
        Debug0("PM_DISCHARGE flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_DISCHARGING;
        p_pmlib_battery->attribute &= ~PMLIB_BATTERY_CHARGING;
    }

    if (p_pm_battery->attribute & PM_AC){
        Debug0("PM_AC flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_AC;
    }

    if (p_pm_battery->attribute & PM_DC){
        Debug0("PM_DC flag\n");
        p_pmlib_battery->attribute |= PMLIB_BATTERY_DC;
    }

    if (p_pm_battery->attribute & PM_REFRESH_DISCHARGE_REQ){
       Debug0("PM_REFRESH_DISCHARGE_REQ flag\n");
       p_pmlib_battery->attribute |= PMLIB_BATTERY_REFRESH_REQ;
    }

    return PM_SUCCESS;
}

/*
 * NAME:     pmstate2asc
 *
 * FUNCTION: Converts the integer value which represents the system state 
 *           in Power Management into the name of that state.
 *           In case of the integer value is expressed as bitwise OR of
 *           plural states, picks up the most significant one of those.
 *           The order of significancy is as follows. 
 *            SHUTDOWN > HIBERNATION > SUSPEND > STANDBY > ENABLE > FULL_ON.
 *
 * NOTES:    
 *
 * RETURN VALUE DESCRIPTION:
 *           Pointer to the name of the state. : successfully processed.
 *           Pointer to the null character     : unsuccessfully processed.
 */
char *
pmstate2asc(int pm_state)
{
    static char buffer[256];

    memset(buffer,0,sizeof(buffer));
    
    if (pm_state == PM_NONE){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"NONE");
    }
    if (pm_state & PM_SYSTEM_FULL_ON){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"FULL_ON");
    }
    if (pm_state & PM_SYSTEM_ENABLE){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"ENABLE");
    }
    if (pm_state & PM_SYSTEM_STANDBY){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"STANDBY");
    }
    if (pm_state & PM_SYSTEM_SUSPEND){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"SUSPEND");
    }
    if (pm_state & PM_SYSTEM_HIBERNATION){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"HIBERNATION");
    }
    if (pm_state & PM_SYSTEM_SHUTDOWN){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"SHUTDOWN");
    }
    if (pm_state & PM_SYSTEM_TERMINATE){
        memset(buffer,0,sizeof(buffer));
        strcpy(buffer,"TERMINATE");
    }

    return buffer;
}


