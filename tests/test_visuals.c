/* Actual controller, time-derived renderer and all directed topology edges. */
#include "../src/ui/draw.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define main dg_visual_fixture_main
#include "../tools/ai_fixtures.c"
#undef main

static uint16_t pixels[224][396],plain[224][396];
static unsigned segments,arrow_pixels,frames,dual_edges;
static bool arrows_only,line_only;
static const DgApp *geometry_app;
static int geometry_from,geometry_to,geometry_lane;
static void raster(void *context,int x,int y,int w,int h,uint16_t ink)
{
 (void)context;assert(x>=0 && y>=0 && x+w<=396 && y+h<=224 && w>0 && h>0);
 for(int row=y;row<y+h;row++)for(int col=x;col<x+w;col++){
  if(line_only){
   int radius=geometry_app->zoom?8:5;
   const int nodes[2]={geometry_from,geometry_to};
   for(unsigned i=0;i<2;i++){
    int nx,ny;dg_screen_position(geometry_app,nodes[i],&nx,&ny);
    assert((col-nx)*(col-nx)+(row-ny)*(row-ny)>radius*radius);
   }
  }
  if(arrows_only){
   int radius=geometry_app->zoom?8:5;
   for(int n=0;n<DG_NODES;n++){
    int nx,ny;dg_screen_position(geometry_app,n,&nx,&ny);
    if((col-nx)*(col-nx)+(row-ny)*(row-ny)<=radius*radius)
     fprintf(stderr,"Head collision zoom=%u %d->%d lane=%d pixel=%d,%d hole=%d (%d,%d)\n",geometry_app->zoom,geometry_from,geometry_to,geometry_lane,col,row,n,nx,ny);
    assert((col-nx)*(col-nx)+(row-ny)*(row-ny)>radius*radius);
   }
   arrow_pixels++;
  }
  pixels[row][col]=ink;
 }
}
static void render(const DgApp *app)
{
 DgApp frozen=*app;dg_render(app,&(DgCanvas){NULL,raster});
 assert(!memcmp(&frozen,app,sizeof frozen));frames++;
}
static void begin(DgApp *app,uint8_t players,uint8_t slot,uint8_t level)
{
 dg_app_init(app,(DgHooks){0},917u);app->players=players;app->slot=slot;app->level=level;
 assert(dg_app_key(app,DGK_F6));assert(dg_app_key(app,DGK_F6));
 assert(app->screen==DG_GAME && dg_game_valid(&app->archive.game));
}
static void finish_cpu(DgApp *app)
{
 DgGame expected=app->archive.game;uint8_t actor=dg_current(&expected);
 DgMove move;DgPath path={0};uint32_t rng;DgAiStats stats;
 assert(dg_ai_choose(&expected,0,NULL,NULL,&move,&rng,&stats));
 assert(dg_find_move(dg_rules(&expected),expected.pos.board,actor,move.from,move.to,NULL,&path));
 uint8_t board[DG_NODES];memcpy(board,expected.pos.board,sizeof board);
 assert(dg_commit(&expected,&move));expected.pos.rng=rng;
 assert(dg_app_cpu(app,NULL,NULL));assert(!app->thinking && app->animation);
 assert(!memcmp(&expected,&app->archive.game,sizeof expected));
 assert(!memcmp(board,app->animation_board,sizeof board));
 assert(!memcmp(&path,&app->path,sizeof path));
 assert(!memcmp(&move,&app->pending_move,sizeof move));assert(rng==app->pending_rng);
 assert(stats.nodes==app->ai_stats.nodes && stats.depth==app->ai_stats.depth && stats.beam==app->ai_stats.beam);
 assert(actor==app->animation_actor && !app->modal);
 unsigned total=(unsigned)(path.length-1u)*DG_HOP_TICKS,updates=0;
 for(unsigned tick=0;tick<=total;tick++){
  bool changed=dg_app_animation_tick(app,tick);
  assert(changed==(tick>0 && tick%DG_FRAME_TICKS==0));
  if(changed)updates++;
  assert(!memcmp(&expected,&app->archive.game,sizeof expected));
  if(tick<total){
   assert(app->animation && !app->thinking);
   assert(app->anim_index==tick/DG_HOP_TICKS && app->anim_phase==(tick%DG_HOP_TICKS)/DG_FRAME_TICKS);
   DgApp frozen=*app;assert(!dg_app_key(app,DGK_EXE));assert(!dg_app_key(app,DGK_F2));assert(!dg_app_key(app,DGK_F4));assert(!dg_app_key(app,DGK_F5));
   assert(!memcmp(&frozen,app,sizeof frozen));
   int x,y,ax,ay,bx,by;ui_animation_position(app,&x,&y);
   dg_screen_position(app,path.node[app->anim_index],&ax,&ay);
   dg_screen_position(app,path.node[app->anim_index+1u],&bx,&by);
   assert(x>=(ax<bx?ax:bx) && x<=(ax>bx?ax:bx));
   assert(y>=(ay<by?ay:by) && y<=(ay>by?ay:by));
   if(app->anim_phase)assert(x!=ax || y!=ay);
   if(changed)render(app);
  }
 }
 assert(updates==(unsigned)(path.length-1u)*DG_HOP_FRAMES && !app->animation);
 assert(app->trails[1].valid && app->trails[1].player==actor);
 assert(!memcmp(&app->trails[1].path,&path,sizeof path));
}
static bool stop(void *unused){(void)unused;return true;}
static void trails_and_lifetime(void)
{
 for(uint8_t players=2;players<=3;players++){
  DgApp app;begin(&app,players,1,DG_NORMAL);
  while(dg_current(&app.archive.game)!=DG_RED)finish_cpu(&app);
  /* Consume one legal YOU move, then the entire AI chain. */
  DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(dg_rules(&app.archive.game),app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
  app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));app.cursor=moves[0].to;assert(dg_app_key(&app,DGK_EXE));
  assert(!app.trails[0].valid && !app.trails[1].valid);
  finish_cpu(&app);DgAiTrail first=app.trails[1];
  if(players==3){
   DgGame frozen=app.archive.game;assert(!dg_app_cpu(&app,stop,NULL));
   assert(!memcmp(&first,&app.trails[1],sizeof first) && !memcmp(&frozen,&app.archive.game,sizeof frozen));
   app.thinking=1;assert(dg_app_thinking_tick(&app,80));app.thinking=0;
   assert(!memcmp(&first,&app.trails[1],sizeof first));finish_cpu(&app);
   assert(!memcmp(&first,&app.trails[0],sizeof first) && app.trails[0].player!=app.trails[1].player);
  }else assert(!app.trails[0].valid && app.trails[1].player==DG_GREEN);
  assert(dg_current(&app.archive.game)==DG_RED);
  DgAiTrail kept[2];memcpy(kept,app.trails,sizeof kept);
  count=dg_generate(dg_rules(&app.archive.game),app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);assert(count);
  assert(dg_app_key(&app,DGK_LEFT));assert(dg_app_key(&app,DGK_F5));
  app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));
  assert(dg_app_key(&app,DGK_EXE));assert(app.notice[0]); /* invalid source = destination */
  app.archive.assist=0;dg_app_preview(&app);app.archive.assist=1;dg_app_preview(&app);
  assert(!memcmp(kept,app.trails,sizeof kept));
  assert(dg_app_key(&app,DGK_EXIT));assert(!memcmp(kept,app.trails,sizeof kept));
  assert(dg_app_key(&app,DGK_EXIT));assert(!memcmp(kept,app.trails,sizeof kept));
  assert(dg_app_key(&app,DGK_F1));assert(dg_app_key(&app,DGK_EXIT));assert(!memcmp(kept,app.trails,sizeof kept));
  DgApp copy=app;assert(dg_app_key(&copy,DGK_F2));assert(!copy.trails[0].valid && !copy.trails[1].valid);
  copy=app;assert(dg_app_key(&copy,DGK_F1));assert(dg_app_key(&copy,DGK_EXE));assert(!copy.trails[0].valid && !copy.trails[1].valid);
  copy=app;assert(dg_app_to_setup(&copy));assert(dg_app_key(&copy,DGK_EXE));assert(!copy.trails[0].valid && !copy.trails[1].valid);
  copy=app;copy.screen=DG_PLAYER;assert(dg_app_key(&copy,DGK_F1));assert(!copy.trails[0].valid && !copy.trails[1].valid);
  copy=app;assert(dg_app_to_setup(&copy));copy.focus=(uint8_t)dg_entry_row(&copy,DG_ENTRY_NEW);
  assert(dg_app_key(&copy,DGK_EXE));assert(!copy.trails[0].valid && !copy.trails[1].valid);
  DgApp cold;dg_app_init(&cold,(DgHooks){0},1);cold.archive=app.archive;assert(dg_app_key(&cold,DGK_F1));
  assert(!cold.trails[0].valid && !cold.trails[1].valid);
  app.cursor=moves[0].from;assert(dg_app_key(&app,DGK_EXE));app.cursor=moves[0].to;assert(dg_app_key(&app,DGK_EXE));
  assert(!app.trails[0].valid && !app.trails[1].valid);
 }
}
static void topology_bound(void)
{
 unsigned largest=0;
 for(int source=0;source<DG_NODES;source++){
  bool seen[DG_NODES]={false};uint8_t queue[DG_NODES];unsigned head=0,tail=1;queue[0]=(uint8_t)source;seen[source]=true;
  while(head<tail){int n=queue[head++];for(int d=0;d<6;d++){
   int next=dg_nodes[n].jump[d];if(next<0 || seen[next])continue;
   seen[next]=true;assert(tail<DG_NODES);queue[tail++]=(uint8_t)next;
  }}
  if(tail>largest)largest=tail;
 }
 assert(largest<=DG_NODES && sizeof(((DgPath *)0)->node)>=largest);
 printf("All 73 jump components: largest %u landing nodes; representative BFS <=%u hops; capacity %zu nodes, no truncation\n",largest,largest-1u,sizeof(((DgPath *)0)->node));
}
static void geometry(void)
{
 DgApp app;begin(&app,3,0,DG_NORMAL);app.selected=DG_NONE;
 DgCanvas canvas={NULL,raster};DgPainter painter={&canvas,4,UI_BOARD_TOP,392,UI_BOARD_BOTTOM};
 for(uint8_t zoom=0;zoom<2;zoom++)for(int from=0;from<DG_NODES;from++)for(int kind=0;kind<2;kind++)for(int direction=0;direction<6;direction++){
  int to=kind?dg_nodes[from].jump[direction]:dg_nodes[from].neighbor[direction];if(to<0)continue;
  app.zoom=zoom;app.cursor=(uint8_t)from;geometry_app=&app;
  int x,y,tx,ty;dg_screen_position(&app,from,&x,&y);dg_screen_position(&app,to,&tx,&ty);
  int dx=tx-x,dy=ty-y;
  for(int lane=-1;lane<=1;lane++){
   geometry_from=from;geometry_to=to;geometry_lane=lane;
   UiTrailSegment s;assert(ui_trail_segment(&app,from,to,lane,&s));segments++;
   assert((s.tx-s.x)*dx+(s.ty-s.y)*dy>0);
   assert((2*s.ax-s.bx-s.cx)*dx+(2*s.ay-s.by-s.cy)*dy>0);
   int radius=zoom?8:5;
   assert((s.x-x)*(s.x-x)+(s.y-y)*(s.y-y)>radius*radius);
   assert((s.tx-tx)*(s.tx-tx)+(s.ty-ty)*(s.ty-ty)>radius*radius);
   /* Actual Bresenham head pixels, checked against every hole, not just ends. */
   arrows_only=true;ui_trail_arrow(&painter,&s,TRAIL_YELLOW);arrows_only=false;
   line_only=true;ui_trail_line(&painter,&s,zoom?3:2,TRAIL_YELLOW);line_only=false;
   UiTrailSegment reverse;assert(ui_trail_segment(&app,to,from,lane,&reverse));
   assert(s.x==reverse.tx && s.y==reverse.ty && s.tx==reverse.x && s.ty==reverse.y);
  }
 }
 /* Both visible lanes, six directions, STEP/JUMP, same and opposite movement. */
 int center=dg_coord(0,0);app.cursor=(uint8_t)dg_coord(-4,0);
 for(uint8_t zoom=0;zoom<2;zoom++)for(int direction=0;direction<6;direction++)for(unsigned opposite=0;opposite<2;opposite++)for(unsigned jump=0;jump<2;jump++){
  app.zoom=zoom;int to=jump?dg_nodes[center].jump[direction]:dg_nodes[center].neighbor[direction];assert(to>=0);
  app.trails[0]=(DgAiTrail){.valid=1,.player=DG_YELLOW,.path={.length=2,.node={(uint8_t)center,(uint8_t)to}}};
  app.trails[1]=(DgAiTrail){.valid=1,.player=DG_GREEN,.path={.length=2,.node={(uint8_t)(opposite?to:center),(uint8_t)(opposite?center:to)}}};
  /* Independent lane paint cannot erase any pixel from the first lane. */
  UiTrailSegment first,second;
  assert(ui_trail_segment(&app,center,to,-1,&first));
  assert(ui_trail_segment(&app,opposite?to:center,opposite?center:to,1,&second));
  memset(pixels,0,sizeof pixels);ui_trail_line(&painter,&first,zoom?3:2,TRAIL_YELLOW);ui_trail_arrow(&painter,&first,TRAIL_YELLOW);
  memcpy(plain,pixels,sizeof plain);
  ui_trail_line(&painter,&second,zoom?3:2,TRAIL_GREEN);ui_trail_arrow(&painter,&second,TRAIL_GREEN);
  for(int row=0;row<224;row++)for(int col=0;col<396;col++)
   if(plain[row][col]==TRAIL_YELLOW){
    if(pixels[row][col]!=TRAIL_YELLOW)fprintf(stderr,"Lane overlap z=%u d=%d opp=%u jump=%u at %d,%d\n",zoom,direction,opposite,jump,col,row);
    assert(pixels[row][col]==TRAIL_YELLOW);
   }
  memcpy(plain,pixels,sizeof plain);memset(pixels,0,sizeof pixels);ui_trails(&painter,&app);
  assert(!memcmp(plain,pixels,sizeof plain));
  unsigned yellow=0,green=0;
  for(int row=0;row<224;row++)for(int col=0;col<396;col++){
   if(pixels[row][col])assert(row>=26 && row<204 && col>=4 && col<392);
   yellow+=pixels[row][col]==TRAIL_YELLOW;green+=pixels[row][col]==TRAIL_GREEN;
  }
  assert(yellow>0 && green>0);dual_edges++;
 }
 /* Full renderer layering: trails cannot change hole interiors, HUD or cursor. */
 for(uint8_t zoom=0;zoom<2;zoom++){
  app.zoom=zoom;app.cursor=(uint8_t)center;app.selected=DG_NONE;app.trails[0].valid=app.trails[1].valid=0;
  render(&app);memcpy(plain,pixels,sizeof plain);
  app.trails[0].valid=app.trails[1].valid=1;render(&app);int radius=zoom?8:5;
  for(int n=0;n<DG_NODES;n++){
   int x,y;dg_screen_position(&app,n,&x,&y);
   for(int oy=-radius;oy<=radius;oy++)for(int ox=-radius;ox<=radius;ox++)if(ox*ox+oy*oy<=radius*radius){
    int px=x+ox,py=y+oy;if(px>=4 && px<392 && py>=26 && py<204)assert(pixels[py][px]==plain[py][px]);
   }
  }
  for(int row=0;row<224;row++)for(int col=0;col<396;col++){
   if(row<26 || row>=204 || plain[row][col]==BOARD_BLUE)assert(pixels[row][col]==plain[row][col]);
  }
 }
 /* The cyan preview cannot erase a single retained AI edge. Hole/selection
    layers still take priority; this is an explicit renderer geometry fixture. */
 app.zoom=0;app.cursor=(uint8_t)dg_coord(-4,0);app.selected=(uint8_t)center;
 app.trails[0].valid=0;
 app.trails[1]=(DgAiTrail){.valid=1,.player=DG_GREEN,
  .path={.length=2,.node={(uint8_t)center,(uint8_t)dg_nodes[center].neighbor[0]}}};
 app.path=app.trails[1].path;render(&app);
 unsigned visible=0;int x,y;dg_screen_position(&app,center,&x,&y);
 for(int row=y-1;row<=y+1;row++)for(int col=x+9;col<=x+10;col++)visible+=pixels[row][col]==TRAIL_GREEN;
 assert(visible>0);
 /* Exact 2/3-pixel axial width and direction-independent stroke raster. */
 for(int width=2;width<=3;width++){
  UiTrailSegment s={.x=100,.y=100,.tx=120,.ty=100};
  memset(pixels,0,sizeof pixels);ui_trail_line(&painter,&s,width,TRAIL_GREEN);
  for(int col=100;col<=120;col++){
   unsigned count=0;for(int row=95;row<=105;row++)count+=pixels[row][col]==TRAIL_GREEN;
   assert(count==(unsigned)width);
  }
  memcpy(plain,pixels,sizeof plain);memset(pixels,0,sizeof pixels);
  s.x=120;s.tx=100;ui_trail_line(&painter,&s,width,TRAIL_GREEN);assert(!memcmp(plain,pixels,sizeof plain));
 }
}
int main(void)
{
 _Static_assert(sizeof(DgAiTrail)==77,"trail RAM bound");
 _Static_assert(sizeof(((DgApp *)0)->animation_board)==73,"pre-board RAM bound");
 assert(UI_YELLOW_TEXT==UI_GOLD && UI_YELLOW_TEXT==ui_level_color(DG_NORMAL) && UI_YELLOW_TEXT==ui_actor_color(DG_YELLOW));
 assert(TRAIL_YELLOW==0xf5c0 && TRAIL_GREEN==0x5644 && TRAIL_GREEN!=BOARD_CYAN);
 assert(PIECE_YELLOW==DG_RGB(31,24,0) && UI_YELLOW_TEXT!=PIECE_YELLOW);
 assert(DG_HOP_TICKS*1000/128>=100 && DG_HOP_TICKS*1000/128<=150);
 topology_bound();trails_and_lifetime();geometry();
 DgApp long_move;begin(&long_move,3,1,DG_NORMAL);long_move.archive.game=multijump(3);long_move.archive.game.level=DG_NORMAL;
 finish_cpu(&long_move);assert(long_move.trails[1].path.length==6); /* five turning hops */
 DgApp winning;begin(&winning,3,1,DG_NORMAL);winning.archive.game=nearly_won(3);winning.archive.game.level=DG_NORMAL;
 assert(dg_app_cpu(&winning,NULL,NULL) && winning.animation && winning.archive.game.pos.winner==DG_GREEN && !winning.modal);
 render(&winning);
 for(int row=204;row<224;row++)for(int col=0;col<396;col++)assert(pixels[row][col]==UI_WHITE);
 dg_app_skip_animation(&winning);assert(winning.modal==DG_MODAL_RESULT);render(&winning);
 printf("Visuals PASS: %u normalized segments, %u safe arrow pixels, %u dual-lane cases, %u immutable interpolated frames; atomic AI/RNG, trail lifetime and layering\n",segments,arrow_pixels,dual_edges,frames);
 return 0;
}
