#include "ui.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static uint16_t pixels[224][396];
static FILE *samples;
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
 assert(fclose(file)==0);printf("%s\n",path);
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
  app.players=players;app.level=levels[i];app.slot=players==2?1:0;capture(&app,directory,name);
 }
 app.players=2;app.slot=1;app.level=DG_HARD;capture(&app,directory,"setup-2p");
 app.screen=DG_SETTINGS;capture(&app,directory,"settings");app.screen=DG_RULES;capture(&app,directory,"rules");
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
 app.thinking=0;assert(dg_app_cpu(&app,NULL,NULL));app.anim_index=1;capture(&app,directory,"cpu-animation");capture(&app,directory,"ai-animation");
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
 readability(&app,directory);
 assert(fclose(samples)==0);
 return EXIT_SUCCESS;
}
