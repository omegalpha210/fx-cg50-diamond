#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned checks;
#define CHECK(expression) do { checks++;if(!(expression)){ \
 fprintf(stderr,"app: %s failed at %s:%d\n",#expression,__FILE__,__LINE__); \
 exit(1); } } while(0)
typedef struct {
 DgApp *app;
 DgArchive disk;
 unsigned saves,systems,sequence,last_save,last_system;
 bool fail,off;
} State;
static bool same_archive(const DgArchive *a,const DgArchive *b)
{
 uint8_t aa[DG_SAVE_BYTES],bb[DG_SAVE_BYTES];
 size_t sa=dg_encode(a,aa,sizeof aa),sb=dg_encode(b,bb,sizeof bb);
 return sa && sa==sb && !memcmp(aa,bb,sa);
}
static bool save_hook(void *context,DgArchive *archive)
{
 State *state=context;state->saves++;state->last_save=++state->sequence;
 if(state->fail)return false;
 uint8_t bytes[DG_SAVE_BYTES];DgArchive next=*archive;
 next.generation=state->disk.generation+1;
 size_t size=dg_encode(&next,bytes,sizeof bytes);CHECK(size>0);
 CHECK(dg_decode(&state->disk,bytes,size));archive->generation=next.generation;
 return true;
}
static void system_hook(void *context,bool off)
{
 State *state=context;state->systems++;state->off=off;
 state->last_system=++state->sequence;
 CHECK(!state->app->dirty && !state->app->thinking && !state->app->animation);
 CHECK(same_archive(&state->app->archive,&state->disk));
}
static void begin(DgApp *app,State *state,uint8_t players,uint8_t slot,uint8_t level)
{
 memset(state,0,sizeof *state);state->app=app;
 DgHooks hooks={state,save_hook,system_hook};dg_app_init(app,hooks,812713);
 app->players=players;CHECK(dg_app_key(app,DGK_F6));CHECK(app->screen==DG_SETUP);
 app->level=level;app->slot=slot;
 CHECK(dg_app_key(app,DGK_F6));CHECK(app->screen==DG_GAME);
 CHECK(app->archive.active && dg_game_valid(&app->archive.game));
 CHECK(same_archive(&app->archive,&state->disk));
}
static void cpu_response(DgApp *app)
{
 DgPosition before=app->archive.game.pos;
 CHECK(dg_app_cpu(app,NULL,NULL));CHECK(app->animation && !app->thinking);
 CHECK(app->archive.game.pos.turns==before.turns+1);
 CHECK(!memcmp(before.board,app->animation_board,DG_NODES));
 DgMove valid;CHECK(dg_find_move(before.board,app->animation_actor,
  app->pending_move.from,app->pending_move.to,&valid,NULL));
 CHECK(!memcmp(&valid,&app->pending_move,sizeof valid));
 unsigned ticks=0;while(app->animation){CHECK(dg_app_animation(app));CHECK(++ticks<=DG_NODES*DG_HOP_FRAMES);}
 CHECK(app->archive.game.pos.turns==before.turns+1);
 CHECK(dg_game_valid(&app->archive.game));
}
static void human_move(DgApp *app)
{
 CHECK(dg_current(&app->archive.game)==DG_RED);
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app->archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);
 CHECK(count>0);app->cursor=moves[0].from;
 CHECK(dg_app_key(app,DGK_EXE));CHECK(app->selected==moves[0].from);
 app->cursor=moves[0].to;CHECK(dg_app_key(app,DGK_EXE));
 CHECK(app->selected==DG_NONE && app->dirty);
 CHECK(dg_game_valid(&app->archive.game));
}
static void test_replacement_failure(void)
{
 DgApp app;State state;begin(&app,&state,3,1,DG_EASY);
 DgArchive old=app.archive,disk=state.disk;
 CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_SETUP);
 app.players=2;app.slot=0;app.level=DG_HARD;state.fail=true;
 unsigned saves=state.saves;
 CHECK(dg_app_key(&app,DGK_F6));CHECK(app.screen==DG_SETUP);
 CHECK(same_archive(&app.archive,&old));CHECK(same_archive(&state.disk,&disk));
 CHECK(state.saves==saves+1);CHECK(strstr(app.notice,"SAVE FAILED")!=NULL);
 /* A pending preference checkpoint also blocks replacement before NEW RNG. */
 app.archive.assist=0;app.dirty=1;uint32_t new_rng=app.new_rng;
 DgArchive dirty=app.archive;
 CHECK(dg_app_key(&app,DGK_F6));CHECK(app.screen==DG_SETUP && app.dirty);
 CHECK(app.new_rng==new_rng && same_archive(&app.archive,&dirty));
 CHECK(same_archive(&state.disk,&disk));
 state.fail=false;CHECK(dg_app_key(&app,DGK_F6));CHECK(app.screen==DG_GAME);
 CHECK(app.archive.game.players==2 && app.archive.game.level==DG_HARD);
 CHECK(app.archive.game.human_slot==0 && !app.dirty && !app.archive.assist);
 CHECK(same_archive(&app.archive,&state.disk));
}
static void test_lifecycle_checkpoints(void)
{
 for(unsigned off=0;off<2;off++){
  DgApp app;State state;begin(&app,&state,2,0,DG_EASY);human_move(&app);
  DgGame committed=app.archive.game;DgArchive prior=state.disk;
  app.thinking=1;app.animation=1;app.path.length=3;state.fail=true;
  CHECK(dg_app_key(&app,off?DGK_OFF:DGK_MENU));CHECK(state.systems==0);
  CHECK(app.dirty && !app.animation && !app.thinking && !app.path.length);
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed));
  CHECK(same_archive(&state.disk,&prior));
  CHECK(strstr(app.notice,"SAVE FAILED")!=NULL);
  state.fail=false;CHECK(dg_app_key(&app,off?DGK_OFF:DGK_MENU));
  CHECK(state.systems==1 && state.off==(off!=0));CHECK(!app.dirty);
  CHECK(state.last_save<state.last_system);CHECK(same_archive(&app.archive,&state.disk));
 }
 DgApp app;State state;begin(&app,&state,2,0,DG_EASY);human_move(&app);
 state.fail=true;CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_GAME && app.dirty);
 state.fail=false;CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_SETUP && !app.dirty);
 CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_PLAYER);
 CHECK(dg_app_key(&app,DGK_F1));CHECK(app.screen==DG_GAME);
 CHECK(same_archive(&app.archive,&state.disk));
}
typedef struct { DgApp *app;unsigned calls,stop;int key; } Cancel;
static bool cancel_hook(void *context)
{
 Cancel *cancel=context;cancel->calls++;
 if(cancel->calls<cancel->stop)return false;
 if(cancel->key)CHECK(dg_app_key(cancel->app,cancel->key));
 return true;
}
static void test_cpu_cancellation(void)
{
 for(uint8_t players=2;players<=3;players++)for(uint8_t level=DG_EASY;level<=DG_NORMAL;level++){
  DgApp app;State state;begin(&app,&state,players,1,level);
  DgGame committed=app.archive.game;unsigned saves=state.saves;
  Cancel cancel={&app,0,1,0};
  CHECK(!dg_app_cpu(&app,cancel_hook,&cancel));CHECK(app.ai_stats.cancelled);
  CHECK(!app.thinking && !app.animation);CHECK(state.saves==saves);
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed));
  cancel.calls=0;cancel.stop=4;cancel.key=DGK_MENU;
  CHECK(!dg_app_cpu(&app,cancel_hook,&cancel));CHECK(app.ai_stats.cancelled);
  CHECK(state.systems==1 && !state.off && !app.animation && !app.thinking);
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed));
  /* Final AI board/RNG commit precedes visual frames; OFF skips only pixels. */
  CHECK(dg_app_cpu(&app,NULL,NULL));CHECK(app.animation);
  CHECK(app.archive.game.pos.turns==committed.pos.turns+1);
  committed=app.archive.game;
  CHECK(dg_app_key(&app,DGK_OFF));CHECK(!app.animation && state.off);
  CHECK(!dg_app_animation(&app));
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed));
 }
}
static void test_undo_restart(void)
{
 for(uint8_t players=2;players<=3;players++)for(uint8_t slot=0;slot<players;slot++)for(uint8_t level=DG_EASY;level<=DG_NORMAL;level++){
  DgApp app;State state;begin(&app,&state,players,slot,level);
  DgGame initial=app.archive.game;
  while(dg_current(&app.archive.game)!=DG_RED)cpu_response(&app);
  CHECK(dg_app_key(&app,DGK_F2));CHECK(!app.archive.game.undo_valid);
  DgPosition before_human=app.archive.game.pos;
  unsigned saves=state.saves;
  human_move(&app);
  while(dg_current(&app.archive.game)!=DG_RED)cpu_response(&app);
  CHECK(state.saves==saves); /* No cursor, animation or search-node writes. */
  CHECK(app.archive.game.undo_valid);app.zoom=1;
  CHECK(dg_app_key(&app,DGK_F2));CHECK(!app.archive.game.undo_valid && app.zoom==1);
  CHECK(!memcmp(&before_human,&app.archive.game.pos,sizeof before_human));
  CHECK(dg_app_key(&app,DGK_F2));
  CHECK(!memcmp(&before_human,&app.archive.game.pos,sizeof before_human));
  human_move(&app);CHECK(app.archive.game.undo_valid);
  DgGame before_restart=app.archive.game;uint32_t new_rng=app.new_rng;
  CHECK(dg_app_key(&app,DGK_F1));CHECK(app.modal==DG_MODAL_RESTART);
  CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.modal==DG_MODAL_NONE);
  CHECK(!memcmp(&before_restart,&app.archive.game,sizeof before_restart));
  CHECK(app.zoom==1 && app.new_rng==new_rng && state.saves==saves);
  CHECK(dg_app_key(&app,DGK_F1));CHECK(dg_app_key(&app,DGK_EXE));
  CHECK(app.modal==DG_MODAL_NONE && !app.dirty && app.archive.active);
  CHECK(!memcmp(&initial,&app.archive.game,sizeof initial));
  CHECK(app.zoom==1 && app.new_rng==new_rng);
  CHECK(state.saves==saves+1 && same_archive(&app.archive,&state.disk));
 }
}
static void near_win(DgApp *app,uint8_t player,int *from,int *to)
{
 DgGame *game=&app->archive.game;memset(game->pos.board,0,sizeof game->pos.board);
 *from=*to=-1;
 for(int n=0;n<DG_NODES && *from<0;n++)if(dg_in_camp(n,dg_goal[player])){
  for(unsigned d=0;d<6;d++){
   int neighbor=dg_nodes[n].neighbor[d];
   if(neighbor>=0 && !dg_in_camp(neighbor,dg_goal[player])){*from=neighbor;*to=n;break;}
  }
 }
 CHECK(*from>=0 && *to>=0);
 for(int n=0;n<DG_NODES;n++)if(n!=*to && dg_in_camp(n,dg_goal[player]))game->pos.board[n]=player;
 game->pos.board[*from]=player;
 for(uint8_t other=DG_RED;other<=DG_GREEN;other++){
  if(other==player || (other==DG_YELLOW && game->players==2))continue;
  unsigned used=0;
  for(int n=0;n<DG_NODES && used<DG_PIECES;n++)if(n!=*to && !game->pos.board[n] && !dg_in_camp(n,dg_goal[other])){
   game->pos.board[n]=other;used++;
  }
  CHECK(used==DG_PIECES);
 }
 for(uint8_t slot=0;slot<game->players;slot++)if(game->order[slot]==player)game->pos.turn=slot;
 CHECK(dg_game_valid(game));
}
static void test_completion(void)
{
 for(uint8_t winner=DG_RED;winner<=DG_GREEN;winner++){
  DgApp app;State state;begin(&app,&state,3,0,DG_EASY);
  int from,to;near_win(&app,winner,&from,&to);
  if(winner==DG_RED){
   app.cursor=(uint8_t)from;CHECK(dg_app_key(&app,DGK_EXE));
   app.cursor=(uint8_t)to;CHECK(dg_app_key(&app,DGK_EXE));
  }else cpu_response(&app);
  CHECK(app.archive.game.pos.winner==winner && app.modal==DG_MODAL_RESULT);
  CHECK(!app.archive.active && !app.dirty && !state.disk.active);
  CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.modal==DG_MODAL_NONE);
  DgPosition terminal=app.archive.game.pos;
  CHECK(dg_app_key(&app,DGK_EXE)==false || !memcmp(&terminal,&app.archive.game.pos,sizeof terminal));
  CHECK(dg_app_key(&app,DGK_F2));CHECK(!memcmp(&terminal,&app.archive.game.pos,sizeof terminal));
  CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_SETUP);
  CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_PLAYER);
  CHECK(dg_app_key(&app,DGK_F2));CHECK(app.screen==DG_PLAYER);
 }
 /* Completion-save failure stays dirty until lifecycle retry succeeds. */
 DgApp app;State state;begin(&app,&state,2,0,DG_EASY);int from,to;
 near_win(&app,DG_RED,&from,&to);state.fail=true;
 app.cursor=(uint8_t)from;CHECK(dg_app_key(&app,DGK_EXE));
 app.cursor=(uint8_t)to;CHECK(dg_app_key(&app,DGK_EXE));
 CHECK(!app.archive.active && app.dirty && state.disk.active);
 CHECK(dg_app_key(&app,DGK_MENU));CHECK(state.systems==0 && app.dirty);
 state.fail=false;CHECK(dg_app_key(&app,DGK_MENU));CHECK(state.systems==1);
 CHECK(!state.disk.active && !app.dirty);
 CHECK(dg_storage_cleanup());
}
static void test_atomic_visual_system(void)
{
 for(uint8_t players=2;players<=3;players++)for(unsigned action=0;action<3;action++){
  DgApp app;State state;begin(&app,&state,players,1,DG_NORMAL);
  DgGame before=app.archive.game;unsigned saves=state.saves;
  CHECK(dg_app_cpu(&app,NULL,NULL));CHECK(app.animation && app.dirty);
  CHECK(app.archive.game.pos.turns==before.pos.turns+1);
  DgGame committed=app.archive.game;CHECK(dg_app_animation_tick(&app,3));
  CHECK(state.saves==saves && !memcmp(&committed,&app.archive.game,sizeof committed));
  app.zoom=1;CHECK(dg_app_key(&app,action==0?DGK_MENU:action==1?DGK_OFF:DGK_EXIT));
  CHECK(!app.animation && !app.thinking && state.saves==saves+1);
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed));
  CHECK(!memcmp(&committed,&state.disk.game,sizeof committed));
  CHECK(app.trails[1].valid && app.trails[1].path.length>=2);
  if(action==2)CHECK(app.screen==DG_SETUP && !state.systems);
  else CHECK(state.systems==1 && state.last_save<state.last_system);
 }
 for(uint8_t actor=DG_YELLOW;actor<=DG_GREEN;actor++)for(unsigned action=0;action<3;action++){
  DgApp app;State state;begin(&app,&state,3,1,DG_EASY);int from,to;
  near_win(&app,actor,&from,&to);unsigned saves=state.saves;
  CHECK(dg_app_cpu(&app,NULL,NULL));CHECK(app.animation && !app.modal);
  CHECK(app.archive.game.pos.winner==actor && !app.archive.active && !app.dirty);
  CHECK(state.saves==saves+1 && !state.disk.active);DgGame committed=app.archive.game;
  CHECK(dg_app_animation_tick(&app,3));CHECK(state.saves==saves+1);
  CHECK(!dg_app_key(&app,DGK_F6) && app.animation && !app.modal);
  CHECK(dg_app_key(&app,action==0?DGK_MENU:action==1?DGK_OFF:DGK_EXIT));
  CHECK(!app.animation && !state.disk.active && same_archive(&app.archive,&state.disk));
  CHECK(!memcmp(&committed,&app.archive.game,sizeof committed) && !app.archive.active);
  CHECK(state.saves==saves+1);
  if(action==2)CHECK(app.screen==DG_SETUP && app.modal==DG_MODAL_NONE && !dg_app_resumable(&app));
  else CHECK(state.systems==1 && app.modal==DG_MODAL_RESULT);
 }
}
static void test_entry_contract(void)
{
 static const int no_resume[4]={DG_ENTRY_NEW,DG_ENTRY_LEVEL,DG_ENTRY_SLOT,DG_ENTRY_ASSIST};
 static const int resume_rows[5]={DG_ENTRY_RESUME,DG_ENTRY_NEW,DG_ENTRY_LEVEL,DG_ENTRY_SLOT,DG_ENTRY_ASSIST};
 for(uint8_t players=2;players<=3;players++)for(unsigned saved=0;saved<3;saved++){
  DgApp app;State state;begin(&app,&state,saved==2?(players==2?3:2):players,0,DG_NORMAL);
  app.archive.active=saved!=0;app.screen=DG_PLAYER;app.players=players;
  DgGame original=app.archive.game;uint32_t new_rng=app.new_rng;unsigned saves=state.saves;
  CHECK(dg_app_key(&app,DGK_F2));CHECK(app.screen==DG_PLAYER);
  CHECK(dg_app_key(&app,DGK_F3));CHECK(app.screen==DG_PLAYER);
  CHECK(dg_app_key(&app,DGK_F6));CHECK(app.screen==DG_SETUP && app.focus==0);
  unsigned count=saved==1?5u:4u;CHECK(dg_entry_count(&app)==count);
  CHECK(dg_entry_action(&app,count)==-1);
  for(unsigned row=0;row<count;row++)CHECK(dg_entry_action(&app,row)==(saved==1?resume_rows[row]:no_resume[row]));
  CHECK(dg_entry_action(&app,count-1)==DG_ENTRY_ASSIST);
  CHECK(dg_app_key(&app,DGK_UP) && app.focus==count-1);
  CHECK(dg_app_key(&app,DGK_DOWN) && app.focus==0);
  for(unsigned row=0;row<(saved==1?2u:1u);row++){
   app.focus=(uint8_t)row;CHECK(dg_app_key(&app,DGK_LEFT));CHECK(dg_app_key(&app,DGK_RIGHT));
   CHECK(app.level==DG_NORMAL && app.slot==0 && app.archive.assist==1 && app.new_rng==new_rng);
  }
  app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_LEVEL);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_LEFT));CHECK(app.level==DG_EASY);
  CHECK(dg_app_key(&app,DGK_RIGHT) && app.level==DG_NORMAL);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_RIGHT));CHECK(app.level==DG_HARD);
  CHECK(dg_app_key(&app,DGK_LEFT) && app.level==DG_NORMAL);
  app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_SLOT);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_RIGHT));CHECK(app.slot==players-1);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_LEFT));CHECK(app.slot==0);
  app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_ASSIST);
  CHECK(dg_app_key(&app,DGK_RIGHT) && app.archive.assist==1 && !app.dirty);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_LEFT));CHECK(!app.archive.assist && app.dirty);
  for(unsigned i=0;i<4;i++)CHECK(dg_app_key(&app,DGK_RIGHT));CHECK(app.archive.assist && app.dirty);
  CHECK(!memcmp(&original,&app.archive.game,sizeof original) && app.new_rng==new_rng && state.saves==saves);
  /* Global Assist is checkpointed on leaving setup, including a mismatched run. */
  CHECK(dg_app_key(&app,DGK_LEFT));CHECK(dg_app_key(&app,DGK_EXIT));
  CHECK(app.screen==DG_PLAYER && !app.dirty && !state.disk.assist);
  if(saved){
   app.players=players==2?3:2;app.level=DG_HARD;app.slot=1;new_rng=app.new_rng;saves=state.saves;
   CHECK(dg_app_key(&app,DGK_F1) && app.screen==DG_GAME);
   CHECK(!memcmp(&original,&app.archive.game,sizeof original));
   CHECK(app.new_rng==new_rng && state.saves==saves && !app.archive.assist);
  }else{CHECK(dg_app_key(&app,DGK_F1) && app.screen==DG_PLAYER);}
 }
 /* EXE and OPEN start NEW from every non-RESUME row, without cycling a value. */
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(uint8_t slot=0;slot<players;slot++)
  for(unsigned level=0;level<3;level++)for(uint8_t assist=0;assist<2;assist++)for(unsigned action=0;action<4;action++)for(unsigned open=0;open<2;open++){
   DgApp app;State state;begin(&app,&state,players,0,DG_EASY);
   CHECK(dg_app_to_setup(&app));app.level=levels[level];app.slot=slot;app.archive.assist=assist;
   app.focus=(uint8_t)dg_entry_row(&app,no_resume[action]);
   uint32_t expected_rng=app.new_rng;DgGame expected;
   CHECK(dg_new(&expected,players,levels[level],slot,dg_random(&expected_rng)));
   CHECK(dg_app_key(&app,open?DGK_F6:DGK_EXE) && app.screen==DG_GAME);
   CHECK(!memcmp(&expected,&app.archive.game,sizeof expected));CHECK(app.new_rng==expected_rng);
   CHECK(app.level==levels[level] && app.slot==slot && app.archive.assist==assist);
   CHECK(app.archive.game.order[slot]==DG_RED && same_archive(&app.archive,&state.disk));
  }
 /* RESUME restores saved game bytes; current selectors cannot overwrite them. */
 for(uint8_t players=2;players<=3;players++)for(unsigned open=0;open<2;open++){
  DgApp app;State state;begin(&app,&state,players,1,DG_NORMAL);cpu_response(&app);
  DgGame saved=app.archive.game;CHECK(dg_app_to_setup(&app));
  app.level=DG_HARD;app.slot=0;app.focus=(uint8_t)dg_entry_row(&app,DG_ENTRY_ASSIST);
  CHECK(dg_app_key(&app,DGK_LEFT));CHECK(app.dirty && !app.archive.assist);
  app.focus=0;uint32_t rng=app.new_rng;unsigned saves=state.saves;
  CHECK(dg_app_key(&app,open?DGK_F6:DGK_EXE) && app.screen==DG_GAME);
  CHECK(!memcmp(&saved,&app.archive.game,sizeof saved) && app.new_rng==rng && state.saves==saves);
  CHECK(app.dirty && !app.archive.assist);CHECK(dg_checkpoint(&app));
  DgApp cold;dg_app_init(&cold,(DgHooks){0},927u);cold.archive=state.disk;cold.players=players==2?3:2;
  CHECK(dg_app_key(&cold,DGK_F1) && cold.screen==DG_GAME && !cold.archive.assist);
  CHECK(!memcmp(&saved,&cold.archive.game,sizeof saved));
 }
 /* Invalid or finished archives never enable either RESUME entry. */
 DgApp invalid;dg_app_init(&invalid,(DgHooks){0},1);invalid.archive.active=1;
 CHECK(!dg_app_resumable(&invalid) && !dg_setup_resume(&invalid));
 CHECK(dg_app_key(&invalid,DGK_F1) && invalid.screen==DG_PLAYER);
}
static void test_exit_hierarchy(void)
{
 for(uint8_t zoom=0;zoom<2;zoom++)for(unsigned selected=0;selected<2;selected++){
  DgApp app;State state;begin(&app,&state,2,0,DG_NORMAL);
  app.zoom=zoom;app.selected=selected?app.cursor:DG_NONE;
  DgGame before=app.archive.game;unsigned saves=state.saves;
  if(selected){CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_GAME && app.selected==DG_NONE && app.zoom==zoom && state.saves==saves);}
  if(zoom){CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_GAME && !app.zoom && state.saves==saves);}
  CHECK(dg_app_key(&app,DGK_EXIT));CHECK(app.screen==DG_SETUP && app.archive.active && app.focus==0);
  CHECK(dg_entry_action(&app,0)==DG_ENTRY_RESUME);
  CHECK(!memcmp(&before,&app.archive.game,sizeof before));
  CHECK(dg_app_key(&app,DGK_EXE) && app.screen==DG_GAME);
  CHECK(!memcmp(&before,&app.archive.game,sizeof before));
 }
 DgApp app;State state;begin(&app,&state,3,0,DG_NORMAL);
 app.zoom=1;app.selected=app.cursor;DgGame before=app.archive.game;
 CHECK(dg_app_key(&app,DGK_F1));CHECK(dg_app_key(&app,DGK_EXIT));
 CHECK(app.modal==DG_MODAL_NONE && app.zoom==1 && app.selected==app.cursor && app.screen==DG_GAME);
 CHECK(dg_app_key(&app,DGK_F4));CHECK(dg_app_key(&app,DGK_EXIT));
 CHECK(app.screen==DG_GAME && app.zoom==1 && app.selected==app.cursor);
 CHECK(!memcmp(&before,&app.archive.game,sizeof before));
 app.modal=DG_MODAL_RESULT;CHECK(dg_app_key(&app,DGK_EXIT));
 CHECK(app.screen==DG_GAME && app.zoom==1 && app.selected==app.cursor && !app.modal);
 /* Preview/busy EXIT bypasses normal selection and zoom hierarchy. */
 app.animation=1;app.path.length=2;CHECK(dg_app_key(&app,DGK_EXIT));
 CHECK(app.screen==DG_SETUP && !app.animation && !app.thinking && !app.path.length && app.selected==DG_NONE);
 CHECK(!memcmp(&before,&app.archive.game,sizeof before));
}
int main(void)
{
 test_replacement_failure();test_lifecycle_checkpoints();test_cpu_cancellation();
 test_undo_restart();test_completion();test_atomic_visual_system();test_entry_contract();test_exit_hierarchy();
 printf("app: %u checks passed; save faults, cancellation, undo, restart, completion\n",checks);
 return 0;
}
