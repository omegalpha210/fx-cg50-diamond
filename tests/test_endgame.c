/* Reuse only the deterministic position builders and independent host DP. */
#define main dg_endgame_audit_main
#include "../tools/endgame_audit.c"
#undef main

static void metrics_and_progress(void)
{
 unsigned samples=0,progress_checks=0,rearrangements=0;
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  for(unsigned goals=7;goals<=9;goals++)for(unsigned seed=1;seed<=4;seed++){
   DgGame game=make_fixture(players,player,goals,seed);DgEndgame metrics,legacy;
   assert(dg_ai_endgame_metrics(dg_rules(&game),game.pos.board,player,&metrics));
   assert(dg_ai_endgame_metrics((DgRules){DG_RULES_V1,players},game.pos.board,player,&legacy));
   assert(legacy.assignment==assignment(game.pos.board,player));
   assert(metrics.assignment>=legacy.assignment && metrics.goals==goals && metrics.goals+metrics.empty+metrics.blocked==10);
   if(players==3){
    uint8_t rotated[DG_NODES]={0};const uint8_t color[4]={0,DG_YELLOW,DG_GREEN,DG_RED};
    for(int n=0;n<DG_NODES;n++){
     int q=dg_nodes[n].q,r=dg_nodes[n].r;
     for(unsigned turn=0;turn<2;turn++){int nq=-r;r=q+r;q=nq;}
     int to=dg_coord(q,r);assert(to>=0);rotated[to]=color[game.pos.board[n]];
    }
    DgEndgame transformed;assert(dg_ai_endgame_metrics(dg_rules(&game),rotated,color[player],&transformed));
    assert(metrics.assignment==transformed.assignment && metrics.goals==transformed.goals && metrics.depth==transformed.depth && metrics.settled==transformed.settled);
   }
   DgMove legal[DG_MAX_MOVES];size_t count=dg_generate(dg_rules(&game),game.pos.board,player,legal,DG_MAX_MOVES);
   bool progress=false;DgMove useless={0};bool have_useless=false;
   for(size_t i=0;i<count;i++){
    uint8_t board[DG_NODES];memcpy(board,game.pos.board,DG_NODES);assert(dg_apply(dg_rules(&game),board,player,&legal[i]));
    DgEndgame after;assert(dg_ai_endgame_metrics(dg_rules(&game),board,player,&after));
    if(after.goals>=goals && after.assignment<metrics.assignment)progress=true;
    else if(after.goals==goals){useless=legal[i];have_useless=true;}
    if(after.goals<goals)rearrangements++; /* Soft policy keeps every legal exit. */
   }
   if(goals==9 && progress && have_useless){
    game.history[0]=(DgRecent){0,player,useless.to,useless.from};game.history_count=game.history_next=1;
    game.level=DG_HARD;DgMove chosen;uint32_t rng;DgAiStats stats;
    assert(dg_ai_choose(&game,0,NULL,NULL,&chosen,&rng,&stats));
    assert(chosen.from!=useless.from || chosen.to!=useless.to);
    assert(dg_commit(&game,&chosen));DgEndgame after;
    assert(dg_ai_endgame_metrics(dg_rules(&game),game.pos.board,player,&after));
    assert(after.goals>=goals && after.assignment<=metrics.assignment);progress_checks++;
   }
   samples++;
  }
 }
 assert(samples==60 && progress_checks>=10 && rearrangements>0);
 printf("Endgame metrics: %u fixtures, independent exact assignment DP, color rotation, %u one-hole progress/reversal checks; %u legal rearrangements retained\n",samples,progress_checks,rearrangements);
}
static void vacated_green_goal(void)
{
 const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(unsigned level=0;level<3;level++){
  DgGame game;assert(dg_new(&game,3,levels[level],0,1));assert(game.order[1]==DG_GREEN);
  memset(game.pos.board,0,DG_NODES);
  for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[DG_GREEN]) && n!=60)game.pos.board[n]=DG_GREEN;
  game.pos.board[51]=DG_GREEN;game.pos.board[60]=DG_RED;
  bool reserved[DG_NODES]={false};reserved[61]=true;
  for(uint8_t p=DG_RED;p<=DG_YELLOW;p++){
   unsigned placed=p==DG_RED?1u:0u;
   for(unsigned pass=0;pass<2 && placed<10;pass++)for(int n=0;n<DG_NODES && placed<10;n++)
    if(!game.pos.board[n] && !reserved[n] && dg_landing_allowed(dg_rules(&game),p,n) && !dg_in_camp(n,dg_goal[p]) &&
       (pass || dg_in_camp(n,dg_home[p]))){game.pos.board[n]=p;placed++;}
   assert(placed==10);
  }
  assert(dg_game_valid(&game));DgMove leave;
  assert(dg_find_move(dg_rules(&game),game.pos.board,DG_RED,60,61,&leave,NULL));assert(dg_commit(&game,&leave));
  assert(dg_current(&game)==DG_GREEN);DgMove finish;uint32_t rng;DgAiStats stats;
  assert(dg_ai_choose(&game,0,NULL,NULL,&finish,&rng,&stats));assert(stats.immediate_win);
  assert(finish.from==51 && finish.to==60);assert(dg_commit(&game,&finish));assert(game.pos.winner==DG_GREEN);
 }
}
int main(void){metrics_and_progress();vacated_green_goal();return 0;}
