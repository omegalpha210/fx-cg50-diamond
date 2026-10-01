#include "ui.h"

bool dg_app_resumable(const DgApp *app)
{return app->archive.active && !app->archive.game.pos.winner && dg_game_valid(&app->archive.game);}
bool dg_setup_resume(const DgApp *app)
{return dg_app_resumable(app) && app->archive.game.players==app->players;}
unsigned dg_entry_count(const DgApp *app)
{return dg_setup_resume(app)?5u:4u;}
int dg_entry_action(const DgApp *app,unsigned row)
{
 if(dg_setup_resume(app)){if(!row)return DG_ENTRY_RESUME;row--;}
 static const int actions[4]={DG_ENTRY_NEW,DG_ENTRY_LEVEL,DG_ENTRY_SLOT,DG_ENTRY_ASSIST};
 return row<4?actions[row]:-1;
}
unsigned dg_entry_row(const DgApp *app,int action)
{for(unsigned row=0;row<dg_entry_count(app);row++)if(dg_entry_action(app,row)==action)return row;return 0;}
