#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "node.h"
#include "priority_queue.h"


void initialize_ai();

move_t get_next_move( uint8_t board[SIZE][SIZE], int max_depth, propagation_t propagation, double heuristic);

node_t* init_node(node_t* node);

node_t** exploreAdd(node_t* node, node_t** explore, int *length);

void update( node_t* child, node_t* base );

void freeExplored( node_t** explored, int length);

void propagate_back(propagation_t propa, node_t* node, double score[SIZE], double empWeight);


int countBoard(uint8_t board[SIZE][SIZE], int mode);

move_t bestMove(double score [SIZE], int traversed[SIZE]);

void avgScore(double score[SIZE],node_t** explored, int length);


#endif
