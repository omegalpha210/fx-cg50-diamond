#ifndef DIAMOND_POWER_H
#define DIAMOND_POWER_H
#include <stdbool.h>
#include <stdint.h>
#define DG_RTC_DAY (24u*60u*60u*128u)
enum { DG_POWER_DIM=1,DG_POWER_RESTORE=2,DG_POWER_OFF=4 };
typedef struct {uint32_t last,idle_ticks,dim_ticks,off_ticks;bool dimmed;} DgPower;
void dg_power_init(DgPower *power,uint32_t now,int backlight,int apo);
unsigned dg_power_tick(DgPower *power,uint32_t now,bool actual_input);
#endif
