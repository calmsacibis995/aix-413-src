static char sccsid[] = "@(#)70	1.1  src/tw/usr/lpp/tls/bin/twuserdict/udemain.c, tw, tw411, GOLD410 7/10/92 14:01:21";
/*
 * COMPONENT_NAME :     (cmdTW) - Traditional Chinese Dictionary Utility
 *
 * FUNCTIONS :          udemain.c
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

/******************** START OF MODULE SPECIFICATION ***************************/
/*                                                                            */
/* MODULE NAME:  udemain.c                                                    */
/*                                                                            */
/* DESCRIPTIVE NAME: User Dictionary Utility Main Function For Widget.        */
/*                                                                            */
/* FUNCTION:                                                                  */
/*           initCursors : To Set Up The Cursor Resource.                     */
/*                                                                            */
/*           freeCursors : To Free Up The Cursor Resource.                    */
/*                                                                            */
/*           associateCursors : Associates All The Windows In The Application */
/*                         With Their Default Cursors.                        */
/*                                                                            */
/*           makeAllBusyCursors : To Set All Cursors To Be theBusyCursor.     */
/*                                                                            */
/*           UDEProcess : Entry routine of UDEProcessRoutine.                 */
/*                                                                            */
/*           CopyProc : Copy Text Selection Into Local Clipboard.             */
/*                                                                            */
/*           ClearProc : Clear Text Selection Into Local Clipboard.           */
/*                                                                            */
/*           PasteProc : Paste Clipboard Text Into Text Area.                 */
/*                                                                            */
/*           ChangeList : Change The List Item.                               */
/*                                                                            */
/*           CheckCommit : Check Commit Pulldown Menu Sensitive.              */
/*                                                                            */
/*           ChangeFocusEH : Event Handler For Change Focus.                  */
/*                                                                            */
/*           KeyboardSelectEH : Event Handler For Keyboard Select.            */
/*                                                                            */
/*           read_help_file : Read the Help Information From Disk.            */
/*                                                                            */
/*           first_token : Get The First Token Of One Line Of Help Information*/
/*                                                                            */
/*           find_nest : To Find The Nest Number Of The Help Button.          */
/*                                                                            */
/*           create_help_button : Create Help Buttons And Label Strings.      */
/*                                                                            */
/*           HelpMenuCB : Help Menu Bar Item Callback Routine.                */
/*                                                                            */
/*           PushHelpButtonCB : Get The Help Information When Push One        */
/*                              Help Button.                                  */
/*                                                                            */
/*           free_help_text : Free Help Information Data Structure.           */
/*                                                                            */
/*           cancate : Add The Source To Dest String.                         */
/*                                                                            */
/*           CreateHelpMenuBar : Create Help Menu Bar Widgets.                */
/*                                                                            */
/*           CreateHelpWorkArea : Create Help Work Area Widget.               */
/*                                                                            */
/*           destroy_handle : destroy handle for help widget.                 */
/*                                                                            */
/*           HelpOkPopUp : PopUp an OK Dialog Box for Help.                   */
/*                                                                            */
/*           HelpCB : Callback Routine For Help Button.                       */
/*                                                                            */
/*           BeforeInputCB : Callback Routine For Before Input Text.          */
/*                                                                            */
/*           EUClen : Return EUC Character Length of Input string.            */
/*                                                                            */
/*           IsPhoneticStr : Check Input Whether Is Phonetic String.          */
/*                                                                            */
/*           IsPhraseStr : Check Input Whether Is Phrase String.              */
/*                                                                            */
/*           CheckEditCB : Callback Routine For Check Edit Pulldown Sensitive.*/
/*                                                                            */
/*           SingleClickCB : Callback Routine For Single Click Action.        */
/*                                                                            */
/*           DoubleClickCB : Callback Routine For Double Click Action.        */
/*                                                                            */
/*           DialogCancelCB : Callback Routine For Dialog Cancel Action.      */
/*                                                                            */
/*           DialogAcceptCB : Callback Routine For Dialog Accept Action.      */
/*                                                                            */
/*           OkPopUp : PupUp an Ok Dialog.                                    */
/*                                                                            */
/*           MenuCB  : Callback Routine For Menu Selection Action.            */
/*                                                                            */
/*           QuestionPopUp : Create a Two Button Question Dialog.             */
/*                                                                            */
/*           CreateMenuBar : Create Menu Bar Widget.                          */
/*                                                                            */
/*           CreateWorkArea : Create Working Area Widget.                     */
/*                                                                            */
/*           main : Main Routine For User Diction Utility.                    */
/*                                                                            */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Andrew Wu                                             */
/*                      Benson Lu                                             */
/*                                                                            */
/* STATUS:   Traditional Phonetic User Dictionary Editor Version 1.0          */
/*                                                                            */
/* CHANGE ACTIVITY:                                                           */
/*                                                                            */
/************************ END OF SPECIFICATION ********************************/

/*****************/
/* include files */
/*****************/
#include "udemain.h"
#include "udeph.h"

Cursor  theArrowCursor;
Cursor  theBusyCursor;       /* XC_watch, used while writing to disk */
Cursor  theTextCursor;       /* XC_xterm ,for text drawing           */
Cursor  theViewCursor;
Cursor  theHandCursor;
Display *theDisplay;
Window  theMainWindow;
Window  theEdit0Window;
Window  theEdit1Window;
Window  theEdit2Window;
Window  theEdit3Window;

char *  help_string;
struct  help *help_first;               /* pointer to the first help token    */
int     help_num;                       /* Total number of help button        */
static  int help_flag;                  /* Help pop-up appear flag            */
static  int quit_flag;
static  char buffer[PHRASE_LEN];        /* Input Buffer                       */
int     TextSetString;
Widget  main1;

void    initCursors();
void    freeCursors();
void    associateCursors();
void    makeAllBusyCursor();
char    *first_token();
void    get_sensitive_help();
int     read_help_file();
void    get_sensitive_help();
void    create_help_button();

void    ReInput();
void    FilterPhonetic();
void    FilterPhrase();
void    Truncate();
int     CopyProc();
void    ClearProc();
void    PasteProc();
void    ChangeList();
void    CheckCommit();
void    ChangeFocusEH();
void    KeyboardSelectEH();
void    HelpCB();
Widget  CreateHelpMenuBar();
void    HelpMenuCB();
void    PushHelpButtonCB();
void    HelpOkPopUp();
void    ExitCB();
Widget  CreateHelpWorkArea();
void    BeforeInputCB();
void    CheckEditCB();
int     EUClen();
int     IsPhoneticStr();
int     IsPhraseStr();
void    SingleClickCB();
void    DoubleClickCB();
void    DialogCancelCB();
void    HelpCancelCB();
void    DialogAcceptCB();
void    OkPopUp();
void    MenuCB();
/* static XtCallbackProc destroy_handle(); */
Widget  QuestionPopUp();
Widget  CreateMenuBar();
Widget  CreateWorkArea();
static Pixel  GetPixel();

/******************************************************************************/
/* FUNCTION    : GetPixel                                                     */
/* DESCRIPTION :                                                              */
/* INPUT       : widget                                                       */
/*               color_string                                                 */
/* OUTPUT      : none.                                                        */
/******************************************************************************/
static Pixel  GetPixel(widget, color_string)
Widget  widget;
char   *color_string;
{
   XrmValue from, to;

   from.size = strlen(color_string) + 1;
   if (from.size < sizeof(String))
      from.size = sizeof(String);
   from.addr = color_string;
   XtConvert(widget, XmRString, &from, XmRPixel, &to);

   if (to.addr != NULL)
      return ((Pixel) *((Pixel *) to.addr));
   else
      return ( (XtArgVal) NULL);
}

/******************************************************************************/
/* FUNCTION    : initCursors                                                  */
/* DESCRIPTION : To set up the cursor resources.                              */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/******************************************************************************/
void initCursors()
{
    theArrowCursor = XCreateFontCursor(theDisplay, XC_top_left_arrow);
    theBusyCursor = XCreateFontCursor(theDisplay,XC_watch);
    theTextCursor = XCreateFontCursor(theDisplay,XC_xterm);
    theViewCursor = XCreateFontCursor(theDisplay,XC_hand2-1);
    theHandCursor = XCreateFontCursor(theDisplay,XC_hand2);
}

/******************************************************************************/
/* FUNCTION    : freeCursors                                                  */
/* DESCRIPTION : To free up the cursor resources.                             */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/******************************************************************************/
void freeCursors()
{
    XFreeCursor(theDisplay, theArrowCursor);
    XFreeCursor(theDisplay,theBusyCursor);
    XFreeCursor(theDisplay,theTextCursor);
    XFreeCursor(theDisplay,theViewCursor);
    XFreeCursor(theDisplay,theHandCursor);
    XFlush(theDisplay);
}

/******************************************************************************/
/* FUNCTION    : associateCursors                                             */
/* DESCRIPTION : Associates all the windows in the application with their     */
/*               default cursors.                                             */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void associateCursors()
{
    XDefineCursor(theDisplay, theMainWindow, theArrowCursor);
    XDefineCursor(theDisplay, theEdit0Window, theTextCursor);
    XDefineCursor(theDisplay, theEdit1Window, theTextCursor);
    XDefineCursor(theDisplay, theEdit2Window, theViewCursor);
    XDefineCursor(theDisplay, theEdit3Window, theHandCursor);
    XFlush(theDisplay);
}

/******************************************************************************/
/* FUNCTION    : makeAllBusyCursor                                            */
/* DESCRIPTION : To let the user know that the program is busy writing to disk*/
/*               (or reading from disk), all the cursors in the application   */
/*               windows are set to the theBusyCursor, before writing to disk.*/
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void makeAllBusyCursor()
{
    XDefineCursor(theDisplay, theMainWindow, theBusyCursor);
    XDefineCursor(theDisplay, theEdit0Window, theBusyCursor);
    XDefineCursor(theDisplay, theEdit1Window, theBusyCursor);
    XDefineCursor(theDisplay, theEdit2Window, theBusyCursor);
    XDefineCursor(theDisplay, theEdit3Window, theBusyCursor);
    XFlush(theDisplay);
}

/******************************************************************************/
/* FUNCTION    : UDEProcess                                                   */
/* DESCRIPTION : To let all the cursors in the application windows are set    */
/*               to the theBusyCursor, before reading/writing the dictionary. */
/*               After processing the dictionary, to let all the windows in   */
/*               the application with their default cursors.                  */
/* INPUT       : obj  = UDE control block.                                    */
/*               flag = flag for entry routine.                               */
/* OUTPUT      : OK or error message.                                         */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int UDEProcess(obj, flag)
UDEobject *obj;                          /* UDE Control Block                 */
int flag;                                /* flag for entry routine            */
{                                        /* return code                       */
  int ret;
  XEvent event;

  makeAllBusyCursor();                   /* Make all busy cursor              */
  ret = UDEProcessRoutine(obj, flag);
  if (ret==ERR_WR || ret==ERR_IF || ret==ERR_AP || ret==ERR_OP || ret==ERR_IC)
  {
     /* Some error about dictionary access , so set the dictionary to be NULL */
     if (strcmp(obj->usrdict->name, obj->open_entry) == 0)
     {
        strcpy(obj->phonetic_view, "");
        strcpy(obj->usrdict->name, "");
        get_dict_cands_to_list_box(obj, "");
        open_flag = False;
        XStoreName(XtDisplay(app_shell),XtWindow(app_shell),title_name);
        XtSetSensitive(merge_button,False);         /* Set Merge button  */
     }
  }
  associateCursors(); /* associate all the windows with their default cursors */
  ChangeList();                        /* Redisplay List item                 */
  /* move all key press events */
  while (True) {
    if (XtPending()) {
      XtNextEvent(&event);
      if (event.type != KeyPress)       /* ignored all the keypress events    */
         XtDispatchEvent(&event);
    }
    else
       break;
  } /* endwhile */
  return(ret);                           /* return code                       */
}

/******************************************************************************/
/* FUNCTION    : CopyProc                                                     */
/* DESCRIPTION : Copy selected string into global variable for clipboard.     */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int   CopyProc()
{
   if ( IsPhraseStr(select_string) == False ) /* Test selection string is     */
   {                                          /* Phrase String ?              */
       XBell(XtDisplay(app_shell),0);         /* Beep */
       return(False);
   }
   if ( select_string != NULL )               /* Has there selection string   */
        clip_string = select_string;          /* save selection string */
   else
   {
       XBell(XtDisplay(app_shell),0);
       return(False);
   }
   return (True);
}

/******************************************************************************/
/* FUNCTION    : ClearProc                                                    */
/* DESCRIPTION : Clear global variable string value for clipboard.            */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   ClearProc()
{
   XClientMessageEvent cm;
   cm.type = ClientMessage;   /* Send Kill_Selection Client Message to the    */
                              /* Selected Widget.                             */
   cm.display = XtDisplay(select_widget);
   cm.message_type = XmInternAtom(XtDisplay(select_widget),
                                  "KILL_SELECTION", FALSE);
   cm.window = XGetSelectionOwner(XtDisplay(select_widget), XA_PRIMARY);
   cm.format = 32;
   cm.data.l[0] = XA_PRIMARY;
   XSendEvent(XtDisplay(select_widget), cm.window, TRUE, NoEventMask, &cm);
}
/******************************************************************************/
/* FUNCTION    : PasteProc                                                    */
/* DESCRIPTION : Paste the clipboard value into current text widget.          */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   PasteProc()
{
   XmTextPosition cursorPos;

   if ( current_widget == edit0 || current_widget == edit1 )
   {
      XtSetArg(al[0],XmNcursorPosition,&cursorPos);      /* Get Cursor Position  */
      XtGetValues(current_widget,al,1);
      XmTextReplace(current_widget,cursorPos,cursorPos,clip_string);
   }
   else
       XBell(XtDisplay(current_widget),0);            /* Beep !               */

}


/******************************************************************************/
/* FUNCTION    : ChangeList                                                   */
/* DESCRIPTION : Redisplay list area item.                                    */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   ChangeList()
{
   int i;
   XmListWidget lw3 = (XmListWidget) edit3;

   list_item[0] = XmStringCreate(obj->phonetic_view,charset);
   ac=0;                                      /* Display Phonetic_view area   */
   XtSetArg(al[ac],XmNitems,list_item);  ac++;
   XtSetArg(al[ac],XmNitemCount,1);  ac++;
   XtSetValues (edit2,al,ac);
   if ( strlen(obj->phonetic_view) == 0 )     /* Phonetic_view area is empty */
      edit_f[2] = False;
   else
      edit_f[2] = True;                           /* Draw Phonetic_view area  */

   if ( obj->phrase_select->highlight != 0 )       /* Deselect Highlight      */
       XmListDeselectPos(edit3,obj->phrase_select->highlight);
   if ( obj->phrase_select->candnum == 0 )        /* There is no item in list */
   {
         obj->phrase_select->highlight = 0;
         edit_f[3] = False;
   }
   for ( i = 0; i < obj->phrase_select->candnum; i++ )      /* Set List Item  */
      list_item[i] = XmStringCreate(obj->phrase_select->candidate[i],charset);
   ac=0;
   XtSetArg(al[ac],XmNitems,list_item);  ac++;
   XtSetArg(al[ac],XmNitemCount,obj->phrase_select->candnum);  ac++;
   XtSetValues (edit3, al, ac);
   if ( obj->phrase_select->highlight > 0 )                 /* Set Highlight  */
   {
      if ( obj->phrase_select->highlight > obj->phrase_select->candnum )
           obj->phrase_select->highlight = obj->phrase_select->candnum;
      lw3->list.InternalList[obj->phrase_select->highlight-1]->selected = TRUE;
   }
}
/******************************************************************************/
/* FUNCTION    : CheckCommit                                                  */
/* DESCRIPTION : Check which PushButton should be Sensitive in Commit pull    */
/*               down menu.                                                   */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   CheckCommit()
{
  char *tmp0,*tmp1;

  tmp0 = XmTextGetString(edit0);   /* Check Phonetic_entry whether has input  */
  if (strcmp(tmp0,NULL)==0)
       edit_f[0] = False;
  else
       edit_f[0] = True;

  tmp1 = XmTextGetString(edit1);   /* Check Phrase_entry whether has input ?  */
  if (strcmp(tmp1,NULL)==0)
       edit_f[1] = False;
  else
       edit_f[1] = True;

  if (strcmp(obj->phonetic_view,NULL)==0)
       edit_f[2] = False;          /* Check Phonetic_view area whether has    */
  else                             /* output ?   */
       edit_f[2] = True;

  if ( edit_f[2]== True )          /* Check Delete Phonetic button sensitive  */
        XtSetSensitive(d_phn_button,True);
  else
        XtSetSensitive(d_phn_button,False);

  if ( edit_f[0]==True && edit_f[2]==True )  /* Check Update Phonetic button  */
       XtSetSensitive(u_phn_button,True);    /* Sensitive  */
  else
       XtSetSensitive(u_phn_button,False);

  if ( edit_f[0]==True && edit_f[1]==True && open_flag==True)   /* Check Add Phrase button      */
       XtSetSensitive(a_phr_button,True);     /* Sensitive   */
  else
       XtSetSensitive(a_phr_button,False);

  if ( edit_f[1]==True && edit_f[3]==True )   /* Check Update Phrase button   */
       XtSetSensitive(u_phr_button,True);     /* Sensitive  */
  else
       XtSetSensitive(u_phr_button,False);

  if ( edit_f[2]==True && edit_f[3]==True )   /* Check Delete Phrase button   */
       XtSetSensitive(d_phr_button,True);     /* Sensitive  */
  else
       XtSetSensitive(d_phr_button,False);
}
/******************************************************************************/
/* FUNCTION    : ChangeFocusEH                                                */
/* DESCRIPTION : Change Focus Event Handler Routine                           */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               event = provided by widget class.                            */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   ChangeFocusEH(w,client_data,event)
Widget   w;
caddr_t  client_data;
XEvent   *event;
{
    switch((int)client_data)
    {
       case EDIT0_EH :
            if ( event->xfocus.type == FocusIn )          /* Check Event Type */
            {
               current_widget = edit0;
               if ( undo_flag==True )           /* allow undo button to work  */
                   XtSetSensitive(undo_button,True); /* sensitive undo button */
               else
                   XtSetSensitive(undo_button,False);
            }
           break;
      case EDIT1_EH :
           if ( event->xfocus.type == FocusIn )           /* Check Event Type */
           {
               current_widget = edit1;
               if ( undo_flag==True )           /* allow undo button to work  */
                  XtSetSensitive(undo_button,True);
               else
                   XtSetSensitive(undo_button,False);
           }
           break;
      case EDIT2_EH :
           if ( event->xfocus.type == FocusIn )           /* Check Event Type */
           {
                XmListSelectPos(edit2,1,False);    /* highlight edit2 */
                XtSetSensitive(cut_button,False);  /* Set Edit Menu Buttons   */
                XtSetSensitive(copy_button,False);    /* Unsensitive  */
                XtSetSensitive(clear_button,False);
                XtSetSensitive(paste_button,False);
                XtSetSensitive(undo_button,False);
           }
           else
                XmListDeselectPos(edit2,1);
           break;
      case EDIT3_EH :
           XtSetSensitive(cut_button,False);     /* Set Edit Menu Buttons     */
           XtSetSensitive(copy_button,False);    /* Unsensitive  */
           XtSetSensitive(clear_button,False);
           XtSetSensitive(paste_button,False);
           XtSetSensitive(undo_button,False);
           break;
    }
    if ( event->xfocus.type == FocusOut )  /* Check Commit Menu Item when */
        CheckCommit();                     /* focus out */
}

/******************************************************************************/
/* FUNCTION    : KeyboardSelectEH                                             */
/* DESCRIPTION : Keyboard Select Event Handler Routine                        */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               event = provided by widget class.                            */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   KeyboardSelectEH(w,client_data,event)
Widget   w;
caddr_t  client_data;
XEvent   *event;
{
    int ListPos;
    KeySym keysym;
    char text[10];
    XmListWidget lw3 = (XmListWidget) edit3;

    switch((int)client_data)
    {
       case EDIT2_EH:
            XLookupString ( event, text, 10, &keysym, 0 );  /* get keysym  */
            switch(keysym)
            {
              case XK_Return:    /* enter */
              case XK_KP_Enter:    /* enter for keypad */
                    TextSetString = True;
                    XmTextSetString(edit0,obj->phonetic_view);
                 break;
              case XK_Up :
              case XK_Down:
                 if ( obj->phrase_select->candnum > 0 )
                    XtSetKeyboardFocus(app_shell,edit3);
                 break;
            }
            break;
       case EDIT3_EH:
            XLookupString ( event, text, 10, &keysym, 0 );
            switch(keysym)
            {
              case XK_Up:      /* up  */    /* already supported by MOTIF  */
              case XK_Down:    /* down */
              case XK_Next:    /* PageDown */
              case XK_Prior:   /* pageUp */
                    edit_f[3] = True;
                    break;
              case XK_Return:    /* enter */
              case XK_KP_Enter:    /* enter for keypad */
                 if(obj->phrase_select->highlight >0)
                 {
                       TextSetString = True;
                       XmTextSetString(edit1,obj->phrase_select->candidate[
                                       obj->phrase_select->highlight-1]);
                 }
                 break;
              case XK_End:    /* End */
                 if ( obj->phrase_select->candnum == 0 )
                      break;
                 edit_f[3] = True;
                 obj->phrase_select->highlight=obj->phrase_select->candnum;
                 XmListSetBottomPos(edit3,obj->phrase_select->highlight);
                 lw3->list.CurrentKbdItem = obj->phrase_select->highlight-1;
                 XmListSelectPos(edit3,obj->phrase_select->highlight,False);
                 break;
              case XK_Home:    /* Home */
                 if ( obj->phrase_select->candnum == 0 )
                      break;
                 obj->phrase_select->highlight=1;
                 if (obj->phrase_select->candnum <=10)
                     ListPos = obj->phrase_select->candnum;
                 else
                     ListPos =10;
                 XmListSetBottomPos(edit3,ListPos);
                 edit_f[3] = True;
                 lw3->list.CurrentKbdItem = obj->phrase_select->highlight-1;
                 XmListSelectPos(edit3,obj->phrase_select->highlight,False);
                 break;
            }
            break;
    }
}

/******************************************************************************/
/* FUNCTION    : read_help_file                                               */
/* DESCRIPTION : read the help information from udehfile in disk              */
/* INPUT       : none.                                                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
/*               Syntax of Help Information File -(udehfile)                 */
/*                                                                           */
/*    < help file > ::= <help block> {<help block>}                          */
/*    <help block> ::= <label number> <label text> <new line>                */
/*                     <help text>                                           */
/*    <label text> ::= <compound string>                                     */
/*    <help text> ::= {<compound string> <new line>}                         */
/*    <label number> ::= <digit> | . {<digit> | . }                          */
/*    <compound string> ::= <simple string> {<simple string>}                */
/*    <simple string> ::= <character> {<character>}                          */
/*    <character> ::= <empty> | <blank> | <digit> | <letter> | <Chinese>     */
/*    <new line> ::= '\n'                                                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/*                  Struct of Help Information                               */
/*                                                                           */
/*   help_first                                                              */
/*        |                                                                  */
/*        v                                                                  */
/*       +--------+                                                          */
/*       | length |                                                          */
/*       +--------+                                                          */
/*       | number |                                                          */
/*       +--------+                                                          */
/*       | nest   |                                                          */
/*       +--------+                                                          */
/*       | label  |                                                          */
/*       +--------+     +--------+-+        +--------+-+                     */
/*       | text   |---->| string | |->....->| string | |->NULL               */
/*       +--------+     +--------+-+        +--------+-+                     */
/*       +---+----+                                                          */
/*           |                                                               */
/*           v                                                               */
/*           :                                                               */
/*           :                                                               */
/*           |                                                               */
/*           v                                                               */
/*       +--------+                                                          */
/*       | length |                                                          */
/*       +--------+                                                          */
/*       | number |                                                          */
/*       +--------+                                                          */
/*       | nest   |                                                          */
/*       +--------+                                                          */
/*       | label  |                                                          */
/*       +--------+     +--------+-+        +--------+-+                     */
/*       | text   |---->| string | |->....->| string | |->NULL               */
/*       +--------+     +--------+-+        +--------+-+                     */
/*       +---+----+                                                          */
/*           |                                                               */
/*           v                                                               */
/*          NULL                                                             */
/*                                                                           */
/*****************************************************************************/
int read_help_file()
{
   FILE *stream;                       /* File Description                   */
   char *token;                        /* Help token                         */
   char *help_string;                  /* pointer to help string             */
   char string[256];                   /* string buffer for input            */
   char *string_ptr;                   /* pointer to input string buffer     */
   int start_pos;                      /* the effective start position in    */
                                       /* input buffer for help text         */
   int nest_num;                       /* the nest number for help button    */
   unsigned int len;                   /* length of help text in one line    */
   struct help *help_token;            /* pointer to help token              */
   struct help *help_ptr;              /* pointer to help token              */
   struct aline *aline_token;          /* poinrt to help text                */
   struct aline *aline_rear;           /* poinrt to the last help text       */
   int number = 0;                     /* counter of help button             */

   /* Create the first one help token, we assume it exist                    */
   help_first = (struct help *) XtMalloc(sizeof(struct help));
   help_first->label = (char *) XtMalloc(sizeof(char)*strlen(ALL_HELP_TITLE)+1);
   strcpy(help_first->label, ALL_HELP_TITLE);    /* Default label text       */
   help_first->nest =0;                          /* Help button in top       */
   help_first->text=NULL;
   help_first->number =0;
   help_first->length= 0;
   help_first->next = NULL;
   help_ptr = help_first;

   if (access(UDEHFILE, EXIST&READ) == ERROR)
   {
/*      fprintf(stderr,"help file not open\n"); */
      return;
   }
   if ((stream = fopen(UDEHFILE,"r")) != NULL)
   {
      while (feof(stream) ==0)
      {
         if ((string_ptr =fgets(string, 256, stream)) == NULL)
            break;
         start_pos=0;
         /* If '#' in the first column of help text , it is a comment.    */
         /* we shall ingore it.                                           */
         if (string[0] != '#')
         {
             token = first_token(string, &start_pos);
             help_string = string+start_pos;
             if (*token != '\0')
             {
                nest_num = find_nest(token);
                help_token = (struct help *) XtMalloc (sizeof(struct help));
                help_token->length=0;
                help_token->number = ++number;
                help_token->nest = nest_num;
                help_token->label = (char *) XtMalloc(strlen(help_string));
                len = strlen(help_string);
                help_string[len-1]='\0';       /* delete new line character*/
                strcpy(help_token->label, help_string);
                help_token->text =  NULL;
                help_token->next = NULL;
                help_ptr->next = help_token;
                help_ptr=help_token;
             }
             else
             {
                if (number == 0)
                {
/*                   fprintf(stderr,"help file format error...\n"); */
                   return;
                }
                aline_token = (struct aline *) XtMalloc (sizeof(struct aline)+1);
                len = strlen(help_string);
                aline_token->string = (char *) XtMalloc(len+1);
                strcpy(aline_token->string, help_string);
                aline_token->next = NULL;
                help_first->length += len;   /* ignore end of line */
                help_token->length += len;
                if (help_ptr->text == NULL)
                {
                   help_ptr->text = aline_token;
                   aline_rear = aline_token;
                }
                else
                {
                   aline_rear->next = aline_token;
                   aline_rear = aline_token;
                }
             }
         }
      }
      fclose(stream);
   }
}


/******************************************************************************/
/* FUNCTION    : first_token                                                  */
/* DESCRIPTION : Get the first token of one line of help information          */
/* INPUT       : string = help information                                    */
/*               start_pos = efficient starting position of first token.      */
/* OUTPUT      : strbuf = the first token.                                    */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
char * first_token(string, start_pos)
char *string;
int *start_pos;
{
   static char strbuf[256];
   char *ptr;
   int len = 0;

   ptr = string;
   while (*ptr == ' ')  ptr++;    /* skip the white space */
 /* we assume that only 0..9 and . were valid character to label number */
   while (((*ptr >= '0') && (*ptr <='9')) || (*ptr == '.'))
   {
       strbuf[len++] = *(ptr++);
   }
   if (len != 0)
   {
      while (*ptr == ' ')              /* skip spaces */
         ptr++;
      *start_pos = ptr - string;
   }
   else
      *start_pos = 0;

   strbuf[len] = '\0';
   return(strbuf);
}

/******************************************************************************/
/* FUNCTION    : find_nest                                                    */
/* DESCRIPTION : To find the nest number of the help button                   */
/* INPUT       : token = the label number token                               */
/* OUTPUT      : the nest number of help button                               */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int find_nest(token)
char *token;
{
   char *ptr;
   int nest=0;

   ptr = token;
   while (*ptr != '\0')
   {
      if (*ptr == '.')                 /* If it is period .                   */
      {
         nest++;                       /* increase nest by 1                  */
      }
      ptr++;
   }
   if (nest ==0)
   {

      ptr = token;
      while (*ptr != '\0')
      {
        if (*ptr != '0')
        {
           nest = 1;
           break;
        }
        ptr++;
      }
   }
   else
   {
      if (*(ptr-1) != '.')
         nest++;
   }
   return(nest);
}

/******************************************************************************/
/* FUNCTION    : create_help_button                                           */
/* DESCRIPTION : create help button and label string                          */
/* INPUT       : parent = the parent widget of help buttons                   */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void create_help_button(parent)
Widget parent;
{
   struct help *help_ptr;
   WidgetList  buttons;
   WidgetList labels;
   int i;

   /* determine how many help buttons to be created */
   help_num=0;                     /* initial number for help button */
   help_ptr = help_first;          /* point to the beginning of help token */
   while (help_ptr != NULL)        /* scan all the help token list */
   {
      help_num++;                  /* increase the help button number by 1 */
      help_ptr = help_ptr->next;   /* point to the next help token */
   }
   buttons = (WidgetList)XtMalloc(sizeof(Widget) * help_num); /* create buttons */
   labels = (WidgetList)XtMalloc(sizeof(Widget) * help_num);  /* create labels */

 /*We create some buttons and labels to browse the sensitive help information */
   ac=0;
   XtSetArg(al[ac],XmNheight, CHINESE_WORD_HEIGHT);ac++;   /* heigh of button */
   XtSetArg(al[ac],XmNwidth, CHINESE_WORD_WIDTH);ac++;     /* width of button */
   XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM);ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNleftOffset,0);  ac++;
   XtSetArg(al[ac],XmNtopOffset,0);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
   XtSetArg(al[ac],XmNshowAsDefault, True); ac++;
   buttons[0] = XtCreateWidget("  ",xmPushButtonWidgetClass,parent, al ,ac);
   XtAddCallback(buttons[0],XmNactivateCallback,PushHelpButtonCB,0);

    ac = 0;
    XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
    XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
    XtSetArg(al[ac],XmNtopOffset,0);  ac++;
    XtSetArg(al[ac],XmNleftOffset,
                    CHINESE_WORD_WIDTH*(help_first->nest+2));  ac++;
    XtSetArg(al[ac],XmNlabelString,
                    XmStringCreate(help_first->label,charset));  ac++;
    labels[0]=XmCreateLabel(parent,"name0",al,ac);

   help_ptr = help_first->next;
   i =1;
   while (help_ptr != NULL)
   {
      /* create button and callback */
      ac=0;
      XtSetArg(al[ac],XmNheight, CHINESE_WORD_HEIGHT);ac++;
      XtSetArg(al[ac],XmNwidth, CHINESE_WORD_WIDTH);ac++;
      XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM);ac++;
      XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
      XtSetArg(al[ac],XmNleftOffset,CHINESE_WORD_WIDTH*help_ptr->nest);  ac++;
      XtSetArg(al[ac],XmNtopOffset,35*i);  ac++;
      XtSetArg(al[ac],XmNshowAsDefault, True); ac++;
      XtSetArg(al[ac],XmNtraversalOn,True); ac++;
      XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
      buttons[i] = XtCreateWidget("  ",xmPushButtonWidgetClass,parent, al ,ac);
      XtAddCallback(buttons[i],XmNactivateCallback,PushHelpButtonCB,i);

      /* Create fix text */
      ac = 0;
      XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
      XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
      XtSetArg(al[ac],XmNtopOffset,35*i);  ac++;
      XtSetArg(al[ac],XmNleftOffset,
                      CHINESE_WORD_WIDTH*(help_ptr->nest+2)); ac++;
      XtSetArg(al[ac],XmNlabelString,
                      XmStringCreate(help_ptr->label,charset));  ac++;
      labels[i]=XmCreateLabel(parent,"name0",al,ac);

      help_ptr = help_ptr->next;
      i++;
   }
   XtManageChildren(buttons,help_num);
   XtManageChildren(labels,help_num);
}

/******************************************************************************/
/* FUNCTION    : HelpMenuCB                                                   */
/* DESCRIPTION : Menu bar item CallBack Routine                               */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      : none                                                         */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   HelpMenuCB(w,client_data,call_data)
Widget     w;
caddr_t    client_data;
caddr_t    call_data;
{
   XmAnyCallbackStruct *cb = (XmAnyCallbackStruct * )call_data;

   switch ((int)client_data)
   {
         case MENU_HELP:
              HelpOkPopUp(help_help_message,help_ok_msg);
              break;
         default:
           fprintf(stderr,"Userdict Help Warning: in menu callback\n");
           break;
   }
}

/******************************************************************************/
/* FUNCTION    : PushHelpButtonCB                                             */
/* DESCRIPTION : get the help information when push one help button.          */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void PushHelpButtonCB(w,client_data,call_data)
Widget w;
caddr_t client_data;
caddr_t call_data;
{
   struct help *help_ptr;
   struct aline *aline_ptr;
   int pos=0;
   int number;
   XmString xmstr;

   number = (int) client_data;            /* help button number */
   help_ptr = help_first->next;           /* point to help token */
   if (help_ptr != NULL)
   {
      if (number != 0)
      {
         while ((help_ptr != NULL) && (help_ptr->number != number))
            help_ptr = help_ptr->next;

         if (help_ptr == NULL)
         {
/*          fprintf(stderr,"This number has no help data !\n");*/
            return;
         }
         aline_ptr = help_ptr->text;
         help_string = (unsigned char *) XtMalloc(help_ptr->length);
         while (aline_ptr != NULL)
         {
            cancate(help_string, aline_ptr->string, &pos);
            aline_ptr = aline_ptr->next;
         }
      }
      else
      {    /* get all help information */
         help_string = (unsigned char *) XtMalloc(help_first->length);
         while (help_ptr != NULL)
         {
            aline_ptr = help_ptr->text;
            while (aline_ptr != NULL)
            {
               cancate(help_string, aline_ptr->string, &pos);
               aline_ptr = aline_ptr->next;
            }
            help_ptr = help_ptr->next;
         }
      }
   }
   else
   {
       help_string = (unsigned char *) XtMalloc(strlen(noHelpInformation)+1);
       strcpy(help_string,noHelpInformation);
   }
   XmTextSetString(text_box,help_string);
   XtFree(help_string);
}

/******************************************************************************/
/* FUNCTION    : free_help_text                                               */
/* DESCRIPTION : free help information data structure                         */
/* INPUT       : none                                                         */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void free_help_text()
{
   struct help *help_ptr;
   struct help *help_next;
   struct aline *aline_ptr;
   struct aline *aline_next;

   /* free all the help information structure */
   help_ptr = help_first;     /* point to the first one of help token */
   while (help_ptr != NULL)
   {
      aline_ptr = help_ptr->text;
      while (aline_ptr != NULL)
      {
         XtFree(aline_ptr->string);
         aline_next = aline_ptr->next;
         XtFree(aline_ptr);
         aline_ptr = aline_next;
       }
       help_next = help_ptr->next;
       XtFree(help_ptr);
       help_ptr = help_next;
    }
}

/******************************************************************************/
/* FUNCTION    : cancate                                                      */
/* DESCRIPTION : add the source string to the pos position of dest string     */
/* INPUT       : dest =                                                       */
/*               source =                                                     */
/*               pos =                                                        */
/* OUTPUT      : none                                                         */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
cancate(dest,source, pos)
char *dest;
char *source;
int *pos;
{
   int i=0;

   while (*(source+i) != '\0')
   {
      *(dest+*pos) = *(source+i);
      (*pos)++;
      i++;
   }
    *(dest+*pos) = '\0';
}

/******************************************************************************/
/* FUNCTION    : CreateHelpMenuBar                                            */
/* DESCRIPTION : Create help MenuBar widgets.                                 */
/* INPUT       : parent = parent Widget.                                      */
/* OUTPUT      : menubar Widget.                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
Widget   CreateHelpMenuBar(parent)
Widget   parent;
{
   Widget    menu_bar1;
   Widget    cascade3,cascade4;
   Widget    menu_pane3,menu_pane4;

   /*******************************/
   /*      Create MenuArea.       */
   /*******************************/
   ac = 0;                                                 /* Create Menu Bar */
   XtSetArg(al[ac],XmNradioBehavior,True); ac++;
   XtSetArg(al[ac],XmNwidth,750); ac++;
   XtSetArg(al[ac],XmNradioBehavior,True); ac++;
   menu_bar1 = XmCreateMenuBar(parent,"help_menu_bar",al,ac);

   /*********************************/
   /*   Create "Quit" PulldownMenu  */
   /*********************************/
   ac = 0;                                      /* Create Quit Pulldown panel */
   menu_pane3 = XmCreatePulldownMenu(menu_bar1,"menu_pane3",al,ac);

   ac = 0;                                              /* Create Quit button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(help_quit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_x); ac++;
   XtSetArg(al[ac],XmNacceleratorText,
                XmStringCreate(m_f3,charset)); ac++;
   XtSetArg(al[ac],XmNaccelerator,m_kf3); ac++;
   quit_button1 = XmCreatePushButton(menu_pane3,"Exit",al,ac);
   XtAddCallback(quit_button1,XmNactivateCallback,HelpCancelCB);
   XtManageChild(quit_button1);

   ac = 0;                                      /* Create Quit cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane3);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(help_exit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_x); ac++;
   cascade3= XmCreateCascadeButton(menu_bar1,"Quit",al,ac);
   XtManageChild(cascade3);

   /*********************************/
   /*   Create "Help" PulldownMenu  */
   /*********************************/
   ac = 0;                                      /* Create Help Pulldown panel */
   menu_pane4 = XmCreatePulldownMenu(menu_bar1,"menu_pane4",al,ac);

   ac = 0;                                              /* Create Help button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(help_help_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_h); ac++;
   help_button1 = XmCreatePushButton(menu_pane4,"Help",al,ac);
   XtAddCallback(help_button1,XmNactivateCallback,HelpMenuCB,MENU_HELP);
   XtManageChild(help_button1);

   ac = 0;                                      /* Create Help cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane4);  ac++;
   XtSetArg(al[ac],XmNlabelString,
            XmStringCreate(help_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_h); ac++;
   cascade4 = XmCreateCascadeButton(menu_bar1,"Help",al,ac);
   XtManageChild(cascade4);

   ac = 0;                                  /* Assign Help widget on Menu bar */
   XtSetArg(al[ac],XmNmenuHelpWidget,cascade4);  ac++;
   XtSetValues(menu_bar1,al,ac);
   return(menu_bar1);
}

/******************************************************************************/
/* FUNCTION    : CreateHelpWorkArea                                           */
/* DESCRIPTION : Create help Work area widget.                                */
/* INPUT       : parent = parent Widget.                                      */
/* OUTPUT      : menubar Widget.                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
Widget CreateHelpWorkArea(parent)
Widget parent;
{
   Widget    scrollWindow;
   Widget    box;

   ac=0;
   XtSetArg(al[ac],XmNrubberPositioning, True); ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM);ac++;
   XtSetArg(al[ac],XmNscrolledWindowMarginHeight, 10);  ac++;
   XtSetArg(al[ac],XmNscrolledWindowMarginWidth, 10);  ac++;
   XtSetArg(al[ac],XmNscrollBarDisplayPolicy, XmAS_NEEDED);  ac++;
   XtSetArg(al[ac],XmNscrollingPolicy, XmAUTOMATIC);  ac++;
/*   XtSetArg(al[ac],XmNvisualPolicy, XmCONSTANT);  ac++;*/
   XtSetArg(al[ac],XmNvisualPolicy, XmVARIABLE);  ac++;
   XtSetArg(al[ac],XmNscrollVertical,True);  ac++;
   XtSetArg(al[ac],XmNscrollHorizontal,True);  ac++;
   XtSetArg(al[ac],XmNheight, 315);  ac++;
   XtSetArg(al[ac],XmNwidth, 750);  ac++;
   XtSetArg(al[ac],XmNtopOffset,30);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   scrollWindow = XmCreateScrolledWindow(parent, "box",al,ac);
   XtManageChild(scrollWindow);

   ac = 0;
   XtSetArg(al[ac],XmNrubberPositioning, True); ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM);ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   box = XmCreateForm(scrollWindow,"box",al,ac);
   XtManageChild(box);
/*   XmAddTabGroup(box);  */  /* 01/21/92 */
   create_help_button(box);

   ac=0;
   XtSetArg(al[ac],XmNworkWindow,box); ac++;
   XtSetValues(scrollWindow,al,ac);
   XmScrolledWindowSetAreas(scrollWindow,NULL,NULL,box);
   return(scrollWindow);
}

/******************************************************************************/
/* FUNCTION    : destroy_handle                                               */
/* DESCRIPTION : destroy handle                                               */
/* INPUT       : widget =                                                     */
/*               client_data =                                                  */
/*               call_data =                                                  */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
static XtCallbackProc destroy_handle(widget,client_data,call_data)
Widget widget;
caddr_t  client_data;
XmAnyCallbackStruct *call_data;
{
  if ((int) client_data == MENU_HELP)
  {
     if (help_flag == True)
     {
         free_help_text();             /* free help information structure */
         help_flag = False;            /* reset the help flag */
     }
  }
  quit_flag = False;                   /* sorry, reset the quit flag here */
}

/******************************************************************************/
/* FUNCTION    : HelpOkPopUp                                                  */
/* DESCRIPTION : PopUp an Ok Dialog Box                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   HelpOkPopUp(title,ok_msg)
char *title;
char *ok_msg;
{                                         /*  Create a one button dialog box  */
   Widget  kid[2];
   int     i;

    ac = 0;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac],XmNnoResize,True); ac++;
    XtSetArg(al[ac],XmNmessageString,
                    XmStringCreate(ok_msg,charset));  ac++;
    XtSetArg(al[ac],XmNokLabelString,
                    XmStringCreate(ok_button_str,charset));  ac++;
    XtSetArg(al[ac], XmNforeground, GetPixel(app_shell, color));  ac++;
    help_dialog = XmCreateInformationDialog(shell1,title,al,ac);
    i = 0;                                          /* Popdown other children */
    kid[i++] = XmMessageBoxGetChild(help_dialog,XmDIALOG_CANCEL_BUTTON);
    kid[i++] = XmMessageBoxGetChild(help_dialog,XmDIALOG_HELP_BUTTON);
    XtUnmanageChildren(kid,i);
    XtAddCallback(help_dialog,XmNokCallback,DialogCancelCB);
    XtManageChild(help_dialog);
}

/******************************************************************************/
/* FUNCTION    : HelpCB                                                       */
/* DESCRIPTION : Help CallBack Routine                                        */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = not used.                                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   HelpCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{

   Widget  workarea1;
   Widget  box1;
   Widget  menu_bar1;

   /***********************************/
   /* Create Help Dialog              */
   /***********************************/

   if (help_flag == FALSE)
      help_flag = True;
   else
      return;
   read_help_file();
   shell1=XtCreateApplicationShell("Window2",topLevelShellWidgetClass,NULL,0);
   ac = 0;
   XtSetArg(al[ac],XmNallowShellResize,False); ac++;
   XtSetArg(al[ac],XmNnoResize,True); ac++;
   XtSetArg(al[ac],XmNshadowThickness, 0); ac++;
   XtSetArg(al[ac],XmNwidth,750);  ac++;
   XtSetArg(al[ac],XmNheight,760);  ac++;
   main1=XmCreateMainWindow(shell1,"help_main",al,ac);
   XtAddCallback(main1,XmNdestroyCallback,destroy_handle,MENU_HELP);
   XtManageChild(main1);

   menu_bar1 = CreateHelpMenuBar(main1);                   /* Create Menu Bar */
   XtManageChild(menu_bar1);

   ac = 0;
   XtSetArg(al[ac],XmNverticalSpacing,40);  ac++;
   box1 = XmCreateForm(main1,"box",al,ac);
   XtManageChild(box1);

   ac = 0;                                               /* Create fixed text */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,0);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                   XmStringCreate(help_button_string,charset));  ac++;
   help_name0=XmCreateLabel(box1,"name0",al,ac);
   XtManageChild(help_name0);

   workarea1 = CreateHelpWorkArea(box1);
   XtManageChild(workarea1);

   ac = 0;                                               /* Create fixed text */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,343);  ac++;
   XtSetArg(al[ac],XmNlabelString,XmStringCreate(help_list_str,charset));  ac++;
   help_name1=XmCreateLabel(box1,"name1",al,ac);
   XtManageChild(help_name1);

   ac=0;
   XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
   XtSetArg(al[ac],XmNcursorPositionVisible,False);  ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM); ac++;
   XtSetArg(al[ac],XmNtopAttachment, XmATTACH_FORM);ac++;
   XtSetArg(al[ac],XmNbottomAttachment, XmATTACH_FORM);ac++;
   XtSetArg(al[ac],XmNscrollBarDisplayPolicy, XmAS_NEEDED);  ac++;
   XtSetArg(al[ac],XmNscrollingPolicy, XmAUTOMATIC);  ac++;
   XtSetArg(al[ac],XmNvisualPolicy, XmCONSTANT);  ac++;
   XtSetArg(al[ac],XmNscrollVertical,True);  ac++;
   XtSetArg(al[ac],XmNscrollHorizontal,True);  ac++;
   XtSetArg(al[ac],XmNeditMode,XmMULTI_LINE_EDIT);  ac++;
   XtSetArg(al[ac],XmNtopWidget,workarea1);  ac++;
   XtSetArg(al[ac],XmNtopOffset,380);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNleftOffset,10);  ac++;
   XtSetArg(al[ac],XmNheight,300);  ac++;
   XtSetArg(al[ac],XmNwidth,750);  ac++;
   text_box = XmCreateScrolledText(box1,"help_text_box",al,ac);
   XmTextSetEditable(text_box,False);
   XtManageChild(text_box);
/*   XmAddTabGroup(text_box);  */  /* 01/21/92 */

   XmMainWindowSetAreas(main1,menu_bar1,NULL,NULL,NULL,box1);
   XtRealizeWidget(shell1);
   XStoreName(XtDisplay(shell1),XtWindow(shell1),help_title);
}

/******************************************************************************/
/* FUNCTION    : ExitCB                                                      */
/* DESCRIPTION :                                                              */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   ExitCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   Widget txt_box;
/*   XtCallbackList  callbacklist; */

   if (quit_flag == False)
      quit_flag = True;
   else
      return;

/*   callbacklist = (XtCallbackList) XtMalloc(sizeof(XtCallbackRec)); */
/*   callbacklist->closure =  "2";                                    */
/*   callbacklist->callback = (XtCallbackProc) destroy_handle;        */
   ac= 0;                                         /*  Create a Exit dialog    */
   XtSetArg(al[ac],XmNautoUnmanage,FALSE); ac++;
   XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
   XtSetArg(al[ac], XmNforeground, GetPixel(app_shell, color));  ac++;
   XtSetArg(al[ac],XmNnoResize,True); ac++;
   XtSetArg(al[ac],XmNokLabelString,
                    XmStringCreate(ok_button_str,charset));  ac++;
   XtSetArg(al[ac],XmNcancelLabelString,
                    XmStringCreate(cancel_button_str,charset));  ac++;
/*   XtSetArg(al[ac],XmNdestroyCallback, callbacklist); ac++;    */
   dialog = XmCreatePromptDialog(app_shell,"Exit_warning",al,ac);
   XtUnmanageChild(XmSelectionBoxGetChild(dialog,XmDIALOG_TEXT));
   XtUnmanageChild(XmSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON));
   ac = 0;
   XtSetArg(al[ac],XmNselectionLabelString,XmStringCreate("",charset)); ac++;
   XtSetValues(dialog,al,ac);
   XtAddCallback(dialog,XmNokCallback,
                           DialogAcceptCB,DIALOG_Q_EXIT);
   XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);

   ac = 0;
   XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac], XmNforeground, GetPixel(app_shell, color));  ac++;
   XtSetArg(al[ac],XmNlabelString,XmStringCreate(exit_msg,charset));  ac++;
   txt_box=XmCreateLabel(dialog,"name1",al,ac);
   XtManageChild(txt_box);
   XtUninstallTranslations(txt_box);            /* Make All translations off  */
   XtManageChild(dialog);
}

/******************************************************************************/
/* FUNCTION    : BeforeInputCB                                               */
/* DESCRIPTION : Modify Verify CallBack Routine for text widget.              */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   BeforeInputCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   XmTextVerifyCallbackStruct *cb = (XmTextVerifyCallbackStruct * )call_data;
   int input_len;
   int insert_len;

   if (TextSetString == True)
   {
       TextSetString = False;
       return;
   }
   if (strlen(cb->text->ptr) < (PHRASE_LEN-1))
      strcpy(buffer, cb->text->ptr);
   else
   {
      strncpy(buffer,cb->text->ptr,(PHRASE_LEN-1));
      buffer[PHRASE_LEN-1] = NULL;
   }
   input_len = strlen(buffer);
   switch((int)client_data)
   {
      case EDIT0_CB:                /* Check Input String is Phonetic string */
            FilterPhonetic(buffer);
            Truncate(edit0, buffer);
            insert_len = strlen(buffer);
            if ( input_len > insert_len)
            {
                 XBell(XtDisplay(edit0),0 );
                 cb->doit = False;
                 if (insert_len > 0)
                    ReInput(edit0, buffer);
            }
            break;
        case EDIT1_CB:                  /* Check Input String is Phrase string */
            FilterPhrase(buffer);
            Truncate(edit1, buffer);
            insert_len = strlen(buffer);
            if ( input_len > insert_len)
            {
                 XBell(XtDisplay(edit0),0 );
                 cb->doit = False;
                 if (insert_len > 0)
                     ReInput(edit1, buffer);
            }
            break;
   }
}

/******************************************************************************/
/* FUNCTION    : ReInput                                                      */
/* DESCRIPTION : input the paste_string to text_widget                        */
/* INPUT       : text_widget =                                                */
/*               paste_string =                                               */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void ReInput(text_widget,paste_string)
Widget text_widget;
char *paste_string;
{
   XmTextPosition cursorPos;

   XtSetArg(al[0],XmNcursorPosition,&cursorPos);      /* Get Cursor Position  */
   XtGetValues(text_widget,al,1);
   XmTextReplace(text_widget,cursorPos,cursorPos,paste_string);
}

/******************************************************************************/
/* FUNCTION    : EUClen                                                       */
/* DESCRIPTION : Get No of EUC Character of Input String.                     */
/* INPUT       : str = EUC code string                                        */
/* OUTPUT      : no of EUC character in the string.                           */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int EUClen(str)
char *str;
{
   int i;
   int ret;

   ret = 0;
   i = 0;
   while( i< strlen(str))      /* may be use mslen(str) NLS function to work. */
   {
      if( str[i] == 0x8e )                /* check character begin with 0x8e  */
      {
          ret ++;
          i += 4;
      }
      else if( str[i] >= 0xa1 )              /* check character begin >= 0xa1 */
      {
          ret ++;
          i += 2;
      }
      else
      {
          ret ++;                            /* Single byte */
          i++;
      }
   }
   return ( ret );
}

/****************************************************************************/
/*FUNCTION    : FilterPhonetic                                              */
/*DESCRIPTION : to get the phonetic code                                    */
/*INPUT       : Phonetic                                                    */
/*OUTPUT      : none                                                        */
/****************************************************************************/
void FilterPhonetic(phonetic)
char *phonetic;                        /* pointer to phonetic code            */
{
   int ptr = 0;
   int ph_ptr = 0;
   int counter =0;

   while ( *(phonetic+ptr) != NULL)
   {
      counter++;
      if(*(phonetic+ptr) >0x80)
         *(phonetic+(ph_ptr++)) = *(phonetic+(ptr++));
      else
         ptr++;
   }
   *(phonetic+ph_ptr) = '\0';

   ptr =0;
   ph_ptr =0;
   while ( ptr < counter)
   {
      /* 0xa5 is the first byte of all phonetic code */
      if ( *(phonetic+ptr) == 0xa5)
      {
         /* second byte of phonetic code is between 0xc7 to 0xf0 */
         if ((*(phonetic+ptr+1) >= 0xc7) && (*(phonetic+ptr+1) <= 0xf0) &&
             (*(phonetic+ptr+1) != 0xed))
         {
            *(phonetic+(ph_ptr++)) = *(phonetic+(ptr++));
            *(phonetic+(ph_ptr++)) = *(phonetic+(ptr++));
         }
         else
            ptr +=2;
      }
      else if (*(phonetic+ptr) >= 0xa1)
         ptr +=2;
      else if (*(phonetic+ptr) == 0x8e)
         ptr +=4;
      else
         ptr++;
   }
   *(phonetic+ph_ptr) = '\0';          /* set end mark of string              */
}
/****************************************************************************/
/*FUNCTION    : FilterPhrase                                                */
/*DESCRIPTION : to get the phrase code                                      */
/*INPUT       : Phrase                                                      */
/*OUTPUT      : none                                                        */
/****************************************************************************/
void FilterPhrase(phrase)
char *phrase;                        /* pointer to phrase code            */
{
   int ptr = 0;
   int ph_ptr = 0;
   int input_len ;

   while ( *(phrase+ptr) != NULL)
   {
      if(*(phrase+ptr) >0x80)
         *(phrase+(ph_ptr++)) = *(phrase+(ptr++));
      else
         ptr++;
   }
   *(phrase+ph_ptr) = '\0';          /* set end mark of string              */
   if ( (input_len=strlen(phrase))%2 != 0 )          /* Check input len is even  */
      *(phrase+ph_ptr-1) = '\0';          /* set end mark of string              */
}

/****************************************************************************/
/*FUNCTION    : Truncate                                                    */
/*DESCRIPTION : Truncate the str to paste into text widget                  */
/*INPUT       : Phrase                                                      */
/*OUTPUT      : none                                                        */
/****************************************************************************/
void Truncate(text_widget, str)
Widget text_widget;
char *str;
{
   int current_len;
   int insert_len;
   int counter;
   int ptr,ph_ptr;

   current_len = EUClen(XmTextGetString(text_widget));      /* Use mslen() */
   insert_len = EUClen(str);
   if( text_widget == edit0 )              /* Check MBCS len whether will  */
   {                                          /* overflow ?                   */
      if (current_len+insert_len<=MAX_PHONETIC_NUM)/* Check whether exceed len  */
          return;
      else
         insert_len = MAX_PHONETIC_NUM - current_len;
   }
   else if ( text_widget == edit1 )
   {
      if (current_len+insert_len<= MAX_PHRASE_NUM) /* Check whether exceed len  */
         return;
      else
        insert_len = MAX_PHRASE_NUM - current_len;
   }
   else
      return;

   for (ptr=0,counter=0;counter < insert_len ; counter++) {
      if (*(str+ptr) == 0x8e)
      {
         if ((ptr+4) <= strlen(str))
            ptr +=4;
         else
         {
            ptr +=2;
            break;
         }
      }
      else
         ptr +=2;
   }
   *(str+ptr) = '\0';
}

/******************************************************************************/
/* FUNCTION    : IsPhoneticStr                                                */
/* DESCRIPTION : Check Input String Whether is Phonetic String.               */
/* INPUT       : str = phonetic entry string                                  */
/* OUTPUT      : True or False                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int   IsPhoneticStr(str)
char *str;
{
   int i;
   int ret;
   int input_len;

   if ( (input_len=strlen(str))%2 != 0 )          /* Check input len is even  */
        return(False);

   if ( input_len == 0 )                           /* Check input len is Zero */
        return(True);

   ret = True;
   i = 0;
   while( i<input_len && ret==True )
   {
      if( str[i] == 0xa5 )                         /* Check Phonetic EUC code */
          if( str[i+1]!=0xed && str[i+1]>=0xc7 && str[i+1]<=0xf0 )
             i+=2;
          else
             ret = False;
      else
         ret = False;
   }
   return( ret );
}
/******************************************************************************/
/* FUNCTION    : IsPhraseStr                                                  */
/* DESCRIPTION : Check Input String Whether is Phrase String.                 */
/* INPUT       : str = phrase entry string                                    */
/* OUTPUT      : True or False                                                */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
int   IsPhraseStr(str)
char *str;
{
   int i;
   int ret;
   int input_len;

   if ( (input_len=strlen(str))%2 != 0 )          /* Check input len is even  */
        return(False);

   if ( input_len == 0 )                           /* Check input len is Zero */
        return(True);

   ret = True;
   i = 0;
   while( i < input_len && ret == True )             /* Check Phrase EUC code */
   {
      if( str[i]==0x8e )
          i += 4;
      else if( str[i]>=0xa1 )
          i += 2;
      else
         ret = False;
   }
   return( ret );
}
/******************************************************************************/
/* FUNCTION    : CheckEditCB                                                  */
/* DESCRIPTION : CheckEdit CallBack Routine for text widget.                  */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   CheckEditCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   char *tmp0=NULL;
   char *tmp1=NULL;
   int   len,len0,len1;

   /************************************************/
   /*  Check Edit Pull Down Menu Button Sensitive  */
   /************************************************/
   tmp0=XmTextGetSelection(edit0);  /* Get Selection String in phonetic_entry */
   len0 = strlen(tmp0);
   tmp1=XmTextGetSelection(edit1);  /* Get Selection String in phrase_entry   */
   len1=strlen(tmp1);
   if ( len0 != 0 || len1 != 0 )               /* Check Whether Has Selection */
   {
      if ( len0 != 0 )                              /* Get Selection Widget   */
         select_widget = edit0;
      else if ( len1 != 0 )
         select_widget = edit1;                       /* Get Selection String */
      select_string = XmTextGetSelection(select_widget);
      XtSetSensitive(cut_button,True);        /* Set Cut,Copy,Clear buttons   */
      XtSetSensitive(copy_button,True);       /* Sensitive    */
      XtSetSensitive(clear_button,True);
   }
   else
   {
      XtSetSensitive(cut_button,False);      /* Set buttons unsensitive  */
      XtSetSensitive(copy_button,False);
      XtSetSensitive(clear_button,False);
   }
   if( strlen(clip_string) != 0 ) /* Check Paste button Sensitive */
      XtSetSensitive(paste_button,True);
   else
      XtSetSensitive(paste_button,False);
   if( strlen(XmTextGetString(edit0)) != 0 && open_flag == True )
                                             /* Check Search button Sensitive */
      XtSetSensitive(search_button,True);
   else
      XtSetSensitive(search_button,False);
}
/******************************************************************************/
/* FUNCTION    : SingleClickCB                                                */
/* DESCRIPTION : mouse single click action CallBack routine.                  */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   SingleClickCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   XmListCallbackStruct *cb = (XmListCallbackStruct * )call_data;
   XmListWidget lw3 = (XmListWidget) edit3;

   switch((int)client_data)
   {
      case  EDIT2_CB :
            XtSetKeyboardFocus(app_shell,edit2);           /* Set Input Focus */
            break;
      case  EDIT3_CB :
            obj->phrase_select->highlight = cb->item_position;
            lw3->list.CurrentKbdItem = obj->phrase_select->highlight-1;
            edit_f[3]= True;
            XtSetKeyboardFocus(app_shell,edit3);           /* Set Input Focus */
            break;
   }
}
/******************************************************************************/
/* FUNCTION    : DoubleClickCB                                                */
/* DESCRIPTION : Mouse Double Click action CallBack Routine                   */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   DoubleClickCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   XmListCallbackStruct *cb =  (XmListCallbackStruct * )call_data;
   XmListWidget lw3 = (XmListWidget) edit3;

   switch((int)client_data)
   {

      case  EDIT2_CB :                      /* Copy String to Phonetic_entry  */
            TextSetString = True;
            XmTextSetString(edit0,obj->phonetic_view);
            XtSetKeyboardFocus(app_shell,edit2);           /* Set Input Focus */
            break;
      case  EDIT3_CB :                        /* Copy String to Phrase_entry  */
            obj->phrase_select->highlight = cb->item_position;
            lw3->list.CurrentKbdItem = obj->phrase_select->highlight-1;
            TextSetString = True;
            XmTextSetString(edit1,obj->phrase_select->candidate[
                                  obj->phrase_select->highlight-1]);
            edit_f[3]= True;
            XtSetKeyboardFocus(app_shell,edit3);           /* Set Input Focus */
            break;
   }
}
/******************************************************************************/
/* FUNCTION    : DialogCancelCB                                               */
/* DESCRIPTION : Cancel Dialog box CallBack Routine                           */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   DialogCancelCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   XtUnmanageChild(w);  /* Pop down dialog */
   quit_flag = False;   /* reset the quit flag */
}

/******************************************************************************/
/* FUNCTION    : HelpCancelCB                                                 */
/* DESCRIPTION : Cancel Dialog box CallBack Routine                           */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   HelpCancelCB(w,client_data,call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   help_flag = False;
   free_help_text();
   XtDestroyWidget(shell1);
}

/******************************************************************************/
/* FUNCTION    : OkPopUp                                                      */
/* DESCRIPTION : PopUp an Ok Dialog Box                                       */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   OkPopUp(title,ok_msg,beep)
char *title;
char *ok_msg;
int   beep;
{                                         /*  Create a one button dialog box  */
   Widget  kid[2];
   int     i;

    ac = 0;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac],XmNnoResize,True); ac++;
    XtSetArg(al[ac],XmNmessageString,
                    XmStringCreate(ok_msg,charset));  ac++;
    XtSetArg(al[ac],XmNokLabelString,
                    XmStringCreate(ok_button_str,charset));  ac++;
    XtSetArg(al[ac], XmNforeground, GetPixel(app_shell, color));  ac++;
    if ( beep == OK )
         dialog = XmCreateInformationDialog(edit1,title,al,ac);
    else
         dialog = XmCreateErrorDialog(edit1,title,al,ac);
    i = 0;                                          /* Popdown other children */
    kid[i++] = XmMessageBoxGetChild(dialog,XmDIALOG_CANCEL_BUTTON);
    kid[i++] = XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON);
    XtUnmanageChildren(kid,i);
    XtAddCallback(dialog,XmNokCallback,DialogCancelCB);
    XtManageChild(dialog);
    if ( beep != OK)                     /* Test whether need to beep */
           XBell(XtDisplay(dialog),0);
}

/******************************************************************************/
/* FUNCTION    : DialogAcceptCB                                               */
/* DESCRIPTION : Dialog box accept action CallBack Routine.                   */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   DialogAcceptCB (w, client_data, call_data)
Widget    w;
caddr_t   client_data;
caddr_t   call_data;
{
   XmFileSelectionBoxCallbackStruct *fcb =
            (XmFileSelectionBoxCallbackStruct *) call_data;
   XmFileSelectionBoxCallbackStruct *scb =
            (XmFileSelectionBoxCallbackStruct *) call_data;
   char   *file_str;
   int ret;

   switch ((int)client_data)
   {
      case DIALOG_OPEN:
           if (filename != NULL)             /* Get open dictionary file name */
           {
              XtFree(filename);
              filename = NULL;
           }
           XmStringGetLtoR(fcb->value,charset,&filename);
           strcpy(obj->open_entry,filename );
           XtUnmanageChild(w);
           if ( (ret=UDEProcess(obj,Open_Dict))==OK )  /* Open OK  */
           {
              open_flag = True;
              file_str=(char *)XtMalloc(PATHLEN);
              strcpy(file_str,file_title);
              strcat(file_str,filename);          /* Set string in Title bar  */
              XStoreName(XtDisplay(app_shell),XtWindow(app_shell),file_str);
              XtFree(file_str);
              XtSetSensitive(merge_button,True);         /* Set Merge button  */
              OkPopUp(open_file_ok,ok_file_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(open_file_error,err_msg(ret),ret);
           break;
      case DIALOG_MERGE:
           if (filename != NULL)            /* Get merge dictionary file name */
           {
              XtFree(filename);
              filename = NULL;
           }
           XmStringGetLtoR(scb->value,charset,&filename);
           strcpy(obj->merge_entry,filename);
           XtUnmanageChild(w);
           if((ret=UDEProcess(obj,Merge_Dict))==OK)  /* Merge OK */
              OkPopUp(merge_file_ok,ok_merge_msg,ret);
           else
              OkPopUp(merge_file_error,err_msg(ret),ret);
           break;
      case DIALOG_Q_U_PHN:                      /* Dialog for Update Phonetic */
           XtUnmanageChild(w);
           strcpy(obj->phonetic_entry,XmTextGetString(edit0));
           if ((ret=UDEProcess(obj,Update_Phonetic))==OK)  /* Update OK */
           {
              undo_flag = True;
              OkPopUp(update_phonetic_ok,ok_u_phn_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(update_phonetic_error,err_msg(ret),ret);
           break;
      case DIALOG_Q_U_PHR:                        /* Dialog for Update Phrase */
           XtUnmanageChild(w);
           strcpy(obj->phrase_entry ,XmTextGetString(edit1));
           if ( (ret=UDEProcess(obj,Update_Phrase))==OK )   /* Update OK */
           {
              undo_flag = True;
              OkPopUp(update_phrase_ok,ok_u_phr_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(update_phrase_error,err_msg(ret),ret);
           break;
      case DIALOG_Q_D_PHN:                      /* Dialog for Delete Phonetic */
           XtUnmanageChild(w);
           if ((ret=UDEProcess(obj,Delete_Phonetic))==OK)  /* Delete Ok */
           {
              undo_flag = True;
              OkPopUp(delete_phonetic_ok,ok_d_phn_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(delete_phonetic_error,err_msg(ret),ret);
           break;
      case DIALOG_Q_D_PHR:                      /* Dialog for Delete Phrase   */
           XtUnmanageChild(w);
           if ((ret=UDEProcess(obj,Delete_Phrase))==OK)  /* Delete OK  */
           {
              undo_flag = True;
              OkPopUp(delete_phrase_ok,ok_d_phr_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(delete_phrase_error,err_msg(ret),ret);
           break;
      case DIALOG_Q_EXIT:                          /* Dialog for Exit Program */
           UDEDestroy(obj);
           freeCursors();
           XtCloseDisplay(XtDisplay(app_shell));
           exit(0);
           break;
      default:
           fprintf(stderr,"Userdict Warning: Accept callback nothing.\n");
           break;
   }
}

/******************************************************************************/
/* FUNCTION    : MenuCB                                                       */
/* DESCRIPTION : Menu bar item CallBack Routine                               */
/* INPUT       : w = Calling widget.                                          */
/*               client_data = passing by calling procedure.                  */
/*               call_data = provided by widget class.                        */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
void   MenuCB(w,client_data,call_data)
Widget     w;
caddr_t    client_data;
caddr_t    call_data;
{
   int   ret;
   int   i;
   Widget  kid[5];
   XmAnyCallbackStruct *cb = (XmAnyCallbackStruct * )call_data;

   switch ((int)client_data)
   {
      case MENU_A_PHR:          /* Add Phrase button in Commit Pull down Menu */
           strcpy(obj->phonetic_entry,XmTextGetString(edit0));
           strcpy(obj->phrase_entry,XmTextGetString(edit1));
           if ( (ret=UDEProcess(obj, Add_Phrase))==OK )  /* Add OK */
           {
              undo_flag = True;
              OkPopUp(add_phrase_ok,ok_a_phr_msg,ret);
           }                                                 /* PopUp Message */
           else
              OkPopUp(add_phrase_error,err_msg(ret),ret);
           break;
      case MENU_U_PHN:     /* Update Phonetic button in Commit Pull down Menu */
           dialog = QuestionPopUp(update_phonetic,u_phn_d_msg);
           XtAddCallback(dialog,XmNokCallback,
                                DialogAcceptCB,DIALOG_Q_U_PHN);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           break;
      case MENU_U_PHR:       /* Update Phrase button in Commit Pull down Menu */
           dialog = QuestionPopUp(update_phrase,u_phr_d_msg);
           XtAddCallback(dialog,XmNokCallback,
                                DialogAcceptCB,DIALOG_Q_U_PHR);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           break;
      case MENU_D_PHN:     /* Delete Phonetic button in Commit Pull down Menu */
           dialog = QuestionPopUp(delete_phonetic,d_phn_d_msg);
           XtAddCallback(dialog,XmNokCallback,
                                DialogAcceptCB,DIALOG_Q_D_PHN);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           break;
      case MENU_D_PHR:       /* Delete Phrase button in Commit Pull down Menu */
           dialog = QuestionPopUp(delete_phrase,d_phr_d_msg);
           XtAddCallback(dialog,XmNokCallback,
                                DialogAcceptCB,DIALOG_Q_D_PHR);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           break;
      case MENU_OPEN:                   /* Open button in File Pull down Menu */
           ac=0;
           XtSetArg(al[ac],XmNselectionLabelString,
                    XmStringCreate(open_msg,charset));  ac++;
           XtSetArg(al[ac],XmNinput, True); ac++;
           XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
           XtSetArg(al[ac],XmNnoResize,True); ac++;
           XtSetArg(al[ac],XmNokLabelString,
                            XmStringCreate(ok_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNcancelLabelString,
                            XmStringCreate(cancel_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNapplyLabelString,
                            XmStringCreate(filter_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNlistLabelString,
                            XmStringCreate(file_str,charset));  ac++;
           XtSetArg(al[ac],XmNfilterLabelString,
                            XmStringCreate(filter_label_str,charset));  ac++;
           dialog = XmCreateFileSelectionDialog(edit0,
                                   file_selection_dialog,al,ac);
           XtAddCallback(dialog,XmNokCallback,
                                      DialogAcceptCB,DIALOG_OPEN);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           i = 0;                                  /* Popdown other children */
           kid[i++] = XmFileSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON);
           XtUnmanageChildren(kid,i);
           break;
      case MENU_MERGE:                 /* Merge button in File Pull down Menu */
           ac = 0;
           XtSetArg(al[ac],XmNselectionLabelString,
                    XmStringCreate(merge_msg,charset));  ac++;
           XtSetArg(al[ac],XmNinput, True); ac++;
           XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
           XtSetArg(al[ac],XmNnoResize,True); ac++;
           XtSetArg(al[ac],XmNokLabelString,
                            XmStringCreate(ok_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNcancelLabelString,
                            XmStringCreate(cancel_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNapplyLabelString,
                            XmStringCreate(filter_button_str,charset));  ac++;
           XtSetArg(al[ac],XmNlistLabelString,
                            XmStringCreate(file_str,charset));  ac++;
           XtSetArg(al[ac],XmNfilterLabelString,
                            XmStringCreate(filter_label_str,charset));  ac++;
           dialog = XmCreateFileSelectionDialog(edit0,
                                    "Merge_File_Dialog",al,ac);
           XtAddCallback(dialog,XmNokCallback,
                                   DialogAcceptCB,DIALOG_MERGE);
           XtAddCallback(dialog,XmNcancelCallback,DialogCancelCB);
           XtManageChild(dialog);
           i = 0;                                  /* Popdown other children */
           kid[i++] = XmFileSelectionBoxGetChild(dialog,XmDIALOG_HELP_BUTTON);
           XtUnmanageChildren(kid,i);
           break;
      case MENU_SEARCH:               /* Search button in Edit Pull down Menu */
           if ( strlen(XmTextGetString(edit0)) == 0 )
               break;
           strcpy(obj->phonetic_entry,XmTextGetString(edit0));
           if( (ret=UDEProcess(obj, Phonetic_Search))== OK )
              undo_flag = True;
           else
              OkPopUp(search_warning,err_msg(ret),ret);
           break;
      case MENU_CUT:                     /* Cut button in Edit Pull down Menu */
           if( CopyProc()==True )
                XmTextRemove(select_widget);
           else
              XmTextClearSelection(select_widget,cb->event->xbutton.time);
           break;
      case MENU_COPY:                   /* Copy button in Edit Pull down Menu */
           CopyProc();
           XmTextClearSelection(select_widget,cb->event->xbutton.time);
           break;
      case MENU_PASTE:                 /* Paste button in Edit Pull down Menu */
           PasteProc();
           XmTextClearSelection(select_widget,cb->event->xbutton.time);
           break;
      case MENU_CLEAR:                 /* Clear button in Edit Pull down Menu */
           if ( IsPhraseStr(select_string) == False ) /* Test selection string is     */
           {                                          /* Phrase String ?              */
                XBell(XtDisplay(app_shell),0);         /* Beep */
                XmTextClearSelection(select_widget,cb->event->xbutton.time);
           }
           else
                XmTextRemove(select_widget);
           break;
      case MENU_UNDO:                   /* Undo button in Edit Pull down Menu */
           if ( current_widget == edit0 )
           {
              TextSetString = True;
              XmTextSetString(current_widget,obj->phonetic_view);
           }
           else if ( current_widget == edit1 )
           {
              TextSetString = True;
              if ( obj->phrase_select->highlight > 0 )
                 XmTextSetString(current_widget,obj->phrase_select->candidate[
                                 obj->phrase_select->highlight-1]);
              else
                 XmTextSetString(current_widget,NULL);
           }
           else
              fprintf(stderr,"UserDict Warning : Undo nothing.\n");
           break;
      default:
           fprintf(stderr,"Userdict Warning: in menu callback\n");
           break;
   }
}

/******************************************************************************/
/* FUNCTION    : QuestionPopUp                                                */
/* DESCRIPTION : PopUp an Question Dialog Box                                 */
/* INPUT       :                                                              */
/* OUTPUT      :                                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
Widget  QuestionPopUp(title,message)
char *title;
char *message;
{                                         /*  Create a one button dialog box  */
   Widget  dialog,kid;

    ac = 0;
    XtSetArg(al[ac],XmNdialogStyle,XmDIALOG_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac],XmNnoResize,True); ac++;
    XtSetArg(al[ac],XmNmessageString,
                    XmStringCreate(message,charset));  ac++;
    XtSetArg(al[ac],XmNokLabelString,
                    XmStringCreate(ok_button_str,charset));  ac++;
    XtSetArg(al[ac],XmNcancelLabelString,
                    XmStringCreate(cancel_button_str,charset));  ac++;
    XtSetArg(al[ac], XmNforeground, GetPixel(app_shell, color));  ac++;
    dialog = XmCreateQuestionDialog(edit1,title,al,ac);
    kid = XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON);
    XtUnmanageChild(kid);
    return( dialog );
}
/******************************************************************************/
/* FUNCTION    : CreateMenuBar                                                */
/* DESCRIPTION : Create MenuBar widgets.                                      */
/* INPUT       : parent = parent Widget.                                      */
/* OUTPUT      : menubar Widget.                                              */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
Widget   CreateMenuBar(parent)
Widget   parent;
{
   Widget    menu_bar;
   Widget    cascade0,cascade1,cascade2,cascade3,cascade4;
   Widget    menu_pane0,menu_pane1,menu_pane2,menu_pane3,menu_pane4;

   /*******************************/
   /*      Create MenuArea.       */
   /*******************************/
   ac = 0;                                                 /* Create Menu Bar */
   XtSetArg(al[ac],XmNradioBehavior,True); ac++;
   menu_bar = XmCreateMenuBar(parent,"menu_bar",al,ac);

   /************************************/
   /*   Create "Commit" PulldownMenu   */
   /************************************/
   ac = 0;                                    /* Create Commit Pulldown panel */
   menu_pane0 = XmCreatePulldownMenu(menu_bar,"menu_pane0",al,ac);

   ac = 0;                                        /* Create Add Phrase button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(a_phr_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_a); ac++;
   a_phr_button = XmCreatePushButton(menu_pane0,"Add Phrase",al,ac);
   XtAddCallback(a_phr_button,XmNactivateCallback,MenuCB,MENU_A_PHR);
   XtManageChild(a_phr_button);
   XtSetSensitive(a_phr_button,False);

   ac = 0;                                   /* Create Update Phonetic button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(u_phn_str, charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_b); ac++;
   u_phn_button = XmCreatePushButton(menu_pane0,"Update Phonetic",al,ac);
   XtAddCallback(u_phn_button,XmNactivateCallback,MenuCB,MENU_U_PHN);
   XtManageChild(u_phn_button);
   XtSetSensitive(u_phn_button,False);

   ac = 0;                                     /* Create Update Phrase button */
   XtSetArg(al[ac],XmNlabelString,
                XmStringCreate(u_phr_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_c); ac++;
   u_phr_button = XmCreatePushButton(menu_pane0,"Update Phrase",al,ac);
   XtAddCallback(u_phr_button,XmNactivateCallback,MenuCB,MENU_U_PHR);
   XtManageChild(u_phr_button);
   XtSetSensitive(u_phr_button,False);

   ac = 0;                                   /* Create Delete Phonetic button */
   XtSetArg(al[ac],XmNlabelString,
               XmStringCreate(d_phn_str,charset)); ac++;
   XtSetArg(al[ac], XmNmnemonic,m_d); ac++;
   d_phn_button = XmCreatePushButton(menu_pane0,"Update Phonetic",al,ac);
   XtAddCallback (d_phn_button,XmNactivateCallback,MenuCB,MENU_D_PHN);
   XtManageChild (d_phn_button);
   XtSetSensitive(d_phn_button,False);

   ac = 0;                                     /* Create Delete Phrase button */
   XtSetArg(al[ac],XmNlabelString,
              XmStringCreate(d_phr_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_e); ac++;
   d_phr_button = XmCreatePushButton(menu_pane0,"Update Phonetic",al,ac);
   XtAddCallback(d_phr_button,XmNactivateCallback,MenuCB,MENU_D_PHR);
   XtManageChild(d_phr_button);
   XtSetSensitive(d_phr_button,False);

   ac = 0;                                    /* Create Commit cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane0);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(commit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_s); ac++;
   cascade0= XmCreateCascadeButton(menu_bar,"Commit",al,ac);
   XtManageChild(cascade0);

   /*********************************/
   /*   Create "Edit" PulldownMenu  */
   /*********************************/
   ac = 0;                                      /* Create Edit Pulldown panel */
   menu_pane1 = XmCreatePulldownMenu(menu_bar,"menu_pane1",al,ac);

   ac = 0;                                               /* Create Cut button */
   XtSetArg(al[ac],XmNlabelString,
               XmStringCreate(cut_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_t); ac++;
   cut_button = XmCreatePushButton(menu_pane1,"Cut",al,ac);
   XtAddCallback(cut_button,XmNactivateCallback,MenuCB,MENU_CUT);
   XtManageChild(cut_button);
   XtSetSensitive(cut_button,False);

   ac = 0;                                              /* Create Copy button */
   XtSetArg(al[ac],XmNlabelString,
               XmStringCreate(copy_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_c); ac++;
   copy_button = XmCreatePushButton(menu_pane1,"Copy",al,ac);
   XtAddCallback(copy_button,XmNactivateCallback,MenuCB,MENU_COPY);
   XtManageChild(copy_button);
   XtSetSensitive(copy_button,False);

   ac = 0;                                             /* Create Clear button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(clear_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_r); ac++;
   clear_button = XmCreatePushButton(menu_pane1,"Clear",al,ac);
   XtAddCallback(clear_button,XmNactivateCallback,MenuCB,MENU_CLEAR);
   XtManageChild(clear_button);
   XtSetSensitive(clear_button,False);

   ac = 0;                                             /* Create Paste button */
   XtSetArg(al[ac],XmNlabelString,
   XmStringCreate(paste_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_p); ac++;
   paste_button = XmCreatePushButton(menu_pane1,"Paste",al,ac); ac++;
   XtAddCallback(paste_button,XmNactivateCallback,MenuCB,MENU_PASTE);
   XtManageChild(paste_button);
   XtSetSensitive(paste_button,False);

   ac = 0;                                              /* Create Undo button */
   XtSetArg(al[ac],XmNlabelString,
   XmStringCreate(undo_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_u); ac++;
   undo_button = XmCreatePushButton(menu_pane1,"Undo",al,ac);
   XtAddCallback(undo_button,XmNactivateCallback,MenuCB,MENU_UNDO);
   XtManageChild(undo_button);
   XtSetSensitive(undo_button,False);

   ac = 0;                                            /* Create Search button */
   XtSetArg(al[ac],XmNlabelString,
   XmStringCreate(search_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_s); ac++;
   search_button = XmCreatePushButton(menu_pane1,"Search",al,ac);
   XtAddCallback(search_button,XmNactivateCallback,MenuCB,MENU_SEARCH);
   XtManageChild(search_button);
   XtSetSensitive(search_button,False);

   ac = 0;                                      /* Create Edit cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane1);  ac++;
   XtSetArg(al[ac],XmNlabelString,
        XmStringCreate(edit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_e); ac++;
   cascade1= XmCreateCascadeButton(menu_bar,"Edit",al,ac);
   XtManageChild(cascade1);

   /********************************/
   /*  Create "File" PulldownMenu  */
   /********************************/
   ac = 0;                                      /* Create File Pulldown panel */
   menu_pane2 = XmCreatePulldownMenu(menu_bar,"menu_pane2",al,ac);

   ac = 0;                                              /* Create Open button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(open_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_o); ac++;
   open_button = XmCreatePushButton(menu_pane2,"Open",al,ac);
   XtAddCallback(open_button,XmNactivateCallback,MenuCB,MENU_OPEN);
   XtManageChild(open_button);

   ac = 0;                                             /* Create Merge button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(merge_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_m); ac++;
   merge_button = XmCreatePushButton(menu_pane2,"New",al,ac);
   XtAddCallback(merge_button,XmNactivateCallback,MenuCB,MENU_MERGE);
   XtManageChild(merge_button);
   XtSetSensitive(merge_button,False);

   ac = 0;                                      /* Create File cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane2);  ac++;
   XtSetArg(al[ac],XmNlabelString,
            XmStringCreate(file_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_f); ac++;
   cascade2 = XmCreateCascadeButton(menu_bar,"File",al,ac);
   XtManageChild(cascade2);

   /*********************************/
   /*   Create "Quit" PulldownMenu  */
   /*********************************/
   ac = 0;                                      /* Create Quit Pulldown panel */
   menu_pane3 = XmCreatePulldownMenu(menu_bar,"menu_pane3",al,ac);

   ac = 0;                                              /* Create Quit button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(quit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_x); ac++;
   XtSetArg(al[ac],XmNacceleratorText,
                XmStringCreate(m_f3,charset)); ac++;
   XtSetArg(al[ac],XmNaccelerator,m_kf3); ac++;
   quit_button = XmCreatePushButton(menu_pane3,"Exit",al,ac);
   XtAddCallback(quit_button,XmNactivateCallback,ExitCB);
   XtManageChild(quit_button);

   ac = 0;                                            /* Create Resume button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(resume_str,charset)); ac++;
   XtSetArg(al[ac], XmNmnemonic,m_r); ac++;
   resume_button = XmCreatePushButton(menu_pane3,"Resume",al,ac);
   XtManageChild(resume_button);

   ac = 0;                                      /* Create Quit cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane3);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(exit_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_x); ac++;
   cascade3= XmCreateCascadeButton(menu_bar,"Quit",al,ac);
   XtManageChild(cascade3);

   /*********************************/
   /*   Create "Help" PulldownMenu  */
   /*********************************/
   ac = 0;                                      /* Create Help Pulldown panel */
   menu_pane4 = XmCreatePulldownMenu(menu_bar,"menu_pane4",al,ac);

   ac = 0;                                              /* Create Help button */
   XtSetArg(al[ac],XmNlabelString,
                 XmStringCreate(help_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_h); ac++;
   XtSetArg(al[ac],XmNacceleratorText,
                XmStringCreate(m_f1,charset)); ac++;
   XtSetArg(al[ac],XmNaccelerator,m_kf1); ac++;
   help_button = XmCreatePushButton(menu_pane4,"Help",al,ac);
   XtAddCallback(help_button,XmNactivateCallback,HelpCB);
   XtManageChild(help_button);

   ac = 0;                                      /* Create Help cascade button */
   XtSetArg(al[ac],XmNsubMenuId,menu_pane4);  ac++;
   XtSetArg(al[ac],XmNlabelString,
            XmStringCreate(help_str,charset)); ac++;
   XtSetArg(al[ac],XmNmnemonic,m_h); ac++;
   cascade4 = XmCreateCascadeButton(menu_bar,help_str,al,ac);
   XtManageChild(cascade4);

   ac = 0;                                  /* Assign Help widget on Menu bar */
   XtSetArg(al[ac],XmNmenuHelpWidget,cascade4);  ac++;
   XtSetValues(menu_bar,al,ac);

   return(menu_bar);
}
/******************************************************************************/
/* FUNCTION    : CreateWorkArea                                               */
/* DESCRIPTION : Create Client Area widgets.                                  */
/* INPUT       : parent = parent Widget                                       */
/* OUTPUT      : Work Area Widget                                             */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/
Widget   CreateWorkArea(parent)
Widget   parent;
{
   Widget    box;
   Widget    frame0, frame1, frame2, frame3;
   Widget    frame2a, frame2b, frame3a, frame3b;
   Widget    name0,name1,name2,name3;
   Widget    kid[10];

   ac = 0;                                          /* Create outer Form box. */
   XtSetArg(al[ac],XmNverticalSpacing,40);  ac++;
   box = XmCreateForm(parent,"box",al,ac);
   XtManageChild(box);

   ac = 0;                                           /* Create Top most frame */
   XtSetArg(al[ac],XmNrubberPositioning,True);  ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame0 = XmCreateForm(box,"frame0",al,ac);
   XtManageChild(frame0);

   ac = 0;                                               /* Create fixed text */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                   XmStringCreate(phonetic_entry_str,charset));  ac++;
   name0=XmCreateLabel(frame0,"name0",al,ac);
   XtManageChild(name0);

   ac = 0;                                /* Create Phonetic_entry text frame */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNleftWidget,name0);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNcolumns,MAX_PHONETIC_NUM*2);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
   XtSetArg(al[ac],XmNeditable, True); ac++;
   XtSetArg(al[ac],XmNinput, True); ac++;
   XtSetArg(al[ac],XmNeditMode, XmSINGLE_LINE_EDIT); ac++;
   XtSetArg(al[ac],XmNautoShowCursorPosition, True); ac++;
   edit0=XmCreateText(frame0,"edit0",al,ac);
   XtManageChild(edit0);
/*   XmAddTabGroup(edit0);   */  /* 01/21/92 */
/*   XmTextSetMaxLength(edit0,MAX_PHONETIC_NUM);*/ /* for NLS  */
   XtAddEventHandler(edit0,FocusChangeMask,False,ChangeFocusEH,EDIT0_EH);
   XtAddCallback(edit0,XmNactivateCallback,MenuCB,MENU_SEARCH);
   XtAddCallback(edit0,XmNmodifyVerifyCallback,BeforeInputCB,EDIT0_CB);
   undo_flag = False;
   open_flag = False;
   XtAddCallback(edit0,XmNlosingFocusCallback,CheckEditCB);

   ac = 0;                      /* Create 1st frame under the Top Most Frame  */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNtopWidget,frame0);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame1 = XmCreateForm(box,"frame1",al,ac);
   XtManageChild(frame1);

   ac = 0;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNlabelString,
                   XmStringCreate(phrase_entry_str,charset));  ac++;
   name1=XmCreateLabel(frame1,"name1",al,ac);
   XtManageChild(name1);

   ac = 0;                                       /* Create Phrase_entry frame */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNleftWidget,name1);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNcolumns,MAX_PHRASE_NUM*2);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNselectionArray,carray);  ac++;
   XtSetArg(al[ac],XmNeditable, True); ac++;
   XtSetArg(al[ac],XmNinput, True); ac++;
   XtSetArg(al[ac],XmNeditMode, XmSINGLE_LINE_EDIT); ac++;
   XtSetArg(al[ac],XmNautoShowCursorPosition, True); ac++;
   edit1=XmCreateText(frame1,"edit1",al,ac);
   XtManageChild(edit1);
/*   XmAddTabGroup(edit1);  */  /* 01/21/92 */
/*   XmTextSetMaxLength(edit1,MAX_PHRASE_NUM);*/ /* for NLS */
   XtAddEventHandler(edit1,FocusChangeMask,False,ChangeFocusEH,EDIT1_EH);
   XtAddCallback(edit1,XmNmodifyVerifyCallback,BeforeInputCB,EDIT1_CB);
   XtAddCallback(edit1,XmNlosingFocusCallback,CheckEditCB);

   ac = 0;                      /* Create 2nd frame under the Top Most Frame  */
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNtopWidget,frame1);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNborderWidth,0);  ac++;
   frame2 = XmCreateForm(box,"frame2",al,ac);
   XtManageChild(frame2);

   ac = 0;
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame2a = XmCreateForm(frame2,"frame2a",al,ac);
   XtManageChild(frame2a);

   ac = 0;                                               /* Create fixed text */
   XtSetArg(al[ac],XmNlabelString,XmStringCreate(find_str,charset));  ac++;
   name2=XmCreateLabel(frame2a,"name2",al,ac);
   XtManageChild(name2);

   ac = 0;                                /* Create Phonetic_view outer frame */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNleftWidget,frame2a);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame2b = XmCreateForm(frame2,"frame2b",al,ac);
   XtManageChild(frame2b);
   ac = 0;                                      /* Create Phonetic_view frame */
   XtSetArg(al[ac],XmNselectionPolicy,XmSINGLE_SELECT);  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNshadowThickness,0);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   XtSetArg(al[ac],XmNwidth,CHINESE_WORD_WIDTH * MAX_PHONETIC_NUM);  ac++;
   edit2 = XmCreateList(frame2b,"edit2",al,ac);
   XtManageChild(edit2);
/*   XmAddTabGroup(edit2);   */  /* 01/21/92 */
   XtAddEventHandler(edit2,FocusChangeMask,False,ChangeFocusEH,EDIT2_EH);
   XtAddEventHandler(edit2,KeyPressMask,False,KeyboardSelectEH,EDIT2_EH);
   XtAddCallback(edit2,XmNsingleSelectionCallback,SingleClickCB,EDIT2_CB);
   XtAddCallback(edit2,XmNdefaultActionCallback,DoubleClickCB,EDIT2_CB);

   ac = 0;                      /* Create 3rd frame under the Top Most Frame  */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNtopWidget,frame2);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame3 =XmCreateForm (box,"frame3",al,ac);
   XtManageChild (frame3);
   ac = 0;                        /* Create frame ready to contain fixed text */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame3a = XmCreateForm(frame3,"frame3a",al,ac);
   XtManageChild(frame3a);

   ac = 0;                                               /* Create fixed text */
   XtSetArg(al[ac],XmNlabelString,XmStringCreate(find_end_str,charset));  ac++;
   name3=XmCreateLabel(frame3a,"name3",al,ac);
   XtManageChild(name3);

   ac = 0;                                  /* Create Phrase_view outer frame */
   XtSetArg(al[ac],XmNleftAttachment,XmATTACH_WIDGET);  ac++;
   XtSetArg(al[ac],XmNleftWidget,frame3a);  ac++;
   XtSetArg(al[ac],XmNrightAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopAttachment,XmATTACH_FORM);  ac++;
   XtSetArg(al[ac],XmNtopOffset,20);  ac++;
   frame3b = XmCreateForm(frame3,"frame3b",al,ac);
   XtManageChild(frame3b);
   ac = 0;                                        /* Create Phrase_view frame */
   XtSetArg(al[ac],XmNvisibleItemCount,10); ac++;
   XtSetArg(al[ac],XmNwidth,CHINESE_WORD_WIDTH * (MAX_PHRASE_NUM+1));  ac++;
   XtSetArg(al[ac],XmNtraversalOn,True); ac++;
   XtSetArg(al[ac],XmNlistSizePolicy,XmCONSTANT); ac++;
   edit3 = XmCreateScrolledList(frame3b, "edit3",al,ac);
   XtManageChild(edit3);
/*   XmAddTabGroup(edit3);  */   /* 01/21/92 */
   XtAddEventHandler(edit3,FocusChangeMask,False,ChangeFocusEH,EDIT3_EH);
   XtAddEventHandler(edit3,KeyPressMask,False,KeyboardSelectEH,EDIT3_EH);
   XtAddCallback(edit3,XmNbrowseSelectionCallback,SingleClickCB,EDIT3_CB);
   XtAddCallback(edit3,XmNdefaultActionCallback,DoubleClickCB,EDIT3_CB);

   return(box);
}

/******************************************************************************/
/* FUNCTION    : main                                                         */
/* DESCRIPTION : user dictionary utility main routine                         */
/* INPUT       : argc = argument count passed by command line.                */
/*               argv = argument list passed by command line.                 */
/* OUTPUT      : none.                                                        */
/* CALLED      :                                                              */
/* CALL        :                                                              */
/******************************************************************************/

main(argc, argv, envp)
  int argc;
  char **argv;
  char **envp;
{
   Widget    main;
   Widget    menu_bar;
   Widget    form;
   char *locale;
   int ret;
   int theScreen;
   int theDepth;

   locale = setlocale(LC_CTYPE,"");                             /* Set locale */
   if ((strcmp(locale, LANGUAGE) != 0) && (strcmp(locale, "en_TW") != 0))
      fprintf(stderr,"Language environment is invalid, to export LANG=zh_TW\n");
   if ((ret=UDEInitial()) !=OK)                             /* Initialize obj */
   {
       fprintf(stderr,"%s \n",err_msg(ret));
       exit(1);
   }

   /**********************/
   /* initialize toolkit */
   /**********************/
   app_shell = XtInitialize(argv[0],"ude",NULL,0,&argc,argv);
   ac = 0;
   XtSetArg(al[ac],XmNallowShellResize,False); ac++;
   XtSetArg(al[ac],XmNshadowThickness, 0); ac++;
   XtSetArg(al[ac],XmNinput, True); ac++;
   XtSetArg(al[ac],XmNwidth,600);  ac++;
   XtSetArg(al[ac],XmNheight,760);  ac++;
   XtSetArg(al[ac],XmNnoResize,True); ac++;
   main = XtCreateWidget("main",
                           xmMainWindowWidgetClass,app_shell,al,ac);
   XtManageChild(main);

   menu_bar = CreateMenuBar(main);                         /* Create Menu Bar */
   XtManageChild(menu_bar);

   form = CreateWorkArea(main);                           /* Create Work Area */
   XtManageChild(form);

   XmMainWindowSetAreas(main,menu_bar,NULL,NULL,NULL,form);

   XtRealizeWidget(app_shell);
   help_flag = False;
   quit_flag = False;
   TextSetString = False;
   title_name = argv[0];

   theDisplay = XtDisplay(app_shell);
   theScreen = DefaultScreen(theDisplay);
   theDepth = DefaultDepth(theDisplay, theScreen);
   if (theDepth ==1)
      color = BLACK;     /* momochrome monitor*/
   else
      color = RED;       /* color monitor */
   initCursors();
   theMainWindow = XtWindow(main);
   theEdit0Window = XtWindow(edit0);
   theEdit1Window = XtWindow(edit1);
   theEdit2Window = XtWindow(edit2);
   theEdit3Window = XtWindow(edit3);
   associateCursors();
   XGrabButton(XtDisplay(edit3),AnyButton, AnyModifier,
               XtWindow(edit3), True,
               ButtonPressMask | ButtonMotionMask |
               ButtonReleaseMask,
               GrabModeAsync, GrabModeAsync,
               XtWindow(edit3),
               XCreateFontCursor(XtDisplay(edit3),XC_crosshair));
   XtMainLoop();
}

