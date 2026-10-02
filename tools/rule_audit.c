#include "diamond.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
 bool first=true;puts("[");
 for(uint8_t players=2;players<=3;players++)for(uint8_t player=DG_RED;player<=DG_GREEN;player++){
  if(players==2 && player==DG_YELLOW)continue;
  if(!first)puts(",");first=false;DgRules rules={DG_RULES_V2,players};
  printf("{\"players\":%u,\"player\":%u,\"allowed_nodes\":[",(unsigned)players,(unsigned)player);
  bool first_node=true;for(int n=0;n<DG_NODES;n++)if(dg_landing_allowed(rules,player,n)){
   printf(first_node?"%d":",%d",n);first_node=false;
  }
  printf("],\"reachability\":[");bool first_home=true;
  for(int start=0;start<DG_NODES;start++)if(dg_in_camp(start,dg_home[player])){
   uint8_t queue[DG_NODES];bool seen[DG_NODES]={false};unsigned head=0,tail=0;
   queue[tail++]=(uint8_t)start;seen[start]=true;
   while(head<tail){uint8_t from=queue[head++],board[DG_NODES]={0};board[from]=player;
    DgMove moves[DG_NODES];size_t count=dg_piece_moves(rules,board,player,from,moves,DG_NODES);
    for(size_t i=0;i<count;i++)if(!seen[moves[i].to]){seen[moves[i].to]=true;queue[tail++]=moves[i].to;}
   }
   unsigned goals=0;for(int n=0;n<DG_NODES;n++)if(seen[n] && dg_in_camp(n,dg_goal[player]))goals++;
   assert(goals==10);printf(first_home?"":" ,");first_home=false;
   printf("{\"start\":%d,\"reachable_goals\":%u}",start,goals);
  }
  printf("]}");
 }
 puts("\n]");return 0;
}
