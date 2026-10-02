#include "ai.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <string.h>

typedef struct { unsigned calls,stop; } CancelState;
static bool stop_search(void *context)
{
 CancelState *state = context;
 ++state->calls;
 return state->calls >= state->stop;
}

static void immediate_win(uint8_t player)
{
 DgGame game;
 assert(dg_new(&game,3u,DG_HARD,0u,17u));
 game.rules_revision=DG_RULES_V1; /* Historical unrestricted tactical fixture. */
 memset(game.pos.board,0,sizeof(game.pos.board));
 int vacancy = -1,source = -1;
 for(int n = 0; n < DG_NODES && source < 0; ++n) {
  if(!dg_in_camp(n,dg_goal[player])) continue;
  for(unsigned d = 0u; d < 6u; ++d) {
   int adjacent = dg_nodes[n].neighbor[d];
   if(adjacent >= 0 && !dg_in_camp(adjacent,dg_goal[player])) {
    vacancy = n; source = adjacent; break;
   }
  }
 }
 assert(source >= 0 && vacancy >= 0);
 for(int n = 0; n < DG_NODES; ++n)
  if(dg_in_camp(n,dg_goal[player]) && n != vacancy) game.pos.board[n] = player;
 game.pos.board[source] = player;
 for(uint8_t other = DG_RED; other <= DG_GREEN; ++other) {
  if(other == player) continue;
  unsigned used = 0u;
  for(unsigned n = 0u; n < DG_NODES && used < DG_PIECES; ++n) {
   if(game.pos.board[n] == DG_EMPTY && (int)n != vacancy &&
      !dg_in_camp((int)n,dg_goal[other])) {
    game.pos.board[n] = other; ++used;
   }
  }
  assert(used == DG_PIECES);
 }
 for(uint8_t slot = 0u; slot < game.players; ++slot) if(game.order[slot] == player) game.pos.turn = slot;
 assert(dg_game_valid(&game));
 DgMove move;
 uint32_t rng;
 DgAiStats stats;
 assert(dg_ai_choose(&game,5000u,NULL,NULL,&move,&rng,&stats));
 assert(move.to == (uint8_t)vacancy && move.from == (uint8_t)source);
 assert(dg_commit(&game,&move));
 assert(game.pos.winner == player);
}

static uint64_t golden_mix(uint64_t hash,uint32_t value)
{
 for(unsigned byte=0u;byte<4u;++byte){hash^=(value>>(8u*byte))&255u;hash*=UINT64_C(1099511628211);}
 return hash;
}
static void easy_golden(void)
{
 static const struct {uint8_t players,slot;uint32_t seed;uint64_t hash;} cases[]={
{2u,0u,1u,UINT64_C(0xa342fe18b1f4b777)},
{2u,0u,17u,UINT64_C(0xba41cefa0488b626)},
{2u,0u,71u,UINT64_C(0x42434d89bc47ec2b)},
{2u,0u,123456u,UINT64_C(0xabbd1eeecaa8b398)},
{2u,1u,1u,UINT64_C(0x486e002341262ee8)},
{2u,1u,17u,UINT64_C(0xf5b09675cb2fc493)},
{2u,1u,71u,UINT64_C(0x4ebc97c6ff5f47e1)},
{2u,1u,123456u,UINT64_C(0xb1c9317458b3220a)},
{3u,0u,1u,UINT64_C(0x757cebd37f50245e)},
{3u,0u,17u,UINT64_C(0x67d021bf2e0b147a)},
{3u,0u,71u,UINT64_C(0x0133b9287fb7d4ad)},
{3u,0u,123456u,UINT64_C(0xba98c554b3cd0dff)},
{3u,1u,1u,UINT64_C(0x754eede2d4b143b1)},
{3u,1u,17u,UINT64_C(0x25bd618a4ea25bdf)},
{3u,1u,71u,UINT64_C(0x9a9fb2248abf85f9)},
{3u,1u,123456u,UINT64_C(0x4ed0beb526c40161)},
{3u,2u,1u,UINT64_C(0xc1c2bff2b2983c1d)},
{3u,2u,17u,UINT64_C(0xfdeb48d9ada108db)},
{3u,2u,71u,UINT64_C(0x35b30ea85d8a7555)},
{3u,2u,123456u,UINT64_C(0xdb99616f2f674102)},
};
 for(unsigned index=0u;index<sizeof cases/sizeof cases[0];++index){
  DgGame game;assert(dg_new(&game,cases[index].players,DG_EASY,cases[index].slot,cases[index].seed));
  game.rules_revision=DG_RULES_V1; /* Historical opening/midgame RNG evidence. */
  uint64_t hash=UINT64_C(14695981039346656037);
  for(unsigned step=0u;step<36u;++step){
   DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&game,0u,NULL,NULL,&move,&rng,&stats));
   hash=golden_mix(hash,move.from);hash=golden_mix(hash,move.to);
   hash=golden_mix(hash,move.type);hash=golden_mix(hash,move.hops);hash=golden_mix(hash,rng);
   hash=golden_mix(hash,(uint32_t)dg_ai_evaluate(game.pos.board,dg_current(&game)));
   assert(dg_commit(&game,&move));game.pos.rng=rng;
  }
  assert(hash==cases[index].hash);
 }
}

void test_ai(void)
{
 assert(dg_ai_workspace_bytes() < 16384u);
 easy_golden();
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 DgAiProfile profile;assert(!dg_ai_profile(1u,DG_NORMAL,&profile));assert(!dg_ai_profile(2u,255u,&profile));
 assert(!dg_ai_profile(3u,DG_HARD,NULL));
 for(uint8_t players = 2u; players <= 3u; ++players)
  for(unsigned difficulty=0u;difficulty<3u;++difficulty) {
   uint8_t level=levels[difficulty];
   DgGame game,snapshot;
   assert(dg_new(&game,players,level,0u,123456u));
   snapshot = game;
   DgMove a,b,valid;
   uint32_t rng_a,rng_b;
   DgAiStats stats_a,stats_b;
   assert(dg_ai_choose(&game,128u,NULL,NULL,&a,&rng_a,&stats_a));
   assert(memcmp(&game,&snapshot,sizeof(game)) == 0);
   assert(dg_ai_choose(&game,128u,NULL,NULL,&b,&rng_b,&stats_b));
   assert(memcmp(&a,&b,sizeof(a)) == 0 && rng_a == rng_b);
   assert(dg_find_move(dg_rules(&game),game.pos.board,dg_current(&game),a.from,a.to,&valid,NULL));
   assert(memcmp(&a,&valid,sizeof(a)) == 0);
   assert(stats_a.legal_moves > 0u && stats_a.cancelled == 0u);
   if(level != DG_EASY) {
    assert(rng_a == game.pos.rng && stats_a.nodes <= 128u);
    DgMove fallback;
    uint32_t fallback_rng;
    DgAiStats fallback_stats;
    assert(dg_ai_choose(&game,1u,NULL,NULL,&fallback,&fallback_rng,&fallback_stats));
    assert(fallback_stats.nodes <= 1u && fallback_stats.depth == 0u);
    assert(dg_find_move(dg_rules(&game),game.pos.board,dg_current(&game),fallback.from,fallback.to,NULL,NULL));
    assert(stats_a.nodes == stats_b.nodes && stats_a.depth == stats_b.depth);
    CancelState during_search = {0u,40u};
    assert(!dg_ai_choose(&game,12000u,stop_search,&during_search,&a,&rng_a,&stats_a));
    assert(stats_a.cancelled && stats_a.nodes > 0u);
    assert(memcmp(&game,&snapshot,sizeof(game)) == 0);
   } else assert(rng_a != game.pos.rng);
   CancelState immediate = {0u,1u};
   assert(!dg_ai_choose(&game,128u,stop_search,&immediate,&a,&rng_a,&stats_a));
   assert(stats_a.cancelled && immediate.calls == 1u);
   assert(memcmp(&game,&snapshot,sizeof(game)) == 0);
   CancelState delayed = {0u,4u};
   assert(!dg_ai_choose(&game,128u,stop_search,&delayed,&a,&rng_a,&stats_a));
   assert(stats_a.cancelled && delayed.calls == 4u);
   assert(memcmp(&game,&snapshot,sizeof(game)) == 0);
  }
 for(uint8_t player = DG_RED; player <= DG_GREEN; ++player) immediate_win(player);
}

#ifdef DG_AI_TEST_MAIN
#include <stdio.h>
int main(void) { test_ai(); puts("AI tests passed"); return 0; }
#endif
