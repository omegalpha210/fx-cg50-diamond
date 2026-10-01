/* Pixel contracts for the actual renderer, independent expected font masks. */
#include "../src/ui/draw.h"
#include "../src/ui/font_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint16_t pixels[224][396],prior[224][396];
static unsigned frames,calls;
static void raster(void *context,int x,int y,int w,int h,uint16_t ink)
{
 (void)context;
 assert(w>0 && h>0 && x>=0 && y>=0 && x+w<=396 && y+h<=224);calls++;
 for(int row=y;row<y+h;row++)for(int col=x;col<x+w;col++)pixels[row][col]=ink;
}
static void render(const DgApp *app)
{
 DgApp before=*app;dg_render(app,&(DgCanvas){NULL,raster});
 assert(!memcmp(app,&before,sizeof before));frames++;
}
static void expected_text(int x,int y,const char *s,uint16_t ink,int numerator,int denominator)
{
 int advance=0;
 for(;*s;s++){
  unsigned ch=(unsigned char)*s;assert(ch>=32 && ch<=126);ch-=32;
  for(int row=0;row<11;row++)for(int col=0;col<glyph_width[ch];col++)if(glyph_rows[ch][row]&(1u<<col)){
   int x1=x+(advance+col)*numerator/denominator,x2=x+(advance+col+1)*numerator/denominator;
   int y1=y+row*numerator/denominator,y2=y+(row+1)*numerator/denominator;
   assert(x1>=0 && y1>=0 && x2<=396 && y2<=224);
   for(int r=y1;r<y2;r++)for(int c=x1;c<x2;c++)assert(pixels[r][c]==ink);
  }
  advance+=glyph_width[ch]+1;
 }
}
static void strip(const char *const labels[6],const uint16_t backgrounds[6],const uint16_t inks[6])
{
 /* Every pixel is checked: blank slots, separators, centered action-only text.
    Any extra F-number, dash, border or clipped label fails this contract. */
 for(int i=0;i<6;i++){
  uint16_t expected[20][66];
  for(int r=0;r<20;r++)for(int c=0;c<66;c++)expected[r][c]=UI_WHITE;
  if(labels[i][0]){
   for(int r=1;r<19;r++)for(int c=1;c<65;c++)expected[r][c]=backgrounds[i];
   int width=ui_text_width(labels[i],1,1);assert(width<=60);
   int x=1+(64-width)/2,advance=0;
   for(const char *s=labels[i];*s;s++){
    unsigned ch=(unsigned char)*s-32;
    for(int r=0;r<11;r++)for(int c=0;c<glyph_width[ch];c++)if(glyph_rows[ch][r]&(1u<<c))
     expected[5+r][x+advance+c]=inks[i];
    advance+=glyph_width[ch]+1;
   }
  }
  for(int r=0;r<20;r++)for(int c=0;c<66;c++)assert(pixels[204+r][66*i+c]==expected[r][c]);
 }
}
static void menu_strip(bool resume,bool setup)
{
 const char *labels[6]={"SET",resume?"RESUME":"","","RULES","",setup?"PLAY":"NEXT"};
 uint16_t bg[6]={UI_SET,UI_BLUE,0,UI_BLACK,0,setup?UI_RUN:UI_NEXT};
 uint16_t fg[6]={UI_BLACK,UI_WHITE,0,UI_WHITE,0,setup?UI_WHITE:UI_BLACK};strip(labels,bg,fg);
}
static void game_strip(bool undo,const char *action,bool idle)
{
 const char *labels[6]={idle?"RESTART":"",undo?"UNDO":"","","RULES","ZOOM",action};
 uint16_t bg[6]={UI_RESTART,UI_UNDO,0,UI_BLACK,UI_BLUE,UI_RUN};
 uint16_t fg[6]={UI_BLACK,UI_BLACK,0,UI_WHITE,UI_WHITE,UI_WHITE};strip(labels,bg,fg);
}
static void blank_strip(void)
{for(int y=204;y<224;y++)for(int x=0;x<396;x++)assert(pixels[y][x]==UI_WHITE);}
static void box(int x,int y,int w,int h,uint16_t ink,int thick)
{
 for(int r=0;r<h;r++)for(int c=0;c<w;c++)if(r<thick || r>=h-thick || c<thick || c>=w-thick)
  assert(pixels[y+r][x+c]==ink);
}
static bool glyph_pixel(int x,int y,const char *text)
{
 if(y<0 || y>=UI_FONT_HEIGHT)return false;
 for(;*text;text++){
  unsigned ch=(unsigned char)*text-32;
  if(x>=0 && x<glyph_width[ch])return (glyph_rows[ch][y]&(1u<<x))!=0;
  x-=glyph_width[ch]+1;
 }
 return false;
}
static void actor(int x,int y,uint8_t player,bool ai)
{
 int text_x=x-15+(31-ui_text_width("AI",1,1))/2;
 for(int dy=-15;dy<=15;dy++)for(int dx=-15;dx<=15;dx++){
  int square=dx*dx+dy*dy;if(square>225)continue;
  uint16_t expected=square<=196?ui_piece_color(player):UI_BLACK;
  if(ai && glyph_pixel(x+dx-text_x,dy+5,"AI"))expected=UI_BLACK;
  assert(pixels[y+dy][x+dx]==expected);
 }
 if(ai)expected_text(text_x,y-5,"AI",UI_BLACK,1,1);
}
static void face(int cx,int cy,uint8_t level,uint16_t background)
{
 /* Independently specified mouth/brow sample masks, plus all disk pixels. */
 static const int brow[10][2]={{-5,-6},{-4,-6},{-3,-5},{-2,-5},{-1,-4},
                              {1,-4},{2,-4},{3,-5},{4,-5},{5,-6}};
 uint16_t fill=level==DG_EASY?PIECE_GREEN:level==DG_NORMAL?PIECE_YELLOW:PIECE_RED;
 for(int dy=-9;dy<=9;dy++)for(int dx=-9;dx<=9;dx++){
  int square=dx*dx+dy*dy;
  uint16_t expected=square>81?background:square>64?UI_BLACK:fill;
  bool eye=(dy==-3 || dy==-2) && (dx==-4 || dx==-3 || dx==3 || dx==4);
  bool mouth=level==DG_NORMAL?(dy==3 && dx>=-4 && dx<=4):
   (dy==(level==DG_EASY?4:2) && dx>=-2 && dx<=2) ||
   (dy==3 && (dx==-3 || dx==3)) ||
   (dy==(level==DG_EASY?2:4) && (dx==-4 || dx==4));
  bool eyebrow=false;
  if(level==DG_HARD)for(unsigned i=0;i<10;i++)if(dx==brow[i][0] && dy==brow[i][1])eyebrow=true;
  if(eye || mouth || eyebrow)expected=UI_BLACK;
  assert(pixels[cy+dy][cx+dx]==expected);
 }
}
static void options(DgApp *app)
{
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(unsigned index=0;index<3;index++)for(uint8_t slot=0;slot<players;slot++){
  uint8_t level=levels[index];
  app->screen=DG_SETUP;app->players=players;app->level=level;app->slot=slot;render(app);menu_strip(false,true);
  for(unsigned i=0;i<3;i++){
   box(7+(int)i*130,51,122,46,level==levels[i]?UI_FOCUS:UI_LINE,level==levels[i]?2:1);
   assert(pixels[55][10+i*130]==(level==levels[i]?UI_SELECTED:UI_WHITE));
  }
  int width=players==2?187:122,gap=players==2?195:130;
  for(uint8_t i=0;i<players;i++){
   box(7+i*gap,131,width,46,slot==i?UI_FOCUS:UI_LINE,slot==i?2:1);
   assert(pixels[135][10+i*gap]==(slot==i?UI_SELECTED:UI_WHITE));
  }
  /* LEVEL and FIRST/HUMAN use the same font scale and centered baseline. */
  const char *const level_names[3]={"EASY","NORMAL","HARD"};
  for(unsigned i=0;i<3;i++){
   const char *name=level_names[i];int x=7+(int)i*130,left=x+(122-ui_text_width(name,3,2)-30)/2;
   expected_text(left+30,66,name,UI_INK,3,2);
   assert(left>=x+3 && left+30+ui_text_width(name,3,2)<x+119);
   face(left+9,74,levels[i],level==levels[i]?UI_SELECTED:UI_WHITE);
  }
  const char *const names[3]={"1ST","2ND","3RD"};
  for(uint8_t i=0;i<players;i++){
   const char *name=players==2?(i?"AI":"HUMAN"):names[i];
   expected_text(7+i*gap+(width-ui_text_width(name,3,2))/2,146,name,UI_INK,3,2);
  }
 }
 for(uint8_t assist=0;assist<2;assist++){
  app->screen=DG_SETTINGS;app->archive.assist=assist;render(app);blank_strip();
  box(7,61,187,46,assist?UI_LINE:UI_FOCUS,assist?1:2);
  box(202,61,187,46,assist?UI_FOCUS:UI_LINE,assist?2:1);
 }
}
static void relocate(DgGame *game,uint8_t player,int destination)
{
 assert(destination>=0 && !game->pos.board[destination]);
 for(int n=0;n<DG_NODES;n++)if(game->pos.board[n]==player && dg_in_camp(n,dg_home[player])){
  game->pos.board[n]=0;game->pos.board[destination]=player;return;
 }
 assert(false);
}
static void disc_contract(int x,int y,int radius,uint16_t fill)
{
 for(int dy=-radius-1;dy<=radius+1;dy++)for(int dx=-radius-1;dx<=radius+1;dx++){
  int square=dx*dx+dy*dy;
  if(square>(radius+1)*(radius+1))continue;
  uint16_t expected=square<=radius*radius?fill:UI_BLACK;
  if(pixels[y+dy][x+dx]!=expected)fprintf(stderr,"disc center=%d,%d radius=%d pixel=%d,%d expected=%04x actual=%04x\n",x,y,radius,dx,dy,(unsigned)expected,(unsigned)pixels[y+dy][x+dx]);
  assert(pixels[y+dy][x+dx]==expected);
 }
}
static void board_fill(DgApp *app)
{
 assert(dg_new(&app->archive.game,3,DG_EASY,0,123456));app->screen=DG_GAME;
 const uint8_t players[3]={DG_RED,DG_GREEN,DG_YELLOW};
 int locations[3]={dg_coord(-1,-1),dg_coord(1,-1),dg_coord(-1,1)};
 for(unsigned i=0;i<3;i++)relocate(&app->archive.game,players[i],locations[i]);
 int source=dg_coord(-2,0),middle=dg_coord(-1,0),destination=dg_coord(0,0),empty=dg_coord(2,1);
 relocate(&app->archive.game,DG_RED,source);relocate(&app->archive.game,DG_GREEN,middle);
 assert(dg_game_valid(&app->archive.game));
 assert(dg_find_move(app->archive.game.pos.board,DG_RED,source,destination,NULL,NULL));
 assert(!dg_find_move(app->archive.game.pos.board,DG_RED,source,empty,NULL,NULL));
 for(uint8_t zoom=0;zoom<2;zoom++){
  app->zoom=zoom;app->cursor=(uint8_t)empty;app->selected=DG_NONE;
  app->path.length=0;app->archive.assist=1;render(app);int radius=zoom?7:4,x,y;
  for(unsigned i=0;i<3;i++){
   dg_screen_position(app,locations[i],&x,&y);disc_contract(x,y,radius,ui_piece_color(players[i]));
  }
  dg_screen_position(app,empty,&x,&y);disc_contract(x,y,radius,UI_WHITE);
  app->selected=(uint8_t)source;render(app);
  dg_screen_position(app,destination,&x,&y);disc_contract(x,y,radius,BOARD_CYAN);
  assert(pixels[y][x-radius-2]!=BOARD_CYAN && pixels[y][x+radius+2]!=BOARD_CYAN);
  dg_screen_position(app,source,&x,&y);disc_contract(x,y,radius,PIECE_RED);
  assert(pixels[y][x+radius+2]==PIECE_YELLOW && pixels[y][x+radius+4]==BOARD_INK);
  app->cursor=(uint8_t)destination;render(app);
  dg_screen_position(app,destination,&x,&y);disc_contract(x,y,radius,BOARD_CYAN);
  assert(pixels[y-radius-5][x]==BOARD_BLUE && pixels[y+radius+5][x]==BOARD_BLUE);
  assert(pixels[y][x-radius-5]==BOARD_BLUE && pixels[y][x+radius+5]==BOARD_BLUE);
  app->archive.assist=0;render(app);disc_contract(x,y,radius,UI_WHITE);
  for(int r=UI_BOARD_TOP;r<UI_BOARD_BOTTOM;r++)for(int c=0;c<396;c++)assert(pixels[r][c]!=BOARD_CYAN);
 }
 app->zoom=0;app->selected=DG_NONE;app->cursor=36;app->archive.assist=1;
}
static bool overlap(int x,int y,int w,int h,int bx,int by,int bw,int bh)
{return x<bx+bw && bx<x+w && y<by+bh && by<y+h;}
static void board_bounds(DgApp *app)
{
 for(uint8_t zoom=0;zoom<2;zoom++)for(uint8_t n=0;n<DG_NODES;n++){
  app->zoom=zoom;app->cursor=n;int x,y;dg_screen_position(app,n,&x,&y);
  int radius=zoom?12:9;
  assert(x-radius-2>=4 && x+radius+2<392 && y-radius>=26 && y+radius<204);
  assert(!overlap(x-radius-2,y-radius,2*radius+5,2*radius+1,6,30,99,17));
  assert(!overlap(x-radius-2,y-radius,2*radius+5,2*radius+1,284,30,106,51));
  render(app);game_strip(false,"SELECT",true);
  /* All four cursor edges survive clipping and HUD overlays at every node. */
  assert(pixels[y-radius][x]==BOARD_BLUE && pixels[y+radius][x]==BOARD_BLUE);
  assert(pixels[y][x-radius]==BOARD_BLUE && pixels[y][x+radius]==BOARD_BLUE);
 }
 app->zoom=0;app->cursor=36;
}
static void notices(DgApp *app)
{
 static const char *const messages[]={"INVALID MOVE","DESTINATION OCCUPIED","SELECT YOUR PIECE",
  "SAVE FAILED - RETRY EXIT","NEW GAME SAVE FAILED","CPU HAS NO MOVE","CPU MOVE REJECTED",
  "STORAGE CLOSE FAILED","RECOVERED OLDER SAVE","INVALID SAVE - FRESH SETUP","SAVE READ ERROR","IDLE TIMER UNAVAILABLE"};
 app->notice[0]=0;render(app);memcpy(prior,pixels,sizeof prior);
 for(unsigned i=0;i<sizeof messages/sizeof messages[0];i++){
  snprintf(app->notice,sizeof app->notice,"%s",messages[i]);render(app);
  const char *visible=messages[i];
  if(!strcmp(visible,"CPU HAS NO MOVE"))visible="AI HAS NO MOVE";
  else if(!strcmp(visible,"CPU MOVE REJECTED"))visible="AI MOVE REJECTED";
  int width=ui_text_width(visible,1,1)+12;assert(width<300);
  for(int y=0;y<204;y++)for(int x=0;x<396;x++)
   if(!(x>=6 && x<6+width && y>=28 && y<47))assert(pixels[y][x]==prior[y][x]);
  expected_text(12,32,visible,UI_RUN,1,1);
  for(int y=28;y<47;y++)assert(pixels[y][6]==UI_RUN);
 }
 app->notice[0]=0;
}
static void modal_bounds(DgApp *app)
{
 app->modal=DG_MODAL_RESTART;render(app);blank_strip();box(76,58,244,88,UI_INK,2);
 expected_text(76+(244-ui_text_width("RESTART GAME?",1,1))/2,75,"RESTART GAME?",UI_INK,1,1);
 expected_text(98,109,"EXE: YES",UI_INK,1,1);expected_text(98,126,"EXIT: NO",UI_INK,1,1);
 app->modal=DG_MODAL_RESULT;
 for(uint8_t players=2;players<=3;players++)for(uint8_t winner=DG_RED;winner<=DG_GREEN;winner++){
  if(players==2 && winner==DG_YELLOW)continue;
  app->archive.game.players=players;app->archive.game.pos.winner=winner;
  app->archive.game.pos.turns=UINT32_MAX;render(app);blank_strip();box(76,50,244,104,UI_INK,2);
  const char *title=winner==DG_RED?"YOU WIN":players==2?"AI WINS":winner==DG_YELLOW?"YELLOW AI WINS":"GREEN AI WINS";
  int tw=ui_text_width(title,3,2);assert(tw<=200);
  expected_text(76+(244-tw)/2,67,title,UI_INK,3,2);
  char stats[32];snprintf(stats,sizeof stats,"TURNS %lu   EASY",(unsigned long)UINT32_MAX);
  int sw=ui_text_width(stats,1,1);assert(sw<=200);
  expected_text(76+(244-sw)/2,93,stats,UI_MUTED,1,1);
  expected_text(98,114,"EXE: NEW GAME",UI_INK,1,1);expected_text(98,131,"EXIT: VIEW BOARD",UI_INK,1,1);
 }
 app->modal=DG_MODAL_NONE;render(app);game_strip(false,"NEW",false);
}
static void hud_bounds(DgApp *app)
{
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(unsigned index=0;index<3;index++){
  uint8_t level=levels[index];
  assert(dg_new(&app->archive.game,players,level,0,123456));
  app->archive.game.pos.turns=UINT32_MAX;
  for(uint8_t turn=0;turn<players;turn++){
   app->archive.game.pos.turn=turn;render(app);
   uint8_t current=dg_current(&app->archive.game);
   const char *label=current==DG_RED?"HUMAN":current==DG_GREEN?"GREEN AI":"YELLOW AI";
   const char *level_name=index==0?"EASY":index==1?"NORMAL":"HARD";
   char config[20],counter[20];snprintf(config,sizeof config,"%uP / %s",(unsigned)players,level_name);
   snprintf(counter,sizeof counter,"T %lu",(unsigned long)UINT32_MAX);
   int cw=ui_text_width(config,1,1),cx=(396-cw)/2,tx=388-ui_text_width(counter,1,1);
   assert(50+ui_text_width(label,1,1)<cx && cx+cw<tx);
   expected_text(8,6,"TURN",UI_MUTED,1,1);expected_text(50,6,label,UI_INK,1,1);
   expected_text(cx,6,config,UI_MUTED,1,1);expected_text(tx,6,counter,UI_MUTED,1,1);
  }
 }
}
int main(void)
{
 assert(ui_text_width("RESTART",1,1)==57 && ui_text_width("SELECT",1,1)==48);
 assert(UI_RESTART==0xffe0 && UI_UNDO==0xf81f && UI_SET==0x37e6 && UI_NEXT==0x07ff && UI_RUN==0xf800);
 DgApp app;dg_app_init(&app,(DgHooks){0},123456);
 for(uint8_t players=2;players<=3;players++){
  app.players=players;render(&app);menu_strip(false,false);
  box(7,54,187,128,players==2?UI_FOCUS:UI_LINE,players==2?3:1);
  box(202,54,187,128,players==3?UI_FOCUS:UI_LINE,players==3?3:1);
  expected_text(9,35,"PLAYER",UI_MUTED,1,1);
  expected_text(19+ui_text_width("PLAYER",1,1),35,"(YOU ARE RED)",PIECE_RED,1,1);
  actor(68,105,DG_RED,false);actor(132,105,DG_GREEN,true);
  actor(247,105,DG_RED,false);actor(295,105,DG_YELLOW,true);actor(343,105,DG_GREEN,true);
 }
 app.archive.active=1;render(&app);menu_strip(true,false);options(&app);
 app.screen=DG_RULES;
 for(uint8_t offset=0;offset<=12;offset++){
  app.rules_scroll=offset;render(&app);blank_strip();int thumb=151*9/21,top=35+offset*(151-thumb)/12;
  for(int y=35;y<186;y++)assert(pixels[y][383]==(y>=top && y<top+thumb?UI_BLUE:UI_LINE));
 }
 assert(dg_new(&app.archive.game,3,DG_EASY,0,123456));app.screen=DG_GAME;app.archive.assist=1;
 board_bounds(&app);board_fill(&app);notices(&app);hud_bounds(&app);
 assert(dg_new(&app.archive.game,3,DG_EASY,0,123456));
 /* Existing undo-enabled condition at the end of a full human decision. */
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 assert(dg_commit(&app.archive.game,&moves[0]));
 while(dg_current(&app.archive.game)!=DG_RED){
  assert(dg_app_cpu(&app,NULL,NULL));render(&app);game_strip(false,"",false);
  while(app.animation)assert(dg_app_animation(&app));
 }
 render(&app);game_strip(true,"SELECT",true);
 app.selected=moves[0].to;render(&app);game_strip(true,"MOVE",true);
 app.selected=DG_NONE;app.thinking=1;render(&app);game_strip(false,"",false);app.thinking=0;
 modal_bounds(&app);
 printf("Renderer geometry PASS: %u immutable frames, %u bounded rectangles; exact softkeys, faces/AI icons, solid discs/Assist, fonts, selectors, HUD/cursors, all rule offsets, notices and modals\n",frames,calls);
 return 0;
}
