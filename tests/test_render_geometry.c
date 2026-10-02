/* Pixel contracts for the actual renderer, independent expected font masks. */
#include "../src/ui/draw.h"
#include "../src/ui/font_data.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint16_t pixels[224][396],prior[224][396];
static unsigned frames,calls,paper_count;
static UiBox paper_boxes[16];
static void raster(void *context,int x,int y,int w,int h,uint16_t ink)
{
 (void)context;
 if(ink==UI_PAPER && !(x==0 && y==0 && w==396 && h==224)){assert(paper_count<16);paper_boxes[paper_count++]=(UiBox){x,y,w,h};}
 assert(w>0 && h>0 && x>=0 && y>=0 && x+w<=396 && y+h<=224);calls++;
 for(int row=y;row<y+h;row++)for(int col=x;col<x+w;col++)pixels[row][col]=ink;
}
static void render(const DgApp *app)
{
 paper_count=0;DgApp before=*app;dg_render(app,&(DgCanvas){NULL,raster});
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
 const char *labels[6]={!setup && resume?"RESUME":"","","","RULES","",setup?"OPEN":"NEXT"};
 uint16_t bg[6]={UI_BLUE,0,0,UI_BLACK,0,setup?UI_INK:UI_NEXT};
 uint16_t fg[6]={UI_WHITE,0,0,UI_WHITE,0,setup?UI_WHITE:UI_BLACK};strip(labels,bg,fg);
}
static void game_strip(bool undo,const char *action,bool idle)
{
 bool busy=!idle && !action[0];
 const char *labels[6]={idle?"RESTART":"",undo?"UNDO":"","",busy?"":"RULES",busy?"":"ZOOM",action};
 uint16_t bg[6]={UI_RESTART,UI_UNDO,0,UI_BLACK,UI_BLUE,UI_RUN};
 uint16_t fg[6]={UI_BLACK,UI_BLACK,0,UI_WHITE,UI_WHITE,UI_WHITE};strip(labels,bg,fg);
}
static void blank_strip(void)
{for(int y=204;y<224;y++)for(int x=0;x<396;x++)assert(pixels[y][x]==UI_WHITE);}
static void box(int x,int y,int w,int h,uint16_t ink,int thick)
{
 for(int r=0;r<h;r++)for(int c=0;c<w;c++)if(r<thick || r>=h-thick || c<thick || c>=w-thick)
  {if(pixels[y+r][x+c]!=ink)fprintf(stderr,"box=%d,%d %dx%d thick=%d pixel=%d,%d expected=%04x actual=%04x\n",x,y,w,h,thick,c,r,ink,pixels[y+r][x+c]);assert(pixels[y+r][x+c]==ink);}
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
  if(!ai && (dx*dx+(dy+5)*(dy+5)<=16 ||
     (dy>=1 && dy<=9 && dx*dx+(dy-9)*(dy-9)<=81)))expected=UI_WHITE;
  assert(pixels[y+dy][x+dx]==expected);
 }
 if(ai)expected_text(text_x,y-5,"AI",UI_BLACK,1,1);
}
static void option_contract(int y,const char *const *names,unsigned count,unsigned selected,const uint16_t *inks)
{
 int total=0;for(unsigned i=0;i<count;i++)total+=ui_text_width(names[i],1,1)+10;
 int gap=(228-total)/(int)(count-1),x=146;
 for(unsigned i=0;i<count;i++){
  int w=ui_text_width(names[i],1,1)+10;uint16_t ink=inks?inks[i]:UI_BLUE;
  expected_text(x+5,y+9,names[i],ink,1,1);
  if(i==selected){box(x,y+4,w,21,ink,1);for(int dx=3;dx<w-3;dx++)assert(pixels[y+23][x+dx]==ink);}
  assert(x>=146 && x+w<=374);x+=w+gap;
 }
}
static void options(DgApp *app)
{
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 static const char *const level_names[3]={"EASY","NORMAL","HARD"};
 static const char *const first[2]={"YOU","AI"},*const slots[3]={"1ST","2ND","3RD"},*const assist_names[2]={"OFF","ON"};
 static const uint16_t inks[3]={PIECE_GREEN,UI_GOLD,PIECE_RED};
 for(uint8_t players=2;players<=3;players++)for(unsigned resume=0;resume<3;resume++){
  assert(dg_new(&app->archive.game,resume==2?(players==2?3:2):players,DG_NORMAL,0,123456));
  app->archive.active=resume!=0;app->screen=DG_SETUP;app->players=players;
  unsigned count=resume==1?5u:4u;
  for(unsigned index=0;index<3;index++)for(uint8_t slot=0;slot<players;slot++)for(uint8_t assist=0;assist<2;assist++)for(unsigned focus=0;focus<count;focus++){
   app->level=levels[index];app->slot=slot;app->archive.assist=assist;app->focus=(uint8_t)focus;
   render(app);menu_strip(false,true);
   if(resume==1)expected_text(12+ui_text_width("Saved: ",1,1)+1,173,"NORMAL",UI_YELLOW_TEXT,1,1);
   for(unsigned row=0;row<count;row++){
    int y=(count==5?30:34)+(int)row*(count==5?27:34),h=count==5?25:29;char number[4];
    snprintf(number,sizeof number,"%u",row+1);expected_text(20,y+9,number,UI_MUTED,1,1);
    unsigned offset=resume==1?1u:0u;
    const char *label=row<offset?"RESUME":row==offset?"NEW GAME":row==offset+1?"DIFFICULTY":row==offset+2?(players==2?"FIRST":"YOU"):"ASSIST";
    expected_text(43,y+9,label,UI_INK,1,1);
    int thick=row==focus?2:1;uint16_t edge=row==focus?UI_BLUE:UI_LINE;
    for(int r=0;r<h;r++)for(int c=0;c<376;c++)if(r<thick || r>=h-thick || c<thick || c>=376-thick){
     /* NUM GAME compact options occupy the row's bottom value-area border. */
     if(count==5 && row>offset && r>=23 && c>=136 && c<364)continue;
     assert(pixels[y+r][10+c]==edge);
    }
    assert(pixels[y+3][13]==(row==focus?UI_SELECTED:UI_WHITE));
    if(row==offset+1)option_contract(y,level_names,3,index,inks);
    if(row==offset+2)option_contract(y,players==2?first:slots,players,slot,NULL);
    if(row==offset+3)option_contract(y,assist_names,2,assist,NULL);
   }
  }
 }
 app->focus=0;
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
 assert(dg_find_move(dg_rules(&app->archive.game),app->archive.game.pos.board,DG_RED,source,destination,NULL,NULL));
 assert(!dg_find_move(dg_rules(&app->archive.game),app->archive.game.pos.board,DG_RED,source,empty,NULL,NULL));
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
  /* Compact status panels must leave every focused cursor visible. */
  assert(!overlap(x-radius-2,y-radius,2*radius+5,2*radius+1,6,30,ui_text_width("ASSIST OFF",1,1)+4,15));
  assert(!overlap(x-radius-2,y-radius,2*radius+5,2*radius+1,386-ui_text_width("YELLOW 10/10",1,1),30,ui_text_width("YELLOW 10/10",1,1)+4,45));
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
  UiNotice notice;ui_notice_layout(app,&notice);UiBox b=notice.box;
  assert(b.x==6 && b.y==84 && b.w<=107 && b.h<=54);
  for(int y=0;y<204;y++)for(int x=0;x<396;x++)
   if(!(x>=b.x && x<b.x+b.w && y>=b.y && y<b.y+b.h))assert(pixels[y][x]==prior[y][x]);
  for(unsigned row=0;row<notice.count;row++)expected_text(b.x+2,b.y+2+(int)row*13,notice.line[row],UI_RUN,1,1);
  /* Visible spelling is preserved through wrapping, including CPU -> AI. */
  char joined[64]="";for(unsigned row=0;row<notice.count;row++){if(row)strcat(joined," ");strcat(joined,notice.line[row]);}
  assert(!strcmp(joined,visible));
 }
 app->notice[0]=0;
}
static void modal_bounds(DgApp *app)
{
 app->modal=DG_MODAL_RESTART;render(app);blank_strip();box(76,58,244,88,UI_INK,2);
 expected_text(76+(244-ui_text_width("RESTART GAME?",1,1))/2,75,"RESTART GAME?",UI_INK,1,1);
 expected_text(98,109,"EXE: YES",UI_INK,1,1);expected_text(98,126,"EXIT: NO",UI_INK,1,1);
 app->modal=DG_MODAL_RESULT;
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(uint8_t winner=DG_RED;winner<=DG_GREEN;winner++)for(unsigned index=0;index<3;index++){
  if(players==2 && winner==DG_YELLOW)continue;
  app->archive.game.players=players;app->archive.game.pos.winner=winner;app->archive.game.level=levels[index];
  app->archive.game.pos.turns=UINT32_MAX;render(app);blank_strip();box(76,50,244,104,UI_INK,2);
  const char *title=winner==DG_RED?"YOU WIN":players==2?"AI WINS":winner==DG_YELLOW?"YELLOW AI WINS":"GREEN AI WINS";
  int tw=ui_text_width(title,3,2);assert(tw<=200);
  expected_text(76+(244-tw)/2,67,title,ui_actor_color(winner),3,2);
  const char *level=ui_level_label(levels[index]);
  char stats[32],prefix[32];snprintf(stats,sizeof stats,"TURNS %lu   %s",(unsigned long)UINT32_MAX,level);
  snprintf(prefix,sizeof prefix,"TURNS %lu   ",(unsigned long)UINT32_MAX);
  int sw=ui_text_width(stats,1,1);assert(sw<=200);
  expected_text(76+(244-sw)/2,93,prefix,UI_MUTED,1,1);
  expected_text(76+(244-sw)/2+sw-ui_text_width(level,1,1),93,level,ui_level_color(levels[index]),1,1);
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
   const char *label=current==DG_RED?"YOU":current==DG_GREEN?"GREEN AI":"YELLOW AI";
   const char *level_name=index==0?"EASY":index==1?"NORMAL":"HARD";
   char config[20],counter[20];snprintf(config,sizeof config,"%uP / %s",(unsigned)players,level_name);
   snprintf(counter,sizeof counter,"T %lu",(unsigned long)UINT32_MAX);
   int cw=ui_text_width(config,1,1),cx=(396-cw)/2,tx=388-ui_text_width(counter,1,1);
   int actor_x=8+ui_text_width("TURN : ",1,1)+1;
   assert(actor_x+ui_text_width(label,1,1)<cx && cx+cw<tx);
   expected_text(8,6,"TURN :",UI_INK,1,1);expected_text(actor_x,6,label,ui_actor_color(current),1,1);
   expected_text(cx,6,config,ui_level_color(level),1,1);expected_text(tx,6,counter,UI_MUTED,1,1);
  }
 }
}
static bool boxes_overlap(UiBox a,UiBox b)
{return overlap(a.x,a.y,a.w,a.h,b.x,b.y,b.w,b.h);}
static void overlay_bounds(DgApp *app)
{
 /* Whole overview board bbox includes the largest cursor ticks, not just holes. */
 UiBox board={396,224,0,0};int right=0,bottom=0;app->zoom=0;
 for(int n=0;n<DG_NODES;n++){
  int x,y;dg_screen_position(app,n,&x,&y);
  if(x-11<board.x)board.x=x-11;if(y-9<board.y)board.y=y-9;
  if(x+12>right)right=x+12;if(y+10>bottom)bottom=y+10;
 }
 board.w=right-board.x;board.h=bottom-board.y;
 assert(board.x==115 && board.y==27 && right==282 && bottom==202);
 assert(!boxes_overlap(board,(UiBox){0,0,396,24}));
 UiBox thinking=ui_thinking_box();assert(thinking.x==6 && thinking.y==57 && thinking.w==ui_text_width("THINKING...",1,1)+4 && thinking.h==15);
 for(uint8_t zoom=0;zoom<2;zoom++)for(uint8_t players=2;players<=3;players++)for(uint8_t assist=0;assist<2;assist++){
  assert(dg_new(&app->archive.game,players,DG_NORMAL,0,123456));
  app->screen=DG_GAME;app->modal=DG_MODAL_NONE;app->zoom=zoom;app->cursor=36;app->selected=DG_NONE;
  app->archive.assist=assist;app->thinking=1;
  for(uint8_t phase=0;phase<3;phase++){
   app->thinking_phase=phase;render(app);assert(paper_count==(unsigned)players+2u);
   assert(paper_boxes[0].w==ui_text_width(assist?"ASSIST ON":"ASSIST OFF",1,1)+4 && paper_boxes[0].h==15);
   for(unsigned i=0;i<paper_count;i++){
    UiBox b=paper_boxes[i];assert(b.w<=107 && b.h==15);
    if(!zoom){assert(!boxes_overlap(b,board));assert(b.x+b.w<=board.x-2 || b.x>=right+2);}
   }
   static const char *const phases[3]={"THINKING.","THINKING..","THINKING..."};
   for(int dy=0;dy<thinking.h;dy++)for(int dx=0;dx<thinking.w;dx++){
    uint16_t expected=glyph_pixel(dx-2,dy-2,phases[phase])?UI_BLUE:UI_PAPER;
    assert(pixels[thinking.y+dy][thinking.x+dx]==expected);
   }
  }
  app->thinking_phase=2;render(app);memcpy(prior,pixels,sizeof prior);DgArchive frozen=app->archive;uint32_t new_rng=app->new_rng;
  assert(!dg_app_thinking_tick(app,119));assert(dg_app_thinking_tick(app,120) && !app->thinking_phase);
  paper_count=0;dg_render_thinking(app,&(DgCanvas){NULL,raster});assert(paper_count==1 && !memcmp(&paper_boxes[0],&thinking,sizeof thinking));
  for(int y=0;y<224;y++)for(int x=0;x<396;x++){
   if(x>=thinking.x && x<thinking.x+thinking.w && y>=thinking.y && y<thinking.y+thinking.h)
    assert(pixels[y][x]==(glyph_pixel(x-thinking.x-2,y-thinking.y-2,"THINKING.")?UI_BLUE:UI_PAPER));
   else assert(pixels[y][x]==prior[y][x]);
  }
  assert(!memcmp(&frozen,&app->archive,sizeof frozen) && app->new_rng==new_rng);
  app->thinking=0;assert(!dg_app_thinking_tick(app,160));
  snprintf(app->notice,sizeof app->notice,"INVALID SAVE - FRESH SETUP");UiNotice notice;ui_notice_layout(app,&notice);
  assert(notice.box.w<=107 && notice.box.h<=54);
  if(!zoom)assert(!boxes_overlap(notice.box,board) && notice.box.x+notice.box.w<=board.x-2);
  render(app);app->notice[0]=0;
 }
 app->zoom=0;app->thinking=0;
 printf("Overview board bbox: %d,%d %dx%d; THINKING bbox: %d,%d %dx%d; panels measured +4 padding, warnings <=107x54\n",board.x,board.y,board.w,board.h,thinking.x,thinking.y,thinking.w,thinking.h);
}
static void goal_colors(DgApp *app)
{
 for(uint8_t players=2;players<=3;players++){
  assert(dg_new(&app->archive.game,players,DG_NORMAL,0,123456));
  DgGame *g=&app->archive.game;memset(g->pos.board,0,sizeof g->pos.board);
  g->rules_revision=DG_RULES_V1; /* Preserve the historical color-layout fixture. */
  const unsigned counts[4]={0,5,3,4};
  for(uint8_t who=DG_RED;who<=DG_GREEN;who++)if(who!=DG_YELLOW || players==3){
   unsigned used=0;for(int n=0;n<DG_NODES && used<counts[who];n++)if(dg_in_camp(n,dg_goal[who])){assert(!g->pos.board[n]);g->pos.board[n]=who;used++;}
   for(int n=0;n<DG_NODES && used<10;n++)if(!g->pos.board[n] && !dg_in_camp(n,dg_goal[DG_RED]) && !dg_in_camp(n,dg_goal[DG_YELLOW]) && !dg_in_camp(n,dg_goal[DG_GREEN])){g->pos.board[n]=who;used++;}
   assert(used==10);
  }
  assert(dg_game_valid(g));app->screen=DG_GAME;app->modal=0;app->selected=DG_NONE;app->zoom=0;render(app);
  unsigned row=0;
  for(uint8_t who=DG_RED;who<=DG_GREEN;who++)if(who!=DG_YELLOW || players==3){
   char text[24];snprintf(text,sizeof text,"%s %u/10",who==DG_RED?"YOU":who==DG_YELLOW?"YELLOW":"GREEN",counts[who]);
   int w=ui_text_width(text,1,1),x=388-w,y=32+(int)row++*15;
   expected_text(x,y,text,who==DG_YELLOW?UI_GOLD:who==DG_RED?PIECE_RED:PIECE_GREEN,1,1);
   for(int dy=-2;dy<13;dy++)for(int dx=-2;dx<w+2;dx++)
    assert(pixels[y+dy][x+dx]==(glyph_pixel(dx,dy,text)?ui_actor_color(who):UI_PAPER));
  }
 }
}
static void animated_hud(void)
{
 DgApp app;dg_app_init(&app,(DgHooks){0},812713);
 assert(dg_new(&app.archive.game,3,DG_NORMAL,1,1));app.archive.active=1;app.screen=DG_GAME;
 uint8_t actor=dg_current(&app.archive.game);assert(dg_app_cpu(&app,NULL,NULL));
 assert(app.animation && actor!=dg_current(&app.archive.game));render(&app);blank_strip();
 int x=8+ui_text_width("TURN : ",1,1)+1;
 expected_text(x,6,actor==DG_GREEN?"GREEN AI":"YELLOW AI",ui_actor_color(actor),1,1);
 while(app.animation)assert(dg_app_animation(&app));render(&app);
 uint8_t current=dg_current(&app.archive.game);
 expected_text(x,6,current==DG_RED?"YOU":current==DG_GREEN?"GREEN AI":"YELLOW AI",ui_actor_color(current),1,1);
}
int main(void)
{
 assert(ui_text_width("RESTART",1,1)==57 && ui_text_width("SELECT",1,1)==48);
 assert(UI_RESTART==0xffe0 && UI_UNDO==0xf81f && UI_NEXT==0x07ff && UI_RUN==0xf800);
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
 assert(dg_new(&app.archive.game,3,DG_NORMAL,1,123456));app.archive.active=1;app.players=2;render(&app);menu_strip(true,false);options(&app);
 app.screen=DG_RULES;
 for(uint8_t offset=0;offset<=15;offset++){
  app.rules_scroll=offset;render(&app);blank_strip();int thumb=151*9/24,top=35+offset*(151-thumb)/15;
  for(int y=35;y<186;y++)assert(pixels[y][383]==(y>=top && y<top+thumb?UI_BLUE:UI_LINE));
 }
 assert(dg_new(&app.archive.game,3,DG_EASY,0,123456));app.screen=DG_GAME;app.archive.assist=1;
 board_bounds(&app);board_fill(&app);notices(&app);hud_bounds(&app);overlay_bounds(&app);goal_colors(&app);
 assert(dg_new(&app.archive.game,3,DG_EASY,0,123456));
 /* Existing undo-enabled condition at the end of a full human decision. */
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(dg_rules(&app.archive.game),app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 assert(dg_commit(&app.archive.game,&moves[0]));
 while(dg_current(&app.archive.game)!=DG_RED){
  assert(dg_app_cpu(&app,NULL,NULL));render(&app);game_strip(false,"",false);
  while(app.animation)assert(dg_app_animation(&app));
 }
 render(&app);game_strip(true,"SELECT",true);
 app.selected=moves[0].to;render(&app);game_strip(true,"MOVE",true);
 app.selected=DG_NONE;app.thinking=1;render(&app);game_strip(false,"",false);app.thinking=0;
 modal_bounds(&app);animated_hud();
 printf("Renderer geometry PASS: %u immutable frames, %u bounded rectangles; exact softkeys, profile/AI icons, solid discs/Assist, fonts, selectors, HUD/cursors, all rule offsets, notices and modals\n",frames,calls);
 return 0;
}
