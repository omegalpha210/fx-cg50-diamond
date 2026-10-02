#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const uint8_t camps[6][10]={
 {0,1,2,3,4,5,9,10,11,12},{12,13,14,15,22,23,24,31,32,39},
 {39,46,47,54,55,56,63,64,65,66},{60,61,62,63,67,68,69,70,71,72},
 {33,40,41,48,49,50,57,58,59,60},{6,7,8,9,16,17,18,25,26,33}};
static unsigned checked;
static bool expected(uint8_t players,uint8_t player,int node)
{
 bool own=false,opponent=false;
 for(uint8_t owner=DG_RED;owner<=DG_GREEN;owner++){
  if(owner==DG_YELLOW && players==2)continue;
  for(unsigned i=0;i<10;i++)if(camps[dg_home[owner]][i]==node || camps[dg_goal[owner]][i]==node){
   if(owner==player)own=true;else opponent=true;
  }
 }
 return own || !opponent;
}
static void geometry(void)
{
 static const uint8_t shared[6]={9,12,33,39,60,63};
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  DgRules rules={DG_RULES_V2,players};
  for(int n=0;n<DG_NODES;n++){assert(dg_landing_allowed(rules,player,n)==expected(players,player,n));checked++;}
  for(unsigned i=0;i<10;i++){
   assert(dg_landing_allowed(rules,player,camps[dg_home[player]][i]));
   assert(dg_landing_allowed(rules,player,camps[dg_goal[player]][i]));
   /* Traverse actual legal empty-board steps from each starting hole. */
   bool seen[DG_NODES]={false};uint8_t queue[DG_NODES];unsigned head=0,tail=0;
   queue[tail++]=camps[dg_home[player]][i];seen[queue[0]]=true;
   while(head<tail){
    uint8_t from=queue[head++],board[DG_NODES]={0};board[from]=player;
    DgMove moves[DG_NODES];size_t count=dg_piece_moves(rules,board,player,from,moves,DG_NODES);
    for(size_t m=0;m<count;m++)if(!seen[moves[m].to]){seen[moves[m].to]=true;queue[tail++]=moves[m].to;}
   }
   for(unsigned goal=0;goal<10;goal++){assert(seen[camps[dg_goal[player]][goal]]);checked++;}
  }
  for(unsigned i=0;i<6;i++){assert(dg_landing_allowed(rules,player,shared[i])==expected(players,player,shared[i]));checked++;}
 }
 assert(dg_landing_allowed((DgRules){DG_RULES_V2,2},DG_RED,6));
 assert(!dg_landing_allowed((DgRules){DG_RULES_V2,3},DG_RED,6));
 assert(dg_landing_allowed((DgRules){DG_RULES_V2,3},DG_RED,9));
 assert(dg_landing_allowed((DgRules){DG_RULES_V2,3},DG_YELLOW,9));
 assert(!dg_landing_allowed((DgRules){DG_RULES_V2,3},DG_GREEN,9));
}
static void landings(void)
{
 unsigned crossed_forbidden=0,crossed_opponent=0,forbidden_intermediate=0,goal_exits=0;
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  DgRules rules={DG_RULES_V2,players};
  for(int from=0;from<DG_NODES;from++)if(expected(players,player,from))for(unsigned d=0;d<6;d++){
   int mid=dg_nodes[from].neighbor[d],to=dg_nodes[from].jump[d];uint8_t board[DG_NODES]={0};board[from]=player;
   if(mid>=0){assert(dg_find_move(rules,board,player,from,mid,NULL,NULL)==expected(players,player,mid));checked++;}
   if(to<0)continue;
   board[mid]=player==DG_GREEN?DG_RED:DG_GREEN;
   bool allowed=expected(players,player,to);
   assert(dg_find_move(rules,board,player,from,to,NULL,NULL)==allowed);checked++;
   if(allowed && !expected(players,player,mid))crossed_forbidden++;
   if(allowed)for(uint8_t other=DG_RED;other<=DG_GREEN;other++)
    if(other!=player && (players==3 || other!=DG_YELLOW) &&
       (dg_in_camp(mid,dg_home[other]) || dg_in_camp(mid,dg_goal[other])))crossed_opponent++;
   if(allowed && dg_in_camp(from,dg_goal[player]) && !dg_in_camp(to,dg_goal[player]))goal_exits++;
   if(allowed)continue;
   for(unsigned e=0;e<6;e++){
    int mid2=dg_nodes[to].neighbor[e],end=dg_nodes[to].jump[e];
    if(mid2<0 || end<0 || mid2==from || end==from || end==mid || !expected(players,player,end))continue;
    board[mid2]=DG_GREEN;DgMove legacy;
    if(dg_find_move(DG_LEGACY_RULES,board,player,from,end,&legacy,NULL) && legacy.hops>=2 &&
       !dg_find_move(rules,board,player,from,end,NULL,NULL))forbidden_intermediate++;
    if(mid2!=mid)board[mid2]=0;
   }
  }
 }
 /* Exhaustion finds no two-step pair with allowed endpoints and an active
    opponent-camp middle on this lattice. No extra middle filter is added. */
 assert(crossed_forbidden==0 && crossed_opponent==0 && forbidden_intermediate>0 && goal_exits>0);
 printf("landing audit: %u shared opponent middles crossed legally, %u intermediate exclusions, %u legal goal exits\n",crossed_opponent,forbidden_intermediate,goal_exits);
}
static void wins(void)
{
 const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  for(unsigned level=0;level<3;level++){
   DgGame game;assert(dg_new(&game,players,levels[level],0,31));memset(game.pos.board,0,DG_NODES);
   int vacancy=-1,from=-1;
   for(unsigned i=0;i<10 && from<0;i++)for(unsigned d=0;d<6 && from<0;d++){
    int n=dg_nodes[camps[dg_goal[player]][i]].neighbor[d];
    if(n>=0 && !dg_in_camp(n,dg_goal[player]) && expected(players,player,n)){vacancy=camps[dg_goal[player]][i];from=n;}
   }
   assert(from>=0);
   for(unsigned i=0;i<10;i++)if(camps[dg_goal[player]][i]!=vacancy)game.pos.board[camps[dg_goal[player]][i]]=player;
   game.pos.board[from]=player;
   for(uint8_t other=DG_RED;other<=DG_GREEN;other++){
    if(other==player || (players==2 && other==DG_YELLOW))continue;
    unsigned placed=0;
    for(unsigned pass=0;pass<2 && placed<10;pass++)for(int n=0;n<DG_NODES && placed<10;n++)
     if(!game.pos.board[n] && n!=vacancy && expected(players,other,n) && !dg_in_camp(n,dg_goal[other]) &&
        (pass || dg_in_camp(n,dg_home[other]))){game.pos.board[n]=other;placed++;}
    assert(placed==10);
   }
   for(uint8_t slot=0;slot<players;slot++)if(game.order[slot]==player)game.pos.turn=slot;
   assert(dg_game_valid(&game));DgMove move;uint32_t rng;DgAiStats stats;
   assert(dg_ai_choose(&game,0,NULL,NULL,&move,&rng,&stats));
   assert(stats.immediate_win && stats.nodes==0 && rng==game.pos.rng);
   assert(move.from==from && move.to==vacancy);assert(dg_commit(&game,&move));
   assert(dg_won(game.pos.board,player) && game.pos.winner==player);checked++;
  }
 }
}
static void assist(void)
{
 DgApp app;dg_app_init(&app,(DgHooks){0},17);app.screen=DG_GAME;app.archive.active=1;
 assert(dg_new(&app.archive.game,3,DG_EASY,0,17));
 DgGame *g=&app.archive.game;int source=-1,to=-1;
 for(int n=0;n<DG_NODES && source<0;n++)if(!g->pos.board[n] && !dg_landing_allowed(dg_rules(g),DG_RED,n))
  for(unsigned d=0;d<6;d++){int adjacent=dg_nodes[n].neighbor[d];if(adjacent>=0 && dg_landing_allowed(dg_rules(g),DG_RED,adjacent) && !g->pos.board[adjacent]){source=adjacent;to=n;break;}}
 assert(source>=0);int own=camps[dg_home[DG_RED]][0];g->pos.board[own]=0;g->pos.board[source]=DG_RED;
 app.selected=(uint8_t)source;app.cursor=(uint8_t)to;app.archive.assist=1;
 dg_app_preview(&app);assert(!app.path.length);
 DgGame before=*g;assert(dg_app_key(&app,DGK_EXE));assert(!strcmp(app.notice,"OPPONENT CAMP"));
 assert(!memcmp(g,&before,sizeof before) && app.modal==DG_MODAL_NONE);
 DgMove moves[DG_NODES];size_t count=dg_piece_moves(dg_rules(g),g->pos.board,DG_RED,(uint8_t)source,moves,DG_NODES);
 for(size_t i=0;i<count;i++)assert(moves[i].to!=to);
}
int main(void)
{geometry();landings();wins();assist();printf("V2 rules PASS: %u checks; all homes/goals, shared corners, reachability, 15 immediate wins and Assist\n",checked);return 0;}
