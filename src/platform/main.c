#include "ui.h"
#include "power.h"
#include "usb_lifecycle.h"
#include "usb_native.h"
#include "menu_boundary.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <gint/drivers/keydev.h>
#include <gint/drivers/r61524.h>
#include <gint/gint.h>
#include <gint/rtc.h>
#include <gint/timer.h>
#include <gint/cpu.h>
#include <stdio.h>
static DgApp app;
static DgPower power_state;
static UsbLifecycle usb;
static CgMenuBoundary menu_boundary;
static bool deferred_off;
static volatile int wakeup;
static int scheduler=-1,pending_action;
static bool timer_active,rtc_active,shift_pending,brightness_saved;
static uint16_t saved_brightness;
static uint32_t blocked,animation_last,thinking_started;
/* Match libfxcg's scalar ABI: duration returns one char in 30-second units. */
char dg_os_backlight_duration(void);
int dg_os_apo_minutes(void);
#if !defined(DG_NATIVE_TEST_SDK_H)
__asm__(
".text\n.align 2\n.global _dg_os_enable_menu_return\n"
"_dg_os_enable_menu_return:\n"
" mov.l 1f,r0\nmov.l 2f,r2\njmp @r2\nnop\n"
".align 2\n1: .long 0x1ea6\n2: .long 0x80020070\n"
);
int dg_os_enable_menu_return(void);
static int enable_menu_return(void *unused)
{ (void)unused; return dg_os_enable_menu_return(); }
#endif
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
 else rtc_active=rtc_periodic_enable(RTC_64Hz,GINT_CALL(pulse));
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
 if(!off){cg_menu_request(&menu_boundary,rtc_ticks());return;}
 if(!usb_handoff_begin(&usb,usb_native_sample()))return;
 if(!dg_storage_cleanup()){
  snprintf(app.notice,sizeof app.notice,"STORAGE CLOSE FAILED");
  usb_handoff_end(&usb,usb_native_sample());return;
 }
 stop_clock();restore_light();barrier();
#if !defined(DG_NATIVE_TEST_SDK_H)
 /* Safe OS Parking Rule (KhiCAS pattern):
    When user presses SHIFT+AC/ON or APO occurs,
    commit save above, wait for key releases, call Syscall 0x1EA6,
    and cleanly park into Casio OS Main Menu via gint_osmenu(). */
 while (keydown(KEY_ACON) || keydown(KEY_SHIFT) || keydown(KEY_MENU) || keydown(KEY_EXIT)) sleep();
 clearevents();
 (void)gint_world_switch(GINT_CALL(enable_menu_return,(void *)NULL));
 gint_osmenu();
#else
 if(off)gint_poweroff(true);else gint_osmenu();
#endif
 (void)gint_world_switch(GINT_CALL(read_power,(void *)NULL));
 barrier();start_clock();animation_last=rtc_ticks();
 usb_handoff_end(&usb,usb_native_sample());
}
static bool service_menu(void)
{
 int boundary=cg_menu_step(&menu_boundary,keydev_std(),rtc_ticks(),DG_RTC_DAY);
 if(boundary==CG_MENU_IDLE || boundary==CG_MENU_WAIT)return false;
 if(boundary!=CG_MENU_READY){
  cg_menu_cancel(&menu_boundary);snprintf(app.notice,sizeof app.notice,"MENU INPUT BUSY - RELEASE AND RETRY");return true;
 }
 if(!usb_handoff_begin(&usb,usb_native_sample()))return false;
 if(!dg_storage_cleanup()){
  cg_menu_cancel(&menu_boundary);snprintf(app.notice,sizeof app.notice,"STORAGE CLOSE FAILED");
  usb_handoff_end(&usb,usb_native_sample());return true;
 }
 boundary=cg_menu_step(&menu_boundary,keydev_std(),rtc_ticks(),DG_RTC_DAY);
 if(boundary!=CG_MENU_READY){
  usb_handoff_end(&usb,usb_native_sample());
  if(boundary==CG_MENU_INVALID || boundary==CG_MENU_TIMEOUT){cg_menu_cancel(&menu_boundary);snprintf(app.notice,sizeof app.notice,"MENU INPUT BUSY - RELEASE AND RETRY");return true;}
  return false;
 }
 cg_menu_cancel(&menu_boundary);stop_clock();restore_light();shift_pending=false;
#if !defined(DG_NATIVE_TEST_SDK_H)
 while (keydown(KEY_MENU) || keydown(KEY_EXIT)) sleep();
 clearevents();
 (void)gint_world_switch(GINT_CALL(enable_menu_return,(void *)NULL));
#endif
 /* Queue is empty and both scanner/event states stayed released across a
    fresh scan. Never clear newly queued requests at either side of this call. */
 gint_osmenu();(void)gint_world_switch(GINT_CALL(read_power,(void *)NULL));
 start_clock();animation_last=rtc_ticks();usb_handoff_end(&usb,usb_native_sample());return true;
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
 usb_observe(&usb,usb_native_sample());
 if(usb.pending)pending_action=DGK_MENU;
}
static bool dispatch_key(int key)
{
 bool system=key==DGK_MENU || key==DGK_OFF;
 if(system)(void)usb_take_request(&usb);
 bool redraw=dg_app_key(&app,key);
 if(system && !menu_boundary.pending){
  /* Include insertions during a failed checkpoint with no OS callback. */
  (void)usb_handoff_begin(&usb,usb_native_sample());
  usb_handoff_end(&usb,usb_native_sample());
 }
 return redraw;
}
/* Main-thread cooperative callback: never computes AI or writes flash in ISR. */
static bool cancel_search(void *context)
{
 (void)context;
 for(unsigned i=0;i<32;i++){
  key_event_t event=keydev_read(keydev_std(),false,NULL);idle(event);
  int key=logical_key(event);
  if(key==DGK_MENU || key==DGK_OFF || key==DGK_EXIT)pending_action=key;
  if(usb.pending)pending_action=DGK_MENU;
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
 else if(pending_action)(void)dispatch_key(pending_action);
 pending_action=0;if(!menu_boundary.pending)barrier();animation_last=rtc_ticks();draw();
}
static bool animation_frame(void)
{
 if(!app.animation)return false;
 uint32_t now=rtc_ticks(),elapsed=now>=animation_last?now-animation_last:DG_RTC_DAY-animation_last+now;
 bool redraw=dg_app_animation_tick(&app,elapsed);
 if(!app.animation)barrier();
 return redraw;
}
int main(void)
{
 cg_menu_cancel(&menu_boundary);deferred_off=false;
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
 /* Wake at 20 ms for four intermediate frames; power uses elapsed RTC time. */
 scheduler=timer_configure(TIMER_ANY,20000,GINT_CALL(pulse));
 (void)gint_world_switch(GINT_CALL(read_power,(void *)NULL));start_clock();
 usb_initialize(&usb,usb_native_sample());
 if(scheduler<0 && !rtc_active)snprintf(app.notice,sizeof app.notice,"IDLE TIMER UNAVAILABLE");
 animation_last=rtc_ticks();barrier();draw();
 for(;;){
  if(deferred_off && !menu_boundary.pending){deferred_off=false;(void)dispatch_key(DGK_OFF);draw();continue;}
  if(menu_boundary.pending){
   key_event_t event={0};
   for(unsigned count=0;count<32;count++){
    event=keydev_read(keydev_std(),false,NULL);idle(event);int key=logical_key(event);
    if(key==DGK_OFF || pending_action==DGK_OFF)deferred_off=true;
    if(key==DGK_EXIT){cg_menu_cancel(&menu_boundary);snprintf(app.notice,sizeof app.notice,"MENU CANCELLED");}
    else if(key==DGK_MENU)cg_menu_request(&menu_boundary,rtc_ticks());
    (void)usb_take_request(&usb);pending_action=0;
    if(event.type==KEYEV_NONE)break;
   }
   if(service_menu() || !menu_boundary.pending)draw();
   if(event.type==KEYEV_NONE)sleep();
   continue;
  }
  bool cpu=app.screen==DG_GAME && !app.modal && !app.animation && !app.archive.game.pos.winner && dg_current(&app.archive.game)!=DG_RED;
  if(cpu){
   cpu_turn();continue;
  }
  wakeup=0;
  key_event_t event=keydev_read(keydev_std(),true,(timer_active || rtc_active)?&wakeup:NULL);
  idle(event);int key=logical_key(event);if(pending_action){key=pending_action;pending_action=0;}
  bool redraw=false;uint8_t old_screen=app.screen,old_modal=app.modal;uint32_t old_turns=app.archive.game.pos.turns;
  if(key)redraw=dispatch_key(key);
  if(!menu_boundary.pending && (old_screen!=app.screen || old_modal!=app.modal || old_turns!=app.archive.game.pos.turns))barrier();
  redraw=animation_frame() || redraw;
  if(redraw)draw();
 }
}
