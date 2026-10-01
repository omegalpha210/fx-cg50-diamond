#include "draw.h"
#include <string.h>

void dg_render(const DgApp *app,const DgCanvas *canvas)
{
 DgPainter p={canvas,0,0,396,224};ui_rect(&p,0,0,396,224,UI_PAPER);
 const char *labels[6]={"","","","","",""};
 if(app->screen==DG_GAME)ui_game_screen(&p,app,labels);
 else ui_menu_screen(&p,app,labels);
 if(app->notice[0]){
  const char *notice=app->notice;
  if(!strcmp(notice,"CPU HAS NO MOVE"))notice="AI HAS NO MOVE";
  else if(!strcmp(notice,"CPU MOVE REJECTED"))notice="AI MOVE REJECTED";
  /* DIFF EQ graph_message: measured top-left backing, red normal text. */
  int width=ui_text_width(notice,1,1)+12;
  ui_rect(&p,6,28,width,19,UI_WHITE);ui_rect(&p,6,28,2,19,UI_RUN);
  ui_text(&p,12,32,notice,UI_RUN,1,1);
 }
 ui_softkeys(&p,labels);
}
