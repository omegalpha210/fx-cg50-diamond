#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Synthetic archives produced by the pre-NORMAL v1 codec. */
static void legacy(void)
{
 static const char *const paths[]={"tests/fixtures/save-v1-easy.bin","tests/fixtures/save-v1-hard.bin"};
 for(uint8_t level=0;level<2;level++){
  uint8_t data[DG_SAVE_BYTES],encoded[DG_SAVE_BYTES];
  FILE *file=fopen(paths[level],"rb");assert(file);
  size_t size=fread(data,1,sizeof data,file);assert(!ferror(file));assert(!fclose(file));
  DgArchive archive;assert(data[25]==level && dg_decode(&archive,data,size));
  assert(archive.game.level==level);
  assert(dg_encode(&archive,encoded,sizeof encoded)==size && !memcmp(data,encoded,size));
  DgApp app;dg_app_init(&app,(DgHooks){0},1);app.archive=archive;
  assert(dg_app_key(&app,DGK_F1) && app.screen==DG_GAME);
  assert(app.level==level && app.archive.game.level==level);
 }
}
typedef struct {uint8_t data[2][DG_SAVE_BYTES];int size[2];} Copies;
static int read_copy(void *context,unsigned slot,uint8_t *data,size_t cap)
{
 Copies *copies=context;assert(slot<2);
 if(copies->size[slot]<0)return -2;
 assert((size_t)copies->size[slot]<=cap);memcpy(data,copies->data[slot],(size_t)copies->size[slot]);
 return copies->size[slot];
}
static bool write_copy(void *context,unsigned slot,const uint8_t *data,size_t size)
{
 Copies *copies=context;assert(slot<2 && size<=DG_SAVE_BYTES);
 memcpy(copies->data[slot],data,size);copies->size[slot]=(int)size;return true;
}
static void normal_roundtrip(void)
{
 for(uint8_t players=2;players<=3;players++)for(uint8_t slot=0;slot<players;slot++){
  DgArchive archive={0},cold;archive.active=1;archive.assist=1;
  assert(dg_new(&archive.game,players,DG_NORMAL,slot,0x54b729));
  Copies copies={{{0}}, {-1,-1}};DgStorageIO io={&copies,read_copy,write_copy};
  for(unsigned turn=0;turn<(unsigned)players*2u;turn++){
   DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&archive.game,0,NULL,NULL,&move,&rng,&stats));
   assert(dg_commit(&archive.game,&move));archive.game.pos.rng=rng;
  }
  while(dg_current(&archive.game)!=DG_RED){
   DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&archive.game,0,NULL,NULL,&move,&rng,&stats));
   assert(dg_commit(&archive.game,&move));archive.game.pos.rng=rng;
  }
  assert(archive.game.undo_valid && dg_storage_save_io(&archive,&io));
  assert(copies.data[0][25]==2);
  assert(dg_storage_load_io(&cold,&io)==DG_LOAD_OK && cold.game.level==DG_NORMAL);
  uint8_t a[DG_SAVE_BYTES],b[DG_SAVE_BYTES];size_t size=dg_encode(&archive,a,sizeof a);
  assert(dg_encode(&cold,b,sizeof b)==size && !memcmp(a,b,size));
  DgGame restored=cold.game;assert(dg_undo(&restored));assert(restored.level==DG_NORMAL);
  uint8_t order[3];memcpy(order,restored.order,sizeof order);dg_restart(&restored);
  assert(restored.level==DG_NORMAL && !memcmp(order,restored.order,sizeof order));
  DgMove next;uint32_t next_rng;DgAiStats next_stats;
  assert(dg_ai_choose(&archive.game,0,NULL,NULL,&next,&next_rng,&next_stats));
  assert(dg_commit(&archive.game,&next));archive.game.pos.rng=next_rng;
  assert(dg_storage_save_io(&archive,&io));copies.data[1][50]^=1;
  assert(dg_storage_load_io(&cold,&io)==DG_LOAD_RECOVERED && cold.game.level==DG_NORMAL);
  assert(dg_encode(&cold,b,sizeof b)==size && !memcmp(a,b,size));
 }
}
static void result_new(void)
{
 for(uint8_t players=2;players<=3;players++){
  DgApp app;dg_app_init(&app,(DgHooks){0},812713);app.players=players;
  assert(dg_app_key(&app,DGK_F6));app.level=DG_NORMAL;app.slot=0;
  assert(dg_app_key(&app,DGK_F6));DgGame *game=&app.archive.game;
  memset(game->pos.board,0,sizeof game->pos.board);
  for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[DG_RED]))game->pos.board[n]=DG_RED;
  for(uint8_t p=DG_YELLOW;p<=DG_GREEN;p++)if(p!=DG_YELLOW || players==3){
   unsigned count=0;
   for(int n=0;n<DG_NODES && count<DG_PIECES;n++)if(!game->pos.board[n] && !dg_in_camp(n,dg_goal[p]) && dg_landing_allowed(dg_rules(game),p,n)){
    game->pos.board[n]=p;count++;
   }
   assert(count==DG_PIECES);
  }
  game->pos.turn=game->human_slot;game->pos.winner=DG_RED;assert(dg_game_valid(game));
  app.archive.active=0;app.modal=DG_MODAL_RESULT;
  assert(dg_app_key(&app,DGK_EXE));
  assert(app.archive.active && !game->pos.winner && !game->pos.turns);
  assert(game->level==DG_NORMAL && app.level==DG_NORMAL && dg_game_valid(game));
 }
}
int main(void)
{
 assert(DG_EASY==0 && DG_HARD==1 && DG_NORMAL==2);
 legacy();normal_roundtrip();result_new();
 puts("difficulty: legacy v1 EASY/HARD exact re-encode; NORMAL cold load, A/B recovery, undo/restart/order and result NEW PASS");
 return 0;
}
