#include "ui.h"
#include <stdio.h>
#include <string.h>
static void warning(DgApp *app,const char *text)
{snprintf(app->notice,sizeof app->notice,"%s",text);}
static void clear_trails(DgApp *app)
{memset(app->trails,0,sizeof app->trails);}
void dg_app_init(DgApp *app,DgHooks hooks,uint32_t seed)
{
 memset(app,0,sizeof *app);app->hooks=hooks;app->new_rng=seed?seed:1;
 app->archive.assist=1;app->players=3;app->cursor=36;app->selected=DG_NONE;
}
bool dg_checkpoint(DgApp *app)
{
 if(!app->dirty)return true;
 if(app->hooks.save && !app->hooks.save(app->hooks.context,&app->archive)){warning(app,"SAVE FAILED - RETRY EXIT");return false;}
 app->dirty=0;return true;
}
static void setup_from_game(DgApp *app)
{
 app->players=app->archive.game.players;app->level=app->archive.game.level;app->slot=app->archive.game.human_slot;
}
static void enter_game(DgApp *app)
{
 app->screen=DG_GAME;app->modal=DG_MODAL_NONE;app->thinking=app->animation=0;app->selected=DG_NONE;app->path.length=0;app->zoom=0;
 clear_trails(app);
 app->cursor=36;for(int n=0;n<DG_NODES;n++)if(app->archive.game.pos.board[n]==DG_RED){app->cursor=(uint8_t)n;break;}
 setup_from_game(app);
}
bool dg_app_to_setup(DgApp *app)
{
 dg_app_skip_animation(app);app->thinking=0;app->modal=DG_MODAL_NONE;app->selected=DG_NONE;app->path.length=0;
 if(!dg_checkpoint(app))return false;
 setup_from_game(app);app->screen=DG_SETUP;app->focus=0;return true;
}
bool dg_app_thinking_tick(DgApp *app,uint32_t elapsed_ticks)
{
 if(!app->thinking)return false;
 uint8_t phase=(uint8_t)((elapsed_ticks/DG_THINKING_TICKS)%3u);
 if(phase==app->thinking_phase)return false;
 app->thinking_phase=phase;return true;
}
static bool new_game(DgApp *app)
{
 if(!dg_checkpoint(app))return false;
 DgArchive next=app->archive;
 if(!dg_new(&next.game,app->players,app->level,app->slot,dg_random(&app->new_rng)))return false;
 next.active=1;
 /* Preserve the old logical resume until the replacement validates on disk. */
 if(app->hooks.save && !app->hooks.save(app->hooks.context,&next)){warning(app,"NEW GAME SAVE FAILED");return false;}
 app->archive=next;app->dirty=0;enter_game(app);return true;
}
void dg_app_preview(DgApp *app)
{
 if(app->animation || app->thinking)return;
 app->path.length=0;
 if(app->screen==DG_GAME && !app->animation && app->selected!=DG_NONE && app->archive.assist)
  (void)dg_find_move(app->archive.game.pos.board,DG_RED,app->selected,app->cursor,NULL,&app->path);
}
static void finished(DgApp *app)
{
 app->dirty=1;app->selected=DG_NONE;app->path.length=0;
 if(app->archive.game.pos.winner){app->archive.active=0;app->modal=DG_MODAL_RESULT;(void)dg_checkpoint(app);}
}
static void parent_screen(DgApp *app,uint8_t screen)
{app->parent=app->screen;app->screen=screen;app->rules_scroll=0;}
static uint8_t adjust(uint8_t v,int direction,uint8_t max)
{return direction<0?(v?v-1:0):(v<max?v+1:max);}
static uint8_t adjust_level(uint8_t value,int direction)
{
 /* Stable save IDs are independent of the clamped on-screen order. */
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 uint8_t at=0;
 for(uint8_t i=0;i<3;i++)if(levels[i]==value)at=i;
 return levels[adjust(at,direction,2)];
}
bool dg_app_key(DgApp *app,int key)
{
 if(key==DGK_MENU || key==DGK_OFF){
  dg_app_skip_animation(app);app->thinking=0;app->path.length=0;
  if(dg_checkpoint(app) && app->hooks.system)app->hooks.system(app->hooks.context,key==DGK_OFF);
  return true;
 }
 app->notice[0]=0;
 if(app->screen==DG_RULES){
  if(key==DGK_EXIT)app->screen=app->parent;
  else if(key==DGK_UP)app->rules_scroll=adjust(app->rules_scroll,-1,12);
  else if(key==DGK_DOWN)app->rules_scroll=adjust(app->rules_scroll,1,12);
  return true;
 }
 if(app->screen==DG_PLAYER || app->screen==DG_SETUP){
  if(key==DGK_F4){parent_screen(app,DG_RULES);return true;}
 }
 if(app->screen==DG_PLAYER){
  if(key==DGK_UP || key==DGK_LEFT)app->players=2;
  else if(key==DGK_DOWN || key==DGK_RIGHT)app->players=3;
  else if(key==DGK_F1 && dg_app_resumable(app)){enter_game(app);}
  else if(key==DGK_EXE || key==DGK_F6){app->screen=DG_SETUP;app->focus=0;if(app->slot>=app->players)app->slot=(uint8_t)(app->players-1);}
  return true;
 }
 if(app->screen==DG_SETUP){
  unsigned count=dg_entry_count(app);if(app->focus>=count)app->focus=0;
  int action=dg_entry_action(app,app->focus);
  if(key==DGK_UP)app->focus=(uint8_t)((app->focus+count-1)%count);
  else if(key==DGK_DOWN)app->focus=(uint8_t)((app->focus+1u)%count);
  else if(key==DGK_LEFT || key==DGK_RIGHT){
   int direction=key==DGK_LEFT?-1:1;
   if(action==DG_ENTRY_LEVEL)app->level=adjust_level(app->level,direction);
   else if(action==DG_ENTRY_SLOT)app->slot=adjust(app->slot,direction,(uint8_t)(app->players-1));
   else if(action==DG_ENTRY_ASSIST){
    uint8_t value=key==DGK_RIGHT?1:0;
    if(value!=app->archive.assist){app->archive.assist=value;app->dirty=1;}
   }
  }
  else if(key==DGK_EXE || key==DGK_F6){
   if(action==DG_ENTRY_RESUME)enter_game(app);else (void)new_game(app);
  }
  else if(key==DGK_EXIT){if(dg_checkpoint(app))app->screen=DG_PLAYER;}
  return true;
 }
 DgGame *game=&app->archive.game;
 if(app->thinking || app->animation){
  if(key==DGK_EXIT){(void)dg_app_to_setup(app);return true;}
  return false;
 }
 if(app->modal==DG_MODAL_RESTART){
  if(key==DGK_EXIT)app->modal=DG_MODAL_NONE;
  else if(key==DGK_EXE){dg_restart(game);app->archive.active=1;app->modal=DG_MODAL_NONE;app->selected=DG_NONE;app->thinking=app->animation=0;app->path.length=0;clear_trails(app);app->dirty=1;(void)dg_checkpoint(app);}
  return true;
 }
 if(app->modal==DG_MODAL_RESULT){
  if(key==DGK_EXIT)app->modal=DG_MODAL_NONE;else if(key==DGK_EXE)(void)new_game(app);return true;
 }
 if(key==DGK_F4){parent_screen(app,DG_RULES);return true;}
 if(key==DGK_F5){app->zoom^=1;return true;}
 if(key==DGK_EXIT){
  if(app->thinking || app->animation){(void)dg_app_to_setup(app);return true;}
  app->path.length=0;
  if(app->selected!=DG_NONE){app->selected=DG_NONE;return true;}
  if(app->zoom){app->zoom=0;return true;}
  (void)dg_app_to_setup(app);return true;
 }
 if(game->pos.winner){if(key==DGK_F6)(void)new_game(app);return true;}
 if(app->thinking || app->animation)return false;
 if(key==DGK_F1){app->modal=DG_MODAL_RESTART;return true;}
 if(key==DGK_F2){if(dg_undo(game)){app->selected=DG_NONE;app->path.length=0;clear_trails(app);app->dirty=1;}return true;}
 if(dg_current(game)!=DG_RED)return false;
 if(key>=DGK_UP && key<=DGK_RIGHT){app->cursor=(uint8_t)dg_nodes[app->cursor].nav[key-DGK_UP];dg_app_preview(app);return true;}
 if(key==DGK_EXE || key==DGK_F6){
  if(app->selected==DG_NONE){
   if(game->pos.board[app->cursor]!=DG_RED)warning(app,"SELECT YOUR PIECE");
   else{app->selected=app->cursor;dg_app_preview(app);}
  }else{
   DgMove move;
   if(!dg_find_move(game->pos.board,DG_RED,app->selected,app->cursor,&move,NULL))warning(app,game->pos.board[app->cursor]?"DESTINATION OCCUPIED":"INVALID MOVE");
   else if(dg_commit(game,&move)){clear_trails(app);finished(app);}
  }
  return true;
 }
 return false;
}
bool dg_app_cpu(DgApp *app,DgCancel cancel,void *context)
{
 if(app->screen!=DG_GAME || app->modal || app->archive.game.pos.winner || dg_current(&app->archive.game)==DG_RED || app->animation)return false;
 app->thinking=1;app->thinking_phase=0;
 bool result=dg_ai_choose(&app->archive.game,0,cancel,context,&app->pending_move,&app->pending_rng,&app->ai_stats);
 app->thinking=0;
 if(!result){if(!app->ai_stats.cancelled)warning(app,"CPU HAS NO MOVE");return false;}
 DgGame *game=&app->archive.game;uint8_t actor=dg_current(game);
 memset(&app->path,0,sizeof app->path);
 if(!dg_find_move(game->pos.board,actor,app->pending_move.from,app->pending_move.to,NULL,&app->path)
    || app->path.length<2 || app->path.length>DG_NODES){warning(app,"CPU MOVE REJECTED");return false;}
 memcpy(app->animation_board,game->pos.board,DG_NODES);
 /* The engine validates the final move once. No frame touches game or RNG. */
 if(!dg_commit(game,&app->pending_move)){app->path.length=0;warning(app,"CPU MOVE REJECTED");return false;}
 game->pos.rng=app->pending_rng;app->dirty=1;app->selected=DG_NONE;
 app->animation_actor=actor;app->animation_ticks=0;app->anim_index=app->anim_phase=0;app->animation=1;
 if(game->pos.winner){app->archive.active=0;(void)dg_checkpoint(app);}
 return true;
}
void dg_app_skip_animation(DgApp *app)
{
 if(!app->animation)return;
 /* Preserve the other AI's immediately previous path, then append this one. */
 DgAiTrail *earlier=&app->trails[0],*later=&app->trails[1];
 if(later->valid && later->player!=app->animation_actor)*earlier=*later;
 else if(!earlier->valid || earlier->player==app->animation_actor)earlier->valid=0;
 *later=(DgAiTrail){.valid=app->path.length>=2 && app->path.length<=DG_NODES && app->animation_actor>=DG_YELLOW && app->animation_actor<=DG_GREEN,
  .player=app->animation_actor,.path=app->path};
 unsigned last=app->path.length?app->path.length-1u:0;
 app->animation_ticks=(uint16_t)(last*DG_HOP_TICKS);
 app->anim_index=(uint8_t)last;app->anim_phase=0;
 app->animation=0;
 app->path.length=0;
 if(app->archive.game.pos.winner)app->modal=DG_MODAL_RESULT;
}
bool dg_app_animation_tick(DgApp *app,uint32_t elapsed_ticks)
{
 if(!app->animation)return false;
 uint32_t total=(uint32_t)(app->path.length-1u)*DG_HOP_TICKS;
 if(elapsed_ticks>=total){dg_app_skip_animation(app);return true;}
 uint16_t ticks=(uint16_t)(elapsed_ticks/DG_FRAME_TICKS*DG_FRAME_TICKS);
 if(ticks<=app->animation_ticks)return false;
 app->animation_ticks=ticks;app->anim_index=(uint8_t)(ticks/DG_HOP_TICKS);
 app->anim_phase=(uint8_t)((ticks%DG_HOP_TICKS)/DG_FRAME_TICKS);return true;
}
bool dg_app_animation(DgApp *app)
{return dg_app_animation_tick(app,(uint32_t)app->animation_ticks+DG_FRAME_TICKS);}
