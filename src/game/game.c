#include "diamond.h"
#include <string.h>
uint8_t dg_current(const DgGame *game)
{return game && (game->players==2 || game->players==3) && game->pos.turn<game->players?game->order[game->pos.turn]:DG_EMPTY;}
bool dg_position_valid(const DgGame *game,const DgPosition *pos,bool undo)
{
 if(!game || !pos || (game->players!=2 && game->players!=3) || pos->turn>=game->players || !pos->rng || pos->winner>DG_GREEN)return false;
 unsigned count[4]={0};
 for(int i=0;i<DG_NODES;i++){if(pos->board[i]>DG_GREEN)return false;count[pos->board[i]]++;}
 if(count[DG_RED]!=10 || count[DG_GREEN]!=10 || count[DG_YELLOW]!=(game->players==3?10u:0u))return false;
 if(undo && (pos->winner || game->order[pos->turn]!=DG_RED))return false;
 if(pos->winner && (pos->winner!=game->order[pos->turn] || !dg_won(pos->board,pos->winner)))return false;
 for(int p=1;p<=3;p++)if(p!=pos->winner && dg_won(pos->board,(uint8_t)p))return false;
 return true;
}
bool dg_game_valid(const DgGame *game)
{
 if(!game || (game->players!=2 && game->players!=3) || !dg_level_valid(game->level) || game->human_slot>=game->players || game->undo_valid>1 || !game->seed || !game->initial_rng)return false;
 unsigned seen=0;for(int i=0;i<game->players;i++){
  unsigned p=game->order[i];if(p<1 || p>3 || (seen&(1u<<p)))return false;seen|=1u<<p;
 }
 if(seen!=(game->players==2?10u:14u) || game->order[game->human_slot]!=DG_RED || (game->players==2 && game->order[2]!=DG_NONE))return false;
 if(!dg_position_valid(game,&game->pos,false))return false;
 if(game->undo_valid && (!dg_position_valid(game,&game->undo,true) || game->undo.turns>=game->pos.turns))return false;
 return true;
}
static void initial(DgGame *game)
{
 memset(&game->pos,0,sizeof game->pos);memset(&game->undo,0,sizeof game->undo);game->undo_valid=0;
 game->pos.rng=game->initial_rng;
 for(int i=0;i<DG_NODES;i++)for(int p=1;p<=3;p++)if((p!=DG_YELLOW || game->players==3) && dg_in_camp(i,dg_home[p]))game->pos.board[i]=(uint8_t)p;
}
bool dg_new(DgGame *game,uint8_t players,uint8_t level,uint8_t slot,uint32_t seed)
{
 if(!game || (players!=2 && players!=3) || !dg_level_valid(level) || slot>=players)return false;
 memset(game,0,sizeof *game);game->players=players;game->level=level;game->human_slot=slot;game->seed=seed?seed:1;
 uint32_t rng=game->seed;game->order[slot]=DG_RED;
 if(players==2){game->order[1-slot]=DG_GREEN;game->order[2]=DG_NONE;}
 else{uint8_t first=(dg_random(&rng)&1)?DG_GREEN:DG_YELLOW;for(int i=0;i<3;i++)if(i!=slot){game->order[i]=first;first=first==DG_GREEN?DG_YELLOW:DG_GREEN;}}
 game->initial_rng=rng;initial(game);return true;
}
void dg_restart(DgGame *game){if(game && (game->players==2 || game->players==3))initial(game);}
bool dg_commit(DgGame *game,const DgMove *move)
{
 if(!dg_game_valid(game) || game->pos.winner)return false;
 DgMove valid;uint8_t player=dg_current(game);
 if(!move || !dg_find_move(game->pos.board,player,move->from,move->to,&valid,NULL) || move->type!=valid.type || move->hops!=valid.hops)return false;
 if(player==DG_RED){game->undo=game->pos;game->undo_valid=1;}
 game->pos.board[valid.from]=0;game->pos.board[valid.to]=player;game->pos.turns++;
 if(dg_won(game->pos.board,player))game->pos.winner=player;
 else game->pos.turn=(uint8_t)((game->pos.turn+1)%game->players);
 return true;
}
bool dg_undo(DgGame *game)
{
 if(!dg_game_valid(game) || game->pos.winner || dg_current(game)!=DG_RED || !game->undo_valid)return false;
 game->pos=game->undo;game->undo_valid=0;memset(&game->undo,0,sizeof game->undo);return true;
}
