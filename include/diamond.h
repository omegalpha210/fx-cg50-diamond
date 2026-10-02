#ifndef DIAMOND_H
#define DIAMOND_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define DG_NODES 73
#define DG_PIECES 10
#define DG_MAX_MOVES 720
#define DG_NONE 255
enum { DG_EMPTY, DG_RED, DG_YELLOW, DG_GREEN };
enum { DG_RULES_V1=1, DG_RULES_V2=2 };
#define DG_RULES_API 2
typedef struct { uint8_t revision,players; } DgRules;
#define DG_LEGACY_RULES ((DgRules){DG_RULES_V1,3})
#define DG_RECENT_TURNS 12
typedef struct { uint32_t hash; uint8_t player,from,to; } DgRecent;
/* Persisted v1 IDs. Display order is EASY, NORMAL, HARD. */
enum { DG_EASY=0, DG_HARD=1, DG_NORMAL=2 };
static inline bool dg_level_valid(uint8_t level){return level<=DG_NORMAL;}
enum { DG_STEP, DG_JUMP };
enum { DG_UP, DG_DOWN, DG_LEFT, DG_RIGHT };
/* Generated integer lattice; x=2*q+r, y=r. Region is a six-bit
   camp membership mask: adjacent ten-hole camps share boundary corners. */
typedef struct { int8_t q,r,region,neighbor[6],jump[6],nav[4]; } DgNode;
extern const DgNode dg_nodes[DG_NODES];
extern const uint8_t dg_home[4],dg_goal[4];
extern const uint8_t dg_distance[DG_NODES][DG_NODES];
bool dg_in_camp(int node,uint8_t camp);
typedef struct { uint8_t from,to,type,hops; } DgMove;
typedef struct { uint8_t length,node[DG_NODES+1]; } DgPath;
typedef struct {
 uint8_t board[DG_NODES],turn,winner;
 uint32_t rng,turns;
} DgPosition;
typedef struct {
 uint8_t players,level,human_slot,order[3],rules_revision;
 uint32_t seed,initial_rng;
 DgPosition pos,undo;
 uint8_t undo_valid;
 /* Bounded AI preference only. Never serialized and never a legality rule. */
 uint8_t history_count,history_next;
 DgRecent history[DG_RECENT_TURNS];
} DgGame;
static inline DgRules dg_rules(const DgGame *game){return (DgRules){game->rules_revision,game->players};}
bool dg_rules_valid(DgRules rules);
bool dg_landing_allowed(DgRules rules,uint8_t player,int node);
uint32_t dg_board_hash(const uint8_t *board,uint8_t next_player);
void dg_history_reset(DgGame *game);
uint32_t dg_random(uint32_t *state);
size_t dg_piece_moves(DgRules rules,const uint8_t *board,uint8_t player,uint8_t from,DgMove *out,size_t cap);
size_t dg_generate(DgRules rules,const uint8_t *board,uint8_t player,DgMove *out,size_t cap);
bool dg_find_move(DgRules rules,const uint8_t *board,uint8_t player,int from,int to,DgMove *move,DgPath *path);
bool dg_apply(DgRules rules,uint8_t *board,uint8_t player,const DgMove *move);
uint8_t dg_goal_count(const uint8_t *board,uint8_t player);
bool dg_won(const uint8_t *board,uint8_t player);
bool dg_position_valid(const DgGame *game,const DgPosition *pos,bool undo);
bool dg_game_valid(const DgGame *game);
bool dg_new(DgGame *game,uint8_t players,uint8_t level,uint8_t human_slot,uint32_t seed);
void dg_restart(DgGame *game);
uint8_t dg_current(const DgGame *game);
bool dg_commit(DgGame *game,const DgMove *move);
bool dg_undo(DgGame *game);
int dg_coord(int q,int r);
#endif
