/* @(#)62	1.1  src/rspc/usr/lib/boot/softros/roslib/textwind.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:40:15  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <colors.h>

#ifndef _H_TEXTWIND
/* character definitions */

#define  UPPERLEFT            'Ú'
#define  UPPERRIGHT           '¿'
#define  LOWERLEFT            'À'
#define  LOWERRIGHT           'Ù'
#define  HORIZONTAL           'Ä'
#define  VERTICAL             '³'

#define  UPPERLEFT2           'É'
#define  UPPERRIGHT2          '»'
#define  LOWERLEFT2           'È'
#define  LOWERRIGHT2          '¼'
#define  HORIZONTAL2          'Í'
#define  VERTICAL2            'º'

#define  HLDIVIDEBAR2         'Ç'
#define  HRDIVIDEBAR2         '¶'

#define  UHALFHORIZONTAL      'ß'
#define  RHALFVERTICAL        'Þ'

#define  MINBUTTON            'þ'
#define  SCROLLTOP            ''
#define  SCROLLBOT            ''
#define  SCROLLBAR            '±'
#define  RIGHTTRIANGLE        ''
#define  LEFTTRIANGLE         ''


#define  LEFTHICKY            '®'
#define  RIGHTHICKY           '¯'

/* key definitions       */
#define  BACKSPACE            0x008
#define  TAB                  0x009
#define  ENTER                0x00D
#define  ESC                  0x01B
#define  SPACEBAR             0x020
#define  BACKTAB              0x10F
#define  F1                   0x13B
#define  F2                   0x13C
#define  F3                   0x13D
#define  F4                   0x13E
#define  F5                   0x13F
#define  F6                   0x140
#define  F7                   0x141
#define  F8                   0x142
#define  F9                   0x143
#define  F10                  0x144
#define  F11                  0x145
#define  F12                  0x146
#define  UPARROW              0x148
#define  PAGEUP               0x149
#define  LEFT_ARROW           0x14B
#define  RIGHT_ARROW          0x14D
#define  DOWNARROW            0x150
#define  PAGEDOWN             0x151
#define  INSERT               0x152
#define  DELETE               0x153

#define  INTERNAL_ERROR       2
#define  QUIT                 -1
#define  NO                   0
#define  YES                  1

#define  UP                   1
#define  DOWN                 0

#define  ON                   1
#define  OFF                  0

#define  SHOW                 1
#define  NOSHOW               0

#define  HELPROW              13
#define  HELPCOL              40


#define  ITEMROW              0
#define  ITEMCOL              0
#define  ITEMWIDTH            80
#define  ITEMHEIGHT           24

#define  MSGWIDTH             40
#define  MSGMAXWIDTH          60
#define  MSGMAXLINES          8
#define  MSGHMARGIN           4
#define  MSGVMARGIN           6

#define  MENUHMARGIN          5
#define  MENUVMARGIN          8
#define  MENUTOPMARGIN        4
#define  MINIMENUVMARGIN      4
#define  MINIMENUTOPMARGIN    2
#define  MENUBOTMARGIN        4
#define  MENURIGHTMARGIN      2
#define  MENUROW              4

#define  CLISTVMAX            20
#define  CLISTHMAX            70
#define  CLISTVMARGIN         4
#define  CLISTHMARGIN         10
#define  CLISTROW             2

#define  ELISTVMARGIN         4
#define  ELISTHMARGIN         4
#define  ELISTMIDMARGIN       3
#define  ELISTTOPMARGIN       2

#define  BUTHMARGIN           4
#define  BUTHRMARGIN          2
#define  BUTVMARGIN           2
#define  BUTLISTMARGIN        3
#define  BUT2BUTMARGIN        3

#define  PROGLINEWIDTH        2
#define  PROGWIDTH            50
#define  PROGHMARGIN          4
#define  PROGVMARGIN          8
#define  PROGMSGROW           2

#define  TOP_SIZE             3
#define  LEFT_MARGIN          2
#define  KEY_SPACING          3

#define  ESC_TYPE             0
#define  ENTER_TYPE           1
#define  YESNO_TYPE           2

/* define colors  */

#define  MENUFOR              CLR_BLACK
#define  MENUBACK             CLR_PALEGRAY
#define  MENUBORD             CLR_BLUE
#define  MENUTBACK            CLR_PALEGRAY
#define  MENUHFOR             CLR_BLACK
#define  MENUHBACK            CLR_CYAN

#define  MINIMENUFOR          CLR_WHITE
#define  MINIMENUBACK         CLR_BLUE
#define  MINIMENUBORD         CLR_WHITE
#define  MINIMENUFOR2         CLR_BLACK
#define  MINIMENUBACK2        CLR_CYAN
#define  MINIMENUHFOR         CLR_BLACK
#define  MINIMENUHBACK        CLR_YELLOW


#define  HELP_STYLE           0x80
#define  HELPFOR              CLR_WHITE
#define  HELPBACK             CLR_BLUE
#define  HELPBORD             CLR_WHITE

#define  ERROR_STYLE          0x40
#define  ERRFOR               CLR_WHITE
#define  ERRBACK              CLR_RED
#define  ERRBORD              CLR_WHITE

#define  WARNING_STYLE        0x20
#define  WARNFOR              CLR_WHITE
#define  WARNBACK             CLR_PINK
#define  WARNBORD             CLR_WHITE

#define  MESS_STYLE           0x10
#define  MESSFOR              CLR_WHITE
#define  MESSBACK             CLR_BLUE
#define  MESSBORD             CLR_WHITE

#define  QUES_STYLE           0x08
#define  QUESFOR              CLR_WHITE
#define  QUESBACK             CLR_BLUE
#define  QUESBORD             CLR_WHITE

#define  ITEMFOR              CLR_BLACK
#define  ITEMBACK             CLR_WHITE
#define  ITEMBORD             CLR_BLUE
#define  ITEMHFOR             CLR_WHITE
#define  ITEMHBACK            CLR_BLACK

#define  CLISTFCOLOR1         CLR_WHITE
#define  CLISTBCOLOR1         CLR_BLUE
#define  CLISTBORDCOLOR       CLR_WHITE
#define  CLISTFCOLOR2         CLR_BLACK
#define  CLISTBCOLOR2         CLR_CYAN
#define  CLISTHFCOLOR         CLR_BLACK
#define  CLISTHBCOLOR         CLR_YELLOW

#define  ELISTFCOLOR          CLR_WHITE
#define  ELISTBCOLOR          CLR_BLUE
#define  ELISTBORDCOLOR       CLR_WHITE
#define  ELISTFCOLOR2         CLR_BLACK
#define  ELISTBCOLOR2         CLR_CYAN
#define  ELISTHFCOLOR         CLR_BLACK
#define  ELISTHBCOLOR         CLR_YELLOW

#define  HBORDCOLOR           CLR_WHITE
#define  BORDCOLOR            CLR_BLACK
#define  MINBUTCOLOR          CLR_BLACK
#define  HMINBUTCOLOR         CLR_YELLOW

#define  BUTFCOLOR            CLR_BLACK
#define  BUTBCOLOR            CLR_GREEN
#define  BUTFCOLOR2           CLR_BLACK
#define  BUTBCOLOR2           CLR_DARKGREEN

#define  PROGFCOLOR           CLR_WHITE
#define  PROGBCOLOR           CLR_BLUE
#define  PROGBORDCOLOR        CLR_WHITE
#define  PROGFCOLOR2          CLR_BLACK
#define  PROGBCOLOR2          CLR_CYAN
#define  PROGBARFCOLOR        CLR_GREEN
#define  PROGBARBCOLOR        CLR_YELLOW
#define  PROGBARBORDCOLOR     CLR_BLACK

#define  ENTRYFCOLOR          CLR_BLACK
#define  ENTRYBCOLOR          CLR_YELLOW

#define  MAXPAGES             10
#define  MAXITEMS             20
#define  MAXLIST              80

#define  HELP_RSP             0x8000
#define  ENTER_RSP            0x4000
#define  ESCAPE_RSP           0x2000
#define  EXIT_RSP             0x1000
#define  YESNO_RSP            0x0800
#define  DEFAULT_RSP          0x0200
#define  DELAY                0x0100

#define  WINDELAYTIME         2

#define  CHOICE1           1
#define  CHOICE2           2
#define  CHOICE3           3
#define  CHOICE4           4
#define  CHOICE5           5
#define  CHOICE6           6
#define  CHOICE7           7
#define  CHOICE8           8
#define  CHOICE9           9
#define  CHOICE10          10

typedef struct button {
   struct button *next;
   char        *str;
   void        (*keyfunc)();
   int         row;
   int         col;
   int         key;
   int         inheritable;
} button;

typedef struct bhdr {
   struct bhdr *previous;
   struct button *next;
   int         width;
   int         twidth;
   int         num;
   char        horizontal;
} ButHdr;

typedef struct hotkey {
   int         key;
   void        (*keyfunc)();
   char        *str;
   int         inheritable;
} hotkey;


typedef struct TextWind{
   unsigned int row;
   unsigned int col;
   int unsigned width;
   int unsigned height;
   int fcolor;
   int bcolor;
   int bordcolor;
   struct TextWind *covered_window;
   struct EntryHdr *ehdr;
   unsigned char * WindData;
   ButHdr *bhdr;
} TextWind;

/*------------*/
/* Menu text. */
/*------------*/
typedef struct menu {
   char **title;
   int    numitems;
   char **items[MAXITEMS];
} Menu;

/*-------------*/
/* Menu Array. */
/*-------------*/
typedef struct MenuArray {
   char *item;
   char *help;
} MenuArray;

/*---------------*/
/* PgMess Text.  */
/*---------------*/
typedef struct pgmess {
   char **title;
   char **message;
} PgMess;

/*-----------------------*/
/* Message Control Block */
/*-----------------------*/
typedef struct _Mhdr {
   char *title;
   char *data;
   struct TextWind *wind;
   unsigned int keys;         // Bit mask of allowed keys
   unsigned int style;        // MESSAGE, HELP, WARNING, ERROR (selects colors)
   ButHdr *bhdr;              // Pointer to user response buttons
   char *curword;             // Pointer to first word in msg area
   int extralines;            // number of extralines asked for on creation
   int numlines;              // Total number of lines in message
   int curline;               // Current top line number
   int height;                // height of msg area
} Mhdr;

/*-----------------*/
/* Menu Ctl Block. */
/*-----------------*/
typedef struct menuhdr {
   TextWind    *wind;         // Pointer to the menus window
   struct EntryHdr *ehdr;
   ButHdr      *bhdr;         // Pointer user response buttons
   int         curitem;       // Currently selected item
   int         width;         // width of max item
   char        type;          // FULLMENU or MINIMENU
} MenuHdr;

typedef struct choicebox {
   char        *text;         // Text to display
   char        *help;         // Help for item
   struct choicebox *next;    // pointer to next choice
   char        selected;      // TRUE or FALSE
} choicebox;

typedef struct CboxHdr {
   struct EntryHdr  *ehdr;
   choicebox   *clist;        // Pointer to first item
}CboxHdr;

typedef struct ShowList {
   char        *item;
   char        *detail;
   char        *help;
   struct ShowList *next;
   char        more;
} ShowList;

typedef struct EntryList {
   char        *item;
   char        *help;
   char        *allowed;
   char        *buffer;
   struct EntryList *next;
   int         selected;
   int         bufsize;
   int         curchar;
   int         leftchar;
   int         type;
} EntryList;

enum ListTypes {NOTYPE, CHECKBOX, RADIOBOX, MORE, ENTRYFIELD, SHOWFIELD, FULLMENU, MINIMENU, MIXEDLIST};

typedef struct ShowHdr {
   struct EntryList *elist;
   struct EntryHdr  *ehdr;
   int         current;
} ShowHdr;

typedef struct EntryHdr {
   struct EntryList *list;
   struct EntryList *pcurrent;
   struct EntryList *lastchoice;
   char        *title;
   struct TextWind *win;
   ButHdr *bhdr;
   int         height;
   int         num;
   int         top;
   int         current;
   int         col1width;
   int         col2width;
   int         col1;
   int         col2;
   int         row;
   int         fcolor;           // foreground color of item (window can be different color)
   int         bcolor;           // background color of item
   int         hfcolor;          // foreground color of highlited item
   int         hbcolor;          // background color of highlited item
   int         type;
   char        scrollbar;
   char        enter;
   char        radio;
} EntryHdr;

typedef struct ProgHdr {
   TextWind    *win;          // window
   char        *title;        // title (can be NULL)
   char        *msg;          // msg
   int         progress;      // last percentage displayed (for redraw)
}ProgHdr;



/* externalized data             */

extern int language;
extern int highlight_color;

/* internal function prototypes  */

void InitTextScreen();
TextWind *text_newwindow(unsigned int row, unsigned int col, unsigned int width, unsigned int height);
TextWind *new_window(int row, int col, int width, int height,int fcol, int bcol);
void text_putwindow(TextWind *wind, int row, int col, int width, int height);
void wind_putchar(TextWind *wind,char c,int row, int col, int fcol, int bcol);
void win_blank(TextWind *wind);
void win_frame(TextWind *wind,int color);
void draw_hdividebar(TextWind *wind,int row, int color);
void text_removewindow(TextWind *wind);
void text_inwind(TextWind *wind, int row, int col,char *textptr, int width,int height, int fcol, int bcol);
int text_countrows(char *str, int width);
void win_print(TextWind *win, int row, int col, char **text);
void win_print_tr(TextWind *win, int row, int col, char *text);
MenuHdr *new_menu(Menu *menutxt, int initial_sel);
MenuHdr *minimenu(Menu *menutxt, int row, int col);
MenuHdr *menulist(MenuArray *mlist, char *title, char type);
void *menu_itemsel(MenuHdr *mhdr, int selection);
void redrawmenu(MenuHdr *menu);
void handle_menu(EntryHdr *hdr,char *exit,int *result,int key);
int service_menu(MenuHdr *menuhdr, PgMess *phelp[]);
int service_menulist(MenuHdr *menuhdr);
void remove_menu(MenuHdr *menuhdr);
int popuprow();
int popupcol();
int popupcol2();
void sound_beep();
Mhdr *text_newmsg(PgMess *msgdata,  unsigned int responses, int style);
Mhdr *text_newmsg_tr(char *title,char *data,unsigned int responses,int style);
int text_responsemsg(PgMess *msgdata, unsigned int responses, int style);
int text_responsemsg_tr(char *title,char *data,unsigned int responses,int style);
int rspmsgcommon(Mhdr *mhdr);
void buildmsg(Mhdr *mhdr,int extralines);
void textmsgfill(Mhdr *mhdr);
int text_msgkeys(Mhdr *mhdr);
void text_scrollmsg(Mhdr *mhdr, int lines, int width);
void draw_scrollbar(TextWind *win,int numlines,int curline,int maxlines,
                   int toprow, int botrow, int col, int fcolor, int bcolor);
void text_msgend(Mhdr *mhdr);
ButHdr *create_buttons(unsigned int responses, char **butptr);
void format_buttons(TextWind *wind,ButHdr *hdr);
void draw_buttons(TextWind *wind, ButHdr *hdr, int highlite);
void remove_buttons(ButHdr *hdr);
void free_button(button *but);
void add_hotkeys(hotkey *keylist);
void block_button_inherit();
void get_key(int *key);
int textentry(PgMess *text,char *result,char **allowed,unsigned int bufsize,char show,PgMess *help);
int charlen(char *str);
CboxHdr *checklist(choicebox *clist, char *title, char type);
void redraw_clist(CboxHdr *hdr);
void draw_clist(CboxHdr *chdr);
void handle_checkbox(EntryHdr *hdr,char *exit,int *result,int key);
int service_clist(CboxHdr *hdr);
void remove_clist(CboxHdr *hdr);
choicebox *make_clist(choicebox *cbox, char *text, char *help, char selected);
void unmake_clist(choicebox *list);
void redrawprogmsg(ProgHdr *phdr);
void drawprogbar(ProgHdr *phdr,int percent);
void remove_progmsg(ProgHdr *phdr);
ShowHdr *showlist(ShowList *slist, char *title);
int service_showlist(ShowHdr *hdr);
void update_elist(EntryHdr *hdr);
void remove_showlist(ShowHdr *hdr);
void free_showlist(ShowList *list);
void free_entrylist(EntryList *list);
EntryHdr *entrylist(EntryList *elist, char *title);
void redraw_elist(EntryHdr *hdr);
void draw_elist(EntryHdr *hdr);
void remove_elist(EntryHdr *hdr);
void show_entry(EntryList *list,TextWind *wind,int row,int col,int width,
               int fcolor,int bcolor,char show,char more,char enter);
int handle_entry(EntryList *list,TextWind *wind,int row,int col,int width,
                  char show);
EntryList *make_entrylist(EntryList *elist,char *item,char *help,int numchars,char *allowed,
                           int type);
void unmake_entrylist(EntryList *elist);

#endif
