/* Host-only fixture audit. host_seconds is measured host CPU time, never
   calculator latency. Opening and midgame are reachable from NEW; the other
   four fixtures are deliberately constructed engine-valid tactical positions,
   with exactly ten pieces per participant, rather than claims of reachability.
   Run: build/host/ai_fixtures > docs/validation/ai-fixtures.csv */
#include "ai.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static unsigned checks;
#define CHECK(expression) do { checks++;if(!(expression)){ \
 fprintf(stderr,"ai fixtures: %s failed at %s:%d\n",#expression,__FILE__,__LINE__); \
 exit(1); } } while(0)
enum { OPENING,MIDGAME,CONGESTION,MULTIJUMP,NEARLY_WON,BLOCKING,FIXTURES };
static const char *const names[FIXTURES]={
 "opening","reachable_midgame","goal_congestion","long_multi_jump","nearly_won","blocking_heavy"
};

static unsigned pieces(const DgGame *game,uint8_t player)
{
 unsigned count=0;
 for(unsigned n=0;n<DG_NODES;n++)if(game->pos.board[n]==player)count++;
 return count;
}
/* Deterministic coordinate-distance placement fills remaining pieces while
   excluding that player's goal, reserved landing holes and occupied holes. */
static void fill_to(DgGame *game,uint8_t player,unsigned total,const bool *reserved,int anchor,bool distant)
{
 unsigned count=pieces(game,player);
 while(count<total){
  int best=-1,score=distant?INT_MIN:INT_MAX;
  for(int n=0;n<DG_NODES;n++){
   if(game->pos.board[n] || (reserved && reserved[n]) || dg_in_camp(n,dg_goal[player]))continue;
   int candidate=dg_distance[n][anchor];
   if(best<0 || (distant?candidate>score:candidate<score)){best=n;score=candidate;}
  }
  CHECK(best>=0);game->pos.board[best]=player;count++;
 }
 CHECK(count==total);
}
static DgGame base(uint8_t players,bool synthetic)
{
 DgGame game;CHECK(dg_new(&game,players,DG_EASY,1,1));
 CHECK(dg_current(&game)==DG_GREEN);
 if(synthetic){
  /* Historical unrestricted fixtures remain explicit V1 compatibility tests. */
  game.rules_revision=DG_RULES_V1;
  memset(game.pos.board,0,sizeof game.pos.board);
  memset(&game.undo,0,sizeof game.undo);game.undo_valid=0;
 }
 return game;
}
static DgGame midgame(uint8_t players)
{
 DgGame game=base(players,false);
 /* Twelve actual engine-validated EASY moves per participant: RED uses
    the same policy only to create a reproducible reachable benchmark. */
 unsigned plies=(unsigned)players*12;
 for(unsigned turn=0;turn<plies;turn++){
  DgMove move;uint32_t rng;DgAiStats stats;DgGame before=game;
  CHECK(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));
  CHECK(!memcmp(&before,&game,sizeof game));CHECK(dg_commit(&game,&move));
  game.pos.rng=rng;CHECK(dg_game_valid(&game));CHECK(!game.pos.winner);
 }
 CHECK(game.pos.turns==plies && dg_current(&game)==DG_GREEN);
 return game;
}
static DgGame congestion(uint8_t players)
{
 DgGame game=base(players,true);unsigned own=0;
 for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[DG_GREEN])){
  game.pos.board[n]=own<6?DG_GREEN:DG_RED;own++;
 }
 CHECK(own==DG_PIECES);
 int anchor=dg_coord(-3,0);CHECK(anchor>=0);
 fill_to(&game,DG_GREEN,DG_PIECES,NULL,anchor,false);
 fill_to(&game,DG_RED,DG_PIECES,NULL,anchor,false);
 if(players==3)fill_to(&game,DG_YELLOW,DG_PIECES,NULL,anchor,false);
 CHECK(dg_goal_count(game.pos.board,DG_GREEN)==6);
 return game;
}
static DgGame multijump(uint8_t players)
{
 DgGame game=base(players,true);bool reserved[DG_NODES]={false};
 const int landings[5][2]={{-2,0},{0,0},{0,2},{2,2},{2,0}};
 const int crossed[4][2]={{-1,0},{0,1},{1,2},{2,1}};
 for(unsigned i=0;i<5;i++){
  int n=dg_coord(landings[i][0],landings[i][1]);CHECK(n>=0);reserved[n]=true;
  if(!i)game.pos.board[n]=DG_GREEN;
 }
 for(unsigned i=0;i<4;i++){
  int n=dg_coord(crossed[i][0],crossed[i][1]);CHECK(n>=0);
  game.pos.board[n]=DG_RED;
 }
 int center=dg_coord(0,0);CHECK(center>=0);
 fill_to(&game,DG_GREEN,DG_PIECES,reserved,center,true);
 fill_to(&game,DG_RED,DG_PIECES,reserved,center,true);
 if(players==3)fill_to(&game,DG_YELLOW,DG_PIECES,reserved,center,true);
 DgMove route;DgPath path;
 CHECK(dg_find_move(dg_rules(&game),game.pos.board,DG_GREEN,dg_coord(-2,0),dg_coord(2,0),&route,&path));
 CHECK(route.type==DG_JUMP && route.hops>=2 && path.length==route.hops+1);
 return game;
}
static DgGame nearly_won(uint8_t players)
{
 DgGame game=base(players,true);int vacancy=-1,source=-1;
 for(int n=0;n<DG_NODES && source<0;n++)if(dg_in_camp(n,dg_goal[DG_GREEN])){
  for(unsigned d=0;d<6;d++){
   int neighbor=dg_nodes[n].neighbor[d];
   if(neighbor>=0 && !dg_in_camp(neighbor,dg_goal[DG_GREEN])){vacancy=n;source=neighbor;break;}
  }
 }
 CHECK(vacancy>=0 && source>=0);
 for(int n=0;n<DG_NODES;n++)if(n!=vacancy && dg_in_camp(n,dg_goal[DG_GREEN]))game.pos.board[n]=DG_GREEN;
 game.pos.board[source]=DG_GREEN;bool reserved[DG_NODES]={false};reserved[vacancy]=true;
 int center=dg_coord(0,0);CHECK(center>=0);
 fill_to(&game,DG_RED,DG_PIECES,reserved,center,false);
 if(players==3)fill_to(&game,DG_YELLOW,DG_PIECES,reserved,center,false);
 CHECK(dg_goal_count(game.pos.board,DG_GREEN)==9);
 DgMove winning;CHECK(dg_find_move(dg_rules(&game),game.pos.board,DG_GREEN,source,vacancy,&winning,NULL));
 DgGame copy=game;CHECK(dg_commit(&copy,&winning));CHECK(copy.pos.winner==DG_GREEN);
 return game;
}
static unsigned blocked_green(const DgGame *game)
{
 unsigned blocked=0;
 for(unsigned n=0;n<DG_NODES;n++)if(game->pos.board[n]==DG_GREEN){
  unsigned neighbors=0,occupied=0;
  for(unsigned d=0;d<6;d++){
   int next=dg_nodes[n].neighbor[d];
   if(next>=0){neighbors++;occupied+=game->pos.board[next]!=DG_EMPTY;}
  }
  if(neighbors && neighbors==occupied)blocked++;
 }
 return blocked;
}
static DgGame blocking(uint8_t players)
{
 DgGame game=base(players,true);int center=dg_coord(0,0);CHECK(center>=0);
 fill_to(&game,DG_GREEN,DG_PIECES,NULL,center,false);
 fill_to(&game,DG_RED,DG_PIECES,NULL,center,false);
 if(players==3)fill_to(&game,DG_YELLOW,DG_PIECES,NULL,center,false);
 CHECK(blocked_green(&game)>0);
 return game;
}
static uint64_t board_hash(const DgGame *game)
{
 uint64_t hash=UINT64_C(14695981039346656037);
 for(unsigned n=0;n<DG_NODES;n++){hash^=game->pos.board[n];hash*=UINT64_C(1099511628211);}
 hash^=game->pos.turn;return hash;
}
static void bench(DgGame game,unsigned fixture)
{
 CHECK(dg_game_valid(&game) && dg_current(&game)==DG_GREEN && !game.pos.winner);
 CHECK(pieces(&game,DG_RED)==DG_PIECES && pieces(&game,DG_GREEN)==DG_PIECES);
 CHECK(pieces(&game,DG_YELLOW)==(game.players==3?DG_PIECES:0u));
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(dg_rules(&game),game.pos.board,DG_GREEN,moves,DG_MAX_MOVES);
 CHECK(count>0 && count<=DG_MAX_MOVES);
 unsigned max_hops=0,goal_occupied=0;
 for(size_t i=0;i<count;i++)if(moves[i].type==DG_JUMP && moves[i].hops>max_hops)max_hops=moves[i].hops;
 for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[DG_GREEN]) && game.pos.board[n])goal_occupied++;
 if(fixture==MULTIJUMP)CHECK(max_hops>=2);
 if(fixture==CONGESTION)CHECK(goal_occupied==DG_PIECES);
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(unsigned difficulty=0u;difficulty<3u;++difficulty){
  uint8_t level=levels[difficulty];
  game.level=level;DgGame before=game;DgMove move,valid;uint32_t rng;DgAiStats stats;
  clock_t start=clock();CHECK(start!=(clock_t)-1);
  CHECK(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));
  clock_t end=clock();CHECK(end!=(clock_t)-1);
  double elapsed=(double)(end-start)/(double)CLOCKS_PER_SEC;
  CHECK(!memcmp(&before,&game,sizeof game));
  CHECK(dg_find_move(dg_rules(&game),game.pos.board,DG_GREEN,move.from,move.to,&valid,NULL));
  CHECK(!memcmp(&move,&valid,sizeof move));CHECK(stats.legal_moves==count && !stats.cancelled);
  DgAiProfile profile;CHECK(dg_ai_profile(game.players,level,&profile));
  if(level!=DG_EASY)CHECK(stats.nodes<=profile.node_budget);
  DgGame applied=game;CHECK(dg_commit(&applied,&move));applied.pos.rng=rng;
  CHECK(dg_game_valid(&applied));
  if(fixture==NEARLY_WON)CHECK(applied.pos.winner==DG_GREEN);
  printf("%uP,%s,%s,%s,%lu,%016llx,%u,%u,%u,%u,%lu,%lu,%u,%lu,%u,%.6f\n",
   (unsigned)game.players,names[fixture],fixture<=MIDGAME?"reachable":"synthetic_engine_valid",
   level==DG_EASY?"EASY":(level==DG_NORMAL?"NORMAL":"HARD"),(unsigned long)game.pos.turns,(unsigned long long)board_hash(&game),
   (unsigned)dg_goal_count(game.pos.board,DG_GREEN),goal_occupied,max_hops,blocked_green(&game),
   (unsigned long)count,(unsigned long)stats.nodes,(unsigned)stats.depth,
   (unsigned long)stats.tt_hits,(unsigned)stats.beam,elapsed);
 }
}
int main(void)
{
 puts("mode,fixture,provenance,level,turns,board_hash,green_goal,goal_occupied,max_jump_hops,fully_blocked_green,legal_moves,nodes,depth,tt_hits,beam,host_seconds");
 for(uint8_t players=2;players<=3;players++){
  bench(base(players,false),OPENING);bench(midgame(players),MIDGAME);
  bench(congestion(players),CONGESTION);bench(multijump(players),MULTIJUMP);
  bench(nearly_won(players),NEARLY_WON);bench(blocking(players),BLOCKING);
 }
 fprintf(stderr,"AI fixtures PASS: 36 EASY/NORMAL/HARD selections across 12 positions; %u checks; host timing only\n",checks);
 return 0;
}
