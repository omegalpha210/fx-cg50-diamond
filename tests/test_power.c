#include "power.h"
#include "diamond.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static void timeout_matrix(void)
{
 static const int backlights[]={1,2,6,-1,0,3,5,7};
 static const int apos[]={10,60,-1,0,30,600};
 for(unsigned b=0;b<sizeof backlights/sizeof backlights[0];b++)
  for(unsigned a=0;a<sizeof apos/sizeof apos[0];a++){
   int light=backlights[b],off=apos[a];DgPower p;dg_power_init(&p,0,light,off);
   uint32_t seconds=(light==1 || light==2 || light==6)?(uint32_t)light*30u:60u;
   uint32_t minutes=(off==10 || off==60)?(uint32_t)off:10u;
   assert(p.dim_ticks==seconds*128u && p.off_ticks==minutes*60u*128u);
   assert(dg_power_tick(&p,p.dim_ticks-1u,false)==0);
   assert(dg_power_tick(&p,p.dim_ticks,false)==DG_POWER_DIM);
   assert(dg_power_tick(&p,p.dim_ticks+1u,false)==0);
   assert(dg_power_tick(&p,p.off_ticks-1u,false)==0);
   assert(dg_power_tick(&p,p.off_ticks,false)==(DG_POWER_OFF|DG_POWER_RESTORE));
   /* A foreground failure is not retried by every following wake tick. */
   for(uint32_t t=1;t<=100;t++)assert(!(dg_power_tick(&p,p.off_ticks+t*128u,false)&DG_POWER_OFF));
  }
}
static void input_priority_and_wrap(void)
{
 DgPower p;dg_power_init(&p,0,1,10);
 assert(dg_power_tick(&p,p.dim_ticks,true)==0 && !p.dimmed && !p.idle_ticks);
 assert(dg_power_tick(&p,2u*p.dim_ticks,false)==DG_POWER_DIM);
 assert(dg_power_tick(&p,2u*p.dim_ticks+p.off_ticks,true)==DG_POWER_RESTORE);
 assert(!p.dimmed && !p.idle_ticks);
 assert(dg_power_tick(&p,p.last,true)==0); /* One restoration per dim interval. */
 dg_power_init(&p,DG_RTC_DAY-64u,6,60);
 assert(dg_power_tick(&p,64u,false)==0 && p.idle_ticks==128u);
 for(unsigned i=0;i<160;i++)assert(dg_power_tick(&p,64u+8u*(i+1u),false)==0);
 assert(p.idle_ticks==1408u); /* 11 seconds, exact 128-Hz accumulation. */
}
int main(void)
{
 timeout_matrix();input_priority_and_wrap();
 DgPower p;dg_power_init(&p,0,2,10);
 assert(p.dim_ticks==60u*128u && p.off_ticks==600u*128u);
 assert(dg_power_tick(&p,59u*128u,false)==0);
 assert(dg_power_tick(&p,60u*128u,false)==DG_POWER_DIM);
 assert(dg_power_tick(&p,61u*128u,false)==0);
 assert(dg_power_tick(&p,62u*128u,true)==DG_POWER_RESTORE);
 assert(p.idle_ticks==0);
 assert(dg_power_tick(&p,122u*128u,false)==DG_POWER_DIM);
 assert(dg_power_tick(&p,662u*128u,false)==(DG_POWER_OFF|DG_POWER_RESTORE));
 dg_power_init(&p,DG_RTC_DAY-10,6,60);
 assert(dg_power_tick(&p,20,false)==0 && p.idle_ticks==30);
 dg_power_init(&p,0,-1,999);
 assert(p.dim_ticks==60u*128u && p.off_ticks==600u*128u);
 assert(dg_power_tick(&p,600u*128u,true)==0 && p.idle_ticks==0);
 assert(dg_power_tick(&p,1200u*128u,false)==(DG_POWER_OFF));
 DgGame game;memset(&game,0,sizeof game);game.players=255;game.pos.turn=254;game.pos.rng=1;
 assert(!dg_position_valid(&game,&game.pos,false));assert(dg_current(&game)==DG_EMPTY);assert(!dg_commit(&game,NULL));
 assert(!dg_game_valid(NULL));assert(!dg_position_valid(NULL,NULL,false));assert(dg_current(NULL)==0);assert(!dg_new(NULL,2,0,0,1));assert(!dg_undo(NULL));
 assert(dg_generate(NULL,DG_RED,NULL,0)==0);assert(!dg_find_move(NULL,DG_RED,0,1,NULL,NULL));assert(!dg_apply(NULL,DG_RED,NULL));
 puts("Power: 48 OS-value/fallback combinations, exact deadlines, no repeated OFF, input priority, restore-once, midnight/fractional ticks and prior malformed-state checks PASS");return 0;
}
