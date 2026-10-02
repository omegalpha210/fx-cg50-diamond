#include "ai.h"
#include <limits.h>
#include <string.h>

#define AI_MAX_DEPTH 9u
#define AI_ROOT_WIDTH 32u
#define AI_TWO_WIDTH 16u
#define AI_THREE_WIDTH 10u
#define AI_WIN 1000000
#define AI_INF 2000000

typedef struct { DgMove move; int score; } RankedMove;
typedef struct { int value[4]; } Vector;
typedef struct {
 DgRules rules;
 const DgGame *game;
 DgEndgame root_metrics;
 uint8_t endgame[4];
 uint8_t board[DG_NODES],order[3],players,root;
 uint8_t ancestor[AI_MAX_DEPTH + 1u][DG_NODES];
 uint8_t goal_nodes[4][DG_PIECES],goal_distance[4][DG_NODES],home_distance[4][DG_NODES];
 DgMove generated[DG_MAX_MOVES];
 RankedMove candidate[AI_MAX_DEPTH + 1u][AI_ROOT_WIDTH];
 uint32_t budget;
 DgAiProfile profile;
 DgCancel cancel;
 void *context;
 DgAiStats stats;
 bool stopped,root_tactics,endgame_evaluation;
} AiWorkspace;
/* One bounded workspace: no heap allocations or recursive move-buffer copies. */
static AiWorkspace ai;
static bool distances_ready;

bool dg_ai_profile(uint8_t players,uint8_t level,DgAiProfile *profile)
{
 if(profile==NULL || (players!=2u && players!=3u))return false;
 if(level==DG_EASY)*profile=(DgAiProfile){0u,1u,AI_ROOT_WIDTH,3u};
 else if(level==DG_NORMAL)*profile=players==2u?(DgAiProfile){1600u,2u,24u,8u}:(DgAiProfile){1600u,4u,20u,3u};
 else if(level==DG_HARD)*profile=players==2u?(DgAiProfile){12000u,4u,AI_ROOT_WIDTH,AI_TWO_WIDTH}:(DgAiProfile){12000u,7u,AI_ROOT_WIDTH,2u};
 else return false;
 return true;
}

static void prepare_distances(void)
{
 if(distances_ready) return;
 for(uint8_t p = DG_RED; p <= DG_GREEN; ++p) {
  unsigned goal_index = 0u;
  for(unsigned n = 0u; n < DG_NODES; ++n) {
   unsigned goal = UINT_MAX,home = UINT_MAX;
   for(unsigned t = 0u; t < DG_NODES; ++t) {
    if(dg_in_camp((int)t,dg_goal[p]) && dg_distance[n][t] < goal) goal = dg_distance[n][t];
    if(dg_in_camp((int)t,dg_home[p]) && dg_distance[n][t] < home) home = dg_distance[n][t];
   }
   ai.goal_distance[p][n] = (uint8_t)goal;
   ai.home_distance[p][n] = (uint8_t)home;
   if(dg_in_camp((int)n,dg_goal[p]) && goal_index < DG_PIECES)
    ai.goal_nodes[p][goal_index++] = (uint8_t)n;
  }
 }
 distances_ready = true;
}

int dg_ai_evaluate(const uint8_t *board,uint8_t player)
{
 if(board == NULL || player < DG_RED || player > DG_GREEN) return 0;
 prepare_distances();
 int score = 0;
 unsigned goal_count = 0u;
 for(unsigned n = 0u; n < DG_NODES; ++n) {
  if(board[n] != player) continue;
  bool in_goal = dg_in_camp((int)n,dg_goal[player]);
  if(in_goal) {
   ++goal_count;
   /* Pack the distant tip first; this leaves goal entrances usable. */
   score += 300 + 5 * (int)ai.home_distance[player][n];
  } else {
   unsigned vacant_distance = UINT_MAX;
   for(unsigned t = 0u; t < DG_PIECES; ++t) {
    unsigned goal = ai.goal_nodes[player][t];
    if(board[goal] != player && dg_distance[n][goal] < vacant_distance)
     vacant_distance = dg_distance[n][goal];
   }
   if(vacant_distance != UINT_MAX) score -= 25 * (int)vacant_distance;
   /* Lingering in another player's target can create mutual endgame
      blockades. Discourage it as a heuristic, never a legality rule. */
   for(uint8_t other = DG_RED; other <= DG_GREEN; ++other)
    if(other != player && dg_in_camp((int)n,dg_goal[other])) score -= 180;
   for(unsigned d = 0u; d < 6u; ++d) {
    int neighbor = dg_nodes[n].neighbor[d],landing = dg_nodes[n].jump[d];
    if(neighbor < 0) continue;
    if(board[neighbor] == DG_EMPTY) score += 2;
    else if(landing >= 0 && board[landing] == DG_EMPTY) score += 3;
   }
  }
  score -= 60 * (int)ai.goal_distance[player][n];
  score += 3 * (int)ai.home_distance[player][n];
  if(dg_in_camp((int)n,dg_home[player])) score -= 35;
 }
 return goal_count == DG_PIECES ? AI_WIN : score;
}

static int evaluate(const uint8_t *board,uint8_t player)
{
 int score=dg_ai_evaluate(board,player);
 if(score==AI_WIN || !ai.endgame_evaluation)return score;
 if(ai.endgame[player] || dg_goal_count(board,player)>=DG_ENDGAME_GOALS){
  DgEndgame metrics;
  if(dg_ai_endgame_metrics(ai.rules,board,player,&metrics))
   score+=600*(int)metrics.goals-180*(int)metrics.assignment+12*(int)metrics.depth+24*(int)metrics.settled-60*(int)metrics.blocked;
 }
 return score;
}
static int recent_penalty(DgMove move)
{
 if(!ai.game || !ai.endgame_evaluation || !ai.endgame[ai.root])return 0;
 DgEndgame after;if(!dg_ai_endgame_metrics(ai.rules,ai.board,ai.root,&after))return 0;
 int penalty=0;
 if(after.goals<ai.root_metrics.goals)penalty+=(ai.root_tactics?600:200)*(int)(ai.root_metrics.goals-after.goals);
 if(after.assignment>ai.root_metrics.assignment)penalty+=(ai.root_tactics?120:40)*(int)(after.assignment-ai.root_metrics.assignment);
 if(after.goals>ai.root_metrics.goals || (after.goals==ai.root_metrics.goals && after.assignment<ai.root_metrics.assignment))return penalty;
 uint8_t turn=(uint8_t)((ai.game->pos.turn+1u)%ai.players);
 uint32_t hash=dg_board_hash(ai.board,ai.order[turn]);bool own_seen=false;
 for(unsigned i=0;i<ai.game->history_count;i++){
  unsigned at=(ai.game->history_next+DG_RECENT_TURNS-1u-i)%DG_RECENT_TURNS;
  const DgRecent *recent=&ai.game->history[at];
  if(recent->hash==hash)penalty+=ai.root_tactics?240:80;
  if(!own_seen && recent->player==ai.root){
   own_seen=true;if(recent->from==move.to && recent->to==move.from)penalty+=ai.root_tactics?120:40;
  }
 }
 return penalty;
}
size_t dg_ai_workspace_bytes(void) { return sizeof(ai) + sizeof(distances_ready)+dg_ai_endgame_workspace_bytes(); }

static bool cancelled(void)
{
 if(ai.cancel != NULL && ai.cancel(ai.context)) {
  ai.stats.cancelled = 1u;
  ai.stopped = true;
 }
 return ai.stopped;
}

static bool visit(void)
{
 if(ai.stopped) return false;
 if((ai.stats.nodes & 31u) == 0u && cancelled()) return false;
 if(ai.stats.nodes >= ai.budget) { ai.stopped = true; return false; }
 ++ai.stats.nodes;
 return true;
}

static void make_move(DgMove move,uint8_t player)
{
 ai.board[move.from] = DG_EMPTY;
 ai.board[move.to] = player;
}

static void unmake_move(DgMove move,uint8_t player)
{
 ai.board[move.to] = DG_EMPTY;
 ai.board[move.from] = player;
}

static bool precedes(RankedMove a,RankedMove b)
{
 if(a.score != b.score) return a.score > b.score;
 if(a.move.from != b.move.from) return a.move.from < b.move.from;
 return a.move.to < b.move.to;
}

/* Threat recognition uses the engine's path validator, not another move
   legality implementation. A nine-goal player has one outside piece. */
static bool immediate_threat(uint8_t player)
{
 if(dg_goal_count(ai.board,player)!=9u)return false;
 int from=-1,to=-1;
 for(int node=0;node<DG_NODES;++node){
  if(ai.board[node]==player && !dg_in_camp(node,dg_goal[player]))from=node;
  if(dg_in_camp(node,dg_goal[player]) && ai.board[node]==DG_EMPTY)to=node;
 }
 return from>=0 && to>=0 && dg_find_move(ai.rules,ai.board,player,from,to,NULL,NULL);
}

/* Engine supplies all legality. Ranking only selects a bounded search beam. */
static unsigned rank_moves(uint8_t player,unsigned ply,unsigned width,bool root)
{
 size_t count = dg_generate(ai.rules,ai.board,player,ai.generated,DG_MAX_MOVES);
 if(root) ai.stats.legal_moves = (uint16_t)count;
 if(count > DG_MAX_MOVES) count = DG_MAX_MOVES;
 /* Every legal immediate win is considered before beam pruning. */
 if(dg_goal_count(ai.board,player)==DG_PIECES-1u)for(size_t i=0;i<count;i++){
  if((root || (i&31u)==0u) && cancelled())return 0;
  DgMove move=ai.generated[i];
  if(!dg_in_camp(move.from,dg_goal[player]) && dg_in_camp(move.to,dg_goal[player])){
   ai.candidate[ply][0]=(RankedMove){move,AI_WIN};return 1;
  }
 }
 unsigned used = 0u,threats=0u;
 if(root && ai.root_tactics)for(unsigned side=0u;side<ai.players;++side)
  if(ai.order[side]!=player && immediate_threat(ai.order[side]))threats|=1u<<ai.order[side];
 for(size_t i = 0u; i < count; ++i) {
  if((root || (i & 31u) == 0u) && cancelled()) return 0u;
  DgMove move = ai.generated[i];
  make_move(move,player);
  RankedMove item = {move,evaluate(ai.board,player)};
  if(root && item.score<AI_WIN)item.score-=recent_penalty(move);
  if(threats && item.score<AI_WIN){
   bool survives=true;
   for(uint8_t other=DG_RED;other<=DG_GREEN;++other)
    if((threats&(1u<<other)) && immediate_threat(other))survives=false;
   if(survives)item.score+=AI_WIN/2; /* Retain defensive options in HARD's beam. */
  }
  unmake_move(move,player);
  /* A small ordering bonus preserves goal progress as the primary score. */
  if(item.score < AI_WIN) item.score += (int)move.hops;
  unsigned at = used;
  if(at == width) {
   if(!precedes(item,ai.candidate[ply][at - 1u])) continue;
   --at;
  } else ++used;
  while(at > 0u && precedes(item,ai.candidate[ply][at - 1u])) {
   ai.candidate[ply][at] = ai.candidate[ply][at - 1u];
   --at;
  }
  ai.candidate[ply][at] = item;
 }
 return used;
}

static uint8_t winner(void)
{
 for(unsigned i = 0u; i < ai.players; ++i)
  if(dg_won(ai.board,ai.order[i])) return ai.order[i];
 return DG_EMPTY;
}

static int two_evaluation(uint8_t won)
{
 uint8_t opponent = ai.order[0] == ai.root ? ai.order[1] : ai.order[0];
 if(won != DG_EMPTY) return won == ai.root ? AI_WIN : -AI_WIN;
 return evaluate(ai.board,ai.root) - evaluate(ai.board,opponent);
}

static int alphabeta(unsigned remaining,unsigned ply,unsigned turn,int alpha,int beta)
{
 if(!visit()) return 0;
 uint8_t won = winner();
 if(won!=DG_EMPTY)return two_evaluation(won);
 for(unsigned earlier = 0u; earlier < ply; ++earlier)
  if((ply - earlier) % 2u == 0u && memcmp(ai.board,ai.ancestor[earlier],DG_NODES) == 0)
   return ai.endgame[ai.root]?two_evaluation(DG_EMPTY)+(ai.order[turn]==ai.root?160:-160):0;
 memcpy(ai.ancestor[ply],ai.board,DG_NODES);
 if(remaining == 0u || won != DG_EMPTY) return two_evaluation(won);
 uint8_t player = ai.order[turn];
 unsigned count = rank_moves(player,ply,ai.profile.beam,false);
 if(ai.stopped) return 0;
 if(count == 0u) return two_evaluation(DG_EMPTY);
 int best = player == ai.root ? -AI_INF : AI_INF;
 for(unsigned i = 0u; i < count; ++i) {
  DgMove move = ai.candidate[ply][i].move;
  make_move(move,player);
  int value = alphabeta(remaining - 1u,ply + 1u,(turn + 1u) % 2u,alpha,beta);
  unmake_move(move,player);
  if(ai.stopped) return 0;
  if(player == ai.root) {
   if(value > best) best = value;
   if(best > alpha) alpha = best;
  } else {
   if(value < best) best = value;
   if(best < beta) beta = best;
  }
  if(alpha >= beta) break;
 }
 return best;
}

static Vector vector_evaluation(uint8_t won)
{
 Vector result = {{0,0,0,0}};
 for(unsigned p = DG_RED; p <= DG_GREEN; ++p)
  result.value[p] = won == DG_EMPTY ? evaluate(ai.board,(uint8_t)p)
    : (won == p ? AI_WIN : -AI_WIN);
 return result;
}

static Vector maxn(unsigned remaining,unsigned ply,unsigned turn)
{
 Vector zero = {{0,0,0,0}};
 if(!visit()) return zero;
 uint8_t won = winner();
 if(remaining == 0u || won != DG_EMPTY) return vector_evaluation(won);
 if(ai.endgame[ai.root])for(unsigned earlier=ply%3u;earlier<ply;earlier+=3u)
  if(!memcmp(ai.board,ai.ancestor[earlier],DG_NODES)){
   Vector value=vector_evaluation(DG_EMPTY);value.value[ai.order[(turn+2u)%3u]]-=160;return value;
  }
 memcpy(ai.ancestor[ply],ai.board,DG_NODES);
 uint8_t player = ai.order[turn];
 unsigned count = rank_moves(player,ply,ai.profile.beam,false);
 if(ai.stopped) return zero;
 if(count == 0u) return vector_evaluation(DG_EMPTY);
 Vector best = zero;
 int best_component = -AI_INF;
 for(unsigned i = 0u; i < count; ++i) {
  DgMove move = ai.candidate[ply][i].move;
  make_move(move,player);
  Vector value = maxn(remaining - 1u,ply + 1u,(turn + 1u) % 3u);
  unmake_move(move,player);
  if(ai.stopped) return zero;
  /* No coalition: each turn maximizes that player's own component. */
  if(value.value[player] > best_component) {
   best_component = value.value[player];
   best = value;
  }
 }
 return best;
}

bool dg_ai_choose(const DgGame *game,uint32_t node_budget,DgCancel cancel,void *context,DgMove *move,uint32_t *next_rng,DgAiStats *stats)
{
 if(stats != NULL) memset(stats,0,sizeof(*stats));
 if(game == NULL || move == NULL || next_rng == NULL || !dg_game_valid(game) || game->pos.winner != DG_EMPTY) return false;
 prepare_distances();
 memcpy(ai.board,game->pos.board,sizeof(ai.board));
 memcpy(ai.ancestor[0],game->pos.board,DG_NODES);
 memcpy(ai.order,game->order,sizeof(ai.order));
 ai.players = game->players;
 ai.rules=dg_rules(game);
 ai.root = dg_current(game);
 ai.cancel = cancel;
 ai.context = context;
 if(!dg_ai_profile(ai.players,game->level,&ai.profile))return false;
 ai.budget = node_budget != 0u ? node_budget : ai.profile.node_budget;
 ai.stopped = false;
 ai.root_tactics=game->level==DG_HARD;
 ai.game=game;ai.endgame_evaluation=game->level!=DG_EASY;
 for(uint8_t p=DG_RED;p<=DG_GREEN;p++)ai.endgame[p]=(uint8_t)(dg_goal_count(ai.board,p)>=DG_ENDGAME_GOALS);
 (void)dg_ai_endgame_metrics(ai.rules,ai.board,ai.root,&ai.root_metrics);
 if(ai.root_tactics && ai.endgame[ai.root]){
  ai.profile.depth=(uint8_t)(ai.profile.depth+2u);
  if(!node_budget)ai.budget=24000u;
 }
 memset(&ai.stats,0,sizeof(ai.stats));
 ai.stats.beam = ai.profile.beam;
 ai.stats.endgame=ai.endgame[ai.root];
 unsigned count = rank_moves(ai.root,0u,ai.profile.root_width,true);
 bool success = count != 0u && !ai.stopped;
 uint32_t rng = game->pos.rng;
 DgMove chosen = {0u,0u,DG_STEP,0u};
 if(success) chosen = ai.candidate[0][0].move;
 if(success && ai.candidate[0][0].score>=AI_WIN){
  ai.stats.immediate_win=1;ai.stats.depth=1;
 } else if(success && game->level == DG_EASY) {
  unsigned top = count < 3u ? count : 3u;
  while(top > 1u && ai.candidate[0][0].score - ai.candidate[0][top - 1u].score > 70) --top;
  if(ai.candidate[0][0].score >= AI_WIN) top = 1u;
  ai.stats.beam = (uint8_t)top;
  static const unsigned weight[3] = {4u,2u,1u};
  unsigned total = 0u;
  for(unsigned i = 0u; i < top; ++i) total += weight[i];
  unsigned pick = dg_random(&rng) % total;
  for(unsigned i = 0u; i < top; ++i) {
   if(pick < weight[i]) { chosen = ai.candidate[0][i].move; break; }
   pick -= weight[i];
  }
  ai.stats.depth = 1u;
  ai.stats.nodes = ai.stats.legal_moves;
 } else if(success) {
  unsigned maximum_depth = ai.profile.depth;
  for(unsigned depth = 1u; depth <= maximum_depth; ++depth) {
   int best_value = -AI_INF;
   DgMove iteration_best = chosen;
   for(unsigned i = 0u; i < count; ++i) {
    if(cancelled()) break;
    DgMove candidate = ai.candidate[0][i].move;
    make_move(candidate,ai.root);
    int value;
    unsigned turn = ((unsigned)game->pos.turn + 1u) % ai.players;
    if(ai.players == 2u) value = alphabeta(depth - 1u,1u,turn,best_value,AI_INF);
    else value = maxn(depth - 1u,1u,turn).value[ai.root];
    if(value>-AI_WIN/2 && value<AI_WIN/2)value-=recent_penalty(candidate);
    unmake_move(candidate,ai.root);
    if(ai.stopped) break;
    if(value > best_value) { best_value = value; iteration_best = candidate; }
   }
   if(ai.stopped) break;
   chosen = iteration_best;
   ai.stats.depth = (uint8_t)depth;
   if(best_value >= AI_WIN) break;
  }
 }
 if(ai.stats.cancelled != 0u) success = false;
 if(success) {
  /* Revalidate against the unchanged committed board before publication. */
  success = dg_find_move(dg_rules(game),game->pos.board,ai.root,chosen.from,chosen.to,move,NULL);
  if(success) *next_rng = rng;
 }
 if(stats != NULL) *stats = ai.stats;
 return success;
}

#ifdef DG_HOST
bool dg_ai_reference_score(const DgGame *game,const DgMove *move,int *score,DgAiStats *stats)
{
 if(stats!=NULL)memset(stats,0,sizeof *stats);
 if(game==NULL || move==NULL || score==NULL || !dg_game_valid(game) || game->pos.winner)return false;
 DgMove valid;
 if(!dg_find_move(dg_rules(game),game->pos.board,dg_current(game),move->from,move->to,&valid,NULL) || memcmp(move,&valid,sizeof valid))return false;
 prepare_distances();memcpy(ai.board,game->pos.board,DG_NODES);memcpy(ai.ancestor[0],ai.board,DG_NODES);
 memcpy(ai.order,game->order,sizeof ai.order);ai.players=game->players;ai.root=dg_current(game);
 ai.rules=dg_rules(game);
 ai.profile=ai.players==2u?(DgAiProfile){1000000u,5u,AI_ROOT_WIDTH,20u}:(DgAiProfile){1000000u,7u,AI_ROOT_WIDTH,3u};
 ai.budget=ai.profile.node_budget;ai.cancel=NULL;ai.context=NULL;ai.stopped=false;ai.root_tactics=false;
 ai.game=game;ai.endgame_evaluation=true;
 for(uint8_t p=DG_RED;p<=DG_GREEN;p++)ai.endgame[p]=(uint8_t)(dg_goal_count(ai.board,p)>=DG_ENDGAME_GOALS);
 (void)dg_ai_endgame_metrics(ai.rules,ai.board,ai.root,&ai.root_metrics);
 memset(&ai.stats,0,sizeof ai.stats);ai.stats.beam=ai.profile.beam;
 make_move(*move,ai.root);unsigned turn=((unsigned)game->pos.turn+1u)%ai.players;
 int value=ai.players==2u?alphabeta(ai.profile.depth-1u,1u,turn,-AI_INF,AI_INF):maxn(ai.profile.depth-1u,1u,turn).value[ai.root];
 unmake_move(*move,ai.root);
 if(!ai.stopped){*score=value;ai.stats.depth=ai.profile.depth;}
 if(stats!=NULL)*stats=ai.stats;
 return !ai.stopped;
}
#endif
