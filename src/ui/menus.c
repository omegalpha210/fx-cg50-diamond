#include "draw.h"
#include <stdio.h>

/* NUM GAME entry(): normal font, measured option widths, outline/underline. */
static void entry_options(DgPainter *p,int y,const char *const *names,
 unsigned count,unsigned selected,const uint16_t *colors)
{
 int total=0;
 for(unsigned i=0;i<count;i++)total+=ui_text_width(names[i],1,1)+10;
 int gap=count>1?(228-total)/(int)(count-1):0,x=146;
 for(unsigned i=0;i<count;i++){
  int w=ui_text_width(names[i],1,1)+10;
  uint16_t ink=colors?colors[i]:UI_BLUE;
  if(i==selected){
   ui_rect(p,x,y+4,w,21,UI_WHITE);ui_border(p,x,y+4,w,21,ink,1);
   ui_rect(p,x+3,y+23,w-6,2,ink);
  }
  ui_text(p,x+5,y+9,names[i],ink,1,1);x+=w+gap;
 }
}
static void player(DgPainter *p,const DgApp *app,const char **labels)
{
 ui_title(p,"DIAMOND",NULL);ui_text(p,9,35,"PLAYER",UI_MUTED,1,1);
 ui_text(p,9+ui_text_width("PLAYER",1,1)+10,35,"(YOU ARE RED)",UI_RED_TEXT,1,1);
 for(int i=0;i<2;i++){
  int x=UI_CARD_X+i*(UI_CARD_W+UI_CARD_GAP);bool selected=app->players==(uint8_t)(i+2);
  ui_rect(p,x,54,UI_CARD_W,128,selected?UI_SELECTED:UI_WHITE);
  ui_border(p,x,54,UI_CARD_W,128,selected?UI_FOCUS:UI_LINE,selected?3:1);
  if(i==0){ui_piece(p,x+61,105,15,DG_RED,true);ui_piece(p,x+125,105,15,DG_GREEN,false);}
  else{ui_piece(p,x+45,105,15,DG_RED,true);ui_piece(p,x+93,105,15,DG_YELLOW,false);ui_piece(p,x+141,105,15,DG_GREEN,false);}
  ui_center(p,x,148,UI_CARD_W,i?"3 PLAYER":"2 PLAYER",UI_INK,3,2);
 }
 labels[0]=dg_app_resumable(app)?"RESUME":"";labels[3]="RULES";labels[5]="NEXT";
}
static void setup(DgPainter *p,const DgApp *app,const char **labels)
{
 ui_title(p,"GAME SETUP",app->players==2?"2 PLAYER":"3 PLAYER");
 unsigned count=dg_entry_count(app);
 for(unsigned row=0;row<count;row++){
  int action=dg_entry_action(app,row);char number[4];
  snprintf(number,sizeof number,"%u",row+1);
  const char *name=action==DG_ENTRY_RESUME?"RESUME":action==DG_ENTRY_NEW?"NEW GAME":
   action==DG_ENTRY_LEVEL?"DIFFICULTY":action==DG_ENTRY_SLOT?(app->players==2?"FIRST":"YOU"):"ASSIST";
  bool compact=count>4;int y=(compact?30:34)+(int)row*(compact?27:34),h=compact?25:29;
  ui_rect(p,10,y,376,h,row==app->focus?UI_SELECTED:UI_WHITE);
  ui_border(p,10,y,376,h,row==app->focus?UI_BLUE:UI_LINE,row==app->focus?2:1);
  ui_text(p,20,y+9,number,UI_MUTED,1,1);ui_text(p,43,y+9,name,UI_INK,1,1);
  if(action==DG_ENTRY_LEVEL){
   static const char *const names[3]={"EASY","NORMAL","HARD"};
   static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
   static const uint16_t colors[3]={UI_GREEN_TEXT,UI_YELLOW_TEXT,UI_RED_TEXT};unsigned selected=0;
   for(unsigned i=0;i<3;i++)if(app->level==levels[i])selected=i;
   entry_options(p,y,names,3,selected,colors);
  }else if(action==DG_ENTRY_SLOT){
   static const char *const first[2]={"YOU","AI"},*const slots[3]={"1ST","2ND","3RD"};
   entry_options(p,y,app->players==2?first:slots,app->players,app->slot,NULL);
  }else if(action==DG_ENTRY_ASSIST){
   static const char *const assist[2]={"OFF","ON"};
   entry_options(p,y,assist,2,app->archive.assist,NULL);
  }
 }
 int action=dg_entry_action(app,app->focus);
 const char *hint=action==DG_ENTRY_RESUME?"Continue the saved game.":action==DG_ENTRY_NEW?"New game with these settings.":
  action==DG_ENTRY_LEVEL?"LEFT/RIGHT: difficulty for NEW GAME.":action==DG_ENTRY_SLOT?
  (app->players==2?"LEFT/RIGHT: YOU or AI starts.":"LEFT/RIGHT: your turn slot."):"LEFT/RIGHT: legal destination hints.";
 if(dg_setup_resume(app) && !app->notice[0]){
  char saved[48];snprintf(saved,sizeof saved,"Saved: %s / %s",ui_level_label(app->archive.game.level),
   app->players==2?(app->archive.game.human_slot?"AI FIRST":"YOU FIRST"):
   app->archive.game.human_slot==0?"YOU 1ST":app->archive.game.human_slot==1?"YOU 2ND":"YOU 3RD");
  ui_text(p,12,173,saved,UI_MUTED,1,1);
  ui_text(p,12+ui_text_width("Saved: ",1,1)+1,173,ui_level_label(app->archive.game.level),
   ui_level_color(app->archive.game.level),1,1);
 }
 ui_text(p,12,188,hint,UI_MUTED,1,1);labels[3]="RULES";labels[5]="OPEN";
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
  "ARROWS NAVIGATE. EXE CONFIRMS.","EXIT CANCELS / RETURNS TO SETUP.","F1 RESTART. F2 UNDO YOUR TURN.",
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
 else rules(p,app);
}
