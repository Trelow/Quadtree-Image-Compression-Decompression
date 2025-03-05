#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct pixel {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} pixel_t;

typedef struct tree {
  pixel_t pixel;
  unsigned char is_leaf;
  struct tree *child[4];
} tree_t;

typedef struct queue_node {
  tree_t *node;
  struct queue_node *next;
} queue_node_t;

typedef struct queue {
  struct queue_node *front;
  struct queue_node *rear;
  int size;

} queue_t;