/* Actual common renderer and RGB565 raster cost; host CPU time only. */
#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static uint16_t pixels[224][396];
static unsigned calls;
static void raster(void *context,int x,int y,int w,int h,uint16_t ink)
{
 (void)context;calls++;
 for(int row=y;row<y+h;row++)for(int col=x;col<x+w;col++)pixels[row][col]=ink;
}
static void measure(DgApp *app,const char *name)
{
 const unsigned batches=12,frames=200;double total=0,minimum=1e9;
 DgCanvas canvas={NULL,raster};dg_render(app,&canvas);calls=0;
 for(unsigned b=0;b<batches;b++){
  clock_t start=clock();
  for(unsigned i=0;i<frames;i++)dg_render(app,&canvas);
  double us=(double)(clock()-start)*1000000.0/((double)CLOCKS_PER_SEC*frames);
  total+=us;if(us<minimum)minimum=us;
 }
 printf("%s,%u,%.3f,%.3f,%u,%04x\n",name,batches*frames,total/batches,minimum,
  calls/(batches*frames),(unsigned)pixels[100][198]);
}
int main(void)
{
 DgApp app;dg_app_init(&app,(DgHooks){0},123456);
 assert(dg_new(&app.archive.game,3,DG_EASY,0,123456));
 app.screen=DG_GAME;app.archive.active=1;app.cursor=36;
 puts("scene,frames,mean_host_us,min_batch_host_us,rect_calls_per_frame,sample_rgb565");
 measure(&app,"overview");app.zoom=1;measure(&app,"zoom");app.zoom=0;
 /* Search a deterministic reachable sequence for a dense Assist fixture. */
 DgGame opening=app.archive.game,walk=opening;DgMove moves[DG_NODES];size_t most=0;
 for(unsigned ply=0;ply<90 && !walk.pos.winner;ply++){
  if(dg_current(&walk)==DG_RED)for(uint8_t n=0;n<DG_NODES;n++)if(walk.pos.board[n]==DG_RED){
   size_t count=dg_piece_moves(dg_rules(&walk),walk.pos.board,DG_RED,n,moves,DG_NODES);
   if(count>most){most=count;app.archive.game=walk;app.selected=n;}
  }
  DgMove move;uint32_t rng;DgAiStats stats;
  assert(dg_ai_choose(&walk,0,NULL,NULL,&move,&rng,&stats));
  assert(dg_commit(&walk,&move));walk.pos.rng=rng;
 }
 assert(most>=10);app.cursor=app.selected;dg_app_preview(&app);
 fprintf(stderr,"Assist benchmark: %lu destinations at reachable turn %lu\n",(unsigned long)most,(unsigned long)app.archive.game.pos.turns);
 measure(&app,"assist_destinations");app.selected=DG_NONE;app.path.length=0;
 app.archive.game=opening;
 app.archive.game.pos.turn=1;assert(dg_app_cpu(&app,NULL,NULL));
 assert(dg_app_animation_tick(&app,6));measure(&app,"cpu_animation");
 return 0;
}
