#include "ui.h"
#include "power.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/drivers/keydev.h>
#include <gint/drivers/r61524.h>
#include <gint/gint.h>
#include <gint/rtc.h>
#include <gint/timer.h>
#include <stdio.h>
static DgApp app;
static DgPower power_state;
static volatile int wakeup;
static int scheduler=-1,pending_action;
static bool timer_active,rtc_active,shift_pending,brightness_saved;
static uint16_t saved_brightness;
static uint32_t blocked,animation_last,thinking_started;
/* Match libfxcg's scalar ABI: duration returns one char in 30-second units. */
char dg_os_backlight_duration(void);
int dg_os_apo_minutes(void);
static void rect(void *context,int x,int y,int w,int h,uint16_t color)
{(void)context;if(w>0 && h>0)drect(x,y,x+w-1,y+h-1,(int)color);}
static void draw(void){DgCanvas canvas={NULL,rect};dg_render(&app,&canvas);dupdate();}
static bool save(void *context,DgArchive *archive)
{(void)context;return dg_storage_save(archive);}
static const int native_keys[]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_EXE,KEY_EXIT,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_MENU,KEY_ACON};
static void barrier(void)
{
 clearevents();blocked=0;shift_pending=false;
 for(unsigned i=0;i<sizeof native_keys/sizeof native_keys[0];i++)if(keydown(native_keys[i]))blocked|=1u<<i;
}
static void restore_light(void)
{if(brightness_saved){r61524_set(0x5a1,saved_brightness);brightness_saved=false;}power_state.dimmed=false;}
static int read_power(void *unused)
{(void)unused;dg_power_init(&power_state,rtc_ticks(),dg_os_backlight_duration(),dg_os_apo_minutes());return 0;}
static int pulse(void){wakeup=1;return TIMER_CONTINUE;}
static void start_clock(void)
{
 wakeup=0;
 if(scheduler>=0){timer_start(scheduler);timer_active=true;}
 else rtc_active=rtc_periodic_enable(RTC_16Hz,GINT_CALL(pulse));
}
static void stop_clock(void)
{
 if(timer_active){timer_pause(scheduler);timer_active=false;}
 if(rtc_active){rtc_periodic_disable();rtc_active=false;}
 wakeup=0;
}
static void system_action(void *context,bool off)
{
 (void)context;
 if(!dg_storage_cleanup()){snprintf(app.notice,sizeof app.notice,"STORAGE CLOSE FAILED");return;}
 stop_clock();restore_light();barrier();
 if(off)gint_poweroff(true);else gint_osmenu();
 (void)gint_world_switch(GINT_CALL(read_power,(void *)NULL));
 barrier();start_clock();animation_last=rtc_ticks();
}
static int repeat(int key,int duration,int count)
{(void)duration;if(key!=KEY_UP && key!=KEY_DOWN && key!=KEY_LEFT && key!=KEY_RIGHT)return -1;return count?125000:500000;}
static int logical_key(key_event_t e)
{
 if(e.key==KEY_SHIFT){if(e.type==KEYEV_DOWN)shift_pending=true;return 0;}
 for(unsigned i=0;i<sizeof native_keys/sizeof native_keys[0];i++)if(e.key==native_keys[i]){
  uint32_t bit=1u<<i;
  if(e.type==KEYEV_UP){blocked&=~bit;return 0;}
  if(e.type!=KEYEV_DOWN && e.type!=KEYEV_HOLD)return 0;
  if(blocked&bit)return 0;
  if(e.type==KEYEV_HOLD && i>=4)return 0;
  int key=(int)i+DGK_UP;
  if(e.key==KEY_ACON){key=(shift_pending || keydown(KEY_SHIFT))?DGK_OFF:0;}
  shift_pending=false;return key;
 }
 if(e.type==KEYEV_DOWN)shift_pending=false;
 return 0;
}
static void idle(key_event_t e)
{
 bool input=e.type==KEYEV_DOWN || e.type==KEYEV_HOLD;
 unsigned flags=dg_power_tick(&power_state,rtc_ticks(),input);
 if(flags&DG_POWER_RESTORE)restore_light();
 if(flags&DG_POWER_DIM){saved_brightness=(uint16_t)r61524_get(0x5a1);brightness_saved=true;r61524_set(0x5a1,saved_brightness<0x14?saved_brightness:0x14);}
 if(flags&DG_POWER_OFF)pending_action=DGK_OFF;
}
/* Main-thread cooperative callback: never computes AI or writes flash in ISR. */
static bool cancel_search(void *context)
{
 (void)context;
 for(unsigned i=0;i<32;i++){
  key_event_t event=keydev_read(keydev_std(),false,NULL);idle(event);
  int key=logical_key(event);
  if(key==DGK_MENU || key==DGK_OFF || key==DGK_EXIT)pending_action=key;
  if(pending_action)return true;
  if(event.type==KEYEV_NONE)break;
 }
 uint32_t now=rtc_ticks(),elapsed=now>=thinking_started?now-thinking_started:DG_RTC_DAY-thinking_started+now;
 if(dg_app_thinking_tick(&app,elapsed)){
  DgCanvas canvas={NULL,rect};dg_render_thinking(&app,&canvas);dupdate();
 }
 return false;
}
static void cpu_turn(void)
{
 app.thinking=1;app.thinking_phase=0;thinking_started=rtc_ticks();draw();pending_action=0;
 (void)dg_app_cpu(&app,cancel_search,NULL);
 /* dg_app_cpu cleared thinking already; busy EXIT still bypasses zoom. */
 if(pending_action==DGK_EXIT)(void)dg_app_to_setup(&app);
 else if(pending_action)(void)dg_app_key(&app,pending_action);
 pending_action=0;barrier();animation_last=rtc_ticks();draw();
}
int main(void)
{
 /* One framebuffer; the renderer targets the native 396x224 gint VRAM. */
 dsetvram(gint_vram,NULL);
 rtc_time_t date;rtc_get_time(&date);
 uint32_t seed=rtc_ticks()^((uint32_t)date.year<<16)^((uint32_t)date.month_day<<8)^date.month;
 dg_app_init(&app,(DgHooks){NULL,save,system_action},seed);
 int loaded=dg_storage_load(&app.archive);
 if(loaded==DG_LOAD_RECOVERED)snprintf(app.notice,sizeof app.notice,"RECOVERED OLDER SAVE");
 else if(loaded==DG_LOAD_INVALID)snprintf(app.notice,sizeof app.notice,"INVALID SAVE - FRESH SETUP");
 else if(loaded==DG_LOAD_IO_ERROR){snprintf(app.notice,sizeof app.notice,"SAVE READ ERROR");app.dirty=1;}
 keydev_set_transform(keydev_std(),(keydev_transform_t){KEYDEV_TR_REPEATS,repeat});
 scheduler=timer_configure(TIMER_ANY,50000,GINT_CALL(pulse));
 (void)gint_world_switch(GINT_CALL(read_power,(void *)NULL));start_clock();
 if(scheduler<0 && !rtc_active)snprintf(app.notice,sizeof app.notice,"IDLE TIMER UNAVAILABLE");
 animation_last=rtc_ticks();barrier();draw();
 for(;;){
  bool cpu=app.screen==DG_GAME && !app.modal && !app.animation && !app.archive.game.pos.winner && dg_current(&app.archive.game)!=DG_RED;
  if(cpu){
   cpu_turn();continue;
  }
  wakeup=0;
  key_event_t event=keydev_read(keydev_std(),true,(timer_active || rtc_active)?&wakeup:NULL);
  idle(event);int key=logical_key(event);if(pending_action){key=pending_action;pending_action=0;}
  bool redraw=false;uint8_t old_screen=app.screen,old_modal=app.modal;uint32_t old_turns=app.archive.game.pos.turns;
  if(key)redraw=dg_app_key(&app,key);
  if(old_screen!=app.screen || old_modal!=app.modal || old_turns!=app.archive.game.pos.turns)barrier();
  uint32_t now=rtc_ticks(),elapsed=now>=animation_last?now-animation_last:DG_RTC_DAY-animation_last+now;
  if(app.animation && elapsed>=6){redraw=dg_app_animation(&app);animation_last=now;if(!app.animation)barrier();}
  if(redraw)draw();
 }
}
