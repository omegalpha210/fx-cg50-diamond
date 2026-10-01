#include "draw.h"
#include "font_data.h"
#include <string.h>

void ui_rect(DgPainter *p,int x,int y,int w,int h,uint16_t ink)
{
 if(x<p->left){w-=p->left-x;x=p->left;}
 if(y<p->top){h-=p->top-y;y=p->top;}
 if(x+w>p->right)w=p->right-x;
 if(y+h>p->bottom)h=p->bottom-y;
 if(w>0 && h>0)p->canvas->rect(p->canvas->context,x,y,w,h,ink);
}
void ui_line(DgPainter *p,int x,int y,int tx,int ty,uint16_t ink)
{
 int dx=tx-x,dy=ty-y,sx=dx<0?-1:1,sy=dy<0?-1:1;
 if(dx<0)dx=-dx;
 if(dy<0)dy=-dy;
 int err=dx-dy;
 for(;;){ui_rect(p,x,y,1,1,ink);if(x==tx && y==ty)break;
  int twice=2*err;if(twice>-dy){err-=dy;x+=sx;}if(twice<dx){err+=dx;y+=sy;}}
}
void ui_border(DgPainter *p,int x,int y,int w,int h,uint16_t ink,int thick)
{
 ui_rect(p,x,y,w,thick,ink);ui_rect(p,x,y+h-thick,w,thick,ink);
 ui_rect(p,x,y,thick,h,ink);ui_rect(p,x+w-thick,y,thick,h,ink);
}
int ui_text_width(const char *s,int numerator,int denominator)
{
 int width=0;
 for(;*s;s++){
  unsigned ch=(unsigned char)*s;if(ch<32 || ch>126)ch='?';
  width+=glyph_width[ch-32]+1;
 }
 return width?(width-1)*numerator/denominator:0;
}
void ui_text(DgPainter *p,int x,int y,const char *s,uint16_t ink,int numerator,int denominator)
{
 /* SOKOBAN text_ratio, with adjacent row pixels submitted as one run.
    The exact atlas and integer scale rule are shared by host and SH. */
 int advance=0;
 for(;*s;s++){
  unsigned ch=(unsigned char)*s;if(ch<32 || ch>126)ch='?';ch-=32;
  for(int row=0;row<UI_FONT_HEIGHT;row++){
   int col=0;
   while(col<glyph_width[ch]){
    if(!(glyph_rows[ch][row]&(1u<<col))){col++;continue;}
    int first=col++;
    while(col<glyph_width[ch] && (glyph_rows[ch][row]&(1u<<col)))col++;
    int x1=(advance+first)*numerator/denominator,x2=(advance+col)*numerator/denominator;
    int y1=row*numerator/denominator,y2=(row+1)*numerator/denominator;
    ui_rect(p,x+x1,y+y1,x2-x1,y2-y1,ink);
   }
  }
  advance+=glyph_width[ch]+1;
 }
}
void ui_center(DgPainter *p,int x,int y,int w,const char *s,uint16_t ink,int numerator,int denominator)
{ui_text(p,x+(w-ui_text_width(s,numerator,denominator))/2,y,s,ink,numerator,denominator);}
void ui_title(DgPainter *p,const char *s,const char *right)
{
 ui_rect(p,0,0,396,24,UI_INK);ui_text(p,8,6,s,UI_WHITE,1,1);
 if(right)ui_text(p,388-ui_text_width(right,1,1),6,right,DG_RGB(25,28,28),1,1);
}
void ui_softkeys(DgPainter *p,const char *const labels[6])
{
 ui_rect(p,0,UI_SOFTKEY_TOP,396,20,UI_WHITE);
 for(int i=0;i<6;i++){
  const char *s=labels[i];if(!s[0])continue;
  uint16_t bg=UI_BLUE,fg=UI_WHITE;
  if(!strcmp(s,"NEXT")){bg=UI_NEXT;fg=UI_BLACK;}
  else if(!strcmp(s,"UNDO")){bg=UI_UNDO;fg=UI_BLACK;}
  else if(!strcmp(s,"RESTART")){bg=UI_RESTART;fg=UI_BLACK;}
  else if(!strcmp(s,"RULES"))bg=UI_BLACK;
  else if(!strcmp(s,"OPEN"))bg=UI_INK;
  else if(!strcmp(s,"PLAY") || !strcmp(s,"MOVE") || !strcmp(s,"SELECT") || !strcmp(s,"NEW"))bg=UI_RUN;
  ui_rect(p,i*66+1,205,64,18,bg);ui_center(p,i*66+1,209,64,s,fg,1,1);
 }
}
void ui_disc(DgPainter *p,int x,int y,int radius,uint16_t ink)
{
 for(int dy=-radius;dy<=radius;dy++){
  int width=0;while((width+1)*(width+1)+dy*dy<=radius*radius)width++;
  ui_rect(p,x-width,y+dy,2*width+1,1,ink);
 }
}
void ui_ring(DgPainter *p,int x,int y,int radius,uint16_t ink)
{
 for(int dy=-radius;dy<=radius;dy++)for(int dx=-radius;dx<=radius;dx++){
  int sq=dx*dx+dy*dy;
  if(sq<=radius*radius && sq>=(radius-1)*(radius-1))ui_rect(p,x+dx,y+dy,1,1,ink);
 }
}
uint16_t ui_piece_color(uint8_t player)
{return player==DG_RED?PIECE_RED:player==DG_YELLOW?PIECE_YELLOW:PIECE_GREEN;}
uint16_t ui_actor_color(uint8_t player)
{return player==DG_YELLOW?UI_GOLD:ui_piece_color(player);}
uint16_t ui_level_color(uint8_t level)
{return level==DG_EASY?PIECE_GREEN:level==DG_NORMAL?UI_GOLD:PIECE_RED;}
void ui_piece(DgPainter *p,int x,int y,int radius,uint8_t player,bool human)
{
 ui_disc(p,x,y,radius,UI_BLACK);ui_disc(p,x,y,radius-1,ui_piece_color(player));
 if(human){
  ui_disc(p,x,y-5,4,UI_WHITE);
  /* Integer upper-body silhouette: rounded shoulders, flat lower edge. */
  DgPainter shoulders=*p;shoulders.top=y+1;shoulders.bottom=y+10;
  ui_disc(&shoulders,x,y+9,9,UI_WHITE);
 }else ui_center(p,x-radius,y-UI_FONT_HEIGHT/2,2*radius+1,"AI",UI_BLACK,1,1);
}
const char *ui_level_label(uint8_t level)
{
 return level==DG_EASY?"EASY":level==DG_NORMAL?"NORMAL":level==DG_HARD?"HARD":"?";
}
