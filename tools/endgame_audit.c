/* Host-only beta.3/beta.4 comparison. This file also compiles unchanged against
   the beta.3 core. Distances reported here are the common unrestricted lattice
   step-distance proxy, not a claim about the number of legal turns to win. */
#include "ai.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef DG_RULES_API
#define RULES(game) dg_rules(game),
#else
#define RULES(game)
#endif
static bool use_legacy;
static void revision(DgGame *game)
{
#ifdef DG_RULES_API
 if(use_legacy)game->rules_revision=DG_RULES_V1;
#else
 (void)game;(void)use_legacy;
#endif
}

static unsigned assignment(const uint8_t *board,uint8_t player)
{
 uint8_t pieces[DG_PIECES],holes[DG_PIECES];unsigned count=0,empty=0;
 for(unsigned n=0;n<DG_NODES;n++){
  if(board[n]==player && !dg_in_camp((int)n,dg_goal[player]))pieces[count++]=(uint8_t)n;
  if(board[n]!=player && dg_in_camp((int)n,dg_goal[player]))holes[empty++]=(uint8_t)n;
 }
 assert(count==empty && count<=DG_PIECES);
 unsigned dp[1u<<DG_PIECES];
 for(unsigned mask=0;mask<(1u<<count);mask++)dp[mask]=UINT_MAX/2u;
 dp[0]=0;
 for(unsigned mask=0;mask<(1u<<count);mask++){
  unsigned used=0;for(unsigned b=mask;b;b>>=1)used+=b&1u;
  if(used==count)continue;
  for(unsigned h=0;h<count;h++)if(!(mask&(1u<<h))){
   unsigned next=mask|(1u<<h),cost=dp[mask]+dg_distance[pieces[used]][holes[h]];
   if(cost<dp[next])dp[next]=cost;
  }
 }
 return dp[(1u<<count)-1u];
}
static const char *level_name(uint8_t level)
{return level==DG_EASY?"EASY":level==DG_NORMAL?"NORMAL":"HARD";}
static unsigned wins(const DgGame *game)
{
 DgMove moves[DG_MAX_MOVES];uint8_t board[DG_NODES],player=dg_current(game);
 size_t count=dg_generate(RULES(game) game->pos.board,player,moves,DG_MAX_MOVES);unsigned total=0;
 assert(count<=DG_MAX_MOVES);
 for(size_t i=0;i<count;i++){
  memcpy(board,game->pos.board,DG_NODES);
  assert(dg_apply(RULES(game) board,player,&moves[i]));total+=dg_won(board,player);
 }
 return total;
}
static void selection(DgGame *game,unsigned fixture,const char *kind,DgMove *previous)
{
 uint8_t player=dg_current(game);unsigned before=dg_goal_count(game->pos.board,player);
 unsigned cost=assignment(game->pos.board,player),winning=wins(game);
 DgMove move;DgAiStats stats;uint32_t rng;DgGame snapshot=*game;
 clock_t start=clock();bool ok=dg_ai_choose(game,0,NULL,NULL,&move,&rng,&stats);
 double seconds=(double)(clock()-start)/CLOCKS_PER_SEC;
 assert(ok && !memcmp(&snapshot,game,sizeof snapshot));
 assert(dg_find_move(RULES(game) game->pos.board,player,move.from,move.to,NULL,NULL));
 bool reverse=previous[player].from==move.to && previous[player].to==move.from;
 assert(dg_commit(game,&move));game->pos.rng=rng;
 unsigned after=dg_goal_count(game->pos.board,player);
 printf("%u,%s,%u,%u,%s,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%lu,%u,%.6f\n",
  fixture,kind,(unsigned)game->players,(unsigned)player,level_name(game->level),
  (unsigned long)snapshot.pos.turns,(unsigned)move.from,(unsigned)move.to,before,after,
  cost,assignment(game->pos.board,player),(unsigned)(after<before),winning,
  (unsigned)(winning && game->pos.winner!=player),(unsigned)reverse,
  (unsigned long)stats.nodes,(unsigned)stats.depth,seconds);
 previous[player]=move;
}
static void fill_opponents(DgGame *game,uint8_t player,const bool *reserved)
{
 for(unsigned slot=0;slot<game->players;slot++){
  uint8_t other=game->order[slot];if(other==player)continue;
  unsigned count=0;
  for(unsigned pass=0;pass<2 && count<DG_PIECES;pass++)for(unsigned n=0;n<DG_NODES && count<DG_PIECES;n++){
   bool preferred=pass==0?dg_in_camp((int)n,dg_home[other]):dg_nodes[n].region==0;
   if(preferred && !reserved[n] && !game->pos.board[n]){game->pos.board[n]=other;count++;}
  }
  assert(count==DG_PIECES);
 }
}
static DgGame make_fixture(uint8_t players,uint8_t player,unsigned count,unsigned seed)
{
   DgGame initial;assert(dg_new(&initial,players,DG_EASY,0,seed));
   revision(&initial);
   memset(initial.pos.board,0,DG_NODES);bool reserved[DG_NODES]={false};
   uint8_t goals[DG_PIECES],neutral[DG_NODES];unsigned g=0,ns=0;
   for(unsigned n=0;n<DG_NODES;n++){
    if(dg_in_camp((int)n,dg_goal[player])){goals[g++]=(uint8_t)n;reserved[n]=true;}
    if(!dg_nodes[n].region)neutral[ns++]=(uint8_t)n;
   }
   assert(g==DG_PIECES);uint32_t shuffle=seed*7919u+(uint32_t)player*37u;
   for(unsigned i=g-1;i>0;i--){unsigned j=dg_random(&shuffle)%(i+1u);uint8_t temp=goals[i];goals[i]=goals[j];goals[j]=temp;}
   for(unsigned i=ns-1;i>0;i--){unsigned j=dg_random(&shuffle)%(i+1u);uint8_t temp=neutral[i];neutral[i]=neutral[j];neutral[j]=temp;}
   for(unsigned i=0;i<count;i++)initial.pos.board[goals[i]]=player;
   for(unsigned i=0;i<DG_PIECES-count;i++)initial.pos.board[neutral[i]]=player;
   fill_opponents(&initial,player,reserved);
   for(uint8_t slot=0;slot<players;slot++)if(initial.order[slot]==player)initial.pos.turn=slot;
   assert(dg_game_valid(&initial));
 return initial;
}
static void fixtures(void)
{
 unsigned id=0;const uint8_t levels[]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  for(unsigned count=7;count<=9;count++)for(unsigned seed=1;seed<=4;seed++){
   DgGame initial=make_fixture(players,player,count,seed);
   for(unsigned level=0;level<3;level++){
    DgGame game=initial;game.level=levels[level];DgMove previous[4];memset(previous,255,sizeof previous);
    selection(&game,id,"synthetic_endgame",previous);
   }
   id++;
  }
 }
}
/* Reproduce historical matched openings, then log each own turn. The limit
   is diagnostic only. No game rule is created by this host harness. */
static void trace(uint8_t players,uint32_t seed,uint8_t slot,const uint8_t *levels,unsigned cap)
{
 DgGame game;assert(dg_new(&game,players,DG_EASY,slot,seed));
 revision(&game);
 DgMove previous[4];memset(previous,255,sizeof previous);
 unsigned opening=players==2?8u:9u;
 for(unsigned ply=0;ply<cap && !game.pos.winner;ply++){
  game.level=ply<opening?DG_EASY:levels[dg_current(&game)];
  selection(&game,0,"historical_seed_trace",previous);
 }
 fprintf(stderr,"trace: turns=%lu winner=%u goals=%u/%u/%u\n",(unsigned long)game.pos.turns,
  (unsigned)game.pos.winner,(unsigned)dg_goal_count(game.pos.board,DG_RED),
  (unsigned)dg_goal_count(game.pos.board,DG_YELLOW),(unsigned)dg_goal_count(game.pos.board,DG_GREEN));
}
int main(int argc,char **argv)
{
 if(argc>1 && !strcmp(argv[1],"--legacy")){use_legacy=true;argc--;argv++;}
 puts("fixture,kind,players,player,level,ply,from,to,goal_before,goal_after,assignment_before,assignment_after,goal_exit,immediate_wins,missed_win,reversal,nodes,depth,host_seconds");
 if(argc==8){
  uint8_t levels[4]={0,(uint8_t)atoi(argv[4]),(uint8_t)atoi(argv[5]),(uint8_t)atoi(argv[6])};
  trace((uint8_t)atoi(argv[1]),(uint32_t)strtoul(argv[2],NULL,10),(uint8_t)atoi(argv[3]),levels,(unsigned)atoi(argv[7]));
 }else fixtures();
 return 0;
}
