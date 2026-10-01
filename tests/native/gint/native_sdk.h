#ifndef DG_NATIVE_TEST_SDK_H
#define DG_NATIVE_TEST_SDK_H
/* Narrow host doubles for the APIs used by src/platform/main.c.
   They exercise application control flow, never hardware behavior. */
#include <stdbool.h>
#include <stdint.h>
enum {
 KEY_UP=1,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_EXE,KEY_EXIT,
 KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_MENU,KEY_ACON,KEY_SHIFT
};
enum { KEYEV_NONE,KEYEV_DOWN,KEYEV_UP,KEYEV_HOLD };
typedef struct { int key,type; } key_event_t;
typedef struct { int unused; } keydev_t;
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
