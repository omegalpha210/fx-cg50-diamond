/* Host-only difficulty quality and matched-game audit. Include the existing
   engine-valid tactical builders; the calculator never links this file. */
#define main dg_fixture_audit_main
#include "ai_fixtures.c"
#undef main

#define POSITION_COUNT 32u
static DgGame suite[2][POSITION_COUNT];
static char labels[2][POSITION_COUNT][48];
static void build_suite(void)
{
 for(uint8_t players=2u;players<=3u;++players){
  unsigned mode=(unsigned)players-2u;
  suite[mode][0]=base(players,false);suite[mode][1]=midgame(players);
  suite[mode][2]=congestion(players);suite[mode][3]=multijump(players);
  suite[mode][4]=nearly_won(players);suite[mode][5]=blocking(players);
  suite[mode][6]=nearly_won(players);suite[mode][6].pos.turn=suite[mode][6].human_slot;
  CHECK(dg_new(&suite[mode][7],players,DG_EASY,0u,23u));
  static const char *const first[8]={"opening_green","reachable_middle","goal_congestion","long_jump","near_win","central_blocking","defensive_threat","opening_red"};
  for(unsigned i=0u;i<8u;++i)(void)snprintf(labels[mode][i],sizeof labels[mode][i],"%s",first[i]);
  static const uint32_t seeds[3]={19u,71u,991u};
  static const unsigned rounds[8]={2u,4u,6u,8u,10u,12u,16u,20u};
  for(unsigned seed=0u;seed<3u;++seed){
   DgGame game;CHECK(dg_new(&game,players,DG_EASY,1u,seeds[seed]));
   for(unsigned stage=0u;stage<8u;++stage){
    unsigned target=rounds[stage]*(unsigned)players;
    while(game.pos.turns<target){
     DgMove move;uint32_t rng;DgAiStats stats;
     CHECK(!game.pos.winner);CHECK(dg_ai_choose(&game,0u,NULL,NULL,&move,&rng,&stats));
     CHECK(dg_commit(&game,&move));game.pos.rng=rng;
    }
    unsigned index=8u+seed*8u+stage;suite[mode][index]=game;
    const char *category=stage<2u?"early_middle":(stage<5u?"central_race":"goal_entry");
    (void)snprintf(labels[mode][index],sizeof labels[mode][index],"%s_s%lu_r%u",category,(unsigned long)seeds[seed],rounds[stage]);
   }
  }
  for(unsigned index=0u;index<POSITION_COUNT;++index)CHECK(dg_game_valid(&suite[mode][index]) && !suite[mode][index].pos.winner);
 }
}

static void choices(void)
{
 puts("mode,position,category,board_hash,current,level,from,to,type,hops,next_rng,legal_moves,nodes,depth,beam,host_seconds");
#ifdef DG_STRENGTH_BASELINE
 static const uint8_t levels[2]={DG_EASY,DG_HARD};
#else
 static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
#endif
 for(unsigned mode=0u;mode<2u;++mode)for(unsigned position=0u;position<POSITION_COUNT;++position)
  for(unsigned difficulty=0u;difficulty<sizeof levels/sizeof levels[0];++difficulty){
   DgGame game=suite[mode][position];game.level=levels[difficulty];
   DgGame before=game;DgMove move;uint32_t rng;DgAiStats stats;clock_t start=clock();
   CHECK(dg_ai_choose(&game,0u,NULL,NULL,&move,&rng,&stats));
   double elapsed=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
   CHECK(!memcmp(&before,&game,sizeof game));CHECK(dg_find_move(dg_rules(&game),game.pos.board,dg_current(&game),move.from,move.to,NULL,NULL));
   const char *name=difficulty==0u?"EASY":
#ifdef DG_STRENGTH_BASELINE
   "HARD";
#else
   (difficulty==1u?"NORMAL":"HARD");
#endif
   printf("%uP,%u,%s,%016llx,%u,%s,%u,%u,%u,%u,%lu,%u,%lu,%u,%u,%.6f\n",
    (unsigned)game.players,position,labels[mode][position],(unsigned long long)board_hash(&game),(unsigned)dg_current(&game),name,
    (unsigned)move.from,(unsigned)move.to,(unsigned)move.type,(unsigned)move.hops,(unsigned long)rng,
    (unsigned)stats.legal_moves,(unsigned long)stats.nodes,(unsigned)stats.depth,(unsigned)stats.beam,elapsed);
  }
}

#ifndef DG_STRENGTH_BASELINE
static const char *level_name(uint8_t level)
{return level==DG_EASY?"EASY":(level==DG_NORMAL?"NORMAL":"HARD");}
static void paths(void)
{
 puts("mode,position,level,from,to,type,hops,next_rng,nodes,path");
 const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
 for(unsigned mode=0;mode<2;mode++)for(unsigned pos=0;pos<POSITION_COUNT;pos++)for(unsigned l=0;l<3;l++){
  DgGame g=suite[mode][pos];g.level=levels[l];DgMove m;DgPath p={0};uint32_t rng;DgAiStats stats;
  CHECK(dg_ai_choose(&g,0,NULL,NULL,&m,&rng,&stats));
  CHECK(dg_find_move(dg_rules(&g),g.pos.board,dg_current(&g),m.from,m.to,NULL,&p));
  printf("%uP,%u,%s,%u,%u,%u,%u,%lu,%lu,",(unsigned)g.players,pos,level_name(g.level),(unsigned)m.from,(unsigned)m.to,(unsigned)m.type,(unsigned)m.hops,(unsigned long)rng,(unsigned long)stats.nodes);
  for(unsigned n=0;n<p.length;n++)printf(n?":%u":"%u",(unsigned)p.node[n]);
  puts("");
 }
}

static unsigned number(const char *text,unsigned fallback)
{
 if(text==NULL)return fallback;char *end=NULL;unsigned long value=strtoul(text,&end,10);
 return *end==0 && value>0ul && value<=10000ul?(unsigned)value:fallback;
}
static void quality(void)
{
 puts("mode,position,category,current,level,from,to,reference_best,reference_choice,regret,select_nodes,select_depth,select_host_seconds,reference_nodes,reference_host_seconds");
 for(unsigned mode=0u;mode<2u;++mode){
  double regret_total[3]={0.0,0.0,0.0};unsigned best_count[3]={0u,0u,0u};
  for(unsigned position=0u;position<POSITION_COUNT;++position){
   DgGame game=suite[mode][position];DgMove chosen[3];DgAiStats selected_stats[3];double selected_time[3];
   static const uint8_t levels[3]={DG_EASY,DG_NORMAL,DG_HARD};
   for(unsigned level=0u;level<3u;++level){
    game.level=levels[level];uint32_t rng;clock_t start=clock();
    CHECK(dg_ai_choose(&game,0u,NULL,NULL,&chosen[level],&rng,&selected_stats[level]));
    selected_time[level]=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
   }
   DgMove legal[DG_MAX_MOVES];int score[DG_MAX_MOVES];
   size_t count=dg_generate(dg_rules(&game),game.pos.board,dg_current(&game),legal,DG_MAX_MOVES);
   CHECK(count>0u && count<=DG_MAX_MOVES);int best=-2000000;uint64_t nodes=0u;clock_t reference_start=clock();
   for(size_t candidate=0u;candidate<count;++candidate){
    DgAiStats stats;CHECK(dg_ai_reference_score(&game,&legal[candidate],&score[candidate],&stats));
    nodes+=stats.nodes;if(score[candidate]>best)best=score[candidate];
   }
   double reference_time=(double)(clock()-reference_start)/(double)CLOCKS_PER_SEC;
   for(unsigned level=0u;level<3u;++level){
    int value=INT_MIN;
    for(size_t candidate=0u;candidate<count;++candidate)
     if(!memcmp(&legal[candidate],&chosen[level],sizeof(DgMove)))value=score[candidate];
    CHECK(value!=INT_MIN && value<=best);int regret=best-value;regret_total[level]+=(double)regret;
    if(regret==0)++best_count[level];
    printf("%uP,%u,%s,%u,%s,%u,%u,%d,%d,%d,%lu,%u,%.6f,%llu,%.6f\n",
     (unsigned)game.players,position,labels[mode][position],(unsigned)dg_current(&game),level_name(levels[level]),
     (unsigned)chosen[level].from,(unsigned)chosen[level].to,best,value,regret,
     (unsigned long)selected_stats[level].nodes,(unsigned)selected_stats[level].depth,selected_time[level],
     (unsigned long long)nodes,reference_time);
   }
   fflush(stdout);
  }
  fprintf(stderr,"%uP quality mean_regret EASY=%.3f NORMAL=%.3f HARD=%.3f; best=%u/%u/%u of%u; evaluator-based reference only\n",
   mode+2u,regret_total[0]/POSITION_COUNT,regret_total[1]/POSITION_COUNT,regret_total[2]/POSITION_COUNT,
   best_count[0],best_count[1],best_count[2],POSITION_COUNT);
 }
}
static void play_move(DgGame *game,uint8_t level,uint64_t *nodes)
{
 game->level=level;DgGame before=*game;DgMove move;uint32_t rng;DgAiStats stats;
 CHECK(dg_ai_choose(game,0u,NULL,NULL,&move,&rng,&stats));
 CHECK(!memcmp(&before,game,sizeof before));
 CHECK(dg_find_move(dg_rules(game),game->pos.board,dg_current(game),move.from,move.to,NULL,NULL));
 CHECK(dg_commit(game,&move));game->pos.rng=rng;CHECK(dg_game_valid(game));
 if(nodes!=NULL)*nodes+=stats.nodes;
}
static bool repeat_state(const DgGame *game,const DgPosition *position)
{return game->pos.turn==position->turn && game->pos.rng==position->rng && !memcmp(game->pos.board,position->board,DG_NODES);}
static void tournament(unsigned seeds,unsigned cap)
{
 static const uint8_t pair[3][2]={{DG_NORMAL,DG_EASY},{DG_HARD,DG_NORMAL},{DG_HARD,DG_EASY}};
 puts("mode,pair,seed,red,green,first,winner,turns,red_goal,green_goal,nodes,cycle,timeout,host_seconds");
 for(unsigned pairing=0u;pairing<3u;++pairing){
  unsigned wins[2]={0u,0u},timeouts=0u,cycles=0u;uint64_t total_turns=0u;
  for(unsigned seed=0u;seed<seeds;++seed)for(uint8_t first=0u;first<2u;++first)for(unsigned swap=0u;swap<2u;++swap){
   uint32_t game_seed=UINT32_C(0x13579bdf)+seed*UINT32_C(104729);
   DgGame game;CHECK(dg_new(&game,2u,DG_EASY,first,game_seed));
   /* The same seeded EASY opening precedes each seat-swapped pairing. */
   for(unsigned opening=0u;opening<8u;++opening)play_move(&game,DG_EASY,NULL);
   uint8_t red=pair[pairing][swap],green=pair[pairing][1u-swap];
   DgPosition recent[128];unsigned recent_count=0u,recent_at=0u;bool cycle=false;uint64_t nodes=0u;clock_t start=clock();
   while(!game.pos.winner && game.pos.turns<cap){
    for(unsigned prior=0u;prior<recent_count;++prior)if(repeat_state(&game,&recent[prior]))cycle=true;
    if(cycle)break; /* Diagnostic stop for deterministic state recurrence, not a game rule. */
    recent[recent_at++%128u]=game.pos;if(recent_count<128u)++recent_count;
    play_move(&game,dg_current(&game)==DG_RED?red:green,&nodes);
   }
   double time=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
   unsigned timeout=!game.pos.winner && !cycle;timeouts+=timeout;cycles+=cycle?1u:0u;total_turns+=game.pos.turns;
   if(game.pos.winner){uint8_t winner_level=game.pos.winner==DG_RED?red:green;wins[winner_level==pair[pairing][0]?0u:1u]++;}
   printf("2P,%s_vs_%s,%lu,%s,%s,%s,%s,%lu,%u,%u,%llu,%u,%u,%.6f\n",
    level_name(pair[pairing][0]),level_name(pair[pairing][1]),(unsigned long)game_seed,level_name(red),level_name(green),
    first==0u?"RED":"GREEN",game.pos.winner?level_name(game.pos.winner==DG_RED?red:green):"NONE",
    (unsigned long)game.pos.turns,(unsigned)dg_goal_count(game.pos.board,DG_RED),(unsigned)dg_goal_count(game.pos.board,DG_GREEN),
    (unsigned long long)nodes,cycle?1u:0u,timeout,time);fflush(stdout);
  }
  fprintf(stderr,"2P %s_vs_%s games=%u wins=%u/%u cycles=%u timeouts=%u mean_turns=%.2f illegal=0\n",
   level_name(pair[pairing][0]),level_name(pair[pairing][1]),seeds*4u,wins[0],wins[1],cycles,timeouts,
   (double)total_turns/(double)(seeds*4u));
 }
}
static void three_player(unsigned seeds,unsigned cap,bool control)
{
 static const uint8_t permutations[6][3]={
  {DG_EASY,DG_NORMAL,DG_HARD},{DG_EASY,DG_HARD,DG_NORMAL},{DG_NORMAL,DG_EASY,DG_HARD},
  {DG_NORMAL,DG_HARD,DG_EASY},{DG_HARD,DG_EASY,DG_NORMAL},{DG_HARD,DG_NORMAL,DG_EASY}
 };
 static const uint8_t controls[6][3]={
  {DG_NORMAL,DG_EASY,DG_EASY},{DG_EASY,DG_NORMAL,DG_EASY},{DG_EASY,DG_EASY,DG_NORMAL},
  {DG_EASY,DG_NORMAL,DG_NORMAL},{DG_NORMAL,DG_EASY,DG_NORMAL},{DG_NORMAL,DG_NORMAL,DG_EASY}
 };
 unsigned exposure[3]={0u,0u,0u};
 unsigned wins[3]={0u,0u,0u},timeouts=0u,cycles=0u,games=0u;uint64_t turns=0u,progress[3]={0u,0u,0u};double ranks[3]={0.0,0.0,0.0};
 puts("mode,seed,red,yellow,green,human_slot,winner,turns,red_goal,yellow_goal,green_goal,nodes,cycle,timeout,host_seconds");
 for(unsigned seed=0u;seed<seeds;++seed)for(unsigned rotation=0u;rotation<6u;++rotation)for(uint8_t slot=0u;slot<3u;++slot){
  uint32_t game_seed=UINT32_C(0x2468ace1)+seed*UINT32_C(104729);
  DgGame game;CHECK(dg_new(&game,3u,DG_EASY,slot,game_seed));
  for(unsigned opening=0u;opening<9u;++opening)play_move(&game,DG_EASY,NULL);
  const uint8_t *permutation=control?controls[rotation]:permutations[rotation];
  uint8_t level[4]={DG_EASY,permutation[0],permutation[1],permutation[2]};
  DgPosition recent[128];unsigned recent_count=0u,recent_at=0u;bool cycle=false;uint64_t nodes=0u;clock_t start=clock();
  while(!game.pos.winner && game.pos.turns<cap){
   for(unsigned prior=0u;prior<recent_count;++prior)if(repeat_state(&game,&recent[prior]))cycle=true;
   if(cycle)break;
   recent[recent_at++%128u]=game.pos;if(recent_count<128u)++recent_count;
   play_move(&game,level[dg_current(&game)],&nodes);
  }
  double time=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
  unsigned timeout=!game.pos.winner && !cycle;timeouts+=timeout;cycles+=cycle?1u:0u;++games;turns+=game.pos.turns;
  if(game.pos.winner)++wins[level[game.pos.winner]];
  unsigned goals[4]={0u,0u,0u,0u};for(uint8_t player=DG_RED;player<=DG_GREEN;++player)goals[player]=dg_goal_count(game.pos.board,player);
  for(uint8_t player=DG_RED;player<=DG_GREEN;++player){
   ++exposure[level[player]];progress[level[player]]+=goals[player];double rank=1.0;
   for(uint8_t other=DG_RED;other<=DG_GREEN;++other)if(other!=player){
    if(goals[other]>goals[player])rank+=1.0;else if(goals[other]==goals[player])rank+=0.5;
   }
   ranks[level[player]]+=rank;
  }
  printf("%s,%lu,%s,%s,%s,%u,%s,%lu,%u,%u,%u,%llu,%u,%u,%.6f\n",
   control?"3P_CONTROL":"3P",(unsigned long)game_seed,level_name(level[DG_RED]),level_name(level[DG_YELLOW]),level_name(level[DG_GREEN]),(unsigned)slot,
   game.pos.winner?level_name(level[game.pos.winner]):"NONE",(unsigned long)game.pos.turns,goals[DG_RED],goals[DG_YELLOW],goals[DG_GREEN],
   (unsigned long long)nodes,cycle?1u:0u,timeout,time);fflush(stdout);
 }
 fprintf(stderr,"3P games=%u wins EASY/NORMAL/HARD=%u/%u/%u cycles=%u timeouts=%u mean_turns=%.2f illegal=0\n",
  games,wins[DG_EASY],wins[DG_NORMAL],wins[DG_HARD],cycles,timeouts,(double)turns/games);
 fprintf(stderr,"3P mean_goal EASY/NORMAL/HARD=%.3f/%.3f/%.3f; goal_rank_proxy=%.3f/%.3f/%.3f (lower better)\n",
  (double)progress[DG_EASY]/exposure[DG_EASY],(double)progress[DG_NORMAL]/exposure[DG_NORMAL],
  exposure[DG_HARD]?(double)progress[DG_HARD]/exposure[DG_HARD]:0.0,
  ranks[DG_EASY]/exposure[DG_EASY],ranks[DG_NORMAL]/exposure[DG_NORMAL],
  exposure[DG_HARD]?ranks[DG_HARD]/exposure[DG_HARD]:0.0);
}
static uint8_t parse_level(const char *name)
{
 if(!strcmp(name,"EASY"))return DG_EASY;
 if(!strcmp(name,"NORMAL"))return DG_NORMAL;
 if(!strcmp(name,"HARD"))return DG_HARD;
 return DG_NONE;
}
static unsigned split(char *line,char **fields,unsigned capacity)
{
 unsigned count=0u;char *field=strtok(line,",\r\n");
 while(field!=NULL){CHECK(count<capacity);fields[count++]=field;field=strtok(NULL,",\r\n");}
 return count;
}
static void recheck(const char *path,unsigned cap)
{
 FILE *input=fopen(path,"r");CHECK(input!=NULL);char line[512],header[512],*column_names[32],*fields[32];
 CHECK(fgets(header,sizeof header,input)!=NULL);unsigned columns=split(header,column_names,32u);
 int seed_index=-1,red_index=-1,yellow_index=-1,green_index=-1,slot_index=-1,first_index=-1,winner_index=-1,turns_index=-1;
 for(unsigned i=0u;i<columns;++i){
  if(!strcmp(column_names[i],"seed"))seed_index=(int)i;
  else if(!strcmp(column_names[i],"red"))red_index=(int)i;
  else if(!strcmp(column_names[i],"yellow"))yellow_index=(int)i;
  else if(!strcmp(column_names[i],"green"))green_index=(int)i;
  else if(!strcmp(column_names[i],"human_slot"))slot_index=(int)i;
  else if(!strcmp(column_names[i],"first"))first_index=(int)i;
  else if(!strcmp(column_names[i],"winner"))winner_index=(int)i;
  else if(!strcmp(column_names[i],"turns"))turns_index=(int)i;
 }
 CHECK(seed_index>=0 && red_index>=0 && green_index>=0 && winner_index>=0 && turns_index>=0);
 unsigned cases=0u,completed=0u;
 puts("mode,seed,red,yellow,green,human_slot,original_turns,recheck_cap,winner,turns,red_goal,yellow_goal,green_goal,nodes,host_seconds");
 while(fgets(line,sizeof line,input)!=NULL){
  CHECK(split(line,fields,32u)==columns);
  if(strcmp(fields[winner_index],"NONE"))continue;
  unsigned long parsed=strtoul(fields[seed_index],NULL,10);CHECK(parsed>0ul && parsed<=UINT32_MAX);
  uint32_t seed=(uint32_t)parsed;uint8_t players=yellow_index>=0?3u:2u;
  uint8_t slot=players==3u?(uint8_t)strtoul(fields[slot_index],NULL,10):(uint8_t)(!strcmp(fields[first_index],"RED")?0u:1u);
  uint8_t level[4]={DG_EASY,parse_level(fields[red_index]),players==3u?parse_level(fields[yellow_index]):DG_EASY,parse_level(fields[green_index])};
  CHECK(level[DG_RED]!=DG_NONE && level[DG_YELLOW]!=DG_NONE && level[DG_GREEN]!=DG_NONE);
  DgGame game;CHECK(dg_new(&game,players,DG_EASY,slot,seed));
  for(unsigned opening=0u;opening<(players==2u?8u:9u);++opening)play_move(&game,DG_EASY,NULL);
  uint64_t nodes=0u;clock_t start=clock();
  while(!game.pos.winner && game.pos.turns<cap)play_move(&game,level[dg_current(&game)],&nodes);
  double elapsed=(double)(clock()-start)/(double)CLOCKS_PER_SEC;
  ++cases;if(game.pos.winner)++completed;
  printf("%uP,%lu,%s,%s,%s,%u,%s,%u,%s,%lu,%u,%u,%u,%llu,%.6f\n",
   (unsigned)players,parsed,level_name(level[DG_RED]),players==3u?level_name(level[DG_YELLOW]):"ABSENT",level_name(level[DG_GREEN]),
   (unsigned)slot,fields[turns_index],cap,game.pos.winner?level_name(level[game.pos.winner]):"NONE",(unsigned long)game.pos.turns,
   (unsigned)dg_goal_count(game.pos.board,DG_RED),(unsigned)dg_goal_count(game.pos.board,DG_YELLOW),(unsigned)dg_goal_count(game.pos.board,DG_GREEN),
   (unsigned long long)nodes,elapsed);fflush(stdout);
 }
 CHECK(fclose(input)==0);
 fprintf(stderr,"recheck cases=%u completed=%u still_unfinished=%u cap=%u; no gameplay rule change\n",cases,completed,cases-completed,cap);
}

#endif

int main(int argc,char **argv)
{
#ifndef DG_STRENGTH_BASELINE
 if(argc>2 && !strcmp(argv[1],"--recheck"))recheck(argv[2],number(argc>3?argv[3]:NULL,1200u));
 else if(argc>1 && !strcmp(argv[1],"--tournament"))tournament(number(argc>2?argv[2]:NULL,25u),number(argc>3?argv[3]:NULL,400u));
 else if(argc>1 && !strcmp(argv[1],"--3p"))three_player(number(argc>2?argv[2]:NULL,8u),number(argc>3?argv[3]:NULL,400u),false);
 else if(argc>1 && !strcmp(argv[1],"--3p-control"))three_player(number(argc>2?argv[2]:NULL,8u),number(argc>3?argv[3]:NULL,400u),true);
 else{build_suite();if(argc>1 && !strcmp(argv[1],"--quality"))quality();else if(argc>1 && !strcmp(argv[1],"--paths"))paths();else choices();}
#else
 (void)argc;(void)argv;build_suite();choices();
#endif
 fprintf(stderr,"difficulty audit PASS: %u checks; host timing only\n",checks);
 return 0;
}
