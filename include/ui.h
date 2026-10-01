#ifndef DIAMOND_UI_H
#define DIAMOND_UI_H
#include "ai.h"
#include "storage.h"
enum { DG_PLAYER,DG_SETUP,DG_RULES,DG_GAME };
enum { DG_ENTRY_RESUME,DG_ENTRY_NEW,DG_ENTRY_LEVEL,DG_ENTRY_SLOT,DG_ENTRY_ASSIST };
enum { DG_THINKING_TICKS=40 }; /* 312.5 ms at the existing RTC's 128 Hz. */
enum { DG_MODAL_NONE,DG_MODAL_RESTART,DG_MODAL_RESULT };
enum { DGK_UP=1,DGK_DOWN,DGK_LEFT,DGK_RIGHT,DGK_EXE,DGK_EXIT,DGK_F1,DGK_F2,DGK_F3,DGK_F4,DGK_F5,DGK_F6,DGK_MENU,DGK_OFF };
typedef struct {
 void *context;
 bool (*save)(void *,DgArchive *);
 void (*system)(void *,bool);
} DgHooks;
typedef struct {
 DgArchive archive;
 DgHooks hooks;
 uint32_t new_rng;
 uint8_t screen,parent,modal,players,level,slot,focus;
 uint8_t cursor,selected,zoom,rules_scroll,thinking,thinking_phase,animation,anim_index;
 uint8_t dirty;
 char notice[48];
 DgPath path;
 DgMove pending_move;
 uint32_t pending_rng;
 DgAiStats ai_stats;
} DgApp;
typedef struct {void *context;void (*rect)(void *,int,int,int,int,uint16_t);} DgCanvas;
void dg_app_init(DgApp *app,DgHooks hooks,uint32_t seed);
bool dg_app_key(DgApp *app,int key);
bool dg_checkpoint(DgApp *app);
bool dg_app_cpu(DgApp *app,DgCancel cancel,void *context);
bool dg_app_animation(DgApp *app);
bool dg_app_resumable(const DgApp *app);
bool dg_setup_resume(const DgApp *app);
unsigned dg_entry_count(const DgApp *app);
int dg_entry_action(const DgApp *app,unsigned row);
unsigned dg_entry_row(const DgApp *app,int action);
bool dg_app_to_setup(DgApp *app);
bool dg_app_thinking_tick(DgApp *app,uint32_t elapsed_ticks);
void dg_app_preview(DgApp *app);
void dg_render(const DgApp *app,const DgCanvas *canvas);
void dg_render_thinking(const DgApp *app,const DgCanvas *canvas);
void dg_screen_position(const DgApp *app,int node,int *x,int *y);
#endif
