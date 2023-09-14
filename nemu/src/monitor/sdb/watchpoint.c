#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char exps[100];
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void watch_point_display() {
  WP* pos = head;
  while(pos) {
    printf("watchpoint id: %d\texpr: %s\r\n", pos->NO, pos->exps);
    pos = pos->next;
  }
}

int new_wp(char *e) {
  if(free_ == NULL)
    return -1;
  WP* pos = NULL;
  // get curren free position
  // inst before head node
  pos = free_;
  free_ = free_->next;
  pos->next = head;
  head = pos;
  strcpy(head->exps, e);
  if(head->next == NULL) 
    head->NO = 0;
  else
    head->NO = head->next->NO+1;
  return 0;
}

void free_wp(int num) {
  WP* pre = head;
  WP* pos = head;
  while(pos) {
    if(pos->NO == num) {
      pre->next = pos->next;
      // inst before free head node
      pos->next = free_;
      free_ = pos;
      break;
    }
    pre = pos;
    pos = pos->next;
  }
}