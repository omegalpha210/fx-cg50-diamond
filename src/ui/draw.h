#ifndef DIAMOND_DRAW_H
#define DIAMOND_DRAW_H
#include "ui.h"

#define DG_RGB(r,g,b) ((uint16_t)(((r)<<11)|((g)<<6)|(b)))
/* SOKOBAN surface palette; DIFF EQ focus and semantic actions. */
enum {
 UI_INK=DG_RGB(3,5,7),UI_MUTED=DG_RGB(12,14,15),UI_LINE=DG_RGB(24,26,26),
 UI_PAPER=DG_RGB(29,30,30),UI_WHITE=0xffff,UI_BLACK=0,
 UI_BLUE=DG_RGB(3,10,24),UI_FOCUS=DG_RGB(0,19,29),UI_SELECTED=DG_RGB(25,29,31),
 UI_NEXT=0x07ff,UI_UNDO=0xf81f,UI_RESTART=0xffe0,UI_RUN=0xf800,
 /* Original DIAMOND piece / route RGB565 values are deliberately retained. */
 PIECE_RED=DG_RGB(28,3,3),PIECE_YELLOW=DG_RGB(31,24,0),PIECE_GREEN=DG_RGB(0,20,8),
 UI_RED_TEXT=PIECE_RED,UI_GREEN_TEXT=PIECE_GREEN,UI_YELLOW_TEXT=DG_RGB(21,16,0),
 UI_GOLD=UI_YELLOW_TEXT, /* Compatibility alias; one gold text/trail role. */
 BOARD_CYAN=DG_RGB(0,23,26),BOARD_BLUE=DG_RGB(0,12,25),BOARD_INK=DG_RGB(3,5,8),
 UI_SOFTKEY_TOP=204,UI_FONT_HEIGHT=11,UI_CARD_X=7,UI_CARD_W=187,UI_CARD_GAP=8,
 UI_BOARD_TOP=26,UI_BOARD_BOTTOM=204
};
typedef struct {int x,y,w,h;} UiBox;
typedef struct {UiBox box;unsigned count;char line[4][48];} UiNotice;
typedef struct {const DgCanvas *canvas;int left,top,right,bottom;} DgPainter;
typedef struct {int x,y,tx,ty,ax,ay,bx,by,cx,cy;} UiTrailSegment;
void ui_rect(DgPainter *p,int x,int y,int w,int h,uint16_t ink);
void ui_line(DgPainter *p,int x,int y,int tx,int ty,uint16_t ink);
void ui_border(DgPainter *p,int x,int y,int w,int h,uint16_t ink,int thick);
int ui_text_width(const char *s,int numerator,int denominator);
void ui_text(DgPainter *p,int x,int y,const char *s,uint16_t ink,int numerator,int denominator);
void ui_center(DgPainter *p,int x,int y,int w,const char *s,uint16_t ink,int numerator,int denominator);
void ui_title(DgPainter *p,const char *s,const char *right);
void ui_softkeys(DgPainter *p,const char *const labels[6]);
void ui_disc(DgPainter *p,int x,int y,int radius,uint16_t ink);
void ui_ring(DgPainter *p,int x,int y,int radius,uint16_t ink);
uint16_t ui_piece_color(uint8_t player);
uint16_t ui_actor_color(uint8_t player);
uint16_t ui_level_color(uint8_t level);
void ui_piece(DgPainter *p,int x,int y,int radius,uint8_t player,bool human);
const char *ui_level_label(uint8_t level);
void ui_menu_screen(DgPainter *p,const DgApp *app,const char **labels);
void ui_game_screen(DgPainter *p,const DgApp *app,const char **labels);
UiBox ui_thinking_box(void);
void ui_thinking(DgPainter *p,const DgApp *app);
void ui_notice_layout(const DgApp *app,UiNotice *notice);
bool ui_trail_segment(const DgApp *app,int from,int to,int lane,UiTrailSegment *segment);
void ui_trails(DgPainter *p,const DgApp *app);
void ui_animation_position(const DgApp *app,int *x,int *y);
#endif
