#include "draw.h"
#include <string.h>

void ui_notice_layout(const DgApp *app,UiNotice *notice)
{
 memset(notice,0,sizeof *notice);const char *text=app->notice;
 if(!strcmp(text,"CPU HAS NO MOVE"))text="AI HAS NO MOVE";
 else if(!strcmp(text,"CPU MOVE REJECTED"))text="AI MOVE REJECTED";
 int limit=app->screen==DG_GAME?103:372;
 while(*text && notice->count<4){
  char *line=notice->line[notice->count];unsigned length=0,last_space=0;
  while(text[length] && length<47){
   line[length]=text[length];line[length+1]=0;
   if(ui_text_width(line,1,1)>limit){line[length]=0;break;}
   if(text[length]==' ')last_space=length;
   length++;
  }
  if(text[length] && last_space){length=last_space;line[length]=0;}
  int width=ui_text_width(line,1,1);if(width>notice->box.w)notice->box.w=width;
  notice->count++;text+=length;while(*text==' ')text++;
 }
 notice->box.x=app->screen==DG_GAME?6:10;notice->box.y=app->screen==DG_GAME?84:171;
 notice->box.w+=4;notice->box.h=notice->count?(int)notice->count*13+2:0;
}

void dg_render_thinking(const DgApp *app,const DgCanvas *canvas)
{DgPainter p={canvas,0,0,396,224};if(app->screen==DG_GAME && !app->modal)ui_thinking(&p,app);}

void dg_render(const DgApp *app,const DgCanvas *canvas)
{
 DgPainter p={canvas,0,0,396,224};ui_rect(&p,0,0,396,224,UI_PAPER);
 const char *labels[6]={"","","","","",""};
 if(app->screen==DG_GAME)ui_game_screen(&p,app,labels);
 else ui_menu_screen(&p,app,labels);
 if(app->notice[0]){
  UiNotice notice;ui_notice_layout(app,&notice);UiBox b=notice.box;
  ui_rect(&p,b.x,b.y,b.w,b.h,UI_WHITE);
  for(unsigned row=0;row<notice.count;row++)ui_text(&p,b.x+2,b.y+2+(int)row*13,notice.line[row],UI_RUN,1,1);
 }
 ui_softkeys(&p,labels);
}
