#ifdef DG_HOST
#define _POSIX_C_SOURCE 200809L
#endif
#include "storage.h"
#include <limits.h>
#ifdef DG_HOST
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#else
#include <gint/bfile.h>
#include <gint/gint.h>
#endif

static const char *const names[2]={"DGSTATEA.dat","DGSTATEB.dat"};
#ifdef DG_HOST
static int file_read(void *context,unsigned slot,uint8_t *out,size_t cap)
{
 (void)context;if(slot>1 || cap>INT_MAX)return -1;
 int fd=open(names[slot],O_RDONLY);
 if(fd<0)return errno==ENOENT?-2:-1;
 struct stat st;int result=-1;
 if(fstat(fd,&st)==0 && st.st_size>=0 && st.st_size<=INT_MAX){
  size_t size=(size_t)st.st_size;
  if(size>cap)result=(int)size;
  else{
   size_t total=0;
   while(total<size){
    ssize_t n=read(fd,out+total,size-total);
    if(n<0 && errno==EINTR)continue;
    if(n<=0)break;
    total+=(size_t)n;
   }
   if(total==size)result=(int)size;
  }
 }
 if(close(fd)<0)result=-1;
 return result;
}
static bool file_write(void *context,unsigned slot,const uint8_t *data,size_t size)
{
 (void)context;if(slot>1 || size>DG_SAVE_BYTES)return false;
 int fd=open(names[slot],O_WRONLY|O_CREAT|O_TRUNC,0600);
 if(fd<0)return false;
 size_t total=0;
 while(total<size){
  ssize_t n=write(fd,data+total,size-total);
  if(n<0 && errno==EINTR)continue;
  if(n<=0)break;
  total+=(size_t)n;
 }
 bool ok=total==size && fsync(fd)==0;
 if(close(fd)<0)ok=false;
 if(ok){
  /* Persist creation of a previously absent A/B directory entry, too. */
  int directory_fd=open(".",O_RDONLY);
  if(directory_fd<0)return false;
  if(fsync(directory_fd)<0)ok=false;
  if(close(directory_fd)<0)ok=false;
 }
 return ok;
}
#else
/* Only one OS handle is held; a failed close blocks subsequent opens until
   its cleanup succeeds. All these helpers run in the OS world. */
static int pending_close=-1;
static bool cleanup(void)
{
 if(pending_close<0)return true;
 if(BFile_Close(pending_close)<0)return false;
 pending_close=-1;return true;
}
static bool file_close(int fd)
{
 if(BFile_Close(fd)>=0)return true;
 pending_close=fd;return false;
}
static void native_path(uint16_t path[32],unsigned slot)
{
 const char *prefix="\\\\fls0\\";size_t n=0;
 while(*prefix)path[n++]=(uint8_t)*prefix++;
 const char *name=names[slot];
 do{path[n++]=(uint8_t)*name;}while(*name++);
}
static int file_read(void *context,unsigned slot,uint8_t *out,size_t cap)
{
 (void)context;if(slot>1 || cap>INT_MAX || !cleanup())return -1;
 uint16_t path[32];native_path(path,slot);
 int fd=BFile_Open(path,BFile_ReadOnly);
 if(fd<0)return fd==BFile_EntryNotFound?-2:-1;
 int size=BFile_Size(fd),result=-1;
 if(size>=0){
  if((size_t)size>cap)result=size;
  /* Fugue can zero-fill reads past EOF; never request more than Size(). */
  else if(!size || BFile_Read(fd,out,size,0)==size)result=size;
 }
 if(!file_close(fd))result=-1;
 return result;
}
static bool file_write(void *context,unsigned slot,const uint8_t *data,size_t size)
{
 (void)context;
 if(slot>1 || size>DG_SAVE_BYTES || (size&1) || !cleanup())return false;
 uint16_t path[32];native_path(path,slot);
 /* The transaction layer only selects the older/invalid copy for replacement. */
 int removed=BFile_Remove(path);
 if(removed<0 && removed!=BFile_EntryNotFound)return false;
 int length=(int)size;
 if(BFile_Create(path,BFile_File,&length)<0)return false;
 int fd=BFile_Open(path,BFile_ReadWrite);
 if(fd<0)return false;
 bool ok=BFile_Seek(fd,0)>=0 && BFile_Write(fd,data,length)==length;
 if(!file_close(fd))ok=false;
 return ok;
}
#endif
static const DgStorageIO files={NULL,file_read,file_write};
typedef struct { DgArchive *archive; bool writing,cleaning; } Request;
static int dispatch(void *opaque)
{
 Request *r=opaque;
 if(r->cleaning){
#ifdef DG_HOST
  return 1;
#else
  return (int)cleanup();
#endif
 }
 return r->writing?(int)dg_storage_save_io(r->archive,&files):
  dg_storage_load_io(r->archive,&files);
}
static int transaction(Request *request)
{
#ifdef DG_HOST
 return dispatch(request);
#else
 return gint_world_switch(GINT_CALL(dispatch,(void *)request));
#endif
}
int dg_storage_load(DgArchive *archive)
{
 Request request={archive,false,false};return transaction(&request);
}
bool dg_storage_save(DgArchive *archive)
{
 Request request={archive,true,false};return transaction(&request)!=0;
}
bool dg_storage_cleanup(void)
{
 Request request={NULL,false,true};return transaction(&request)!=0;
}
