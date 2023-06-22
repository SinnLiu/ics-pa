#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM,
  /* TODO: Add more token types */
  TK_PLUS = '+',
  TK_SUB = '-',
  TK_MUL = '*',
  TK_DIV = '/',
  TK_HEX,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"-", '-'},         // sub
  {"==", TK_EQ},        // equal
  {"\\b[0-9]+\\b", TK_NUM},
  {"\\*", '*'},       // mul
  {"\\/", '/'},       // div
  {"\\(", '('},
  {"\\)", ')'},
  {"\\b0x[0-9,a-e]+\\b",TK_HEX},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        
        // switch (rules[i].token_type) {
        //   case TK_NUM:
            
        //     strncpy(tokens[nr_token].str, substr_start, substr_len);
        //     break;
        //   case '+':
        //     break;
        //   default: TODO();
        // }
        if(rules[i].token_type != TK_NOTYPE) {
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

int order(char op) {
  if(op == '(') return 0;
  if(op == '+' || op == '-') return 1;
  if(op == '*' || op == '/') return 2;
  return -1;
}

bool calculate(int *num_array, int p_num_array, char op) {
  if(op == '*') {
    num_array[p_num_array-2] *= num_array[p_num_array-1];
  }
  else if(op == '/') {
    if(num_array[p_num_array-1] == 0) return false;
    num_array[p_num_array-2] /= num_array[p_num_array-1];
  }
  else if(op == '+') {
    num_array[p_num_array-2] += num_array[p_num_array-1];         
  }
  else if(op == '-') {
    num_array[p_num_array-2] -= num_array[p_num_array-1];
  }
  return true;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // TODO();
  // for(int i = 0 ; i < nr_token ; i++) {
  //   printf("%s\n",tokens[i].str);
  // }
  *success = true;
  int num_array[32] = {0};
  int p_num_array = 0;
  char char_array[32] = {0};
  int p_char_array = 0;
  for(int i = 0 ; i < nr_token ; i++) {
    if(!(*success)) break;
    switch(tokens[i].type) {
      case TK_NUM:
        int temp_num = 0;
        int p_temp_num = 0;
        while(tokens[i].str[p_temp_num] != '\0') {
          temp_num = temp_num * 10 + (tokens[i].str[p_temp_num] - '0');
          p_temp_num++;
        }
        num_array[p_num_array++] = temp_num;
        break;
      case '(':
        char_array[p_char_array++] = tokens[i].type;
        break;
      case ')':
        while(p_char_array != 0 && char_array[p_char_array-1] != '(') {
          *success = calculate(num_array,p_num_array,char_array[p_char_array-1]);
          p_num_array--;
          p_char_array--;
        }
        if(p_char_array == 0 || char_array[p_char_array-1] != '(') 
          *success = false;
        else
          p_char_array--;
        break;
      case '+':
      case '-':
      case '*':
      case '/':
        while(p_char_array != 0) {
          if(order(char_array[p_char_array-1]) >= order(tokens[i].type)) {
            *success = calculate(num_array,p_num_array,char_array[p_char_array-1]);
            p_num_array--;
            p_char_array--;
          }
          else break;
        }
        char_array[p_char_array++] = tokens[i].type;
        break;
      case TK_HEX:
        int temp_num_hex = 0;
        int p_temp_num_hex = 2;
        while(tokens[i].str[p_temp_num_hex] != '\0') {
          temp_num_hex <<= 4; 
          if(tokens[i].str[p_temp_num_hex] >= 'a') 
            temp_num_hex += (10 + (tokens[i].str[p_temp_num_hex] - 'a'));
          else 
            temp_num_hex += (tokens[i].str[p_temp_num_hex] - '0');
          p_temp_num_hex++;
        }
        num_array[p_num_array++] = temp_num_hex;
        break;
      default:
        break;
    }
  }
  
  while(p_char_array != 0 && (*success)) {
    if(char_array[p_char_array-1] == '(') {*success = false; break;}
    *success = calculate(num_array,p_num_array,char_array[p_char_array-1]);
    p_num_array--;
    p_char_array--;
  }
  if(!(*success)) return 0;
  // printf("%d\n",num_array[0]);
  return num_array[0];
}
