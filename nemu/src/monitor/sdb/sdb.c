#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void watch_point_display();
int new_wp(char *e);
void free_wp(int num);
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_info(char *args) {
  
  char *arg = strtok(NULL, " ");
  if (arg == NULL) return 0;
  else if (0 == strcmp(arg, "r")) isa_reg_display();
  else if (0 == strcmp(arg, "w"))  {
    watch_point_display();
  }
  else printf("Error in cmd input\n");
  return 0;
}

static int cmd_si(char *args) {
  char *arg = strtok(NULL, " ");
  int step;
  if( arg == NULL) step = 1;
  else sscanf(arg,"%d",&step);
  cpu_exec(step);
  return 0;
}

static int cmd_e(char *args) {
  char *arg = strtok(NULL, " ");
  bool success;
  word_t res = 0;
  if(arg == NULL) printf("Please input the express\n");
  else res = expr(arg, &success);
  if(success)
    printf("%d\n",res);
  else
    printf("The express is error pls check\n");
  return 0;
}

static int cmd_w(char *args) {
  new_wp(args);
  return 0;
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  if( arg == NULL)
    printf("PLs input the delete number\r\n");
  int num;
  sscanf(arg,"%d",&num);
  free_wp(num);
  printf("Delete the %d watchpoint successd\r\n", num);
  return 0;
};

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"info", "Display infomations", cmd_info},
  {"si", "Single step exe", cmd_si},
  {"e", "Express", cmd_e},
  {"w", "Add watchpoint", cmd_w},
  {"d", "delete watchpoint", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
