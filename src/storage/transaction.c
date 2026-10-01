#include "storage.h"
#include <string.h>

typedef struct { int bytes; bool valid; DgArchive archive; } Slot;

static Slot read_slot(const DgStorageIO *io,unsigned slot)
{
 Slot result;uint8_t bytes[DG_SAVE_BYTES];memset(&result,0,sizeof result);
 result.bytes=io->read(io->context,slot,bytes,sizeof bytes);
 if(result.bytes>=0 && (size_t)result.bytes<=sizeof bytes)
  result.valid=dg_decode(&result.archive,bytes,(size_t)result.bytes);
 return result;
}
/* Serial-number arithmetic remains ordered across UINT32_MAX -> 0. */
static bool newer(uint32_t a,uint32_t b)
{
 uint32_t difference=a-b;
 return difference && difference<UINT32_C(0x80000000);
}
static unsigned latest(const Slot slots[2])
{
 if(!slots[0].valid)return 1;
 if(!slots[1].valid)return 0;
 return newer(slots[1].archive.generation,slots[0].archive.generation)?1:0;
}
int dg_storage_load_io(DgArchive *archive,const DgStorageIO *io)
{
 if(!archive || !io || !io->read)return DG_LOAD_IO_ERROR;
 Slot slots[2];slots[0]=read_slot(io,0);slots[1]=read_slot(io,1);
 if(slots[0].valid || slots[1].valid){
  unsigned chosen=latest(slots);
  *archive=slots[chosen].archive;
  return !slots[chosen^1].valid && slots[chosen^1].bytes!=-2?
   DG_LOAD_RECOVERED:DG_LOAD_OK;
 }
 memset(archive,0,sizeof *archive);archive->assist=1;
 if(slots[0].bytes==-2 && slots[1].bytes==-2)return DG_LOAD_ABSENT;
 if((slots[0].bytes<0 && slots[0].bytes!=-2) ||
    (slots[1].bytes<0 && slots[1].bytes!=-2))return DG_LOAD_IO_ERROR;
 return DG_LOAD_INVALID;
}
bool dg_storage_save_io(DgArchive *archive,const DgStorageIO *io)
{
 if(!archive || !io || !io->read || !io->write)return false;
 /* Validate the proposed state before even opening either old record. */
 uint8_t bytes[DG_SAVE_BYTES];
 if(!dg_encode(archive,bytes,sizeof bytes))return false;
 Slot slots[2];slots[0]=read_slot(io,0);slots[1]=read_slot(io,1);
 /* A read failure leaves ownership/freshness uncertain: preserve both files. */
 if((slots[0].bytes<0 && slots[0].bytes!=-2) ||
    (slots[1].bytes<0 && slots[1].bytes!=-2))return false;
 bool present=slots[0].valid || slots[1].valid;
 unsigned newest=present?latest(slots):1;
 unsigned target=present?(newest^1):0;
 DgArchive proposed=*archive;
 proposed.generation=present?slots[newest].archive.generation+1:1;
 size_t size=dg_encode(&proposed,bytes,sizeof bytes);
 if(!size || !io->write(io->context,target,bytes,size))return false;
 uint8_t verified[DG_SAVE_BYTES];
 int count=io->read(io->context,target,verified,sizeof verified);
 DgArchive decoded;
 if(count<0 || (size_t)count!=size || memcmp(bytes,verified,size) ||
    !dg_decode(&decoded,verified,size))return false;
 archive->generation=proposed.generation;
 return true;
}
