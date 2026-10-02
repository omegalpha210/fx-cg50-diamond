#include "ui.h"
#include "../src/ui/draw.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {unsigned saves,menus,offs,rects;bool fail;} Fixture;
static bool save(void *context,DgArchive *archive)
{Fixture *f=context;f->saves++;assert(!archive->active || dg_game_valid(&archive->game));return !f->fail;}
static void system_menu(void *context,bool off)
{Fixture *f=context;if(off)f->offs++;else f->menus++;}
static void raster(void *context,int x,int y,int w,int h,uint16_t color)
{
 Fixture *f=context;(void)color;assert(w>0 && h>0 && x>=0 && y>=0 && x+w<=396 && y+h<=224);f->rects++;
}
static bool cancel(void *context){(void)context;return true;}
static void redraw(DgApp *app,Fixture *f)
{unsigned before=f->rects;dg_render(app,&(DgCanvas){f,raster});assert(f->rects>before);}
static void start(DgApp *app,uint8_t players)
{
 app->screen=DG_PLAYER;app->players=players;app->slot=0;app->level=DG_EASY;
 assert(dg_app_key(app,DGK_F6));assert(app->screen==DG_SETUP);app->focus=(uint8_t)dg_entry_row(app,DG_ENTRY_NEW);assert(dg_app_key(app,DGK_F6));
 assert(app->screen==DG_GAME && app->archive.active && dg_current(&app->archive.game)==DG_RED);
}
static void navigation(DgApp *app)
{
 int queue[DG_NODES],head=0,tail=0;bool seen[DG_NODES]={false};
 app->cursor=36;queue[tail++]=36;seen[36]=true;
 while(head<tail){int from=queue[head++];for(int d=0;d<4;d++){
  app->cursor=(uint8_t)from;assert(dg_app_key(app,DGK_UP+d));int to=dg_nodes[from].nav[d];assert(app->cursor==(uint8_t)to);
  if(!seen[to]){seen[to]=true;queue[tail++]=to;}
 }}
 assert(tail==DG_NODES);
 for(int n=0;n<DG_NODES;n++){
  int x,y;app->cursor=(uint8_t)n;app->zoom=0;dg_screen_position(app,n,&x,&y);assert(x-9>=4 && x+9<392 && y-9>=UI_BOARD_TOP && y+9<UI_BOARD_BOTTOM);
  uint8_t selected=app->selected;assert(dg_app_key(app,DGK_F5));assert(app->cursor==(uint8_t)n && app->selected==selected && app->zoom==1);
  dg_screen_position(app,n,&x,&y);assert(x-12>=4 && x+12<392 && y-12>=UI_BOARD_TOP && y+12<UI_BOARD_BOTTOM);
  for(int d=0;d<4;d++){app->cursor=(uint8_t)n;assert(dg_app_key(app,DGK_UP+d));assert(app->cursor==(uint8_t)dg_nodes[n].nav[d]);}
 }
 app->zoom=0;app->cursor=36;
}
int main(void)
{
 Fixture f={0};DgApp app;dg_app_init(&app,(DgHooks){&f,save,system_menu},1234);
 assert(app.screen==DG_PLAYER && app.players==3 && app.archive.assist==1);redraw(&app,&f);
 assert(dg_app_key(&app,DGK_F6));assert(app.screen==DG_SETUP);
 assert(DG_EASY==0 && DG_HARD==1 && DG_NORMAL==2);
 assert(dg_app_key(&app,DGK_DOWN));
 assert(dg_app_key(&app,DGK_LEFT));assert(app.level==DG_EASY);
 assert(dg_app_key(&app,DGK_RIGHT));assert(app.level==DG_NORMAL);
 assert(dg_app_key(&app,DGK_RIGHT));assert(app.level==DG_HARD);
 assert(dg_app_key(&app,DGK_RIGHT));assert(app.level==DG_HARD);
 assert(dg_app_key(&app,DGK_LEFT));assert(app.level==DG_NORMAL);
 assert(dg_app_key(&app,DGK_LEFT));assert(app.level==DG_EASY);
 assert(dg_app_key(&app,DGK_LEFT));assert(app.level==DG_EASY);
 assert(dg_app_key(&app,DGK_DOWN));assert(dg_app_key(&app,DGK_RIGHT));assert(dg_app_key(&app,DGK_RIGHT));assert(dg_app_key(&app,DGK_RIGHT));assert(app.slot==2);
 assert(dg_app_key(&app,DGK_LEFT));assert(app.slot==1);redraw(&app,&f);
 assert(dg_app_key(&app,DGK_EXE));assert(app.screen==DG_GAME && app.archive.game.human_slot==1);
 DgGame original=app.archive.game;assert(dg_app_key(&app,DGK_F1));assert(app.modal==DG_MODAL_RESTART);redraw(&app,&f);
 assert(dg_app_key(&app,DGK_EXIT));assert(app.modal==DG_MODAL_NONE && memcmp(&original,&app.archive.game,sizeof original)==0);
 assert(dg_app_key(&app,DGK_F1));assert(dg_app_key(&app,DGK_EXE));assert(memcmp(original.order,app.archive.game.order,3)==0 && original.seed==app.archive.game.seed);
 start(&app,2);navigation(&app);redraw(&app,&f);
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(dg_rules(&app.archive.game),app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));assert(app.selected==moves[0].from);
 app.cursor=moves[0].to;dg_app_preview(&app);assert(app.path.length>=2);redraw(&app,&f);
 app.archive.assist=0;dg_app_preview(&app);assert(app.path.length==0);redraw(&app,&f);
 DgPosition unchanged=app.archive.game.pos;app.cursor=app.selected;assert(dg_app_key(&app,DGK_EXE));
 assert(app.notice[0] && memcmp(&unchanged,&app.archive.game.pos,sizeof unchanged)==0);redraw(&app,&f);
 app.cursor=moves[0].to;assert(dg_app_key(&app,DGK_EXE));assert(dg_current(&app.archive.game)==DG_GREEN && app.archive.game.undo_valid);
 DgPosition human_move=app.archive.game.pos;assert(!dg_app_cpu(&app,cancel,NULL));assert(memcmp(&human_move,&app.archive.game.pos,sizeof human_move)==0);
 assert(dg_app_cpu(&app,NULL,NULL));assert(app.animation && app.path.length>=2);redraw(&app,&f);
 DgPosition committed=app.archive.game.pos;
 assert(dg_app_key(&app,DGK_MENU));assert(!app.animation && memcmp(&committed,&app.archive.game.pos,sizeof committed)==0 && f.menus==1);
 assert(dg_current(&app.archive.game)==DG_RED);
 assert(dg_app_key(&app,DGK_F2));assert(memcmp(&app.archive.game.pos,&unchanged,sizeof unchanged)==0 && !app.archive.game.undo_valid);
 assert(dg_app_key(&app,DGK_F2));assert(memcmp(&app.archive.game.pos,&unchanged,sizeof unchanged)==0);
 assert(dg_app_key(&app,DGK_EXIT));assert(app.screen==DG_SETUP);app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_ASSIST);
 assert(dg_app_key(&app,DGK_RIGHT));assert(app.archive.assist==1);assert(dg_app_key(&app,DGK_LEFT));assert(app.archive.assist==0);redraw(&app,&f);
 f.fail=true;assert(dg_app_key(&app,DGK_EXIT));assert(app.screen==DG_SETUP && app.notice[0]);f.fail=false;
 assert(dg_app_key(&app,DGK_EXIT));assert(app.screen==DG_PLAYER);assert(dg_app_key(&app,DGK_F6));assert(app.screen==DG_SETUP);assert(dg_app_key(&app,DGK_F4));assert(app.screen==DG_RULES);
 for(int i=0;i<30;i++)assert(dg_app_key(&app,DGK_DOWN));redraw(&app,&f);assert(dg_app_key(&app,DGK_EXIT));assert(app.screen==DG_SETUP);
 assert(dg_app_key(&app,DGK_EXIT));assert(app.screen==DG_PLAYER);assert(dg_app_key(&app,DGK_F1));assert(app.screen==DG_GAME && app.archive.game.players==2);
 assert(dg_app_key(&app,DGK_OFF));assert(f.offs==1);assert(f.saves>0);
 memset(app.archive.game.pos.board,0,sizeof app.archive.game.pos.board);
 for(int n=0;n<DG_NODES;n++){
  if(dg_in_camp(n,dg_goal[DG_RED]))app.archive.game.pos.board[n]=DG_RED;
  else if(dg_in_camp(n,dg_home[DG_GREEN]))app.archive.game.pos.board[n]=DG_GREEN;
 }
 app.archive.game.pos.board[dg_coord(0,0)]=DG_GREEN;
 app.archive.game.pos.winner=DG_RED;app.archive.active=0;app.modal=DG_MODAL_RESULT;
 unchanged=app.archive.game.pos;assert(dg_app_key(&app,DGK_F2));assert(memcmp(&unchanged,&app.archive.game.pos,sizeof unchanged)==0);
 assert(dg_app_key(&app,DGK_EXIT));assert(app.modal==DG_MODAL_NONE);assert(dg_app_key(&app,DGK_F2));assert(memcmp(&unchanged,&app.archive.game.pos,sizeof unchanged)==0);
 assert(dg_app_key(&app,DGK_EXE));assert(memcmp(&unchanged,&app.archive.game.pos,sizeof unchanged)==0);redraw(&app,&f);
 printf("UI: setup clamps, real key navigation all73, zoom all73, Assist, warnings, restart, CPU cancel/animation, undo, setup preference save failure, rules and resume passed; %u draw calls\n",f.rects);
 return 0;
}
