#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  printf("%s: %#010x\n","pc",cpu.pc);

  for(int idx = 0 ; idx < 32/4 ; idx ++) {
    for(int col = 0 ; col < 4 ; col++) {
      const char *name = reg_name(idx*col, 0);
      printf("%s: %#010x\t|\t",name,cpu.gpr[idx*col]._32);
    }
    printf("\n");
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  // how to find reg name?
  bool is_find = false;
  uint32_t reg_value = 0;
  for (int idx = 0 ; idx < 32 ; idx++) {
    if(strcmp(regs[idx], s) == 0) {
      is_find = true;
      reg_value = cpu.gpr[idx]._32;
    }
  }
  if(is_find) {
    printf("reg %s is %d\r\n",s,reg_value);
  }
  else {
    printf("there is no reg about %s\r\n",s);
  }
  return 0;
}
