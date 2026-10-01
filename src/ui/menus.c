#include "draw.h"
#include <stdio.h>

static void option(DgPainter *p,int x,int y,int w,const char *s,bool selected)
{
 ui_rect(p,x,y,w,UI_OPTION_H,selected?UI_SELECTED:UI_WHITE);
 ui_border(p,x,y,w,UI_OPTION_H,selected?UI_FOCUS:UI_LINE,selected?2:1);
 int textw=ui_text_width(s,3,2),left=x+(w-textw)/2;
 ui_text(p,left,y+15,s,UI_INK,3,2);
}
static void level_option(DgPainter *p,int x,uint8_t level,bool selected)
{
 const char *label=ui_level_label(level);int y=51,w=122;
 ui_rect(p,x,y,w,UI_OPTION_H,selected?UI_SELECTED:UI_WHITE);
 ui_border(p,x,y,w,UI_OPTION_H,selected?UI_FOCUS:UI_LINE,selected?2:1);
 int left=x+(w-ui_text_width(label,3,2)-30)/2,cx=left+9,cy=y+UI_OPTION_H/2;
 uint16_t color=level==DG_EASY?PIECE_GREEN:level==DG_NORMAL?PIECE_YELLOW:PIECE_RED;
 ui_disc(p,cx,cy,9,UI_BLACK);ui_disc(p,cx,cy,8,color);
 ui_rect(p,cx-4,cy-3,2,2,UI_BLACK);ui_rect(p,cx+3,cy-3,2,2,UI_BLACK);
 if(level==DG_NORMAL)ui_line(p,cx-4,cy+3,cx+4,cy+3,UI_BLACK);
 else{
  int edge=level==DG_EASY?2:4,middle=level==DG_EASY?4:2;
  ui_line(p,cx-4,cy+edge,cx-2,cy+middle,UI_BLACK);
  ui_line(p,cx-2,cy+middle,cx+2,cy+middle,UI_BLACK);
  ui_line(p,cx+2,cy+middle,cx+4,cy+edge,UI_BLACK);
 }
 if(level==DG_HARD){
  ui_line(p,cx-5,cy-6,cx-1,cy-4,UI_BLACK);
  ui_line(p,cx+1,cy-4,cx+5,cy-6,UI_BLACK);
 }
 ui_text(p,left+30,y+15,label,UI_INK,3,2);
}
static void player(DgPainter *p,const DgApp *app,const char **labels)
{
 ui_title(p,"DIAMOND",NULL);ui_text(p,9,35,"PLAYER",UI_MUTED,1,1);
 ui_text(p,9+ui_text_width("PLAYER",1,1)+10,35,"(YOU ARE RED)",PIECE_RED,1,1);
 for(int i=0;i<2;i++){
  int x=UI_CARD_X+i*(UI_CARD_W+UI_CARD_GAP);bool selected=app->players==(uint8_t)(i+2);
  ui_rect(p,x,54,UI_CARD_W,128,selected?UI_SELECTED:UI_WHITE);
  ui_border(p,x,54,UI_CARD_W,128,selected?UI_FOCUS:UI_LINE,selected?3:1);
  if(i==0){ui_piece(p,x+61,105,15,DG_RED,true);ui_piece(p,x+125,105,15,DG_GREEN,false);}
  else{ui_piece(p,x+45,105,15,DG_RED,true);ui_piece(p,x+93,105,15,DG_YELLOW,false);ui_piece(p,x+141,105,15,DG_GREEN,false);}
  ui_center(p,x,148,UI_CARD_W,i?"3 PLAYER":"2 PLAYER",UI_INK,3,2);
 }
 labels[0]="SET";labels[1]=app->archive.active?"RESUME":"";labels[3]="RULES";labels[5]="NEXT";
}
static void setup(DgPainter *p,const DgApp *app,const char **labels)
{
 ui_title(p,"GAME SETUP",app->players==2?"2 PLAYER":"3 PLAYER");
 ui_text(p,9,34,"LEVEL",app->focus==0?UI_BLUE:UI_MUTED,1,1);
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(int i=0;i<3;i++)level_option(p,7+i*130,levels[i],app->level==levels[i]);
 ui_text(p,9,114,app->players==2?"FIRST":"HUMAN",app->focus==1?UI_BLUE:UI_MUTED,1,1);
 if(app->players==2){option(p,7,131,187,"HUMAN",app->slot==0);option(p,202,131,187,"AI",app->slot==1);}
 else{
  static const char *const slots[3]={"1ST","2ND","3RD"};
  for(int i=0;i<3;i++)option(p,7+i*130,131,122,slots[i],app->slot==(uint8_t)i);
 }
 labels[0]="SET";labels[3]="RULES";labels[5]="PLAY";
}
static void settings(DgPainter *p,const DgApp *app)
{
 ui_title(p,"SETTINGS",NULL);ui_text(p,9,42,"ASSIST",UI_BLUE,1,1);
 option(p,7,61,187,"OFF",!app->archive.assist);option(p,202,61,187,"ON",app->archive.assist!=0);
 ui_text(p,9,124,"Show legal destinations.",UI_MUTED,1,1);
}
static void rules(DgPainter *p,const DgApp *app)
{
 /* Text, nine-line viewport and 0..12 scroll offsets are unchanged. */
 static const char *const lines[]={
  "73 POINTS. 10 EQUAL PIECES EACH.","2 OR 3 PLAYERS. YOU ARE RED.","FILL YOUR OPPOSITE CAMP TO WIN.",
  "STEP TO AN ADJACENT EMPTY POINT.","OR JUMP ONE ADJACENT PIECE TO","THE EMPTY POINT DIRECTLY BEYOND.",
  "JUMP OWN OR OTHER PLAYER PIECES.","NO CAPTURE. CROSSED PIECES STAY.","CHAIN JUMPS AND CHANGE DIRECTION.",
  "STOP AFTER ANY JUMP. JUMP OPTIONAL.","END ON A DIFFERENT POINT.","DO NOT MIX STEP AND JUMP.",
  "ALL CAMPS MAY BE ENTERED.","GOAL PIECES MAY LEAVE UNTIL WIN.","ASSIST ON MARKS LEGAL DESTINATIONS.",
  "ARROWS NAVIGATE. EXE CONFIRMS.","EXIT CANCELS / RETURNS TO SETUP.","F1 RESTART. F2 ONE HUMAN TURN UNDO.",
  "F5 OVERVIEW / ZOOM.","MENU OPENS CASIO MAIN MENU.","SHIFT + AC/ON SAVES THEN POWERS OFF."
 };
 unsigned count=(unsigned)(sizeof lines/sizeof lines[0]),offset=app->rules_scroll;
 if(offset>count-9)offset=count-9;
 char page[20];snprintf(page,sizeof page,"%u-%u / %u",offset+1,offset+9,count);ui_title(p,"RULES",page);
 for(unsigned i=0;i<9;i++)ui_text(p,9,35+(int)i*17,lines[offset+i],UI_INK,1,1);
 ui_rect(p,382,35,3,151,UI_LINE);int thumb=151*9/(int)count;
 ui_rect(p,382,35+(int)offset*(151-thumb)/(int)(count-9),3,thumb,UI_BLUE);
 ui_text(p,9,190,"UP / DOWN: SCROLL",UI_MUTED,1,1);
 const char *back="EXIT: BACK";ui_text(p,388-ui_text_width(back,1,1),190,back,UI_MUTED,1,1);
}
void ui_menu_screen(DgPainter *p,const DgApp *app,const char **labels)
{
 if(app->screen==DG_PLAYER)player(p,app,labels);
 else if(app->screen==DG_SETUP)setup(p,app,labels);
 else if(app->screen==DG_SETTINGS)settings(p,app);
 else rules(p,app);
}
