#ifndef DG_NATIVE_TEST_SDK_H
#define DG_NATIVE_TEST_SDK_H
/* Narrow host doubles for the APIs used by src/platform/main.c.
   They exercise application control flow, never hardware behavior. */
#include <stdbool.h>
#include <stdint.h>
enum {
 KEY_UP=0x86,KEY_DOWN=0x75,KEY_LEFT=0x85,KEY_RIGHT=0x76,KEY_EXE=0x15,KEY_EXIT=0x74,
 KEY_F1=0x91,KEY_F2=0x92,KEY_F3=0x93,KEY_F4=0x94,KEY_F5=0x95,KEY_F6=0x96,
 KEY_MENU=0x84,KEY_ACON=0x07,KEY_SHIFT=0x81
};
enum { KEYEV_NONE,KEYEV_DOWN,KEYEV_UP,KEYEV_HOLD };
typedef struct { int key,type; } key_event_t;
#define KEYBOARD_QUEUE_SIZE 32
typedef struct {uint32_t time;int8_t queue_next,queue_end;uint8_t state_now[12],state_queue[12];} keydev_t;
typedef struct { unsigned flags;int (*repeat)(int,int,int); } keydev_transform_t;
#define KEYDEV_TR_REPEATS 1u
keydev_t *keydev_std(void);
key_event_t keydev_read(keydev_t *device,bool wait,volatile int *wakeup);
void keydev_set_transform(keydev_t *device,keydev_transform_t transform);
bool keydown(int key);
void clearevents(void);
extern uint16_t *gint_vram;
void dsetvram(uint16_t *first,uint16_t *second);
void drect(int x1,int y1,int x2,int y2,int color);
void dupdate(void);
uint16_t r61524_get(int reg);
void r61524_set(int reg,uint16_t value);
typedef struct {
 int (*without_argument)(void);
 int (*with_argument)(void *);
 void *argument;
} gint_call_t;
#define DG_NATIVE_CALL0(fn) ((gint_call_t){.without_argument=(fn)})
#define DG_NATIVE_CALL1(fn,arg) ((gint_call_t){.with_argument=(fn),.argument=(arg)})
#define DG_NATIVE_CALL_SELECT(_1,_2,NAME,...) NAME
#define GINT_CALL(...) DG_NATIVE_CALL_SELECT(__VA_ARGS__,DG_NATIVE_CALL1,DG_NATIVE_CALL0,unused)(__VA_ARGS__)
int gint_world_switch(gint_call_t call);
void gint_osmenu(void);
void gint_poweroff(bool key_wait);
typedef struct { uint16_t year;uint8_t month_day,month; } rtc_time_t;
/* Installed gint uses selector 2 for the 64-Hz periodic RTC interrupt. */
#define RTC_64Hz 2
uint32_t rtc_ticks(void);
void rtc_get_time(rtc_time_t *time);
bool rtc_periodic_enable(int frequency,gint_call_t call);
void rtc_periodic_disable(void);
#define TIMER_ANY (-1)
#define TIMER_CONTINUE 0
int timer_configure(int timer,uint32_t delay,gint_call_t call);
void timer_start(int timer);
void timer_pause(int timer);
#endif
