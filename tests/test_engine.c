#include "diamond.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
void test_ai(void);
static unsigned long checks;
#define CHECK(x) do{assert(x);checks++;}while(0)
static const int delta[6][2]={{1,0},{0,1},{-1,1},{-1,0},{0,-1},{1,-1}};
static void topology(void)
{
 unsigned camps[6]={0},center=0,shared=0,degree[7]={0};
 for(int n=0;n<DG_NODES;n++){
  CHECK(dg_coord(dg_nodes[n].q,dg_nodes[n].r)==n);
  for(int k=n+1;k<DG_NODES;k++)CHECK(dg_nodes[n].q!=dg_nodes[k].q || dg_nodes[n].r!=dg_nodes[k].r);
  if(!dg_nodes[n].region)center++;
  unsigned memberships=0,deg=0;
  for(int a=0;a<6;a++)if(dg_in_camp(n,(uint8_t)a)){camps[a]++;memberships++;}
  shared+=memberships==2;
  for(int d=0;d<6;d++){
   int next=dg_coord(dg_nodes[n].q+delta[d][0],dg_nodes[n].r+delta[d][1]);
   CHECK(dg_nodes[n].neighbor[d]==next);
   int to=dg_coord(dg_nodes[n].q+2*delta[d][0],dg_nodes[n].r+2*delta[d][1]);
   CHECK(dg_nodes[n].jump[d]==(next<0?-1:to));
   if(next>=0){CHECK(dg_nodes[next].neighbor[(d+3)%6]==n);deg++;}
  }
  degree[deg]++;
  for(int t=0;t<DG_NODES;t++){CHECK(dg_distance[n][t]!=DG_NONE);CHECK(dg_distance[n][t]==dg_distance[t][n]);}
  int opposite=dg_coord(-dg_nodes[n].q,-dg_nodes[n].r);CHECK(opposite>=0);
  for(int a=0;a<6;a++)CHECK(dg_in_camp(n,(uint8_t)a)==dg_in_camp(opposite,(uint8_t)((a+3)%6)));
  bool visited[DG_NODES]={false};int queue[DG_NODES],head=0,tail=0;queue[tail++]=n;visited[n]=true;
  while(head<tail){int at=queue[head++];for(int d=0;d<4;d++){int to=dg_nodes[at].nav[d];CHECK(to>=0 && to<DG_NODES);if(!visited[to]){visited[to]=true;queue[tail++]=to;}}}
  CHECK(tail==DG_NODES);
 }
 for(int a=0;a<6;a++)CHECK(camps[a]==10);
 CHECK(center==19 && shared==6);CHECK(degree[2]==6 && degree[4]==24 && degree[5]==6 && degree[6]==37);
 CHECK(dg_home[DG_RED]==3 && dg_home[DG_YELLOW]==5 && dg_home[DG_GREEN]==1);
 CHECK(dg_goal[DG_RED]==0 && dg_goal[DG_YELLOW]==2 && dg_goal[DG_GREEN]==4);
}
static void local_moves(void)
{
 uint8_t board[DG_NODES];DgMove move;DgPath path;
 for(int from=0;from<DG_NODES;from++)for(int d=0;d<6;d++){
  memset(board,0,sizeof board);board[from]=DG_RED;
  int mid=dg_nodes[from].neighbor[d],to=dg_nodes[from].jump[d];
  if(mid>=0){CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,mid,&move,&path));CHECK(move.type==DG_STEP && path.length==2);board[mid]=DG_GREEN;CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,mid,NULL,NULL));}
  if(to>=0){
   CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,to,&move,&path));CHECK(move.type==DG_JUMP && move.hops==1 && path.length==2);
   uint8_t original[DG_NODES];memcpy(original,board,sizeof board);CHECK(dg_apply(DG_LEGACY_RULES,board,DG_RED,&move));CHECK(board[mid]==DG_GREEN);
   memcpy(board,original,sizeof board);board[mid]=DG_RED;CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,to,NULL,NULL));
   board[to]=DG_YELLOW;CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,to,NULL,NULL));board[to]=0;board[mid]=0;CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,to,NULL,NULL));
  }
 }
 memset(board,0,sizeof board);int from=dg_coord(-2,0),a=dg_coord(0,0),b=dg_coord(0,2);
 board[from]=DG_RED;board[dg_coord(-1,0)]=DG_GREEN;board[dg_coord(0,1)]=DG_RED;
 CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,a,&move,&path));CHECK(move.hops==1);
 CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,b,&move,&path));CHECK(move.hops==2 && path.node[1]==a);
 /* Two equal shortest paths: lexicographic landing ID wins, cycles terminate. */
 memset(board,0,sizeof board);from=dg_coord(0,0);board[from]=DG_RED;
 board[dg_coord(1,0)]=DG_GREEN;board[dg_coord(2,1)]=DG_GREEN;
 board[dg_coord(0,1)]=DG_YELLOW;board[dg_coord(1,2)]=DG_YELLOW;
 CHECK(dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,dg_coord(2,2),&move,&path));
 CHECK(move.type==DG_JUMP && move.hops==2 && path.node[1]==dg_coord(2,0));
 CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,from,NULL,NULL));
 /* Empty STEP intermediate cannot be used as the start of a jump. */
 memset(board,0,sizeof board);from=dg_coord(-2,0);board[from]=DG_RED;board[dg_coord(0,0)]=DG_GREEN;
 CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,dg_coord(1,0),NULL,NULL));
 CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,-1,0,NULL,NULL));CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_RED,from,DG_NODES,NULL,NULL));
 CHECK(!dg_find_move(DG_LEGACY_RULES,board,DG_GREEN,from,0,NULL,NULL));
}
/* Coordinate-based independent endpoint oracle (not the table neighbors). */
static bool oracle_allowed(const DgGame *game,int node)
{
 if(node<0)return false;
 if(game->rules_revision==DG_RULES_V1)return true;
 uint8_t player=dg_current(game);unsigned mask=(unsigned)dg_nodes[node].region;
 unsigned own=(1u<<dg_home[player])|(1u<<dg_goal[player]);
 unsigned active=game->players==3?63u:27u;
 return (mask&own)!=0 || (mask&active)==0;
}
static void oracle(const DgGame *game,int from,bool *reachable)
{
 const uint8_t *board=game->pos.board;
 memset(reachable,0,DG_NODES*sizeof *reachable);int queue[DG_NODES],head=0,tail=0;bool seen[DG_NODES]={false};queue[tail++]=from;seen[from]=true;
 for(int d=0;d<6;d++){int to=dg_coord(dg_nodes[from].q+delta[d][0],dg_nodes[from].r+delta[d][1]);if(oracle_allowed(game,to) && !board[to])reachable[to]=true;}
 while(head<tail){int at=queue[head++];for(int d=0;d<6;d++){
  int mid=dg_coord(dg_nodes[at].q+delta[d][0],dg_nodes[at].r+delta[d][1]);int to=dg_coord(dg_nodes[at].q+2*delta[d][0],dg_nodes[at].r+2*delta[d][1]);
  if(mid<0 || !oracle_allowed(game,to) || mid==from || !board[mid] || (to!=from && board[to]))continue;
  if(to!=from)reachable[to]=true;
  if(!seen[to]){seen[to]=true;queue[tail++]=to;}
 }}
 reachable[from]=false;
}
static DgMove moves[DG_MAX_MOVES];
static void properties(void)
{
 uint32_t rng=8712345;DgGame game;CHECK(dg_new(&game,3,DG_EASY,0,rng));unsigned chains=0;
 for(int sample=0;sample<5000;sample++){
  if(game.pos.winner || sample%300==0){CHECK(dg_new(&game,(sample/300&1)?2:3,DG_EASY,0,dg_random(&rng)));game.rules_revision=sample/600&1?DG_RULES_V1:DG_RULES_V2;}
  uint8_t p=dg_current(&game);size_t count=dg_generate(dg_rules(&game),game.pos.board,p,moves,DG_MAX_MOVES);CHECK(count>0 && count<=DG_MAX_MOVES);
  bool endpoints[DG_NODES][DG_NODES]={false};
  for(size_t i=0;i<count;i++){
   DgMove m=moves[i];CHECK(!endpoints[m.from][m.to]);endpoints[m.from][m.to]=true;
   DgMove found;DgPath path;CHECK(dg_find_move(dg_rules(&game),game.pos.board,p,m.from,m.to,&found,&path));CHECK(memcmp(&m,&found,sizeof m)==0);
   CHECK(path.length==m.hops+1 && path.node[0]==m.from && path.node[path.length-1]==m.to);
   uint8_t changed[DG_NODES];memcpy(changed,game.pos.board,sizeof changed);CHECK(dg_apply(dg_rules(&game),changed,p,&m));
   for(int n=0;n<DG_NODES;n++)CHECK(changed[n]==(n==m.from?0:n==m.to?p:game.pos.board[n]));
   changed[m.to]=0;changed[m.from]=p;CHECK(memcmp(changed,game.pos.board,sizeof changed)==0);
   if(m.hops>1)chains++;
   for(int hop=1;hop<path.length;hop++){
    int a=path.node[hop-1],b=path.node[hop];int dq=dg_nodes[b].q-dg_nodes[a].q,dr=dg_nodes[b].r-dg_nodes[a].r;bool legal=false;
    for(int d=0;d<6;d++)if(dq==delta[d][0]*(m.type==DG_JUMP?2:1) && dr==delta[d][1]*(m.type==DG_JUMP?2:1)){
     int mid=dg_coord(dg_nodes[a].q+delta[d][0],dg_nodes[a].r+delta[d][1]);legal=m.type==DG_STEP || (mid!=m.from && game.pos.board[mid]!=0);
    }
    CHECK(legal && (b==m.from || game.pos.board[b]==0));
   }
  }
  /* Fully compare the endpoint sets for each of the ten moving pieces. */
  for(int n=0;n<DG_NODES;n++)if(game.pos.board[n]==p){bool expected[DG_NODES];oracle(&game,n,expected);for(int t=0;t<DG_NODES;t++)CHECK(expected[t]==endpoints[n][t]);}
  DgMove chosen=moves[dg_random(&rng)%count];CHECK(dg_commit(&game,&chosen));CHECK(dg_game_valid(&game));
 }
 CHECK(chains>1000);printf("5000 reachable boards; %u chained moves validated\n",chains);
}
static void lifecycle(void)
{
 for(uint8_t players=2;players<=3;players++)for(uint8_t slot=0;slot<players;slot++){
  DgGame game,original;CHECK(dg_new(&game,players,DG_EASY,slot,713));original=game;
  for(int i=0;i<slot;i++){size_t count=dg_generate(dg_rules(&game),game.pos.board,dg_current(&game),moves,DG_MAX_MOVES);CHECK(count>0);CHECK(dg_commit(&game,&moves[0]));}
  CHECK(dg_current(&game)==DG_RED && !dg_undo(&game));DgPosition snap=game.pos;
  size_t count=dg_generate(dg_rules(&game),game.pos.board,DG_RED,moves,DG_MAX_MOVES);CHECK(count>0);CHECK(dg_commit(&game,&moves[0]));
  while(dg_current(&game)!=DG_RED){count=dg_generate(dg_rules(&game),game.pos.board,dg_current(&game),moves,DG_MAX_MOVES);CHECK(count>0);CHECK(dg_commit(&game,&moves[0]));(void)dg_random(&game.pos.rng);}
  CHECK(dg_game_valid(&game));CHECK(dg_undo(&game));CHECK(memcmp(&snap,&game.pos,sizeof snap)==0);CHECK(!dg_undo(&game));
  count=dg_generate(dg_rules(&game),game.pos.board,DG_RED,moves,DG_MAX_MOVES);CHECK(count>0);CHECK(dg_commit(&game,&moves[0]));CHECK(game.undo_valid);
  dg_restart(&game);CHECK(memcmp(&game,&original,sizeof game)==0);
 }
 for(uint8_t p=DG_RED;p<=DG_GREEN;p++){
  uint8_t board[DG_NODES]={0};for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[p]))board[n]=p;
  CHECK(dg_won(board,p));for(int n=0;n<DG_NODES;n++)if(board[n]){board[n]=0;break;}CHECK(!dg_won(board,p));
 }
 unsigned order_seen=0;for(uint32_t seed=1;seed<100;seed++){DgGame g;CHECK(dg_new(&g,3,1,1,seed));order_seen|=g.order[0]==DG_GREEN?1u:2u;}
 CHECK(order_seen==3);
}
int main(void)
{topology();local_moves();lifecycle();properties();test_ai();printf("Engine checks: %lu PASS\n",checks);return 0;}
