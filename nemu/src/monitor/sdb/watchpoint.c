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
    while(head) {
      printf("%s\r\n", head->exps);
    }
}

WP* new_wp() {
  if(free_ == NULL)
    return NULL;
  WP* pos = NULL;
  // get curren free position
  // inst before head node
  pos = free_;
  free_ = free_->next;
  pos->next = head;
  head = pos;
  return head;
}

void free_wp(WP *wp) {
  WP* pre = head;
  WP* pos = head;
  while(pos) {
    if(pos->NO == wp->NO) {
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