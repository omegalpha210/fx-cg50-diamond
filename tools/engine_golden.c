/* UI freeze fingerprint. Hash defined fields/serialized bytes, never padding. */
#include "ai.h"
#include "storage.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint64_t hash;
static void begin(void){hash=UINT64_C(14695981039346656037);}
static void byte(uint8_t value){hash^=value;hash*=UINT64_C(1099511628211);}
static void bytes(const uint8_t *data,size_t size){for(size_t i=0;i<size;i++)byte(data[i]);}
static void word(uint32_t value){for(unsigned i=0;i<4;i++)byte((uint8_t)(value>>(8*i)));}
static void position(const DgPosition *pos)
{bytes(pos->board,DG_NODES);byte(pos->turn);byte(pos->winner);word(pos->rng);word(pos->turns);}
static void relocate(DgGame *game,uint8_t who,int destination)
{
 assert(destination>=0 && !game->pos.board[destination]);
 for(int n=0;n<DG_NODES;n++)if(game->pos.board[n]==who){game->pos.board[n]=0;game->pos.board[destination]=who;return;}
 assert(false);
}
static void fixture(DgGame game,const char *name)
{
 DgMove moves[DG_MAX_MOVES];DgPath path;uint8_t player=dg_current(&game);
 size_t count=dg_generate(dg_rules(&game),game.pos.board,player,moves,DG_MAX_MOVES);assert(count);
 begin();position(&game.pos);
 for(size_t i=0;i<count;i++){
  DgMove found;assert(dg_find_move(dg_rules(&game),game.pos.board,player,moves[i].from,moves[i].to,&found,&path));
  bytes((const uint8_t *)&found,sizeof found);byte(path.length);bytes(path.node,path.length);
  assert(found.from!=found.to);
 }
 printf("%uP %s moves=%lu hash=%016llx\n",(unsigned)game.players,name,(unsigned long)count,(unsigned long long)hash);
 for(uint8_t level=DG_EASY;level<=DG_HARD;level++){
  game.level=level;DgGame original=game;DgMove move;uint32_t rng;DgAiStats stats;
  assert(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));assert(!memcmp(&game,&original,sizeof game));
  printf("%uP %s %s move=%u,%u,%u,%u rng=%lu nodes=%lu depth=%u beam=%u tt=%lu\n",
   (unsigned)game.players,name,level==DG_EASY?"EASY":"HARD",(unsigned)move.from,(unsigned)move.to,
   (unsigned)move.type,(unsigned)move.hops,(unsigned long)rng,(unsigned long)stats.nodes,
   (unsigned)stats.depth,(unsigned)stats.beam,(unsigned long)stats.tt_hits);
 }
 /* Undo is available when CPU replies return the turn to HUMAN. */
 if(player==DG_RED){
  DgPosition initial=game.pos;assert(dg_commit(&game,&moves[0]));
  for(unsigned reply=1;reply<game.players;reply++){
   DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));
   assert(dg_commit(&game,&move));game.pos.rng=rng;
  }
  assert(dg_undo(&game));
  assert(!memcmp(&game.pos,&initial,sizeof initial));begin();position(&game.pos);
  printf("%uP %s undo=%016llx\n",(unsigned)game.players,name,(unsigned long long)hash);
 }
 DgArchive archive={0},decoded={0};archive.assist=1;archive.active=1;archive.generation=17;archive.game=game;
 uint8_t encoded[DG_SAVE_BYTES],roundtrip[DG_SAVE_BYTES];
 size_t size=dg_encode(&archive,encoded,sizeof encoded);assert(size && dg_decode(&decoded,encoded,size));
 assert(dg_encode(&decoded,roundtrip,sizeof roundtrip)==size && !memcmp(encoded,roundtrip,size));
 begin();bytes(encoded,size);printf("%uP %s save=%lu,%016llx\n",(unsigned)game.players,name,(unsigned long)size,(unsigned long long)hash);
}
int main(void)
{
 begin();
 for(unsigned n=0;n<DG_NODES;n++){
  byte((uint8_t)dg_nodes[n].q);byte((uint8_t)dg_nodes[n].r);byte((uint8_t)dg_nodes[n].region);
  for(unsigned d=0;d<6;d++){byte((uint8_t)dg_nodes[n].neighbor[d]);byte((uint8_t)dg_nodes[n].jump[d]);}
  for(unsigned d=0;d<4;d++)byte((uint8_t)dg_nodes[n].nav[d]);
  bytes(dg_distance[n],DG_NODES);
 }
 bytes(dg_home,4);bytes(dg_goal,4);printf("topology=%016llx\n",(unsigned long long)hash);
 for(uint8_t players=2;players<=3;players++){
  DgGame game;assert(dg_new(&game,players,DG_EASY,0,123456));fixture(game,"opening");
  for(unsigned ply=0;ply<(unsigned)players*12;ply++){
   DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));assert(dg_commit(&game,&move));game.pos.rng=rng;
  }
  assert(dg_current(&game)==DG_RED);fixture(game,"reachable_midgame");
  assert(dg_new(&game,players,DG_EASY,0,123456));
  relocate(&game,DG_RED,dg_coord(0,0));relocate(&game,DG_GREEN,dg_coord(1,0));relocate(&game,DG_GREEN,dg_coord(2,-1));
  DgMove jump;DgPath path;assert(dg_find_move(dg_rules(&game),game.pos.board,DG_RED,dg_coord(0,0),dg_coord(2,-2),&jump,&path));
  assert(jump.type==DG_JUMP && jump.hops>=2);
  assert(!dg_find_move(dg_rules(&game),game.pos.board,DG_RED,dg_coord(0,0),dg_coord(0,0),NULL,NULL));
  assert(dg_game_valid(&game));fixture(game,"multi_jump");
 }
 return 0;
}
