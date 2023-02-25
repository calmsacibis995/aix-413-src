/* @(#)71	1.1  src/tw/usr/lpp/tls/bin/twuserdict/udemain.h, tw, tw411, GOLD410 7/10/92 14:01:25 */
/*
 * COMPONENT_NAME :     (CMDTW) - Traditional Chinese Dictionary Utility
 *
 * FUNCTIONS :          udemain.h
 *
 * ORIGINS :            27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
/* #include <Xm/IMMP.h> */
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/ListP.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/Xm.h>
#include "udemainMsg.h"

#define    EDIT0_EH          0
#define    EDIT1_EH          1
#define    EDIT2_EH          2
#define    EDIT3_EH          3

#define    EDIT0_CB          0
#define    EDIT1_CB          1
#define    EDIT2_CB          2
#define    EDIT3_CB          3

#define    MENU_A_PHR      200
#define    MENU_U_PHN      201
#define    MENU_U_PHR      202
#define    MENU_D_PHN      203
#define    MENU_D_PHR      204
#define    MENU_CUT        205
#define    MENU_COPY       206
#define    MENU_PASTE      207
#define    MENU_CLEAR      208
#define    MENU_UNDO       209
#define    MENU_OPEN       210
#define    MENU_MERGE      211
#define    MENU_SEARCH     212
#define    MENU_EXIT       213
#define    MENU_HELP       214
#define    MENU_QUIT       215

#define    DIALOG_OPEN     300
#define    DIALOG_MERGE    301
#define    DIALOG_SEARCH   303
#define    DIALOG_A_PHR    304
#define    DIALOG_U_PHN    305
#define    DIALOG_U_PHR    306
#define    DIALOG_D_PHN    307
#define    DIALOG_D_PHR    308
#define    DIALOG_Q_U_PHN  309
#define    DIALOG_Q_U_PHR  310
#define    DIALOG_Q_D_PHN  311
#define    DIALOG_Q_D_PHR  312
#define    DIALOG_Q_EXIT   313

/*
 *  The Chinese word is 26x27 bitmap font
 */
#define    CHINESE_WORD_WIDTH   26
#define    CHINESE_WORD_HEIGHT  27

#define    M_A                  'A'
#define    M_B                  'B'
#define    M_C                  'C'
#define    M_D                  'D'
#define    M_E                  'E'
#define    M_F                  'F'
#define    M_H                  'H'
#define    M_M                  'M'
#define    M_O                  'O'
#define    M_P                  'P'
#define    M_R                  'R'
#define    M_S                  'S'
#define    M_T                  'T'
#define    M_U                  'U'
#define    M_X                  'X'
#define    M_F1                 "F1"
#define    M_F3                 "F3"
#define    M_KF1                "<Key>F1:"
#define    M_KF3                "<Key>F3:"
#define    LANGUAGE             "zh_TW"
#define    BLACK                "black"
#define    RED                  "red"

static   char  *color;
static char *a_phr_str         =  A_PHR_STR;
static char *u_phn_str         =  U_PHN_STR;
static char *u_phr_str         =  U_PHR_STR;
static char *d_phn_str         =  D_PHN_STR;
static char *d_phr_str         =  D_PHR_STR;
static char *commit_str        =  COMMIT_STR;
static char *open_str          =  OPEN_STR;
static char *merge_str         =  MERGE_STR;
static char *file_str          =  FILE_STR;
static char *cut_str           =  CUT_STR;
static char *copy_str          =  COPY_STR;
static char *paste_str         =  PASTE_STR;
static char *clear_str         =  CLEAR_STR;
static char *undo_str          =  UNDO_STR;
static char *edit_str          =  EDIT_STR;
static char *search_str        =  SEARCH_STR;
static char *exit_str          =  EXIT_STR;
static char *help_exit_str     =  HELP_EXIT_STR;
static char *quit_str          =  QUIT_STR;
static char *help_quit_str     =  HELP_QUIT_STR;
static char *resume_str        =  RESUME_STR;
static char *help_str          =  HELP_STR;
static char *help_help_str     =  HELP_HELP_STR;
static char *phonetic_entry_str=  PHONETIC_ENTRY_STR;
static char *phrase_entry_str  =  PHRASE_ENTRY_STR;
static char *find_str          =  FIND_STR;
static char *find_end_str      =  FIND_END_STR;
static char *open_msg          =  OPEN_MSG;
static char *merge_msg         =  MERGE_MSG;
static char *u_phn_d_msg       =  U_PHN_D_MSG;
static char *u_phr_d_msg       =  U_PHR_D_MSG;
static char *d_phn_d_msg       =  D_PHN_D_MSG;
static char *d_phr_d_msg       =  D_PHR_D_MSG;
static char *ok_file_msg       =  OK_FILE_MSG;
static char *ok_merge_msg      =  OK_MERGE_MSG;
static char *ok_u_phn_msg      =  OK_U_PHN_MSG;
static char *ok_u_phr_msg      =  OK_U_PHR_MSG;
static char *ok_d_phn_msg      =  OK_D_PHN_MSG;
static char *ok_d_phr_msg      =  OK_D_PHR_MSG;
static char *ok_a_phr_msg      =  OK_A_PHR_MSG;
static char *exit_msg          =  EXIT_MSG;
static char *ok_button_str     =  OK_BUTTON_STR;
static char *cancel_button_str =  CANCEL_BUTTON_STR;
static char *help_button_str   =  HELP_BUTTON_STR;
static char *filter_button_str =  FILTER_BUTTON_STR;
static char *filter_label_str  =  FILTER_LABEL_STR;
static char *file_title        =  FILE_TITLE;

static char m_a = M_A;
static char m_b = M_B;
static char m_c = M_C;
static char m_d = M_D;
static char m_e = M_E;
static char m_f = M_F;
static char m_h = M_H;
static char m_m = M_M;
static char m_o = M_O;
static char m_p = M_P;
static char m_r = M_R;
static char m_s = M_S;
static char m_t = M_T;
static char m_u = M_U;
static char m_x = M_X;
static char *m_f1 = M_F1;
static char *m_f3 = M_F3;
static char *m_kf1 = M_KF1;
static char *m_kf3 = M_KF3;


struct help
{
   int length;                         /* the length of help information     */
   int number;                         /* Series number                      */
   int nest;                           /* the nest level for button          */
   char *label;                        /* the string of label widget         */
   struct aline *text;                 /* the text of help information       */
   struct help *next;                  /* pointer to next help token         */
};

struct aline
{
  char* string;                        /* the string of help information     */
  struct aline *next;                  /* pointer to next string             */
};

static XmTextScanType carray[] = {
    XmSELECT_POSITION,XmSELECT_POSITION,XmSELECT_POSITION,XmSELECT_POSITION
};

static   int        open_flag;
static   int        undo_flag;
static   int        edit_f[4];
static   Widget     current_widget;
static   Widget     select_widget;
static   char      *clip_string;
static   char      *select_string;
static   Widget     app_shell;        /*  ApplicationShell                  */
static   Arg        al[20];           /*  arg list                          */
static   int        ac;               /*  arg count                         */
static   XmString   list_item[1024];  /*  list items # DR_MAX_LENGTH/2      */
static   char       *title_name;

static   Widget edit0;
static   Widget edit1;
static   Widget edit2;
static   Widget edit3;
static   Widget a_phr_button;   /* add phrase button                        */
static   Widget u_phn_button;   /* update phonetic button                   */
static   Widget u_phr_button;   /* update phrase button                     */
static   Widget d_phn_button;   /* delete phonetic button                   */
static   Widget d_phr_button;   /* delete phrase button                     */
static   Widget open_button;    /* open dictionary file button              */
static   Widget merge_button;   /* merge dictionary button                  */
static   Widget cut_button;     /* clipboard cut button                     */
static   Widget copy_button;    /* clipboard copy button                    */
static   Widget paste_button;   /* clipboard paste button                   */
static   Widget clear_button;   /* clipboard clear button                   */
static   Widget undo_button;    /* clipboard undo button                    */
static   Widget search_button;  /* clipboard search button                  */
static   Widget quit_button;    /* quit button                              */
static   Widget resume_button;  /* resume button                            */
static   Widget help_button;    /* help button                              */
static   Widget dialog;         /* file selection dialog                    */
static   Widget help_dialog;
static   Widget help_button1;   /* help button for help                     */
static   Widget quit_button1;   /* quit button for help                     */
static   Widget  text_box;
static   Widget  shell1;
static   Widget help_name0,help_name1;
static   char *filename = NULL; /* string containing file name              */
static   XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
                                /* used to set up XmStrings                 */
/*
 *  help information file name
 */
#define    UDEHFILE             "/usr/lpp/tls/bin/udehelp"

/* The following definition of strings do not transfer to Chinese */
#define help_help_message     "help help message"
#define open_file_ok          "open file ok"
#define open_file_error       "open file error"
#define merge_file_ok         "merge file ok"
#define merge_file_error      "merge file error"
#define update_phonetic_ok    "update phonetic ok"
#define update_phonetic_error "update phonetic error"
#define update_phrase_ok      "update phrase ok"
#define update_phrase_error   "update phrase error"
#define delete_phonetic_ok    "delete phonetic ok"
#define delete_phonetic_error "delete phonetic error"
#define delete_phrase_ok      "delete phrase ok"
#define delete_phrase_error   "delete phrase error"
#define add_phrase_ok         "add phrase ok"
#define add_phrase_error      "add phrase error"
#define update_phonetic       "update phonetic"
#define update_phrase         "update phrase"
#define delete_phonetic       "delete phonetic"
#define delete_phrase         "delete phrase"
#define file_selection_dialog "file selection dialog"
#define search_warning        "search warning"
#define help_title            "Help"
/* The above definition of strings do not transfer to Chinese */
