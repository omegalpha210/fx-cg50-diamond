#include "diamond.h"
#include <string.h>

uint32_t dg_random(uint32_t *state)
{
 uint32_t x=*state?*state:UINT32_C(0x6d2b79f5);
 x^=x<<13;x^=x>>17;x^=x<<5;*state=x;return x;
}
int dg_coord(int q,int r)
{
 for(int i=0;i<DG_NODES;i++)if(dg_nodes[i].q==q && dg_nodes[i].r==r)return i;
 return -1;
}
bool dg_in_camp(int node,uint8_t camp)
{return node>=0 && node<DG_NODES && camp<6 && (((uint8_t)dg_nodes[node].region&(1u<<camp))!=0);}
/* Once the source is vacated, every other stationary piece is unchanged.
   A jump state is therefore determined solely by the current landing node.
   BFS visited-state pruning preserves every reachable endpoint, including
   every intermediate landing, without imposing a route rule. */
static void reach(const uint8_t *board,int from,int8_t *parent,uint8_t *depth)
{
 uint8_t queue[DG_NODES];int head=0,tail=0;
 memset(parent,-1,DG_NODES);memset(depth,DG_NONE,DG_NODES);
 parent[from]=(int8_t)from;depth[from]=0;queue[tail++]=(uint8_t)from;
 while(head<tail){
  int current=queue[head++],next[6],count=0;
  for(int d=0;d<6;d++){
   int mid=dg_nodes[current].neighbor[d],to=dg_nodes[current].jump[d];
   if(mid<0 || to<0 || mid==from || board[mid]==DG_EMPTY)continue;
   if(to!=from && board[to]!=DG_EMPTY)continue;
   next[count++]=to;
  }
  /* Deterministic lexicographic shortest path. */
  for(int i=1;i<count;i++){int v=next[i],j=i;while(j && next[j-1]>v){next[j]=next[j-1];j--;}next[j]=v;}
  for(int i=0;i<count;i++){
   int to=next[i];if(depth[to]!=DG_NONE)continue;
   parent[to]=(int8_t)current;depth[to]=(uint8_t)(depth[current]+1);queue[tail++]=(uint8_t)to;
  }
 }
}
size_t dg_piece_moves(const uint8_t *board,uint8_t player,uint8_t from,DgMove *out,size_t cap)
{
 if(!board || player<DG_RED || player>DG_GREEN || from>=DG_NODES || board[from]!=player)return 0;
 int8_t parent[DG_NODES];uint8_t depth[DG_NODES],step[DG_NODES]={0};
 reach(board,from,parent,depth);
 for(int d=0;d<6;d++){int n=dg_nodes[from].neighbor[d];if(n>=0 && board[n]==DG_EMPTY)step[n]=1;}
 size_t count=0;
 for(int n=0;n<DG_NODES;n++){
  if(n==from || (!step[n] && depth[n]==DG_NONE))continue;
  if(count<cap && out)out[count]=(DgMove){from,(uint8_t)n,step[n]?DG_STEP:DG_JUMP,step[n]?1:depth[n]};
  count++;
 }
 return count;
}
size_t dg_generate(const uint8_t *board,uint8_t player,DgMove *out,size_t cap)
{
 if(!board || player<DG_RED || player>DG_GREEN)return 0;
 size_t count=0;
 for(int n=0;n<DG_NODES;n++)if(board[n]==player){
  size_t used=count<cap?count:cap;
  count+=dg_piece_moves(board,player,(uint8_t)n,out?out+used:NULL,cap-used);
 }
 return count;
}
bool dg_find_move(const uint8_t *board,uint8_t player,int from,int to,DgMove *move,DgPath *path)
{
 if(!board || player<DG_RED || player>DG_GREEN || from<0 || from>=DG_NODES || to<0 || to>=DG_NODES || from==to || board[from]!=player || board[to])return false;
 for(int d=0;d<6;d++)if(dg_nodes[from].neighbor[d]==to){
  if(move)*move=(DgMove){(uint8_t)from,(uint8_t)to,DG_STEP,1};
  if(path){path->length=2;path->node[0]=(uint8_t)from;path->node[1]=(uint8_t)to;}return true;
 }
 int8_t parent[DG_NODES];uint8_t depth[DG_NODES];reach(board,from,parent,depth);
 if(depth[to]==DG_NONE)return false;
 if(move)*move=(DgMove){(uint8_t)from,(uint8_t)to,DG_JUMP,depth[to]};
 if(path){path->length=(uint8_t)(depth[to]+1);int at=to;for(int i=depth[to];i>=0;i--){path->node[i]=(uint8_t)at;at=parent[at];}}
 return true;
}
bool dg_apply(uint8_t *board,uint8_t player,const DgMove *move)
{
 DgMove valid;
 if(!move || !dg_find_move(board,player,move->from,move->to,&valid,NULL) || valid.type!=move->type || valid.hops!=move->hops)return false;
 board[move->from]=DG_EMPTY;board[move->to]=player;return true;
}
uint8_t dg_goal_count(const uint8_t *board,uint8_t player)
{
 if(!board || player<DG_RED || player>DG_GREEN)return 0;
 uint8_t count=0;for(int n=0;n<DG_NODES;n++)if(dg_in_camp(n,dg_goal[player]) && board[n]==player)count++;
 return count;
}
bool dg_won(const uint8_t *board,uint8_t player){return dg_goal_count(board,player)==DG_PIECES;}
