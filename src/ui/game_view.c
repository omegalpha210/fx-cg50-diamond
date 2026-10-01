#include "draw.h"
#include <stdio.h>

void dg_screen_position(const DgApp *app,int node,int *x,int *y)
{
 if(node<0 || node>=DG_NODES){*x=198;*y=112;return;}
 int scale=app->zoom?2:1;
 *x=198+(2*(int)dg_nodes[node].q+(int)dg_nodes[node].r)*8*scale;
 *y=114+(int)dg_nodes[node].r*13*scale;
 if(app->zoom){int cursor=app->cursor<DG_NODES?app->cursor:36;
  int shift=-(int)dg_nodes[cursor].r*26;if(shift<-81)shift=-81;if(shift>81)shift=81;*y+=shift;}
}
static void triangle(DgPainter *p,const int x[3],const int y[3],uint16_t ink)
{
 int lo=y[0],hi=y[0];for(int k=1;k<3;k++){if(y[k]<lo)lo=y[k];if(y[k]>hi)hi=y[k];}
 if(lo<p->top)lo=p->top;
 if(hi>=p->bottom)hi=p->bottom-1;
 for(int py=lo;py<=hi;py++){int left=10000,right=-10000;
  for(int k=0;k<3;k++){int j=(k+1)%3;if(y[k]==y[j]){if(py!=y[k])continue;
    if(x[k]<left)left=x[k];
    if(x[k]>right)right=x[k];
    if(x[j]<left)left=x[j];
    if(x[j]>right)right=x[j];
   }else if(py>=(y[k]<y[j]?y[k]:y[j]) && py<=(y[k]>y[j]?y[k]:y[j])){
    int px=x[k]+(py-y[k])*(x[j]-x[k])/(y[j]-y[k]);if(px<left)left=px;if(px>right)right=px;}}
  if(left<=right)ui_rect(p,left,py,right-left+1,1,ink);}
}
static void board(DgPainter *p,const DgApp *app)
{
 DgPainter b={p->canvas,4,UI_BOARD_TOP,392,UI_BOARD_BOTTOM};const DgGame *g=&app->archive.game;
 for(uint8_t camp=0;camp<6;camp++){int xs[3],ys[3],found=0;
  for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,camp)){int degree=0;
   for(int d=0;d<6;d++){int next=dg_nodes[n].neighbor[d];if(next>=0 && dg_in_camp(next,camp))degree++;}
   if(degree==2 && found<3){dg_screen_position(app,n,&xs[found],&ys[found]);found++;}}
  uint16_t tint=DG_RGB(28,30,28);
  for(uint8_t who=DG_RED;who<=DG_GREEN;who++)if(camp==dg_home[who] || camp==dg_goal[who])
   tint=who==DG_RED?DG_RGB(31,26,26):who==DG_YELLOW?DG_RGB(31,30,22):DG_RGB(24,30,26);
  if(found==3)triangle(&b,xs,ys,tint);
 }
 for(int n=0;n<DG_NODES;n++){int x,y;dg_screen_position(app,n,&x,&y);
  for(int d=0;d<3;d++){int next=dg_nodes[n].neighbor[d];if(next>=0){int tx,ty;dg_screen_position(app,next,&tx,&ty);ui_line(&b,x,y,tx,ty,UI_LINE);}}}
 if(app->path.length>1 && (app->selected!=DG_NONE || app->animation))
  for(unsigned i=1;i<app->path.length;i++){int x,y,tx,ty;dg_screen_position(app,app->path.node[i-1],&x,&y);
   dg_screen_position(app,app->path.node[i],&tx,&ty);ui_line(&b,x,y,tx,ty,BOARD_CYAN);ui_line(&b,x+1,y,tx+1,ty,BOARD_CYAN);}
 int radius=app->zoom?7:4;
 for(int n=0;n<DG_NODES;n++){int x,y;dg_screen_position(app,n,&x,&y);
  uint8_t who=g->pos.board[n];
  bool occupied=who!=DG_EMPTY && !(app->animation && n==(int)app->pending_move.from);
  ui_disc(&b,x,y,radius+1,UI_BLACK);ui_disc(&b,x,y,radius,occupied?ui_piece_color(who):UI_WHITE);
 }
 if(app->selected<DG_NODES && app->archive.assist){DgMove moves[DG_NODES];
  size_t count=dg_piece_moves(g->pos.board,DG_RED,app->selected,moves,DG_NODES);
  for(size_t i=0;i<count;i++){int x,y;dg_screen_position(app,moves[i].to,&x,&y);
   ui_disc(&b,x,y,radius+1,UI_BLACK);ui_disc(&b,x,y,radius,BOARD_CYAN);}}
 if(app->selected<DG_NODES){int x,y;dg_screen_position(app,app->selected,&x,&y);ui_ring(&b,x,y,radius+3,PIECE_YELLOW);ui_ring(&b,x,y,radius+4,BOARD_INK);}
 if(app->animation && app->path.length){unsigned at=app->anim_index;if(at>=app->path.length)at=app->path.length-1;
  int x,y;dg_screen_position(app,app->path.node[at],&x,&y);
  ui_disc(&b,x,y,radius+1,UI_BLACK);ui_disc(&b,x,y,radius,ui_piece_color(dg_current(g)));}
 if(app->cursor<DG_NODES){int x,y;dg_screen_position(app,app->cursor,&x,&y);int r=radius+5;
  ui_border(&b,x-r,y-r,2*r+1,2*r+1,BOARD_BLUE,1);ui_rect(&b,x-r-2,y,3,1,BOARD_BLUE);ui_rect(&b,x+r,y,3,1,BOARD_BLUE);}
}
static const char *player_name(uint8_t player)
{return player==DG_RED?"YOU":player==DG_YELLOW?"YELLOW":"GREEN";}
static void status(DgPainter *p,const DgApp *app)
{
 const DgGame *g=&app->archive.game;uint8_t current=dg_current(g);char value[40];
 ui_rect(p,0,0,396,24,UI_WHITE);ui_rect(p,0,23,396,1,UI_LINE);
 ui_text(p,8,6,"TURN",UI_MUTED,1,1);
 ui_text(p,50,6,current==DG_RED?"HUMAN":current==DG_GREEN?"GREEN AI":"YELLOW AI",UI_INK,1,1);
 snprintf(value,sizeof value,"%uP / %s",(unsigned)g->players,ui_level_label(g->level));
 ui_center(p,0,6,396,value,UI_MUTED,1,1);
 snprintf(value,sizeof value,"T %lu",(unsigned long)g->pos.turns);
 ui_text(p,388-ui_text_width(value,1,1),6,value,UI_MUTED,1,1);
 /* Small backing regions preserve readable HUD text in the zoom viewport. */
 ui_rect(p,6,30,99,17,UI_PAPER);
 ui_text(p,8,33,app->archive.assist?"ASSIST ON":"ASSIST OFF",UI_MUTED,1,1);
 int row=0;
 for(uint8_t who=DG_RED;who<=DG_GREEN;who++)if(who!=DG_YELLOW || g->players==3){
  int y=33+row++*17;
  snprintf(value,sizeof value,"%s %u/10",player_name(who),(unsigned)dg_goal_count(g->pos.board,who));
  ui_rect(p,284,y-3,106,17,UI_PAPER);ui_disc(p,287,y+5,2,ui_piece_color(who));
  ui_text(p,388-ui_text_width(value,1,1),y,value,UI_MUTED,1,1);
 }
 if(app->thinking){
  ui_rect(p,6,57,117,18,UI_PAPER);
  /* Static busy marker: the frozen native loop does not redraw during search. */
  ui_ring(p,12,65,4,UI_LINE);ui_line(p,12,61,16,65,UI_BLUE);
  ui_text(p,22,60,"THINKING...",UI_BLUE,1,1);
 }
}
static void modal(DgPainter *p,const DgApp *app)
{
 const DgGame *g=&app->archive.game;
 int w=244,h=app->modal==DG_MODAL_RESTART?88:104,x=(396-w)/2,y=4+(196-h)/2;
 ui_rect(p,x+4,y+4,w,h,DG_RGB(12,14,14));ui_rect(p,x,y,w,h,UI_WHITE);ui_border(p,x,y,w,h,UI_INK,2);
 ui_rect(p,x+2,y+2,w-4,5,app->modal==DG_MODAL_RESTART?UI_RESTART:ui_piece_color(g->pos.winner));
 if(app->modal==DG_MODAL_RESTART){
  ui_center(p,x,y+17,w,"RESTART GAME?",UI_INK,1,1);
  ui_center(p,x,y+34,w,"Same setup and AI order",UI_MUTED,1,1);
  ui_text(p,x+22,y+51,"EXE: YES",UI_INK,1,1);ui_text(p,x+22,y+68,"EXIT: NO",UI_INK,1,1);
 }else{
  char result[32],stats[32];
  if(g->pos.winner==DG_RED)snprintf(result,sizeof result,"YOU WIN");
  else if(g->players==2)snprintf(result,sizeof result,"AI WINS");
  else snprintf(result,sizeof result,"%s AI WINS",player_name(g->pos.winner));
  ui_center(p,x,y+17,w,result,UI_INK,3,2);
  snprintf(stats,sizeof stats,"TURNS %lu   %s",(unsigned long)g->pos.turns,ui_level_label(g->level));
  ui_center(p,x,y+43,w,stats,UI_MUTED,1,1);
  ui_text(p,x+22,y+64,"EXE: NEW GAME",UI_INK,1,1);ui_text(p,x+22,y+81,"EXIT: VIEW BOARD",UI_INK,1,1);
 }
}
void ui_game_screen(DgPainter *p,const DgApp *app,const char **labels)
{
 const DgGame *g=&app->archive.game;
 board(p,app);status(p,app);
 bool human=dg_current(g)==DG_RED && !app->thinking && !app->animation;
 labels[0]=g->pos.winner || app->thinking || app->animation?"":"RESTART";
 labels[1]=g->undo_valid && !g->pos.winner && human?"UNDO":"";
 labels[3]="RULES";labels[4]="ZOOM";
 labels[5]=g->pos.winner?"NEW":human?(app->selected<DG_NODES?"MOVE":"SELECT"):"";
 if(app->modal){
  modal(p,app);
  /* Modal keys are EXE/EXIT; inactive gameplay actions have no labels. */
  for(int i=0;i<6;i++)labels[i]="";
 }
}
