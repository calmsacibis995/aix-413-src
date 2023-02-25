static char sccsid[] = "@(#)73	1.1  src/tw/usr/lpp/tls/bin/twuserdict/udeph.c, tw, tw411, GOLD410 7/10/92 14:01:35";
/*
 * COMPONENT_NAME :     (CMDTW) - Traditional Chinese Dictionary Utility
 *
 * FUNCTIONS :          udeph.c
 *
 * ORIGINS :            27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/************************ START OF MODULE SPECIFICATION ***********************/
/*                                                                            */
/* MODULE NAME:       PhProc                                                  */
/*                                                                            */
/* DESCRIPTIVE NAME:  Update user dictionary                                  */
/*                                                                            */
/* FUNCTION:          UDEInitial : initial the phonetic user dictionary editor*/
/*                                                                            */
/*                    UDEDestroy :  destory all the data of UDE               */
/*                                                                            */
/*                    free_dict : free all the data of dictionary             */
/*                                                                            */
/*                    initial_dict : initialize all the data of dictionary    */
/*                                                                            */
/*                    open_dict : open dictionary and load Master Index       */
/*                                                                            */
/*                    UDEProcessRoutine : entry routine of UDE processs       */
/*                                                                            */
/*                    AddPhraseProc : Add new phrase main routine             */
/*                                                                            */
/*                    MergeDict_AddPhrase : Merge user dictionary and then    */
/*                          add new phrase                                    */
/*                                                                            */
/*                    AddPhrase : Add new phrase                              */
/*                                                                            */
/*                    DeletePhraseProc : Delete phrase main routine           */
/*                                                                            */
/*                    DeletePhrase : Delete phrase                            */
/*                                                                            */
/*                    delete_phrase : Delete phrase                           */
/*                                                                            */
/*                    UpdatePhraseProc : Update phrase main routine           */
/*                                                                            */
/*                    UpdatePhrase : Update phrase                            */
/*                                                                            */
/*                    DeletePhoneticProc : Delete phonetic code main routine  */
/*                                                                            */
/*                    DeletePhonetic : Delete phonetic code                   */
/*                                                                            */
/*                    UpdatePhoneticProc : Update phonetic code main routine  */
/*                                                                            */
/*                    UpdatePhonetic : Update phonetic code                   */
/*                                                                            */
/*                    filter_word : filter word from word buffer              */
/*                                                                            */
/*                    PhoneticSearchProc : Search all words with phonetic code*/
/*                          main routine                                      */
/*                                                                            */
/*                    PhoneticSearch : Search all words with phonetic code    */
/*                                                                            */
/*                    OpenDictProc : open dictionary and load Master Index    */
/*                                                                            */
/*                    MergeDictProc : Merge two dictionaries main routine     */
/*                                                                            */
/*                    MergeDict : Merge two dictionaries together             */
/*                                                                            */
/*                    word_exist_in_dict : check word exist or not            */
/*                                                                            */
/*                    phonetic_key_exist_in_DR : check phonetic exist or not  */
/*                                                                            */
/*                    search_key : search key in dictionary                   */
/*                                                                            */
/*                    search_indexkey : Search index key in Master Index      */
/*                                                                            */
/*                    search_groupkey : Search group key in Dictionary Record */
/*                                                                            */
/*                    search_word : Search word                               */
/*                                                                            */
/*                    replace_index_key_with_group_key : Replace Index Key    */
/*                         with group key                                     */
/*                                                                            */
/*                    create_new_DR : Create new DR                           */
/*                                                                            */
/*                    create_new_DR_insert_a_group : Create new DR and insert */
/*                         a group                                            */
/*                                                                            */
/*                    create_new_DR_insert_more_group : Create new DR and     */
/*                         insert more than one groups                        */
/*                                                                            */
/*                    split_DR_into_2parts : Splite DR into two parts         */
/*                                                                            */
/*                    insert_phrase_into_DR : Insert new word two DR          */
/*                                                                            */
/*                    insert_new_index_key : Insert new Index key to MI       */
/*                                                                            */
/*                    determine_RRN : get an unused Relative Record Number    */
/*                                                                            */
/*                    get_next_rrn : get next Relative Record Number          */
/*                                                                            */
/*                    key_compare : compare two key's value                   */
/*                                                                            */
/*                    delete_group : Delete one group                         */
/*                                                                            */
/*                    key_is_largest_of_DR : Check key is largest one of DR   */
/*                                                                            */
/*                    delete_index_key : Delete index key from MI             */
/*                                                                            */
/*                    largest_group_key : get the largest group key of DR     */
/*                                                                            */
/*                    word_number_greater_1 : Check word number of group is   */
/*                         is greater than one or not                         */
/*                                                                            */
/*                    word_number_of_group : get the word number og group     */
/*                                                                            */
/*                    get_next_word : get next word                           */
/*                                                                            */
/*                    AddGroup : Add one group to DR                          */
/*                                                                            */
/*                    write_DR : write DR to storage                          */
/*                                                                            */
/*                    write_INDEX : write Master Index to storage             */
/*                                                                            */
/*                    reset_modified_time : reset the last modified time      */
/*                                                                            */
/*                    AddAnotherGroup : Add all another groups to Storage     */
/*                                                                            */
/*                    load_INDEX : Load Master Index(MI) to memory            */
/*                                                                            */
/*                    load_DR : Load Dictionary Record (DR) to memory         */
/*                                                                            */
/*                    create_dict : Create a new dictionary                   */
/*                                                                            */
/*                    reset_dict : reset the data of dictionary control block */
/*                                                                            */
/*                    initial_DR : initial the data relative to DR            */
/*                                                                            */
/*                    length_of_DR : The used space of DR                     */
/*                                                                            */
/*                    group_has_enough_space : Check group has enough space   */
/*                         or not                                             */
/*                                                                            */
/*                    MI_has_enough_space : Check MI has enough space or not  */
/*                                                                            */
/*                    DR_has_enough_space : Check DR has enough space or not  */
/*                                                                            */
/*                    group_number_greater_1 : check DR has more than one     */
/*                         group or not                                       */
/*                                                                            */
/*                    get_next_index : get next index key                     */
/*                                                                            */
/*                    get_next_groupkey : get next group key                  */
/*                                                                            */
/*                    get_dict_cands_to_list_box : get all phrase from dict   */
/*                        to phrase selection window candidate buffer         */
/*                                                                            */
/*                    Add_word_to_candidates_buf : Add phrase to candidate buf*/
/*                                                                            */
/*                    check_phrase_str : Check phrase codes are correct or not*/
/*                                                                            */
/*                    check_phonetic_str : Check phonetic codes are correct or*/
/*                        not                                                 */
/*                                                                            */
/*                    AnlPhonetic : Check phonetic code is correct or not and */
/*                         filter the high byte of every phonetic codes       */
/*                                                                            */
/*                    AnlPhrase : Check phrase codes are correct or not       */
/*                                                                            */
/*                    get_highlight_text : get phrase string from phrase      */
/*                         selection window's highligh candidate              */
/*                                                                            */
/*                    dict_change : Check dictionary changed  or not          */
/*                                                                            */
/*                    is_error : Is an error code                             */
/*                                                                            */
/*                    err_msg : return error message text                     */
/*                                                                            */
/*                    replace : replace user dictionary with template one     */
/*                                                                            */
/*                    valid_dictionary : Check dictionary format is valid or  */
/*                         not                                                */
/*                                                                            */
/*                    lock_dict : lock dictionary                             */
/*                                                                            */
/*                    unlock_dict : unlock dictionary                         */
/*                                                                            */
/*                    valid_filename : Check file name is valid or not        */
/*                                                                            */
/* MODULE TYPE:       C                                                       */
/*                                                                            */
/* COMPILER   :       AIX C                                                   */
/*                                                                            */
/* AUTHOR:            Benson Lu                                               */
/*                                                                            */
/* STATUS:   Traditional Phonetic User Dictionary Editor Version 1.0          */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************* END OF SPECIFICATION *******************************/

/*****************/
/* include files */
/*****************/
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/flock.h>

#include "udeph.h"

extern int EUClen();
int     UDEInitial();
void    UDEDestroy();
void    free_dict();
int     initial_dict();
int     UDEProcessRoutine();
int     open_dict();
int     AddPhraseProc();
int     AddPhrase();
int     DeletePhraseProc();
int     DeletePhrase();
int     delete_phrase();
int     UpdatePhraseProc();
int     UpdatePhrase();
int     DeletePhoneticProc();
int     DeletePhonetic();
int     UpdatePhoneticProc();
int     UpdatePhonetic();
int     filter_word();
int     PhoneticSearchProc();
int     PhoneticSearch();
int     OpenDictProc();
int     MergeDictProc();
int     MergeDict();
int     word_exist_in_dict();
int     phonetic_key_exist_in_DR();
int     search_key();
int     search_indexkey();
int     search_groupkey();
int     search_word();
int     replace_index_key_with_group_key();
int     create_new_DR();
int     create_new_DR_insert_a_group();
int     create_new_DR_insert_more_group();
int     split_DR_into_2parts();
int     insert_phrase_into_DR();
int     insert_new_index_key();
int     determine_RRN();
int     get_next_rrn();
int     key_compare();
int     delete_group();
int     key_is_largest_of_DR();
int     delete_index_key();
char    *largest_group_key();
int     word_number_greater_1();
int     word_number_of_group();
char    *get_next_word();
int     AddGroup();
int     write_DR();
int     write_INDEX();
void    reset_modified_time();
int     AddAnotherGroup();
int     load_INDEX();
int     load_DR();
int     create_dict();
void    reset_dict();
int     initial_DR();
int     length_of_DR();
int     length_of_DR();
int     group_has_enough_space();
int     MI_has_enough_space();
int     DR_has_enough_space();
int     group_number_greater_1();
char    *get_next_index();
char    *get_next_groupkey();
int     get_dict_cands_to_list_box();
int     Add_word_to_candidates_buf();
int     check_phrase_str();
int     check_phonetic_str();
int     AnlPhonetic();
int     AnlPhrase();
void    get_highlight_text();
int     dict_change();
int     is_error();
char    *err_msg();
int     replace();
int     valid_dictionary();
int     lock_dict();
int     unlock_dict();
int     valid_filename();
void    ltoa();

unsigned short miend;                  /* End Mark of Master Index (MI)       */
unsigned short drend;                  /* End Mark of Dictionary Record (DR)  */
unsigned char gpend;                   /* End Mark of Group                   */
int rc;                                /* return code                         */
struct stat state;                     /* Status Of dict  File                */
char *modtime;                         /* Modified Time                       */

/******************************************************************************/
/* FUNCTION    : UDEInitial                                                   */
/* DESCRIPTION : initial the phonetic user dictionary editor environment      */
/* INPUT       : NONE                                                         */
/* OUTPUT      : OK = do all the initialization for the UDE                   */
/*               ERROR = some error occur                                     */
/******************************************************************************/
int UDEInitial()
{
   DictInfo *dict;                     /* pointer to dictionary control block */
   static long timeval;
   miend = PH_MI_END_MARK;             /* End Mark of MI                      */
   drend = PH_DR_END_MARK;             /* End Mark of DR                      */
   gpend = PH_GP_END_MARK;             /* End Mark of Group                   */

   obj=(UDEobject *)XtMalloc(sizeof(UDEobject)); /*Allocate UDE Control Block */
   obj->phonetic_entry =XtMalloc(PHONETIC_LEN);  /* initial phonetic_entry    */
   obj->phrase_entry= XtMalloc(PHRASE_LEN);      /* initial phrase_entry      */
   obj->open_entry = XtMalloc(PATHLEN);          /* initial open_entry        */
   obj->merge_entry = XtMalloc(PATHLEN);         /* initial merge_entry       */
   obj->phonetic_view = XtMalloc(PHONETIC_LEN);  /* initial phonetic_view     */

   /* initial phrase_select */
   obj->phrase_select = (SelectInfo *) XtMalloc(sizeof(SelectInfo));
   obj->phrase_select->highlight =0;   /* highlight candidate position        */
   obj->phrase_select->candnum = 0;    /* total candidate number              */
   obj->phrase_select->candidate =     /* pointer to candidate buffer         */
              (char **)XtMalloc(sizeof(caddr_t)*(DR_MAX_LENGTH/2));

   /* initial system dictionary */
   obj->sysdict=(DictInfo *)XtMalloc(sizeof(DictInfo)); /*Allocate system dict*/
   initial_dict(obj->sysdict);                /* initialize system dictionary */
   strcpy(obj->sysdict->name, SYSDICT);          /* default system dict name  */
   if (access(obj->sysdict->name, EXIST&READ) == ERROR) return(ERR_SO);
   obj->sysdict->flag = O_RDONLY;      /* system dictionary is read only      */
   if ((rc = open_dict( obj->sysdict)) != OK)
      return(rc);                      /* some error occurred                 */

   /* initial user dictionary */
   obj->usrdict =(DictInfo *)XtMalloc(sizeof(DictInfo));  /*Allocate user dict*/
   initial_dict(obj->usrdict);/* initialize user dictionary     */
   obj->usrdict->flag =O_RDWR |O_SYNC; /* user dictionary is read and write   */

   /* initial merge dictionary */
   obj->mergedict=(DictInfo *)XtMalloc(sizeof(DictInfo));/*allocate merge dict*/
   initial_dict(obj->mergedict);/* initialize merge dictionary    */
   obj->mergedict->flag = O_RDONLY;    /* merge dictionary is read only       */

   /* initial template dictionary */
   obj->tempdict=(DictInfo *)XtMalloc(sizeof(DictInfo));  /*Allocate temp dict*/
   initial_dict(obj->tempdict);             /* initialize template dictionary */
   obj->tempdict->flag= O_RDWR;        /*template dictionary is read and write*/

   timeval = time((long *) 0);
   srand(timeval);
   return(OK);                          /* Initialize OK                      */
}

/******************************************************************************/
/* FUNCTION    : UDEDestroy                                                   */
/* DESCRIPTION : destory all the data of UDE                                  */
/* INPUT       : NONE                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
void UDEDestroy(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char **cand_ptr;                    /* pointer to candidate buffer         */
   int cnt;                            /* Loop counter                        */

   free(obj->phonetic_entry);          /* destory phonetic_entry              */
   free(obj->phrase_entry);            /* destory phrase_entry                */
   free(obj->open_entry);              /* destory open_entry                  */
   free(obj->merge_entry);             /* destory merge_entry                 */
   free(obj->phonetic_view);           /* destory phonetic_view               */

   /* destory phrase_select */
   cand_ptr = obj->phrase_select->candidate;   /* pointer to candidate buffer */
   for (cnt=0;cnt<obj->phrase_select->candnum;cnt++) /* free candidate buffer */
      free(*(cand_ptr+cnt));
   free(obj->phrase_select);           /* free phrase selection area block    */

   /* destory dictionary */
   free_dict(obj->sysdict);            /* free system dictionary control block*/
   free_dict(obj->usrdict);            /* free user dictionary control block  */
   free_dict(obj->mergedict);          /* free merge dictionary control block */
   free_dict(obj->tempdict);           /* free template dict control block    */

   free(obj);                          /* free UDE control block              */
}

/******************************************************************************/
/* FUNCTION    : free_dic                                                     */
/* DESCRIPTION : free all the data of dictionary                              */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
void free_dict(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   /* free dictionary control block */
   free(dict->last_modified_time);     /* free last_modeified_time buffer     */
   if (dict->fd >=0)
      close(dict->fd);                 /* close dictionary                    */
   free(dict->name);                   /* free dictionary name buffer         */
   free(dict->mi);                     /* free Master Index buffer            */
   free(dict->indexkey);               /* free index key buffer               */
   free(dict->dr);                     /* free DR Buffer                      */
   free(dict->groupkey);               /* free group key buffer               */
   free(dict);                         /* free dictionary control block       */
}

/******************************************************************************/
/* FUNCTION    : initial_dict                                                 */
/* DESCRIPTION : initialize all the data of dictionary                        */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int initial_dict(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   dict->name = XtMalloc(PATHLEN);     /* allocate dictionary file name buffer*/
   dict->fd = -1;                      /* initialize file description         */
   dict->groupkey_len = 0;             /* group key length                    */
   dict->flag = 0;                     /* type of file access permitted       */
   dict->mi =XtMalloc(MI_MAX_LENGTH);  /* allocate Master Index buffer        */

  dict->last_modified_time=XtMalloc(30);  /*allocate file modified time buffer*/

   dict->mi_freespace = MI_MAX_LENGTH; /* free space of MI                    */
   dict->indexkey = XtMalloc(PHONETIC_LEN);      /* allocate index key buffer */

   dict->indexkey_ptr = 0;             /* position of index key               */
   dict->indexkey_len = 0;             /* length of index key                 */
   dict->indexkey_rrn = 0L;            /* rrn of index key                    */
   dict->dr = XtMalloc(DR_MAX_LENGTH); /* allocate DR buffer                  */

   dict->dr_freespace = DR_MAX_LENGTH; /* free space of DR                    */
   dict->groupkey = XtMalloc(PHONETIC_LEN);      /* allocate group key buffer */

   dict->groupkey_ptr = 0;             /* position of group key               */
   dict->groupkey_len = 0;             /* length of group key                 */
   dict->rrn = 0;                      /* rrn of current DR                   */
   return(OK);                         /* Initialize dictionary OK            */
}

/******************************************************************************/
/* FUNCTION    : UDEProcessRoutine                                            */
/* DESCRIPTION : User Dictionary Editor main process                          */
/* INPUT       : obj                                                          */
/*               flag                                                         */
/* OUTPUT      :                                                              */
/******************************************************************************/
int UDEProcessRoutine(obj, flag)
UDEobject *obj;                          /* UDE Control Block                 */
int flag;                                /* flag for entry routine            */
{
   if (flag == Open_Dict)
      return(OpenDictProc(obj));
   else if ((flag == Add_Phrase) || (flag == Delete_Phrase) ||
       (flag == Update_Phrase) || (flag == Delete_Phonetic) ||
       (flag == Update_Phonetic) || (flag == Phonetic_Search))
   {
      if (strlen(obj->usrdict->name) == 0)
         return(FILE_EMPTY);
      if (EUClen(obj->phonetic_entry) > MAX_PHONETIC_NUM)
          return(PHONETIC_LONG);
      if (EUClen(obj->phrase_entry) > MAX_PHRASE_NUM)
         return(PHRASE_LONG);
      if (obj->usrdict->fd < 0)         /* dictionary was not open            */
         if ((rc=open_dict(obj->usrdict)) != OK) return(rc);
/*      if (flag != Phonetic_Search)      */
         if ((rc = lock_dict(obj->usrdict->fd)) != OK) return(rc);
      switch(flag)
     {
        case Add_Phrase :
                rc = AddPhraseProc(obj);
                break;
        case Delete_Phrase :
                rc = DeletePhraseProc(obj);
                break;
        case Update_Phrase :
                rc = UpdatePhraseProc(obj);
                break;
        case Delete_Phonetic :
                rc = DeletePhoneticProc(obj);
                break;
        case Update_Phonetic :
                rc = UpdatePhoneticProc(obj);
                break;
        case Phonetic_Search :
                rc = PhoneticSearchProc(obj);
                break;
     }

 /*    if (flag != Phonetic_Search)    */
        unlock_dict(obj->usrdict->fd);
     return(rc);
   }
   else if ( flag == Merge_Dict )
   {
      rc = MergeDictProc(obj);
      unlock_dict(obj->usrdict->fd);
      unlock_dict(obj->mergedict->fd);
      unlock_dict(obj->tempdict->fd);
      return(rc);
   }
   else                         /* Impossible occurred or for future extented */
      fprintf(stderr,"Userdict Warning: Accept UDEProcess nothing.\n");

}


/******************************************************************************/
/* FUNCTION    : open_dict                                                    */
/* DESCRIPTION : open dictionary and load Master Index                        */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int open_dict(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   int stream;                         /* file description                    */

   /* open dictionary and load Master Index. If dictionary was nonexist */
   /* create it.                                                        */
   if (access(dict->name, EXIST) == ERROR)       /* check dict exist or not   */
      return(create_dict(dict));       /* dictionary nonexist, so create it   */
   else
   {
      if ((stream=open(dict->name, dict->flag)) >= 0)
      {
         if (stream != dict->fd)       /* Not as same as the previous dict    */
         {
            if (dict->fd >= 0)     close(dict->fd);      /* close the old one */
            dict->fd = stream;
         }
         if ((rc= load_INDEX(dict)) == OK)       /* load Master Index         */
            return(valid_dictionary(dict));      /* is dictionary valid ?     */
         else return(rc);                        /* some error occurred       */
      }
      else  return(ERR_OP);                      /* dictionary open error     */
   }
}

/******************************************************************************/
/*FUNCTION    : AddPhraseProc                                                 */
/*DESCRIPTION : Add phonetic & phrase to the phonetic dictionary main routine */
/*INPUT       : obj                                                           */
/*OUTPUT      : OK = add the (phonetic, phrase) to the dictionary             */
/*              ERROR = could not add the (phonetic, phrase) to the dictionary*/
/*NOTE        : phonetic = the phonetic code                                  */
/*              phrase = the phrase code                                      */
/******************************************************************************/
int AddPhraseProc(obj)
UDEobject *obj;                          /* UDE Control Block                 */
{
   char phrase[PHRASE_LEN];              /* phrase code buffer                */
   char phonetic[PHONETIC_LEN];          /* phonetic code buffer              */

  if (strlen(obj->phonetic_entry) == 0) return(ERR_PHONETIC_EMPTY);
  if (strlen(obj->phrase_entry) == 0) return(ERR_PHRASE_EMPTY);

/* check there are string in phonetic entry field and the phrase entry field */
  if (((rc=check_phrase_str(obj->phrase_entry, phrase)) == OK) &&
      ((rc=check_phonetic_str(obj->phonetic_entry,phonetic)) ==OK))
  {
      /* Add phrase into user dict successful or the phrase already exist in */
      /* user dict, this will be acted as no error occurred */
      if ((rc=AddPhrase(obj ,phonetic, phrase,0)) == ERR_MF)
      {
         rc = MergeDict_AddPhrase(obj,phonetic,phrase);
      }
      if ((rc== OK)||(rc==EX_IN_USR))
      {
        if ((rc=phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == TRUE)
           strcpy(obj->phonetic_view, obj->phonetic_entry);
        else if (rc==FALSE)
        {
           /* Add phrase to user dictionary is successful but soon deleted */
           /* by another process                                           */
           strcpy(obj->phonetic_view, ""); /* phonetic view area will be empty*/
           strcpy(phonetic, "");       /* set the phonetic code to be null    */
        }
        else return(rc);               /* some error occurred                 */
        return(get_dict_cands_to_list_box(obj, phonetic));
      }
      else
        return(rc);                    /* some error occurred                 */
  }
  else return(rc);                     /* Phonetic or phrase code error       */
}

/******************************************************************************/
/*FUNCTION    : MergeDict_Addphrase                                               */
/*DESCRIPTION : Merge user dictionary then add phrase into user dictionary    */
/*              as the first phrase                                           */
/*INPUT       : obj = the dictionary to add phrase                            */
/*              phonetic = the phonetic code                                  */
/*              phrase   = the phrase code                                    */
/*OUTPUT      : OK =  add phrase into dictionary                              */
/*              ERROR = can not add phrase into dictionary                    */
/******************************************************************************/
int MergeDict_AddPhrase(obj,phonetic,phrase)
UDEobject *obj;                        /* UDE Control Block                   */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
{
   int random;
   char string[34];

   /* find a template merge dictionary file name */
   strcpy(obj->mergedict->name, ".");
   random = rand();
   ltoa(random, string);
   strcat(obj->mergedict->name, string);
   random=rand();
   ltoa(random,string);
   strcat(obj->mergedict->name, string);

   /* find a template dictionary file name */
   strcpy(obj->tempdict->name, ".");
   random = rand();
   ltoa(random, string);
   strcat(obj->tempdict->name, string);
   random = rand();
   ltoa(random, string);
   strcat(obj->tempdict->name, string);

   if ((rc = MergeDict(obj)) == OK)
   {
      unlock_dict(obj->mergedict->fd);
      unlink(obj->mergedict->name);    /* delete merge dictionary             */
      unlock_dict(obj->tempdict->fd);
      unlink(obj->tempdict->name);     /* delete template dictionary          */
      return(AddPhrase(obj,phonetic, phrase, 0));
   }
   else
      return(rc);
}

/******************************************************************************/
/*FUNCTION    : AddPhrase                                                     */
/*DESCRIPTION : Add phonetic & phrase to the phonetic user dictionary as      */
/*              the nth word of group                                         */
/*INPUT       : obj = the dictionary to add phrase                            */
/*              phonetic = the phonetic code                                  */
/*              phrase   = the phrase code                                    */
/*              nth      = the (phonetic , phrase) as the nth word of group   */
/*OUTPUT      : OK =  add phrase into dictionary                              */
/*              ERROR = can not add phrase into dictionary                    */
/******************************************************************************/
int AddPhrase(obj, phonetic, phrase, nth)
UDEobject *obj;                        /* UDE Control Block                   */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
int nth;               /* insert (phonetic,phrase) as the nth phrase of group */
{
   DictInfo *dict;                     /* pointer to dictionary control block */
   int length;                         /* Inserted length                     */

   /* Check word exist in system dictionary or not */
   /* The internal code of word in system system is not more than 4 bytes */
   if (strlen(phrase) <=4)
   {
      if ((rc = word_exist_in_dict(obj->sysdict,phonetic,phrase)) == TRUE)
         return(EX_IN_SYS);            /* PHONETIC/PHRASE exist in system dict*/
   }
   /* check the length while insert this word */
   if ((rc = phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == TRUE)
   {
      if ((rc = word_exist_in_dict(obj->usrdict,phonetic,phrase)) == TRUE)
         return(EX_IN_USR);            /* PHONETIC/PHRASE exist in user dict  */
      else if (rc == FALSE)
         length = strlen(phrase);      /* the length of phrase                */
      else return(rc);                 /* some error occured                  */
   }
   else if (rc==FALSE)                 /* phonetic do not exist in dict       */
      length = strlen(phonetic)+strlen(phrase)+ gpend_len;
   else return(rc);

   if ((rc = DR_has_enough_space(obj->usrdict, phonetic, length)) == TRUE)
      return(insert_phrase_into_DR(obj->usrdict,phonetic,phrase,nth));
   else if (rc == FALSE)              /*after insert the phrase,DR is overflow*/
   {
      if ((rc= phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == TRUE)
      {
         if ((rc=group_has_enough_space(obj->usrdict,phonetic,length)) == TRUE)
         {
            if ((rc =split_DR_into_2parts(obj->usrdict,phonetic)) != OK)
               return(rc);             /* some error occurred                 */
            if (DR_has_enough_space(obj->usrdict,phonetic,length) == TRUE)
               return(insert_phrase_into_DR(obj->usrdict,phonetic,phrase,nth));
            else if (rc == FALSE)      /* DR has not enough space             */
            {
               if ((rc = split_DR_into_2parts(obj->usrdict, phonetic)) != OK)
                  return(rc);          /* some error occurred                 */
               return(insert_phrase_into_DR(obj->usrdict,phonetic,phrase,nth));
            }
            else return(rc);           /* some error occured                  */
         }
         else if (rc == FALSE)         /* group has not enough space          */
            return(ERR_GF);            /* group was full                      */
         else return(rc);              /* some error occurred                 */
      }
      else if (rc == FALSE)            /* the key is nonexist                 */
      {
         if ((rc=group_number_greater_1(obj->usrdict,phonetic)) == TRUE)
         {
            if ((rc = split_DR_into_2parts(obj->usrdict, phonetic)) != OK)
               return(rc);             /* some error occurred                 */
            if (DR_has_enough_space(obj->usrdict, phonetic,length) == TRUE)
               return(insert_phrase_into_DR(obj->usrdict,phonetic,phrase,nth));
            else
             return(create_new_DR_insert_a_group(obj->usrdict,phonetic,phrase));
         }
         else if (rc == FALSE)         /* group number of DR is 1             */
            return(create_new_DR_insert_a_group(obj->usrdict,phonetic,phrase));
         else return(rc);              /* some error occured                  */
      }
      else return(rc);                 /* some error occured                  */
   }
   else return(rc);                    /* some error occured                  */
}


/******************************************************************************/
/*FUNCTION    : DeletePhraseProc                                              */
/*DESCRIPTION : Delete (phonetic,phrase) of the phonetic dictionary           */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = delete phrase from the dictionary                        */
/*              ERROR = there is some error occur                             */
/*NOTE        : phonetic = the phonetic input code                            */
/*              phrase   = the phrase input code                              */
/******************************************************************************/
int DeletePhraseProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char phonetic[PHONETIC_LEN];        /* phonetic code buffer                */
   char phrase[PHRASE_LEN];            /* phrase code buffer                  */

   if ((rc=check_phonetic_str(obj->phonetic_view, phonetic))== OK)
   {
     if (obj->phrase_select->highlight >0)
        get_highlight_text(obj,phrase);/* get highlight candidate             */
     else
        return(ERR_EMPTY);
     if (((rc = DeletePhrase(obj->usrdict, phonetic, phrase)) == OK) ||
          (rc ==EX_IN_NON))
     {
        if ((rc=phonetic_key_exist_in_DR(obj->usrdict,phonetic))== FALSE)
        {
           strcpy(obj->phonetic_view,"");   /* set phonetic view be empty     */
           strcpy(phonetic, "");       /* set phonetic code be empty          */
        }
        else if (is_error(rc))
             return(rc);               /* some error occurred                 */
        return(get_dict_cands_to_list_box(obj, phonetic));
     }
     else return(rc);                  /* some error occured                  */
  }
  else return(rc);                /* error phonetic code or error phrase code */
}

/******************************************************************************/
/*FUNCTION    : DeletePhrase                                                  */
/*DESCRIPTION : delete the word of (phonetic, phrase) in the dictionary       */
/*INPUT       : dict = the dictionary which wants to delete this word         */
/*              phonetic = the phonetic code of word                          */
/*              phrase = the phrase code of word                              */
/*OUTPUT      : OK = delete phrase from the dictionary                        */
/*              ERROR = there is some error occur                             */
/******************************************************************************/
int DeletePhrase(dict, phonetic, phrase)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
{
   if ((rc = word_exist_in_dict(dict, phonetic, phrase)) == TRUE)
   {
     if ((rc = word_number_greater_1(dict, phonetic)) == TRUE)
        return(delete_phrase(dict, phonetic, phrase));     /* delete phrase   */
     else if (rc == FALSE)             /* phonetic code only has one word     */
         return(DeletePhonetic(dict, phonetic));           /* delete phonetic */
     else return(rc);                  /* some error occured                  */
   }
   else if (rc == FALSE)
      return(EX_IN_NON);               /* word is not exist in dictionary     */
   else return(rc);                    /* some error occured                  */
}

/******************************************************************************/
/* FUNCTION    : delete_phrase                                                */
/* DESCRIPTION : delete phrase which with phonetic code                       */
/* INPUT       : dict                                                         */
/*               phonetic                                                     */
/*               phrase                                                       */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int delete_phrase(dict, phonetic, phrase)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
{
   char *chptr;                        /* pointer to DR                       */
   char *delptr;                       /* pointer to DR                       */

   /* find the position of word and delete it */
   if ((rc = search_word(dict, phonetic, phrase)) == TRUE)
   {
      /* shift all words and groups of this DR after this (phonetic,phrase) */
      /* left the length of phrase */
      delptr = dict->dr + dict->word_ptr;   /* beginning of the word          */
      chptr =  dict->dr + dict->word_ptr +strlen(phrase);  /* the end of word */
      /* shift content of DR left */
      for ( ;*chptr != NULL; *delptr = *chptr,chptr++, delptr++);
      for ( ;*delptr != NULL;*delptr = NULL, delptr++); /* clear other content*/
      return(write_DR(dict));                           /* write DR to storage*/
   }
   else if (rc==FALSE)
      return(EX_IN_NON);               /* word is not exist in dictionary     */
   else return(rc);                    /* some error occured                  */
}

/******************************************************************************/
/*FUNCTION    : UpdatePhraseProc                                              */
/*DESCRIPTION : update the OldPhrase with NewPhrase which have same phonetic  */
/*              code main routine                                             */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = NewPhrase replace OldPhrase                              */
/*              ERROR = the NewPhrase can not replace OldPharase              */
/*NOTE        : usrdict = the currently active phonetic user dictionary       */
/*              phonetic = the phonetic code of OldPhrase and NewPhrase       */
/*              OldPhrase = the old phrase which may be replace by NewPhrase  */
/*              NewPhrase = the new word which want to replace OldPhrase      */
/******************************************************************************/
int UpdatePhraseProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char phonetic[PHONETIC_LEN];        /* phonetic code buffer                */
   char NewPhrase[PHRASE_LEN];         /* new phrase code buffer              */
   char OldPhrase[PHRASE_LEN];         /* old phrase code buffer              */
   int ret;

  if (strlen(obj->phrase_entry) == 0) return(ERR_PHRASE_EMPTY);
  /* check phrase entry window is not empty and highlight candidate exist */
  if ((rc=check_phrase_str(obj->phrase_entry, NewPhrase)) ==  OK)
  {
     if (obj->phrase_select->highlight >0)
        get_highlight_text(obj,OldPhrase);/* get highlight candidate          */
     else
        return(ERR_EMPTY);
     if ((rc=check_phonetic_str(obj->phonetic_view, phonetic)) !=OK)
        return(rc);                    /* phonetic code is error              */
     if (((rc = UpdatePhrase(obj, phonetic, OldPhrase, NewPhrase)) == OK) ||
          (rc == EX_IN_NON))
     {
        ret = rc;
        if ((rc=phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == FALSE)
        {
           /* phonetic view area is empty now */
           strcpy(obj->phonetic_view, "");
           strcpy(phonetic, "");
        }
        else if (is_error(rc)) return(rc);
        rc = get_dict_cands_to_list_box(obj, phonetic);
        if (ret == EX_IN_NON)
           return(ret);
        else
           return(rc);
     }
     else return(rc);                  /* some error occured                  */
  }
  else return(rc);    /* phrase code is error or highlight candidate nonexist */
}

/******************************************************************************/
/*FUNCTION    : UpdatePhrase                                                  */
/*DESCRIPTION : update the OldPhrase with NewPhrase which have same phonetic  */
/*INPUT       : dict = the dictionary                                         */
/*              phonetic = the phonetic code of OldPhrase and NewPhrase       */
/*              OldPhrase = the old phrase which may be replace by NewPhrase  */
/*              NewPhrase = the new word which want to replace OldPhrase      */
/*OUTPUT      : OK = NewPhrase replace OldPhrase                              */
/*              ERROR = the NewPhrase can not replace OldPharase              */
/******************************************************************************/
int UpdatePhrase(obj,phonetic,OldPhrase, NewPhrase)
UDEobject *obj;                        /* UDE Control Block                   */
char *phonetic;                        /* pointer to phonetic code            */
char *OldPhrase;                       /* pointer to old phrase code          */
char *NewPhrase;                       /* pointer to new phrase               */
{
   int ptr;                            /* point to the position of word       */
   int prev_ptr;                       /* point to the position of word       */
   char *chptr;                        /* pointer to  phrase code             */
   char *delptr;                       /* pointer to DR                       */
   char *word;                         /* pointer to word                     */
   int length;                         /* length which be shift               */
   int nth;                            /* insert as the nth word of group     */

   /* The internal code of word in system system is not more than 4 bytes */
   if (strlen(NewPhrase) <=4)
   {
      if ((rc = word_exist_in_dict(obj->sysdict,phonetic,NewPhrase)) == TRUE)
         return(EX_IN_SYS);    /* word with NewPhrase is in system dictionary */
      else if (is_error(rc)) return(rc);  /* some error occurred              */
   }

   if ((rc = word_exist_in_dict(obj->usrdict,phonetic,OldPhrase)) == FALSE)
      return(EX_IN_NON);/*word (phonetic/OldPhrase) is not in user dictionary */
   else if (rc != TRUE)
          return(rc);  /* some error occurred                 */
   if((rc=word_exist_in_dict(obj->usrdict,phonetic,NewPhrase)) == TRUE)
      return(EX_IN_USR);   /* word (phonetic/NewPhrase) is in user dictionary */
   else if (is_error(rc)) return(rc);  /* some error occurred                 */

   /* find start position of group of phonetic code */
   if (is_error(rc=search_groupkey(obj->usrdict, phonetic))) return(rc);
   ptr = obj->usrdict->groupkey_ptr + obj->usrdict->groupkey_len;
   prev_ptr = ptr;

   /* find word position of OldPhrase */
   nth = 0;
   while (key_compare(word=get_next_word(obj->usrdict->dr,&ptr),
                                             OldPhrase) != EQUAL)

   {
      if (strncmp(word, &gpend, gpend_len) == 0)
        return(EX_IN_NON);/*word (phonetic/OldPhrase)is not in user dictionary*/
      prev_ptr = ptr;                  /* position of this word               */
      nth++;                           /* word position number increase by 1  */
   }
   if (strlen(OldPhrase) >= strlen(NewPhrase))
   {
       /* replace OldPhrase with NewPhrase */
       chptr = NewPhrase;
       delptr = obj->usrdict->dr + prev_ptr;
       for ( ; *chptr != NULL; *delptr = *chptr, chptr++, delptr++);
       *(delptr-1) &= 0x7f;            /* set the 7th bit of last byte to off */
       chptr = obj->usrdict->dr+prev_ptr+strlen(OldPhrase);
       if (chptr != delptr)  /*length of OldPhrase is greater than NewPhrase  */
       {
      /* shift all contents next to OldPhrase left LEN(OldPhrase-NewPhrase) */
     for ( ;strncmp(chptr,&drend,drend_len)!=0;*delptr=*chptr,chptr++,delptr++);
          (void) memcpy(delptr,(char *)&drend, drend_len);
          for ( delptr+=drend_len;*delptr != NULL;*delptr = NULL, delptr++);
       }
   }
   else /* strlen(OldPhrase) is less than strlen(NewPhrase) */
   {
      length = strlen(NewPhrase) - strlen(OldPhrase);/* length to shift */
      if (DR_has_enough_space(obj->usrdict,phonetic,length) == TRUE)
      {
        /*  shift length bytes right */
         chptr = obj->usrdict->dr+prev_ptr+strlen(OldPhrase);
         for (delptr=chptr;strncmp(delptr,&drend,drend_len)!=0;delptr++);
         delptr += drend_len;
         for(delptr+=drend_len;delptr>=chptr;*(delptr+length)=*delptr,delptr--);
         /* replace OldPhrase with NewPhrase */
         delptr = obj->usrdict->dr+prev_ptr;
         chptr = NewPhrase;
         for ( ; *chptr != NULL; *delptr = *chptr, chptr++, delptr++);
         *(delptr-1) &= 0x7f;/* set the 7th bit of last byte to off */
      }
      else if (group_has_enough_space(obj->usrdict,phonetic, length) == TRUE)
      {
       /* Because DR has not enough space for replace OldPhrase with NewPhras */
       /* , so first delete OldPhrase and then add NewPhrase to dictionary */
         if ((rc = DeletePhrase(obj->usrdict ,phonetic, OldPhrase)) == OK)
            return(AddPhrase(obj,phonetic, NewPhrase, nth));
         else
            return(rc);                /* some error occured                  */
      }
      else
         return(ERR_GF);               /* group was full                      */
   }
   return(write_DR(obj->usrdict));     /* write DR to storage                 */
}

/******************************************************************************/
/*FUNCTION    : DeletePhoneticProc                                            */
/*DESCRIPTION : Delete the phonetic code in the dictionary main routine       */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = delete the phonetic code from the dictionary             */
/*              ERROR = some error occur                                      */
/*NOTE        : usrdict = the currently active phonetic user dictionary       */
/*              phonetic = the phonetic code of group                         */
/******************************************************************************/
int DeletePhoneticProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char phonetic[PHONETIC_LEN];        /* phonetic code buffer                */

  if ((rc=check_phonetic_str(obj->phonetic_view, phonetic)) == OK)
  {
     if (((rc = DeletePhonetic(obj->usrdict, phonetic)) == OK) ||
         (rc == EX_IN_NON))
     {
        strcpy(obj->phonetic_view, "");          /* clear phonetic view       */
        return(get_dict_cands_to_list_box(obj, ""));
     }
     else return(rc);                  /* some error occured                  */
  }
  else return(rc);                     /* phonetic code is error              */
}


/******************************************************************************/
/*FUNCTION    : DeletePhonetic                                                */
/*DESCRIPTION : Delete the phonetic code in the dictionary                    */
/*INPUT       : dict = the dictionary                                         */
/*              phonetic = the phonetic code og the group                     */
/*OUTPUT      : OK = delete the phonetic code from the dictionary             */
/*              ERROR = some error occur                                      */
/******************************************************************************/
int DeletePhonetic(dict, phonetic)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
{
   if ((rc =phonetic_key_exist_in_DR(dict,phonetic)) == TRUE)
   {
      /* if the number of groups is greater than 1                   */
      /* then the group with phonetic code will be deleted           */
      /* otherwise the phonetic code in Master Index will be deleted */
      if ((rc=group_number_greater_1(dict, phonetic)) ==  TRUE)
         return(delete_group(dict, phonetic));        /* delete group         */
      else if (rc==FALSE)
         return(delete_index_key(dict,phonetic));     /* delete index         */
      else return(rc);                 /* some error occurred                 */
   }
   else if (rc == FALSE)
        return(EX_IN_NON);             /* phonetic code is not exist in dict  */
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/*FUNCTION    : UpdatePhoneticProc                                            */
/*DESCRIPTION : update all the words of OldPhonetic code to NewPhonetic       */
/*                                                         main routine       */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = can update all the words of OldPhonetic to NewPhonetic   */
/*              ERROR = some error occur                                      */
/*NOTE        : usrdict = the dictionary                                      */
/*              NewPhonetic = the new phonetic code of words                  */
/*              OldPhonetic = the old phonetic code of words                  */
/******************************************************************************/
int UpdatePhoneticProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char NewPhonetic[PHONETIC_LEN];     /* new phonetic code buffer            */
   char OldPhonetic[PHONETIC_LEN];     /* old phonetic code buffer            */
   int ret;

  if (strlen(obj->phonetic_entry) == 0) return(ERR_PHONETIC_EMPTY);
  if (strlen(obj->phonetic_view) == 0) return(ERR_PHONETIC_EMPTY);
  /* check phonetic entry window and phonetic view area */
  if (strcmp(obj->phonetic_entry, obj->phonetic_view) == 0)
     return(OK);
  if (((rc=check_phonetic_str(obj->phonetic_entry, NewPhonetic)) == OK) &&
      ((rc=check_phonetic_str(obj->phonetic_view,OldPhonetic)) == OK))
  {
     /* Update words of OldPhonetic group to NewPhonetic code group */
     if (((rc = UpdatePhonetic(obj, NewPhonetic, OldPhonetic)) == OK)||
         (rc == EX_IN_NON))
     {
        ret = rc;
        if ((rc=phonetic_key_exist_in_DR(obj->usrdict,NewPhonetic)) == TRUE)
            strcpy(obj->phonetic_view, obj->phonetic_entry);
        else if (rc==FALSE)
        {
            strcpy(obj->phonetic_view,"");/*set phonetic view area to be empty*/
            strcpy(NewPhonetic, "");
        }
        else return(rc);
        rc = get_dict_cands_to_list_box(obj, NewPhonetic);
        if (ret == EX_IN_NON)
           return(ret);
        else
           return(rc);
     }
     else return(rc);                  /* some error occured                  */
  }
  else return(rc);                     /*Phonetic code or phrase code is error*/
}


/******************************************************************************/
/*FUNCTION    : UpdatePhonetic                                                */
/*DESCRIPTION : update all the words of OldPhonetic code to NewPhonetic       */
/*INPUT       : dict = the dictionary                                         */
/*              NewPhonetic = the new phonetic code of words                  */
/*              OldPhonetic = the old phonetic code of words                  */
/*OUTPUT      : OK = can update all the words of OldPhonetic to NewPhonetic   */
/*              ERROR = some error occur                                      */
/******************************************************************************/
int UpdatePhonetic(obj, NewPhonetic, OldPhonetic)
UDEobject *obj ;                       /* UDE Control Block                   */
char *NewPhonetic;                     /* pointer to new phonetic code        */
char *OldPhonetic;                     /* pointer to old phonetic code        */
{
   char group_buf[DR_MAX_LENGTH];      /* group buffer                        */
   char *chptr;                        /* pointer to DR                       */
   char *gptr;                         /* pointer to group_buf                */
   char *wptr;                         /* pointer to word                     */
   char word[PHRASE_LEN];              /* word code buffer                    */
   int length;/*different length of OldPhonetic's words to NewPhonetic's words*/
   int nth;              /* insert (phonetic,phrase) as the nth word of group */
   int ptr;                            /* point to the position of word       */

   (void) memset(group_buf, NULL, DR_MAX_LENGTH);/* clear group_buf */
   if ((rc = search_groupkey(obj->usrdict, OldPhonetic)) == EQUAL)
   {
     /* copy OldPhonetic group to buffer */
      gptr = group_buf;
      chptr = obj->usrdict->dr + obj->usrdict->groupkey_ptr +
                                 obj->usrdict->groupkey_len;
      for ( ;strncmp(chptr,(char *)&gpend,gpend_len)!=0 ;
                                   *gptr=*chptr,chptr++,gptr++);
      if ((rc=phonetic_key_exist_in_DR(obj->sysdict,NewPhonetic)) == TRUE)
      {
         /* filter the words in OldPhonetic which become to NewPhonetic */
         /* will be a system dictionary word .                          */
         ptr = obj->usrdict->groupkey_ptr + obj->usrdict->groupkey_len;
         while (LOOP)
        {
           strcpy(word, wptr=get_next_word(obj->usrdict->dr, &ptr));
           if (strncmp(word, (char *)&gpend, gpend_len) == 0)
              break;
           else
           {
              /* if (NewPhonetic word) exists in system dictionary then */
              /* filte it from group_buf.                               */
              if((rc=word_exist_in_dict(obj->sysdict, NewPhonetic, word)) == TRUE)
                 filter_word(group_buf, word);
           }
         }
      }
   }
   else if ((rc==LARGE)||(rc==LESS))
     return(EX_IN_NON); /* OldPhonetic is not exist in user dict */
   else return(rc);                    /* some error occured                  */

   if ((rc=search_groupkey(obj->usrdict, NewPhonetic)) == EQUAL)
   {
      /* There are some words of NewPhonetic exist in user dictionary. */
      /* So filter the same words from OldPhonetic.                    */
      ptr = obj->usrdict->groupkey_ptr + obj->usrdict->groupkey_len;
      while (LOOP)
      {
         strcpy(word, wptr = get_next_word(obj->usrdict->dr, &ptr));
         if (strncmp(wptr, (char *)&gpend,gpend_len) == 0)
            break;
         else
            filter_word(group_buf, word);   /* filter word from group_buf     */
      }
   }
   else if (is_error(rc)) return(rc);       /* sonme error occurred           */

   /* If All words of OldPhonetic is already exist in NewPhonetic */
   /* then delete the group of OldPhonetic only */
   for (gptr=group_buf,length=0;*gptr!=NULL;gptr++,length++);   /* find length*/
   if (length == 0)
       return(DeletePhonetic(obj->usrdict,OldPhonetic));

   if ((rc=group_has_enough_space(obj->usrdict, NewPhonetic, length)) == TRUE)
   {
    /* Delete all contents of OldPhonetic from user dictionary and insert all */
    /* different words of OldPhonetic code to the end of NewPhonetic group    */
      if (is_error(rc=search_groupkey(obj->usrdict,NewPhonetic))) return(rc);
      nth = word_number_of_group(obj->usrdict ,NewPhonetic) +1;
      ptr = 0;
      while (strncmp(wptr = get_next_word(group_buf,&ptr),
                                       &gpend,gpend_len) != 0)
      {
         strcpy(word, wptr);
         /* Add word which with NewPhonetic to the end of NewPhonetic group */
         if  ((rc = AddPhrase(obj, NewPhonetic, word, nth)) != OK)
         {
            if (rc != EX_IN_SYS)
              return(rc);                /* some error occurred               */
         }
         else
           nth++;                      /*increase nth by 1,insert as next word*/
      }
      if((rc==OK)||(rc==EX_IN_SYS)) /* Update words of OldPhonetic to NewPhonetic OK */
      {
        if ((rc=phonetic_key_exist_in_DR(obj->usrdict,NewPhonetic)) == TRUE)
           return(DeletePhonetic(obj->usrdict,OldPhonetic));
        else
           return(PHRASE_SYS_DUP);
      }
      else
         return(rc);
   }
   else if (rc==FALSE)  return(ERR_GF);/* group was full                      */
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/* FUNCTION    : filter_word                                                  */
/* DESCRIPTION : filter word from word buffer                                 */
/* INPUT       : word_buf                                                     */
/*               word                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int filter_word(word_buf, word)
char *word_buf;                        /* pointer to word buffer              */
char *word;                            /* pointer to word code                */
{
   int prev_ptr, ptr;                  /* position of wrod_buf                */
   char *word_ptr;                     /* pointer to word                     */
   char *delptr;                       /* pointer to word_buf                 */
   char *chptr;                        /* pointer to word_buf                 */

   ptr = 0;                       /* pointer to the beginning of word_buffer  */
   /* check all words of word_buf to find word and then delete the word */
   /* from word_buf */
   while (LOOP)
   {
      prev_ptr = ptr;                  /*pointer to the position of this word */
      word_ptr = get_next_word(word_buf, &ptr); /* get next word from word_buf*/
      if (key_compare(word, word_ptr) == EQUAL)
      {
         /* delete the word from word_buf */
         delptr = word_buf + prev_ptr;
         chptr = delptr + strlen(word_ptr);
         for ( ;*chptr != NULL;*delptr=*chptr, chptr++, delptr++);
         for ( ; *delptr != NULL; *delptr = NULL, delptr++);
         return(OK);              /* word is exist in word_buf and delete it  */
      }
      else if (strncmp(word_ptr, &gpend,gpend_len) == 0)
         return(EX_IN_NON);            /* word is not exist in word_buf       */
   }
}

/******************************************************************************/
/*FUNCTION    : PhoneticSearchProc                                            */
/*DESCRIPTION : search all words of phonetic code in dictionary main routine  */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = the phonetic code exist in dictionary                    */
/*              ERROR =  the phonetic code do  not exist in dictionary        */
/******************************************************************************/
int PhoneticSearchProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
  char phonetic[PHONETIC_LEN];         /* phonetic code buffer                */

  if (strlen(obj->phonetic_entry) == 0) return(ERR_PHONETIC_EMPTY);
  /* check phonetic entry window to find phonetic code */
  if ((rc=check_phonetic_str(obj->phonetic_entry, phonetic)) == OK)
  {
     if ((rc = PhoneticSearch(obj->usrdict, phonetic)) == OK)
     {
        /* set phonetic code to the phonetic view area  */
        strcpy(obj->phonetic_view, obj->phonetic_entry);
        return(get_dict_cands_to_list_box(obj, phonetic));
     }
     else return(rc);                  /* some error occurred                 */
  }
  else return(rc);                     /* error phonetic code                 */
}


/******************************************************************************/
/*FUNCTION    : PhoneticSearch                                                */
/*DESCRIPTION : search all words of phonetic code in dictionary               */
/*INPUT       : dict  = the dictionary                                        */
/*              phonetic  = the phonetic code of words                        */
/*OUTPUT      : OK = the phonetic code exist in dictionary                    */
/*              ERROR =  the phonetic code do  not exist in dictionary        */
/******************************************************************************/
int PhoneticSearch(dict, phonetic)
DictInfo  *dict;                       /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
{
   /* If find the phonetic code in dict then successful */
   /* otherwise some error occurred */
   if ((rc = search_key(dict,phonetic)) == EQUAL)
      return(OK);                      /* find phonetic code from dict        */
   else if ((rc==LARGE) || (rc==LESS))
      return(EX_IN_NON);          /* phonetic code is not exist in dictionary */
   else  return(rc);                   /* some error occurred                 */
}


/******************************************************************************/
/*FUNCTION    : OpenDictProc                                                  */
/*DESCRIPTION : open filename main routine                                    */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = can open the dictionary                                  */
/*              ERROR = some error occur                                      */
/*NOTE        : open_filename = the dictionary file name want to be open      */
/******************************************************************************/
int OpenDictProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   int stream, temp_stream;            /* file description                    */
   char temp_usrdict_name[PATHLEN];    /* temp user dictionary file name      */
   char phonetic[PHONETIC_LEN];        /* phonetic code buffer                */

   if (strlen(obj->open_entry) == 0)
      return(ERR_IV);                  /* dictionary name is invalid          */
   if (!valid_filename(obj->open_entry))
      return(ERR_IV);
   if (strcmp(obj->open_entry, obj->sysdict->name) == 0)
      return(SAME_US);    /* user dict is not allowed as same as system dict  */
   strcpy(temp_usrdict_name, obj->usrdict->name);     /* save user dict name  */
   temp_stream = obj->usrdict->fd;     /* save user dict file description     */
   if (strcmp(obj->usrdict->name, obj->open_entry) !=0)
   {
      /* the new user dict name is not as same as the old user dict name */
      strcpy(obj->usrdict->name, obj->open_entry);/*copy new name to user dict*/
      strcpy(obj->usrdict->last_modified_time, ""); /* reset the modified time*/
   }
   if ((rc=open_dict(obj->usrdict)) == OK)
   {
     if (check_phonetic_str(obj->phonetic_view, phonetic) != OK)
         strcpy(obj->phonetic_view,"");/* set phonetic view area to be empty  */
     else if ((rc=phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == FALSE)
     {
         strcpy(phonetic, "");
         strcpy(obj->phonetic_view,"");/* set phonetic view area to be empty  */
     }
     else if (is_error(rc)) return(rc);/* some error occurred                 */
     return(get_dict_cands_to_list_box(obj, phonetic));
   }
   else                                /* open new user dictionary is failed  */
   {
      /* recover the user dictionary name and file description */
      strcpy(obj->usrdict->name, temp_usrdict_name);
      obj->usrdict->fd = temp_stream;
      return(rc);                      /* some error occurred                 */
   }
}

/******************************************************************************/
/*FUNCTION    : MergeDictProc                                                 */
/*DESCRIPTION : merge the merge_filename into the open_filename main routine  */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = can merge the merge_filename into the open_filename      */
/*              ERROR = some error occur                                      */
/*NOTE        : merge_filename = the dictionary file name want to merge       */
/*              open_filename = the dictionary file name want to be merge     */
/******************************************************************************/
int MergeDictProc(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char temp_usrdict_name[PATHLEN];    /* temp user dictionary file name      */
   char phonetic[PHONETIC_LEN];        /* phonetic code buffer                */
   int random;
   long time();
   char string[34];

   if (strlen(obj->merge_entry) == 0)
      return(ERR_IV);                  /* dictionary name is invalid          */
   if (!valid_filename(obj->merge_entry))
      return(ERR_IV);
   if (strcmp(obj->usrdict->name, obj->merge_entry) == 0)
      return(SAME_UM);  /* user dictionary , merge dictionary have same name  */
   if (strcmp(obj->merge_entry,obj->sysdict->name) ==0)
      return(SAME_MS);  /* merge dictionary ,system dictionary have same name */
   strcpy(obj->mergedict->name , obj->merge_entry);/* set merge dict */
   if (access(obj->mergedict->name, EXIST&READ) == ERROR) return(ERR_OP);
   /* find a template dictionary file name */
   strcpy(obj->tempdict->name, ".");
   random = rand();
   ltoa(random,string);
   strcat(obj->tempdict->name, string);
   random = rand();
   ltoa(random, string);
   strcat(obj->tempdict->name, string);
   if ((strcmp(obj->tempdict->name, obj->usrdict->name) == 0) ||
       (strcmp(obj->tempdict->name, obj->mergedict->name)==0))
   {
      strcat(obj->tempdict->name,".bak");   /* reassign template dict name    */
   }

   if ((rc = MergeDict(obj)) == OK)
   {
    /*  copy tempfile to usrdict file, and delete tempfile.....*/
      if (check_phonetic_str(obj->phonetic_view,phonetic) != OK)
      {
         /* reassigned phonetic view area */
         if (check_phonetic_str(obj->phonetic_entry, phonetic) != OK)
            strcpy(obj->phonetic_view, ""); /* set phonetic view area empty   */
         else
            strcpy(obj->phonetic_view, obj->phonetic_entry);
      }
      if ((rc=phonetic_key_exist_in_DR(obj->usrdict,phonetic)) == FALSE)
      {
         strcpy(phonetic, "");
         strcpy(obj->phonetic_view, "");    /* set phonetic view area empty   */
      }
      else if (is_error(rc)) return(rc);    /* some error occurred            */
      return(get_dict_cands_to_list_box(obj, phonetic));
   }
   else
      return(rc);                           /* some error occurred            */
}


/******************************************************************************/
/*FUNCTION    : MergeDict                                                     */
/*DESCRIPTION : merge the merge_filename into the open_filename               */
/*INPUT       : NONE                                                          */
/*OUTPUT      : OK = can merge the merge_filename into the open_filename      */
/*              ERROR = some error occur                                      */
/******************************************************************************/
int MergeDict(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   int usr_indexptr;                   /* pointer of user dict index key      */
   int merge_indexptr;                 /* pointer of merge dict index key     */
   char usr_indexkey[PHONETIC_LEN];    /* user dict index key buffer          */
   char *usr_indexkeyptr;              /* pointer of user_indexkey            */
   char usr_drkey[PHONETIC_LEN];       /* user dict group key buffer          */
   char *usr_drkeyptr;                 /* pointer of user dict group key      */
   char merge_indexkey[PHONETIC_LEN];  /* merge dict index key buffer         */
   char *merge_drkeyptr;               /* pointer of merge dict group key     */
   char *merge_indexkeyptr;            /* pointer of merge_indexkey           */
   char merge_drkey[PHONETIC_LEN];     /* merge dict group key buffer         */
   char group_buf[DR_MAX_LENGTH];      /* group buffer                        */
   char *gptr;                         /* pointer of group_buf                */
   char *chptr;                        /* pointer of DR                       */
   char *movptr;                       /* break point of DR to move           */
   char *wordptr;                      /* pointer of word                     */
   char word[PHRASE_LEN];              /* word buffer                         */
   int usr_drptr;                      /* pointer to group of user dict       */
   int merge_drptr;                    /* pointer to group of merge dict      */
   int ptr;                            /* point to the position of word       */
   int usr_length;                     /* the length of group of user dict    */
   int merge_length;                   /* the length of group of merge dict   */
   int counter;                        /* loop counter                        */

   if ((rc=lock_dict(obj->usrdict->fd)) != OK) return(rc);
   /* create a temp file for writing process data */
   if ( access(obj->tempdict->name,EXIST) != ERROR)
      unlink(obj->tempdict->name);     /* delete template dict                */
   if ((rc=create_dict(obj->tempdict)) != OK)
      return(rc);                      /* can not create temp dict            */
   if ((rc = lock_dict(obj->tempdict->fd)) != OK) return(rc);
   if ((rc= open_dict(obj->mergedict)) != OK) return(rc);
   if ((rc =lock_dict(obj->mergedict->fd)) != OK) return(rc);
   if (((rc = load_INDEX(obj->usrdict)) != OK) ||
       ((rc = load_INDEX(obj->mergedict)) != OK))
      return(rc);                      /* some error occurred                 */
   usr_indexptr =0;                    /* point to the beginning of MI        */
   /* get first index key of user dict */
   strcpy(usr_indexkey,
          usr_indexkeyptr=get_next_index(obj->usrdict->mi, &usr_indexptr));
   if (strncmp(usr_indexkey ,(char *)&miend,miend_len) != 0)
   {
      if (search_indexkey(obj->usrdict, usr_indexkey) != EQUAL)
         return(ERR_IC);               /* data of dictionary was inconsistent */
      if ((rc = load_DR(obj->usrdict)) != OK)
         return(rc);                   /* some error occurred                 */
      /* get the first group key of DR */
      usr_drptr = 0;                   /* point to the beginning of DR        */
      /* get the first group key of DR */
      strcpy(usr_drkey,
             usr_drkeyptr = get_next_groupkey(obj->usrdict->dr, &usr_drptr));
   }
   else
      (void)memcpy(usr_drkey, (char *)&drend,drend_len);/* set end mark */
   merge_indexptr = 0;                 /* pointer to the beginning of MI      */
   /* get the first group key of DR */
   strcpy(merge_indexkey,
       merge_indexkeyptr=get_next_index(obj->mergedict->mi,&merge_indexptr));
   if (strncmp(merge_indexkey,(char *)&miend, miend_len) != 0)
   {
      if (search_indexkey(obj->mergedict, merge_indexkey)!= EQUAL)
         return(ERR_IC);               /* data of dictionary was inconsistent */
      if ((rc = load_DR(obj->mergedict)) != OK)
         return(rc);                   /* some error occurred                 */
      /* get the first group key of DR */
      merge_drptr = 0;                 /* pointer to the beginning of DR      */
      /* get first group key of DR */
      strcpy(merge_drkey,
            merge_drkeyptr=get_next_groupkey(obj->mergedict->dr,&merge_drptr));
   }
   else
      (void) memcpy(merge_drkey, (char *)&drend, drend_len);/*set end mark */

   while ((strncmp(usr_indexkey ,(char *)&miend,miend_len) !=  0) &&
          (strncmp(merge_indexkey ,(char *)&miend,miend_len) != 0))
   {
      (void) memset(group_buf, NULL, DR_MAX_LENGTH);  /* set group_buf empty  */
      gptr = group_buf;                /* point to group_buf                  */
      if (key_compare(usr_drkey , merge_drkey) == LESS)
      {
         /* usr_drkey is less than merge_drkey ,so  add the group of */
         /* user dict with usr_drkey to template dict */
         if (search_groupkey(obj->usrdict, usr_drkey) != EQUAL)
            return(ERR_IC);            /* usr_drkey is not exist in user dict */
         /* copy merge_Drkey's group to buffer */
         chptr = obj->usrdict->dr + obj->usrdict->groupkey_ptr+
                 obj->usrdict->groupkey_len;
         for ( ;strncmp(chptr,(char *)&gpend,gpend_len)!=0;
                             *gptr=*chptr,chptr++,gptr++);
         /* Add the group of user dict with usr_drkey to template dict */
         if ((rc = AddGroup(obj, group_buf,usr_drkey)) != OK)
            return(rc);                /* some error occurred                 */
         /* get next group key */
         strcpy(usr_drkey,
                usr_drkeyptr=get_next_groupkey(obj->usrdict->dr, &usr_drptr));
      }
      else if (key_compare(usr_drkey, merge_drkey) == LARGE)
      {
         /*  usr_drkey is large than merge_drkey , so add the group of */
         /*  merge dict with merge_drkey to template dict */
         if (search_groupkey(obj->mergedict, merge_drkey) != EQUAL)
            return(ERR_IC);         /* merge_drkey is not exist in merge dict */
         /* copy merge_Drkey's group to buffer */
         chptr = obj->mergedict->dr + obj->mergedict->groupkey_ptr+
                 obj->mergedict->groupkey_len;
         for ( ;strncmp(chptr,(char *)&gpend,gpend_len)!=0;
                                 *gptr=*chptr,chptr++,gptr++);
         /* Add the group of merge dict with merge_drkey to template dict */
         if ((rc = AddGroup(obj, group_buf, merge_drkey)) != OK)
            return(rc);                /* some error occurred                 */
         /* get next group  key */
         strcpy(merge_drkey,
             merge_drkeyptr=get_next_groupkey(obj->mergedict->dr,&merge_drptr));
      }
      else /* usr_drkey is equal to merge_drkey */
      {
         /*******************************************************/
         /* filter the different words of usrdict and mergedict */
         /*******************************************************/
         if (search_groupkey(obj->mergedict, merge_drkey) != EQUAL)
            return(ERR_IC);       /* merge_drkey is not exist in merge dict   */
         /* copy merge_Drkey's group to buffer */
         chptr = obj->mergedict->dr + obj->mergedict->groupkey_ptr+
                 obj->mergedict->groupkey_len;
         for (;strncmp(chptr,&gpend,gpend_len)!=0;*gptr=*chptr,chptr++,gptr++);

         /* filter the words with usr_drkey of user dict from group_buf */
         /*  (contain words with merge_drkey of merge dict)             */
         if (search_groupkey(obj->usrdict,usr_drkey) != EQUAL)
            return(ERR_IC);            /* usr_drkey is not exist in user dict */
         ptr = obj->usrdict->groupkey_ptr + obj->usrdict->groupkey_len;
         usr_length =  0;
         while (LOOP)
         {
            wordptr = get_next_word(obj->usrdict->dr, &ptr);
            if (strncmp(wordptr, (char *)&gpend, gpend_len) == 0)
            {
                /* find length of words which with merge_drkey  in merge dict */
                /* These words was different from words in usr dict */
                gptr = group_buf;
                for (merge_length=0;*gptr!=NULL;gptr++,merge_length++) ;
                break;
            }
            strcpy(word, wordptr);
            usr_length += strlen(word);
            filter_word(group_buf, word);        /* filter word from group_buf*/
       }
       if ((usr_length+merge_length+strlen(usr_drkey)+drend_len)<=DR_MAX_LENGTH)
       {
     /* concatenate the different words of merge dict to the end of user dict */
           chptr = group_buf + usr_length + merge_length-1;
           movptr = group_buf + merge_length-1;
           for(counter=merge_length;counter>0;*(chptr--)=*(movptr--),counter--);
           chptr = group_buf;
           movptr = obj->usrdict->dr+obj->usrdict->groupkey_ptr+
                     obj->usrdict->groupkey_len;
           for (;strncmp(movptr,(char *)&gpend,gpend_len)!=0;
                    *chptr = *movptr ,chptr++,movptr++);
       /* Add the group of merge dict and the group of user dict to temp dict */
            if ((rc = AddGroup(obj,group_buf,usr_drkey)) != OK)
               return(rc);             /* some error occurred                 */
           /* get next group key of user dict */
            strcpy(usr_drkey,
                   usr_drkeyptr=get_next_groupkey(obj->usrdict->dr,&usr_drptr));
          /* get next group key of merge dict */
            strcpy(merge_drkey,
             merge_drkeyptr=get_next_groupkey(obj->mergedict->dr,&merge_drptr));
         }
         else
            return(ERR_GF);            /* group was full                      */
      }
      if ((strncmp(usr_drkey, (char *)&drend,drend_len) == 0) &&
          (strncmp(usr_indexkey,(char *) &miend,miend_len) != 0))
      {
         /* get next index key of user dict and load the associate DR */
         strcpy(usr_indexkey,
            usr_indexkeyptr=get_next_index(obj->usrdict->mi, &usr_indexptr));
         if (strncmp(usr_indexkey,(char *) &miend,miend_len) != 0)
         {
           if (search_indexkey(obj->usrdict,usr_indexkey) != EQUAL)
               return(ERR_IC);         /* data of dictionary was inconsistent */
            if ((rc = load_DR(obj->usrdict)) != OK)
               return(rc);             /* some error occurred                 */
            usr_drptr = 0;             /* point to the beginning of DR        */
            /* get the first group key of DR */
            strcpy(usr_drkey,
               usr_drkeyptr=get_next_groupkey(obj->usrdict->dr, &usr_drptr));
         }
      }
      if ((strncmp(merge_drkey,(char *) &drend, drend_len) == 0) &&
          (strncmp(merge_indexkey,(char *) &miend, miend_len) !=0))
      {
        /* get next index key of merge dict and load the associate DR */
         strcpy(merge_indexkey,
          merge_indexkeyptr=get_next_index(obj->mergedict->mi,&merge_indexptr));
         if (strncmp(merge_indexkey,(char *) &miend, miend_len) != 0)
         {
            if (search_indexkey(obj->mergedict,merge_indexkey) != EQUAL)
               return(ERR_IC);         /* data of dictionary was inconsistent */
            if ((rc = load_DR(obj->mergedict)) != OK)
               return(rc);             /* some error occurred                 */
            merge_drptr = 0 ;          /* point to the beginning of DR        */
            /* get the first group key of DR */
            strcpy(merge_drkey,
             merge_drkeyptr=get_next_groupkey(obj->mergedict->dr,&merge_drptr));
         }
      }
   }
   rc=OK;
   if (strncmp(usr_indexkey,(char *)&miend, miend_len) != 0)
      rc =AddAnotherGroup(obj, USR, usr_drkey,usr_indexkey);
   else if (strncmp(merge_indexkey, (char *)&miend, miend_len) != 0)
      rc =AddAnotherGroup(obj, MERGE,merge_drkey,merge_indexkey);

   if (rc==OK)
      return(replace(obj));            /* replace user dict with temp dict    */
   else
      return(rc);                      /* some error occurred                 */
}


/******************************************************************************/
/*FUNCTION    : word_exist_in_dict                                            */
/*DESCRIPTION : check (phonetic, phrase) exist in dictionary                  */
/*INPUT       : dict = the dictionary to find this word                       */
/*              phonetic = the phonetic code of this word                     */
/*              phrase  = the code of this word                               */
/*OUTPUT      : TRUE = (phonetic, phrase) exist in dictionary                 */
/*              FALSE = (phonetic, phrase) do not exist in dictionary         */
/******************************************************************************/
int word_exist_in_dict(dict, phonetic,phrase)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
{
   return(search_word(dict, phonetic, phrase));
}


/******************************************************************************/
/*FUNCTION    : phonetic_key_exist_in_DR                                      */
/*DESCRIPTION : check phonetic code exist in current DR                       */
/*INPUT       : dict =  dictionary of this phonetic code                      */
/*              phonetic = phonetic code to find in this dictionary           */
/*OUTPUT      : TRUE = phonetic code exist in DR                              */
/*              FALSE = phonetic code do not exist in DR                      */
/******************************************************************************/
int phonetic_key_exist_in_DR(dict, phonetic)
DictInfo *dict;                        /* dictionary control block            */
char *phonetic;                        /* low bytes of phonetic code          */
{
   if ((rc=search_key(dict, phonetic)) == EQUAL)
      return(TRUE);                    /* phonetic code exist in dict         */
   else if ((rc==LARGE) || (rc==LESS))
      return(FALSE);                   /* phonetic code does not exist in dict*/
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/*FUNCTION    : search_key                                                    */
/*DESCRIPTION : search key_value exist in DR of this dictionary               */
/*INPUT       : dict = the  to search this key_value dictionary               */
/*              key_value = the key value                                     */
/*OUTPUT      : TRUE = key value exist in DR of this dictionary               */
/*              FALSE = key value do not exist in DR of this dictionary       */
/******************************************************************************/
int search_key(dict, phonetic)
DictInfo  *dict;                       /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
{
   if (((rc = search_indexkey(dict, phonetic)) == LARGE)|| (rc == LESS) ||
          (rc==EQUAL))
   {
      /* if currently active DR is not the DR exist key value ,*/
      /* then load another DR to search key_value */
      if (dict->indexkey_rrn == 0)
         return(LARGE);                /* dictionary is empty                 */
      if (dict->indexkey_rrn != dict->rrn)
      {
         dict->rrn = dict->indexkey_rrn;
         if ((rc =load_DR(dict)) != OK)
            return(rc);                /* some error occurred                 */
      }
      return(search_groupkey(dict, phonetic));
   }
   else
      return (rc);                     /* some error occurred                 */
}


/******************************************************************************/
/*FUNCTION    : search_indexkey                                               */
/*DESCRIPTION : search the key value in the main index of dictionary          */
/*INPUT       : dict = the dictionary to search key_value                     */
/*              key_value = the key value to search in main index             */
/*OUTPUT      : LARGE = the key_value is large than any key in main index     */
/*              EQUAL = here is one index key in main index equal to key_value*/
/*              LESS  = the key value is less than one key in main index      */
/******************************************************************************/
int search_indexkey(dict, phonetic)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
{
    int ptr;                           /* point to the position of MI         */
    char *index_key;                   /* point to the index key              */

    if ((rc = dict_change(dict)) != OK)
       return(rc);                     /* some error occurred                 */
    if (dict->indexkey_ptr >0)
    {
       if (key_compare(phonetic, dict->indexkey) == EQUAL)
          return(EQUAL);               /* find phonetic code in MI            */
       else if (key_compare(phonetic, dict->indexkey) == LARGE)
          ptr = dict->indexkey_ptr;    /* point to the position of index key  */
       else
          ptr = 0;                     /* point to the beginning of MI        */
    }
    else
       ptr = 0;                        /*  point to the beginning of MI       */
    dict->indexkey_ptr = ptr;          /* start position to search            */
    index_key = get_next_index(dict->mi, &ptr);  /* get next index key        */
    strcpy(dict->indexkey ,index_key); /* store index key                     */
    while  (key_compare(phonetic , dict->indexkey) == LARGE)
    {
       dict->indexkey_ptr = ptr;       /* point to the position of index key  */
       strcpy(dict->indexkey,index_key = get_next_index(dict->mi, &ptr));
    }
    if  ( strncmp(index_key, (char *)&miend, miend_len) == 0)
    {
       dict->indexkey_len = 0;         /* default length of index key         */
       dict->indexkey_rrn = dict->rrn=0; /* indicate DR does not exist        */
       return(LARGE);       /* phonetic code is large than all indexkey of MI */
    }
    dict->indexkey_len = strlen(dict->indexkey); /* length of index key       */
    ptr = dict->indexkey_ptr;               /* the start position of index key*/
    /* get the rrn of Dr with dict->indexkey, then load DR */
    dict->indexkey_rrn = get_next_rrn(dict->mi, &ptr);     /* get rrn value   */
    if (dict->rrn != dict->indexkey_rrn)
    {
       /* currently active DR does not contain the input phonetic code */
       /* so get another DR which rrn is dict->indexkey_rrn            */
       dict->rrn = dict->indexkey_rrn;                /* update dict->rrn     */
       if ((rc=load_DR(dict)) != OK)                  /* load another DR      */
          return(rc);                                 /* some error occurred  */
    }
    return(key_compare(phonetic, index_key));         /* compare keys         */
}

/******************************************************************************/
/*FUNCTION    : search_groupkey                                               */
/*DESCRIPTION : search group key in dictionary                                */
/*INPUT       : dict = the dictionary to search group key                     */
/*              key_value = the group key to search in DR of dictionary       */
/*OUTPUT      : LARGE = the key_value is large than any key in DR             */
/*              EQUAL = there is one group key in DR equal to key_value       */
/*              LESS  = the key value is less than one key in DR              */
/******************************************************************************/
int search_groupkey(dict,key_value)
DictInfo *dict;                        /* pointer to dictionary control block */
char *key_value;                       /* pointer to the key value            */
{
   int ptr;                            /* pointer of DR                       */
   int prev_ptr;                       /* pointer of DR                       */
   char *group_key;                    /* pointer of group key                */

   if ((rc = dict_change(dict)) != OK) /* Had dict been modified              */
      return(rc);                      /* some error occurred                 */
   else
   {
      if (is_error(rc=search_indexkey(dict,key_value)))
         return(rc);                   /* some error occurred                 */
      if (dict->indexkey_rrn == 0)
         return(LARGE);                /*key_value is large than all index key*/
      if (dict->indexkey_rrn != dict->rrn)
      {
       /* currently active DR does not contain the input phonetic code */
       /* so get another DR which rrn is dict->indexkey_rrn            */
         dict->rrn = dict->indexkey_rrn;         /* updat rrn                 */
         if ((rc=load_DR(dict)) != OK)           /* load another DR           */
            return(rc);                          /* some error occurred       */
      }
   }
   /* find a position begin to search group key from DR */
   if (dict->groupkey_ptr > 0)
   {
      if (key_compare(dict->groupkey, key_value) == EQUAL)
         return(EQUAL);                /* find key_value in DR                */
      else if (key_compare(key_value, dict->groupkey ) == LARGE)
         ptr = dict->groupkey_ptr;
      else
         ptr = 0;                      /* point to beginning of DR            */
   }
   else
      ptr = 0;                         /* point to the beginning of DR        */

   /* search DR to find key_value */
   prev_ptr = ptr;                     /* start position to search group key  */
 while (key_compare(group_key=get_next_groupkey(dict->dr,&ptr),key_value)==LESS)
   {
      if (strncmp(group_key, &drend, drend_len) == 0)
      {
         dict->groupkey_ptr = prev_ptr;
         dict->groupkey_len = 0;       /* default length of end mark of DR    */
         strcpy(dict->groupkey, "");   /* default group key                   */
         return(LARGE);       /* key_value is large than all group key in DR  */
      }
      prev_ptr = ptr;                  /* store the position of group key     */
   }
   dict->groupkey_ptr = prev_ptr;      /* position of group key in DR         */
   dict->groupkey_len = strlen(group_key);       /* length of group key       */
   strcpy(dict->groupkey, group_key);            /* update group key          */
   return(key_compare(key_value, group_key));    /* compare key               */
}


/******************************************************************************/
/*FUNCTION    : search_word                                                   */
/*DESCRIPTION : search (phonetic, phrase) exist in dictionary or not          */
/*INPUT       : dict = the dictionary to search word                          */
/*              phonetic = the phonetic code of word                          */
/*              phrase = the phrase code of word                              */
/*OUTPUT      : TRUE = find the (phonetic,phrase) in this dictionary          */
/*              FALSE = do not find the (phonetic, phrase) in this dictionary */
/******************************************************************************/
int search_word(dict, phonetic,phrase)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
{
   int ptr;                            /* pointer to the position of word     */
   int prev_ptr;                       /* pointer to the position of word     */
   char *word;                         /* pointer of word                     */

   if (((rc=search_indexkey(dict, phonetic)) ==LESS) ||(rc == EQUAL))
   {
      if ((rc=search_groupkey(dict, phonetic)) == EQUAL)
      {
         ptr=dict->groupkey_ptr+dict->groupkey_len; /* start position of words*/
         prev_ptr = ptr;               /* store the word's position           */
         word = get_next_word(dict->dr, &ptr);   /* get next word             */
         /* find word in group which with phonetic code */
         while ( key_compare(phrase, word) !=EQUAL)
         {
            if (strncmp(word,(char *)&gpend, gpend_len) == 0)
               break;
            prev_ptr = ptr;            /* store the word's position           */
            word = get_next_word(dict->dr ,&ptr);     /* get next word        */
         }
         if ( key_compare(phrase, word) == EQUAL)
         {
            dict->word_ptr = prev_ptr;           /* set word's position       */
            return (TRUE);             /* word exist in dict                  */
         }
         else return(FALSE);           /* word does not exist in dict         */
      }
      else if (is_error(rc)) return(rc); /* some error occurred               */
      else return(FALSE);              /* word is not exist in dict           */
   }
   else if (rc== LARGE) return(FALSE); /* word is not exist in dict           */
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/*FUNCTION    : replace_index_key_with_group_key                              */
/*DESCRIPTION : replace index_key with group key in MI                        */
/*INPUT       : dict = the dictionary                                         */
/*              group_key = the new index key                                 */
/*OUTPUT      : TRUE = replace index key with group key                       */
/*              FALSE = the space of MI will be empty                         */
/******************************************************************************/
int replace_index_key_with_group_key(dict, max_group_key, group_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *max_group_key;                   /* pointer to maximum group key        */
char *group_key;                       /* pointer to group key                */
{
   char *chptr;                        /* pointer of MI                       */
   char *delptr;                       /* pointer of max_group_key            */
   int length;         /* the different length of max_group_key and group_key */

   if ((rc = search_indexkey(dict,group_key)) != EQUAL)
   {
      if (is_error(rc)) return(rc);    /* some error occurred                 */
      else return(EX_IN_NON);          /*PHONETIC/PHRASE is not in user/system*/
   }
   if ( dict->indexkey_len >= strlen(max_group_key))
   {
      delptr = dict->mi + dict->indexkey_ptr; /* point to the index key in MI */
      chptr = max_group_key;           /* point to max_group_key              */
      length = strlen(dict->indexkey) - strlen(max_group_key);
      /* replace  group_key with max_group_key */
      for ( ; *chptr != NULL;  *delptr = *chptr, chptr++, delptr++);
      *(delptr-1) &=0x7f;              /* set the 7th bit of last byte to off */
      chptr = dict->mi+dict->indexkey_ptr+dict->indexkey_len;
      if (chptr != delptr)
      {
         for ( ;(strncmp(chptr,(char*)&miend,miend_len)!= 0);
                    *delptr = *chptr , chptr++,delptr++);
         (void)memcpy(delptr, (char *)&miend, miend_len);
         delptr += miend_len;
         for ( ;length>0; *delptr = NULL, delptr++, length--) ;
     }
   }
   else                                /* LEN(group_key) > LEN(index_key)     */
   {
      length = strlen(max_group_key) - strlen(dict->indexkey);
      if (MI_has_enough_space(dict,length) == TRUE)
      {
         /* replace  group_key with max_group_key */
         delptr = dict->mi + dict->indexkey_ptr + dict->indexkey_len;
         for (chptr=delptr;strncmp(chptr,(char *)&miend,miend_len)!=0;chptr++);
         chptr += (miend_len+length);
         for ( ;chptr >= delptr;*chptr = *(chptr-length), chptr--);
         delptr = dict->mi + dict->indexkey_ptr;
         chptr =  max_group_key;
         for ( ; *chptr != NULL;  *delptr = *chptr, chptr++, delptr++);
         *(delptr-1) &= 0x7f;          /* set the 7th bit of last byte to off */
      }
      else
        return(ERR_MF);                /* MI was full                         */
   }
   dict->mi_freespace = dict->mi_freespace - strlen(dict->indexkey) +
                       strlen(dict->groupkey);
   dict->indexkey_len = strlen(max_group_key);
   strcpy(dict->indexkey, max_group_key);
   return(write_INDEX(dict));          /* write MI to storage                 */
}

/******************************************************************************/
/*FUNCTION    : create_new_DR                                                 */
/*DESCRIPTION : create a new DR and add one index key and rrn of this DR to MI*/
/*INPUT       : dict = the dictionary to add a new DR                         */
/*              group_key = the key of this DR                                */
/*OUTPUT      : OK = create a new DR                                          */
/*              ERROR = there is some error while create a new DR             */
/******************************************************************************/
int create_new_DR(dict,group_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *group_key;                       /* pointer to group key                */
{
   int length;                         /* the length of group_key            */

   length = strlen(group_key);
   if (MI_has_enough_space(dict,length)==TRUE)
      return( insert_new_index_key(dict,group_key));
   else
     return(ERR_MF);                   /* MI was full                         */
}


/******************************************************************************/
/*FUNCTION    : create_new_DR_insert_a_group                                  */
/*DESCRIPTION : create a new DR and insert a group to it                      */
/*INPUT       : dict = the dict to add a new DR                               */
/*              group_key = the group key of this group                       */
/*              phrase = the phrase code of this group                        */
/*OUTPUT      : OK = can create a new DR and add a group to it                */
/*              ERROR = there is some error occur                             */
/******************************************************************************/
int create_new_DR_insert_a_group(dict,group_key,phrase)
DictInfo *dict;                        /* pointer to dictionary control block */
char *group_key;                       /* key of phrase code                  */
char *phrase;                          /* pointer to phrase code              */
{
   char *drptr;                        /* pointer of DR                       */
   char *chptr;                        /* pointer of group_key or phrase      */

   if ((rc =create_new_DR(dict, group_key)) == OK)
   {
       /* insert group_key at the beginning of DR */
       drptr = dict->dr;               /* point to DR                         */
       chptr = group_key;              /* point to group_key                  */
       /* insert group_key to DR */
       for ( ; *chptr != NULL;*drptr = *chptr, chptr++, drptr++);
       *(drptr-1) &= 0x7f;             /* set the 7th bit of last byte to off */

       /* insert phrase to DR after group_key */
       chptr = phrase;                 /* point to phrase                     */
       /* insert phrase to DR */
       for ( ; *chptr != NULL;*drptr = *chptr ,chptr++, drptr++);
       *(drptr-1) &= 0x7f;             /* set the 7th bit of last byte to off */

       /* insert End mark of DR */
       memcpy(drptr, (char *) &drend, drend_len);     /* insert end mark      */
       return(write_DR(dict));         /* write DR to storage                 */
   }
   else
      return(rc);                      /* some error occurred                 */
}


/******************************************************************************/
/*FUNCTION    : create_new_DR_insert_more_group                               */
/*DESCRIPTION : create a new DR and insert more groups next to move_stsrt_pos */
/*INPUT       : dict = the dictionary                                         */
/*              move_start_pos = the start position of group to move          */
/*OUTPUT      : OK = can create a new DR and add some groups to it            */
/*              ERROR = there is some error occur                             */
/******************************************************************************/
int create_new_DR_insert_more_group(dict, move_number)
DictInfo *dict;                        /* pointer to dictionary control block */
int move_number ;                 /* number of DR bytes to move to another DR */
{
   int ptr;                            /* point to the position of group key  */
   char *gkeyptr;                      /* pointer to group key                */
   char maxgkey[PHONETIC_LEN];         /* maximum group key buffer            */
   char *drptr;                        /* pointer to group_buf                */
   char *movptr;           /* move pointer of DR to move groups to another DR */
   char *rightptr;                /* pointer to group of DR as the right side */
   char *leftptr;                 /* pointer to group of DR as the left side  */
   char group_buf[DR_MAX_LENGTH];      /* group buffer                        */
   int cnt;                            /* counter                             */

   (void) memset(group_buf, NULL, DR_MAX_LENGTH);     /* set group_buf empty  */
   (void) memset(maxgkey,NULL, PHONETIC_LEN);         /* set maxgkey empty    */

   /* move move_number bytes of DR to group_buf */
   movptr = dict->dr;                  /* point to DR                         */
   drptr = group_buf;                  /* point to group_buf                  */
   for (cnt = 0; cnt < move_number; *(drptr++) = *(movptr++), cnt++);
   (void) memcpy(drptr, (char *)&drend, drend_len);   /* insert end mark      */

   /* search group_buf to find the maximum group key of group_buf */
   ptr = 0;                            /* point to the beginning of group_buf */
   drptr = group_buf;                  /* point to group_buf                  */
   while (strncmp(gkeyptr = get_next_groupkey(drptr, &ptr) ,
                   &drend, drend_len) != 0)
   {
       strcpy(maxgkey, gkeyptr);       /* store group key                     */
   }
   if (strncmp(maxgkey, &gpend, gpend_len) == 0)
      return(EX_IN_NON);               /* PHONETIC/PHRASE is not in dict      */
   if ((rc =create_new_DR(dict, maxgkey)) != OK)
      return(rc);                      /* some error occurred                 */
   else
   {
      /* copy group_buf to DR as the content of DR */
      (void) memcpy(dict->dr, group_buf, DR_MAX_LENGTH);
      return(write_DR(dict));          /* write DR to storage                 */
   }
}

/******************************************************************************/
/*FUNCTION    : Split_DR_into_2parts                                          */
/*DESCRIPTION : splite this DR into 2 parts                                   */
/*INPUT       : dict = the dictionary of DR to splite                         */
/*              group_key = the DR would include this group key               */
/*OUTPUT      : OK =  splite this DR into 2 parts is OK                       */
/*              ERROR = there is some error occur                             */
/******************************************************************************/
int split_DR_into_2parts(dict, group_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *group_key;                       /* pointer to group_key                */
{
    char *drptr;                       /* pointer to dr_buf                   */
    char *chptr;                       /* pointer to DR                       */
    char *groupkey;                    /* pointer to group key                */
    char *mdptr ;                      /* point to the middle of DR           */
    char *movptr ;                     /* indicate where to move to another DR*/
    char maxgkey[PHONETIC_LEN];        /* maximum group key buffer            */
    int drlen;                         /* length of DR                        */
    int middle;                        /* middle pointer of DR                */
    char dr_buf[DR_MAX_LENGTH];        /* DR buffer                           */
    char *rightptr;               /* pointer to group of DR as the right side */
    char *leftptr;                /* pointer to group of DR as the left side  */
    int move_number;              /* number of DR bytes to move to another DR */
    int cnt;                           /* counter                             */
    int ptr;                           /* point to the position of group key  */

    (void) memset(dr_buf, NULL, DR_MAX_LENGTH);  /* set dr_buf to be empty    */
    (void) memset(maxgkey,NULL, PHONETIC_LEN);   /* set maxgkey to be empty   */

    if (is_error(rc=search_key(dict, group_key)))
       return(rc);                     /* some error occurred                 */
    chptr = dict->dr;
    for (drlen=0;strncmp(chptr,&drend,drend_len)!=0;chptr++,drlen++);
    middle = (drlen+drend_len) /2;
    mdptr =  dict->dr + middle;        /* point to middle pointer of DR       */
    /* find right side */
    for (rightptr = mdptr;strncmp(rightptr, &gpend,gpend_len)!=0 ; rightptr++);
    /* find left side */
    for (leftptr=mdptr; (strncmp(leftptr, &gpend,gpend_len)!=0) &&
                                 (leftptr >=dict->dr) ;leftptr--);
    /* determine movptr , movptr is one of rightptr or leftptr */
    if (strncmp(rightptr, (char *)&drend, drend_len) != 0)
    {
        if (leftptr != dict->dr)
           leftptr+= gpend_len;        /* point to start position of one group*/
        else
           return(ERR_GF);             /* group was full                      */
        rightptr += gpend_len;         /* point to start position of one group*/
        movptr = ((mdptr -leftptr) > (rightptr - mdptr)) ? rightptr :  leftptr;
    }
    else if (leftptr != dict->dr)
         movptr = leftptr + gpend_len; /* point to start position of group    */
    else
         return(ERR_GF);               /* group was full                      */

    /* copy the content of DR from movptr to end of DR to dr_buf */
    for (drptr = movptr, chptr = dr_buf ; strncmp(drptr,&drend,drend_len)!=0;
                           *chptr = *drptr, drptr++, chptr++);
    (void) memcpy(chptr, (char *)&drend,drend_len);   /* insert end mark      */
    move_number = movptr - dict->dr;   /*number of bytes to move to another DR*/
    if ((rc = create_new_DR_insert_more_group(dict, move_number-gpend_len))!=OK)
       return(rc);                     /* some error occurred                 */

    ptr = 0;                           /* point to beginning                  */
    drptr = dr_buf;                    /* point to dr_buf                     */
    while  (strncmp(groupkey = get_next_groupkey(drptr, &ptr) ,
                                  (char *) &drend,  drend_len) != 0)
    {
         strcpy(maxgkey, groupkey);    /* store group key                     */
    }
    if ((rc = search_key(dict, maxgkey)) == EQUAL)
    {
       (void) memcpy(dict->dr, dr_buf,DR_MAX_LENGTH); /* copy dr_buf to DR    */
       return(write_DR(dict));         /* write DR to storage                 */
    }
    else if (is_error(rc)) return(rc); /* some error occurred                 */
    else return(EX_IN_NON);            /* PHONETIC/PHRASE is not in dict      */
}


/******************************************************************************/
/*FUNCTION    : insert_phrase_into_DR                                         */
/*DESCRIPTION : insert phrase code as nth word of group which has phonetic code*/
/*INPUT       : dict = the dictionary to insert phrase                        */
/*              phonetic = the phonetic code of this word                     */
/*              phrase = the phrase code of this word                         */
/*              nth = the insert position of this word                        */
/*OUTPUT      : NONE                                                          */
/******************************************************************************/
int insert_phrase_into_DR(dict,phonetic,phrase,nth)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
char *phrase;                          /* pointer to phrase code              */
int nth;                               /* the insert position of this word    */
{
   int count;                          /* word number counter                 */
   char *word;                         /* point to word                       */
   char *drptr;                        /* pointer to DR                       */
   char *movptr;                       /* point to DR                         */
   char *group_key;                    /* point to group key                  */
   int ptr;                            /* point to the position of word       */
   int prev_ptr;                       /* point to the position of word       */
   int length;                         /* length of phrase                    */

   if ((rc = phonetic_key_exist_in_DR(dict, phonetic))==TRUE)
   {
      /* find position to insert phonetic and phrase */
      if (is_error(rc=search_groupkey(dict, phonetic))) return(rc);

      /* to find the position of nth word with phonetic code */
      count =1;                        /* word number counter initial value   */
      ptr=dict->groupkey_ptr+dict->groupkey_len;/*point to beginning of words */
      /* find nth word's position */
      while (count <  nth)
      {
         count ++;                     /* increase counter by 1               */
         word = get_next_word(dict->dr, &ptr);        /* get next word        */
         if (strncmp(word , (char *)&gpend, gpend_len) == 0)
            break;                     /* at the end of group                 */
      }
      length = strlen(phrase);         /* length of phrase                    */
      drptr =  dict->dr + ptr;         /*point to the position of the nth word*/
      for (movptr=drptr; strncmp(movptr, (char*)&drend,drend_len)!=0;movptr++);
      movptr +=drend_len;
   /* shift all words after ptr position of this DR right phrase length bytes*/
      for ( ; drptr <= movptr;*(movptr+length) = *movptr , movptr--);
    /*  insert phrase after ptr position */
      movptr = phrase; /* point to phrase */
      for ( ; *movptr != NULL;*drptr = *movptr, drptr++, movptr++);
      *(drptr-1) &= 0x7f;/* set the 7th bit of last byte to off */
      return(write_DR(dict));          /* write DR to storage                 */
   }
   else if (rc==FALSE)                 /* the key of this phonetic is nonexist*/
   {
      if ( dict->indexkey_rrn == 0)
         return(create_new_DR_insert_a_group(dict, phonetic, phrase));
      /* find position to insert phonetic and phrase */
      ptr = 0;                         /* start position of DR                */
      prev_ptr = ptr;                  /* store the position                  */

      while (key_compare(group_key=get_next_groupkey(dict->dr,&ptr),
                        phonetic)==LESS)
      {
         if ((strncmp(group_key, (char *)&gpend, gpend_len) == 0) ||
             (strncmp(group_key, (char *)&drend, drend_len) == 0))
            break;                     /* end of DR or end of group           */
         prev_ptr = ptr;               /* store the position                  */
      }
      length = strlen(phonetic) + strlen(phrase) +gpend_len; /* insert length */
   /*   shift all words after prev_ptr position of this DR right phrase */
      drptr = dict->dr + prev_ptr;      /* insert position                    */
      for (movptr=drptr; strncmp(movptr,(char *)&drend,drend_len)!=0;movptr++);
      movptr += drend_len;
      for ( ; drptr <= movptr;*(movptr+length) = *movptr, movptr--);
      movptr = phonetic;               /* point to phonetic code              */
    /* insert phonetic after ptr position */
      for ( ; *movptr != NULL;*drptr = *movptr, drptr++, movptr++);
      *(drptr-1)  &= 0x7f;             /* set the 7th bit of last byte to off */
    /*  insert phrase to DR */
      movptr = phrase;                 /* point to phrase                     */
      for ( ; *movptr != NULL;*drptr = *movptr, drptr++, movptr++);
     *(drptr-1) &= 0x7f;               /* set the 7th bit of last byte to off */
     (void) memcpy(drptr, (char *)&gpend, gpend_len);   /* insert GP_END_MARK */
     return(write_DR(dict));           /* write DR to storage                 */
   }
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/*FUNCTION    : insert_new_index_key                                          */
/*DESCRIPTION : add a new index key and rrn to main index                     */
/*INPUT       : dict = the dictionary to add a new index key                  */
/*              key_value = the new index key                                 */
/*OUTPUT      : OK = add a new index key to the main index                    */
/*              ERROR = there is some error occur                             */
/******************************************************************************/
int insert_new_index_key(dict, key_value)
DictInfo *dict;                        /* pointer to dictionary control block */
char *key_value;                       /* pointer to the new index key        */
{
   int length;                         /* length of key_value and RRN         */
   char *miptr;                        /* pointer to MI                       */
   char *chptr;                        /* pointer to MI                       */
   unsigned char *rrnptr;              /* point to rrn                        */
   long int new_rrn;                   /* value of variable rrn               */
   int cnt;                            /* loop counter                        */

   length = strlen(key_value) + RRN_LENGTH;      /* length bytes to insert    */
   if (MI_has_enough_space(dict,length)==TRUE)   /* Has MI enough space ?     */
   {
    /*search index, and find the position in order to insert_group key */
      if (is_error(rc=search_indexkey(dict,key_value))) return(rc);
      if ((new_rrn = determine_RRN(dict->mi)) < 0)   /* find unused rrn value */
         return(ERR_GF);               /* group was full                      */

      /* shift MI right length bytes after miptr */
      miptr = dict->mi+ dict->indexkey_ptr; /* point to index key position    */
      for (chptr=miptr; strncmp(chptr, (char*)&miend,miend_len)!=0;chptr++);
      chptr += miend_len;
      for ( ; chptr >= miptr;*(chptr+length) = *chptr, chptr--);

      /* insert key to mi table */
      chptr = key_value;               /* point to key_value                  */
      for ( ; *chptr != NULL; *miptr = *chptr,chptr++, miptr++);
      *(miptr-1) &= 0x7f;              /* set the 7th bit of last byte to off */

      /* insert rrn to MI */
      rrnptr = (unsigned char *)&new_rrn;
      for (cnt=0;cnt<RRN_LENGTH; *miptr = *(rrnptr+cnt), miptr++,cnt++);

      dict->mi_freespace -= length;    /* update mi_freespace                 */
      dict->indexkey_rrn = new_rrn;    /* update rrn value                    */
      dict->rrn = new_rrn;             /* update rrn value                    */
      strcpy( dict->indexkey, key_value);        /* set index key             */
      strcpy(dict->groupkey, "");      /* set group key                       */
      return(write_INDEX(dict));       /* write MI to storage                 */
   }
   else
      return(ERR_MF);                  /* group was full                      */
}

/******************************************************************************/
/*FUNCTION    : determine_RRN                                                 */
/*DESCRIPTION : to find a valid rrn number                                    */
/*INPUT       : dict = the dictionary to find a unused rrn                    */
/*OUTPUT      : rrn  = a valid value of rrn                                   */
/******************************************************************************/
int determine_RRN(mi)
char *mi;                              /* pointer to the Master Index         */
{
   int ptr;                            /* point to MI                         */
   int count;                          /* rrn number counter                  */
   int *rrn_node;                      /* point to rrn node buffer            */
   int i, j;                           /* loop counter                        */
   int unused_rrn;                     /* unused rrn                          */
   int temp_rrn;                       /* template rrn value                  */
   int rrn;                            /* rrn value                           */
   char change_flag;                   /* sorting flag                        */

   ptr = 0;                            /* point to the beginning of MI        */
   count = -1;                         /* counter of rrn (start with 0 )      */
  /* find the total number of DR */
   while (get_next_rrn(mi,&ptr) > 0)
       count++;                        /* increase by 1                       */

   if (count < 0)                      /* dictionary is empty                 */
      return(MI_MAX_LENGTH);           /* position of first DR                */
   else
   {
     rrn_node=(int *)XtMalloc((count+1)*sizeof(long int));/*allocate rrn buf*/
   }
   ptr =0;                             /* start to the beginning of MI        */
   count = -1;                         /* counter of rrn (start with 0 )      */
   /* reset the rrn value to be 0, 1, 2, 3, ... */
   while ((rrn = get_next_rrn(mi, &ptr)) > 0)
      *(rrn_node+(++count)) = (rrn-MI_MAX_LENGTH) / DR_MAX_LENGTH;
   /* sort rrn node */
   for (i=0; i<=count-1; i++)
   {
      change_flag = 0;                 /* sort changed flag                   */
      for (j= count; j>=i+1; j--)
      {
         if (*(rrn_node+j) < *(rrn_node+j-1))
         {
            /* swap (rrn_node+j) and (rrn_node+j-1) */
            temp_rrn = *(rrn_node+j);
            *(rrn_node+j) = *(rrn_node+j-1);
            *(rrn_node+j-1) = temp_rrn;
            change_flag = 1;           /* changed                             */
         }
      }
      if (! change_flag)
         break;                        /* already in order                    */
   }
   /* find  an unused rrn */
   unused_rrn = 0;                     /* initial value is the first one DR  */
   for (i=0; i<= count; i++)
   {
      if (*(rrn_node+i) != unused_rrn)
         break;
      else
         unused_rrn++;                 /* next rrn                            */
   }
   free(rrn_node);                     /* free rrn node                       */
   return(unused_rrn *DR_MAX_LENGTH+ MI_MAX_LENGTH);  /* an unused rrn        */
}

/******************************************************************************/
/* FUNCTION    : get_next_rrn                                                 */
/* DESCRIPTION : get rrn from Master Index                                    */
/* INPUT       : mi                                                           */
/*               ptr                                                          */
/* OUTPUT      : rrn                                                          */
/******************************************************************************/
int get_next_rrn(mi, ptr)
char *mi;                              /* pointer to the Master Index         */
int *ptr;                              /* point to the position of MI         */
{
   int rrn;                            /* the value of rrn                    */
   int count;                          /* loop counter                        */

   /* skip index key */
   while (LOOP)
   {
       if (strncmp(mi+(*ptr), (char *)&miend, miend_len) == 0)
          return(-1);                  /* End mark of MI                      */
       else if (*(mi+(*ptr)) & 0x80)
          (*ptr)++;                    /* skip to next byte                   */
       else
          break;                       /* End of index key                    */
   }
   /* get the rrn value , the rrn occupy RRN_LENGTH bytes */
   for (count = 0; count < RRN_LENGTH; count++)
   {
       rrn <<= 8;                      /* shift left 8 bits                   */
       rrn |= *(mi+(++(*ptr)));
   }
   (*ptr)++;                           /* skip to next byte                   */
   return(rrn);                        /* value of rrn                        */
}


/******************************************************************************/
/*FUNCTION    : key_compare                                                   */
/*DESCRIPTION : compare key1 and key2                                         */
/*INPUT       : key1                                                          */
/*              key2                                                          */
/*OUTPUT      : LARGE = key1 is larger than key2                              */
/*              EQUAL = key1 is equal to key2                                 */
/*              LESS = key1 is less than key2                                 */
/******************************************************************************/
int key_compare(key1, key2)
char *key1;                            /* pointer to key1                     */
char *key2;                            /* pointer to key2                     */
{
   if ((rc = strcmp(key1,key2)) > 0)
      return(LARGE);                   /* key1 is large than key2             */
   else if (rc < 0)
      return(LESS);                    /* key1 is less than key2              */
   else
      return(EQUAL);                   /* key1 is equal to key2               */
}


/******************************************************************************/
/*FUNCTION    : delete_group                                                  */
/*DESCRIPTION : delete the group which has the group key                      */
/*INPUT       : dict = the dictionary which wnats to delete group             */
/*              group_key = the group key of group                            */
/*OUTPUT      : OK = delete group                                             */
/*              ERROR = there is some error occured when to delete group      */
/******************************************************************************/
int delete_group(dict, group_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *group_key;           /* pointer to the group key which would be deleted */
{
   char *drptr;                        /* pointer to DR                       */
   char max_group_key[PHONETIC_LEN];   /* maximum group key of this DR        */
   char *chptr;                        /* pointer to DR                       */
   int next_ptr;                       /* point to the position of group      */

   if ((rc = phonetic_key_exist_in_DR(dict, group_key)) == TRUE)
   {
      if ((rc = group_number_greater_1(dict,group_key)) == TRUE)
      {
         if (is_error(rc=search_groupkey(dict, group_key))) return(rc);
         if ((rc=key_is_largest_of_DR(dict,group_key)) == TRUE)
         {
            /* write end mark of DR in the position group key */
            drptr = dict->dr + dict->groupkey_ptr -gpend_len;
            (void) memcpy(drptr, (char *)&drend, drend_len);
            drptr += drend_len ;
            for ( ; *drptr != NULL; *drptr = NULL, drptr++);

            initial_DR(dict);
            strcpy(max_group_key, drptr = largest_group_key(dict->dr));
            if ((rc=replace_index_key_with_group_key(dict,
                          max_group_key, group_key)) == OK)
               return(write_DR(dict)); /* write DR to storage                 */
            else
               return(rc);             /* some error occurred                 */
         }
         else if (rc==FALSE)           /* group_key is not largest key of DR  */
         {
            /* delete group with group_key from DR */
            chptr = dict->dr + dict->groupkey_ptr;    /* point to this group  */
            next_ptr = dict->groupkey_ptr;  /*point to start position of group*/
            get_next_groupkey(dict->dr, &next_ptr);/*get next group's position*/
            drptr = dict->dr + next_ptr;    /* point to next group            */
            for ( ; *drptr != NULL; *chptr = *drptr, chptr++, drptr++);
            for ( ; *chptr !=NULL; *chptr =NULL,chptr++);/*set NULL in the end*/
            return(write_DR(dict));    /* write DR to storage                 */
         }
         else return(rc);              /* some error occurred                 */
       }
       else if (rc == FALSE)           /* DR only has one group               */
         return(delete_index_key(dict,group_key));    /* delete index key     */
       else
         return(rc);                   /* some error occurred                 */
   }
   else if (rc==FALSE)
       return(EX_IN_NON);              /* PHONETIC/PHRASE is not in dict      */
   else return(rc);                    /* some error occurred                 */
}

/******************************************************************************/
/*FUNCTION    : key_is_largest_of_DR                                          */
/*DESCRIPTION : check the group_key whether is the largest key of this DR     */
/*INPUT       : dict  = the dictionary                                        */
/*              key_value = the group key of this group                       */
/*OUTPUT      : TRUE = the group_key is the largest key of this DR            */
/*            : FALSE = the group_key is not the largest key of this DR       */
/******************************************************************************/
int key_is_largest_of_DR(dict,key_value)
DictInfo *dict;                        /* pointer to dictionary control block */
char *key_value;                       /* pointer to phonetic code            */
{
   int ptr;                            /* point to the position of group      */
   char *groupkey;                     /* pointer to group key                */

   if ((rc=search_key(dict, key_value)) == EQUAL)
   {
      ptr = 0;                         /* pointer to beginning of DR          */
      while (strncmp(groupkey = get_next_groupkey(dict->dr,&ptr),
                    &drend, drend_len) != 0)
      {
         if (key_compare(groupkey,key_value) == LARGE)
            return(FALSE);             /* key_value is not largest key of DR  */
      }
      return(TRUE);                    /* key_value is largest key of DR      */
   }
   else if (is_error(rc)) return(rc);  /* some error occurred                 */
   else return(FALSE);                 /* key_value does not exist            */
}

/******************************************************************************/
/*FUNCTION    : delete_index_key                                              */
/*DESCRIPTION : delete the index key from main index of dictionary            */
/*INPUT       : dict = the dictionary which wants to delete a index key       */
/*              index_key = the index key wants to delete                     */
/*OUTPUT      : NONE                                                          */
/******************************************************************************/
int delete_index_key(dict, index_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *index_key;                       /* pointer to index key                */
{
   char *miptr;                        /* pointer to MI                       */
   char *drptr;                        /* pointer to DR                       */
   char *movptr;                       /* pointer to MI                       */
   int length;                         /* length of index_key and RRN         */
   long int rrn;                       /* value of RRN                        */

   if ((rc = search_indexkey(dict, index_key)) != EQUAL)
   {
      if (is_error(rc)) return(rc);    /* some error occurred                 */
      else return(EX_IN_NON);          /* index_key does not exist in dict    */
   }
   else                                /* find index_key in MI                */
   {
      /* delete index key , and shift all next index keys left */
      dict->rrn = dict->indexkey_rrn;
      miptr = dict->mi + dict->indexkey_ptr;     /* point to index key        */
      length = dict->indexkey_len + RRN_LENGTH;  /* length bytes to delete    */
      movptr = miptr + length;         /* point to next next index key        */
      for ( ; strncmp(movptr, (char*) &miend,  miend_len) != 0;
                             *miptr=*movptr, miptr++, movptr++);
      (void) memcpy(miptr, (char *)&miend, miend_len);     /* add end mark    */
      miptr+=miend_len;                /* point to the end of MI              */
      for ( ;miptr < movptr+miend_len; *(miptr++)=NULL);/*set NULL in the end */
      dict->mi_freespace +=  length;   /* update mi_freespace                 */
      strcpy(dict->indexkey, "");
      return(write_INDEX(dict));       /* write MI to storage                 */

      /* If you want to clear the DR  then include the following codes, */
      /* but now, we would not do it */
/*
      (void) memset(dict->dr, NULL, DR_MAX_LENGTH);
      (void) memcpy(dict->dr, (char *)&drend, drend_len);
      return(write_DR(dict));
*/
   }
}


/******************************************************************************/
/*FUNCTION    : largest_group_key                                             */
/*DESCRIPTION : find the largest group key of DR                              */
/*INPUT       : dict = the dictionary                                         */
/*              group_key = the group key which included in a DR              */
/*OUTPUT      : max_group_key = the largest group key of DR                   */
/******************************************************************************/
char * largest_group_key(dr)
char *dr;                              /* pointer to DR                       */
{
   char max_groupkey[PHONETIC_LEN];    /* maximum group key buffer            */
   int ptr;                            /* point to the position of group      */
   char *key_value;                    /* point to group key                  */

   (void) memset(max_groupkey, NULL, PHONETIC_LEN);/* set NULL to max_groupkey*/
   (void) memcpy(max_groupkey, (char *)&gpend, gpend_len); /* Add end mark    */
   ptr =0;                             /* point to beginning of dr            */
   while (strncmp(key_value=get_next_groupkey(dr, &ptr),
                   (char *)&drend, drend_len) != 0)
   {
      strcpy(max_groupkey, key_value);  /* store group key                    */
   }
   return(max_groupkey);               /* return maximum group key of DR      */
}

/******************************************************************************/
/*FUNCTION    : word_number_greater_1                                         */
/*DESCRIPTION : find the number of word which phonetic code is key_value      */
/*INPUT       : dict = the dictionary                                         */
/*              key_value = the phonetic code of word                         */
/*OUTPUT      : TRUE = the word number of key_value is greater than 1         */
/*            : FALSE = the word number of key_value is not greater than 1    */
/******************************************************************************/
int word_number_greater_1(dict, key_value)
DictInfo *dict;                        /* pointer to dictionary control block */
char *key_value;                       /* point to key_value                  */
{
   int ptr;                            /* point to the position of word       */
   char *word;                         /* pointer to code of word             */

   if ((rc = phonetic_key_exist_in_DR(dict, key_value)) == TRUE)
   {
      if (is_error(rc=search_groupkey(dict, key_value))) return(rc) ;
      ptr = dict->groupkey_ptr + dict->groupkey_len; /* first word's position */
      if (strncmp(word = get_next_word(dict->dr, &ptr),
                    &gpend, gpend_len) != 0)
      {
         if (strncmp(word=get_next_word(dict->dr,&ptr),
                    &gpend, gpend_len) != 0)
            return(TRUE);              /* there are more than one word        */
      }
   }
   else if (is_error(rc)) return(rc);  /* some error occurred                 */

   return (FALSE);           /*the word with key_value is not greater than one*/
}

/******************************************************************************/
/* FUNCTION    : word_number_of_group                                         */
/* DESCRIPTION : the word number of group which with key_value                */
/* INPUT       : dict                                                         */
/*               key_value                                                    */
/* OUTPUT      : number                                                       */
/******************************************************************************/
int word_number_of_group(dict, key_value)
DictInfo *dict;                        /* pointer to dictionary control block */
char *key_value;                       /* pointer to key value                */
{
   int ptr;                            /* point to the position of word       */
   char *word;                         /* pointer to word                     */
   int number = 0;                     /* number of word which with key_value */

   if ((rc = phonetic_key_exist_in_DR(dict, key_value)) == TRUE)
   {
      if (search_groupkey(dict, key_value) != EQUAL) return(0);
      ptr = dict->groupkey_ptr + dict->groupkey_len;   /*first word's position*/
      while (strncmp(word = get_next_word(dict->dr, &ptr),
              &gpend, gpend_len) != 0)
          number++;                    /* increase number by 1                */
   }
   return(number);                     /* number of words                     */
}

/******************************************************************************/
/*FUNCTION    : get_next_word                                                 */
/*DESCRIPTION : get next word of this group                                   */
/*INPUT       : dict = the dictionary                                         */
/*              ptr = the start position                                      */
/*OUTPUT      : word = the word of group                                      */
/*NOTE        : the value of ptr will be changed and must be return           */
/******************************************************************************/
char *get_next_word(dr, ptr)
char *dr;                              /* pointer to DR                       */
int *ptr;                              /* point to the position of word       */
{
   static char word[PHRASE_LEN];       /* word buffer                         */
   char *drptr;                        /* pointer to dr                       */
   int count = 0;                      /* counter                             */

   (void) memset(word, NULL, PHRASE_LEN);        /* clear word buffer         */
   drptr = dr;                         /* point to the beginning of DR        */
   if ((*(drptr+*ptr)!=NULL) && strncmp((drptr +*ptr), &gpend, gpend_len) != 0)
   {
      while ((*ptr < DR_MAX_LENGTH) && (*(drptr+(*ptr)) & 0x80) &&
             (strncmp((drptr+*ptr),(char *) &gpend,gpend_len) !=0))
         word[count++] = *(drptr+(*ptr)++);      /* add next byte to word     */
     word[count++]=*(drptr+(*ptr)++)|0x80;/*set the 7th bit of last byte to on*/
   }
   else
   {
       (void) memcpy(word, (char*)&gpend, gpend_len); /* set end mark of group*/
   }
   return (word);                      /* return word                         */
}

/******************************************************************************/
/*FUNCTION    : AddGroup                                                      */
/*DESCRIPTION : add the group of dict has group_key to tempdict               */
/*INPUT       : dict = the dictionary                                         */
/*              group_key = the group key of the group which want to be copy  */
/*                          to tempdict                                       */
/*              length = may be the maximum length of group                   */
/*OUTPUT      : OK =  can add the group to tempdict                           */
/*              ERROR = some error occur                                      */
/******************************************************************************/
int AddGroup(obj, group_buf, group_key)
UDEobject *obj;                         /* UDE Control Block                  */
char *group_buf;                        /* pointer to group buffer            */
char *group_key;                        /* pointer to group key               */
{
   int length;                          /* the length of group_buf            */
   char *chptr;                         /* point to group buffer              */
   char *movptr;
   int cnt;                             /* loop counter                       */
   char *drptr;                         /* pointer to DR                      */
   char max_group_key[PHONETIC_LEN];    /* maximum group key buffer           */

   /* find the length of group_buf */
   chptr = group_buf;               /* point to the beginning of group buffer */
   for(length=0;*chptr!=NULL;length++,chptr++);  /*   find length of group_buf*/
   length += strlen(group_key);         /* the length of group to insert to DR*/

  if(is_error(rc=search_key(obj->tempdict,group_key)))
      return(rc);                       /* some error occurred                */
   if ((obj->tempdict->dr_freespace < length) ||
        ( obj->tempdict->dr_freespace >= DR_MAX_LENGTH-drend_len))
   {
      if ((rc= create_new_DR(obj->tempdict, group_key)) == OK)
      {
        /* copy group_key, group_buf and end mark of Dr to DR */
         chptr = obj->tempdict->dr;    /* point to beginning of DR            */
         movptr = group_key;           /* point to group_key                  */
         for ( ; *movptr != NULL; *chptr=*movptr,chptr++, movptr++);
         *(chptr-1) &= 0x7f;           /* set the 7th bit of last byte to off */
         movptr = group_buf;           /* point to beginning of group_buf     */
         for ( ; *movptr != NULL; *chptr = *movptr,chptr++, movptr++);
         (void) memcpy(chptr, (char*)&drend, drend_len);   /* add end mark    */
      }
      else return(rc);                 /* some error occurred                 */
   }
   else                              /* DR has enough space for add group_buf */
   {
      /* Insert group_key and group_buf as the last group of this DR */
      strcpy(max_group_key, drptr = largest_group_key(obj->tempdict->dr));
      /* find the end of this DR */
      chptr = obj->tempdict->dr;       /* point to the beginning of DR        */
      while (strncmp(chptr,(char *) &drend,drend_len) !=0) chptr++;
      (void) memcpy(chptr, (char *)&gpend, gpend_len);     /* Add end mark    */
      chptr += gpend_len;              /* skip end mark of group              */
      movptr = group_key;              /* point to group_key                  */
      for( ;*movptr!=NULL;*chptr=*movptr,chptr++,movptr++);/*insert group key */
      *(chptr-1) &= 0x7f;              /* set the 7th bit of last byte to off */
      movptr = group_buf;              /* point to group_buf                  */
      for ( ;*movptr!=NULL; *chptr=*movptr,chptr++,movptr++);/* insert phrase */
      (void) memcpy(chptr,(char *) &drend, drend_len);  /* Add end mark of DR */
      (void) memcpy(group_buf, obj->tempdict->dr, DR_MAX_LENGTH);

      if ((rc=replace_index_key_with_group_key(obj->tempdict,
                 group_key, max_group_key))!= OK)
        return(rc);                    /* some error occurred                 */
      (void) memcpy(obj->tempdict->dr, group_buf, DR_MAX_LENGTH);
   }
   return(write_DR(obj->tempdict));    /* write DR to storage                 */
}

/******************************************************************************/
/* FUNCTION    : write_DR                                                     */
/* DESCRIPTION : write DR to storage                                          */
/* INPUT       : dict                                                         */
/* OUTPUT      :                                                              */
/******************************************************************************/
int write_DR(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   if (lseek(dict->fd, dict->rrn, SEEK_SET) == ERROR)
      return(ERR_WR);                  /* Read/Write Error                    */
   if (write(dict->fd, dict->dr, DR_MAX_LENGTH) == ERROR)
      return(ERR_WR);                  /* Read/Write Error                    */
   initial_DR(dict);
   return(OK);                         /* write DR to storage OK              */
}

/******************************************************************************/
/* FUNCTION    : write_INDEX                                                  */
/* DESCRIPTION : write Master Index to storage                                */
/* INPUT       : dict                                                         */
/* OUTPUT      :                                                              */
/******************************************************************************/
int write_INDEX(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   if (access(dict->name,EXIST) == ERROR)
      return(ERR_WR);
   else
   {
      if (lseek(dict->fd, 0L, SEEK_SET) == ERROR)
         return(ERR_WR);               /* Read/Write Error                    */
      if ( write(dict->fd, dict->mi, MI_MAX_LENGTH) == -1)
         return(ERR_WR);               /* Write Dictionary file Error         */
   }
   reset_modified_time(dict);          /* reset dict modified time            */
   return(OK);                         /* write MI to storage OK              */
}

/******************************************************************************/
/* FUNCTION    : reset_modified_time                                          */
/* DESCRIPTION : call stat system call to get the last modified time of dict  */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
void reset_modified_time(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   extern char     *ctime();           /* Get Modified Time                   */

   stat(dict->name,  &state);          /* get status of dict                  */
   modtime = ctime(&state.st_mtime);   /* get last modified time              */
   strcpy(dict->last_modified_time, modtime );   /* set modified time         */
}

/******************************************************************************/
/*FUNCTION    : AddAnotherGroup                                               */
/*DESCRIPTION : add all the groups of dict which index key are greater than   */
/*                                        or equal to index_key to tempdict   */
/*INPUT       : dict = the dictionary which groups want to add to tempdict    */
/*              index_key = index key of dict                                 */
/*OUTPUT      : OK =  can add the groups to tempdict                          */
/*              ERROR = some error occur                                      */
/******************************************************************************/
int AddAnotherGroup(obj, dict_flag, dr_key ,index_key)
UDEobject *obj ;                       /* UDE Control Block                   */
char dict_flag;                        /* indicator of dict                   */
char *dr_key;                          /* pointer to group key                */
char *index_key;                       /* pointer to index key                */
{
   DictInfo *dict;                     /* pointer to dict control block       */
   char group_buf[DR_MAX_LENGTH];      /* group buffer                        */
   int drptr;                          /* pointer to DR                       */
   int indexkeyptr;                    /* point to index key                  */
   char *drkeyptr;                     /* point to group key                  */
   char drkey[PHONETIC_LEN];           /* group key buffer                    */
   char *gptr;                         /* pointer to group_buf                */
   char *chptr;                        /* point to DR                         */
   char *indexptr;                     /* point to index key                  */

   /* check the dictionary is user dict or merge dict */
   if (dict_flag == USR)
      dict = obj->usrdict;             /* point to user dict                  */
   else if (dict_flag == MERGE)
      dict = obj->mergedict;           /* point to merge dict                 */
   else
      return(ERROR);                   /* invalid dictionary                  */

   if (strncmp(index_key, (char *)&drend, drend_len) == 0)
      return(OK);                      /* merge finish                        */
   if ((rc = search_indexkey(dict, index_key)) != EQUAL)
   {
      if (is_error(rc)) return(rc);    /* some error occurred                 */
      else return(EX_IN_NON);          /* PHONETIC/PHRASE is not in dict      */
   }
   if (strncmp(dr_key, (char *)&drend,drend_len) != 0)
   {
      if ((rc = search_groupkey(dict, dr_key)) != EQUAL)
      {
          if (is_error(rc)) return(rc);/* some error occurred                 */
          else return(EX_IN_NON);      /* PHONETIC/PHRASE is not in dict      */
      }
      indexkeyptr = dict->indexkey_ptr;
      indexptr=get_next_index(dict->mi,&indexkeyptr);
      strcpy(index_key,indexptr=get_next_index(dict->mi,&indexkeyptr));
      drptr = dict->groupkey_ptr;
      strcpy(dr_key,drkeyptr=get_next_groupkey(dict->dr, &drptr));
      while (strncmp(dr_key, (char *)&drend,drend_len) != 0)
      {
         /* copy Drkey's group to buffer */
         if ((rc = search_groupkey(dict, dr_key)) != EQUAL)
         {
            if (is_error(rc)) return(rc);        /* some error occurred       */
            else return(EX_IN_NON);    /* PHONETIC/PHRASE is not in dict      */
         }
         gptr = group_buf;
         (void) memset(group_buf, NULL, DR_MAX_LENGTH);
         chptr = dict->dr +dict->groupkey_ptr+dict->groupkey_len;
         for ( ;strncmp(chptr,&gpend,gpend_len)!=0;*gptr=*chptr,chptr++,gptr++);
         if ((rc = AddGroup(obj, group_buf,dr_key)) != OK)
            return(rc);                /* some error occurred                 */
         strcpy(dr_key,drkeyptr=get_next_groupkey(dict->dr, &drptr));
      }
   }

   while (strncmp(index_key,(char *)&miend, miend_len)!= 0)
   {
      if (search_indexkey(dict, index_key) != EQUAL)
      {
         if (is_error(rc)) return(rc); /* some error occurred                 */
         else return(EX_IN_NON);       /* PHONETIC/PHRASE is not in dict      */
      }
      if ((rc = load_DR(dict)) != OK)
         return(rc);                   /* some error occurred                 */
      drptr = 0;                       /* point to beginning of DR            */
      while (strncmp(drkeyptr=get_next_groupkey(dict->dr,&drptr),
                        (char *) &drend, drend_len) != 0)
      {
         strcpy(drkey, drkeyptr);      /* store group key                     */
         if ((rc=search_groupkey(dict, drkey)) != EQUAL)
         {
             if (is_error(rc)) return(rc);      /* some error occurred        */
             else return(EX_IN_NON);   /* PHONETIC/PHRASE is not in dict      */
         }
         /* copy all words of group of DR with drkey to group_buf */
         gptr = group_buf;             /* point to group_buf                  */
         (void) memset(group_buf, NULL, DR_MAX_LENGTH);    /* clear group_buf */
        chptr=dict->dr+dict->groupkey_ptr+dict->groupkey_len;/*word's position*/
         for ( ;strncmp(chptr,(char *)&gpend,gpend_len)!=0;*gptr=*chptr,
                     chptr++, gptr++);           /* copy words to group_buf   */
         if ((rc = AddGroup(obj, group_buf, drkey)) != OK)
            return(rc);                /* some error occurred                 */
      }
      strcpy(index_key,indexptr=get_next_index(dict->mi,&indexkeyptr));
   }
   return(OK);                         /* merge finish                        */
}

/******************************************************************************/
/*FUNCTION    : load_INDEX                                                    */
/*DESCRIPTION : load index of dictionary to memory                            */
/*INPUT       : dict = the dictionary to load main index                      */
/*OUTPUT      : NONE                                                          */
/*NOTE        : set the index_update_flag to FALSE                            */
/******************************************************************************/
int load_INDEX(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   int bytesread;

   if (dict->fd >= 0)
   {
      if (lseek(dict->fd, 0L, SEEK_SET) == ERROR) return(ERR_WR);
      if ((bytesread=read(dict->fd,dict->mi, MI_MAX_LENGTH)) == ERROR)
      {
         stat(dict->name, &state);     /* get status of dict                  */
         if (state.st_size >= MI_MAX_LENGTH)
            return(ERR_AP);            /* Dictionary Access not permitted     */
         else
            return(ERR_IF);            /* Invalid dictionary format           */
      }
      if (bytesread != MI_MAX_LENGTH) return(ERR_IF);
      reset_dict(dict);
      return(OK);                      /* load MI OK                          */
   }
   else
      return(open_dict(dict));         /* open dictionary                     */
}

/******************************************************************************/
/*FUNCTION    : load_DR                                                       */
/*DESCRIPTION : load DR from dictionary which offset is calculate by rrn      */
/*INPUT       : dict = the dictionary to load DR                              */
/*              rrn = the offset of DR                                        */
/*OUTPUT      : NONE                                                          */
/*NOTE        : set DR_update_flag to FALSE                                   */
/******************************************************************************/
int load_DR(dict)
DictInfo *dict;                        /* dictionary control block            */
{
   int bytesread;

      if (dict->fd >= 0)
      {
         if (lseek(dict->fd, dict->rrn , SEEK_SET) == ERROR)
            return(ERR_WR);            /* Read/Write Error                    */
         if (read(dict->fd, dict->dr,DR_MAX_LENGTH) == ERROR)
         {
             return(ERR_WR);           /* Read/Write Error                    */
         }
         initial_DR(dict);
         return(OK);                   /* load DR OK                          */
      }
      else
         return(open_dict(dict));      /* open dictionary                     */
}

/******************************************************************************/
/* FUNCTION    : create_dict                                                  */
/* DESCRIPTION : Create dictionary                                            */
/* EXTERNAL REFERENCES:                                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int create_dict(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   unsigned char *drptr;               /* point to DR                         */
   int stream;                         /* file description                    */

   if (strlen(dict->name) == 0) return(ERR_IV);  /*dictionary name is invalid */
   if (access(dict->name,EXIST) == ERROR)
                                        /*open the dictionary for read & write*/
      stream =open(dict->name,O_RDWR | O_CREAT,S_IREAD | S_IWRITE | O_SYNC );
   else
   {
       if ((stream = open(dict->name, dict->flag)) >=0)
       {
          if ((stream != dict->fd) &&(dict->fd >= 0)) close(dict->fd);
          dict->fd = stream;
          return(load_INDEX(dict));    /* load MI                             */
       }
   }
   if (stream <0) return(ERR_AP);      /* Dictionary Access not permitted     */
   if ((stream!=dict->fd) && (dict->fd >=0)) close(dict->fd);
   dict->fd = stream;
   dict->rrn = MI_MAX_LENGTH ;         /* first DR's position                 */
   /* copy end mark of MI to the beginning of MI */
   (void) memset(dict->mi, NULL, MI_MAX_LENGTH); /* clear MI                  */
   (void)memcpy(dict->mi,(char *)&miend, miend_len);       /* Add end mark    */
   /* copyd end mark of DR to the beginning of DR */
   (void) memset(dict->dr, NULL, DR_MAX_LENGTH);           /* clear DR        */
   (void)memcpy(dict->dr,(char*)&drend, drend_len);        /* Add end mark    */
   /* write MI and DR to dictionary */
   if (((rc= write_INDEX(dict)) != OK)||((rc=write_DR(dict))!=OK))
      return(rc);                      /* some error occurred                 */
   else
   {
      dict->mi_freespace = MI_MAX_LENGTH - miend_len;      /* free space of MI*/
      dict->dr_freespace = DR_MAX_LENGTH - drend_len;      /* free space of DR*/
      return(OK);                      /* create dictionary OK                */
   }
}

/******************************************************************************/
/*FUNCTION    : reset_dict                                                    */
/*DESCRIPTION : set some initial condition of dictionary                      */
/*INPUT       : dict = the dictionary to set some initial condition           */
/*OUTPUT      : NONE                                                          */
/******************************************************************************/
void reset_dict(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   char *ctime();                      /* get modified time                   */
   int num;                            /* number of MI                        */
   char *miptr;                        /* pointer to MI                       */

   dict->rrn = 0;                      /* reset RRN to 0                      */
   (void) memcpy(dict->dr, (char *)&drend, drend_len);     /* Add end mark    */
   dict->dr_freespace = DR_MAX_LENGTH -drend_len;
   dict->groupkey_ptr = 0;
   dict->groupkey_len = 0;
   strcpy(dict->groupkey,"");
   for (num=0,miptr=dict->mi;(strncmp(miptr,(char *)&miend,miend_len)!=0) &&
                         (num < MI_MAX_LENGTH); miptr++,num++);
   dict->mi_freespace = MI_MAX_LENGTH - num - miend_len;
   dict->indexkey_ptr = 0;
   dict->indexkey_len = 0;
   strcpy(dict->indexkey, "");
   reset_modified_time(dict);          /* reset last modified time of dict    */
}

/******************************************************************************/
/*FUNCTION    : initial_DR                                                    */
/*DESCRIPTION : set some initial condition of this DR                         */
/*INPUT       : dict = the dictionary to set some initial condition of DR     */
/*OUTPUT      : NONE                                                          */
/******************************************************************************/
int initial_DR(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   int ptr;                            /* point to DR                         */
   char *group_key;                    /* point to group key                  */

   ptr =0;                             /* point to beginning of DR            */
   strcpy(dict->groupkey, group_key=get_next_groupkey(dict->dr, &ptr));
   dict->groupkey_len = strlen(dict->groupkey);  /* first group key of DR     */
   dict->groupkey_ptr = 0;             /* beginning of DR                     */
   dict->dr_freespace = DR_MAX_LENGTH - length_of_DR(dict->dr);
   reset_modified_time(dict);          /* reset modified time                 */
}

/******************************************************************************/
/*FUNCTION    : length_of_DR                                                  */
/*DESCRIPTION : find the used space of this currently active DR               */
/*INPUT       : dict = the dictionary of this DR                              */
/*OUTPUT      :                                                               */
/******************************************************************************/
int length_of_DR(dr)
char *dr;                              /* pointer to DR                       */
{
   int length = 0;                     /* length of DR                        */
   char *drptr;                        /* pointer to dr                       */

   drptr = dr;                         /* point to the beginning of dr        */
   /* search dr to find the end mark of DR */
   while((*(drptr+length)!=NULL)&&(strncmp(drptr+length,&drend,drend_len) !=0))
         length++;                     /* increase by 1                       */
   if (strncmp(drptr+length, (char *)&drend, drend_len) == 0)
      length += drend_len;             /*increase the length of end mark of DR*/
   return(length);
}

/******************************************************************************/
/*FUNCTION    : group_has_enough_space                                        */
/*DESCRIPTION : check group of phonetic has available space for length bytes  */
/*INPUT       : dict = the dictionary                                         */
/*              phonetic = the phonetic code of this group                    */
/*              length = the available space must exist in this group         */
/*OUTPUT      : TRUE = group has length available bytes                       */
/*              FALSE = the available space is less than length bytes         */
/*NOTE        : if there is not any group its group key is phonetic code      */
/*              the default return value is TRUE                              */
/******************************************************************************/
int group_has_enough_space(dict, phonetic, length)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
int length;       /* length bytes to insert to group which with phonetic code */
{
   int count;                          /* counter of group                    */
   int ptr;                            /* point to DR                         */
   char *drptr;                        /* pointer to DR                       */

   if (((rc=search_groupkey(dict, phonetic))==LESS)||(rc==LARGE))
      return(TRUE);                    /* phonetic code does not exist in dict*/
   else if (rc != EQUAL)
       return(rc);                     /* some error occurred                 */
   ptr = 0;                            /* point to the beginning of DR        */
   count =0;
   drptr = dict->dr + dict->groupkey_ptr;   /* point to the beginning of group*/
   /* search group to find the end mark of group */
   while ((ptr < DR_MAX_LENGTH) &&
          (strncmp((drptr+ptr), (char *)&gpend, gpend_len) != 0))
   {
      ptr++;                           /* skip to next byte                   */
      count ++;                        /* increase count by 1                 */
   }
   if ((count+length+gpend_len) < DR_MAX_LENGTH )
      return(TRUE);                    /* group has enough space              */
   else
      return(FALSE);                   /* group has not enough space          */
}

/******************************************************************************/
/*FUNCTION    : MI_has_enough_space                                           */
/*DESCRIPTION : check main index of dict has enough space for insert new index*/
/*INPUT       : dict = the dictionary which wants to add a new index key      */
/*              number = the length of index key and rrn                      */
/*OUTPUT      : TRUE = main index has enough space                            */
/*              FALSE = main index has not enough space                       */
/******************************************************************************/
int MI_has_enough_space(dict,number)
DictInfo *dict;                        /* pointer to dictionary control block */
int number;                            /* number bytes to insert to MI        */
{
   if (dict->mi_freespace >= number)
      return (TRUE);                   /* MI has enough space                 */
   else
      return (FALSE);                  /* MI has not enough space             */
}

/******************************************************************************/
/*FUNCTION    : DR_has_enough_space                                           */
/*DESCRIPTION : Check DR has enough space for insert words or not             */
/*INPUT       : dict = the dictionary which wnats to add words                */
/*              number = the length of words                                  */
/*OUTPUT      : TRUE = the space of DR greater than length                    */
/*              FALSE = the space of DR less than length                      */
/******************************************************************************/
int DR_has_enough_space(dict, phonetic, number)
DictInfo *dict;                        /* pointer to dictionary control block */
char *phonetic;                        /* pointer to phonetic code            */
int number;                            /* number bytes to insert to DR        */
{
   if (is_error(rc=search_key(dict, phonetic)))
      return(rc);                      /* some error occurred                 */
   else if (rc == LARGE) return(TRUE); /* phonetic code was too large         */

   if (dict->dr_freespace >= number)
      return(TRUE);                    /* DR has enough space                 */
   else
      return (FALSE);                  /* DR has not enough space             */
}

/******************************************************************************/
/*FUNCTION    : group_number_greater_1                                        */
/*DESCRIPTION : check the group number of DR is greater than 1 or not         */
/*INPUT       : dict = the dictionary                                         */
/*              group_key = the group key which DR may be include             */
/*OUTPUT      : TRUE = the group number of DR is greater than 1               */
/*              FALSE = the group number of DR is 0 or 1                      */
/******************************************************************************/
int group_number_greater_1(dict, group_key)
DictInfo *dict;                        /* pointer to dictionary control block */
char *group_key;                       /* pointer to group key                */
{
   char *groupkey_ptr;                 /* pointer to group key                */
   int ptr;                            /* point to the DR                     */

   if (is_error(rc=search_key(dict, group_key)))
      return(rc);                      /* some error occurred                 */
   else if (rc==LARGE) return(FALSE);
   ptr = 0;                            /* point to the beginning of DR        */
   if (strncmp(groupkey_ptr = get_next_groupkey(dict->dr,&ptr),
                         &drend, drend_len) != 0)
   {
      if (strncmp(groupkey_ptr = get_next_groupkey(dict->dr, &ptr),
                          &drend, drend_len) !=0)
         return( TRUE );               /* group number of DR is greater than 1*/
   }
   return(FALSE);                 /* group number of DR is not greater than 1 */
}


/******************************************************************************/
/*FUNCTION    : get_next_index                                                */
/*DESCRIPTION : get next index key of MI                                      */
/*INPUT       : dict = the dictionary                                         */
/*              ptr = the start point of MI                                   */
/*OUTPUT      : index_key = the index key of MI                               */
/*NOTE        : the value of ptr must return                                  */
/******************************************************************************/
char * get_next_index(mi, ptr)
char *mi;                              /* pointer to Master Index             */
int *ptr;                              /* pointer to the position of MI       */
{
   static char index_key[PHONETIC_LEN];/* index key buffer                    */
   int count = 0;                      /* counter                             */
   char *miptr;                        /* pointer to MI                       */

   (void) memset(index_key, NULL, PHONETIC_LEN); /* clear index_key           */
   miptr = mi;                         /* point to the beginning of MI        */
   if ((*(miptr+*ptr)!= 0)&&(strncmp((miptr+(*ptr)),&miend, miend_len ) != 0))
   {
      while ((*ptr < MI_MAX_LENGTH) && (*(miptr+(*ptr)) & 0x80))
         index_key[count++] = *(miptr+(*ptr)++); /* get next byte             */
      /* set the 7th bit of last byte to on */
      index_key[count++] = *(miptr+(*ptr)++) | 0x80;
      *ptr += RRN_LENGTH;              /* skip rrn                            */
   }
   else
   {
      /* set the index_key to be end mark of MI */
      (void) memcpy(index_key,(char *)&miend, miend_len);
   }
   return ((char *) index_key);        /* return index key                    */
}

/******************************************************************************/
/*FUNCTION    : get_next_groupkey                                             */
/*DESCRIPTION : get next group key of DR                                      */
/*INPUT       : dict = the dictionary                                         */
/*              ptr = the start point to search                               */
/*OUTPUT      : group key = the group key of group                            */
/*NOTE        : the value of ptr must return                                  */
/******************************************************************************/
char * get_next_groupkey(dr, ptr)
char *dr;                              /* pointer to DR                       */
int *ptr;                              /* point to the position of group      */
{
   static char groupkey[PHONETIC_LEN]; /* group key buffer                    */
   char *drptr;                        /* point to DR                         */
   int count;                          /* counter                             */

   (void) memset(groupkey,NULL, PHONETIC_LEN);   /* set NULL to groupkey      */
   drptr = dr;                         /* point to the beginning of DR        */
   count = 0;                          /* beginning position of groupkey      */
   if ((*(drptr+*ptr) !='\0') && (strncmp((drptr+*ptr),&drend, drend_len) != 0))
   {
       while ((*ptr < DR_MAX_LENGTH) && (*(drptr+*ptr) & 0x80))
       {
          groupkey[count++] = *(drptr +(*ptr)++);/* get next byte             */
       }
      /* set the 7th bit of last byte to on */
       groupkey[count++] = *(drptr+(*ptr)++) | 0x80;
      /* skip all words of this group */
       while (((*ptr) < DR_MAX_LENGTH ) &&
              (strncmp((dr+(*ptr)),(char*)&gpend, gpend_len) != 0))
            (*ptr)++;                  /* skip next byte                      */
       if (strncmp((dr+(*ptr)), &drend, drend_len) != 0)
          (*ptr) += gpend_len;         /* skip end mark of group              */
   }
   else
   {
     /* set end mark to groupkey */
      (void)memcpy(groupkey,(char *)&drend, drend_len);
   }
   return(groupkey);                   /* return group key                    */
}

/******************************************************************************/
/*FUNCTION    : get_dict_cands_to_list_box                                    */
/*DESCRIPTION : get all words of phonetic code in dictionary to list box      */
/*INPUT       : dict = the dictionary to get candidates of phonetic code      */
/*              phonetic = the phonetic input code                            */
/*OUTPUT      : NONE                                                          */
/*NOTE        : if there are no any words of phonetic code in dictionary      */
/*              then the list box will be empty                               */
/******************************************************************************/
int get_dict_cands_to_list_box(obj, phonetic)
UDEobject *obj;                        /* UDE Control Block                   */
char *phonetic;                        /* pointer to phonetic code            */
{
   int ptr;                            /* point to the position of word       */
   char **cand_str;                    /* pointer to candidates string        */
   char *wordptr;                      /* pointer to the word                 */
   char word[PHRASE_LEN];              /* word buffer                         */

   /* free all space of candidates in list box */
   cand_str = obj->phrase_select->candidate;     /* point to candidates       */
   for(ptr=1;ptr <= obj->phrase_select->candnum; ptr++)
      free(*(cand_str)++);             /* free candidates                     */
   obj->phrase_select->candnum = 0;    /* set candidate's number to be 0      */
   if (strlen(phonetic) == 0)          /* phonetic code is empty              */
      return(OK);
   if (phonetic_key_exist_in_DR(obj->usrdict,phonetic) == TRUE)
   {
      /* Add all words with phonetic code to candidate buffer */
      if (is_error(rc=search_groupkey(obj->usrdict,phonetic))) return(rc);
      ptr = obj->usrdict->groupkey_ptr + obj->usrdict->groupkey_len;
      while (strncmp(wordptr = get_next_word(obj->usrdict->dr, &ptr),
                                   &gpend, gpend_len) != 0)
      {
         strcpy(word, wordptr);
         /* Add word to candidate buf */
         if ((rc=Add_word_to_candidates_buf(obj, word)) !=OK)
             return(rc);              /* Insufficient memory                  */
      }
      return(OK);
   }
   else return(EX_IN_NON);             /* PHONETIC/PHRASE is not in dict      */
}
/******************************************************************************/
/*FUNCTION    : Add_word_to_candidates_buf                                    */
/*DESCRIPTION : add candidate to the candidate list                           */
/*INPUT       : word                                                          */
/*OUTPUT      : NONE                                                          */
/******************************************************************************/
int Add_word_to_candidates_buf(obj, word)
UDEobject *obj;                        /* UDE Control Block                   */
char *word;                            /* pointer to word                     */
{
   static char ** cand_str;            /* pointer to candidates string        */
   int word_length;                    /* length of word                      */

   cand_str = obj->phrase_select->candidate;     /* point to candidate buffer */
   word_length = strlen(word) +1;      /* word buffer length                  */
   *(cand_str+obj->phrase_select->candnum) = XtMalloc(word_length+1);
   strcpy(*(cand_str+obj->phrase_select->candnum), word);
   obj->phrase_select->candnum++;      /* candidate number increase by 1      */
   return(OK);
}

/****************************************************************************/
/*FUNCTION    : check_phrase_str                                            */
/*DESCRIPTION : Check string whether it is OK or WRONG as phrase string.    */
/*INPUT       : phrase_buf = the string of phrase code which will be return */
/*OUTPUT      : TRUE = all the words of phrase_buf are valid phrase code    */
/*              FALSE = some error occur                                    */
/*NOTE        : the phrase code of phrase entry field will be copied to     */
/*              phrase_buf                                                  */
/****************************************************************************/
int check_phrase_str(phrase_code, phrase)
char *phrase_code;                     /* pointer to phrase code              */
char *phrase;                          /* pointer to phrase code              */
{
   phrase[0]=NULL;
   if (strlen(phrase_code) > 0)
   {
      strcpy(phrase, phrase_code);
      return(AnlPhrase(phrase));
   }
   else return(ERR_PHRASE_EMPTY);      /*Phonetic code or phrase code is error*/
}

/****************************************************************************/
/*FUNCTION    : check_phonetic_str                                          */
/*DESCRIPTION : Check string whether it is OK or WRONG as phonetic string.  */
/*INPUT       : phonetic_buf = the string of phonetic code which will be    */
/*                                                                   return */
/*OUTPUT      : TRUE = all the words of phonetic_buf are valid phonetic code*/
/*              FALSE = some error occur                                    */
/*NOTE        : the phonetic code of phonetic entry field will be copied to */
/*                                                             phonetic_buf */
/****************************************************************************/
int check_phonetic_str(phonetic_code,phonetic)
char * phonetic_code;                 /* pointer to phonetic code             */
char *phonetic;                       /* pointer to phonetic code             */
{
   phonetic[0]=NULL;
   if (strlen(phonetic_code) > 0)
   {
      strcpy(phonetic, phonetic_code);
      return(AnlPhonetic(phonetic));
   }
   else return(ERR_PHONETIC_EMPTY);    /*Phonetic code or phrase code is error*/
}

/****************************************************************************/
/*FUNCTION    : AnlPhonetic                                                 */
/*DESCRIPTION : to get the low byte of phonetic code                        */
/*INPUT       : Phonetic_buf                                                */
/*OUTPUT      : TRUE = All words of phonetic_buf are phonetic code          */
/*              FALSE = Not all words are phonetic code                     */
/****************************************************************************/
int AnlPhonetic(phonetic)
char *phonetic;                        /* pointer to phonetic code            */
{
   int ptr = 0;
   int ph_ptr = 0;

   /* check phonetic code and truncate the first byte of all phonetic codes*/
   while ( *(phonetic+ptr) != NULL)
   {
      /* 0xa5 is the first byte of all phonetic code */
      if ( *(phonetic+(ptr++)) == 0xa5)
      {
         /* second byte of phonetic code is between 0xc7 to 0xf0 */
         if ((*(phonetic+ptr) >= 0xc7) && (*(phonetic+ptr) <= 0xf0) &&
             (*(phonetic+ptr) != 0xed))
            *(phonetic+(ph_ptr++)) = *(phonetic+(ptr++));
         else
            return(ERR_PHONETIC);      /*Phonetic code or phrase code is error*/
      }
      else
         return(ERR_PHONETIC);         /*Phonetic code or phrase code is error*/
   }
   *(phonetic+ph_ptr) = '\0';          /* set end mark of string              */
   return(OK);
}

/******************************************************************************/
/*FUNCTION    : AnlPhrase                                                     */
/*DESCRIPTION : Check string whether it is OK or WRONG as phrase string       */
/*INPUT       : Phrase_buf                                                    */
/*OUTPUT      : TRUE = All words of phrase_buf are phrase code                */
/*              FALSE = Not all words are phrase code                         */
/******************************************************************************/
int AnlPhrase(phrase)
char * phrase;                        /* pointer to phrase code               */
{
   int ptr = 0;
   int number;

   /* the length of phrase code must be even */
   number = strlen(phrase);
   if (*(phrase+number-1) == 0x80)
      return(ERR_PHRASE);              /*Phonetic code or phrase code is error*/
   if ((number % 2) !=0)
      return(ERR_PHRASE);              /*Phonetic code or phrase code is error*/
   while (*(phrase+ptr))
   {
      /* all bytes of phrase code is greater than 0x80 */
      if ( ((*(phrase+ptr) & 0x80) == 0x00) || (*(phrase+ptr) == 0xff))
         return(ERR_PHRASE);           /*Phonetic code or phrase code is error*/
      ptr++;                           /* skip to next byte                   */
   }
   return(OK);
}

/******************************************************************************/
/* FUNCTION    : get_highlight_text                                           */
/* DESCRIPTION : get highlight candidate from phrase select window            */
/* INPUT       : obj                                                          */
/*               text                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
void get_highlight_text(obj,text)
UDEobject *obj;                        /* UDE Control Block                   */
char *text;
{
   char **cand_ptr;                    /* pointer to candidates               */

   cand_ptr = obj->phrase_select->candidate;
   if (obj->phrase_select->highlight >0)
      strcpy(text, *(cand_ptr+ obj->phrase_select->highlight-1));
   else
      text[0] = NULL;
}

/******************************************************************************/
/* FUNCTION    : dict_change                                                  */
/* DESCRIPTION : Had dictionary been modified or not ?                        */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int dict_change(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   extern char     *ctime();           /* Get Modified Time                   */

    if (access(dict->name, EXIST) == ERROR)      /* dict not exist            */
        return(create_dict(dict));     /* create dict                         */
    stat(dict->name, &state);          /* get staus of dict                   */
    modtime = ctime(&state.st_mtime);  /* get last modified time              */
    if (strcmp(dict->last_modified_time, modtime) != 0)   /* modified ?       */
       return(open_dict(dict));        /* reopen dict                         */
    else return(OK);
}

/******************************************/
/* error number and and its error message */
/******************************************/
static ErrorCode errorcode[] = {
       { ERR_WR, MSG_ERR_WR},
       { ERR_PHRASE, MSG_ERR_PHRASE},
       { ERR_PHONETIC, MSG_ERR_PHONETIC},
       { EX_IN_SYS, MSG_EX_IN_SYS},
       { EX_IN_USR, MSG_EX_IN_USR},
       { ERR_GF, MSG_ERR_GF},
       { EX_IN_NON, MSG_EX_IN_NON},
       { SAME_US, MSG_SAME_US},
       { SAME_MS, MSG_SAME_MS},
       { SAME_UM, MSG_SAME_UM},
       { ERR_IF, MSG_ERR_IF},
       { ERR_AP,MSG_ERR_AP},
       { ERR_OP, MSG_ERR_OP},
       { ERR_IV, MSG_ERR_IV},
       { ERR_SO, MSG_ERR_SO},
       { ERR_MF, MSG_ERR_MF},
       { ERR_IC, MSG_ERR_IC},
       { ERR_UNLOCK,MSG_ERR_UNLOCK},
       { ERR_LOCK, MSG_ERR_LOCK},
       { ERR_PHRASE_EMPTY, MSG_ERR_PHRASE_EMPTY},
       { ERR_PHONETIC_EMPTY, MSG_ERR_PHONETIC_EMPTY},
       { ERR_EMPTY, MSG_ERR_EMPTY},
       { PHONETIC_LONG, MSG_PHONETIC_LONG},
       { PHRASE_LONG, MSG_PHRASE_LONG},
       { PHRASE_SYS_DUP, MSG_PHRASE_SYS_DUP},
       { FILE_EMPTY,  MSG_FILE_EMPTY},
};

#define SIZE_ERRORCODE  (sizeof(errorcode) / sizeof(ErrorCode))

/******************************************************************************/
/* FUNCTION    : is_error                                                     */
/* DESCRIPTION : Is one of the error code number                              */
/* INPUT       : errno                                                        */
/* OUTPUT      :                                                              */
/******************************************************************************/
int is_error(errno)
int errno;                             /* error number                        */
{
   int entry;                          /* loop counter                        */

   /* search errorcode to match errno */
   for (entry=0;entry<SIZE_ERRORCODE;entry++)
   {
      if (errorcode[entry].errno == errno)
         return(1);                    /* errno is a valid error #            */
   }
   return(0);                          /* errno is not an error #             */
}

/******************************************************************************/
/* FUNCTION    : err_msg                                                      */
/* DESCRIPTION : return the error message according to the error code number  */
/* INPUT       : errno                                                        */
/* OUTPUT      :                                                              */
/******************************************************************************/
char *err_msg(errno)
int errno;                             /* error number                        */
{
   int entry;                          /* loop counter                        */

   /* search errorcode to match errno */
   for (entry=0;entry< SIZE_ERRORCODE;entry++)
   {
      if (errorcode[entry].errno == errno)
         return(errorcode[entry].errmsg);        /*return error message string*/
   }
   return((char *)NULL);               /* point to NULL                       */
}

/******************************************************************************/
/* FUNCTION    : replace                                                      */
/* DESCRIPTION : replace user dictionary with template dictionary             */
/* INPUT       : obj                                                          */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int replace(obj)
UDEobject *obj;                        /* UDE Control Block                   */
{
   char command[PATHLEN];

   sprintf(command,"cp %s %s",obj->tempdict->name,obj->usrdict->name);
   if (system(command))
      return(ERR_WR);
   else
   {
      strcpy(obj->usrdict->last_modified_time, "");
      unlink(obj->tempdict->name);     /* delete template dictionary         */
      return(OK);

   }
}

/******************************************************************************/
/* FUNCTION    : valid_dictionary                                             */
/* DESCRIPTION : Check dictionary has a valid dictionary format               */
/* INPUT       : dict                                                         */
/* OUTPUT      : NONE                                                         */
/******************************************************************************/
int valid_dictionary(dict)
DictInfo *dict;                        /* pointer to dictionary control block */
{
   int ptr;
   int num;
   long int max_rrn;
   long int rrn;
   char *indexkey_ptr;

   ptr =0;
   indexkey_ptr = get_next_index(dict->mi, &ptr);
   if (strncmp(indexkey_ptr,(char *)&miend, miend_len) == 0)
   {
      num=miend_len;
      while (num < MI_MAX_LENGTH)
           if (dict->mi[num++] != NULL)
            return(ERR_IF); /* Invalid dictionary format */
      return(OK);
   }
   while (strncmp(indexkey_ptr, &miend,miend_len) != 0)
   {
       if (strlen(indexkey_ptr) > MAX_PHONETIC_NUM)
          return(ERR_IF);                        /* Invalid dictionary format */
       num=0;
       while (*(indexkey_ptr+num))
       {
         if ((*(indexkey_ptr+num) >= 0xc7) && (*(indexkey_ptr+num) <= 0xf0))
            num++;
         else
            return(ERR_IF);  /* Invalid dictionary format */
       }
       indexkey_ptr = get_next_index(dict->mi, &ptr);
   }
   ptr = 0;
   max_rrn=0;

   while ((rrn=get_next_rrn(dict->mi,&ptr)) > 0)
   {
       if ((dict->flag & O_RDWR) == O_RDWR)
       {
          if ((rrn - MI_MAX_LENGTH) % DR_MAX_LENGTH)
               return(ERR_IF);
       }
       if ( max_rrn < rrn) max_rrn = rrn;
   }
   if (max_rrn < MI_MAX_LENGTH) return(ERR_IF);
   if(lseek(dict->fd, max_rrn, SEEK_SET) ==  ERROR)
      return(ERR_IF);                            /* Invalid dictionary format */
   return(OK);
}

/******************************************************************************/
/* FUNCTION    : lock_dict                                                    */
/* DESCRIPTION : lock dictionary for writing                                  */
/* INPUT       : fd                                                           */
/* OUTPUT      :                                                              */
/******************************************************************************/
int lock_dict(fd)
int fd;
{
   int count;

   count = 0;
   lseek(fd, 0L,SEEK_SET);                 /* rewind before lockf             */
   while (lockf(fd, F_TLOCK, 0L) == ERROR) /* 0L -> lock entire file          */
   {
      if (count++ > WaitTime)
         return(ERR_LOCK);                 /* can't lock                      */
      else
         sleep(1);                         /* sleep for one minute            */
   }
   return(OK);
}
/******************************************************************************/
/* FUNCTION    : unlock_dict                                                  */
/* DESCRIPTION : unlock dictionary for writing                                */
/* INPUT       : fd                                                           */
/* OUTPUT      :                                                              */
/******************************************************************************/
int unlock_dict(fd)
int fd;
{
   lseek(fd, 0L, SEEK_SET);
   if (lockf(fd, F_ULOCK, 0L) == ERROR )
   {
     return(ERR_UNLOCK);                   /* can't unlock                    */
   }
   else return(OK);
}

/******************************************************************************/
/* FUNCTION    : valid_filename                                               */
/* DESCRIPTION : check filename is a valid file name or not                   */
/* INPUT       : filename                                                     */
/* OUTPUT      :                                                              */
/******************************************************************************/
int valid_filename(filename)
char *filename;
{
   char * ptr;

   for (ptr=filename; *ptr != '\0' ; ptr++)
     if (*ptr == ' ') return(FALSE);
   if (*(ptr-1) != '/')
      return(TRUE);
   else
      return(FALSE);
}

/******************************************************************************/
/* FUNCTION    : ltoa                                                         */
/* DESCRIPTION : transfer a interger to be a string format                    */
/* INPUT       : value                                                        */
/*               string                                                       */
/* OUTPUT      :                                                              */
/* note        : The ltoa() is a C routine in some C compiler , but AIX C     */
/*               don't support this routine, so we create it                  */
/******************************************************************************/
void ltoa(value, string)
long value;
char *string;
{
   char digit;
   int index = 0;
   int i;
   int radix = 10;

   while (value > 0)
   {
       *(string + (index++)) = value % radix +'0';
       value /= radix;
   }
   *(string+index)=0;
   for (i=0; i< index/2; i++)
   {
      digit = *(string+i);
      *(string+i)=*(string+index-i-1);
      *(string+index-i-1) = digit;
   }
}

