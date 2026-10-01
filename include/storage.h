#ifndef DIAMOND_STORAGE_H
#define DIAMOND_STORAGE_H
#include "diamond.h"
#define DG_SAVE_BYTES 256
typedef struct { uint8_t assist,active; uint32_t generation; DgGame game; } DgArchive;
typedef struct {
 void *context;
 /* read: -2 absent, -1 error, otherwise exact size; never exceed buffer cap. */
 int (*read)(void *,unsigned,uint8_t *,size_t);
 bool (*write)(void *,unsigned,const uint8_t *,size_t);
} DgStorageIO;
enum { DG_LOAD_ABSENT,DG_LOAD_OK,DG_LOAD_RECOVERED,DG_LOAD_INVALID,DG_LOAD_IO_ERROR };
size_t dg_encode(const DgArchive *archive,uint8_t *out,size_t cap);
bool dg_decode(DgArchive *archive,const uint8_t *data,size_t size);
int dg_storage_load_io(DgArchive *archive,const DgStorageIO *io);
bool dg_storage_save_io(DgArchive *archive,const DgStorageIO *io);
int dg_storage_load(DgArchive *archive);
bool dg_storage_save(DgArchive *archive);
/* Retry an OS handle retained after a failed close before MENU/OFF. */
bool dg_storage_cleanup(void);
#endif
