/* Host-only diagnostics. Timing below is host CPU time, never calculator time. */
#include "ai.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint32_t bounded(const char *text,uint32_t fallback)
{
 if(text == NULL) return fallback;
 char *end = NULL;
 unsigned long value = strtoul(text,&end,10);
 return *end == '\0' && value > 0ul && value <= UINT32_MAX ? (uint32_t)value : fallback;
}

static double seconds(clock_t start) { return (double)(clock() - start) / (double)CLOCKS_PER_SEC; }

static void apply_ai(DgGame *game,uint32_t budget,DgAiStats *stats)
{
 DgMove move,valid;
 uint32_t rng;
 DgGame before = *game;
 assert(dg_ai_choose(game,budget,NULL,NULL,&move,&rng,stats));
 assert(memcmp(game,&before,sizeof(before)) == 0);
 assert(dg_find_move(dg_rules(game),game->pos.board,dg_current(game),move.from,move.to,&valid,NULL));
 assert(memcmp(&move,&valid,sizeof(move)) == 0);
 assert(dg_commit(game,&move));
 game->pos.rng = rng;
 assert(dg_game_valid(game));
}

static void bench(void)
{
 puts("mode,level,position,legal_moves,nodes,depth,tt_hits,beam,host_seconds");
 for(uint8_t players = 2u; players <= 3u; ++players) {
  DgGame game;
  assert(dg_new(&game,players,DG_EASY,0u,12345u));
  for(unsigned position = 0u; position < 3u; ++position) {
   static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
   for(unsigned difficulty=0u;difficulty<3u;++difficulty) {
    uint8_t level=levels[difficulty];
    DgMove move;
    uint32_t rng;
    DgAiStats stats;
    game.level = level;
    clock_t start = clock();
    assert(dg_ai_choose(&game,0u,NULL,NULL,&move,&rng,&stats));
    printf("%uP,%s,%u,%u,%lu,%u,%lu,%u,%.6f\n",(unsigned)players,level == DG_EASY ? "EASY" : (level==DG_NORMAL?"NORMAL":"HARD"),position,
     (unsigned)stats.legal_moves,(unsigned long)stats.nodes,(unsigned)stats.depth,
     (unsigned long)stats.tt_hits,(unsigned)stats.beam,seconds(start));
   }
   game.level = DG_EASY;
   for(unsigned step = 0u; step < 20u && game.pos.winner == DG_EMPTY; ++step) {
    DgAiStats stats; apply_ai(&game,0u,&stats);
   }
   if(game.pos.winner != DG_EMPTY) break;
  }
 }
 printf("AI workspace bytes: %lu; TT bytes: 0\n",(unsigned long)dg_ai_workspace_bytes());
}

static void stress(uint32_t positions,uint32_t budget)
{
 clock_t start = clock();
 uint32_t random_state = 987654321u;
 uint64_t nodes = 0u;
 for(uint8_t players = 2u; players <= 3u; ++players) {
  DgGame game;
  assert(dg_new(&game,players,DG_EASY,0u,random_state));
  for(uint32_t index = 0u; index < positions; ++index) {
   static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
   for(unsigned difficulty=0u;difficulty<3u;++difficulty) {
    uint8_t level=levels[difficulty];
    game.level = level;
    DgMove move;
    uint32_t rng;
    DgAiStats stats;
    DgGame before = game;
    assert(dg_ai_choose(&game,budget,NULL,NULL,&move,&rng,&stats));
    assert(memcmp(&before,&game,sizeof(game)) == 0);
    uint8_t copied[DG_NODES];
    memcpy(copied,game.pos.board,sizeof(copied));
    assert(dg_apply(dg_rules(&game),copied,dg_current(&game),&move));
    unsigned pieces[4] = {0u,0u,0u,0u};
    for(unsigned n = 0u; n < DG_NODES; ++n) ++pieces[copied[n]];
    assert(pieces[DG_RED] == DG_PIECES && pieces[DG_GREEN] == DG_PIECES);
    assert(pieces[DG_YELLOW] == (players == 3u ? DG_PIECES : 0u));
    nodes += stats.nodes;
   }
   DgMove moves[DG_MAX_MOVES];
   size_t count = dg_generate(dg_rules(&game),game.pos.board,dg_current(&game),moves,DG_MAX_MOVES);
   assert(count > 0u && count <= DG_MAX_MOVES);
   assert(dg_commit(&game,&moves[dg_random(&random_state) % count]));
   assert(dg_game_valid(&game));
   if(game.pos.winner != DG_EMPTY || (index + 1u) % 200u == 0u)
    assert(dg_new(&game,players,DG_EASY,(uint8_t)(dg_random(&random_state) % players),dg_random(&random_state)));
  }
 }
 printf("reachable_positions=%lu ai_legality_checks=%lu nodes=%llu host_seconds=%.3f\n",
  (unsigned long)(positions * 2u),(unsigned long)(positions * 6u),(unsigned long long)nodes,seconds(start));
}

static uint64_t board_hash(const DgGame *game)
{
 uint64_t hash = UINT64_C(14695981039346656037);
 for(unsigned n = 0u; n < DG_NODES; ++n) { hash ^= game->pos.board[n]; hash *= UINT64_C(1099511628211); }
 hash ^= game->pos.turn;
 return hash;
}

static void selfplay(uint32_t games,uint32_t cap,uint32_t budget)
{
 puts("mode,policy,games,completed,capped,cycle_games,mean_turns,maximum_turns,host_seconds");
 for(uint8_t players = 2u; players <= 3u; ++players)
  for(unsigned policy = 0u; policy < 3u; ++policy) {
   clock_t start = clock();
   uint32_t completed = 0u,cycles = 0u,maximum = 0u;
   uint64_t turns = 0u;
   for(uint32_t index = 0u; index < games; ++index) {
    DgGame game;
    assert(dg_new(&game,players,DG_EASY,(uint8_t)(index % players),index + 24681u));
    uint64_t recent[128] = {0u};
    bool cycle = false;
    for(uint32_t step = 0u; step < cap && game.pos.winner == DG_EMPTY; ++step) {
     game.level = policy == 0u ? DG_EASY : (policy == 1u ? DG_HARD : (uint8_t)((step / players) & 1u));
     uint64_t hash = board_hash(&game);
     for(unsigned k = 0u; k < 128u; ++k) if(recent[k] == hash) cycle = true;
     recent[step % 128u] = hash;
     DgAiStats stats;
     apply_ai(&game,budget,&stats);
    }
    if(game.pos.winner != DG_EMPTY) ++completed;
    if(cycle) ++cycles;
    turns += game.pos.turns;
    if(game.pos.turns > maximum) maximum = game.pos.turns;
   }
   printf("%uP,%s,%lu,%lu,%lu,%lu,%.2f,%lu,%.3f\n",(unsigned)players,
    policy == 0u ? "EASY" : (policy == 1u ? "HARD" : "MIXED"),(unsigned long)games,
    (unsigned long)completed,(unsigned long)(games - completed),(unsigned long)cycles,
    (double)turns / (double)games,(unsigned long)maximum,seconds(start));
   fflush(stdout);
  }
}

int main(int argc,char **argv)
{
 if(argc < 2 || strcmp(argv[1],"--bench") == 0) bench();
 else if(strcmp(argv[1],"--stress") == 0) stress(bounded(argc > 2 ? argv[2] : NULL,2000u),bounded(argc > 3 ? argv[3] : NULL,128u));
 else if(strcmp(argv[1],"--selfplay") == 0) selfplay(bounded(argc > 2 ? argv[2] : NULL,100u),bounded(argc > 3 ? argv[3] : NULL,1200u),bounded(argc > 4 ? argv[4] : NULL,0u));
 else { fputs("Usage: ai_audit --bench | --stress [positions_per_mode] [budget] | --selfplay [games_per_policy_mode] [simulation_cap] [budget]\n",stderr); return 2; }
 return 0;
}
