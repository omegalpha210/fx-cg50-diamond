#include "ai.h"
#include <string.h>

static struct {
 DgRules rules;
 uint8_t goals[4][DG_PIECES],distance[4][DG_PIECES][DG_NODES];
} endgame_cache;
size_t dg_ai_endgame_workspace_bytes(void){return sizeof endgame_cache;}
static unsigned goal_depth(uint8_t player,unsigned node)
{
 int q=dg_nodes[node].q,r=dg_nodes[node].r;
 /* Rotate to the top arm: boundary r=-3, tip r=-6. */
 for(unsigned i=0;i<dg_goal[player];i++){int next_q=q+r;r=-q;q=next_q;}
 return (unsigned)(-r-3);
}
static void distances(DgRules rules)
{
 if(endgame_cache.rules.revision==rules.revision && endgame_cache.rules.players==rules.players)return;
 memset(endgame_cache.distance,DG_NONE,sizeof endgame_cache.distance);
 for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(rules.players==2 && player==DG_YELLOW)continue;
  unsigned goal=0;
  for(int start=0;start<DG_NODES;start++)if(dg_in_camp(start,dg_goal[player])){
   endgame_cache.goals[player][goal]=(uint8_t)start;
   uint8_t *distance=endgame_cache.distance[player][goal++],queue[DG_NODES];unsigned head=0,tail=0;
   distance[start]=0;queue[tail++]=(uint8_t)start;
   while(head<tail){unsigned at=queue[head++];for(unsigned d=0;d<6;d++){
    int to=dg_nodes[at].neighbor[d];
    if(dg_landing_allowed(rules,player,to) && distance[to]==DG_NONE){distance[to]=(uint8_t)(distance[at]+1u);queue[tail++]=(uint8_t)to;}
   }}
  }
 }
 endgame_cache.rules=rules;
}
bool dg_ai_endgame_metrics(DgRules rules,const uint8_t *board,uint8_t player,DgEndgame *metrics)
{
 if(!board || !metrics || !dg_rules_valid(rules) || player<DG_RED || player>DG_GREEN || (rules.players==2 && player==DG_YELLOW))return false;
 distances(rules);memset(metrics,0,sizeof *metrics);
 uint8_t pieces[DG_PIECES],holes[DG_PIECES];unsigned count=0,remaining=0;
 for(unsigned n=0;n<DG_NODES;n++)if(board[n]==player){
  if(!dg_landing_allowed(rules,player,(int)n))return false;
  if(dg_in_camp((int)n,dg_goal[player])){
   unsigned depth=goal_depth(player,n);metrics->goals++;metrics->depth=(uint8_t)(metrics->depth+depth);
   bool settled=true;
   for(unsigned d=0;d<6;d++){int next=dg_nodes[n].neighbor[d];
    if(next>=0 && dg_in_camp(next,dg_goal[player]) && goal_depth(player,(unsigned)next)>depth && board[next]!=player)settled=false;
   }
   metrics->settled+=(uint8_t)settled;
  }else{if(count>=DG_PIECES)return false;pieces[count++]=(uint8_t)n;}
 }
 for(unsigned i=0;i<DG_PIECES;i++)if(board[endgame_cache.goals[player][i]]!=player){
  holes[remaining++]=(uint8_t)i;
  if(board[endgame_cache.goals[player][i]]==DG_EMPTY)metrics->empty++;else metrics->blocked++;
 }
 if(count!=remaining || count+metrics->goals!=DG_PIECES)return false;
 /* Exact Hungarian assignment, O(n^3), at most ten pieces. Opponent-occupied
    goal holes remain required targets and are also counted as blocked. */
 int u[DG_PIECES+1]={0},v[DG_PIECES+1]={0};unsigned p[DG_PIECES+1]={0},way[DG_PIECES+1]={0};
 for(unsigned i=1;i<=count;i++){
  p[0]=i;unsigned j0=0;int minimum[DG_PIECES+1];bool used[DG_PIECES+1]={false};
  for(unsigned j=0;j<=count;j++)minimum[j]=2000000;
  do{
   used[j0]=true;unsigned i0=p[j0],j1=0;int delta=2000000;
   for(unsigned j=1;j<=count;j++)if(!used[j]){
    int cost=(int)endgame_cache.distance[player][holes[j-1]][pieces[i0-1]]-u[i0]-v[j];
    if(cost<minimum[j]){minimum[j]=cost;way[j]=j0;}
    if(minimum[j]<delta){delta=minimum[j];j1=j;}
   }
   for(unsigned j=0;j<=count;j++)if(used[j]){u[p[j]]+=delta;v[j]-=delta;}else minimum[j]-=delta;
   j0=j1;
  }while(p[j0]);
  do{unsigned j1=way[j0];p[j0]=p[j1];j0=j1;}while(j0);
 }
 metrics->assignment=(uint16_t)(-v[0]);return true;
}
