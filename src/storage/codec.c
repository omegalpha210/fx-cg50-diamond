#include "storage.h"
#include <string.h>

enum { HEADER=20, FLAGS=24, CONFIG=40, POSITION=84 };
static const uint8_t magic[8]={'D','G','S','A','V','E','0','1'};

static uint32_t get32(const uint8_t *p)
{
 return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static void put32(uint8_t *p,uint32_t n)
{
 for(unsigned i=0;i<4;i++)p[i]=(uint8_t)(n>>(8*i));
}
/* CRC-32/ISO-HDLC covers the whole record with its CRC field zeroed. */
static uint32_t checksum(const uint8_t *p,size_t size)
{
 uint32_t crc=UINT32_MAX;
 for(size_t i=0;i<size;i++){
  crc^=(i>=16 && i<HEADER)?0:p[i];
  for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^(UINT32_C(0xedb88320)&(0-(crc&1)));
 }
 return ~crc;
}
static void put_position(uint8_t *p,const DgPosition *position)
{
 memcpy(p,position->board,DG_NODES);
 p[73]=position->turn;p[74]=position->winner;p[75]=0;
 put32(p+76,position->rng);put32(p+80,position->turns);
}
static bool get_position(DgPosition *position,const uint8_t *p)
{
 if(p[75])return false;
 memcpy(position->board,p,DG_NODES);
 position->turn=p[73];position->winner=p[74];
 position->rng=get32(p+76);position->turns=get32(p+80);
 return true;
}
size_t dg_encode(const DgArchive *archive,uint8_t *out,size_t cap)
{
 if(!archive || !out || archive->assist>1 || archive->active>1)return 0;
 size_t size=FLAGS;
 if(archive->active){
  if(archive->game.undo_valid>1 || !dg_game_valid(&archive->game) ||
     archive->game.pos.winner!=DG_EMPTY)return 0;
  size=CONFIG+POSITION+(archive->game.undo_valid?POSITION:0);
 }
 if(size>cap || size>DG_SAVE_BYTES)return 0;
 memset(out,0,size);memcpy(out,magic,sizeof magic);
 out[8]=archive->active && archive->game.rules_revision==DG_RULES_V1?1:2;
 out[10]=(uint8_t)size;out[11]=(uint8_t)(size>>8);
 put32(out+12,archive->generation);
 out[20]=archive->assist;out[21]=archive->active;
 if(archive->active){
  const DgGame *g=&archive->game;
  out[22]=g->undo_valid;
  out[24]=g->players;out[25]=g->level;out[26]=g->human_slot;
  memcpy(out+27,g->order,3);
  if(out[8]==2)out[30]=g->rules_revision;
  put32(out+32,g->seed);put32(out+36,g->initial_rng);
  put_position(out+CONFIG,&g->pos);
  if(g->undo_valid)put_position(out+CONFIG+POSITION,&g->undo);
 }
 put32(out+16,checksum(out,size));
 return size;
}
bool dg_decode(DgArchive *archive,const uint8_t *data,size_t size)
{
 if(!archive || !data || size<FLAGS || size>DG_SAVE_BYTES ||
    memcmp(data,magic,sizeof magic) || (data[8]!=1 && data[8]!=2) || data[9] ||
    ((size_t)data[10]|((size_t)data[11]<<8))!=size ||
    get32(data+16)!=checksum(data,size) ||
    data[20]>1 || data[21]>1 || data[22]>1 || data[23])return false;
 DgArchive decoded;memset(&decoded,0,sizeof decoded);
 decoded.assist=data[20];decoded.active=data[21];decoded.generation=get32(data+12);
 if(!decoded.active){
  if(size!=FLAGS || data[22])return false;
 }else{
  size_t expected=CONFIG+POSITION+(data[22]?POSITION:0);
  if(size!=expected || (data[8]==1?data[30]!=0:data[30]!=DG_RULES_V2) || data[31])return false;
  DgGame *g=&decoded.game;
  g->players=data[24];g->level=data[25];g->human_slot=data[26];
  g->rules_revision=data[8]==1?DG_RULES_V1:data[30];
  memcpy(g->order,data+27,3);
  g->seed=get32(data+32);g->initial_rng=get32(data+36);
  g->undo_valid=data[22];
  if(!get_position(&g->pos,data+CONFIG) ||
     (g->undo_valid && !get_position(&g->undo,data+CONFIG+POSITION)) ||
     !dg_game_valid(g) || g->pos.winner!=DG_EMPTY)return false;
 }
 *archive=decoded;
 return true;
}
