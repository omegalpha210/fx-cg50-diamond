#ifndef DIAMOND_AI_H
#define DIAMOND_AI_H
#include "diamond.h"
typedef bool (*DgCancel)(void *context);
typedef struct { uint32_t nodes,tt_hits; uint16_t legal_moves; uint8_t depth,beam,cancelled; } DgAiStats;
typedef struct { uint32_t node_budget;uint8_t depth,root_width,beam; } DgAiProfile;
bool dg_ai_profile(uint8_t players,uint8_t level,DgAiProfile *profile);
#ifdef DG_HOST
/* Host audit only: score one legal forced root move at a larger fixed horizon.
   The returned utility is the current player's 2P score or 3P component. */
bool dg_ai_reference_score(const DgGame *game,const DgMove *move,int *score,DgAiStats *stats);
#endif
/* Search does not mutate game. EASY consumes returned RNG only on commit. */
bool dg_ai_choose(const DgGame *game,uint32_t node_budget,DgCancel cancel,void *context,DgMove *move,uint32_t *next_rng,DgAiStats *stats);
size_t dg_ai_workspace_bytes(void);
int dg_ai_evaluate(const uint8_t *board,uint8_t player);
#endif
