#include "ui.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
/* Reuse the existing engine-valid tactical builders on host only. */
#define main dg_capture_fixture_main
#include "ai_fixtures.c"
#undef main

static uint16_t pixels[224][396];
static FILE *samples;
static unsigned capture_count;
static void raster(void *context,int x,int y,int w,int h,uint16_t ink)
{
 (void)context;assert(x>=0 && y>=0 && x+w<=396 && y+h<=224 && w>0 && h>0);
 for(int row=y;row<y+h;row++)for(int col=x;col<x+w;col++)pixels[row][col]=ink;
}
static void capture(const DgApp *app,const char *directory,const char *name)
{
 DgCanvas canvas={NULL,raster};dg_render(app,&canvas);char path[512];
 int count=snprintf(path,sizeof path,"%s/%s.ppm",directory,name);assert(count>0 && (size_t)count<sizeof path);
 FILE *file=fopen(path,"wb");assert(file);fprintf(file,"P6\n396 224\n255\n");
 for(int row=0;row<224;row++)for(int col=0;col<396;col++){
  uint16_t c=pixels[row][col];unsigned r=(c>>11)&31u,g=(c>>6)&31u,b=c&31u;
  unsigned char rgb[3]={(unsigned char)(r*255/31),(unsigned char)(g*255/31),(unsigned char)(b*255/31)};
  assert(fwrite(rgb,1,3,file)==3);
 }
 capture_count++;assert(fclose(file)==0);printf("%s\n",path);
}
static void game(DgApp *app,uint8_t players)
{
 dg_app_init(app,(DgHooks){0},123456);assert(dg_new(&app->archive.game,players,DG_EASY,0,123456));
 app->archive.active=1;app->screen=DG_GAME;app->players=players;app->cursor=36;app->selected=DG_NONE;
}
static void relocate(DgGame *game,uint8_t who,int destination)
{
 assert(destination>=0 && game->pos.board[destination]==DG_EMPTY);
 for(int n=0;n<DG_NODES;n++)if(game->pos.board[n]==who && dg_in_camp(n,dg_home[who])){game->pos.board[n]=DG_EMPTY;game->pos.board[destination]=who;return;}
 assert(false && "missing fixture piece");
}
static void sample(const DgApp *app,const char *scene,const char *kind,int node)
{
 int x,y;dg_screen_position(app,node,&x,&y);
 fprintf(samples,"%s,%s,%u,%d,%d,%d\n",kind,scene,(unsigned)app->zoom,x,y,app->zoom?7:4);
}
static void readability(DgApp *app,const char *directory)
{
 game(app,3);
 int red=dg_coord(-1,-1),green=dg_coord(1,-1),yellow=dg_coord(-1,1);
 int empty=dg_coord(2,1),cyan=dg_coord(0,0),source=dg_coord(-2,0);
 relocate(&app->archive.game,DG_RED,red);relocate(&app->archive.game,DG_GREEN,green);
 relocate(&app->archive.game,DG_YELLOW,yellow);relocate(&app->archive.game,DG_RED,source);
 relocate(&app->archive.game,DG_GREEN,dg_coord(-1,0));
 assert(dg_game_valid(&app->archive.game));
 assert(dg_find_move(app->archive.game.pos.board,DG_RED,source,cyan,NULL,NULL));
 assert(!dg_find_move(app->archive.game.pos.board,DG_RED,source,empty,NULL,NULL));
 app->selected=(uint8_t)source;app->cursor=(uint8_t)dg_coord(-3,0);dg_app_preview(app);
 for(uint8_t zoom=0;zoom<2;zoom++){
  app->zoom=zoom;const char *scene=zoom?"piece-readability-zoom":"piece-readability";
  capture(app,directory,scene);
  sample(app,scene,"red",red);sample(app,scene,"green",green);sample(app,scene,"yellow",yellow);
  sample(app,scene,"empty",empty);sample(app,scene,"cyan",cyan);
 }
}
static void result_fixture(DgApp *app,uint8_t players,uint8_t winner,uint8_t level)
{
 game(app,players);DgGame *g=&app->archive.game;g->level=level;
 memset(g->pos.board,0,sizeof g->pos.board);
 for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[winner]))g->pos.board[n]=winner;
 for(uint8_t who=DG_RED;who<=DG_GREEN;who++){
  if(who==winner || (who==DG_YELLOW && players==2))continue;
  unsigned used=0;
  for(int n=0;n<DG_NODES && used<DG_PIECES;n++)if(!g->pos.board[n] && !dg_in_camp(n,dg_goal[who])){
   g->pos.board[n]=who;used++;
  }
  assert(used==DG_PIECES);
 }
 for(uint8_t slot=0;slot<players;slot++)if(g->order[slot]==winner)g->pos.turn=slot;
 g->pos.winner=winner;g->pos.turns=137;app->archive.active=0;app->modal=DG_MODAL_RESULT;
 assert(dg_game_valid(g));
}
static void goal_fixture(DgApp *app,uint8_t players,uint8_t actor,uint8_t level)
{
 game(app,players);DgGame *g=&app->archive.game;g->level=level;memset(g->pos.board,0,sizeof g->pos.board);
 const unsigned counts[4]={0,5,3,4};
 for(uint8_t who=DG_RED;who<=DG_GREEN;who++)if(who!=DG_YELLOW || players==3){
  unsigned used=0;for(int n=0;n<DG_NODES && used<counts[who];n++)if(dg_in_camp(n,dg_goal[who])){assert(!g->pos.board[n]);g->pos.board[n]=who;used++;}
  for(int n=0;n<DG_NODES && used<10;n++)if(!g->pos.board[n] && !dg_in_camp(n,dg_goal[DG_RED]) && !dg_in_camp(n,dg_goal[DG_YELLOW]) && !dg_in_camp(n,dg_goal[DG_GREEN])){g->pos.board[n]=who;used++;}
  assert(used==10);
 }
 for(uint8_t slot=0;slot<players;slot++)if(g->order[slot]==actor)g->pos.turn=slot;
 assert(dg_game_valid(g));
}
static void entry_captures(DgApp *app,const char *directory)
{
 for(uint8_t players=2;players<=3;players++){
  dg_app_init(app,(DgHooks){0},123456);app->players=players;
  assert(dg_app_key(app,DGK_F6));
  char name[48];snprintf(name,sizeof name,"setup-%up-no-resume",(unsigned)players);capture(app,directory,name);
  for(uint8_t slot=0;slot<players;slot++){
   app->slot=slot;app->focus=(uint8_t)dg_entry_row(app,DG_ENTRY_SLOT);
   snprintf(name,sizeof name,players==2?"setup-2p-first-%s":"setup-3p-human-%s",players==2?(slot?"ai":"human"):slot==0?"1st":slot==1?"2nd":"3rd");
   capture(app,directory,name);
  }
  app->slot=0;app->focus=(uint8_t)dg_entry_row(app,DG_ENTRY_ASSIST);
  for(uint8_t assist=0;assist<2;assist++){
   app->archive.assist=assist;snprintf(name,sizeof name,"setup-%up-assist-%s",(unsigned)players,assist?"on":"off");capture(app,directory,name);
  }
  game(app,players);app->archive.game.level=DG_NORMAL;app->screen=DG_PLAYER;
  snprintf(name,sizeof name,"player-resume-%up",(unsigned)players);capture(app,directory,name);
  assert(dg_app_key(app,DGK_F6));snprintf(name,sizeof name,"setup-%up-resume",(unsigned)players);capture(app,directory,name);
  /* Opposite tile: global shortcut remains, local RESUME is absent. */
  app->screen=DG_PLAYER;app->players=players==2?3:2;
  snprintf(name,sizeof name,"player-resume-saved-%up-tile-%up",(unsigned)players,(unsigned)app->players);capture(app,directory,name);
  assert(dg_app_key(app,DGK_F6));snprintf(name,sizeof name,"setup-%up-mismatch",(unsigned)app->players);capture(app,directory,name);
 }
 goal_fixture(app,2,DG_RED,DG_EASY);capture(app,directory,"hud-human-2p-easy");
 goal_fixture(app,3,DG_YELLOW,DG_NORMAL);capture(app,directory,"hud-yellow-3p-normal");
 goal_fixture(app,3,DG_GREEN,DG_HARD);capture(app,directory,"hud-green-3p-hard");
 app->thinking=1;
 for(uint8_t zoom=0;zoom<2;zoom++)for(uint8_t phase=0;phase<3;phase++){
  char name[48];app->zoom=zoom;app->thinking_phase=phase;
  snprintf(name,sizeof name,zoom?"thinking-zoom-%u":"thinking-%u",(unsigned)phase+1);capture(app,directory,name);
 }
 app->thinking=0;app->zoom=1;capture(app,directory,"zoom-unselected");
 game(app,3);DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app->archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app->selected=moves[0].from;app->cursor=moves[0].to;app->zoom=1;dg_app_preview(app);capture(app,directory,"selected-zoom");
 app->zoom=0;capture(app,directory,"selected-overview");
 snprintf(app->notice,sizeof app->notice,"DESTINATION OCCUPIED");capture(app,directory,"warning-long");
 app->zoom=1;capture(app,directory,"warning-zoom");
}
static void long_ai(DgApp *app,uint8_t actor)
{
 game(app,3);app->archive.game=multijump(3);DgGame *g=&app->archive.game;
 g->level=DG_NORMAL;
 if(actor==DG_YELLOW){
  uint8_t reflected[DG_NODES]={0};
  for(int n=0;n<DG_NODES;n++){
   int target=dg_coord(-(int)dg_nodes[n].q-(int)dg_nodes[n].r,dg_nodes[n].r);assert(target>=0);
   uint8_t who=g->pos.board[n];reflected[target]=who==DG_GREEN?DG_YELLOW:who==DG_YELLOW?DG_GREEN:who;
  }
  memcpy(g->pos.board,reflected,sizeof reflected);
 }
 g->human_slot=2;g->order[0]=actor;g->order[1]=actor==DG_GREEN?DG_YELLOW:DG_GREEN;g->order[2]=DG_RED;g->pos.turn=0;
 assert(dg_game_valid(g));app->cursor=(uint8_t)dg_coord(0,0);
}
static void visual_captures(DgApp *app,const char *directory)
{
 char filename[512];snprintf(filename,sizeof filename,"%s/animation-frames.csv",directory);
 FILE *metadata=fopen(filename,"w");assert(metadata);
 fprintf(metadata,"scene,rtc_ticks,actor,from,to,hops,segment,phase,committed_turns,logical_actor,animation,path\n");
 long_ai(app,DG_GREEN);DgGame before=app->archive.game;
 assert(dg_app_cpu(app,NULL,NULL));DgPath chosen=app->path;assert(chosen.length>=4);
 unsigned total=(unsigned)(chosen.length-1u)*DG_HOP_TICKS;
 for(unsigned tick=0;tick<=total;tick+=DG_FRAME_TICKS){
  if(tick)assert(dg_app_animation_tick(app,tick));
  assert(app->archive.game.pos.turns==before.pos.turns+1 && !app->thinking);
  char scene[48];snprintf(scene,sizeof scene,"animation-%03u",tick);capture(app,directory,scene);
  fprintf(metadata,"%s,%u,%u,%u,%u,%u,%u,%u,%lu,%u,%u,",scene,tick,(unsigned)app->animation_actor,
   (unsigned)app->pending_move.from,(unsigned)app->pending_move.to,(unsigned)app->pending_move.hops,
   (unsigned)app->anim_index,(unsigned)app->anim_phase,(unsigned long)app->archive.game.pos.turns,
   (unsigned)dg_current(&app->archive.game),(unsigned)app->animation);
  for(unsigned n=0;n<chosen.length;n++)fprintf(metadata,n?":%u":"%u",(unsigned)chosen.node[n]);fprintf(metadata,"\n");
 }
 assert(fclose(metadata)==0);capture(app,directory,"green-long-trail");
 capture(app,directory,"animation-final-trail");
 app->thinking=1;app->thinking_phase=2;capture(app,directory,"trails-thinking");app->thinking=0;
 assert(dg_app_cpu(app,NULL,NULL));while(app->animation)assert(dg_app_animation(app));
 assert(dg_current(&app->archive.game)==DG_RED && app->trails[0].valid && app->trails[1].valid);
 capture(app,directory,"trails-you");capture(app,directory,"trails-both");
 app->zoom=1;capture(app,directory,"trails-zoom");app->zoom=0;
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app->archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app->selected=moves[0].from;app->cursor=moves[0].to;dg_app_preview(app);
 capture(app,directory,"trails-assist");app->cursor=app->selected;dg_app_preview(app);capture(app,directory,"trails-selected");
 app->zoom=1;capture(app,directory,"trails-assist-zoom");
 long_ai(app,DG_YELLOW);assert(dg_app_cpu(app,NULL,NULL));assert(app->path.length>=4);
 while(app->animation)assert(dg_app_animation(app));capture(app,directory,"yellow-long-trail");
 app->zoom=1;capture(app,directory,"yellow-long-trail-zoom");
 /* Geometry fixtures: the actual trail primitives, not simulated AI choices. */
 game(app,3);app->archive.game.level=DG_NORMAL;app->cursor=(uint8_t)dg_coord(-4,0);
 int center=dg_coord(0,0);
 for(uint8_t zoom=0;zoom<2;zoom++)for(int direction=0;direction<6;direction++)for(unsigned opposite=0;opposite<2;opposite++){
  int to=dg_nodes[center].neighbor[direction];assert(to>=0);app->zoom=zoom;
  app->trails[0]=(DgAiTrail){.valid=1,.player=DG_YELLOW,.path={.length=2,.node={(uint8_t)center,(uint8_t)to}}};
  app->trails[1]=(DgAiTrail){.valid=1,.player=DG_GREEN,.path={.length=2,.node={(uint8_t)(opposite?to:center),(uint8_t)(opposite?center:to)}}};
  char scene[48];snprintf(scene,sizeof scene,"shared-%s-d%d-%s",opposite?"opposite":"same",direction,zoom?"zoom":"overview");capture(app,directory,scene);
 }
 game(app,2);app->archive.game.pos.turn=1;assert(dg_app_cpu(app,NULL,NULL));while(app->animation)assert(dg_app_animation(app));
 capture(app,directory,"trails-2p");
}
int main(int argc,char **argv)
{
 const char *directory=argc>1?argv[1]:"docs/screenshots";
 if(mkdir(directory,0755)!=0 && errno!=EEXIST){perror(directory);return EXIT_FAILURE;}
 char sample_path[512];int sample_count=snprintf(sample_path,sizeof sample_path,"%s/piece-samples.csv",directory);
 assert(sample_count>0 && (size_t)sample_count<sizeof sample_path);
 samples=fopen(sample_path,"w");assert(samples);fprintf(samples,"kind,scene,zoom,x,y,radius\n");
 DgApp app;dg_app_init(&app,(DgHooks){0},123456);capture(&app,directory,"player");
 app.players=2;capture(&app,directory,"player-2p");app.players=3;
 app.screen=DG_SETUP;capture(&app,directory,"setup-3p");
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 static const char *const level_names[3]={"easy","normal","hard"};
 for(uint8_t players=2;players<=3;players++)for(unsigned i=0;i<3;i++){
  char name[32];snprintf(name,sizeof name,"setup-%up-%s",(unsigned)players,level_names[i]);
  app.players=players;app.level=levels[i];app.slot=players==2?1:0;app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_LEVEL);capture(&app,directory,name);
 }
 app.players=2;app.slot=1;app.level=DG_HARD;capture(&app,directory,"setup-2p");
 app.screen=DG_RULES;capture(&app,directory,"rules");
 game(&app,2);capture(&app,directory,"2p-overview");game(&app,3);capture(&app,directory,"3p-overview");
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app.selected=moves[0].from;app.cursor=app.selected;capture(&app,directory,"piece-selection");
 app.cursor=moves[0].to;dg_app_preview(&app);capture(&app,directory,"assist-on");
 app.archive.assist=0;dg_app_preview(&app);capture(&app,directory,"assist-off");
 snprintf(app.notice,sizeof app.notice,"INVALID MOVE");capture(&app,directory,"warning");
 game(&app,2);relocate(&app.archive.game,DG_RED,dg_coord(0,0));relocate(&app.archive.game,DG_GREEN,dg_coord(1,0));relocate(&app.archive.game,DG_GREEN,dg_coord(2,-1));
 assert(dg_game_valid(&app.archive.game));app.selected=(uint8_t)dg_coord(0,0);app.cursor=(uint8_t)dg_coord(2,-2);
 dg_app_preview(&app);assert(app.path.length>=3);capture(&app,directory,"multi-jump");
 app.zoom=1;capture(&app,directory,"zoom");app.archive.assist=0;dg_app_preview(&app);capture(&app,directory,"zoom-assist-off");
 app.archive.assist=1;app.zoom=0;app.selected=DG_NONE;app.path.length=0;app.thinking=1;app.archive.game.pos.turn=1;capture(&app,directory,"cpu-thinking");capture(&app,directory,"ai-thinking");
 app.thinking=0;assert(dg_app_cpu(&app,NULL,NULL));assert(dg_app_animation_tick(&app,6));capture(&app,directory,"cpu-animation");capture(&app,directory,"ai-animation");
 app.animation=0;app.path.length=0;app.modal=DG_MODAL_RESTART;capture(&app,directory,"restart");
 game(&app,2);memset(app.archive.game.pos.board,0,sizeof app.archive.game.pos.board);
 for(int n=0;n<DG_NODES;n++){if(dg_in_camp(n,dg_goal[DG_RED]))app.archive.game.pos.board[n]=DG_RED;
  else if(dg_in_camp(n,dg_home[DG_GREEN]))app.archive.game.pos.board[n]=DG_GREEN;}
 app.archive.game.pos.board[dg_coord(0,0)]=DG_GREEN;
 app.archive.game.pos.winner=DG_RED;app.archive.game.pos.turns=137;app.modal=DG_MODAL_RESULT;app.archive.active=0;capture(&app,directory,"result");
 app.modal=DG_MODAL_NONE;capture(&app,directory,"result-board");
 result_fixture(&app,2,DG_GREEN,DG_NORMAL);capture(&app,directory,"result-normal");
 result_fixture(&app,3,DG_GREEN,DG_HARD);capture(&app,directory,"result-green-ai");
 result_fixture(&app,3,DG_YELLOW,DG_EASY);capture(&app,directory,"result-yellow-ai");
 /* Additional acceptance views exercise labels absent from the opening set. */
 game(&app,3);app.screen=DG_PLAYER;capture(&app,directory,"player-resume");
 game(&app,2);count=dg_generate(app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));
 app.cursor=moves[0].to;assert(dg_app_key(&app,DGK_EXE));
 assert(dg_app_cpu(&app,NULL,NULL));while(app.animation)assert(dg_app_animation(&app));
 assert(dg_current(&app.archive.game)==DG_RED && app.archive.game.undo_valid);
 capture(&app,directory,"undo-ready");
 count=dg_generate(app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
 app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));capture(&app,directory,"move-with-undo");
 assert(dg_app_key(&app,DGK_F4));app.rules_scroll=12;capture(&app,directory,"rules-controls");
 entry_captures(&app,directory);readability(&app,directory);visual_captures(&app,directory);
 assert(fclose(samples)==0);printf("%u actual-renderer frames\n",capture_count);
 return EXIT_SUCCESS;
}
