#include "ui.h"
#include "power.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Compile the real native static functions into this test translation unit. */
#define main dg_native_main
#include "../src/platform/main.c"
#undef main

static unsigned checks;
#define CHECK(expression) do { ++checks; if(!(expression)) { \
 fprintf(stderr,"native: %s failed at %s:%d\n",#expression,__FILE__,__LINE__);exit(1); \
 } } while(0)

typedef struct {
 uint32_t now;
 bool held[32],cleanup_ok,save_ok,rtc_ok,world;
 int brightness,backlight,apo;
 unsigned saves,cleanups,menus,offs,starts,pauses,enables,disables,updates,rectangles,reads,clears;
 unsigned backlight_queries,apo_queries,light_reads,light_writes,rtc_reads;
 key_event_t queue[128];
 unsigned head,tail;
 char trace[256];unsigned trace_length;
 DgArchive disk;
} Mock;
static Mock mock;
static keydev_t device;
static uint16_t pixel;
uint16_t *gint_vram=&pixel;

static void record(char event)
{
 CHECK(mock.trace_length+1u<sizeof mock.trace);
 mock.trace[mock.trace_length++]=event;mock.trace[mock.trace_length]=0;
}
static void held_event(key_event_t event)
{
 if(event.key<=0 || event.key>=(int)(sizeof mock.held/sizeof mock.held[0]))return;
 if(event.type==KEYEV_DOWN)mock.held[event.key]=true;
 if(event.type==KEYEV_UP)mock.held[event.key]=false;
}
static int deliver(int key,int type)
{
 key_event_t event={key,type};held_event(event);return logical_key(event);
}
static void enqueue(int key,int type)
{
 CHECK(mock.tail<sizeof mock.queue/sizeof mock.queue[0]);
 mock.queue[mock.tail++]=(key_event_t){key,type};
}
keydev_t *keydev_std(void){return &device;}
key_event_t keydev_read(keydev_t *dev,bool wait,volatile int *flag)
{
 CHECK(dev==&device);(void)wait;(void)flag;++mock.reads;
 if(mock.head==mock.tail)return (key_event_t){0,KEYEV_NONE};
 key_event_t event=mock.queue[mock.head++];held_event(event);return event;
}
void keydev_set_transform(keydev_t *dev,keydev_transform_t transform)
{CHECK(dev==&device && transform.flags==KEYDEV_TR_REPEATS && transform.repeat!=NULL);}
bool keydown(int key)
{return key>0 && key<(int)(sizeof mock.held/sizeof mock.held[0]) && mock.held[key];}
void clearevents(void){++mock.clears;mock.head=mock.tail;record('B');}
void dsetvram(uint16_t *first,uint16_t *second){CHECK(first==gint_vram && second==NULL);}
void drect(int x1,int y1,int x2,int y2,int color)
{(void)color;CHECK(x1<=x2 && y1<=y2);++mock.rectangles;}
void dupdate(void){++mock.updates;}
uint16_t r61524_get(int reg){CHECK(reg==0x5a1);++mock.light_reads;return (uint16_t)mock.brightness;}
void r61524_set(int reg,uint16_t value){CHECK(reg==0x5a1);++mock.light_writes;mock.brightness=value;record('L');}
uint32_t rtc_ticks(void){++mock.rtc_reads;return mock.now;}
void rtc_get_time(rtc_time_t *time){*time=(rtc_time_t){2026,1,10};}
char dg_os_backlight_duration(void){CHECK(mock.world);++mock.backlight_queries;return (char)mock.backlight;}
int dg_os_apo_minutes(void){CHECK(mock.world);++mock.apo_queries;return mock.apo;}
int gint_world_switch(gint_call_t call)
{
 record('W');CHECK(!mock.world);mock.world=true;int result;
 if(call.with_argument!=NULL)result=call.with_argument(call.argument);
 else{CHECK(call.without_argument!=NULL);result=call.without_argument();}
 mock.world=false;return result;
}
static void os_entry(void)
{
 CHECK(!mock.world && mock.cleanup_ok && !timer_active && !rtc_active);
 CHECK(!app.dirty && !app.thinking && !app.animation);
 CHECK(!brightness_saved && !power_state.dimmed);
 mock.now+=128u;
}
void gint_osmenu(void){os_entry();++mock.menus;record('M');}
void gint_poweroff(bool key_wait){CHECK(key_wait);os_entry();++mock.offs;record('O');}
int timer_configure(int timer,uint32_t delay,gint_call_t call)
{CHECK(timer==TIMER_ANY && delay==50000u && call.without_argument==pulse);return 7;}
void timer_start(int timer){CHECK(timer==7);++mock.starts;record('T');}
void timer_pause(int timer){CHECK(timer==7);++mock.pauses;record('P');}
bool rtc_periodic_enable(int frequency,gint_call_t call)
{CHECK(frequency==RTC_16Hz && call.without_argument==pulse);++mock.enables;record('R');return mock.rtc_ok;}
void rtc_periodic_disable(void){++mock.disables;record('D');}
int dg_storage_load(DgArchive *archive){(void)archive;return DG_LOAD_ABSENT;}
bool dg_storage_save(DgArchive *archive)
{
 ++mock.saves;record('S');
 if(!mock.save_ok)return false;
 uint8_t data[DG_SAVE_BYTES];CHECK(dg_encode(archive,data,sizeof data)>0u);
 ++archive->generation;mock.disk=*archive;return true;
}
bool dg_storage_cleanup(void){++mock.cleanups;record('C');return mock.cleanup_ok;}

static void reset(void)
{
 memset(&mock,0,sizeof mock);mock.cleanup_ok=mock.save_ok=mock.rtc_ok=true;
 mock.brightness=0x80;mock.backlight=1;mock.apo=10;
 scheduler=7;pending_action=0;timer_active=rtc_active=shift_pending=brightness_saved=false;
 saved_brightness=0;blocked=animation_last=0;wakeup=0;
 dg_app_init(&app,(DgHooks){NULL,save,system_action},718361u);
 dg_power_init(&power_state,0,mock.backlight,mock.apo);
}
static void clear_trace(void){mock.trace_length=0;mock.trace[0]=0;}
static void game(uint8_t players,uint8_t level)
{
 CHECK(dg_new(&app.archive.game,players,level,1u,24681u));
 app.archive.active=1;app.screen=DG_GAME;app.selected=DG_NONE;
 app.players=players;app.level=level;app.slot=1u;app.dirty=1u;
 CHECK(dg_current(&app.archive.game)!=DG_RED && dg_game_valid(&app.archive.game));
}

static void test_shift_and_barrier(void)
{
 reset();
 CHECK(deliver(KEY_SHIFT,KEYEV_DOWN)==0 && shift_pending);
 CHECK(deliver(KEY_SHIFT,KEYEV_UP)==0 && shift_pending && !keydown(KEY_SHIFT));
 CHECK(deliver(KEY_ACON,KEYEV_DOWN)==DGK_OFF && !shift_pending);
 CHECK(deliver(KEY_ACON,KEYEV_UP)==0);
 CHECK(deliver(KEY_ACON,KEYEV_DOWN)==0);
 CHECK(deliver(KEY_ACON,KEYEV_UP)==0);
 CHECK(deliver(KEY_SHIFT,KEYEV_DOWN)==0);
 CHECK(deliver(KEY_ACON,KEYEV_DOWN)==DGK_OFF); /* Held SHIFT. */
 CHECK(deliver(KEY_ACON,KEYEV_HOLD)==0);CHECK(deliver(KEY_ACON,KEYEV_UP)==0);
 CHECK(deliver(KEY_SHIFT,KEYEV_UP)==0);
 CHECK(deliver(KEY_SHIFT,KEYEV_DOWN)==0);CHECK(deliver(KEY_SHIFT,KEYEV_UP)==0);
 CHECK(deliver(KEY_UP,KEYEV_DOWN)==DGK_UP && !shift_pending);
 CHECK(deliver(KEY_ACON,KEYEV_DOWN)==0);
 reset();CHECK(deliver(KEY_SHIFT,KEYEV_DOWN)==0);barrier();CHECK(!shift_pending);
 CHECK(deliver(KEY_ACON,KEYEV_DOWN)==DGK_OFF); /* Held modifier crosses barrier. */

 for(unsigned i=0u;i<sizeof native_keys/sizeof native_keys[0];++i){
  reset();mock.held[native_keys[i]]=true;
  enqueue(KEY_EXE,KEYEV_DOWN);barrier();CHECK(mock.head==mock.tail);
  uint32_t bit=1u<<i;CHECK((blocked&bit)!=0u);
  CHECK(deliver(native_keys[i],KEYEV_HOLD)==0);
  CHECK(deliver(native_keys[i],KEYEV_DOWN)==0);
  CHECK(deliver(native_keys[i],KEYEV_UP)==0 && (blocked&bit)==0u);
  if(native_keys[i]==KEY_ACON)mock.held[KEY_SHIFT]=true;
  CHECK(deliver(native_keys[i],KEYEV_DOWN)==(int)i+DGK_UP);
  CHECK(deliver(native_keys[i],KEYEV_HOLD)==(i<4u?(int)i+DGK_UP:0));
 }
 CHECK(repeat(KEY_UP,0,0)==500000);CHECK(repeat(KEY_LEFT,0,1)==125000);
 CHECK(repeat(KEY_EXE,0,0)==-1);CHECK(repeat(KEY_MENU,0,1)==-1);
}

static void test_system_order(void)
{
 for(unsigned off=0u;off<2u;++off){
  reset();game(2u,DG_EASY);start_clock();
  brightness_saved=true;saved_brightness=0x80;mock.brightness=0x14;power_state.dimmed=true;
  app.thinking=app.animation=1u;app.path.length=3u;
  DgGame committed=app.archive.game;clear_trace();
  CHECK(dg_app_key(&app,off?DGK_OFF:DGK_MENU));
  CHECK(strcmp(mock.trace,off?"SCPLBOWBT":"SCPLBMWBT")==0);
  CHECK(mock.saves==1u && mock.cleanups==1u && mock.pauses==1u && mock.starts==2u);
  CHECK(mock.offs==off && mock.menus==1u-off);
  CHECK(timer_active && !rtc_active && !app.dirty && power_state.last==mock.now);
  CHECK(mock.brightness==0x80 && animation_last==mock.now);
  CHECK(memcmp(&committed,&app.archive.game,sizeof committed)==0);
  CHECK(memcmp(&committed,&mock.disk.game,sizeof committed)==0);
 }
 /* An unavailable ETMU uses the real RTC fallback flow. */
 reset();game(3u,DG_HARD);scheduler=-1;start_clock();clear_trace();
 CHECK(rtc_active && !timer_active);
 CHECK(dg_app_key(&app,DGK_OFF));CHECK(strcmp(mock.trace,"SCDBOWBR")==0);
 CHECK(mock.disables==1u && mock.enables==2u && rtc_active && !timer_active);
 /* Failed handle cleanup prevents OS entry and does not pause the clock. */
 reset();game(2u,DG_EASY);start_clock();mock.cleanup_ok=false;clear_trace();
 CHECK(dg_app_key(&app,DGK_MENU));CHECK(strcmp(mock.trace,"SC")==0);
 CHECK(mock.menus==0u && mock.offs==0u && mock.pauses==0u && timer_active);
 CHECK(strstr(app.notice,"STORAGE CLOSE FAILED")!=NULL);
 mock.cleanup_ok=true;clear_trace();CHECK(dg_app_key(&app,DGK_MENU));
 CHECK(strcmp(mock.trace,"CPBMWBT")==0 && mock.menus==1u);
 /* A failed dirty checkpoint prevents even the cleanup/OS callback. */
 reset();game(2u,DG_HARD);start_clock();mock.save_ok=false;clear_trace();
 CHECK(dg_app_key(&app,DGK_OFF));CHECK(strcmp(mock.trace,"S")==0);
 CHECK(app.dirty && mock.cleanups==0u && mock.offs==0u && timer_active);
}

static void test_cpu_keys(void)
{
 static const uint8_t levels[]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2u;players<=3u;++players)for(unsigned profile=0u;profile<sizeof levels/sizeof levels[0];++profile)
  for(unsigned action=0u;action<4u;++action){
   reset();game(players,levels[profile]);start_clock();clear_trace();
   DgGame committed=app.archive.game;
   int expected=action==0u?DGK_MENU:(action==1u?DGK_EXIT:DGK_OFF);
   if(action==0u)enqueue(KEY_MENU,KEYEV_DOWN);
   else if(action==1u)enqueue(KEY_EXIT,KEYEV_DOWN);
   else if(action==2u){enqueue(KEY_SHIFT,KEYEV_DOWN);enqueue(KEY_SHIFT,KEYEV_UP);enqueue(KEY_ACON,KEYEV_DOWN);}
   else{mock.held[KEY_SHIFT]=true;enqueue(KEY_ACON,KEYEV_DOWN);}
   CHECK(!dg_app_cpu(&app,cancel_search,NULL));CHECK(app.ai_stats.cancelled && pending_action==expected);
   CHECK(!app.thinking && !app.animation && mock.saves==0u && mock.cleanups==0u);
   CHECK(memcmp(&committed,&app.archive.game,sizeof committed)==0);
   CHECK(dg_app_key(&app,pending_action));pending_action=0;
   CHECK(memcmp(&committed,&app.archive.game,sizeof committed)==0);
   CHECK(memcmp(&committed,&mock.disk.game,sizeof committed)==0);
   CHECK(mock.saves==1u && !app.dirty);
   if(expected==DGK_EXIT)CHECK(app.screen==DG_SETUP && mock.cleanups==0u && mock.menus==0u && mock.offs==0u);
   else CHECK(mock.cleanups==1u && mock.menus==(expected==DGK_MENU?1u:0u) && mock.offs==(expected==DGK_OFF?1u:0u));
  }
 /* The callback has a bounded 32-event drain and retains later system input. */
 reset();game(2u,DG_HARD);
 for(unsigned i=0u;i<32u;++i)enqueue(KEY_UP,KEYEV_HOLD);
 enqueue(KEY_MENU,KEYEV_DOWN);
 CHECK(!cancel_search(NULL) && mock.reads==32u && pending_action==0);
 CHECK(cancel_search(NULL) && mock.reads==33u && pending_action==DGK_MENU);
}

static void test_idle_and_pulse(void)
{
 reset();game(2u,DG_EASY);start_clock();
 mock.now=30u*128u;idle((key_event_t){0,KEYEV_NONE});
 CHECK(power_state.dimmed && brightness_saved && saved_brightness==0x80 && mock.brightness==0x14);
 uint32_t idle_before=power_state.idle_ticks,last_before=power_state.last;
 mock.now+=128u;CHECK(pulse()==TIMER_CONTINUE && wakeup==1);
 draw();CHECK(mock.updates==1u && mock.rectangles>0u);
 CHECK(power_state.idle_ticks==idle_before && power_state.last==last_before);
 CHECK(!cancel_search(NULL));CHECK(power_state.idle_ticks==idle_before+128u && power_state.dimmed);
 mock.now+=128u;idle((key_event_t){KEY_UP,KEYEV_UP});
 CHECK(power_state.idle_ticks==idle_before+256u); /* Release is not activity. */
 mock.now+=128u;idle((key_event_t){KEY_RIGHT,KEYEV_HOLD});
 CHECK(power_state.idle_ticks==0u && !power_state.dimmed && !brightness_saved && mock.brightness==0x80);
 mock.now+=600u*128u;
 DgGame committed=app.archive.game;CHECK(cancel_search(NULL) && pending_action==DGK_OFF);
 CHECK(memcmp(&committed,&app.archive.game,sizeof committed)==0 && mock.saves==0u);
 CHECK(dg_app_key(&app,pending_action));CHECK(mock.offs==1u && mock.saves==1u);
 CHECK(memcmp(&committed,&mock.disk.game,sizeof committed)==0);
 /* A failed RTC fallback is reported by state, not mistaken for an active timer. */
 reset();scheduler=-1;mock.rtc_ok=false;start_clock();CHECK(!rtc_active && !timer_active);
 stop_clock();CHECK(mock.disables==0u && mock.pauses==0u);
}

static void test_power_settings(void)
{
 static const int lights[]={1,2,6,-1,0,3,5,7};
 static const int apos[]={10,60,-1,0,30,600};
 for(unsigned b=0;b<sizeof lights/sizeof lights[0];b++)
  for(unsigned a=0;a<sizeof apos/sizeof apos[0];a++){
   reset();mock.now=971u;mock.backlight=lights[b];mock.apo=apos[a];
   CHECK(gint_world_switch(GINT_CALL(read_power,(void *)NULL))==0);
   uint32_t seconds=(lights[b]==1 || lights[b]==2 || lights[b]==6)?(uint32_t)lights[b]*30u:60u;
   uint32_t minutes=(apos[a]==10 || apos[a]==60)?(uint32_t)apos[a]:10u;
   CHECK(power_state.dim_ticks==seconds*128u && power_state.off_ticks==minutes*60u*128u);
   CHECK(power_state.last==mock.now && !power_state.idle_ticks && !power_state.dimmed);
   CHECK(mock.backlight_queries==1u && mock.apo_queries==1u && !mock.world);
   CHECK(mock.light_writes==0u && mock.saves==0u && mock.offs==0u);
  }
 /* Fresh queries after a supported MENU return, including a changed OS policy. */
 reset();CHECK(gint_world_switch(GINT_CALL(read_power,(void *)NULL))==0);
 game(2u,DG_EASY);start_clock();mock.backlight=6;mock.apo=60;
 CHECK(dg_app_key(&app,DGK_MENU));
 CHECK(mock.backlight_queries==2u && mock.apo_queries==2u);
 CHECK(power_state.dim_ticks==180u*128u && power_state.off_ticks==3600u*128u && power_state.last==mock.now);
}

static void test_wake_once_and_low_brightness(void)
{
 reset();CHECK(dg_new(&app.archive.game,2u,DG_EASY,0u,3671u));
 app.archive.active=1;app.screen=DG_GAME;app.players=2u;
 DgMove moves[DG_MAX_MOVES];size_t count=dg_generate(app.archive.game.pos.board,DG_RED,moves,DG_MAX_MOVES);CHECK(count>0u);
 app.selected=moves[0].from;app.cursor=moves[0].to;
 mock.now=30u*128u;idle((key_event_t){0,KEYEV_NONE});
 CHECK(mock.light_reads==1u && mock.light_writes==1u && mock.brightness==0x14);
 for(unsigned i=0;i<4u;i++){mock.now+=128u;idle((key_event_t){0,KEYEV_NONE});draw();}
 CHECK(mock.light_reads==1u && mock.light_writes==1u && brightness_saved);
 key_event_t press={KEY_EXE,KEYEV_DOWN};held_event(press);mock.now+=128u;idle(press);
 CHECK(mock.brightness==0x80 && mock.light_writes==2u && !brightness_saved && !power_state.idle_ticks);
 CHECK(dg_app_key(&app,logical_key(press)));CHECK(app.archive.game.pos.turns==1u && app.selected==DG_NONE && !app.modal);
 key_event_t hold={KEY_EXE,KEYEV_HOLD};idle(hold);CHECK(logical_key(hold)==0);
 CHECK(app.archive.game.pos.turns==1u && mock.light_writes==2u); /* No duplicate action or restoration. */
 /* A low saved PWM value is never raised by dimming, and is restored exactly. */
 reset();mock.brightness=0x08;mock.now=30u*128u;idle((key_event_t){0,KEYEV_NONE});
 CHECK(saved_brightness==0x08 && mock.brightness==0x08 && brightness_saved);
 mock.now+=1u;idle((key_event_t){KEY_UP,KEYEV_DOWN});CHECK(mock.brightness==0x08 && !brightness_saved);
 CHECK(mock.light_reads==1u && mock.light_writes==2u);
}

static void test_internal_activity_and_interrupt(void)
{
 reset();game(2u,DG_EASY);start_clock();mock.now=29u*128u;
 idle((key_event_t){0,KEYEV_NONE});DgPower before=power_state;DgApp committed=app;
 unsigned rtc_before=mock.rtc_reads,saves_before=mock.saves,light_before=mock.light_writes;
 for(unsigned i=0;i<100u;i++)CHECK(pulse()==TIMER_CONTINUE);
 CHECK(memcmp(&before,&power_state,sizeof before)==0 && memcmp(&committed,&app,sizeof committed)==0);
 CHECK(mock.rtc_reads==rtc_before && mock.saves==saves_before && mock.light_writes==light_before && mock.offs==0u);
 snprintf(app.notice,sizeof app.notice,"WARNING");draw();app.notice[0]=0;draw();
 CHECK(memcmp(&before,&power_state,sizeof before)==0);
 CHECK(dg_checkpoint(&app));CHECK(memcmp(&before,&power_state,sizeof before)==0);
 CHECK(dg_app_cpu(&app,cancel_search,NULL));CHECK(app.animation);
 CHECK(memcmp(&before,&power_state,sizeof before)==0);draw();
 CHECK(dg_app_animation(&app));CHECK(memcmp(&before,&power_state,sizeof before)==0);draw();
 mock.now=30u*128u;idle((key_event_t){0,KEYEV_NONE});CHECK(power_state.dimmed && mock.brightness==0x14);
 mock.now=31u*128u;idle((key_event_t){KEY_UP,KEYEV_UP});CHECK(power_state.idle_ticks==31u*128u && power_state.dimmed);
 /* A modifier-only physical press counts; no gameplay key needs to be produced. */
 mock.now=32u*128u;idle((key_event_t){KEY_SHIFT,KEYEV_DOWN});CHECK(!power_state.idle_ticks && !brightness_saved && mock.brightness==0x80);
}

static void test_automatic_off_failure_is_finite(void)
{
 reset();game(3u,DG_HARD);start_clock();mock.save_ok=false;
 DgGame committed=app.archive.game;mock.now=600u*128u;
 CHECK(!dg_app_cpu(&app,cancel_search,NULL));CHECK(app.ai_stats.cancelled && pending_action==DGK_OFF);
 CHECK(mock.saves==0u && mock.cleanups==0u && mock.offs==0u && memcmp(&committed,&app.archive.game,sizeof committed)==0);
 CHECK(dg_app_key(&app,pending_action));pending_action=0;
 CHECK(mock.saves==1u && app.dirty && mock.cleanups==0u && mock.offs==0u && timer_active);
 CHECK(strstr(app.notice,"SAVE FAILED")!=NULL);
 for(unsigned i=0;i<20u;i++){mock.now+=128u;idle((key_event_t){0,KEYEV_NONE});CHECK(!pending_action);}
 CHECK(mock.saves==1u && mock.offs==0u && memcmp(&committed,&app.archive.game,sizeof committed)==0);
 mock.save_ok=true;CHECK(dg_app_key(&app,DGK_OFF));CHECK(mock.saves==2u && mock.offs==1u && !app.dirty);
 CHECK(memcmp(&committed,&mock.disk.game,sizeof committed)==0);
 /* Handle-close failure also leaves the foreground clock running and no hot retry. */
 reset();game(2u,DG_EASY);start_clock();mock.cleanup_ok=false;mock.now=600u*128u;
 idle((key_event_t){0,KEYEV_NONE});CHECK(pending_action==DGK_OFF);
 CHECK(dg_app_key(&app,pending_action));pending_action=0;
 CHECK(mock.saves==1u && mock.cleanups==1u && mock.offs==0u && timer_active);
 mock.now+=128u;idle((key_event_t){0,KEYEV_NONE});CHECK(!pending_action && mock.saves==1u && mock.cleanups==1u);
}

int main(void)
{
 test_shift_and_barrier();test_system_order();test_cpu_keys();test_idle_and_pulse();
 test_power_settings();test_wake_once_and_low_brightness();test_internal_activity_and_interrupt();test_automatic_off_failure_is_finite();
 printf("native shim: %u checks passed; real key/barrier/cancel/system/idle code, no hardware claim\n",checks);
 return 0;
}
