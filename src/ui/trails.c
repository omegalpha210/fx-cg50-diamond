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
 /* A 5px head needs +/-3px lanes: +/-2 (and 2.5 after diagonal
    rounding) shares a wing pixel. Exhaustive raster tests enforce a gap. */
 int offset=sign*lane*768;
 int ox=rounded(-dy*offset,length),oy=rounded(dx*offset,length);
 x+=ox;y+=oy;tx+=ox;ty+=oy;
 int trim=app->zoom?9:6;
 s->x=x+along(dx,trim,length);s->y=y+along(dy,trim,length);
 s->tx=tx-along(dx,trim,length);s->ty=ty-along(dy,trim,length);
 /* Center the whole filled head in the open gap. For JUMP, use the gap
    beyond the crossed hole; the path itself is unchanged. */
 int back=app->zoom?5:4;
 int head=length*(jump?3:2)/4+back*128;
 /* Stagger broad zoom heads along the canonical tangent, preserving each
    arrow's true direction and keeping the two inner wing pixels separate. */
 if(app->zoom)head+=lane*sign*256;
 s->ax=x+rounded(dx*head,length);s->ay=y+rounded(dy*head,length);
 int bx=x+rounded(dx*(head-back*256),length),by=y+rounded(dy*(head-back*256),length);
 int half=app->zoom?3:2;
 int nx=along(-dy,half,length),ny=along(dx,half,length);
 s->bx=bx+nx;s->by=by+ny;s->cx=bx-nx;s->cy=by-ny;
 return true;
}
static int minimum(int a,int b){return a<b?a:b;}
static int maximum(int a,int b){return a>b?a:b;}
void ui_trail_line(DgPainter *p,const UiTrailSegment *s,int width,uint16_t ink)
{
 int dx=s->tx-s->x,dy=s->ty-s->y,square=dx*dx+dy*dy;
 if(!square)return;
 int length=root((unsigned)square*65536u),sign=dx<0 || (!dx && dy<0)?-1:1;
 int lo=maximum(minimum(s->y,s->ty)-width,p->top);
 int hi=minimum(maximum(s->y,s->ty)+width,p->bottom-1);
 int left=maximum(minimum(s->x,s->tx)-width,p->left);
 int right=minimum(maximum(s->x,s->tx)+width,p->right-1);
 for(int y=lo;y<=hi;y++)for(int x=left;x<=right;x++){
  int px=x-s->x,py=y-s->y,dot=px*dx+py*dy;
  int cross=sign*(dx*py-dy*px)*512;
  /* Half-open normal interval gives exactly 2/3 pixels on axial lines.
     Canonical direction preserves the same raster when a hop reverses. */
  if(dot>=0 && dot<=square && cross>=-width*length && cross<width*length)
   ui_rect(p,x,y,1,1,ink);
 }
}
static int cross(int ax,int ay,int bx,int by,int x,int y)
{return (bx-ax)*(y-ay)-(by-ay)*(x-ax);}
void ui_trail_arrow(DgPainter *p,const UiTrailSegment *s,uint16_t ink)
{
 int left=minimum(s->ax,minimum(s->bx,s->cx)),right=maximum(s->ax,maximum(s->bx,s->cx));
 int top=minimum(s->ay,minimum(s->by,s->cy)),bottom=maximum(s->ay,maximum(s->by,s->cy));
 for(int y=top;y<=bottom;y++)for(int x=left;x<=right;x++){
  int a=cross(s->ax,s->ay,s->bx,s->by,x,y),b=cross(s->bx,s->by,s->cx,s->cy,x,y);
  int c=cross(s->cx,s->cy,s->ax,s->ay,x,y);
  if((a>=0 && b>=0 && c>=0) || (a<=0 && b<=0 && c<=0))ui_rect(p,x,y,1,1,ink);
 }
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
   uint16_t ink=trail->player==DG_YELLOW?TRAIL_YELLOW:TRAIL_GREEN;
   ui_trail_line(p,&s,app->zoom?3:2,ink);
   ui_trail_arrow(p,&s,ink); /* Exactly one head per actual hop. */
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
