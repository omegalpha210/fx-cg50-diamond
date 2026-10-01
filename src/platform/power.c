#include "power.h"
#include <limits.h>
void dg_power_init(DgPower *power,uint32_t now,int backlight,int apo)
{
 *power=(DgPower){.last=now,
  .dim_ticks=(uint32_t)((backlight==1 || backlight==2 || backlight==6)?backlight*30:60)*128u,
  .off_ticks=(uint32_t)((apo==10 || apo==60)?apo:10)*60u*128u};
}
unsigned dg_power_tick(DgPower *power,uint32_t now,bool actual_input)
{
 uint32_t elapsed=now>=power->last?now-power->last:DG_RTC_DAY-power->last+now;power->last=now;
 if(actual_input){power->idle_ticks=0;if(power->dimmed){power->dimmed=false;return DG_POWER_RESTORE;}return 0;}
 power->idle_ticks=elapsed>UINT32_MAX-power->idle_ticks?UINT32_MAX:power->idle_ticks+elapsed;
 if(power->idle_ticks>=power->off_ticks){unsigned result=DG_POWER_OFF|(power->dimmed?DG_POWER_RESTORE:0);power->dimmed=false;power->idle_ticks=0;return result;}
 if(!power->dimmed && power->idle_ticks>=power->dim_ticks){power->dimmed=true;return DG_POWER_DIM;}
 return 0;
}
