#include "draw.h"

/* Q8 Euclidean length; bounded screen-space STEP/JUMP vectors only. */
static int root(unsigned square)
{
 unsigned result=0,bit=1u<<30;
 while(bit>square)bit>>=2;
 while(bit){if(square>=result+bit){square-=result+bit;result=(result>>1)+bit;}
  else result>>=1;
  bit>>=2;}
 return (int)result;
}
static int rounded(int value,int divisor)
{return value<0?-((-value+divisor/2)/divisor):(value+divisor/2)/divisor;}
static int along(int component,int distance,int length)
{return rounded(component*distance*256,length);}
bool ui_trail_segment(const DgApp *app,int from,int to,int lane,UiTrailSegment *s)
{
 if(from<0 || from>=DG_NODES || to<0 || to>=DG_NODES || from==to || lane<-1 || lane>1)return false;
 bool jump=false,found=false;
 for(int d=0;d<6;d++){
  if(dg_nodes[from].neighbor[d]==to)found=true;
  if(dg_nodes[from].jump[d]==to){found=true;jump=true;}
 }
 if(!found)return false;
 int x,y,tx,ty;dg_screen_position(app,from,&x,&y);dg_screen_position(app,to,&tx,&ty);
 int dx=tx-x,dy=ty-y,length=root((unsigned)(dx*dx+dy*dy)*65536u);
 /* Canonical low-id -> high-id normal, regardless of movement direction. */
 int sign=from<to?1:-1;
 int ox=along(-dy,sign*lane,length),oy=along(dx,sign*lane,length);
 x+=ox;y+=oy;tx+=ox;ty+=oy;
 int trim=app->zoom?9:6;
 s->x=x+along(dx,trim,length);s->y=y+along(dy,trim,length);
 s->tx=tx-along(dx,trim,length);s->ty=ty-along(dy,trim,length);
 /* For JUMP, put the head in the open interval beyond the crossed hole. */
 int head=rounded(length*(jump?3:2),1024)+(jump?1:0);
 /* One-pixel STEP-head stagger keeps short shared chevrons distinguishable. */
 if(!jump && lane>0)head++;
 int back=app->zoom?3:2;
 s->ax=x+along(dx,head,length);s->ay=y+along(dy,head,length);
 int bx=x+along(dx,head-back,length),by=y+along(dy,head-back,length);
 int nx=along(-dy,1,length),ny=along(dx,1,length);
 s->bx=bx+nx;s->by=by+ny;s->cx=bx-nx;s->cy=by-ny;
 return true;
}
static bool shared(const DgPath *other,int from,int to)
{
 for(unsigned i=1;i<other->length;i++){
  int a=other->node[i-1],b=other->node[i];
  if((a==from && b==to) || (a==to && b==from))return true;
 }
 return false;
}
void ui_trails(DgPainter *p,const DgApp *app)
{
 for(unsigned slot=0;slot<2;slot++){
  const DgAiTrail *trail=&app->trails[slot],*other=&app->trails[1u-slot];
  if(!trail->valid || trail->player<DG_YELLOW || trail->player>DG_GREEN || trail->path.length>DG_NODES)continue;
  for(unsigned i=1;i<trail->path.length;i++){
   int from=trail->path.node[i-1],to=trail->path.node[i];
   int lane=other->valid && other->path.length<=DG_NODES && shared(&other->path,from,to)?(slot?1:-1):0;
   UiTrailSegment s;if(!ui_trail_segment(app,from,to,lane,&s))continue;
   uint16_t ink=ui_actor_color(trail->player);
   ui_line(p,s.x,s.y,s.tx,s.ty,ink);
   ui_line(p,s.ax,s.ay,s.bx,s.by,ink);ui_line(p,s.ax,s.ay,s.cx,s.cy,ink);
  }
 }
}
void ui_animation_position(const DgApp *app,int *x,int *y)
{
 unsigned at=app->anim_index;if(at+1u>=app->path.length)at=app->path.length-2u;
 int tx,ty;dg_screen_position(app,app->path.node[at],x,y);
 dg_screen_position(app,app->path.node[at+1u],&tx,&ty);
 *x+=rounded((tx-*x)*(int)app->anim_phase,DG_HOP_FRAMES);
 *y+=rounded((ty-*y)*(int)app->anim_phase,DG_HOP_FRAMES);
}
