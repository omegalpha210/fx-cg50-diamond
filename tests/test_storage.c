#define _POSIX_C_SOURCE 200809L
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif
#include "storage.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned checks;
#define CHECK(expression) do { checks++;if(!(expression)){ \
 fprintf(stderr,"storage: %s failed at %s:%d\n",#expression,__FILE__,__LINE__); \
 exit(1); } } while(0)

typedef struct {
 uint8_t data[2][DG_SAVE_BYTES];
 int size[2];
 unsigned reads,writes,last_slot,fail_read_at;
 int read_error_slot;
 bool fail_write,cut_write,corrupt_write;
} Memory;
static void empty(Memory *m)
{
 memset(m,0,sizeof *m);m->size[0]=m->size[1]=-2;m->read_error_slot=-1;
}
static int memory_read(void *context,unsigned slot,uint8_t *out,size_t cap)
{
 Memory *m=context;m->reads++;
 if(slot>1 || m->reads==m->fail_read_at || (int)slot==m->read_error_slot)return -1;
 int size=m->size[slot];
 if(size>=0 && (size_t)size<=cap)memcpy(out,m->data[slot],(size_t)size);
 return size;
}
static bool memory_write(void *context,unsigned slot,const uint8_t *data,size_t size)
{
 Memory *m=context;m->writes++;m->last_slot=slot;
 if(slot>1 || size>DG_SAVE_BYTES || m->fail_write)return false;
 size_t amount=m->cut_write?size/2:size;
 memcpy(m->data[slot],data,amount);m->size[slot]=(int)amount;
 if(m->corrupt_write)m->data[slot][amount-1]^=1;
 return !m->cut_write;
}
static DgStorageIO memory_io(Memory *memory)
{
 DgStorageIO io={memory,memory_read,memory_write};return io;
}
static DgArchive initial(uint8_t players,uint8_t slot)
{
 DgArchive result;memset(&result,0,sizeof result);result.assist=1;result.active=1;
 CHECK(dg_new(&result.game,players,DG_HARD,slot,UINT32_C(0x54b729)));
 return result;
}
static void advance(DgGame *game)
{
 DgMove moves[DG_MAX_MOVES];
 size_t count=dg_generate(dg_rules(game),game->pos.board,dg_current(game),moves,DG_MAX_MOVES);
 CHECK(count>0 && count<=DG_MAX_MOVES);
 unsigned index=dg_random(&game->pos.rng)%(unsigned)count;
 CHECK(dg_commit(game,&moves[index]));
}
static bool same(const DgArchive *a,const DgArchive *b)
{
 uint8_t aa[DG_SAVE_BYTES],bb[DG_SAVE_BYTES];
 size_t sa=dg_encode(a,aa,sizeof aa),sb=dg_encode(b,bb,sizeof bb);
 return sa && sa==sb && !memcmp(aa,bb,sa);
}
static void repair_crc(uint8_t *data,size_t size)
{
 memset(data+16,0,4);uint32_t crc=UINT32_MAX;
 for(size_t i=0;i<size;i++){
  crc^=data[i];
  for(unsigned bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1)?UINT32_C(0xedb88320):0);
 }
 crc=~crc;for(unsigned i=0;i<4;i++)data[16+i]=(uint8_t)(crc>>(i*8));
}
static void reject_change(const uint8_t *original,size_t size,size_t offset,uint8_t value)
{
 uint8_t changed[DG_SAVE_BYTES];memcpy(changed,original,size);changed[offset]=value;
 repair_crc(changed,size);
 DgArchive result=initial(2,0),before=result;
 CHECK(!dg_decode(&result,changed,size));CHECK(same(&result,&before));
}
static void test_codec(void)
{
 DgArchive source=initial(2,0),decoded;
 source.generation=UINT32_C(0xfefdfcfb);
 uint8_t bytes[DG_SAVE_BYTES];size_t size=dg_encode(&source,bytes,sizeof bytes);
 CHECK(size==124);CHECK(bytes[12]==0xfb && bytes[15]==0xfe);
 CHECK(dg_decode(&decoded,bytes,size));CHECK(same(&source,&decoded));
 CHECK(decoded.game.order[2]==DG_NONE);
 CHECK(!dg_encode(&source,bytes,size-1));
 for(size_t i=0;i<size;i++){
  uint8_t prior=bytes[i];bytes[i]^=1;
  CHECK(!dg_decode(&decoded,bytes,size));bytes[i]=prior;
 }
 for(size_t length=0;length<size;length++)CHECK(!dg_decode(&decoded,bytes,length));
 bytes[size]=0;CHECK(!dg_decode(&decoded,bytes,size+1));
 CHECK(!dg_decode(&decoded,bytes,DG_SAVE_BYTES+1));
 CHECK(!dg_decode(NULL,bytes,size));CHECK(!dg_decode(&decoded,NULL,size));
 const size_t offsets[]={0,8,9,10,20,21,22,23,24,25,26,27,28,29,30,31,40,113,114,115};
 const uint8_t values[]={0,3,1,0,2,2,1,1,1,3,2,DG_GREEN,DG_RED,0,1,1,4,2,DG_GREEN,1};
 for(size_t i=0;i<sizeof offsets/sizeof offsets[0];i++)reject_change(bytes,size,offsets[i],values[i]);
 for(size_t offset=32;offset<=36;offset+=4){
  uint8_t changed[DG_SAVE_BYTES];memcpy(changed,bytes,size);memset(changed+offset,0,4);
  repair_crc(changed,size);CHECK(!dg_decode(&decoded,changed,size));
 }
 uint8_t changed[DG_SAVE_BYTES];memcpy(changed,bytes,size);memset(changed+116,0,4);
 repair_crc(changed,size);CHECK(!dg_decode(&decoded,changed,size));
 for(unsigned i=0;i<DG_NODES;i++)if(source.game.pos.board[i]==DG_RED){
  reject_change(bytes,size,40+i,DG_EMPTY);break;
 }
 advance(&source.game);size=dg_encode(&source,bytes,sizeof bytes);
 CHECK(source.game.undo_valid && size==208);
 CHECK(dg_decode(&decoded,bytes,size));CHECK(same(&source,&decoded));
 CHECK(!memcmp(&source.game.undo,&decoded.game.undo,sizeof source.game.undo));
 reject_change(bytes,size,22,0);reject_change(bytes,size,124+73,1);
 reject_change(bytes,size,124+74,DG_RED);reject_change(bytes,size,124+75,1);
 memcpy(changed,bytes,size);memcpy(changed+124+80,bytes+40+80,4);
 repair_crc(changed,size);CHECK(!dg_decode(&decoded,changed,size));
 source.assist=0;source.active=0;size=dg_encode(&source,bytes,sizeof bytes);
 CHECK(size==24);CHECK(dg_decode(&decoded,bytes,size));
 CHECK(!decoded.active && !decoded.assist && !decoded.game.undo_valid);
 reject_change(bytes,size,22,1);
 source.assist=2;CHECK(!dg_encode(&source,bytes,sizeof bytes));
 source.assist=1;source.active=2;CHECK(!dg_encode(&source,bytes,sizeof bytes));
 /* Completion remains a valid engine position, but never an active resume. */
 source=initial(2,0);
 memset(source.game.pos.board,0,sizeof source.game.pos.board);
 unsigned green=0;
 for(unsigned node=0;node<DG_NODES;node++){
  if(dg_in_camp((int)node,dg_goal[DG_RED]))source.game.pos.board[node]=DG_RED;
 }
 for(unsigned node=0;node<DG_NODES && green<DG_PIECES;node++){
  if(!source.game.pos.board[node] && !dg_in_camp((int)node,dg_goal[DG_GREEN]) && dg_landing_allowed(dg_rules(&source.game),DG_GREEN,(int)node)){
   source.game.pos.board[node]=DG_GREEN;green++;
  }
 }
 source.game.pos.winner=DG_RED;
 CHECK(dg_game_valid(&source.game));CHECK(!dg_encode(&source,bytes,sizeof bytes));
 source.active=0;CHECK(dg_encode(&source,bytes,sizeof bytes)==24);
}
static void test_transactions(void)
{
 Memory memory;empty(&memory);DgStorageIO io=memory_io(&memory);DgArchive loaded;
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_ABSENT);
 CHECK(loaded.assist==1 && !loaded.active);
 DgArchive first=initial(3,0);
 CHECK(dg_storage_save_io(&first,&io));CHECK(first.generation==1 && memory.last_slot==0);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&first,&loaded));
 DgArchive second=first;advance(&second.game);
 CHECK(dg_storage_save_io(&second,&io));CHECK(second.generation==2 && memory.last_slot==1);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&second,&loaded));
 memory.data[1][50]^=1;
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(same(&first,&loaded));
 DgArchive replacement=initial(2,1);
 memory.fail_write=true;
 CHECK(!dg_storage_save_io(&replacement,&io));CHECK(replacement.generation==0);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(same(&first,&loaded));
 memory.fail_write=false;memory.cut_write=true;
 CHECK(!dg_storage_save_io(&replacement,&io));CHECK(memory.last_slot==1);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(same(&first,&loaded));
 memory.cut_write=false;memory.corrupt_write=true;
 CHECK(!dg_storage_save_io(&replacement,&io));
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(same(&first,&loaded));
 memory.corrupt_write=false;memory.read_error_slot=1;
 unsigned writes=memory.writes;
 CHECK(!dg_storage_save_io(&replacement,&io));CHECK(memory.writes==writes);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(same(&first,&loaded));
 memory.read_error_slot=-1;
 CHECK(dg_storage_save_io(&replacement,&io));CHECK(replacement.generation==2);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&replacement,&loaded));
 DgArchive tombstone=replacement;tombstone.active=0;tombstone.assist=0;
 CHECK(dg_storage_save_io(&tombstone,&io));CHECK(memory.size[memory.last_slot]==24);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);
 CHECK(!loaded.active && !loaded.assist && loaded.generation==3);
 /* A failed tombstone never destroys the newest unfinished game. */
 CHECK(dg_storage_save_io(&replacement,&io));
 memory.cut_write=true;CHECK(!dg_storage_save_io(&tombstone,&io));
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED);CHECK(loaded.active);
 memory.cut_write=false;
 /* Read-back I/O failure reports failure while retaining the old copy. */
 unsigned protected_slot=memory.last_slot^1;
 memory.fail_read_at=memory.reads+3;
 CHECK(!dg_storage_save_io(&replacement,&io));
 CHECK(dg_decode(&loaded,memory.data[protected_slot],(size_t)memory.size[protected_slot]));
 memory.fail_read_at=0;
 /* Invalid input must not touch disk. */
 DgArchive invalid=replacement;invalid.game.pos.rng=0;writes=memory.writes;
 unsigned reads=memory.reads;CHECK(!dg_storage_save_io(&invalid,&io));
 CHECK(memory.writes==writes && memory.reads==reads);
 empty(&memory);memory.size[0]=5;memory.size[1]=DG_SAVE_BYTES+1;
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_INVALID);CHECK(!loaded.active && loaded.assist);
 memory.read_error_slot=0;CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_IO_ERROR);
 empty(&memory);memory.cut_write=true;
 CHECK(!dg_storage_save_io(&first,&io));CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_INVALID);
 memory.cut_write=false;CHECK(dg_storage_save_io(&first,&io));CHECK(first.generation==1);
 CHECK(!dg_storage_save_io(NULL,&io));CHECK(!dg_storage_save_io(&first,NULL));
 CHECK(dg_storage_load_io(NULL,&io)==DG_LOAD_IO_ERROR);
 CHECK(dg_storage_load_io(&loaded,NULL)==DG_LOAD_IO_ERROR);
}
static void test_generation_wrap(void)
{
 Memory memory;empty(&memory);DgStorageIO io=memory_io(&memory);
 DgArchive old=initial(2,0);old.generation=UINT32_MAX;
 memory.size[0]=(int)dg_encode(&old,memory.data[0],DG_SAVE_BYTES);
 DgArchive next=old;next.assist=0;
 CHECK(dg_storage_save_io(&next,&io));CHECK(next.generation==0 && memory.last_slot==1);
 DgArchive loaded;CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&next,&loaded));
 CHECK(dg_storage_save_io(&next,&io));CHECK(next.generation==1 && memory.last_slot==0);
 /* Equal generations consistently select A. */
 old.generation=next.generation;
 memory.size[1]=(int)dg_encode(&old,memory.data[1],DG_SAVE_BYTES);
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&next,&loaded));
}
static void test_reachable_roundtrips(void)
{
 for(uint8_t players=2;players<=3;players++)for(uint8_t slot=0;slot<players;slot++){
  Memory memory;empty(&memory);DgStorageIO io=memory_io(&memory);
  DgArchive original=initial(players,slot),loaded;
  for(unsigned turn=0;turn<240 && !original.game.pos.winner;turn++){
   CHECK(dg_storage_save_io(&original,&io));
   CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK);CHECK(same(&original,&loaded));
   CHECK(!memcmp(&original.game.pos,&loaded.game.pos,sizeof original.game.pos));
   if(original.game.undo_valid){
    CHECK(!memcmp(&original.game.undo,&loaded.game.undo,sizeof original.game.undo));
    if(dg_current(&loaded.game)==DG_RED){
     DgGame undo_original=original.game,undo_loaded=loaded.game;
     CHECK(dg_undo(&undo_original));CHECK(dg_undo(&undo_loaded));
     CHECK(!memcmp(&undo_original,&undo_loaded,sizeof undo_original));
     CHECK(!dg_undo(&undo_loaded));
    }
   }
   advance(&original.game);
  }
 }
}
static void test_native_host(void)
{
 int cwd=open(".",O_RDONLY);CHECK(cwd>=0);
 char temporary[]="/tmp/diamond-storage-XXXXXX";CHECK(mkdtemp(temporary)!=NULL);
 CHECK(chdir(temporary)==0);
 DgArchive archive=initial(3,2),loaded;
 CHECK(dg_storage_load(&loaded)==DG_LOAD_ABSENT);
 CHECK(dg_storage_save(&archive));CHECK(dg_storage_save(&archive));
 CHECK(dg_storage_load(&loaded)==DG_LOAD_OK);CHECK(same(&archive,&loaded));
 int fd=open("DGSTATEB.dat",O_WRONLY|O_TRUNC);CHECK(fd>=0);
 CHECK(write(fd,"bad",3)==3);CHECK(close(fd)==0);
 CHECK(dg_storage_load(&loaded)==DG_LOAD_RECOVERED);CHECK(loaded.generation==1);
 archive.active=0;CHECK(dg_storage_save(&archive));
 CHECK(dg_storage_load(&loaded)==DG_LOAD_OK);CHECK(!loaded.active);
 CHECK(unlink("DGSTATEA.dat")==0);CHECK(unlink("DGSTATEB.dat")==0);
 CHECK(fchdir(cwd)==0);CHECK(close(cwd)==0);CHECK(rmdir(temporary)==0);
}
static void test_rule_revisions(void)
{
 DgArchive old=initial(3,0),loaded;old.game.rules_revision=DG_RULES_V1;
 /* This pre-correction placement is forbidden in V2, valid in V1. */
 CHECK(!old.game.pos.board[40]);old.game.pos.board[72]=DG_EMPTY;old.game.pos.board[40]=DG_RED;
 CHECK(dg_game_valid(&old.game));DgArchive strict=old;strict.game.rules_revision=DG_RULES_V2;
 CHECK(!dg_game_valid(&strict.game));
 uint8_t bytes[DG_SAVE_BYTES],again[DG_SAVE_BYTES];size_t size=dg_encode(&old,bytes,sizeof bytes);
 CHECK(size==124 && bytes[8]==1 && bytes[30]==0);CHECK(dg_decode(&loaded,bytes,size));
 CHECK(loaded.game.rules_revision==DG_RULES_V1 && !memcmp(&loaded.game.pos,&old.game.pos,sizeof old.game.pos));
 CHECK(loaded.game.level==old.game.level && !memcmp(loaded.game.order,old.game.order,3));
 CHECK(dg_encode(&loaded,again,sizeof again)==size && !memcmp(bytes,again,size));
 advance(&loaded.game);CHECK(loaded.game.history_count==1);size=dg_encode(&loaded,bytes,sizeof bytes);
 DgArchive resumed;CHECK(dg_decode(&resumed,bytes,size));CHECK(!resumed.game.history_count && !resumed.game.history_next);
 CHECK(same(&loaded,&resumed));dg_restart(&resumed.game);CHECK(resumed.game.rules_revision==DG_RULES_V1);
 CHECK(dg_new(&resumed.game,3,DG_NORMAL,2,71));CHECK(resumed.game.rules_revision==DG_RULES_V2);
 size=dg_encode(&resumed,bytes,sizeof bytes);CHECK(size==124 && bytes[8]==2 && bytes[30]==DG_RULES_V2);
 CHECK(dg_decode(&loaded,bytes,size) && loaded.game.rules_revision==DG_RULES_V2);
 Memory memory;empty(&memory);DgStorageIO io=memory_io(&memory);
 CHECK(dg_storage_save_io(&old,&io));CHECK(dg_storage_save_io(&resumed,&io));
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_OK && loaded.game.rules_revision==DG_RULES_V2);
 memory.data[memory.last_slot][50]^=1;
 CHECK(dg_storage_load_io(&loaded,&io)==DG_LOAD_RECOVERED && loaded.game.rules_revision==DG_RULES_V1);
}
int main(void)
{
 test_codec();test_transactions();test_generation_wrap();test_reachable_roundtrips();test_native_host();test_rule_revisions();
 printf("storage: %u checks passed; A/B faults, undo, wrap and native host files\n",checks);
 return 0;
}
