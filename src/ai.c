#include <time.h>
#include <stdlib.h>
#include "ai.h"
#include "utils.h"
#include "priority_queue.h"

extern int genNodes_G;
extern int expNodes_G;


struct heap h;

/**
 *h heap, contains an array of pointers to node_t
 *It contains the size of the array.
 */

void initialize_ai(){
	heap_init(&h);
}

/**
 * Find best action by building all possible paths up to depth max_depth
 * and back propagate using either max or avg
 */

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, double empWeight ){
  /*Moves*/
  move_t best_action;
  move_t move=0;
  
  
  /*Recording Data: Record of Scores(LEFT->DOWN)*/
  double score[4]={0,0,0,0};
  //Traversed is flag needed: Cases where Score is 0 for all curMoves, but some of the curMoves not actual moves.
  int traversed[4]={0,0,0,0};  

 int i,j;
  /*Initializing a node to contain the current board*/
  node_t *value =NULL;
  value=init_node(value); 
  
  for (i=0;i<SIZE;i++){
    for(j=0;j<SIZE;j++){
      value->board[i][j]= board[i][j];
      }
  }
 
 /*Explored Array and a pointer to track its Length*/
  
 int* length=(int*)malloc (sizeof(int));
    *length=0;
 node_t **explored= (node_t**)malloc(*length*sizeof(node_t*));
 if(explored==NULL|| length==NULL) exit(-1);                                //Malloc Error

 
 /*Push current state into heap*/ 
  heap_push(&h, value);
  
  while(h.count>0){
    /*A node to capture nodes poped out of heap*/
     node_t* base;
     base=heap_delete(&h);   
     
    /*Node added to explore*/
     explored=exploreAdd(base,explored,length);      
     
    /*Search when not past max_depth*/
     if(base->depth<max_depth){
     
       /*Add a random tile to the popped out board*/
        if(base->depth>0){
          addRandom(base->board);
        }
        /*Loop through all four actions*/
        for(i=left;i<=down;i++){
        
          node_t* child=NULL;                          //A child is created to copy the base node
          child=init_node(child);   
          update(child, base);                        
          child->move=i;
          
          genNodes_G++;                                //Generated Nodes
          
            /*If a move is valid*/
           if(execute_move_t( child->board, &(child->priority), i)){
             expNodes_G ++;                            //Expanded Nodes
               
               /*Flags the possible first moves*/
               if(child->depth==1){
                 traversed[i]=1;
               }
               
               /*Process the scores*/
               propagate_back(propagation, child,score, empWeight);     
            
           /*Child is pushed into heap, or freed if not valid move*/          
               heap_push(&h, child);
           }
           else{
             free (child);
             }
             
           }  
     } 
   }
   
  /*Obtain an average, Done at end avoid small number division/multiplication problems*/
  if(propagation==1){
   avgScore(score,explored, *length);
  }
  
  /*Obtain the best move, unless depth =0 */
  move=bestMove(score, traversed);
  if(max_depth==0){
  	move = rand() % 4;
  }

   
   
  /*Free explored elements, explored, its length*/
   freeExplored( explored, *length);
  free (explored);
  free(length);
  
  best_action=move;
	return best_action;
}

/*Empty Cells: 1, Coner Piece: 0*/
int countBoard(uint8_t board[SIZE][SIZE], int mode ){
  int i,j;
  int heuristic=0;
  for(i=0;i<SIZE;i++){
    for(j=0;j<SIZE;j++){
      if( mode==1&&board[i][j]==0){      //Count empty cells
        heuristic+=1;
      }
      if(mode==0&&heuristic<board[i][j]){    //Find Maximum of Board  
        heuristic=board[i][j];
      }
    }
  }
  /*Corners: If maximum is at the corner*/      
  if(mode==0){
    if(board[0][0]==heuristic || board[0][3]==heuristic|| board[3][0]==heuristic||board[3][3]==heuristic){
      heuristic=1;
    }
    else
        heuristic=0;
  }
  return heuristic;
}

/*Process Score Avg*/
void avgScore(double score[SIZE],node_t** explored, int length){
  int i;
  for(i=0;i<(length);i++){
    if(explored[i]->depth==1)
     score[explored[i]->move]/=explored[i]->num_childs;
    }
} 
/*Break the tie*/

move_t bestMove(double score [SIZE], int traversed[SIZE]){
  move_t move=0;
  double max=0;
 int ties[SIZE]= {0,0,0,0};    //For breaking ties if ther exists
 int i,flag=0;
   
 /*The current move has to be valid*/
  for(i=0;i<4;i++){         
    if(!traversed[i]){
      continue;
    }
    if(max<=score[i]){
         max=score[i];
         move=i;
      } 
  }
 /*Record if there are ties*/
  for(i=0; i<4;i++){
      if(traversed[i]&&score[i]==max){
        ties[i]=1;
        flag=1;
     }
  }
  /*If there is a tie, randomly pick a move*/
    if(flag){
    	srand(time(NULL));
       move= rand()%4;
      while(ties[move]==0){
        move= rand()%4;
      }
    }
  return move;  
}

/*Every exploreAdd, realloc one extra space to contain the addition node*/
node_t** exploreAdd(node_t* node, node_t** explore, int *length){ 

    explore= (node_t**)realloc(explore, (*length+1)*sizeof(node_t*));
    if(explore==NULL){ 
       printf("Error allocatinga memory...\n");
		    exit(-1);
    }
     explore[*length]=node;
    *length+=1;
    
    return explore;
}

/*Free all explored elements*/
void freeExplored( node_t** explored, int length){
  int i;
  for(i=length-1;i>=0;i--){
    free(explored[i]);
  }
}

void propagate_back(propagation_t propa, node_t* node, double score[SIZE], double empWeight){
  
  node_t* tmp=node; 
  int max=0,count=0;
  double priority= (double)node->priority;
  double empty=0;
  
   /*Heuristics, 0.35 best*/
   
   max=countBoard(node->board,0);      //Getting Max Corner
   count= countBoard(node->board,1);   //Getting Empty Tiles
   empty=count*empWeight;              //Weight for Empty Tiles
   
   priority=priority*(1+max+count);    //Max and Count give bonus
  
  while(tmp->depth>1){                //Find the current move this child belongs to
    tmp=tmp->parent;
  }
  
  if(propa==0){                       //If propagation = Max
    if(score[tmp->move]< priority){
      score[tmp->move]= priority;
    }
  }
 
 
  if(propa==1){                        //If propagation = Avg
    if(node->depth>1){ 
          priority/=node->depth;      //If depth is high, less likely to occur in game
    }
     score[tmp->move]+=priority;      //Sum all scores
     tmp->num_childs+=1;               //Count children
  }
}

/*Used to prepare child node*/
void update( node_t* child, node_t* base ){
  *child =*base;
  child->priority=base->priority;
  child->depth= base->depth+1;
  child->parent =base;  
}

/*Initializing node*/
node_t* init_node(node_t* node){
  node=(node_t*) malloc (sizeof(node_t));
  if(node==NULL){
    printf("Error allocatinga memory...\n");
		exit(-1);
  }
    node->priority=0;
    node->depth=0;
    node->num_childs=0;
    node->move=0;
    node->parent=NULL;
    return node;
}



